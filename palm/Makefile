
include Makefile-include

all: TDIR =-nd
debug: TDIR =-d
hires: TDIR =-ndh
dhires: TDIR =-dh
cheat: TDIR =-dc

OBJDIR = out$(TDIR)

%: OBJDIR =out$(TDIR)

VERSION := $(shell grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
nearlylive: VERSION = $(shell date +%Y.%m.%d)

DSTAMP := $(shell date "+%Y/%m/%d@%H:%M")

all debug hires dhires cheat: VER
	$(MAKE) -f Makefile-dowork OBJDIR=out$(TDIR) $@

clean:
	$(MAKE) -f Makefile-dowork OBJDIR=out$(TDIR) $@
# Build Stamping

VER: version.stm touch.stm

temp:
	@mkdir -p temp

version.stm: temp ../CHANGELOG
	@echo "VERSION 1 \"$(VERSION)\"" > version.rcp
	$(RC.rcp) version.rcp temp
	@rm -f version.rcp
	@echo touching>$@

touch.stm: temp
	@echo "#include \"simcity_resconsts.h\"" > touch.rcp
	@printf "STRING ID StrID_build \"$(DSTAMP)\"" >> touch.rcp
	$(RC.rcp) touch.rcp temp
	@rm -f touch.rcp

# Building zip/gz files
#
DDIR=pcity-palm_$(VERSION)

DISTFILES=../INSTALL ../CHANGELOG ../COPYING ../GAMEHINTS ../README

dist nearlylive: all hires
	@mkdir -p $(DDIR)
	@cp $(DISTFILES) out-nd/*.prc *.pdb $(DDIR)
	@tar cvf - $(DDIR) | gzip -9c >$(DDIR)-normal.tar.gz
	@zip -9R $(DDIR)-normal.zip $(DDIR)/*
	@rm -f $(DDIR)/*
	@cp $(DISTFILES) out-ndh/*.prc *.pdb $(DDIR)
	@tar cvf - $(DDIR) | gzip -9c >$(DDIR)-sony.tar.gz
	@zip -9R $(DDIR)-sony.zip $(DDIR)/*
	@rm -rf $(DDIR)

