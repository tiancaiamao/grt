#include "event.h"

struct event {
  struct poller *poller;
  struct poll_event *events;
  int num;
};

void
event_add(struct event* e, struct handle* h, bool read, bool write) {
  struct poll_event ev;
  ev.read = read;
  ev.write = write;
  ev.ud = h;
  poll_add(e->poller, handle->fd, &ev);
}

void
event_loop(struct event* e) {
  for (;;) {
    int n = poll_wait(e->poller, e->events, e->num, 0);
    int i;
    for (i=0; i<n; i++) {
      struct handle* h = e->events[i].ud;
      h->event_callback(h);
    }
  }
}
