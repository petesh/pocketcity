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

EXPORT GameStruct game;
EXPORT vGameStruct vgame;
EXPORT vGameVisuals visuals;
EXPORT AppConfig_t gameConfig;
EXPORT void *worldPtr;
EXPORT void *growablePtr;

EXPORT char *getDate(char *temp);
EXPORT Int32 scaleNumber(UInt32 old_value, Char *scale);
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

#if defined(__cplusplus)
}
#endif

#endif
