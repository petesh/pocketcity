VERSION := $(shell grep Version ../CHANGELOG | head -1 | awk '{print $$2}')
CC = m68k-palmos-gcc
CFLAGS = -O2 -Wall -DPALM
LDLIBS =
OBJECTS = stack.o simcity.o handler.o drawing.o globals.o build.o \
	simulation.o HiRes.o savegame.o disaster.o map.o budget.o
PROGNAME = "Pocket City"
PROGTOKEN = PCit
PRCTARGETS = pcity pcity-grey pcity-color mg-pcity
DMENU = pcitymain_opt.stm

.PHONY: all debug clean cheat
.PRECIOUS: simcity-grey.stm zones-grey.stm simcity-color.stm zones-color.stm

##################
### make all #####
##################
all debug: $(addsuffix .prc,$(PRCTARGETS))
hires dhires: $(addsuffix .prc,$(addprefix sy-,$(PRCTARGETS)))

##################
### make cheat ###
##################
cheat: CFLAGS += -DCHEAT
cheat: debug

##################
### make debug ###
##################
debug: CFLAGS += -g -DDEBUG
debug: DMENU = pcitymain.stm

hires: HCFLAGS = -DSONY_CLIE

dhires: HCFLAGS = -g -DSONY_CLIE -DDEBUG
debug: DMENU = pcitymain.stm

#### the different versions
pcity-%.prc: simcity-%.stm zones-%.stm game.stm grcs.stm version.stm $(DMENU)
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def *.grc \
		temp$*/*.bin temp/*.bin

mg-pcity.prc: game.stm grcs.stm mg.stm version.stm $(DMENU)
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def *.grc \
		mg/*.bin temp/*.bin

pcity.prc: simcity.stm zones.stm game.stm grcs.stm version.stm $(DMENU)
	build-prc -o pcity.prc -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		*.grc tempbw/*.bin temp/*.bin

sy-pcity.prc: sy/ game.stm sy/syicons.stm simcity.stm zones.stm \
	sy/sy.stm version.stm $(DMENU)
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		sy/*.grc tempbw/*.bin temp/*.bin

sy-mg-pcity.prc: sy/ game.stm sy/syicons.stm simcity.stm zones.stm \
	sy/sy.stm mg.stm version.stm $(DMENU)
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		sy/*.grc mg/*.bin temp/*.bin sy/*.bin

sy-pcity-%.prc: sy/ game.stm sy/syicons.stm simcity-%.stm zones-%.stm \
	sy/sy.stm version.stm $(DMENU)
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		sy/*.grc temp$*/*.bin temp/*.bin sy/*.bin

#### binary resources
grcs.stm: simcity
	m68k-palmos-obj-res simcity
	echo touching>grcs.stm

#### resources

# for all
game.stm: game.rcp simcity_resconsts.h
	pilrc -q game.rcp temp
	echo touching>$@

version.stm: ../CHANGELOG
	@echo "VERSION 1 \"$(VERSION)\"" > version.rcp
	@pilrc -q version.rcp temp
	@echo touching>$@

pcitymain_opt.stm: pcitymain.rcp
	@sed '/@@DEBUG@@/d' pcitymain.rcp > mainmenu.rcp
	@pilrc -q mainmenu.rcp temp
	@rm -f mainmenu.rcp
	@echo touching>$@

pcitymain.stm: pcitymain.rcp
	@sed 's/@@DEBUG@@//' pcitymain.rcp > mainmenu.rcp
	@pilrc -q mainmenu.rcp temp
	@rm -f mainmenu.rcp
	@echo touching>$@

# This will take care of all *.rcp
# the files must be called this:
# *-color.rcp => tempcolor/
# *-grey.rcp  => tempgrey/
# *.rcp       => tempbw/

# simcity_resconsts.h
%.stm: %.rcp
	pilrc -q $< temp$(if $(findstring -,$*),$(word 2, $(subst -, ,$*)),bw)
	echo touching>$*.stm
####################################


#### actual source

simcity: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o simcity $(LDLIBS)

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

sy/simcity: $(SYOBJECTS)
	$(CC) $(HCFLAGS) $(CFLAGS) $(SYOBJECTS) -o $@ $(LDLIBS)

sy/simcity.o: simcity.c simcity.h ../source/handler.h ../source/ui.h \
	../source/zakdef.h

sy/%.o: ../source/%.c ../source/%.h ../source/zakdef.h ../source/ui.h
	$(CC) $(HCFLAGS) $(CFLAGS) -c $< -o $@

sy/%.o: %.c
	$(CC) $(HCFLAGS) $(CFLAGS) -c $< -o $@

sy/sy.stm: sy/simcity
	cd sy; m68k-palmos-obj-res simcity
	echo touching>sy/sy.stm

sy/syicons.stm: hricons.rcp
	pilrc -q hricons.rcp sy
	echo touching>$@

mg.stm: mg/ mg.rcp simcity_resconsts.h
	pilrc -q mg.rcp mg
	echo touching>$@

mg/:
	mkdir -p mg/
#

# dist
DDIR=pcity-palm_$(VERSION)

dist: all hires
	@mkdir -p $(DDIR)
	@cp ../Changelog ../Copying ../Gamehints ../Readme pc*.prc mg*.prc $(DDIR)
	@tar cvzf normal-$(DDIR).tar.gz $(DDIR)
	@zip -9R normal-$(DDIR).zip $(DDIR)/*
	@rm -f $(DDIR)/*
	@cp ../Changelog ../Copying ../Gamehints ../Readme sy*.prc $(DDIR)
	@tar cvzf sony-$(DDIR).tar.gz $(DDIR)
	@zip -9R sony-$(DDIR).zip $(DDIR)/*
	@rm -rf $(DDIR)

#
# make clean 
#

clean:
	rm -f *.bin *.o *.grc *.stm simcity *.prc \
		tempbw/*.bin tempcolor/*.bin temp/*.bin tempgrey/*.bin sy/* \
		mg/* *.zip *.gz
