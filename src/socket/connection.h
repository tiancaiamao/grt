#ifndef _CONNECTION_
#define _CONNECTION_

struct packet;
struct event;
struct connection;

struct connection* connection_create(int fd);
void connection_close();
void event_add_connection(struct event*, struct connection*, void (*callback)(struct packet*, int, struct connection*));
void event_del_connection(struct event*, struct connection*);
void connection_send(struct connection*, struct packet*, void (*callback)(struct connection*));

#endif
