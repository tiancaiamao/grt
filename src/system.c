// 这个包存在的原因是需要一个能跨vm的table，阻止核心服务被多次加载
#include "lua.h"
#include "lauxlib.h"
#include "schedule.h"

enum {
  SERVICE_SOCKET = 1,
  SERVICE_MAX,
};

static channelid srv_table[SERVICE_MAX];

static int
lget(lua_State *L) {
  lua_Integer srv_id = luaL_checkinteger(L, 1);
  if (srv_table[srv_id] == 0) {
    lua_pushnil(L);
  } else {
    lua_pushinteger(L, srv_table[srv_id]);
  }
  return 1;
}

static int
lset(lua_State *L) {
  lua_Integer srv_id = luaL_checkinteger(L, 1);
  channelid ch_id = luaL_checkinteger(L, 2);
  srv_table[srv_id] = ch_id;
  return 0;
}

int 
luaopen_csystem(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "get", lget },
		{ "set", lset },
		{ NULL, NULL },
	};
  luaL_newlib(L, l);
  lua_pushstring(L, "SOCKET");
  lua_pushinteger(L, SERVICE_SOCKET);
  lua_settable(L, -3);
	return 1;
}
