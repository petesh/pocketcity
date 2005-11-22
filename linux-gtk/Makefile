VERSION := $(shell grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
GCPPFLAGS := $(shell pkg-config --cflags gtk+-2.0) \
	-DGTK_DISABLE_COMPAT_H -DGDK_DISABLE_DEPRECATED

ifeq ($(CC),suncc)
GCCFLAGS = -v -erroff=%none -errwarn=%all
OPTFLAGS = -xO2
else
GCCFLAGS = -W -Wall -Werror
OPTFLAGS = -O2
endif
INCLUDEDIRS = -I . -I ../source -I ../nix-common -I ../graphic
CPPFLAGS=$(GCPPFLAGS) -DLINUX $(INCLUDEDIRS)
CFLAGS = $(CPPFLAGS) $(OPTFLAGS) $(GCCFLAGS)
GLFLAGS = $(shell pkg-config --libs gtk+-2.0)
LIBS = 
LDLIBS = $(LIBS) $(GLFLAGS)
# -lefence

VPATH=../source:../nix-common

debug : OBJDIR=out-debug

all: OBJDIR=out-default

debug all :
	$(MAKE) OBJDIR=$(OBJDIR) -f Makefile-dowork $@

.PHONY: all debug clean

clean:
	rm -rf out-default out-debug

clobber:
	rm -f $(OBJDIR)/*.o core*

DDIR=pocketcity-linux-GTK_$(VERSION)

dist: default
	@mkdir -p $(DDIR)
	@cp ../Changelog ../Copying ../Gamehints ../Readme \
	  default/pocketcity $(DDIR)
	@mkdir -p $(DDIR)/graphic/icons
	@cp graphic/*.png $(DDIR)/graphic
	@cp graphic/icons/*.png $(DDIR)/graphic/icons
	@tar cvzf $(DDIR).tar.gz $(DDIR)
	@rm -rf $(DDIR)

splint:
	splint $(CPPFLAGS) *.c
