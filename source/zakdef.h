#if !defined(_ZAKDEF_H_)
#define	_ZAKDEF_H_

#include <appconfig.h>

/*
 * how often the disasters are
 * updated - in seconds
 */
#define	SIM_GAME_LOOP_DISASTER  2

/*
 * the possible errors/warnings
 * Make Sure they don't intersect, there's one place where they meet
 */
typedef enum erdiType_en {
	enSTART = 0,
	enOutOfMemory,
	enOutOfMoney,
	enEND,
	diSTART = 79,
	diFireOutbreak,
	diPlantExplosion,
	diMonster,
	diDragon,
	diMeteor,
	diEND
} erdiType;

/*
 * the settings for the speeds
 * the time is how long in seconds
 * a single game month is
 */
#define	SPEED_SLOW	15
#define	SPEED_MEDIUM	10
#define	SPEED_FAST	5
#define	SPEED_TURBO	1
#define	SPEED_PAUSED	0

/*
 * here's the meaning of the
 * bytes in WorldFlags[]
 */
#define	TYPE_DIRT		0
#define	ZONE_COMMERCIAL		1
#define	ZONE_RESIDENTIAL	2
#define	ZONE_INDUSTRIAL		3

typedef enum {
	ztWhat = 0,
	ztCommercial,
	ztResidential,
	ztIndustrial
} zoneType;

typedef enum {
	dtUp = 0,
	dtRight,
	dtDown,
	dtLeft
} dirType;

struct zoneTypeValue {
	char		description[255];
	UInt8	   value;
	UInt8	   pollution;
	UInt8	   crime;
};

/*
 * these might be used here, but
 * the graphic slot is still free for other
 * uses and are used for the water/powerloss
 * overlay at the moment
 */
#define	TYPE_ROAD			   4
#define	TYPE_POWER_LINE		 5
#define	TYPE_POWERROAD_1		6
#define	TYPE_POWERROAD_2		7
#define	TYPE_WATER_PIPE		 8
#define	TYPE_NOT_USED		   9

#define	TYPE_TREE			   21
#define	TYPE_WATER			  22
#define	TYPE_FIRE_STATION	   23
#define	TYPE_POLICE_STATION	 24
#define	TYPE_MILITARY_BASE	  25
#define	TYPE_WATER_PUMP		 26
/* 27 .. 30 are unused, but match CarryWater! Note this! */
/* 30..39 commercial */
#define	TYPE_COMMERCIAL_MIN	 30
#define	TYPE_COMMERCIAL_MAX	 39
/* 40..49 residential */
#define	TYPE_RESIDENTIAL_MIN	40
#define	TYPE_RESIDENTIAL_MAX	49
/* 50..59 industrial */
#define	TYPE_INDUSTRIAL_MIN	 50
#define	TYPE_INDUSTRIAL_MAX	 59

#define	TYPE_POWER_PLANT		60
#define	TYPE_NUCLEAR_PLANT	  61
#define	TYPE_WASTE			  62
#define	TYPE_FIRE1			  63
#define	TYPE_FIRE2			  64
#define	TYPE_FIRE3			  65
#define	TYPE_REAL_WATER		 66
#define	TYPE_CRATER			 67
#define	TYPE_WATERROAD_1		68
#define	TYPE_WATERROAD_2		69
#define	TYPE_BRIDGE			 81

/* defines for the BuildCount[] array */
#define	COUNT_RESIDENTIAL	0
#define	COUNT_COMMERCIAL	1
#define	COUNT_INDUSTRIAL	2
#define	COUNT_ROADS		3
#define	COUNT_TREES		4
#define	COUNT_WATER		5
#define	COUNT_POWERLINES	6
#define	COUNT_POWERPLANTS	7
#define	COUNT_NUCLEARPLANTS	8
#define	COUNT_WASTE		9
#define	COUNT_FIRE		10
#define	COUNT_FIRE_STATIONS	11
#define	COUNT_POLICE_STATIONS	12
#define	COUNT_MILITARY_BASES	13
#define	COUNT_WATERPIPES	14
#define	COUNT_WATER_PUMPS	15

/* Supply units per plant */
#define	SUPPLY_POWER_PLANT	  100
#define	SUPPLY_NUCLEAR_PLANT	300
#define	SUPPLY_WATER_PUMP	   200

/* income per zone/level */
#define	INCOME_RESIDENTIAL	  25
#define	INCOME_COMMERCIAL	   35
#define	INCOME_INDUSTRIAL	   30

/* for the upkeep[] array */
#define	UPKEEPS_TRAFFIC		 0
#define	UPKEEPS_POWER		   1
#define	UPKEEPS_DEFENCE		 2

/* upkeep cost per tile */
#define	UPKEEP_ROAD			 2
#define	UPKEEP_POWERLINE		1
#define	UPKEEP_NUCLEARPLANT	 500
#define	UPKEEP_POWERPLANT	   200
#define	UPKEEP_FIRE_STATIONS	150
#define	UPKEEP_POLICE_STATIONS  100
#define	UPKEEP_MILITARY_BASES   500

/* moveable objects */
/* max 10, or savegames for palm fail... */
#define	NUM_OF_OBJECTS		  10
/* must be at least 3 */
#define	OBJ_CHANCE_OF_TURNING   5
typedef struct _moveable_object {
		UInt16 x;
		UInt16 y;
		UInt16 dir;
		UInt16 active;
} MoveableObject;

enum Objects {
	OBJ_MONSTER = 0,
	OBJ_DRAGON,
	OBJ_CHOPPER, /* not done */
	OBJ_SHIP,	/* not done */
	OBJ_TRAIN	/* not done */
};

/* defence units */
/* max 10, or savegames for palm fail... */
#define	NUM_OF_UNITS			10

/* what each field in the objects[] are used for */
#define	DEF_POLICE_START		0
#define	DEF_POLICE_END		  2
#define	DEF_FIREMEN_START	   3
#define	DEF_FIREMEN_END		 7
#define	DEF_MILITARY_START	  8
#define	DEF_MILITARY_END		9

/* Update codes for grids */
#define	GRID_POWER			  1
#define	GRID_WATER			  2
#define	GRID_ALL				(GRID_POWER|GRID_WATER)

typedef struct _defence_unit {
		UInt16 x;
		UInt16 y;
		UInt16 active;
		UInt16 type;
} DefenceUnit;

enum DefenceUnitTypes {
	DuFireman = 0, DuPolice, DuMilitary
};

#define	CITYNAMELEN 20

/*
 * this is the central game struct only one of this exists at a time
 * and is called 'game'. This entire struct will be saved between games.
 * Anything with an underscore will be removed later on.
 */

typedef struct _game_struct05 {
	Int8		version[4];
	UInt8		mapsize;	/* The size of each axis of the map */
	Int16		_visible_x;	/* deprecated */
	Int16		_visible_y;	/* deprecated */
	Int16		map_xpos;	/* start visible x axis */
	Int16		map_ypos;	/* start visible y axis */
	Int16		_cursor_xpos;	/* Deprecated */
	Int16		_cursor_ypos;	/* Deprecated */
	Int32		credits;	/* Show me the money */
	UInt32		_BuildCount[20]; /* Deprecated */
	UInt32		TimeElapsed;	/* Number of months past 00 */
	UInt8		tax;		/* Tax rate */
	UInt8		auto_bulldoze;	/* was _tilesize ... Deprecated */
	UInt16		gameLoopSeconds; /* Speed */
	Int8		cityname[CITYNAMELEN]; /* Name of city */
	UInt8		upkeep[3];	/* upkeep %ages for bits */
	UInt8		diff_disaster;	/* rate of disasters */
	DefenceUnit	units[10];	/* Defence Units */
	MoveableObject  objects[10];	/* Special objects */
} GameStruct05;

/* Desired new version */
typedef struct _game_struct06 {
	Int8		version[4];	/* version of game */
	UInt8		mapsize;	/* size of map, it's a square */
	Int8		bigendian;	/* Do I really need this? */
	Int8		map_xpos;	/* position on screen that's visible */
	Int8		map_ypos;	/* Position on screen that's visible */
	Int32		credits;	/* amount of money we've got. */
	UInt32		TimeElapsed;	/* number of months since 00 */
	UInt8		tax;		/* tax rate */
	UInt8		gameLoopSeconds; /* speed of game */
	UInt8		diff_disaster;  /* merge of difficulty and disaster */
	UInt8		auto_bulldoze;  /* are we auto-bulldozing */
	Int8		cityname[20];   /* Name of city */
	/* howsat ? */
	UInt16		defenceUnitCount;
	UInt16		moveableObjectCount;
	/* DefenceUnit		*units; */
	/* MoveableObject	 *objects; */
} GameStruct06;

typedef GameStruct05 GameStruct;

typedef struct _psuCount {
	struct _psuCount *next;
	int gridType;
} psuCount;

typedef struct _vgame_struct {
	UInt16		mapmul;
	int		gridsToUpdate;
	long unsigned	BuildCount[20];	/* count of elements */
	unsigned char	tileSize;	/* for math */
	unsigned short	oldLoopSeconds;	/* Speed... for pause toggles */
	int		visible_x;	/* visible tiles on the X */
	int		visible_y;	/* visible tiles on the y */
	int		cursor_xpos;	/* cursor ?? */
	int		cursor_ypos;	/* cursor ?? */
} vGameStruct;

/* Application Configuration ... not per-game settings */
typedef struct _appConfig_01 {
	int	version;		/* version */
	PlatformAppConfig   pc;
} appConfig_01_t;

typedef appConfig_01_t AppConfig_t;

#define	CONFIG_VERSION 1

#define	SAVEGAMEVERSION	 "PC05"

#define	GetMapSize() (game.mapsize)
#define	SetMapSize(x) { game.mapsize = (x); \
	vgame.mapmul = game.mapsize * game.mapsize; \
}
#define	GetMapMul() (vgame.mapmul)
#define	AddGridUpdate(T)		(vgame.gridsToUpdate |= T)
#define	NeedsUpdate(T)		  (vgame.gridsToUpdate | T)
#define	ClearUpdate(T)		  (vgame.gridsToUpdate &= ~T)

/* a very nice macro */
#define	WORLDPOS(x, y)	((x) + (y) * (GetMapSize()))

#define	SaveSpeed()			 { \
	vgame.oldLoopSeconds = game.gameLoopSeconds; \
	game.gameLoopSeconds = SPEED_PAUSED; \
}

#define	RestoreSpeed()		  { \
	game.gameLoopSeconds = vgame.oldLoopSeconds; \
}

#endif /* _ZAKDEF_H */
