#if !defined(_GLOBALS_H_)
#define _GLOBALS_H_

#include <zakdef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern GameStruct game;
extern vGameStruct vgame;
extern AppConfig_t gameConfig;

void LongToString(signed long value, char* out);
char* GetDate(char * temp);
void UIDrawPop(void);
void UIDoTaxes(void);
void *getIndexOf(char *ary, int addit, int key);
UInt8 GetDisasterLevel(void);
void SetDisasterLevel(UInt8 value);
UInt8 GetDifficultyLevel(void);
void SetDifficultyLevel(UInt8 value);

#ifdef __cplusplus
}
#endif

#endif

