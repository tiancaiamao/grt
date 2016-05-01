osx:
	gcc -Wall -bundle -undefined dynamic_lookup -fPIC socket.c epoll.c -o c.so

linux:
	gcc -I /usr/include/lua5.2 -fpic -Wall --shared socket.c epoll.c -o c.so
