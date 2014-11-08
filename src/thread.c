#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "impl.h"

static pthread_key_t prockey;
void _threadinit(void) {
    pthread_key_create(&prockey, NULL);
}

static void null(void *arg) {}

int maxprocs(int num) {
	int ret;
	struct M* m;
	
	ret = _sched.threadnproc;
	if (ret < num) {
		int i;
		for (i=0; i<num-ret; i++) {
			_threadcreate(null, NULL, 4<<10);
		}
	}
    return ret;
}

void _threadsleep(struct Cond *r) {
    int ret;

    pthread_mutex_lock(&r->l);
    pthread_cond_init(&r->cond, NULL);
    r->asleep = 1;
    ret = pthread_cond_wait(&r->cond, &r->l);
    if (ret != 0) {
        abort();
    }
    pthread_cond_destroy(&r->cond);
    r->asleep = 0;
}

void _procwakeup(struct Cond *r) {
    if(r->asleep){
        r->asleep = 0;
        pthread_cond_signal(&r->cond);
    }
}

struct M* _thread() {
	struct M* m;
    m = pthread_getspecific(prockey);
	return m;
};

void _threadbindm(struct M* m) {
    if (pthread_setspecific(prockey, m) != 0)
        abort();
}

void _delthread(struct M *m) {
	pthread_mutex_lock(&_sched.threadprocslock);
	if(m->prev)
		m->prev->next = m->next;
	else
		_sched.threadprocs = m->next;
	if(m->next)
		m->next->prev = m->prev;
	else
		_sched.threadprocstail = m->prev;
	pthread_mutex_unlock(&_sched.threadprocslock);
}

static void _addthread(struct M *m) {
	pthread_mutex_lock(&_sched.threadprocslock);
	if(_sched.threadprocstail){
		_sched.threadprocstail->next = m;
		m->prev = _sched.threadprocstail;
	}else{
		_sched.threadprocs = m;
		m->prev = NULL;
	}
	_sched.threadprocstail = m;
	m->next = NULL;
	pthread_mutex_unlock(&_sched.threadprocslock);
}

static void startprocfn(void *v) {
    void **a;
    uchar *stk;
    void (*fn)(void*);
    struct M *p;
    int pid0, pid1;
    
    a = (void**)v;
    fn = a[0];
    p = a[1];
    stk = a[2];
    pid0 = (int)a[4];
    pid1 = getpid();
    free(a);
    p->osprocid = pthread_self();
    pthread_detach(p->osprocid);
	_threadbindm(p);
    
    (*fn)(p);
    
    // delayfreestack(stk, pid0, pid1);
    _exit(0);
}

void _threadstart(struct M *m, void (*fn)(struct M*)) {
    void **a;
    
    a = malloc(2*sizeof a[0]);
    a[0] = (void*)fn;
    a[1] = m;
    
    if(pthread_create(&m->osprocid, NULL, (void*(*)(void*))startprocfn, (void*)a) < 0){
        abort();
    }
}

struct M* _threadalloc(void) {
	struct M *m;

	m = malloc(sizeof(*m));
	memset(m, 0, sizeof(*m));
	_addthread(m);
	pthread_mutex_lock(&_sched.threadnproclock);
	_sched.threadnproc++;
	pthread_mutex_unlock(&_sched.threadnproclock);
	return m;
}

void _threadcreate(void (*fn)(void*), void *arg, uint stack) {
	struct G *g;
	struct M *m;

	m = _threadalloc();
	if (fn != NULL) {
		g = _grtcreate(m, fn, arg, stack);		
	}

	_threadstart(m, _scheduler);
	return;
}
