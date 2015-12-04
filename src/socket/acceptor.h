#ifndef _ACCEPTOR_
#define  _ACCEPTOR_

struct acceptor;
struct event;

struct acceptor* acceptor_create(int port);
void acceptor_release(struct acceptor*);

void event_add_acceptor(struct event*, struct acceptor*, void (*callback)(struct connection *conn));

#endif
