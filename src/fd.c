#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "impl.h"

enum
{
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
		now = nsec();
		if(now >= alarmtime)
			ms = 0;
		else if(now+5*1000*1000*1000LL >= alarmtime)
			ms = (int)(alarmtime - now)/1000000;
		else
			ms = 5000;

		if(poll(pollfd, npollfd, ms) < 0){
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
	}
}

// uint grtdelay(uint ms) {
// 	unsigned long  when, now;
// 	struct G *t;
//
// 	if(!startedfdgrt){
// 		startedfdgrt = 1;
// 		grtcreate(fdgrt, 0, 32768);
// 	}
//
// 	now = nsec();
// 	when = now+(uvlong)ms*1000000;
// 	for(t=sleeping.head; t!=NULL && t->alarmtime < when; t=t->next)
// 		;
//
// 	if(t){
// 		grtrunning->prev = t->prev;
// 		grtrunning->next = t;
// 	}else{
// 		grtrunning->prev = sleeping.tail;
// 		grtrunning->next = nil;
// 	}
//
// 	t = grtrunning;
// 	t->alarmtime = when;
// 	if(t->prev)
// 		t->prev->next = t;
// 	else
// 		sleeping.head = t;
// 	if(t->next)
// 		t->next->prev = t;
// 	else
// 		sleeping.tail = t;
//
// 	if(!t->system && sleepingcounted++ == 0)
// 		grtcount++;
// 	grtswitch();
//
// 	return (nsec() - now)/1000000;
// }

void fdwait(int fd, int rw) {
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

///* Like fdread but always calls fdwait before reading. */
//int fdread1(int fd, void *buf, int n) {
//	ssize_t m;
//	
//	do
//		fdwait(fd, 'r');
//	while((m = read(fd, buf, n)) < 0 && errno == EAGAIN);
//	return m;
//}

ssize_t fdread(int fd, void *buf, int n) {
	ssize_t m;
	
	while((m=read(fd, buf, n)) < 0 && errno == EAGAIN)
		fdwait(fd, 'r');
	return m;
}

ssize_t fdwrite(int fd, void *buf, int n) {
	ssize_t m, tot;
	
	for(tot=0; tot<n; tot+=m){
		while((m=write(fd, (char*)buf+tot, n-tot)) < 0 && errno == EAGAIN)
			fdwait(fd, 'w');
		if(m < 0)
			return m;
		if(m == 0)
			break;
	}
	return tot;
}

int
fdnoblock(int fd)
{
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK);
}

static unsigned long nsec(void)
{
	struct timeval tv;

	if(gettimeofday(&tv, 0) < 0)
		return -1;
	return (unsigned long)tv.tv_sec*1000*1000*1000 + tv.tv_usec*1000;
}

