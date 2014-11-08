#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "impl.h"

enum {
	MAXFD = 1024
};

static struct pollfd pollfd[MAXFD];
static struct G *pollgrt[MAXFD];
static int npollfd;
static int startedfdthread;
static struct Glist sleeping;
//static int sleepingcounted;
static unsigned long nsec(void);
static long alarmtime;

static void fdthread(struct M* m) {
	int i, ms;
	unsigned long now;

	for(;;){
		/* sleep at most 5s */
		// now = nsec();
		// if(now >= alarmtime)
		// 	ms = 0;
		// else if(now+5*1000*1000*1000LL >= alarmtime)
		// 	ms = (int)(alarmtime - now)/1000000;
		// else
		// ms = 5000;

		if(poll(pollfd, npollfd, -1) < 0) {
			if(errno == EINTR)
				continue;
			fprintf(stderr, "poll: %s\n", strerror(errno));
			exit(0);
		}

		/* wake up the guys who deserve it */
		for(i=0; i<npollfd; i++){
			while(i < npollfd && pollfd[i].revents) {
				_grtready(pollgrt[i]);
				--npollfd;
				pollfd[i] = pollfd[npollfd];
				pollgrt[i] = pollgrt[npollfd];
			}
		}
		
		// alarmtime = now+5*1000*1000*1000LL;
	}
}


void _fdwait(int fd, int rw) {
	struct M *m;
	struct G *g;
	int bits;
	
	if(!startedfdthread){
		m = _threadalloc();
		m->sysproc = 1; // TODO thread safe
		pthread_mutex_lock(&_sched.threadnproclock);
	    _sched.threadnsysproc++;
		pthread_mutex_unlock(&_sched.threadnproclock);
		_threadstart(m, fdthread);		
		startedfdthread = 1;
	}

	if(npollfd >= MAXFD){
		printf("too many poll file descriptors\n");
		abort();
	}
	
	m = _thread();
	g = m->g;
    _grtblock(g);
    _grtstate("fdwait for %s", rw=='r' ? "read" : rw=='w' ? "write" : "error");
	bits = 0;
	switch(rw){
	case 'r':
		bits |= POLLIN;
		break;
	case 'w':
		bits |= POLLOUT;
		break;
	}

	pollgrt[npollfd] = _thread()->g;
	pollfd[npollfd].fd = fd;
	pollfd[npollfd].events = bits;
	pollfd[npollfd].revents = 0;
	npollfd++;
	_grtswitch();
}