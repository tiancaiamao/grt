#ifndef _POLLER_
#define _POLLER_

#include <stdbool.h>

struct event {
  void *ud;
  bool read;
  bool write;
};

int poll_add(struct poller *po, int fd, struct event *ev);
void poll_del(struct poller *po, int fd);
int poll_wait(struct poller *po, struct event *ev, int max, int timeout);
struct poller* poll_create();
void poll_release();

#endif
