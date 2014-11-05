#include "lib9.h"
#include "thread.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void timeproc(void *arg) {
	int fd;
	int len, ret;
	long current_time;
	char *str;
	
	printf("accept a connection...\n");
	current_time = time(NULL);
	if (current_time == (time_t)-1) {
		fprintf(stderr, "failture to get current time");
		goto close;
	}
	str = ctime(current_time);
	
	fd = *((int*)arg);	
	len  = strlen(str);
	ret = fdwrite(fd, &str, len);
	if (ret != len) {
		fprintf(stderr, "write error");
		goto close;
	}
	printf("send current time:%s\n", str);
close:
	close(fd);
	return;
}

void threadmain(int argc, char* arg[]) {
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
		tid = threadcreate(timeproc, (void*)&client, 4<<10);
		printf("create a new thread %d\n", tid);
	}
exit:
	return;
}