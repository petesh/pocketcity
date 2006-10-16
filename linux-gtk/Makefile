VPATH=../source:../nix-common

include ../Makefile-settings

OBJDIR=out-$(BUILD_MODE)

TARGET_debug=debug
TARGET_release=release

debug release profile:
	$(MAKE) OBJDIR=$(OBJDIR) BUILD_MODE=$(BUILD_MODE) -f Makefile-dowork $@

all: $(TARGET_$(BUILD_MODE))

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
