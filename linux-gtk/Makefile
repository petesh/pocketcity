VERSION := $(shell grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
GCPPFLAGS := $(shell pkg-config --cflags gtk+-2.0) \
	-DGTK_DISABLE_COMPAT_H -DGDK_DISABLE_DEPRECATED
ifeq ($(CC),gcc)
GCCFLAGS = -W -Wall -Werror
OPTFLAGS = -O2
else
GCCFLAGS = -v -erroff=%none -errwarn=%all
OPTFLAGS = -xO2
endif
INCLUDEDIRS = -I . -I ../source -I ../nix-common -I ../graphic
CPPFLAGS=$(GCPPFLAGS) -DLINUX $(INCLUDEDIRS)
CFLAGS = $(CPPFLAGS) $(OPTFLAGS) $(GCCFLAGS)
GLFLAGS = $(shell pkg-config --libs gtk+-2.0)
LIBS = 
LDLIBS = $(LIBS) $(GLFLAGS)
# -lefence

default: debug

VPATH=../source:../nix-common

include ../Makefile-GlobalEntries
include ../nix-common/Makefile-nixcommon

OBJECTS = $(OBJECTS_CARELESS) $(OBJECTS_NIX) main.o savegame_fe.o budget.o \
	simulation-ui.o	uibits.o zonemon.o

.PHONY: all debug clean

all: pocketcity

debug: CFLAGS += -g -DDEBUG -pg
debug: OPTFLAGS=
debug: all

pocketcity: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

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
