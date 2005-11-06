/*!
 * \file
 * \brief interface to the global routines
 */

#if !defined(_GLOBALS_H_)
#define	_GLOBALS_H_

#include <zakdef.h>
#include <compilerpragmas.h>
#include <globals_include.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief thye types of things that need a graphic update
 *
 * This list contains all the items that may need to be updated as a result
 * of changes to the simulation state. It is inteded to defer the repainting
 * of the screen until the end. By doing this we reduce the number of painting
 * calls significantly; it only checks what needs to be done and performs it.
 * By doing this we allow multi-threading of the simulation routine. It simply
 * tags what needs to be painted and a separate drawing thread can take care
 * of it.
 *
 * This is a significant departure from previous implementations where we
 * repainted the items at the direct request of the command.
 */
typedef enum {
	gu_playarea = (1), /*!< The play area needs updating */
	gu_credits = (1<<1), /*!< The credits value needs updating */
	gu_population = (1<<2), /*!< The population needs updating */
	gu_date = (1<<3), /*!< The date field needs updating */
	gu_location = (1<<4), /*!< The map location needs updating (minimap) */
	gu_buildicon = (1<<5), /*!< The build icon needs updating */
	gu_speed = (1<<6), /*!< The speed indicator needs updating */
	gu_desires = (1<<7), /*!< The desires of the city */
	gu_all = (1<<8) /*!< Everything needs painting */
} graphicupdate_t;

/*!
 * \brief these fields are to control game options.
 */
typedef enum {
	sb_autobulldoze = (1), /*!< do we auto-bulldoze trees */
	sb_showminimap = (1<<1), /*!< do we show the minimap */
	sb_detailedminimap = (1<<2) /*!< is the minimap detailed */
} gamestatusbit_t;

/*! set the Auto Bulldoze flag */
#define	SETAUTOBULLDOZE(X)	setGameBit(sb_autobulldoze, (X))
/*! get the Auto Bulldoze flag */
#define	GETAUTOBULLDOZE()	getGameBit(sb_autobulldoze)
/*! set if the minimap is visible */
#define	SETMINIMAPVISIBLE(X)	setGameBit(sb_showminimap, (X))
/*! get if the minimap is visible */
#define	GETMINIMAPVISIBLE()	getGameBit(sb_showminimap)
/*! set to show the minimap in a detailed form */
#define	SETMINIMAPDETAILED(X)	setGameBit(sb_detailedminimap, (X))
/*! get if the minimap is supposed to be detailed */
#define	GETMINIMAPDETAILED()	getGameBit(sb_detailedminimap)

/*! This is the game structure - it is saved */
extern GameStruct game;
/*! This is the volatile game structure - it is not saved */
extern vGameStruct vgame;
/*!
 * \brief This contains the information on the visuals tilesize and the likes
 */
extern vGameVisuals visuals;
/*! This is the game configuration - system choices */
extern AppConfig_t gameConfig;

/*! This is the pointer to the world */
extern char *worldPtr;
/*! This is the pointer to the state of the world */
extern char *flagPtr;

/*! This is the pointer to the power state of the world */
extern char *powerMap;

/*! This is the water state of the world */
extern char *waterMap;

/*! This is the pollution map */
extern char *pollutionMap;

/*! This is the crime map. */
extern char *crimeMap;

EXPORT char *getDate(char *temp);

EXPORT Int32 scaleNumber32(Int32 old_value, Char *scale);
EXPORT Int16 scaleNumber16(Int16 old_value, Char *scale);

EXPORT void *getIndexOf(char *ary, Int16 addit, Int16 key);
EXPORT UInt8 getDisasterLevel(void);
EXPORT void setDisasterLevel(UInt8 value);
EXPORT UInt8 getDifficultyLevel(void);
EXPORT void setDifficultyLevel(UInt8 value);

EXPORT Int16 InitWorld(void);
EXPORT Int16 ResizeWorld(UInt32 size);

EXPORT void PurgeWorld(void);

EXPORT welem_t getWorld(UInt32 pos);
EXPORT void setWorld(UInt32 pos, welem_t value);
EXPORT selem_t getWorldFlags(UInt32 pos);
EXPORT void setWorldFlags(UInt32 pos, selem_t value);
EXPORT void orWorldFlags(UInt32 pos, selem_t value);
EXPORT void andWorldFlags(UInt32 pos, selem_t value);
EXPORT void clearWorldFlags(UInt32 pos, selem_t value);
EXPORT void getWorldAndFlag(UInt32 pos, welem_t *world, selem_t *flag);
EXPORT void setWorldAndFlag(UInt32 pos, welem_t value, selem_t status);

EXPORT void addGraphicUpdate(graphicupdate_t entity);
EXPORT void removeGraphicUpdate(graphicupdate_t entity);
EXPORT UInt8 checkGraphicUpdate(graphicupdate_t entity);
EXPORT UInt8 checkAnyGraphicUpdate(void);
EXPORT void clearGraphicUpdate(void);

EXPORT void setGameBit(gamestatusbit_t bit, UInt8 value);
EXPORT UInt8 getGameBit(gamestatusbit_t bit);

EXPORT void PackBits(void *src, void *dest, UInt8 nbits, UInt32 count);
EXPORT void UnpackBits(void *src, void *dest, UInt8 nbits, UInt32 count);

#if defined(__cplusplus)
}
#endif

#endif
