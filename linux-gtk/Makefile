SOURCEFILES = main.c ../source/handler.c ../source/build.c ../source/disaster.c \
                     ../source/drawing.c  ../source/globals.c ../source/simulation.c
all:
	gcc -Wall -g $(SOURCEFILES) -o pcity `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0 gthread-2.0`
