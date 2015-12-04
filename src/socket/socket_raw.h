#ifndef _SOCKET_RAW_
#define _SOCKET_RAW_

int socket_raw_connect(const char* host, int port);
int socket_raw_noblock(int fd);
int socket_raw_listen(int port);
void socket_raw_close(int fd);

#endif
