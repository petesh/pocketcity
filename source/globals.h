#if !defined(_GLOBALS_H_)
#define _GLOBALS_H_

#include <zakdef.h>

extern GameStruct game;
extern vGameStruct vgame;
extern AppConfig_t gameConfig;

#ifdef __cplusplus
extern "C" {
#endif

void LongToString(signed long value, char* out);
char* GetDate(char * temp);
void UIDrawPop(void);
void UIDoTaxes(void);
void *getIndexOf(char *ary, int addit, int key);

#ifdef __cplusplus
}
#endif

#endif

