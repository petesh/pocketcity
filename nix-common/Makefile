# Makefile for the tests
#

CFLAGS=-g -Wall
CPPFLAGS=-I. -I../source -I../graphic -I../nix-common -I../linux-gtk

TESTBINARIES=test_list test_horst test_pack
TESTOBJS=stack.o stringsearch.o pack.o

all: $(TESTBINARIES)

test_%: test_%.c $(TESTOBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(TESTOBJS)

%.o: ../source/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $*.c

clean:
	rm -f $(TESTBINARIES) $(TESTBINARIES:%=%.exe) *.o core
