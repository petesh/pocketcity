
include Makefile-include

BINNAME=pcity

NDTARGETS=lores hires
DTARGETS=debug dhires

TARGETS=$(NDTARGETS) $(DTARGETS) cheat

lores: TDIR =-nd

debug: TDIR =-d
hires: TDIR =-ndh
hires: BINNAME=hires-pcity
dhires: TDIR =-dh
dhires: BINNAME=hires-pcity
cheat: TDIR =-dc

OBJDIR = out$(TDIR)

%: OBJDIR =out$(TDIR)

VERSION := $(shell grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
nearlylive: VERSION = $(shell date +%Y.%m.%d)

DSTAMP := $(shell date "+%Y/%m/%d@%H:%M")

all: lores

$(TARGETS): VER
	$(MAKE) -f Makefile-dowork BINARY=$(BINNAME) OBJDIR=out$(TDIR) $@

clean:
	$(MAKE) -f Makefile-dowork BINARY=$(BINNAME) OBJDIR=out$(TDIR) $@
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

full: $(TARGETS)

dist nearlylive: $(NDTARGETS)
	@mkdir -p $(DDIR)
	@cp $(DISTFILES) out-nd/*.prc *.pdb $(DDIR)
	@tar cvf - $(DDIR) | gzip -9c >$(DDIR).tar.gz
	@cp out-ndh/*.prc $(DDIR)
	@zip -9R $(DDIR).zip $(DDIR)/*
	@rm -rf $(DDIR)

