#ifndef INCLUDE_ZAKDEF_H
#define INCLUDE_ZAKDEF_H

#define TBMP 1415736688 // some palmOS stuff ;)

/* how often the disasters are
 * updated - in seconds
 */
#define SIM_GAME_LOOP_DISASTER  2

/* the possible errors/warnings
 */
#define ERROR_OUT_OF_MEMORY     1
#define ERROR_OUT_OF_MONEY      2
#define ERROR_FIRE_OUTBREAK     3
#define ERROR_PLANT_EXPLOSION   4
#define ERROR_MONSTER           5
#define ERROR_DRAGON            6
#define ERROR_METEOR            7

/* the settings for the speeds
 * the time is how long in seconds
 * a single game month is
 */
#define SPEED_SLOW              15
#define SPEED_MEDIUM            10
#define SPEED_FAST              5
#define SPEED_TURBO             1
#define SPEED_PAUSED            0

/* here's the meaning of the
 * bytes in WorldFlags[]
 */
#define TYPE_DIRT               0
#define ZONE_COMMERCIAL         1
#define ZONE_RESIDENTIAL        2
#define ZONE_INDUSTRIAL         3

#define TYPE_ROAD               4 // these might be used here, but
#define TYPE_POWER_LINE         5 // the graphic slot is still free for other uses
#define TYPE_WATER_PIPE         8 // and are used for the water/powerloss overlay
#define TYPE_NOT_USED           9 // at the moment

#define TYPE_POWERROAD_1        6
#define TYPE_POWERROAD_2        7

#define TYPE_TREE               21
#define TYPE_WATER              22
#define TYPE_FIRE_STATION       23
#define TYPE_POLICE_STATION     24
#define TYPE_MILITARY_BASE      25
#define TYPE_WATER_PUMP         26
#define TYPE_POWER_PLANT        60
#define TYPE_NUCLEAR_PLANT      61
#define TYPE_WASTE              62
#define TYPE_FIRE1              63
#define TYPE_FIRE2              64
#define TYPE_FIRE3              65
#define TYPE_REAL_WATER         66
#define TYPE_CRATER             67
#define TYPE_WATERROAD_1        68
#define TYPE_WATERROAD_2        69
#define TYPE_BRIDGE             81

/* defines for the BuildCount[] array
 */
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
#define COUNT_FIRE_STATIONS    11
#define COUNT_POLICE_STATIONS  12
#define COUNT_MILITARY_BASES   13
#define COUNT_WATERPIPES       14
#define COUNT_WATER_PUMPS      15


//income pr. zone/level
#define INCOME_RESIDENTIAL      25
#define INCOME_COMMERCIAL       35
#define INCOME_INDUSTRIAL       30

// for the upkeep[] array
#define UPKEEPS_TRAFFIC         0
#define UPKEEPS_POWER           1
#define UPKEEPS_DEFENCE         2

//upkeep cost pr. tile
#define UPKEEP_ROAD             2
#define UPKEEP_POWERLINE        1
#define UPKEEP_NUCLEARPLANT     500
#define UPKEEP_POWERPLANT       200
#define UPKEEP_FIRE_STATIONS    150
#define UPKEEP_POLICE_STATIONS  100
#define UPKEEP_MILITARY_BASES   500

// a very nice macro
#define WORLDPOS(x,y)		((x)+(y)*(game.mapsize))


// moveable objects
#define NUM_OF_OBJECTS          10 // max 10, or savegames for palm fail...
#define OBJ_CHANCE_OF_TURNING   5 // must be at least 3
typedef struct _moveable_object {
        unsigned short x;
        unsigned short y;
        unsigned short dir;
        unsigned short active;
} MoveableObject;

enum Objects {  OBJ_MONSTER,
                OBJ_DRAGON,
                OBJ_CHOPPER, // not done
                OBJ_SHIP,    // not done
                OBJ_TRAIN    // not done
              };

// defence units
#define NUM_OF_UNITS            10 // max 10, or savegames for palm fail...

// what each field in the objects[] are used for
#define DEF_POLICE_START        0
#define DEF_POLICE_END          2
#define DEF_FIREMEN_START       3
#define DEF_FIREMEN_END         7
#define DEF_MILITARY_START      8
#define DEF_MILITARY_END        9

typedef struct _defence_unit {
        unsigned short x;
        unsigned short y;
        unsigned short active;
        unsigned short type;
} DefenceUnit;

enum DefenceUnitTypes { DEFENCE_FIREMEN,
                        DEFENCE_POLICE,
                        DEFENCE_MILITARY
                      };


// this is the central game struct
// only one of this exists at a time
// and is called `game`
// This entire struct will be saved
// between games

typedef struct _game_struct {
    char            version[4];
    char            mapsize;
    int             visible_x;
    int             visible_y;
    int             map_xpos;
    int             map_ypos;
    int             cursor_xpos;
    int             cursor_ypos;
    long signed     credits;
    long unsigned   BuildCount[20];
    long unsigned   TimeElapsed;
    unsigned char   tax;
    unsigned char   tileSize;
    unsigned short  gameLoopSeconds;
    char            cityname[20];
    unsigned char   upkeep[3];
    unsigned char   disaster_level;
    DefenceUnit     units[10];
    MoveableObject  objects[10];
} GameStruct;

#define SAVEGAMEVERSION     "PC05"

#endif
