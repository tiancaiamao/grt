#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "impl.h"

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
		_fdwait(fd, 'r');
	return m;
}

ssize_t fdwrite(int fd, void *buf, int n) {
	ssize_t m, tot;
	
	for(tot=0; tot<n; tot+=m){
		while((m=write(fd, (char*)buf+tot, n-tot)) < 0 && errno == EAGAIN)
			_fdwait(fd, 'w');
		if(m < 0)
			return m;
		if(m == 0)
			break;
	}
	return tot;
}

int fdnoblock(int fd) {
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK);
}

// static unsigned long nsec(void) {
// 	struct timeval tv;
//
// 	if(gettimeofday(&tv, 0) < 0)
// 		return -1;
// 	return (unsigned long)tv.tv_sec*1000*1000*1000 + tv.tv_usec*1000;
// }

