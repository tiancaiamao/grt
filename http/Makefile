osx:
	gcc -Wall -g -bundle -undefined dynamic_lookup -fPIC http_parser.c http_parser_lua.c -o chttp.so

linux:
	gcc -I /usr/include/lua5.2 -fpic -Wall --shared http_parser.c http_parser_lua.c -o chttp.so
