#inlclude "event.h"

struct acceptor {
  int fd;
  struct handle handle;
  void *lua; // lua meta table
};

static void
acceptor_event_callback(void *ud) {
  int offset = ((struct acceptor*)0)->handle;
  struct *acceptor acceptor = ud - offset;

  // 从acceptor得到callback
  // 调用callback
}
