
CC = m68k-palmos-gcc
CFLAGS = -O2 -Wall -DPALM
OBJECTS = simcity.o handler.o drawing.o globals.o build.o simulation.o \
		  disaster.o savegame.o map.o budget.o
PROGNAME = "Pocket City"
PROGTOKEN = PCit
PRCTARGETS = pcity pcity-grey pcity-color

.PHONY: all debug clean cheat
.PRECIOUS: simcity-grey.stm zones-grey.stm simcity-color.stm zones-color.stm

##################
### make all #####
##################
all: $(addsuffix .prc,$(PRCTARGETS))

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


#### the different versions
pcity-%.prc: grcs.stm simcity-%.stm zones-%.stm game.stm
	build-prc -o $@ -n $(PROGNAME) -c $(PROGTOKEN) pcity.def *.grc temp$*/*.bin temp/*.bin

pcity.prc: grcs.stm simcity.stm zones.stm game.stm
	build-prc -o pcity.prc -n $(PROGNAME) -c $(PROGTOKEN) pcity.def *.grc tempbw/*.bin temp/*.bin
    

#### binary resources


grcs.stm: simcity
	m68k-palmos-obj-res simcity
	echo touching>grcs.stm

#### resources

# for all
game.stm: game.rcp
	pilrc -q game.rcp temp
	echo touching>game.stm

###################################
## This will take care of all *.rcp
## the files must be called this:
## *-color.rcp => tempcolor/
## *-grey.rcp  => tempgrey/
## *.rcp       => tempbw/

%.stm: %.rcp
	pilrc -q $< temp$(if $(findstring -,$*),$(word 2, $(subst -, ,$*)),bw)
	echo touching>$*.stm
####################################
	

#### actual source

simcity: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o simcity

simcity.o: simcity.c simcity.h ../source/handler.h ../source/ui.h ../source/zakdef.h
    
%.o: ../source/%.c ../source/%.h ../source/zakdef.h ../source/ui.h 
	$(CC) $(CFLAGS) -c $< -o $@

##################
### make clean ###
##################
clean:
	rm -f *.bin *.o *.grc *.stm simcity tempbw/*.bin tempcolor/*.bin temp/*.bin tempgrey/*.bin
