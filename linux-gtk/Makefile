CC = gcc
CFLAGS = -O2 -Wall -DLINUX `pkg-config --cflags gtk+-2.0`
LFLAGS = `pkg-config --libs gtk+-2.0`
OBJECTS = main.o savegame.o budget.o \
			handler.o build.o disaster.o drawing.o globals.o simulation.o

.PHONY: all debug clean

all: pcity

debug: CFLAGS += -g -DDEBUG
debug: all

pcity: $(OBJECTS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJECTS) -o pcity

%.o: %.c %.h ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: ../source/%.c ../source/%.h ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
