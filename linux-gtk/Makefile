VERSION := $(shell grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
CC = gcc
GCPPFLAGS := $(shell pkg-config --cflags gtk+-2.0) \
	-DGTK_DISABLE_COMPAT_H -DGDK_DISABLE_DEPRECATED
GCCFLAGS = -W -Wall -Werror
INCLUDEDIRS = -I . -I ../source -I ../nix-common -I ../graphic
OPTFLAGS = -O2
CPPFLAGS=$(GCPPFLAGS) -DLINUX $(INCLUDEDIRS)
CFLAGS = $(CPPFLAGS) $(OPTFLAGS) $(GCCFLAGS)
GLFLAGS = $(shell pkg-config --libs gtk+-win32-2.0) 
LIBS = 
LDLIBS = $(LIBS) $(GLFLAGS)
# -lefence

OBJECTS = main.o savegame_fe.o budget.o \
	stack.o savegame_be.o pglobals.o nix_utils.o simulation-ui.o \
	handler.o build.o disaster.o drawing.o globals.o simulation.o \
	uibits.o stringsearch.o zonemon.o zonestrings.o

.PHONY: all debug clean

default: debug

all: pocketcity

debug: CFLAGS += -g -DDEBUG -pg
debug: OPTFLAGS=
debug: all

pocketcity: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

%.o: %.c %.h ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: ../nix-common/%.c ../nix-common/%.h ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: ../source/%.c ../source/%.h ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: ../nix-common/%.c ../source/%.h ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: ../nix-common/%.c ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o pocketcity core* pocketcity.exe

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

splint:
	splint $(CPPFLAGS) *.c
