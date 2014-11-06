#include "grt.h"
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

void grtmain(int argc, char *argv[]) {
	Channel* c;
	
	c = chancreate(sizeof(int), 0);
	
	for (int i=0; i<3; i++) {
		grtcreate(worker, c, 4<<10);
		
	}

	for (int i=0; i<10; i++) {
		chansend(c, (void*)&i);
	}
	
	chanfree(c);
	return;
}