#include "timer.h"
#include <stdlib.h>

struct timer {
  struct timer_event** heap;
  int size;
  int cap;
};

void
timer_init(struct timer *t) {
  t->size = 0;
  t->cap = 32;
  t->heap = calloc(t->cap, sizeof(struct timer_event*));
}

void
timer_release(struct timer *t) {
  free(t->heap);
  t->size = 0;
  t->cap = 0;
  t->heap = NULL;
}

static void
timer_siftup(struct timer *t, int i) {
  struct timer_event **heap = t->heap;
  struct timer_event *tmp = heap[i];
  unsigned int when = tmp->when;
  while (i>0) {
    int p = (i-1) / 4; // parent
    if (when >= heap[p]->when) {
      break;
    }
    heap[i] = heap[p];
    heap[p] = tmp;
    heap[i]->i = i;
    heap[p]->i = p;
    i = p;
  }
}

static void
timer_siftdown(struct timer *t, int idx) {
  
}

void
timer_add(struct timer *t, struct timer_event *ev) {
  ev->i = t->size;
  if (t->size >= t->cap) {
    t->cap = t->cap * 2;
    t->heap = realloc(t->heap, t->cap);
  }
  t->heap[t->size] = ev;

  timer_siftup(t, ev->i);

  if (ev->i == 0) {

  }
}

void
timer_del(struct timer *t, struct timer_event *ev) {
  int last = t->size - 1;
  int i = ev->i;
  if (i != last) {
    t->heap[i] = t->heap[last];
    t->heap[i]->i = i;
  }
  t->heap[last] = NULL;
  t->size--;
  if (i != last) {
    timer_siftup(t, i);
    timer_siftdown(t, i);
  }
}

void
timer_update(struct timer *t, unsigned int now) {
  for (;;) {
    if (t->size == 0) return;
    struct timer_event *ev = t->heap[0];
    if (ev->when > now) return;

    if (ev->period > 0) {
      // this is a ticker, adjust next time to fire
      ev->when += ev->period;
      timer_siftdown(t, 0);
    } else {
      int last = t->size - 1;
      if (last > 0) {
        t->heap[0] = t->heap[last];
        t->heap[0]->i = 0;
      }
      t->heap[last] = NULL;
      t->size--;
      if (last>0) {
        timer_siftdown(t, 0);
      }
      ev->i = -1; // mark as removed
    }

    ev->callback(ev->data);
  }
}
