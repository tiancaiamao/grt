#include "grt.h"
#include <stdio.h>
#include <stdlib.h>

void grtmain(int argc, char *argv[]) {
	char *c = malloc(100);
	printf("hello world!\n");
	free(c);
	return;
}