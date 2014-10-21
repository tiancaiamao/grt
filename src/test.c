#include <stdio.h>
#include "grt.h"

void test(void *arg) {
	struct C* c;
	
	c = (struct C*)arg;
	printf("run here in test\n");
	chansend(c, NULL);
}

void taskmain(int argc, char *argv[]) {
	struct C*	c;
		
	printf("in main\n");
	c = chancreate(0, 0);
	taskcreate(test, c, 8<<10);
	chanrecv(c, NULL);
	printf("return to main\n");
	chanfree(c);
	return;
}