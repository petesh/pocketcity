
PLATFORM = PALM

CC = m68k-palmos-gcc
AWK = gawk
MULTIGEN = m68k-palmos-multigen
BUILDPRC = build-prc
RC = pilrc

VERSION := $(shell grep Version ../CHANGELOG | head -1 | awk '{print $$2}')

RCFLAGS = -q

OPTFLAGS = -O2
WARNFLAGS = -Wall -Werror-implicit-function-declaration
INCLUDES = -I . -I ../source
DEFINES = -D$(PLATFORM)

CFLAGS = $(OPTFLAGS) $(WARNFLAGS) $(DEFINES) $(INCLUDES)
LDLIBS =
OBJECTS = stack.o simcity.o handler.o drawing.o globals.o build.o \
	simulation.o HiRes.o disaster.o palmutils.o savegame_be.o \
	pcity-sections.o \
	options.o map.o budget.o query.o savegame.o

BLDIR=build

debug: BLDIR=build-debug
hires: BLDIR=build-sony
dhires: BLDIR=build-debug-sony

OBJDIR = $(BLDIR)

DESTOBJS = $(OBJECTS:%=$(OBJDIR)/%)

PROGNAME = "Pocket City"
DBNAME = "Pocket City Tiles"
PROGTOKEN = PCit
# tile database type
COLORDBTOKEN = tidb
#PRCTARGETS = pcity

DEBUG=0
HIRES=0

DSTAMP := $(shell date "+%Y/%m/%d@%H:%M")

RC.rcp=$(RC) $(RCFLAGS)

# Multi language support - game-$(LANGUAGES).stm
LANGUAGES=EN

GAMESTMS=$(LANGUAGES:%=game-%.stm)

PRCTARGETS=$(LANGUAGES:%=$(OBJDIR)/pcity-%.prc)

# Resource PDBs (graphical; non-language)
COLORTYPES=bw grey color merged
COLORTARGETS=$(COLORTYPES:%=pcity-%.pdb)

.PHONY: all debug clean cheat hires dhires
.PRECIOUS: %.stm game-%.stm $(OBJDIR) temp temp-%

#
# make all
#

all debug hires dhires: VER $(PRCTARGETS) $(COLORTARGETS)

nearlylive: VERSION = $(shell date +%Y.%m.%d)
nearlylive: VER dist

VER:
	@rm -f version.rcp version.stm


#
# make cheat
#
cheat: CFLAGS += -DCHEAT
cheat: debug

##################
### make debug ###
##################

debug dhires: CFLAGS += -g -DDEBUG
debug dhires: DEBUG=1

hires dhires: CFLAGS += -DSONY_CLIE
hires dhires: HIRES=1

# the different language versions
$(OBJDIR)/pcity-%.prc: game-%.stm version.stm touch.stm $(OBJDIR)/pcity
	$(BUILDPRC) -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		$(OBJDIR)/pcity temp-$*/*.bin

# This is the color database
pcity-%.pdb: simcity-%.stm zones-%.stm
	$(BUILDPRC) -o $@ -n $(DBNAME) -c $(PROGTOKEN) -t $(COLORDBTOKEN) \
		temp-$*/*.bin

$(OBJDIR)/pcity-sections.o: $(OBJDIR)/pcity-sections.s
	$(CC) -c $< -o $@

$(OBJDIR)/pcity-sections.s $(OBJDIR)/pcity-sections.ld: pcity.def
	$(MULTIGEN) -b $(OBJDIR)/pcity-sections pcity.def

#### binary resources
#grcs.stm: simcity
#	@m68k-palmos-obj-res simcity
#	@echo touching>grcs.stm

#### resources

# for all

temp:
	@mkdir -p temp

temp-%:
	@mkdir -p temp-$*

version.stm: temp ../CHANGELOG
	@echo "VERSION 1 \"$(VERSION)\"" > version.rcp
	$(RC.rcp) version.rcp temp
	@rm -f version.rcp
	@echo touching>$@

# Language
game-%.stm: temp-% game.rcp simcity_resconsts.h
	@$(AWK) -v DEBUG=$(DEBUG) -v HIRES=$(HIRES) -f filter.awk \
		game.rcp > ngame-$*.rcp
	$(RC.rcp) -L $* ngame-$*.rcp temp-$*
	@cat ngame-$*.rcp > $@ && rm -f ngame-$*.rcp

touch.stm: temp
	@echo "#include \"simcity_resconsts.h\"" > touch.rcp
	@printf "STRING ID StrID_build \"$(DSTAMP)\"" >> touch.rcp
	$(RC.rcp) touch.rcp temp
	@rm -f touch.rcp

# This will take care of all *.rcp
# the files must be called this:
#	*-?.rcp => temp-?

# simcity_resconsts.h
%.stm: %.rcp
	@end=$(word 2, $(subst -, ,$*)); \
	if [ ! -d temp-$$end ]; then mkdir -p temp-$$end; fi; \
	echo $(RC.rcp) -L $$end $< temp-$$end; \
	$(RC.rcp) -L $$end $< temp-$$end;
	@echo touching>$@

#
# actual source
#

$(OBJDIR):
	@-[ -d "$@" ] || mkdir -p $@

$(OBJDIR)/pcity: $(OBJDIR) $(DESTOBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DESTOBJS) -o $@ $(LDLIBS) \
		$(OBJDIR)/pcity-sections.ld

$(OBJDIR)/simcity.o: simcity.c simcity.h ../source/handler.h ../source/ui.h \
	../source/zakdef.h simcity_resconsts.h

$(OBJDIR)/%.o: ../source/%.c ../source/%.h ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#
# dist
#

DDIR=pcity-palm_$(VERSION)

DISTFILES=../INSTALL ../CHANGELOG ../COPYING ../GAMEHINTS ../README

dist: all hires
	@mkdir -p $(DDIR)
	@cp $(DISTFILES) pc*.prc mg*.prc $(DDIR)
	@tar cvf - $(DDIR) | gzip -9c >$(DDIR)-normal.tar.gz
	@zip -9R $(DDIR)-normal.zip $(DDIR)/*
	@rm -f $(DDIR)/*
	@cp $(DISTFILES) sy*.prc $(DDIR)
	@tar cvf - $(DDIR) | gzip -9c >$(DDIR)-sony.tar.gz
	@zip -9R $(DDIR)-sony.zip $(DDIR)/*
	@rm -rf $(DDIR)

#
# make clean 
#

clean:
	rm -f *.stm *.pdb temp*/*.bin build*/* *.zip *.gz
	rm -rf temp* build*
