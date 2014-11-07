#include "grt.h"
#include <unistd.h>
#include <stdio.h>

void grtmain(int argc, char* argv[]) {
	int fd;
	int sz;
	char buf[200];
	
	fd = netdial(TCP, "localhost", 9998);
	if (fd < 0) {
		goto exit;
	}
	sz = fdread(fd, buf, 200);
	if (sz < 0) {
		goto error;
	}
	buf[sz] = '\0';
	printf("%s", buf);
error:
	close(fd);
exit:
	return;
}