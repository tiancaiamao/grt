#ifndef _EVENT_
#define _EVENT_

struct event;

// 可以往event里面加入handle
// connector/acceptor/connection这些都是继承自handle的
struct handle {
  int fd;
  void (*event_callback) (void *ud);
};


struct event* event_create();
void event_add(struct event* e, struct handle* h, bool read, bool write);
void event_del(struct event* e, struct handle* h);
void event_loop(struct event*);

#endif
