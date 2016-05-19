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
    int p = (i-1) / 2; // parent
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
    struct timer_event **heap = t->heap;
    struct timer_event *tmp = heap[idx];

    int left = (2*idx)+1;
    int right = 2*(idx+1);
    while(right < t->size) {
        int smaller = heap[left]->when < heap[right]->when ? left : right;
        if (tmp->when < heap[smaller]->when) {
            heap[idx] = tmp;
            heap[idx]->i = idx;
            return;
        }
        heap[idx] = heap[smaller];
        heap[idx]->i = idx;

        idx = smaller;
        left = (2*idx)+1;
        right = 2*(idx+1);
    }
    if (left < t->size && heap[left]->when < tmp->when) {
        heap[idx] = heap[left];
        heap[idx]->i = idx;
        idx = left;
    }

    heap[idx] = tmp;
    heap[idx]->i = idx;
}

void
timer_add(struct timer *t, struct timer_event *ev) {
  ev->i = t->size;
  if (t->size >= t->cap) {
    t->cap = t->cap * 2;
    t->heap = realloc(t->heap, t->cap);
  }
  t->heap[t->size] = ev;
  t->size++;

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

struct timer_event*
timer_latest(struct timer *t) {
    if (t->size == 0) return NULL;
    return t->heap[0];
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

#ifdef _TEST_

#include <stdio.h>

static void
func(void *data) {
    struct timer_event *e = data;
    printf("timer %d period %d\n", e->when, e->period);
}

int main() {
    struct timer t;
    timer_init(&t);
    int bombs[] = {56, 37, 31, 68, 723, 264, 342, 445, 884, 5212, 2635, 997};

    for (int i=0; i<sizeof(bombs)/sizeof(int); i++) {
        struct timer_event *e = malloc(sizeof(*e));
        e->when = bombs[i];
        if (i==3) {
            e->period = 5;
        }
        e->callback = func;
        e->data = e;
        timer_add(&t, e);
    }


    struct timer_event *e = timer_latest(&t);
    printf("latest = %d\n", e->when);

    for (unsigned int i=0; i<1000; i++) {
        timer_update(&t, i);
    }

    timer_release(&t);
}

#endif
