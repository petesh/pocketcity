
include Makefile-include

BINNAME=pcity

NDTARGETS=lores palm5 sony
DTARGETS=debug dpalm5 dsony

.PHONY: nextdir
.NOTPARALLEL:
.PRECIOUS: %.stm %.rcp

# Resource PDBs (graphical; non-language)
DEPTHS := bw twobit grey color merged
DENSITIES := single # onehalf double all
ITARGETS= $(foreach density,$(DENSITIES),$(DEPTHS:%=%-$(density)))
STUBS := multidensity-stubs.txt

COLORTARGETS := $(ITARGETS:%=$(BINNAME)-%.pdb)

TARGETS=$(DTARGETS) $(NDTARGETS) cheat

lores: TDIR =-nd

debug: TDIR =-d

palm5: TDIR =-nd5
palm5: BINNAME=palm5-pcity

dpalm5: TDIR =-d5
dpalm5: BINNAME=palm5-pcity

sony: TDIR =-nds
sony: BINNAME=sony-pcity

dsony: TDIR =-ds
dsony: BINNAME=sony-pcity

cheat: TDIR =-dc

OBJDIR = out$(TDIR)

%: OBJDIR =out$(TDIR)

VERSION := $(shell /bin/grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
nearlylive: VERSION = $(shell date +%Y.%m.%d)

DSTAMP := $(shell date "+%Y/%m/%d@%H:%M")

all: nextdir full

$(TARGETS): VER $(COLORTARGETS)
	$(MAKE) $(MAKEFLAGS) -f Makefile-dowork BINARY=$(BINNAME) \
		OBJDIR=out$(TDIR) $@

clean:
	rm -rf .temp-* $(BINNAME)-*.rcp *.pdb
	$(MAKE) $(MAKEFLAGS) -f Makefile-dowork BINARY=$(BINNAME) \
		OBJDIR=out$(TDIR) $@
# Build Stamping

VER: version.stm touch.stm

.temp:
	@mkdir -p .temp

version.stm: .temp ../CHANGELOG
	@echo "VERSION 1 \"$(VERSION)\"" > version.rcp
	$(RC.rcp) version.rcp .temp
	@rm -f version.rcp
	@echo touching>$@

touch.stm: .temp
	@echo "#include \"simcity_resconsts.h\"" > touch.rcp
	@printf "STRING ID StrID_build \"$(DSTAMP)\"" >> touch.rcp
	$(RC.rcp) touch.rcp .temp
	@rm -f touch.rcp

# Building zip/gz files
#
DDIR=pcity-palm_$(VERSION)

DISTFILES=../INSTALL ../CHANGELOG ../COPYING ../GAMEHINTS ../README

# This is the first target listed; hence it is the default target.
full: $(TARGETS)

dist nearlylive: $(NDTARGETS)
	@mkdir -p $(DDIR)
	@cp $(DISTFILES) *.pdb $(DDIR)
	for I in out-nd out-nd5 out-nds; do \
		cp $$I/*.prc $(DDIR); \
	done
	@tar cvf - $(DDIR) | gzip -9c >$(DDIR).tar.gz
	@zip -9R $(DDIR).zip $(DDIR)/*
	@rm -rf $(DDIR)

# This will take care of all *.rcp
# the files must be called this:
#	*-?-?.rcp => .temp-?
#
%.stm: %.rcp
	@cols=$(word 2, $(subst -, ,$*)); \
	dens=$(word 3, $(subst -, ,$*)); \
	echo $(RC.rcp) $(@:%.stm=%.rcp) .temp-$$cols-$$dens; \
	$(RC.rcp) $(@:%.stm=%.rcp) .temp-$$cols-$$dens

%.rcp: $(STUBS)
	@cols=$(word 2, $(subst -, ,$*)); \
	dens=$(word 3, $(subst -, ,$*)); \
	if [ ! -d .temp-$$cols-$$dens ]; then mkdir -p .temp-$$cols-$$dens; fi; \
	./generatercp.sh -c $$cols -d $$dens $(STUBS) $@

# The tile databases
pcity-%.pdb: pcity-%.stm
	$(BUILDPRC) --no-check-resources --bundle -o $@ -n $(DBNAME) \
		-c $(PROGTOKEN) -t $(COLORDBTOKEN) .temp-$*/*.bin


nextdir:
	cd graphic; $(MAKE) $(MAKEFLAGS)

FRC:
