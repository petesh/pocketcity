VERSION := $(shell grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
CC = gcc
GFLAGS = `pkg-config --cflags gtk+-2.0`
CFLAGS = -O2 -Wall -DLINUX $(GFLAGS)
GLFLAGS = `pkg-config --libs gtk+-2.0`
LFLAGS = $(GLFLAGS)
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
	rm -f *.o pcity core*

clobber:
	rm -f *.o core*

DDIR=pcity-linux-GTK_$(VERSION)

dist:
	@mkdir -p $(DDIR)
	@cp ../Changelog ../Copying ../Gamehints ../Readme pcity $(DDIR)
	@mkdir -p $(DDIR)/graphic/icons
	@cp graphic/*.xpm $(DDIR)/graphic
	@cp graphic/icons/*.xpm $(DDIR)/graphic/icons
	@tar cvzf $(DDIR).tar.gz $(DDIR)
	@rm -rf $(DDIR)
