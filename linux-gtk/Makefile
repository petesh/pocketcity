VERSION := $(shell grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
CC = gcc
GFLAGS := $(shell pkg-config --cflags gtk+-2.0)
CFLAGS = -O2 -Wall -DLINUX $(GFLAGS) -I . -I ../source
GLFLAGS = $(shell pkg-config --libs gtk+-2.0)
LFLAGS = $(GLFLAGS)
OBJECTS = main.o savegame.o budget.o \
	handler.o build.o disaster.o drawing.o globals.o simulation.o \
	stack.o

.PHONY: all debug clean

all: pocketcity

debug: CFLAGS += -g -DDEBUG
debug: all

pocketcity: $(OBJECTS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJECTS) -o $@

%.o: %.c %.h ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: ../source/%.c ../source/%.h ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o pocketcity core*

clobber:
	rm -f *.o core*

DDIR=pocketcity-linux-GTK_$(VERSION)

dist:
	@mkdir -p $(DDIR)
	@cp ../Changelog ../Copying ../Gamehints ../Readme pocketcity $(DDIR)
	@mkdir -p $(DDIR)/graphic/icons
	@cp graphic/*.png $(DDIR)/graphic
	@cp graphic/icons/*.png $(DDIR)/graphic/icons
	@tar cvzf $(DDIR).tar.gz $(DDIR)
	@rm -rf $(DDIR)
