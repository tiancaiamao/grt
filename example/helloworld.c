#include "lib9.h"
#include <stdio.h>

void threadmain(void* args) {
	char *c = malloc(100);
	printf("hello world!\n");
	free(c);
	return;
}