#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define MAXEVENTS 64

static int
make_socket_non_blocking(int sfd) {
	int flags, s;
	flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1) {
		perror("fcntl");
		return -1;
	}
	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1) {
		perror("fcntl");
		return -1;
	}
	return 0;
}

static int
create_and_bind(const char *port) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	 /* Return IPv4 and IPv6 choices */
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
	hints.ai_flags = AI_PASSIVE;	 /* All interfaces */

	s = getaddrinfo(NULL, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1) continue;
		s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0) {
			/* We managed to bind successfully! */
			break;
		}
		close(sfd);
	}
	if (rp == NULL) {
		fprintf(stderr, "Could not bind\n");
		return -1;
	}
	freeaddrinfo(result);
	return sfd;
}

static int
l_listen(lua_State* L) {
	const char *addr = luaL_checkstring(L, 1);
	int sfd = create_and_bind(addr);
	if (sfd == -1) abort();

	int s = make_socket_non_blocking(sfd);
	if (s == -1) abort();

	s = listen(sfd, SOMAXCONN);
	if (s == -1) {
		perror("listen");
		abort();
	}
	lua_pop(L, 1);
	lua_pushinteger(L, sfd);
	return 1;
}

static int
l_read(lua_State* L) {
	int fd = luaL_checkinteger(L, 1);
	luaL_Buffer buf;
	luaL_buffinit(L, &buf);
	lua_settop(L, 0);

	char tmp[256];
	int sz = read(fd, tmp, 256);
	while(sz > 0) {
		luaL_addlstring(&buf, tmp, sz);
		sz = read(fd, tmp, 256);
	}

	if (sz == 0) goto closed;
	if (sz < 0) {
		if (errno==ECONNRESET) goto closed;
		if (errno!=EAGAIN && errno!=EWOULDBLOCK) {
			luaL_pushresult(&buf);
			lua_pushstring(L, strerror(errno));
			return 2;
		}
	}
	luaL_pushresult(&buf);
	return 1;
closed:
	close(fd);
	return 0;
}

static int
l_write(lua_State* L) {
	int fd = luaL_checkinteger(L, 1);
	size_t len;
	const char *data = luaL_checklstring(L, 2, &len);
	lua_settop(L, 0);

	int off = 0;
	while(off < len) {
		int n = write(fd, data+off, len-off);
		if (n == 0) break; // TODO: socket closed?
		if (n<0) {
			if (errno==EAGAIN && errno==EWOULDBLOCK)
				break;

			lua_pushinteger(L, off);
			lua_pushstring(L, strerror(errno));
			return 2;
		}
		off += n;
	}

	lua_pushinteger(L, off);
	return 1;
}

static int
l_close(lua_State* L) {
	int fd = luaL_checkinteger(L, 1);
	close(fd);
	return 0;
}

static int
l_accept(lua_State* L) {
	int lnfd = luaL_checkinteger(L, 1);
	int fd = accept(lnfd, NULL, NULL);
	lua_settop(L, 0);
	lua_pushinteger(L, fd);
	if (fd < 0) {
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	return 1;
}

int
luaopen_c_socket(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg socklib[] = {
		{"listen", l_listen},
		{"accept", l_accept},
		{"read", l_read},
		{"write", l_write},
		{"close", l_close},
		{NULL, NULL}
	};
	luaL_newlib(L, socklib);
	return 1;
}
