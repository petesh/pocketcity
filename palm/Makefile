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
CCFLAGS = -O2 -Wall

pcity.prc:  bin.stm grcs.stm zones.stm
	build-prc pcity.prc "Pocket City" PCit *.grc *.bin

bin.stm: simcity.rcp
	pilrc simcity.rcp
	echo touching>bin.stm

zones.stm: zones.rcp
	pilrc zones.rcp
	echo touching>zones.stm


grcs.stm: simcity
	m68k-palmos-obj-res simcity
	echo touching>grcs.stm


#the linker - don't forget to add new objects here
simcity: simcity.o handler.o drawing.o globals.o build.o simulation.o
	$(CC) $(CCFLAGS) -O1 simcity.o handler.o drawing.o globals.o build.o simulation.o -o simcity


#objects (one for each .c file)
simcity.o: simcity.c simcity.h ../source/handler.h ../source/ui.h \
           ../source/zakdef.h
	$(CC) $(CCFLAGS) -O1 -c simcity.c -o simcity.o

handler.o: ../source/handler.c ../source/handler.h ../source/ui.h \
           ../source/zakdef.h
	$(CC) $(CCFLAGS) -O1 -c ../source/handler.c -o handler.o

drawing.o: ../source/drawing.c ../source/drawing.h ../source/ui.h \
           ../source/zakdef.h
	$(CC) $(CCFLAGS) -O1 -c ../source/drawing.c -o drawing.o

globals.o: ../source/globals.c 
	$(CC) $(CCFLAGS) -O1 -c ../source/globals.c -o globals.o

build.o: ../source/build.c 
	$(CC) $(CCFLAGS) -O1 -c ../source/build.c -o build.o

simulation.o: ../source/simulation.c 
	$(CC) $(CCFLAGS) -O1 -c ../source/simulation.c -o simulation.o
