###############################################################################
#
# Copyright (c) 2000 Palm, Inc. or its subsidiaries.
# All rights reserved.
#
# File: Makefile
#
# Release: 
#
###############################################################################

CC = m68k-palmos-gcc
CCFLAGS = -O2 -Wall -g

all: blackwhite color

blackwhite: bin.stm grcs.stm zones.stm game.stm
	build-prc pcity.prc "Pocket City" PCit *.grc tempbw/*.bin temp/*.bin
	
color:  bin-color.stm grcs.stm zones-color.stm game.stm
	build-prc pcity-color.prc "Pocket City" PCit *.grc tempcolor/*.bin temp/*.bin
    
bin.stm: simcity.rcp
	pilrc -q simcity.rcp tempbw
	echo touching>bin.stm

bin-color.stm: simcity-color.rcp
	pilrc -q simcity-color.rcp tempcolor
	echo touching>bin-color.stm

zones.stm: zones.rcp
	pilrc -q zones.rcp tempbw
	echo touching>zones.stm

game.stm: game.rcp
	pilrc -q game.rcp temp
	echo touching>game.stm

zones-color.stm: zones-color.rcp
	pilrc -q zones-color.rcp tempcolor
	echo touching>zones-color.stm

grcs.stm: simcity
	m68k-palmos-obj-res simcity
	echo touching>grcs.stm


#the linker - don't forget to add new objects here
simcity: simcity.o handler.o drawing.o globals.o build.o simulation.o disaster.o
	$(CC) $(CCFLAGS) -O1 simcity.o handler.o drawing.o globals.o build.o simulation.o disaster.o -o simcity


#objects (one for each .c file)
simcity.o: simcity.c simcity.h ../source/handler.h ../source/ui.h \
           ../source/zakdef.h
	$(CC) $(CCFLAGS) -O1 -c simcity.c -o simcity.o

handler.o: ../source/handler.c ../source/handler.h ../source/ui.h \
           ../source/zakdef.h
	$(CC) $(CCFLAGS) -O1 -c ../source/handler.c -o handler.o

disaster.o: ../source/disaster.c ../source/disaster.h ../source/ui.h \
           ../source/zakdef.h
	$(CC) $(CCFLAGS) -O1 -c ../source/disaster.c -o disaster.o

drawing.o: ../source/drawing.c ../source/drawing.h ../source/ui.h \
           ../source/zakdef.h
	$(CC) $(CCFLAGS) -O1 -c ../source/drawing.c -o drawing.o

globals.o: ../source/globals.c 
	$(CC) $(CCFLAGS) -O1 -c ../source/globals.c -o globals.o

build.o: ../source/build.c 
	$(CC) $(CCFLAGS) -O1 -c ../source/build.c -o build.o

simulation.o: ../source/simulation.c 
	$(CC) $(CCFLAGS) -O1 -c ../source/simulation.c -o simulation.o


clean:
	rm -f *.bin *.o *.grc *.stm simcity tempbw/*.bin tempcolor/*.bin temp/*.bin
