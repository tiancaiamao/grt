SRCS = ltask.c handlemap.c queue.c schedule.c serialize.c

all :
	echo 'make macosx or make linux or make mingw'

macosx : ltask.dylib
linux : ltask.so
mingw : ltask.dll

ltask.dylib : $(SRCS)
	gcc -g -Wall -bundle -undefined dynamic_lookup -fPIC -o $@ $^ -lpthread

ltask.so : $(SRCS)
	gcc -Wall -g --shared -fpic -o$@ $^ -lpthread

ltask.dll : $(SRCS)
	gcc -Wall -g --shared -o $@ $^ -I/usr/local/include -L/usr/local/bin -llua53

clean :
	rm -f ltask.dylib ltask.so ltask.dll
