/*! \file
 * \brief the core set of definitions for the game.
 *
 * This consists of all the important types and structures that are
 * needed to make the game function correctly.
 */

#if !defined(_ZAKDEF_H_)
#define	_ZAKDEF_H_

#include <appconfig.h>

/*! \brief how often the disasters are updated - in seconds */
#define	SIM_GAME_LOOP_DISASTER  2

/*!
 * \brief the possible errors/warnings.
 *
 * Make sure they don't intersect as the overlap in the warning dialogs.
 */
typedef enum erdiType_en {
	enSTART = 0, /*!< starting guard for errors */
	enOutOfMemory, /*!< out of memory error */
	enOutOfMoney, /*!< out of money */
	enEND, /*!< ending guard for errors */
	diSTART = 79, /*!< starting disaster guard */
	diFireOutbreak, /*!< a fire disaster */
	diPlantExplosion, /*!< a power plant explosion */
	diMonster, /*!< a monster */
	diDragon, /*!< a dragon */
	diMeteor, /*!< a meteor */
	diEND /*!< ending guard for disasters */
} erdiType;

/*! \brief the number of seconds in a month at slow speed */
#define	SPEED_SLOW	15
/*! \brief the number of seconds in a month at medium speed */
#define	SPEED_MEDIUM	10
/*! \brief the number of seconds in a month at fast speed */
#define	SPEED_FAST	5
/*! \brief the number of seconds in a month at turbo speed */
#define	SPEED_TURBO	1
/*! \brief the number of seconds in a month at pause speed */
#define	SPEED_PAUSED	0

/*
 * here's the meaning of the
 * bytes in WorldFlags[]
 */
/*! \brief the mapping of dirt to the world map */
#define	TYPE_DIRT		0
/*! \brief unoccupied commercial zone mapping */
#define	ZONE_COMMERCIAL		1
/*! \brief unoccupied residential zone mapping */
#define	ZONE_RESIDENTIAL	2
/*! \brief unoccupied commercial zone mapping */
#define	ZONE_INDUSTRIAL		3

/*! \brief zone identification for scoring */
typedef enum {
	ztWhat = 0, /*!< Unknown Zone */
	ztCommercial, /*!< Commercial Zone */
	ztResidential, /*!< Residential Zone */
	ztIndustrial /*!< Industrial Zone */
} zoneType;

/*! \brief directions for checking power distribution routines */
typedef enum {
	dtUp = 0, /*!< Up */
	dtRight, /*!< right */
	dtDown, /*!< down */
	dtLeft /*!< left */
} dirType;

/*! \brief the zone types for querying */
struct zoneTypeValue {
	char	description[255]; /*!< The description of the zone */
	UInt8	value;	/*!< The Value of the zone */
	UInt8	pollution; /*!< Pollution level of the zone */
	UInt8	crime; /*!< Crime level of the zone */
};

/*
 * these might be used here, but
 * the graphic slot is still free for other
 * uses and are used for the water/powerloss
 * overlay at the moment
 */
/*! \brief map item is a road */
#define	TYPE_ROAD		4
/*! \brief map item is a power line */
#define	TYPE_POWER_LINE		5
/*! \brief map item is power line intersecting a road (horizontal) */
#define	TYPE_POWERROAD_1	6
/*! \brief map item is power line intersecting a road (vertical) */
#define	TYPE_POWERROAD_2	7
/*! \brief map item is water pipe */
#define	TYPE_WATER_PIPE		8
/*! \brief map item is unused */
#define	TYPE_NOT_USED		9

/*! \brief map item is a tree */
#define	TYPE_TREE		21
/*! \brief map item is lake water */
#define	TYPE_WATER		22
/*! \brief map item is a fire station */
#define	TYPE_FIRE_STATION	23
/*! \brief map item is a police station */
#define	TYPE_POLICE_STATION	24
/*! \brief map item is a military base */
#define	TYPE_MILITARY_BASE	25
/*! \brief map item is a water pump */
#define	TYPE_WATER_PUMP		26
/* 27 .. 30 are unused, but match CarryWater! Note this! */
/* 30..39 commercial */
/*! \brief minimum valued commercial zone */
#define	TYPE_COMMERCIAL_MIN	30
/*! \brief maximum valued commercial zone */
#define	TYPE_COMMERCIAL_MAX	39
/* 40..49 residential */
/*! \brief minimum valued residential zone */
#define	TYPE_RESIDENTIAL_MIN	40
/*! \brief maximum valued residential zone */
#define	TYPE_RESIDENTIAL_MAX	49
/* 50..59 industrial */
/*! \brief minimum valued industrial zone */
#define	TYPE_INDUSTRIAL_MIN	50
/*! \brief maximum valued industrial zone */
#define	TYPE_INDUSTRIAL_MAX	59

/*! \brief a single zoned coal power plant */
#define	TYPE_POWER_PLANT	60
/*! \brief a single zoned nuclear power plant */
#define	TYPE_NUCLEAR_PLANT	61
/*! \brief wasteland */
#define	TYPE_WASTE		62
/*! \brief fire type (1) */
#define	TYPE_FIRE1		63
/*! \brief fire type (2) */
#define	TYPE_FIRE2		64
/*! \brief fire type (3) */
#define	TYPE_FIRE3		65
/*! \brief Natural water */
#define	TYPE_REAL_WATER		66
/*! \brief crater - caused by meteor */
#define	TYPE_CRATER		67
/*! \brief bridge over water (horizontal?) */
#define	TYPE_WATERROAD_1	68
/*! \brief bridge over water (vertical?) */
#define	TYPE_WATERROAD_2	69

/*! \brief bridge (direction?) */
#define	TYPE_BRIDGE		81

/*! \brief elements for the BuildCount[] array */
typedef enum {
	bc_residential = 0, /*!< count of residential units */
	bc_commercial, /*!< count of commercial units */
	bc_industrial, /*!< count of industrial units */
	bc_roads, /*!< count of roads */
	bc_trees, /*!< count of trees */
	bc_water, /*!< count of water */
	bc_coalplants, /*!< count of coal power plants */
	bc_nuclearplants, /*!< count of nuclear power plants */
	bc_powerlines, /*!< count of power lines */
	bc_waste, /*!< count of wasteland zones */
	bc_fire, /*!< count of fire elements */
	bc_fire_stations, /*!< count of fire station */
	bc_police_stations, /*!< count of police stations */
	bc_military_bases, /*!< count of military bases */
	bc_waterpipes, /*!< count of water pipes */
	bc_waterpumps /*!< count of water pumps */
} BuildCount;

/* Supply units per plant */
/*! \brief number of power units supplied by a coal power plant */
#define	SUPPLY_POWER_PLANT	100
/*! \brief number of power units supplied by a nuclear power plant */
#define	SUPPLY_NUCLEAR_PLANT	300
/*! \brief number of zones supplied by a water pump */
#define	SUPPLY_WATER_PUMP	200

/* income per zone/level */
/*! \brief income from a residential zone per value level */
#define	INCOME_RESIDENTIAL	25
/*! \brief income from a commercial zone per value level */
#define	INCOME_COMMERCIAL	35
/*! \brief income from an industrial zone per value level */
#define	INCOME_INDUSTRIAL	30

/* for the upkeep[] array */

/*!
 * \brief the entries in the upkeep array.
 *
 * If you add something here you will have to add it to the savegame structure
 * so it will require revving up the savegame version.
 */
typedef enum {
	ue_traffic = 0, /*!< Upkeep percentage entry for the traffic */
	ue_power, /*!< Upkeep percentage entry for the power */
	ue_defense, /*!< Upkeep percentage entry for the defence */
	ue_tail /*!< Tail ender, for allocating the array in game structure */
} UpkeepEntries;

/* upkeep cost per tile */

/*! \brief upkeep cost of a bridge */
#define	UPKEEP_BRIDGE		10
/*! \brief upkeep cost of a road */
#define	UPKEEP_ROAD		2
/*! \brief upkeep cost of a powerline */
#define	UPKEEP_POWERLINE	1
/*! \brief upkeep cost of a coal power plant */
#define	UPKEEP_NUCLEARPLANT	500
/*! \brief upkeep cost of a nuclear power plant */
#define	UPKEEP_POWERPLANT	200
/*! \brief upkeep cost of a fire station */
#define	UPKEEP_FIRE_STATIONS	150
/*! \brief upkeep cost of a police station */
#define	UPKEEP_POLICE_STATIONS  100
/*! \brief upkeep cost of a military base */
#define	UPKEEP_MILITARY_BASES   500

/*!
 * \brief the number of objects that may be on the screen at a time.
 *
 * Currently defined as a fixed count, if you exceed this value then it
 * will cause an overflow of an array in the savegame structure
 */
#define	NUM_OF_OBJECTS		10
/* must be at least 3 */

/*! \brief Chance of object turning either clockwise or anti-clockwise */
#define	OBJ_CHANCE_OF_TURNING	5

/*! \brief a moveable item on the map */
typedef struct _moveable_object {
	UInt16 x; /*!< x position on the map */
	UInt16 y; /*!< y position on the map */
	UInt16 dir; /*!< direction object is facing */
	UInt16 active; /*!< object is active */
} MoveableObject;

/*! \brief type of object */
typedef enum {
	obj_monster = 0, /*!< 'zilla */
	obj_dragon, /*!< Fire dragon */
	obj_chopper, /*!< Helicopter */
	obj_ship, /*!< a boat to wander up & down the river */
	obj_train /*!< a train */
} Objects;

/*!
 * \brief the number of defence units that may be on the screen at a time.
 *
 * Currently defined as a fixed count, if you exceed this value then it
 * will cause an overflow of an array in the savegame structure
 */
#define	NUM_OF_UNITS		10

/* what each field in the objects[] are used for */
/*! \brief index in object array that police units start from */
#define	DEF_POLICE_START	0
/*! \brief index in object array that police units end at */
#define	DEF_POLICE_END		2
/*! \brief index in object array that fireman units start from */
#define	DEF_FIREMEN_START	3
/*! \brief index in object array that fireman units end at */
#define	DEF_FIREMEN_END		7
/*! \brief index in object array that military units start from */
#define	DEF_MILITARY_START	8
/*! \brief index in object array that military units end at */
#define	DEF_MILITARY_END	9

/* Update codes for grids */
/*! \brief code to say to update the power grid */
#define	GRID_POWER		1
/*! \brief code to say to update the water grid */
#define	GRID_WATER		2
/*! \brief code to say to update both water and power grids */
#define	GRID_ALL		(GRID_POWER|GRID_WATER)

/*! \brief The type of a defence unit */
typedef enum DefenceUnitTypes {
	DuFireman = 0, DuPolice, DuMilitary
} DefenceUnitTypes;

/*! \brief a defence unit structure */
typedef struct _defence_unit {
	Int16	x;	/*!< x position of the defence unit */
	Int16	y;	/*!< y position of the defence unit */
	UInt16	active; /*!< flag indicating this unit is active */
	DefenceUnitTypes type; /*!< the defence unit type */
} DefenceUnit;

/*! \brief the maximum length of the city name string */
#define	CITYNAMELEN 20

/*!
 * \brief the central game structure.
 *
 * This is the central game struct only one of this exists at a time
 * and is called 'game'. This entire struct will be saved between games.
 *
 * Anything with an underscore will be removed later on.
 */

typedef struct _game_struct05 {
	Int8		version[4]; /*!< Version code of savegame */
	UInt8		mapsize;	/*!< The size of each axis of the map */
	Int16		_visible_x;	/*!< deprecated */
	Int16		_visible_y;	/*!< deprecated */
	Int16		map_xpos;	/*!< start visible x axis */
	Int16		map_ypos;	/*!< start visible y axis */
	Int16		_cursor_xpos;	/*!< Deprecated */
	Int16		_cursor_ypos;	/*!< Deprecated */
	Int32		credits;	/*!< amount of money */
	UInt32		_BuildCount[20]; /*!< Deprecated */
	UInt32		TimeElapsed;	/*!< Number of months past 00 */
	UInt8		tax;		/*!< Tax rate */
	UInt8		auto_bulldoze;	/*!< do we auto-bulldoze?*/
	UInt16		gameLoopSeconds; /*!< real seconds per game month */
	Char		cityname[CITYNAMELEN]; /*!< Name of city */
	UInt8		upkeep[ue_tail];	/*!< upkeep %ages for bits */
	UInt8		diff_disaster;	/*!< rate of disasters */
	DefenceUnit	units[NUM_OF_UNITS];	/*!< Defence Units */
	MoveableObject  objects[NUM_OF_OBJECTS]; /*!< Special objects */
} GameStruct05;

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

/*
 * Followed by:
 * Map
 * Grid0 .. Gridn
 */

/*! \brief currently supported save game version */
typedef GameStruct05 GameStruct;

/* crap! */
typedef struct _psuCount {
	struct _psuCount *next;
	int gridType;
} psuCount;

/*! \brief volatile game structure
 *
 * items that are needed during the execution of the game, but not needed
 * for persistence.
 */
typedef struct _vgame_struct {
	UInt16		mapmul;	/*!< x*y */
	int		gridsToUpdate; /*!< grids to update next update loop */
	long unsigned	BuildCount[bc_waterpumps]; /*!< count of elements */
	unsigned char	tileSize;	/*!< size of a tile */
	unsigned short	oldLoopSeconds;	/*!< last selected speed - for pause */
	int		visible_x;	/*!< visible tiles on the X */
	int		visible_y;	/*!< visible tiles on the y */
	int		cursor_xpos;	/*!< cursor ?? */
	int		cursor_ypos;	/*!< cursor ?? */
} vGameStruct;

/*!
 * \brief appliation configuration.
 *
 * This contains configuration information for an application; not for an
 * individual game.
 */
typedef struct _appConfig_01 {
	int	version;	/*!< version of configuration information */
	PlatformAppConfig   pc; /*!< platform specific configuration */
} appConfig_01_t;

/*! \brief currently used application conofiguration version */
typedef appConfig_01_t AppConfig_t;

/*! \brief version number of the configuration file */
#define	CONFIG_VERSION 1

/*! \brief save game version */
#define	SAVEGAMEVERSION	 "PC05"

/*! \brief get the map size */
#define	GetMapSize() (game.mapsize)
/*!
 * \brief set the map size
 *
 * Does not allocate any extra memory.
 * \param x the new map size
 */
#define	SetMapSize(x) { game.mapsize = (x); \
	vgame.mapmul = game.mapsize * game.mapsize; \
}

/*! \brief get the map array size */
#define	GetMapMul() (vgame.mapmul)
/*!
 * \brief add a grid to be updated
 * \param T the grid to add
 */
#define	AddGridUpdate(T)		(vgame.gridsToUpdate |= T)
/*!
 * \brief Check if a grid need updating
 * \param T the grid to check
 * \return Whether the grid need updating
 */
#define	NeedsUpdate(T)		  (vgame.gridsToUpdate | T)
/*!
 * \brief Clear the need to update a sertain grid.
 * \param T the grid to clear
 */
#define	ClearUpdate(T)		  (vgame.gridsToUpdate &= ~T)

/*!
 * \brief get the position of a map location in the world array
 * \param x the x position
 * \param y the y position
 * \return the position in the array
 */
#define	WORLDPOS(x, y)	((x) + (y) * (GetMapSize()))

/*! \brief save the current speed, and change the speed to paused */
#define	SaveSpeed()			 { \
	vgame.oldLoopSeconds = game.gameLoopSeconds; \
	game.gameLoopSeconds = SPEED_PAUSED; \
}

/*! \brief restore the saved game speed */
#define	RestoreSpeed()		  { \
	game.gameLoopSeconds = vgame.oldLoopSeconds; \
}

#endif /* _ZAKDEF_H */
