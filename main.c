#include <pthread.h>
#include <stdlib.h>
#include "impl.h"

static void _schedinit() {
	int maxprocs;
	char * env;
	int i;

	env = getenv("GOMAXPROCS");
	if (env != NULL) {
		maxprocs = atoi(env);	
		for (i=0; i<maxprocs; i++) {
			_threadcreate(NULL, _scheduler, STACK);
		}	
	}
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