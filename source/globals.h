/*! \file
 * \brief interface to the global routines
 */

#if !defined(_GLOBALS_H_)
#define	_GLOBALS_H_

#include <zakdef.h>
#include <compilerpragmas.h>

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
	gu_playarea = (1),
	gu_credits = (1<<1),
	gu_population = (1<<2),
	gu_date = (1<<3),
	gu_location = (1<<4),
	gu_buildicon = (1<<5),
	gu_speed = (1<<6),
	gu_desires = (1<<7),
	gu_all = (1<<8)
} graphicupdate_t;

extern GameStruct game;
extern vGameStruct vgame;
extern vGameVisuals visuals;
extern AppConfig_t gameConfig;
extern void *worldPtr;
extern void *growablePtr;

EXPORT char *getDate(char *temp);
EXPORT UInt32 scaleNumber(UInt32 old_value, Char *scale);
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
EXPORT void getWorldAndFlag(UInt32 pos, welem_t *world, selem_t *flag);
EXPORT void setWorldAndFlag(UInt32 pos, welem_t value, selem_t status);

EXPORT void addGraphicUpdate(graphicupdate_t entity);
EXPORT void removeGraphicUpdate(graphicupdate_t entity);
EXPORT UInt8 checkGraphicUpdate(graphicupdate_t entity);
EXPORT UInt8 checkAnyGraphicUpdate(void);
EXPORT void clearGraphicUpdate(void);

#if defined(__cplusplus)
}
#endif

#endif
