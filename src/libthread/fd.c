#include "threadimpl.h"
#include <sys/poll.h>
#include <fcntl.h>

enum
{
	MAXFD = 1024
};

static struct pollfd pollfd[MAXFD];
static _Thread *polltask[MAXFD];
static int npollfd;
static int startedfdtask;
static _Threadlist sleeping;
static int sleepingcounted;

static void
delthread(_Threadlist *l, _Thread *t)
{
	if(t->prev)
		t->prev->next = t->next;
	else
		l->head = t->next;
	if(t->next)
		t->next->prev = t->prev;
	else
		l->tail = t->prev;
}

void
fdtask(void *v)
{
	int i, ms;
	_Thread *t;
	uvlong now;
	
	// tasksystem();
	threadsetname("fdtask");
	for(;;){
		/* let everyone else run */
		while(threadyield() > 0)
			;
		/* we're the only one runnable - poll for i/o */
		errno = 0;
		threadsetstate("poll");
		if((t=sleeping.head) == nil)
			ms = -1;
		else{
			/* sleep at most 5s */
			now = nsec();
			if(now >= t->alarmtime)
				ms = 0;
			else if(now+5*1000*1000*1000LL >= t->alarmtime)
				ms = (t->alarmtime - now)/1000000;
			else
				ms = 5000;
		}
		if(poll(pollfd, npollfd, ms) < 0){
			if(errno == EINTR)
				continue;
			fprint(2, "poll: %s\n", strerror(errno));
			threadexitsall(0);
		}

		/* wake up the guys who deserve it */
		for(i=0; i<npollfd; i++){
			while(i < npollfd && pollfd[i].revents){
				_threadready(polltask[i]);
				--npollfd;
				pollfd[i] = pollfd[npollfd];
				polltask[i] = polltask[npollfd];
			}
		}
		
		now = nsec();
		while((t=sleeping.head) && now >= t->alarmtime){
			delthread(&sleeping, t);
			// if(!t->system && --sleepingcounted == 0)
				// taskcount--;
			_threadready(t);
		}
	}
}

// uint
// taskdelay(uint ms)
// {
//	 uvlong when, now;
//	 _Thread *t;
//
//	 if(!startedfdtask){
//		 startedfdtask = 1;
//		 taskcreate(fdtask, 0, 32768);
//	 }
//
//	 now = nsec();
//	 when = now+(uvlong)ms*1000000;
//	 for(t=sleeping.head; t!=nil && t->alarmtime < when; t=t->next)
//		 ;
//
//	 if(t){
//		 taskrunning->prev = t->prev;
//		 taskrunning->next = t;
//	 }else{
//		 taskrunning->prev = sleeping.tail;
//		 taskrunning->next = nil;
//	 }
//
//	 t = taskrunning;
//	 t->alarmtime = when;
//	 if(t->prev)
//		 t->prev->next = t;
//	 else
//		 sleeping.head = t;
//	 if(t->next)
//		 t->next->prev = t;
//	 else
//		 sleeping.tail = t;
//
//	 if(!t->system && sleepingcounted++ == 0)
//		 taskcount++;
//	 _threadswitch();
//
//	 return (nsec() - now)/1000000;
// }

void
fdwait(int fd, int rw)
{
	_Thread* taskrunning;
	int bits;

	if(!startedfdtask){
		startedfdtask = 1;
		threadcreate(fdtask, 0, 32768);
	}

	if(npollfd >= MAXFD){
		fprint(2, "too many poll file descriptors\n");
		abort();
	}
	
	threadsetstate("fdwait for %s", rw=='r' ? "read" : rw=='w' ? "write" : "error");
	bits = 0;
	switch(rw){
	case 'r':
		bits |= POLLIN;
		break;
	case 'w':
		bits |= POLLOUT;
		break;
	}
	
	taskrunning = proc()->thread;
	polltask[npollfd] = taskrunning;
	pollfd[npollfd].fd = fd;
	pollfd[npollfd].events = bits;
	pollfd[npollfd].revents = 0;
	npollfd++;
	_threadswitch();
}

/* Like fdread but always calls fdwait before reading. */
int
fdread1(int fd, void *buf, int n)
{
	int m;
	
	do
		fdwait(fd, 'r');
	while((m = read(fd, buf, n)) < 0 && errno == EAGAIN);
	return m;
}

int
fdread(int fd, void *buf, int n)
{
	int m;
	
	while((m=read(fd, buf, n)) < 0 && errno == EAGAIN)
		fdwait(fd, 'r');
	return m;
}

int
fdwrite(int fd, void *buf, int n)
{
	int m, tot;
	
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