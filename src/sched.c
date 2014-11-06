#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
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
	_threadbindm(m);
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
	struct M *tmp;

    pthread_mutex_lock(&m->lock);
    for(;;) {
        while((g = m->runqueue.head) == NULL) {
			// steal from other M
			struct M *find = NULL;
			int max = 0;
			for (tmp = _sched.threadprocs; tmp != _sched.threadprocstail; tmp = tmp->next) {
				pthread_mutex_lock(&tmp->lock);
				if (tmp->runqueue.length > max) {
					max = tmp->runqueue.length;
					find = tmp;
				}
				pthread_mutex_unlock(&tmp->lock);
			}
			
			if (find && max >= 4) {
				// steal one half
				pthread_mutex_lock(&find->lock);
				for (int i=0; i<max/2; i++) {
					g = find->runqueue.head;
					_deltask(&find->runqueue, g);
					_addtask(&m->runqueue, g);
				}
				pthread_mutex_unlock(&find->lock);
				continue;
			} 
			if (max == 0 && m->nsleep == 5) {
				goto Out;
			}
			// nothing to do, have a rest
			m->nsleep++;
			sleep(1);
        }

		m->nsleep = 0;
        _deltask(&m->runqueue, g);
        pthread_mutex_unlock(&m->lock);
        m->g = g;
        m->nswitch++;

        _contextswitch(&m->schedcontext, &g->context);

        m->g = NULL;
        pthread_mutex_lock(&m->lock);
        if(g->status == G_DEAD){
            delGinM(m, g);
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
