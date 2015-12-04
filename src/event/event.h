#ifndef _EVENT_
#define _EVENT_

struct event;

struct event* event_create();
void event_loop(struct event*);

#endif
