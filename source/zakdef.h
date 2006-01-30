/*!
 * \file
 * \brief the core set of definitions for the game.
 *
 * This consists of all the important types and structures that are
 * needed to make the game function correctly.
 */

#if !defined(_ZAKDEF_H_)
#define	_ZAKDEF_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <appconfig.h>
#include <tileheader.h>
#include <stack.h>

#define	PACKAGE	"pocketcity"

/*! \brief the number of statistics held for a year/decade */
#define	STATS_PER	4

/*! \brief number of years/decades kept */
#define	STATS_COUNT	10

/*! \brief number of entries in array */
#define	STAT_ENTRIES	(STATS_PER * STATS_COUNT)

#define	MAX_UINT16	(~(UInt16)0)
/*! \brief value for normalizing the cashflow within the range */
#define	OFFSET_FOR_CASHFLOW_BC	(1UL<<31)
/*! \brief this offset is 1/2 the value of an unsigned int16 */
#define	OFFSET_FOR_CASHFLOW_STAT	((~(Uint16)0) >> 1)

/*! \brief this is the mask for setting the cashflow value */
#define	CASHFLOW_STATMASK	(0xffff)

/*! \brief how often the disasters are updated - in seconds */
#define	SIM_GAME_LOOP_DISASTER  2

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

/*
 * This begins the description of how the tile/status system works in practice.
 *
 * Zones are assigned a base 'type' which determines their basic make-up. Some
 * tiles can possess no more state than 'it is'. That means that the value
 * of the tile at that location maps onto the underlying zone in a one-to-one
 * fashion. The value may not actually map to the tile in question.
 *
 * This is not the case for most other tiles. These tiles possess a 'value'
 * which is a value from 0-16. This is actually 4 sets of 4 values. Each of
 * the sets corresponds to the differing densities, each of the values to the
 * corresponding 'value' of the zone.
 *
 * Getting the icons is accomplished using the 'GetSpecialGraphic()' call which
 * is in the drawing.c file.
 *
 * All the definitions below are intended to provide the backing definitions
 * that actually are used in the program.
 *
 * Anything beginning with Z_ is a zone entry
 *
 * Everything has been moved to the generated tileheader.h file
 */

/*! \brief the number of tiles that are stored laterally on a 'tilestripe' */
#define	HORIZONTAL_TILESIZE	32

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
#define	GRID_POWER		(UInt8)1
/*! \brief code to say to update the water grid */
#define	GRID_WATER		(UInt8)2
/*! \brief code to say to update both water and power grids */
#define	GRID_ALL		(GRID_POWER|GRID_WATER)

/*! \brief the maximum length of the city name string */
#define	CITYNAMELEN 20

/*! \brief version number of the configuration file */
#define	CONFIG_VERSION 1

/*! \brief save game version */
#define	SAVEGAMEVERSION	 "PC07"

#define	GG	game
/*! \brief get the map size */
#define	getMapWidth() (GG.mapx)

/*! \brief get the map height */
#define	getMapHeight() (GG.mapy)

/*!
 * \brief set the map variables for the vgame structure
 * \param X the size on the X axis
 * \param Y the size on the Y axis
 */
#define	setMapVariables(X, Y)	{ \
	GG.mapx = (X); \
	GG.mapy = (Y); \
	vgame.mapmul = (UInt16)GG.mapx * GG.mapy; \
}

/*! \brief get the map array size */
#define	MapMul() (vgame.mapmul)

/*! \brief get the world pointer size .. based on map */
#define	WorldSize() (vgame.mapmul)

/*!
 * \brief add a grid to be updated
 * \param T the grid to add
 */
#define	AddGridUpdate(T)		(GG.gridsToUpdate |= (T))
/*!
 * \brief Check if a grid need updating
 * \param T the grid to check
 * \return whether the grid need updating
 */
#define	NeedsUpdate(T)		  (GG.gridsToUpdate & (T))
/*!
 * \brief Clear the need to update a sertain grid.
 * \param T the grid to clear
 */
#define	ClearUpdate(T)		  (GG.gridsToUpdate &= (UInt8)(~(UInt8)(T)))

/*!
 * \brief get the position of a map location in the world array
 * \param x the x position
 * \param y the y position
 * \return the position in the array
 */
#define	WORLDPOS(x, y)	((x) + (y) * (getMapWidth()))

/*! \brief save the current speed, and change the speed to paused */
#define	SaveSpeed()			 { \
	vgame.oldLoopSeconds = GG.gameLoopSeconds; \
	GG.gameLoopSeconds = SPEED_PAUSED; \
}

/*! \brief restore the saved game speed */
#define	RestoreSpeed()		  { \
	GG.gameLoopSeconds = vgame.oldLoopSeconds; \
}

/*! \brief get the number of months that have elapsed in the game */
#define	getMonthsElapsed()	(GG.TimeElapsed >> 2)

#define	getMapXPos()	(GG.map_xpos)
#define	setMapXPos(x)	GG.map_xpos = (x)
#define	getMapYPos()	(GG.map_ypos)
#define	setMapYPos(y)	GG.map_ypos = (y)
#define	getLoopSeconds()	(GG.gameLoopSeconds)
#define	setLoopSeconds(L)	GG.gameLoopSeconds = (L)
#define	getCredits()	(GG.credits)
#define	setCredits(C)	GG.credits = (C)
#define	incCredits(V)	GG.credits += (V)
#define	decCredits(V)	GG.credits -= (V)
#define	getUpkeep(K)	(GG.upkeep[K])
#define	setUpkeep(K, V)	GG.upkeep[K] = (V)
#define	getStatistics(K)	(&(GG.statistics[K]))
#define	setTimeElapsed(X)	GG.TimeElapsed = (X)
#define	incrementTimeElapsed(X)	GG.TimeElapsed += (X)
#define	setGameVersion(V)	strncpy((char *)GG.version, (char *)V, 4)
#define	setTax(T)	GG.tax = (T)
#define	getTax()	(GG.tax)

#define getGameInProgress()	(vgame.gameInProgress)
#define setGameInProgress(X)	vgame.gameInProgress = (X)

#define getGamePlaying()	(vgame.playing)
#define PauseGame()	vgame.playing = (0)
#define ResumeGame()	vgame.playing = (1)

/* Typedefs */

/*! \brief the type of the world elements */
typedef UInt8	welem_t;

/*! \brief the type of the world status flags */
typedef UInt8	selem_t;

/*!
 * \brief the possible errors/warnings.
 *
 * Make sure they don't intersect as the overlap in the warning dialogs.
 */
typedef enum {
	diSTART = 0, /*!< starting disaster guard */
	diFireOutbreak, /*!< a fire disaster */
	diPlantExplosion, /*!< a power plant explosion */
	diMonster, /*!< a monster */
	diDragon, /*!< a dragon */
	diMeteor, /*!< a meteor */
	diEND /*!< ending guard for disasters */
} disaster_t;

typedef enum {
	seOutOfMemory = 1, /*!< out of memory error */
	seInvalidSaveGame, /*!< invalid save game load was attempted */
	seUnknownBuildItem /*!< Unknown Build Item */
} syserror_t;

typedef enum {
	peSTART = 0, /*!< starting guard for errors */
	peFineOnMoney, /*!< money status is OK */
	peLowOnMoney, /*!< low on money */
	peOutOfMoney, /*!< out of money */
	peFineOnPower, /*!< power is OK */
	peLowOnPower, /*!< running low on power */
	peOutOfPower, /*!< power consumption > power supply */
	peFineOnWater, /*!< water is OK */
	peLowOnWater, /*!< running low on water */
	peOutOfWater, /*!< running out of water */
	peEND /*!< ending guard for errors */
} problem_t;

/*! \brief zone identification for scoring */
typedef enum {
	ztWhat = 0, /*!< Unknown Zone */
	ztCommercial = Z_COMMERCIAL_SLUM, /*!< Commercial Zone - connivant */
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

/*!
 * \brief the statistics structure.
 *
 * This is used by the simulation to record the various values for use in the
 * graph screen.
 *
 * The items are obtained using the buildcount array, so we need a mapping of
 * the items in the build count array to the entries in the statistics array
 */
typedef enum {
	st_cashflow = 0, /*!< History information about cashflow */
	st_pollution, /*!< History information about pollution */
	st_crime, /*!< History information about crime */
	st_residential, /*!< Histroy information about residential averages */
	st_commercial, /*!< History information about commercial values */
	st_industrial, /*!< History information about industrial values */
	st_tail /*!< tail ender, for allocating the array */
} StatisticItem;

/*!
 * \brief the structure containing graphical history
 *
 * the entries are logarithmically scaled.
 * We have: 4 entries per year for the first 10 years
 * Then we have: 4 entries per decade for the next 100 years
 */
typedef struct _history {
	/*! values from last ten years */
	UInt16		last_ten[STATS_COUNT];
	/*! values from last century */
	UInt16		last_century[STATS_COUNT];
} stat_item;

/*!
 * \note Once a tree/forest becomes adjacent to an occupied area it becomes
 * part of the 'natural' forest
 */
/*! \brief elements for the BuildCount[] array */
typedef enum {
	bc_count_residential = 0, /*!< count of residential areas */
	bc_value_residential, /*!< values of residential units */
	bc_count_commercial, /*!< count of the commercial units */
	bc_value_commercial, /*!< value of commercial units */
	bc_count_industrial, /*!< count of industrial units */
	bc_value_industrial, /*!< value of industrial units */
	bc_count_roads, /*!< count of roads */
	bc_value_roads, /*!< value of roads */
	bc_count_trees, /*!< count of trees/forests/parks */
	bc_value_trees, /*!< value of trees/forests/parks (unnatural) */
	bc_water, /*!< count of water (unnatural) */
	bc_coalplants, /*!< count of coal power plants */
	bc_nuclearplants, /*!< count of nuclear power plants */
	bc_powerlines, /*!< count of power lines */
	bc_waterpumps, /*!< count of water pumps */
	bc_waterpipes, /*!< count of water pipes */
	bc_waste, /*!< count of wasteland zones */
	bc_radioactive, /*< count of radioactive areas */
	bc_fire, /*!< count of fire elements */
	bc_fire_stations, /*!< count of fire stations */
	bc_police_departments, /*!< count of police stations */
	bc_military_bases, /*!< count of military bases */
	bc_cashflow, /*!< cashflow value */
	bc_pollution, /*!< pollution valuation */
	bc_crime, /*!< Criminal level */
	bc_count_rail, /*!< Count of rail zones */
	bc_value_rail, /*!< Value of rail zones */
	bc_tail		/*!< tail ender */
} BuildCount;

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
	obj_plane, /*!< Aeroplane */
	obj_ship, /*!< a boat to wander up & down the river */
	obj_train, /*!< a train */
	obj_power, /*!< the invisible power-supply fairy */
	obj_water  /*!< The invisible water-supply fairy */
} Objects;

/*! \brief The type of a defence unit */
typedef enum {
	DuFireman = 0, DuPolice, DuMilitary, DuPadding = 512
} DefenceUnitTypes;

/*! \brief a defence unit structure */
typedef struct _defence_unit {
	UInt16	x;	/*!< x position of the defence unit */
	UInt16	y;	/*!< y position of the defence unit */
	UInt16	active; /*!< flag indicating this unit is active */
	DefenceUnitTypes type; /*!< the defence unit type */
} DefenceUnit;

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

/*!
 * \brief desire elements
 */
typedef enum {
	de_evaluation = 0,
	de_residential,
	de_commercial,
	de_industrial,
	de_end
} desire_elt;

/*!
 * \brief the central game structure.
 *
 * This is the central game struct only one of this exists at a time
 * and is called 'game'. This entire struct will be saved between games.
 *
 * Anything with an underscore will be removed later on.
 */
typedef struct _game_struct06a {
	UInt8	version[4];	/*!< version of game */
	UInt8	mapx;		/*!< size of map on the X-axis */
	UInt8	mapy;		/*!< size of the map on the Y-axis */
	Int8	map_xpos;	/*!< X-position on screen that's visible */
	Int8	map_ypos;	/*!< Y-position on screen that's visible */
	Int32	credits;	/*!< amount of money we've got. */
	UInt32	TimeElapsed;	/*!< number of half-months since 00 */
	UInt8	tax;		/*!< tax rate */
	UInt8	gameLoopSeconds; /*!< speed of game */
	UInt8	diff_disaster;  /*!< merge of difficulty and disaster */
	/*! auto-bulldozing, visible minimap, detailed minimap */
	UInt8	gas_bits;
	Int8	cityname[20];   /*!< Name of city */
	UInt8	upkeep[ue_tail];	/*!< upkeep %ages for bits */
	UInt8	gridsToUpdate;	/*!< Grids to be updated on next grid cycle */

	Int16	desires[de_end];	/*!< desire elements (and evaluation) */

	stat_item statistics[st_tail]; /*!< statistics */
	DefenceUnit	units[NUM_OF_UNITS]; /*!< active units */
	MoveableObject	objects[NUM_OF_OBJECTS]; /*!< active objects */
} GameStruct06;

/*! \brief the statistic to value structure */
typedef struct _stat_to_value {
	StatisticItem	item;	/*!< Item we are concerned with */
	BuildCount	offset; /*!< Offset into object */
} stat_to_value;

/*! \brief the statistic to value array */
extern stat_to_value statvalues[];

/*
 * Followed by:
 * Map
 * Grid0 .. Gridn
 */

/*! \brief currently supported save game version */
typedef GameStruct06	GameStruct;

/*!
 * \brief volatile game structure
 *
 * Items that are needed during the execution of the game, but not needed
 * for persistence.
 */
typedef struct _vgame_struct {
	UInt16	mapmul;	/*!< x*y */
	UInt32	prior_credit; /*!< last month's credit value */
	Int16	BuildCount[bc_tail]; /*!< count of elements */
	UInt16	oldLoopSeconds;	/*!< last selected speed - for pause */
	Int8	gameInProgress; /*!< is game progressing */
	Int8	playing; /*!< is game in play (paused for dialogs etc.) */
	lsObj_t	*powers; /*!< list of power supplies */
	lsObj_t	*waters; /*!< list of water supplies */
} vGameStruct;

/*! \brief game visual entities related to the game, but not the simulation */
typedef struct _visual_tag {
	UInt8	TileSize;	/*!< size of a tile */
	UInt8	MapTileSize;	/*!< size of a tile on the map */
	UInt16	visible_x;	/*!< visible tiles on the X */
	UInt16	visible_y;	/*!< visible tiles on the y */
	UInt16	cursor_xpos;	/*!< cursor ?? */
	UInt16	cursor_ypos;	/*!< cursor ?? */
} vGameVisuals;


#define	mapTileSize()	(visuals.MapTileSize)
#define	setMapTileSize(X)	visuals.MapTileSize = (X)
#define	gameTileSize()	(visuals.TileSize)
#define	setGameTileSize(X)	visuals.TileSize = (X)

#define	inGameTiles(X)	((X) / visuals.TileSize)

#define	getVisibleX()	(visuals.visible_x)
#define	getVisibleY()	(visuals.visible_y)
#define	setVisibleX(X)	visuals.visible_x = (X)
#define	setVisibleY(Y)	visuals.visible_y = (Y)

#define	getCursorX()	(visuals.cursor_xpos)
#define	setCursor(X)	visuals.cursor_xpos = (X)
#define	getCursorY()	(visuals.cursor_ypos)
#define	setCursorY(Y)	visuals.cursor_ypos = (Y)

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

/*!
 * \brief Function to return the character string for a month (short)
 *
 * This function is needed for the various platforms as they do not all work
 * the same way. We get the advantage in (l)unix that the language will be
 * derived from the current locale.
 * \param month the month to get
 * \param string the buffer to fit the string into
 * \param length the maximum string acceptable in string
 * \return the month in a string
 */
Char *getMonthString(UInt16 month, Char *string, UInt16 length);

#if defined(__cplusplus)
}
#endif

#endif /* _ZAKDEF_H */
