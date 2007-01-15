
include Makefile-include
include ../Makefile-settings

BIN=PocketCity
TILE=$(BIN)

NDTARGETS=palm5 lores sony
DTARGETS=dpalm5 dlores dsony cheat
LTARGETS=lpalm5 llores lsony

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

lores: TDIR =release-standard
dlores: TDIR =debug-standard
llores: TDIR =logging-standard

palm5: TDIR =release-palm5
dpalm5: TDIR =debug-palm5
lpalm5: TDIR =logging-palm5

sony: TDIR =release-sony
dsony: TDIR =debug-sony
lsony: TDIR =logging-sony

cheat: TDIR =cheat

OBJDIR = out-$(TDIR)

%: OBJDIR = out-$(TDIR)

VERSION := $(shell /bin/grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
nearlylive: VERSION = $(shell date +%Y.%m.%d)

DSTAMP := $(shell date "+%Y/%m/%d@%H:%M")

debug: debug-nextdir $(DTARGETS)
logging: debug-nextdir $(LTARGETS)
release: release-nextdir $(TARGETS)
nearlylive: all-nextdir
dist: all-nextdir

all: $(BUILD_MODE)

TARGETDIRPRE=standard debug palm5 debug-palm5 sony debug-sony cheat \
    logging logging-palm5 logging-sony

CLEANTARGETS=$(TARGETDIRPRE:%=purge-%)

$(TARGETS) $(DTARGETS) $(LTARGETS): VER $(COLORTARGETS)
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

touch.ro: *.c *.h
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

dist nearlylive: release-nextdir $(NDTARGETS)
	@mkdir -p $(DDIR)
	@cp $(DISTFILES) *.pdb $(DDIR)
	@for dir in out-release-*; do \
		for file in $$dir/*.prc; do \
			cp $$file $(DDIR)/`echo $$file | sed -e 's/out-release-//' -e 's/\//-/'`; \
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
