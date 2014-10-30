
CC=g++
STRIP=strip
CPPFLAGS= -D__STDC_FORMAT_MACROS -shared -Wall
prefix ?=/usr/local
prefix_inc :=$(prefix)/include
prefix_lib :=$(prefix)/lib
target :=libavtool.so libsock.so
target_inc :=sockets.h avtool.h
.phony: all clean

all:$(target)
libsock.so:sockets.o
	$(CC) $(CPPFLAGS) -fPIC -o $@ $^
	$(STRIP) $@
libavtool.so:avtool.o
	$(CC) $(CPPFLAGS)  -fPIC -o $@ $^
	$(STRIP) $@
install:all
	install $(target) $(prefix_lib)
	install $(target_inc) $(prefix_inc)
clean:
	-rm -rf *.o *.so