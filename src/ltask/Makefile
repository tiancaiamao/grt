SRCS = ltask.c handlemap.c queue.c schedule.c serialize.c

macosx : $(SRCS)
	gcc -Wall -bundle -undefined dynamic_lookup -fPIC -o ../../lib/ltask.so $^ -lpthread

linux : ltask.so
mingw : ltask.dll

luaclib/csystem.dylib : src/system.c
	gcc -g -Wall -bundle -undefined dynamic_lookup -fPIC -o $@ $^

luaclib/csocket.dylib : src/socket_lib.c
	gcc -g -Wall -bundle -undefined dynamic_lookup -fPIC -o $@ $^

ltask.so : $(SRCS)
	gcc -Wall -g --shared -fpic -o$@ $^ -lpthread

ltask.dll : $(SRCS)
	gcc -Wall -g --shared -o $@ $^ -I/usr/local/include -L/usr/local/bin -llua53

clean :
	rm -rf luaclib/*
