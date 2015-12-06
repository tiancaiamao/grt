#include "event.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdbool.h>

#define TNAME "acceptor_meta"

struct connection {
    struct handle ud; // 继承handle

    bool read;
    bool write;
    void *lua;

  
};

static void
connection_event_callback(void *ud, struct poll_event *e) {
  int offset = ((struct handle*)0)->ud;
  struct connection *cnct = ud - offset;

  if (e->read) {
    
  }
  // 从lua拿到lua表
  // 从table中得到回调函数
  // 调用回调
}

static int
lconnection_send(lua_State *L) {
  int len;
  struct connection *conn = luaL_checkudata(L, 1, TNAME);
  char *data = luaL_checklstring(L, 2, &len);
}
