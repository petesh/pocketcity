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

VPATH=../source:../nix-common

debug : CFLAGS += -g -DDEBUG -DLOGGING -pg
debug : OPTFLAGS=
debug : OBJDIR= debug

all: OBJDIR= default

debug all:
	$(MAKE) $(MAKEFLAGS) OBJDIR=$(OBJDIR) $(OBJDIR)/pocketcity

#debug all : $(OBJDIR)/pocketcity

include ../Makefile-GlobalEntries
include ../nix-common/Makefile-nixcommon

OBJECTS = $(OBJECTS_CARELESS) $(OBJECTS_NIX) main.o savegame_fe.o budget.o \
	simulation-ui.o	uibits.o zonemon.o

OBJDEST = $(OBJECTS:%=$(OBJDIR)/%)

.PHONY: all debug clean

$(OBJDIR)/pocketcity: $(OBJDIR) $(OBJDEST)
	$(CC) $(CFLAGS) $(OBJDEST) -o $@ $(LDLIBS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf default debug

clobber:
	rm -f $(OBJDIR)/*.o core*

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

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
