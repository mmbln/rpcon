# 
# Makefile
#

.SUFFIXES:
.SUFFIXES: .c .o

OBJECTS = rspsrv.o controller.o time_control.o jsonsrv.o fifo.o
LIBS = -ljansson -lpthread -lm
CC = gcc
CFLAGS = -O2

rspsrv: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f rspsrv *.o

unittest:
	[ -d ./testdir ] || mkdir ./testdir
	rm -f ./testdir/*
	$(CC) -O2 -DUNITTEST -o ./testdir/fifo fifo.c
