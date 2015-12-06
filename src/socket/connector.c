#include "event.h"
#include <stdbool.h>

struct connector {
  struct handle ud; // 继承handle
  void *lua;
};

static void
connector_event_callback(void *ud) {
  int offset = ((struct handle*)0)->ud;
  struct connector *cnct = ud - offset;
  // 从lua拿到lua表
  // 从table中得到回调函数
  // 调用回调
}
