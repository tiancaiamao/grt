#ifndef _POLLER_
#define _POLLER_


enum {
  POLL_READ = 1,
  POLL_WRITE = 2,
};

#include <stdbool.h>

struct poll_event {
  void *ud;
  bool read;
  bool write;
};

int poll_add(struct poller *po, int fd, struct poll_event *ev);
void poll_del(struct poller *po, int fd);
int poll_wait(struct poller *po, struct poll_event *ev, int max, int timeout);
struct poller* poll_create();
void poll_release();

#endif
