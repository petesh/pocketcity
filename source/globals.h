#if !defined(_GLOBALS_H_)
#define _GLOBALS_H_

#include "zakdef.h"

extern GameStruct game;
extern vGameStruct vgame;

extern void LongToString(signed long value, char* out);
extern char* GetDate(char * temp);
extern void UIDrawPop(void);
extern void UIDoTaxes(void);
extern void *arIndex(char *ary, int addit, int key);

#endif

