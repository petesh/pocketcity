#ifndef INCLUDE_ZAKDEF_H
#define INCLUDE_ZAKDEF_H

#define TBMP 1415736688

#define SIM_GAME_LOOP_DISASTER  2

#define ERROR_OUT_OF_MEMORY     1
#define ERROR_OUT_OF_MONEY      2
#define ERROR_FIRE_OUTBREAK     3
#define ERROR_PLANT_EXPLOSION   4

#define SPEED_SLOW              15
#define SPEED_MEDIUM            10
#define SPEED_FAST              5
#define SPEED_TURBO             1
#define SPEED_PAUSED            0


#define ZONE_COMMERCIAL         1
#define ZONE_RESIDENTIAL        2
#define ZONE_INDUSTRIAL         3

#define TYPE_DIRT               0
#define TYPE_FIRE1              63
#define TYPE_FIRE2              64
#define TYPE_FIRE3              65
#define TYPE_REAL_WATER         66
#define TYPE_POWER_PLANT        60
#define TYPE_NUCLEAR_PLANT      61
#define TYPE_WASTE              62
#define TYPE_WATER              22
#define TYPE_TREE               21
#define TYPE_POWER_LINE         5
#define TYPE_ROAD               4

#define COUNT_RESIDENTIAL       0
#define COUNT_COMMERCIAL        1
#define COUNT_INDUSTRIAL        2
#define COUNT_ROADS             3
#define COUNT_TREES             4
#define COUNT_WATER             5
#define COUNT_POWERLINES        6
#define COUNT_POWERPLANTS       7
#define COUNT_NUCLEARPLANTS     8
#define COUNT_WASTE             9
#define COUNT_FIRE             10

//income pr. zone/level
#define INCOME_RESIDENTIAL      25
#define INCOME_COMMERCIAL       35
#define INCOME_INDUSTRIAL       30

//upkeep cost pr. tile
#define UPKEEP_ROAD             2
#define UPKEEP_POWERLINE        1
#define UPKEEP_NUCLEARPLANT     500
#define UPKEEP_POWERPLANT       200

#define WORLDPOS(x,y)		(x+y*mapsize)


// moveable objects
#define NUM_OF_OBJECTS          1
#define OBJ_CHANCE_OF_TURNING   5 // must be at least 3
typedef struct _moveable_object {
        unsigned short x;
        unsigned short y;
        unsigned short dir;
        unsigned short active;
} MoveableObject;

enum Objects {  OBJ_MONSTER,
                OBJ_CHOPPER, // not done
                OBJ_SHIP,    // not done
                OBJ_TRAIN};  // not done

#endif
