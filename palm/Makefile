
CC = m68k-palmos-gcc
CFLAGS = -O2 -Wall -DPALM
OBJECTS = simcity.o handler.o drawing.o globals.o build.o simulation.o \
	HiRes.o savegame.o disaster.o map.o budget.o
PROGNAME = "Pocket City"
PROGTOKEN = PCit
PRCTARGETS = pcity pcity-grey pcity-color

.PHONY: all debug clean cheat
.PRECIOUS: simcity-grey.stm zones-grey.stm simcity-color.stm zones-color.stm

##################
### make all #####
##################
all: $(addsuffix .prc,$(PRCTARGETS))

hires dhires: all hr-pcity.prc

##################
### make cheat ###
##################
cheat: CFLAGS += -DCHEAT
cheat: debug

##################
### make debug ###
##################
debug: CFLAGS += -g -DDEBUG
debug: all

hires: HCFLAGS = -DSONY_CLIE
hires: all

dhires: HCFLAGS = -g -DSONY_CLIE -DDEBUG
dhires: all

#### the different versions
pcity-%.prc: grcs.stm simcity-%.stm zones-%.stm game.stm
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def *.grc \
		temp$*/*.bin temp/*.bin

pcity.prc: grcs.stm simcity.stm zones.stm game.stm
	build-prc -o pcity.prc -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		*.grc tempbw/*.bin temp/*.bin

hr-pcity.prc: hr/ hr/hr.stm zones-color.stm game.stm hricons.stm
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def \
		hr/*.grc tempcolor/*.bin temp/*.bin hr/*.bin

#### binary resources
grcs.stm: simcity
	m68k-palmos-obj-res simcity
	echo touching>grcs.stm

#### resources

# for all
game.stm: game.rcp
	pilrc -q game.rcp temp
	echo touching>$@

#
# This will take care of all *.rcp
# the files must be called this:
# *-color.rcp => tempcolor/
# *-grey.rcp  => tempgrey/
# *.rcp       => tempbw/

%.stm: %.rcp
	pilrc -q $< temp$(if $(findstring -,$*),$(word 2, $(subst -, ,$*)),bw)
	echo touching>$*.stm
####################################


#### actual source

simcity: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o simcity

simcity.o: simcity.c simcity.h ../source/handler.h ../source/ui.h \
	../source/zakdef.h

%.o: ../source/%.c ../source/%.h ../source/zakdef.h ../source/ui.h
	$(CC) $(CFLAGS) -c $< -o $@
#
# Hi res objects
#

HROBJECTS=$(addprefix hr/,$(OBJECTS))

hr/:
	@-[ -d "$@" ] || mkdir -p $@

hr/simcity: $(HROBJECTS)
	$(CC) $(HCFLAGS) $(CFLAGS) $(HROBJECTS) -o $@

hr/simcity.o: simcity.c simcity.h ../source/handler.h ../source/ui.h \
	../source/zakdef.h

hr/%.o: ../source/%.c ../source/%.h ../source/zakdef.h ../source/ui.h
	$(CC) $(HCFLAGS) $(CFLAGS) -c $< -o $@

hr/%.o: %.c
	$(CC) $(HCFLAGS) $(CFLAGS) -c $< -o $@

hr/hr.stm: hr/simcity
	cd hr; m68k-palmos-obj-res simcity
	echo touching>hr.stm

hricons.stm: hricons.rcp
	pilrc -q hricons.rcp hr
	echo touching>$@

#
# make clean 
#

clean:
	rm -f *.bin *.o *.grc *.stm simcity *.prc \
		tempbw/*.bin tempcolor/*.bin temp/*.bin tempgrey/*.bin hr/*
