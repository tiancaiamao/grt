#include "socket/connection.h"
#include "socket/acceptor.h"
#include "event/event.h"

static struct event* evt;

static void
on_finish(struct connection *conn) {
  event_del_connection(evt, conn);
  connection_close(conn);
}

static void
on_data(struct packet *p, int err, struct connection *conn) {
  if (err == 0) {
    connection_send(conn, p, on_finish);
  } else {
    connection_close(conn);
  }
}

static void
on_new_client(struct connection *conn) {
  event_add_connection(evt, conn, on_data);
}

int main() {
  evt = event_create();

  struct acceptor* acpt = acceptor_create(8088);
  event_add_acceptor(evt, acpt, on_new_client);

  event_loop(evt);
}
