# Makefile for the tests
#

CFLAGS=-g -Wall
CPPFLAGS=-I. -I../source

TESTBINARIES=test_list test_horst
TESTOBJS=stack.o stringsearch.o

all: $(TESTBINARIES)

test_%: test_%.c $(TESTOBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(TESTOBJS)

%.o: ../source/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $*.c

clean:
	rm -f $(TESTBINARIES) $(TESTBINARIES:%=%.exe) *.o core
