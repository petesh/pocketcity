#include "../source/zakdef.h"

// global vars from simcity.c
#define SGTYP   'DATA'
#define SGNAME  "PCitySave"
extern short int game_in_progress;
extern MemPtr worldPtr;
extern MemPtr worldFlagsPtr;
extern short int oldROM;
#ifdef DEBUG
extern void UIWriteLog(char * s);
#else
#define UIWriteLog(x)
#endif
