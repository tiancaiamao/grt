#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>

#include "impl.h"

void grtstate(char *fmt, ...) {
    va_list arg;
    struct G *t;
    struct M *m;

    m = _thread();
    t = m->g;
    va_start(arg, fmt);
    vsnprintf(t->state, sizeof t->name, fmt, arg);
    va_end(arg);
}

void _delgrt(struct Glist *l, struct G *g) {
	if(g->prev)
		g->prev->next = g->next;
	else
		l->head = g->next;
	if(g->next)
		g->next->prev = g->prev;
	else
		l->tail = g->prev;
	l->length--;
}

void _addgrt(struct Glist *l, struct G *g)
{
	if(l->tail){
		l->tail->next = g;
		g->prev = l->tail;
		l->length++;
	}else{
		l->head = g;
		g->prev = NULL;
		l->length = 1;
	}
	l->tail = g;
	g->next = NULL;
}

// debugging
void grtname(char *fmt, ...) {
	va_list arg;
	struct G *t;

	t = _thread()->g;
	va_start(arg, fmt);
	vsnprintf(t->name, sizeof t->name, fmt, arg);
	va_end(arg);
}

void _grtready(struct G *g)
{
	struct M *m;
    
    m = g->m;
	g->status = G_READY;
    _addgrt(&m->runqueue, g);
}

static void grtexits(char *msg) {
    struct M *m;
    struct G *g;
    
    m = _thread();
    g = m->g;
	g->status = G_DEAD;
    _contextswitch(&g->context, &m->schedcontext);
}

void _needstack(int n) {
    struct G *g;
    
    g = _thread()->g;
    
    if((char*)&g <= (char*)g->stk
       || (char*)&g - (char*)g->stk < 256+n){
        printf( "grt stack overflow: &t=%p tstk=%p n=%d\n", &g, g->stk, 256+n);
        abort();
    }
}

void _grtswitch(void) {
    struct M *m;
    struct G *g;
    
    _needstack(0);
    m = _thread();
    g = m->g;
    _contextswitch(&g->context, &m->schedcontext);
}

static void grtstart(uint y, uint x) {
	struct G *g;
	ulong z;

	z = (ulong)x << 16;
	z <<= 16;
	z |= y;
	g = (struct G*)z;
	g->startfn(g->startarg);
	grtexits(NULL);
}

static struct G* _grtalloc(void (*fn)(void*), void *arg, uint stack) {
	struct G *g;
	sigset_t zero;
	uint x, y;
	ulong z;

	/* allocate the grt and stack together */
	g = malloc(sizeof(struct Glist)+stack);
	memset(g, 0, sizeof(struct G));
	g->stk = (uchar*)(g+1);
	g->stksize = stack;
	// g->id = ++grtidgen;
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
	makecontext(&g->context.uc, (void(*)())grtstart, 2, y, x);

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

struct G* _grtcreate(struct M* m, void (*fn)(void*), void *arg, uint stack) {
	struct G *g;
	
	g = _grtalloc(fn, arg, stack);
	g->m = m;
	addGtoM(m, g);
	_grtready(g);
	return g;
}

int grtcreate(void (*fn)(void*), void *arg, uint stack) {
	struct G *g;
	g = _grtcreate(_thread(), fn, arg, stack);
	return g->id;
}
