
include Makefile-include

BIN=PocketCity
TILE=$(BIN)

NDTARGETS=lores palm5 sony
DTARGETS=dlores dpalm5 dsony cheat

.PHONY: %-nextdir
.NOTPARALLEL:
.PRECIOUS: %.stm %.rcp

# Resource PDBs (graphical; non-language)
DEPTHS := bw twobit grey color merged
DENSITIES := 1 # onehalf double all
ITARGETS= $(foreach density,$(DENSITIES),$(DEPTHS:%=%-$(density)))
STUBS := multidensity-stubs.txt

COLORTARGETS := $(ITARGETS:%=$(TILE)-%.pdb)

TARGETS=$(NDTARGETS)

BINNAME=$(BIN)

lores: TDIR =standard
#lores: BINNAME=$(BIN)

dlores: TDIR =debug
#dlores: BINNAME=$(BIN)

palm5: TDIR =palm5
#palm5: BINNAME=$(BIN)

dpalm5: TDIR =debug-palm5
#dpalm5: BINNAME=$(BIN)

sony: TDIR =sony
#sony: BINNAME=$(BIN)

dsony: TDIR =debug-sony
#dsony: BINNAME=$(BIN)

cheat: TDIR =cheat
#cheat: BINNAME=$(BIN)

OBJDIR = out-$(TDIR)

%: OBJDIR = out-$(TDIR)

VERSION := $(shell /bin/grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
nearlylive: VERSION = $(shell date +%Y.%m.%d)

DSTAMP := $(shell date "+%Y/%m/%d@%H:%M")

debug: debug-nextdir $(DTARGETS)
all: all-nextdir $(TARGETS)
nearlylive: all-nextdir
dist: all-nextdir

TARGETDIRPRE=standard debug palm5 debug-palm5 sony debug-sony cheat
CLEANTARGETS=$(TARGETDIRPRE:%=purge-%)

$(TARGETS) $(DTARGETS): VER $(COLORTARGETS)
	$(MAKE) -f Makefile-dowork BINARY=$(BINNAME) \
		OBJDIR=out-$(TDIR) BINPRE=$(TDIR) $@

clean: clean-graphic $(CLEANTARGETS)
	rm -rf *.ro $(BIN)-*.rcp *.pdb *.zip *.gz .temp-*

clean-%:
	cd $(@:clean-%=%) && $(MAKE) clean

purge-%:
	$(MAKE) -f Makefile-dowork OBJDIR=out-$(@:purge-%=%) \
		BINARY=$(BINNAME) clean

# Build Stamping

VER: version.ro touch.ro

.temp:
	@mkdir -p .temp

version.ro: ../CHANGELOG
	@echo "VERSION 1 \"$(VERSION)\"" > version.rcp
	$(RC.rcp) version.rcp 
	@rm -f version.rcp

touch.ro: FRC
	@echo "#include \"simcity_resconsts.h\"" > touch.rcp
	@printf "STRING ID StrID_build \"$(DSTAMP)\"" >> touch.rcp
	$(RC.rcp) touch.rcp
	@rm -f touch.rcp

# Building zip/gz files
#
DDIR=PocketCity-palm_$(VERSION)

DISTFILES=../INSTALL ../CHANGELOG ../COPYING ../GAMEHINTS ../README

# This is the first target listed; hence it is the default target.
full: $(TARGETS)

dist nearlylive: all-nextdir $(NDTARGETS)
	@mkdir -p $(DDIR)
	@cp $(DISTFILES) *.pdb $(DDIR)
	@for dir in out-*; do \
		for file in $$dir/*.prc; do \
			cp $$file $(DDIR)/`echo $$file | sed -e 's/out-//' -e 's/\//-/'`; \
		done; \
	done
	@tar cvf - $(DDIR) | gzip -9c >$(DDIR).tar.gz
	@zip -9R $(DDIR).zip $(DDIR)/*
	@rm -rf $(DDIR)

# This will take care of all *.rcp
# the files must be called this:
#	*-?-?.rcp => .temp-?
#
%.ro: %.rcp
	@cols=$(word 2, $(subst -, ,$*)); \
	dens=$(word 3, $(subst -, ,$*)); \
	echo $(RC.rcp) $(@:%.ro=%.rcp) $$cols-$$dens.ro; \
	$(RC.rcp) $(@:%.ro=%.rcp) $$cols-$$dens.ro

%.rcp: $(STUBS)
	@cols=$(word 2, $(subst -, ,$*)); \
	dens=$(word 3, $(subst -, ,$*)); \
	if [ ! -d .temp-$$cols-$$dens ]; then \
		mkdir -p .temp-$$cols-$$dens; \
	fi; \
	./generatercp.sh -c $$cols -d $$dens $(STUBS) $@

# The tile databases
$(TILE)-%.pdb: PocketCity-%.ro
	$(BUILDPRC) --no-check-resources --bundle -o $@ -n $(DBNAME) \
		-c $(PROGTOKEN) -t $(COLORDBTOKEN) $*.ro

%-nextdir:
	cd graphic && $(MAKE) $(@:%-nextdir=%)

FRC:
