VPATH=../source:../nix-common

debug : OBJDIR=out-debug

all: OBJDIR=out-default

profile: OBJDIR=out-profile

debug all profile:
	$(MAKE) OBJDIR=$(OBJDIR) -f Makefile-dowork $@

.PHONY: all debug clean

clean:
	rm -rf out-*

clobber:
	rm -f out-*/*.o core*

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
