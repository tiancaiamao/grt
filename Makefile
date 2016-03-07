SRCS = \
src/ltask.c \
src/handlemap.c \
src/queue.c \
src/schedule.c \
src/serialize.c

all :
	echo 'make macosx or make linux or make mingw'

macosx : luaclib/ltask.dylib luaclib/csocket.dylib luaclib/csystem.dylib
linux : ltask.so
mingw : ltask.dl

luaclib/csystem.dylib : src/system.c
	gcc -g -Wall -bundle -undefined dynamic_lookup -fPIC -o $@ $^

luaclib/csocket.dylib : src/socket_lib.c
	gcc -g -Wall -bundle -undefined dynamic_lookup -fPIC -o $@ $^

luaclib/ltask.dylib : $(SRCS)
	gcc -g -Wall -bundle -undefined dynamic_lookup -fPIC -o $@ $^ -lpthread

ltask.so : $(SRCS)
	gcc -Wall -g --shared -fpic -o$@ $^ -lpthread

ltask.dll : $(SRCS)
	gcc -Wall -g --shared -o $@ $^ -I/usr/local/include -L/usr/local/bin -llua53

clean :
	rm -rf luaclib/*
