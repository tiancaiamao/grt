/* Linux epoll(2) based ae.c module
 *
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "impl.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

typedef struct aeApiState {
    int epfd;
    struct epoll_event *events;
	int setsize;
} aeApiState;

static int fdmasks[15000];
static aeApiState state;
static int startedfdthread;
static int npoll;

static int aeApiCreate(aeApiState *state) {
    state->events = malloc(sizeof(struct epoll_event)*state->setsize);
    if (!state->events) {
        return -1;
    }
    state->epfd = epoll_create(1024); /* 1024 is just a hint for the kernel */
    if (state->epfd == -1) {
        free(state->events);
        return -1;
    }
    return 0;
}

static int aeApiResize(aeApiState *state, int setsize) {
    state->events = realloc(state->events, sizeof(struct epoll_event)*setsize);
	state->setsize = setsize;
    return 0;
}

static void aeApiFree(aeApiState *state) {
    close(state->epfd);
    free(state->events);    
}

static int aeApiAddEvent(aeApiState *state, int fd, int mask, struct G *g) {
    struct epoll_event ee;
	int op;

    if (mask == 'r') ee.events |= EPOLLIN;
    if (mask == 'w') ee.events |= EPOLLOUT;
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an ADD operation. */
    op = fdmasks[fd] == 0 ?
            EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    ee.events = 0;
    mask |= fdmasks[fd]; /* Merge old events */
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = fd;
	ee.data.ptr = g;
    if (epoll_ctl(state->epfd,op,fd,&ee) == -1) return -1;
    return 0;
}

static void aeApiDelEvent(aeApiState *state, int fd, int delmask) {
    struct epoll_event ee;
    int mask = fdmasks[fd] & (~delmask);

    ee.events = 0;
    if (mask & EPOLLIN) ee.events |= EPOLLIN;
    if (mask & EPOLLOUT) ee.events |= EPOLLOUT;
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = fd;
    if (mask != 0) {
        epoll_ctl(state->epfd,EPOLL_CTL_MOD,fd,&ee);
    } else {
        /* Note, Kernel < 2.6.9 requires a non null event pointer even for
         * EPOLL_CTL_DEL. */
        epoll_ctl(state->epfd,EPOLL_CTL_DEL,fd,&ee);
    }
}

static void fdthread(struct M* m) {
	int n, retval;
	
	for (;;) {
	    retval = epoll_wait(state.epfd,state.events, npoll, -1);
	    if (retval > 0) {
	        int j;

	        for (j = 0; j < retval; j++) {
	            int mask = 0;
	            struct epoll_event *e = state.events+j;
				
				_grtready(e->data.ptr);
				aeApiDelEvent(&state, e->data.fd, e->events);

	            // if (e->events & EPOLLIN) mask |= AE_READABLE;
	            // if (e->events & EPOLLOUT) mask |= AE_WRITABLE;
	            // if (e->events & EPOLLERR) mask |= AE_WRITABLE;
	            // if (e->events & EPOLLHUP) mask |= AE_WRITABLE;
	            // state->fired[j].fd = e->data.fd;
	            // state->fired[j].mask = mask;
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
		if (aeApiCreate(&state) < 0) {
			fprintf(stderr, "epoll init failed");
			abort();
		}
	} 
	
	if(npoll >= state.setsize) {
		aeApiResize(&state, state.setsize+1024);
	}

	aeApiAddEvent(&state, fd, rw, g);
	fdmasks[fd] = EPOLL_CTL_ADD;
	npoll++;
	
	if (!startedfdthread) {
		_threadstart(m, fdthread);
		startedfdthread = 1;
	}
	
    _grtblock(g);
    _grtstate("fdwait for %s", rw=='r' ? "read" : rw=='w' ? "write" : "error");
	_grtswitch();
}