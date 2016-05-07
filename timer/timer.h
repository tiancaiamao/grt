#ifndef _TIMER_H
#define _TIMER_H

struct timer;

struct timer_event {
  unsigned int when;
  unsigned int period;
  void (*callback) (void *data);
  void *data;

  int i; // heap index
};

void timer_init(struct timer *t);
void timer_release(struct timer *t);

void timer_add(struct timer *t, struct timer_event *ev);
void timer_del(struct timer *t, struct timer_event *ev);

void timer_update(struct timer *t, unsigned int now);
struct timer_event* timer_latest(struct timer *t);

#endif
