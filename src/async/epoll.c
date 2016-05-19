#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "socket_poll.h"

static int
lepoll_create(lua_State *L) {
	int efd = sp_create();
	if (efd == -1) {
		perror("epoll_create");
		abort();
	}
	lua_pushinteger(L, efd);
	return 1;
}

enum {
	CTL_ADD = 1,
	CTL_DEL = 2,
	CTL_WRITE = 3,

	MASK_READ = 4,
	MASK_WRITE = 8,
};

static int
lepoll_ctl(lua_State *L) {
	int epfd = luaL_checkinteger(L, 1);
	int op = luaL_checkinteger(L, 2);
	int fd = luaL_checkinteger(L, 3);

	switch(op) {
		case CTL_ADD:
			sp_add(epfd, fd, fd);
			break;
		case CTL_DEL:
			sp_del(epfd, fd);
			break;
		case CTL_WRITE:
			sp_write(epfd, fd, fd, lua_toboolean(L, 4));
			break;
	}
	return 0;
}

static const int MAXEVENTS = 64;

static int
lepoll_wait(lua_State *L) {
	int efd = luaL_checkinteger(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);

	struct event events[MAXEVENTS];
	int n = sp_wait(efd, events, MAXEVENTS, 200);
	int i;
	for(i = 0; i < n; i++) {
		lua_rawgeti(L, -1, i+1);
		if (lua_type(L, -1) != LUA_TTABLE) {
			lua_pop(L, 1);
			lua_newtable(L);
		}
		lua_pushstring(L, "fd");
		lua_pushinteger(L, events[i].s);
		lua_rawset(L, -3);

		int mask = 0;
		if (events[i].read) mask |= MASK_READ;
		if (events[i].write) mask |= MASK_WRITE;
		lua_pushstring(L, "mask");
		lua_pushinteger(L, mask);
		lua_rawset(L, -3);
		lua_rawseti(L, -2, i+1);
	}
	lua_settop(L, 0);
	lua_pushinteger(L, n);
	return 1;
}

int
luaopen_c_epoll(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg lib[] = {
		{"create", lepoll_create},
		{"ctl", lepoll_ctl},
		{"wait", lepoll_wait},
		{NULL, NULL}
	};
	luaL_newlib(L, lib);

	lua_pushstring(L, "CTL_ADD");
	lua_pushinteger(L, CTL_ADD);
	lua_rawset(L, -3);

	lua_pushstring(L, "CTL_DEL");
	lua_pushinteger(L, CTL_DEL);
	lua_rawset(L, -3);

	lua_pushstring(L, "CTL_WRITE");
	lua_pushinteger(L, CTL_WRITE);
	lua_rawset(L, -3);

	lua_pushstring(L, "MASK_READ");
	lua_pushinteger(L, MASK_READ);
	lua_rawset(L, -3);

	lua_pushstring(L, "MASK_WRITE");
	lua_pushinteger(L, MASK_WRITE);
	lua_rawset(L, -3);

	return 1;
}
