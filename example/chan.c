#include "lib9.h"
#include "thread.h"
#include <stdio.h>

void worker(void *arg) {
	Channel *c;
	int tmp;
	
	c = (Channel*)arg;
	for (;;) {
		chanrecv(c, (void*)&tmp);
		printf("worker recv %d\n", tmp);
	}
}

void threadmain(int argc, char *argv[]) {
	Channel* c;
	
	c = chancreate(sizeof(int), 0);
	
	for (int i=0; i<3; i++) {
		threadcreate(worker, c, 4<<10);
		
	}

	for (int i=0; i<10; i++) {
		chansend(c, (void*)&i);
	}
	
	chanfree(c);
	exit(0);
}