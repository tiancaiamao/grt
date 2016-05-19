#include <stdio.h>
#include "http_parser.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

static int
message_begin_cb(http_parser *parser) {
    lua_State *L = parser->data;
    printf("---lua stack size: %d\n", lua_gettop(L));
    printf("-------message_begin\n");
    return 0;
}

static int
url_cb(http_parser *parser, const char *data, size_t len) {
    lua_State *L = parser->data;
    printf("---lua stack size: %d\n", lua_gettop(L));
    printf("--------url cb:%s %p\n", data, L);
    lua_pushstring(L, "URL");
    lua_pushlstring(L, data, len);
    lua_rawset(L, 1);
    return 0;
}

static int
status_cb(http_parser *parser, const char *data, size_t len) {
    lua_State *L = parser->data;
    printf("---lua stack size: %d\n", lua_gettop(L));
    printf("---------status:%s\n", data);
    lua_pushstring(L, "Status");
    lua_pushlstring(L, data, len);
    lua_rawset(L, 1);
    return 0;
}

static int
header_field_cb(http_parser *parser, const char *data, size_t len) {
    // getchar();
    lua_State *L = parser->data;
    printf("---lua stack size: %d\n", lua_gettop(L));
    printf("--------field:%s\n", data);
    lua_pushstring(L, "last_field");
    lua_pushlstring(L, data, len);
    lua_rawset(L, 1);
    return 0;
}

static int
header_value_cb(http_parser *parser, const char *data, size_t len) {
    lua_State *L = parser->data;
    printf("---lua stack size: %d\n", lua_gettop(L));
    printf("--------value:%s\n", data);

    // head = Response[Head] or {}
    lua_pushstring(L, "Head");
    lua_pushstring(L, "Head");
    lua_rawget(L, 1);
    if (lua_type(L, -1) == LUA_TNIL) {
        printf("第一次会新建head表");
        lua_pop(L, 1);
        lua_newtable(L);
    }

    // head[Response[last_field]] = data
    lua_pushstring(L, "last_field");
    lua_rawget(L, 1);
    // TODO check not nil
    printf("应该check不为nil的");
    lua_pushlstring(L, data, len);
    lua_rawset(L, -3);

    // Response[Head] = head
    lua_rawset(L, 1);

    // Response[last_field] = nil
    lua_pushstring(L, "last_field");
    lua_pushnil(L);
    lua_rawset(L, 1);
    return 0;
}

static int
headers_complete_cb(http_parser *parser) {
    lua_State *L = parser->data;
    printf("---lua stack size: %d\n", lua_gettop(L));
    printf("---------headers complete\n");
    lua_pushstring(L, "headers_complete");
    lua_pushboolean(L, 1); 
    return 0;
}

static int
body_cb(http_parser *parser, const char *data, size_t len) {
    lua_State *L = parser->data;
    lua_pushstring(L, "Body");
    lua_pushlstring(L, data, len);
    lua_rawset(L, 1);
    return 0;
}

static int
message_complete_cb(http_parser *parser) {
    lua_State *L = parser->data;
    printf("---lua stack size: %d\n", lua_gettop(L));
    lua_pushstring(L, "complete");
    lua_pushboolean(L, 1);
    lua_rawset(L, 1);
    return 0;
}

static int
chunk_header_cb(http_parser *parser) {
    printf("----chunk_header_cb not implement yet\n");
    return 1;
}

static int
chunk_complete_cb(http_parser *parser) {
    printf("chunk_complete_cb not implement yet\n");
    return 1;
}

static http_parser_settings settings = {
  .on_message_begin = &message_begin_cb,
  .on_url = &url_cb,
  .on_status = &status_cb,
  .on_header_field = &header_field_cb,
  .on_header_value = header_value_cb,
  .on_headers_complete = headers_complete_cb,
  .on_body = body_cb,
  .on_message_complete = message_complete_cb,
  .on_chunk_header = chunk_header_cb,
  .on_chunk_complete = chunk_complete_cb,
};

static int
l_parse_http(lua_State *L) {
    http_parser *parser = NULL;
    lua_pushstring(L, "parser");
    lua_rawget(L, 1);
    if (lua_type(L, -1) == LUA_TNIL) {
        printf("新建parser");
        lua_pop(L, 1);
        lua_pushstring(L, "parser");
        parser = lua_newuserdata(L, sizeof(http_parser));
        http_parser_init(parser, HTTP_REQUEST);
        lua_rawset(L, 1);
    } else {
        printf("重用parser");
        parser = lua_touserdata(L, -1);
    }

    printf("运行到这里了, request[parse]\n");

    size_t len;
    const char *data = luaL_checklstring(L, 2, &len);
    lua_pop(L, 1);
    parser->data = L; // for callback to use

    printf("parser = %p\n", parser);
    size_t parsed = http_parser_execute(parser, &settings, data, len);
    if (parsed != len) {
        printf("error happend?? input %zu, parsed %zu", len, parsed);
        return 0;
    }

    printf("运行到返回前\n");
    return 0;
}

int
luaopen_chttp(lua_State *L) {
    luaL_Reg lib[] = {
        {"parse", l_parse_http},
        {NULL, NULL},
    };
    luaL_newlib(L, lib);
    return 1;
}
