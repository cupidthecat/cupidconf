CC = cc
CFLAGS ?= -O2 -Wall -Wextra -std=c11
AR ?= ar
all: libcupidconf.a
cupidconf.o: cupidconf.c cupidconf.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
libcupidconf.a: cupidconf.o
	$(AR) rcs $@ $^
tests/test_conf: tests/test_conf.c cupidconf.c cupidconf.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -I. tests/test_conf.c cupidconf.c $(LDFLAGS) $(LDLIBS) -o $@
test: tests/test_conf
	./tests/test_conf
clean:
	rm -f cupidconf.o libcupidconf.a tests/test_conf
.PHONY: all test clean
