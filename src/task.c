#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>

#include "impl.h"

void _deltask(struct Glist *l, struct G *g) {
	if(g->prev)
		g->prev->next = g->next;
	else
		l->head = g->next;
	if(g->next)
		g->next->prev = g->prev;
	else
		l->tail = g->prev;
}

static void addtask(struct Glist *l, struct G *g)
{
	if(l->tail){
		l->tail->next = g;
		g->prev = l->tail;
	}else{
		l->head = g;
		g->prev = NULL;
	}
	l->tail = g;
	g->next = NULL;
}

// debugging
void taskname(char *fmt, ...) {
	va_list arg;
	struct G *t;

	t = _thread()->g;
	va_start(arg, fmt);
	vsnprintf(t->name, sizeof t->name, fmt, arg);
	va_end(arg);
}

static void contextswitch(Context *from, Context *to) {
    if(swapcontext(&from->uc, &to->uc) < 0){
        abort();
    }
}

void _taskready(struct G *g)
{
	struct M *m;
    
    m = g->m;
	g->ready = 1;
	addtask(&m->runqueue, g);
}

static void taskexits(char *msg) {
    struct M *m;
    struct G *g;
    
    m = _thread();
    g = m->g;
    g->exiting = 1;
    contextswitch(&g->context, &m->schedcontext);
}

void _needstack(int n) {
    struct G *g;
    
    g = _thread()->g;
    
    if((char*)&g <= (char*)g->stk
       || (char*)&g - (char*)g->stk < 256+n){
        printf( "task stack overflow: &t=%p tstk=%p n=%d\n", &g, g->stk, 256+n);
        abort();
    }
}

void _taskswitch(void) {
    struct M *m;
    struct G *g;
    
    _needstack(0);
    m = _thread();
    g = m->g;
    contextswitch(&g->context, &m->schedcontext);
}

static void taskstart(uint y, uint x) {
	struct G *g;
	ulong z;

	z = (ulong)x << 16;
	z <<= 16;
	z |= y;
	g = (struct G*)z;
	g->startfn(g->startarg);
	taskexits(NULL);
}

static struct G* _taskalloc(void (*fn)(void*), void *arg, uint stack) {
	struct G *g;
	sigset_t zero;
	uint x, y;
	ulong z;

	/* allocate the task and stack together */
	g = malloc(sizeof(struct Glist)+stack);
	memset(g, 0, sizeof(struct G));
	g->stk = (uchar*)(g+1);
	g->stksize = stack;
	// g->id = ++taskidgen;
	g->startfn = fn;
	g->startarg = arg;

	/* do a reasonable initialization */
	memset(&g->context.uc, 0, sizeof(g->context.uc));
	 sigemptyset(&zero);
	 sigprocmask(SIG_BLOCK, &zero, &g->context.uc.uc_sigmask);

	/* must initialize with current context */
	if(getcontext(&g->context.uc) < 0){
		abort();
	}

	/* call makecontext to do the real work. */
	/* leave a few words open on both ends */
	g->context.uc.uc_stack.ss_sp = g->stk+8;
	g->context.uc.uc_stack.ss_size = g->stksize-64;

	/*
	 * All this magic is because you have to pass makecontext a
	 * function that takes some number of word-sized variables,
	 * and on 64-bit machines pointers are bigger than words.
	 */
	z = (ulong)g;
	y = (uint)z;
	z >>= 16;	/* hide undefined 32-bit shift from 32-bit compilers */
	x = (uint)(z>>16);
	makecontext(&g->context.uc, (void(*)())taskstart, 2, y, x);

	return g;
}


static void addGtoM(struct M *m, struct G *g) {
	struct Glist *l;

	l = &m->allg;
	if(l->tail){
		l->tail->allnext = g;
		g->allprev = l->tail;
	}else{
		l->head = g;
		g->allprev = NULL;
	}
	l->tail = g;
	g->allnext = NULL;
}

static void delGinM(struct M *m, struct G *g) {
	struct Glist *l;

	l = &m->allg;
	if(g->allprev)
		g->allprev->allnext = g->allnext;
	else
		l->head = g->allnext;
	if(g->allnext)
		g->allnext->allprev = g->allprev;
	else
		l->tail = g->allprev;
}

struct G* _taskcreate(struct M* m, void (*fn)(void*), void *arg, uint stack) {
	struct G *g;
	
	g = _taskalloc(fn, arg, stack);
	g->m = m;
	addGtoM(m, g);
	m->nthread++;
	_taskready(g);
	return g;
}

int taskcreate(void (*fn)(void*), void *arg, uint stack) {
	struct G *g;
	g = _taskcreate(_thread(), fn, arg, stack);
	return g->id;
}

void _scheduler(struct M *m) {
	struct G *g;
	
    _threadbindm(m);
	pthread_mutex_lock(&m->lock);
	for(;;) {
		while((g = m->runqueue.head) == NULL) {
            if (m->nthread == 0) {
                goto Out;
            }
			if((g = m->idlequeue.head) != NULL) {
				// while((t = p->idlequeue.head) != NULL){
					_deltask(&m->idlequeue, g);
					addtask(&m->runqueue, g);
				// }
				continue;
			}
			// p->runrend.l = &p->lock;
			// _threaddebug("scheduler sleep");
			// _procsleep(&p->runrend);
			// _threaddebug("scheduler wake");
		}

		// assert(p->pinthread == NULL || p->pinthread == t);
		_deltask(&m->runqueue, g);
		pthread_mutex_unlock(&m->lock);
		m->g = g;
		m->nswitch++;

		contextswitch(&m->schedcontext, &g->context);

		m->g = NULL;
		pthread_mutex_lock(&m->lock);
		if(g->exiting){
			delGinM(m, g);
			m->nthread--;
			free(g);
		}
	}
Out:
    _delthread(m);
    pthread_mutex_lock(&_sched.threadnproclock);
    if(m->sysproc)
        --_sched.threadnsysproc;
    if(--_sched.threadnproc == _sched.threadnsysproc)
        exit(0);
    pthread_mutex_unlock(&_sched.threadnproclock);
    pthread_mutex_unlock(&m->lock);
    _threadbindm(NULL);
    free(m);
}