/*! \file
 * \brief interface to the global routines
 */

#if !defined(_GLOBALS_H_)
#define	_GLOBALS_H_

#include <zakdef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern GameStruct game;
extern vGameStruct vgame;
extern AppConfig_t gameConfig;
extern void *worldFlags;

void LongToString(Int32 value, char *out);
char *GetDate(char *temp);
void UIDrawPop(void);
void UIDoTaxes(void);
void *getIndexOf(char *ary, Int16 addit, Int16 key);
UInt8 GetDisasterLevel(void);
void SetDisasterLevel(UInt8 value);
UInt8 GetDifficultyLevel(void);
void SetDifficultyLevel(UInt8 value);

#ifdef __cplusplus
}
#endif

#endif
