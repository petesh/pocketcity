
include Makefile-include

BIN=pcity

NDTARGETS=lores palm5 sony
DTARGETS=dlores dpalm5 dsony cheat

.PHONY: %-nextdir
.NOTPARALLEL:
.PRECIOUS: %.stm %.rcp

# Resource PDBs (graphical; non-language)
DEPTHS := bw twobit grey color merged
DENSITIES := single # onehalf double all
ITARGETS= $(foreach density,$(DENSITIES),$(DEPTHS:%=%-$(density)))
STUBS := multidensity-stubs.txt

COLORTARGETS := $(ITARGETS:%=$(BIN)-%.pdb)

TARGETS=$(NDTARGETS)

lores: TDIR =nd
lores: BINNAME=$(BIN)

dlores: TDIR =d
dlores: BINNAME=debug-$(BIN)

palm5: TDIR =nd5
palm5: BINNAME=palm5-$(BIN)

dpalm5: TDIR =d5
dpalm5: BINNAME=debug-palm5-$(BIN)

sony: TDIR =nds
sony: BINNAME=sony-$(BIN)

dsony: TDIR =ds
dsony: BINNAME=debug-sony-$(BIN)

cheat: TDIR =dc
cheat: BINNAME=debug-cheat-$(BIN)

OBJDIR = .out-$(TDIR)

%: OBJDIR = .out-$(TDIR)

VERSION := $(shell /bin/grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
nearlylive: VERSION = $(shell date +%Y.%m.%d)

DSTAMP := $(shell date "+%Y/%m/%d@%H:%M")

all: all-nextdir $(TARGETS)
debug: debug-nextdir $(DTARGETS)
nearlylive: all-nextdir
dist: all-nextdir

$(TARGETS) $(DTARGETS): VER $(COLORTARGETS)
	$(MAKE) $(MAKEFLAGS) -f Makefile-dowork BINARY=$(BINNAME) \
		OBJDIR=.out-$(TDIR) BINPRE=$(TDIR) $@

clean: clean-graphic
	rm -rf .temp-* $(BIN)-*.rcp *.pdb *.prc
	$(MAKE) $(MAKEFLAGS) -f Makefile-dowork BINARY=$(BINNAME) \
		OBJDIR=.out-$(TDIR) $@

clean-%:
	cd $(@:clean-%=%) && $(MAKE) $(MAKEFLAGS) clean
	
# Build Stamping

VER: version.stm touch.stm

.temp:
	@mkdir -p .temp

version.stm: .temp ../CHANGELOG
	@echo "VERSION 1 \"$(VERSION)\"" > version.rcp
	$(RC.rcp) version.rcp .temp
	@rm -f version.rcp
	@echo touching $(VERSION)>$@

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

dist nearlylive: all-nextdir $(NDTARGETS)
	@mkdir -p $(DDIR)
	@cp $(DISTFILES) *.pdb *.prc $(DDIR)
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


%-nextdir:
	cd graphic && $(MAKE) $(MAKEFLAGS) $(@:%-nextdir=%)

FRC:
