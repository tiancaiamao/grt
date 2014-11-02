#include <pthread.h>
#include <stdlib.h>
#include "impl.h"

static void _schedinit() {
}

static int taskargc;
static char **taskargv;
extern void taskmain(int argc, char *argv[]);
static void taskmainstart(void *v) {
	taskmain(taskargc, taskargv);
}

int main(int argc, char **argv) {
	struct M *m;
	
	taskargc = argc;
	taskargv = argv;	

	_threadinit();
	_schedinit();
	_sysmon();
    
    m = _threadalloc();
    _taskcreate(m, taskmainstart, NULL, 8<<10);
	_scheduler(m);
    
	// never reach here
	abort();
	return 0;
}

void _contextswitch(Context *from, Context *to) {
    if(swapcontext(&from->uc, &to->uc) < 0){
        abort();
    }
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

void _scheduler(struct M *m) {
    struct G *g;

    _threadbindm(m);
    pthread_mutex_lock(&m->lock);
    for(;;) {
        while((g = m->runqueue.head) == NULL) {
            if (m->numg == 0) {
                    goto Out;
            }
            if((g = m->idlequeue.head) != NULL) {
                 while((g = m->idlequeue.head) != NULL){
                    _deltask(&m->idlequeue, g);
                    _addtask(&m->runqueue, g);
                 }
                continue;
            }
            // _threaddebug("scheduler sleep");
             _threadsleep(&m->cond);
            // _threaddebug("scheduler wake");
        }

        _deltask(&m->runqueue, g);
        pthread_mutex_unlock(&m->lock);
        m->g = g;
        m->nswitch++;

        _contextswitch(&m->schedcontext, &g->context);

        m->g = NULL;
        pthread_mutex_lock(&m->lock);
        if(g->exiting){
            delGinM(m, g);
            m->numg--;
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
