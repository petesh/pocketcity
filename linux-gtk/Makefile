SOURCEFILES = main.c savegame.c budget.c \
					../source/handler.c ../source/build.c ../source/disaster.c \
                    ../source/drawing.c  ../source/globals.c ../source/simulation.c
DEBUGFLAGS = -g
#DEBUGFLAGS += -a -pg

all:
	gcc -Wall $(DEBUGFLAGS) $(SOURCEFILES) -o pcity `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`
