#include "grt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void helloproc(void *arg) {
	int fd;
	int len, ret;
	char *str = "hello world!\n";
	
	printf("accept a connection...\n");
	
	fd = *((int*)arg);	
	len  = strlen(str);
	ret = fdwrite(fd, str, len);
	if (ret != len) {
		fprintf(stderr, "write error");
		goto close;
	}
	printf("send success:%s\n", str);
close:
	close(fd);
	return;
}

void grtmain(int argc, char* arg[]) {
	int fd;
	int client;
	int tid;
	
	fd = netannounce(TCP, "localhost", 9998);
	if (fd < 0) {
		goto exit;
	}
	
	for (;;) {		
		client = netaccept(fd, NULL, NULL);
		if (client < 0) {
			continue;
		}
		tid = grtcreate(helloproc, (void*)&client, 4<<10);
		printf("create a new grt %d\n", tid);
	}
exit:
	return;
}