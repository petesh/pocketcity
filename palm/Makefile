VERSION := $(shell grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
CC = m68k-palmos-gcc
CFLAGS = -O2 -Wall -DPALM -I . -I ../source
#LDFLAGS = -T text_64k
LDLIBS =
OBJECTS = stack.o simcity.o handler.o drawing.o globals.o build.o \
	simulation.o HiRes.o savegame.o disaster.o map.o budget.o pcity-sections.o
PROGNAME = "Pocket City"
PROGTOKEN = PCit
PRCTARGETS = pcity pcity-grey pcity-color mg-pcity

SEDIS = '/@@DEBUG@@/d'
HRSED = '/@@HIRES@@/d'

DSTAMP := $(shell date "+%Y/%m/%d@%H:%M")


.PHONY: all debug clean cheat hires dhires
.PRECIOUS: simcity-grey.stm zones-grey.stm simcity-color.stm zones-color.stm

#
# make all
#

all debug: VER $(addsuffix .prc,$(PRCTARGETS))
hires dhires: VER $(addsuffix .prc,$(addprefix sy-,$(PRCTARGETS)))

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
debug dhires: SEDIS = 's/@@DEBUG@@//'
debug dhires: HRSED = 's/@@HIRES@@//'

dhires hires: HCFLAGS = -DSONY_CLIE
dhires hires: HRSED = 's/@@HIRES@@//'

#### the different versions
pcity-%.prc: simcity-%.stm zones-%.stm game.stm version.stm touch.stm \
		pcitymain.stm pcity
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		pcity temp$*/*.bin temp/*.bin

mg-pcity.prc: game.stm mg.stm version.stm touch.stm pcitymain.stm pcity
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		pcity mg/*.bin temp/*.bin

pcity.prc: simcity.stm zones.stm game.stm version.stm touch.stm pcitymain.stm pcity
	build-prc -o pcity.prc -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		pcity tempbw/*.bin temp/*.bin

sy-pcity.prc: sy/ game.stm sy/syicons.stm simcity.stm zones.stm \
		version.stm touch.stm pcitymain.stm sy/pcity
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		sy/pcity tempbw/*.bin temp/*.bin

sy-mg-pcity.prc: sy/ game.stm sy/syicons.stm simcity.stm zones.stm \
		mg.stm version.stm touch.stm pcitymain.stm sy/pcity
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		sy/pcity mg/*.bin temp/*.bin sy/*.bin

sy-pcity-%.prc: sy/ game.stm sy/syicons.stm simcity-%.stm zones-%.stm \
		version.stm touch.stm pcitymain.stm sy/pcity
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		sy/pcity temp$*/*.bin temp/*.bin sy/*.bin

pcity-sections.o: pcity-sections.s
	$(CC) -c pcity-sections.s

sy/pcity-sections.o: pcity-sections.s
	$(CC) -c pcity-sections.s -o sy/pcity-sections.o

pcity-sections.s pcity-sections.ld: pcity.def
	$(MULTIGEN) pcity.def

#### binary resources
#grcs.stm: simcity
#	@m68k-palmos-obj-res simcity
#	@echo touching>grcs.stm

#### resources

# for all

version.stm: ../CHANGELOG
	@echo "VERSION 1 \"$(VERSION)\"" > version.rcp
	@pilrc -q version.rcp temp
	@rm -f version.rcp
	@echo touching>$@

pcitymain.stm: pcitymain.rcp
	@sed -e $(SEDIS) pcitymain.rcp > mainmenu.rcp
	@pilrc -q mainmenu.rcp temp
	@rm -f mainmenu.rcp
	@echo $(SEDIS)>$@

game.stm: game.rcp simcity_resconsts.h
	@sed -e $(HRSED) game.rcp > ngame.rcp
	@pilrc -q ngame.rcp temp
	@rm -f ngame.rcp
	@echo $(HRSED)>$@

touch.stm:
	@echo "#include \"simcity_resconsts.h\"" > touch.rcp
	@printf "STRING ID StrID_build \"$(DSTAMP)\"" >> touch.rcp
	@pilrc -q touch.rcp temp
	@rm -f touch.rcp

# This will take care of all *.rcp
# the files must be called this:
# *-color.rcp => tempcolor/
# *-grey.rcp  => tempgrey/
# *.rcp       => tempbw/

# simcity_resconsts.h
%.stm: %.rcp
	@pilrc -q $< temp$(if $(findstring -,$*),$(word 2, $(subst -, ,$*)),bw)
	@echo touching>$*.stm

#
# actual source
#

pcity: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o pcity $(LDLIBS) pcity-sections.ld

simcity.o: simcity.c simcity.h ../source/handler.h ../source/ui.h \
	../source/zakdef.h simcity_resconsts.h

%.o: ../source/%.c ../source/%.h ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@
#
# Hi res objects
#

SYOBJECTS=$(addprefix sy/,$(OBJECTS))

sy/:
	@-[ -d "$@" ] || mkdir -p $@

sy/pcity: $(SYOBJECTS)
	$(CC) $(HCFLAGS) $(CFLAGS) $(LDFLAGS) $(SYOBJECTS) -o $@ $(LDLIBS) pcity-sections.ld

sy/simcity.o: simcity.c simcity.h ../source/handler.h ../source/ui.h \
	../source/zakdef.h

sy/%.o: ../source/%.c ../source/%.h ../source/zakdef.h ../source/ui.h
	$(CC) $(HCFLAGS) $(CFLAGS) -c $< -o $@

sy/%.o: %.c
	$(CC) $(HCFLAGS) $(CFLAGS) -c $< -o $@

#sy/grcs.stm: sy/simcity
#	@cd sy; m68k-palmos-obj-res simcity
#	@echo touching>sy/sy.stm

sy/syicons.stm: hricons.rcp
	@pilrc -q hricons.rcp sy
	@echo touching>$@

mg.stm: mg/ mg.rcp simcity_resconsts.h
	@pilrc -q mg.rcp mg
	@echo touching>$@

mg/:
	@mkdir -p mg/

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
	rm -f *.bin *.o *.grc *.stm simcity *.prc \
		tempbw/*.bin tempcolor/*.bin temp/*.bin tempgrey/*.bin sy/* \
		mg/* *.zip *.gz
	rm -rf mg/ sy/
