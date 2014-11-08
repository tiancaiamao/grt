#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include "impl.h"

typedef struct aeApiState {
    int kqfd;
    struct kevent *events;
	int setsize;
} aeApiState;

static aeApiState state;
static int npoll;
static int startedfdthread;

static int aeApiCreate(aeApiState *state, int size) {
    if (state == NULL) return -1;
    state->events = malloc(sizeof(struct kevent)*size);
    if (!state->events) {
        return -1;
    }
	state->setsize = size;
    state->kqfd = kqueue();
    if (state->kqfd == -1) {
        free(state->events);
        return -1;
    }
    return 0;
}

static int aeApiResize(aeApiState *state, int setsize) {
    state->events = realloc(state->events, sizeof(struct kevent)*setsize);
	state->setsize = setsize;
    return 0;
}

static void aeApiFree(aeApiState *state) {
    close(state->kqfd);
    free(state->events);
}

static int aeApiAddEvent(aeApiState *state, int fd, char rw, struct G *g) {
    struct kevent ke;

    if (rw == 'r') {
        EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, g);
        if (kevent(state->kqfd, &ke, 1, NULL, 0, NULL) == -1) return -1;
    }
    if (rw == 'w') {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, g);
        if (kevent(state->kqfd, &ke, 1, NULL, 0, NULL) == -1) return -1;
    }
    return 0;
}

static void aeApiDelEvent(aeApiState *state, int fd, char rw) {
    struct kevent ke;

    if (rw == 'r') {
        EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        kevent(state->kqfd, &ke, 1, NULL, 0, NULL);
    }
    if (rw == 'w') {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        kevent(state->kqfd, &ke, 1, NULL, 0, NULL);
    }
}

static void fdthread(struct M* m) {
	int n, retval;
	
	for (;;) {
        retval = kevent(state.kqfd, NULL, 0, state.events, state.setsize, NULL);
	    if (retval > 0) {
	        int i;

	        for(i = 0; i < retval; i++) {
				char c;
	            struct kevent *e = state.events+i;
				
		        if (e->flags & EV_ERROR) {
		           fprintf(stderr, "EV_ERROR: %s\n", strerror(e->data));
				   close(e->ident);
				   continue;
		        } 
		        
				_grtready(e->udata);
				switch (e->filter) {
					case EVFILT_READ:
						c = 'r';
						break;
					case EVFILT_WRITE:
						c = 'w';
						break;
				}
				aeApiDelEvent(&state, e->ident, c);
				
	            // if (e->filter == EVFILT_READ) mask |= AE_READABLE;
	            // if (e->filter == EVFILT_WRITE) mask |= AE_WRITABLE;
	            // state->fired[i].fd = e->ident;
	            // state->fired[i].mask = mask;
	        }
	    }
	}
}


void _fdwait(int fd, int rw) {
	struct M *m;
	struct G *g;
	int bits;
	
	m = _thread();
	g = m->g;
	
	if (!startedfdthread) {
		m = _threadalloc();
		m->sysproc = 1; // TODO thread safe
		pthread_mutex_lock(&_sched.threadnproclock);
	    _sched.threadnsysproc++;
		pthread_mutex_unlock(&_sched.threadnproclock);
		
		npoll = 0;
		if (aeApiCreate(&state, 15000) < 0) {
			fprintf(stderr, "kqueue init failed");
			abort();
		}
	} 
	
	if(npoll >= state.setsize) {
		aeApiResize(&state, state.setsize+1024);
	}

	aeApiAddEvent(&state, fd, rw, g);
	npoll++;
	
	if (!startedfdthread) {
		_threadstart(m, fdthread);
		startedfdthread = 1;
	}
	
    _grtblock(g);
    _grtstate("fdwait for %s", rw=='r' ? "read" : rw=='w' ? "write" : "error");
	_grtswitch();
}