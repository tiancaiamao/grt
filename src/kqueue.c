#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "impl.h"

static struct kevent *clist;
static struct kevent *elist;
static int kq;
static int POLLSIZE;
static int npollfd;
static int startedfdthread;

static void fdthread(struct M* m) {
	int n;
	
	for (;;) {
		n = kevent(kq, clist, npollfd, elist, npollfd, NULL);
		if (n < 0) {
			perror("kevent failed");
			return;
		} 
		
		for (int i=0; i<n; i++) {
	        if (elist[i].flags & EV_ERROR) {
	           fprintf(stderr, "EV_ERROR: %s\n", strerror(elist[i].data));
			   close(elist[i].ident);
			   continue;
	        }
			
			_grtready(elist[i].udata);
		}
	}
}


void _fdwait(int fd, int rw) {
	struct M *m;
	struct G *g;
	int bits;
	
	m = _thread();
	g = m->g;

	bits = 0;
	switch(rw){
	case 'r':
		bits |= EVFILT_READ;
		break;
	case 'w':
		bits |= EVFILT_WRITE;
		break;
	}
	
	if (!startedfdthread) {
		m = _threadalloc();
		m->sysproc = 1; // TODO thread safe
		pthread_mutex_lock(&_sched.threadnproclock);
	    _sched.threadnsysproc++;
		pthread_mutex_unlock(&_sched.threadnproclock);

		POLLSIZE = 1024;
		clist = malloc(POLLSIZE * sizeof(struct kevent));
		elist = malloc(POLLSIZE * sizeof(struct kevent));

		kq = kqueue();
		if (kq < 0) {
			perror("kqueue");
			abort();
		}
		
	    EV_SET(&clist[npollfd], fd, bits, EV_ADD|EV_ENABLE, 0, 0, g);
	    EV_SET(&elist[npollfd], fd, bits, EV_ADD|EV_ENABLE, 0, 0, g);
		npollfd = 1;

		_threadstart(m, fdthread);
		startedfdthread = 1;
	} else {
		int succ;
		if(npollfd >= POLLSIZE) {		
			POLLSIZE += 1024;
			clist = realloc(clist, POLLSIZE * sizeof(struct kevent));
			elist = realloc(elist, POLLSIZE * sizeof(struct kevent));
			if (clist == NULL && elist == NULL) {
				fprintf(stderr, "out of memory");
				abort();
			}
		}

	    EV_SET(&clist[npollfd], fd, bits, EV_ADD|EV_ENABLE, 0, 0, g);
		succ = kevent(kq, &clist[npollfd], 1, NULL, 0, NULL);
		if (succ < 0) {
			// TODO
			abort();
		}
		npollfd++;
	}
	
    _grtblock(g);
    _grtstate("fdwait for %s", rw=='r' ? "read" : rw=='w' ? "write" : "error");
	_grtswitch();
}