#include "zakdef.h"
#if defined(PALM)
#include <StringMgr.h>
#include <unix_stdio.h>
#else
#include <stdio.h>
#endif

/* this is the central game struct */
GameStruct game;
/* This is the volatile game structure (memoizing to reduce op/s) */
vGameStruct vgame;

int GetCiffer(int number, signed long value);

int
GetCiffer(int number, signed long value)
{
    if (value < 0) { value = 0-value; }

    switch(number) {
        case 10: return (value/1000000000);
        case 9: return (value%1000000000/100000000);
        case 8: return (value%100000000/10000000);
        case 7: return (value%10000000/1000000);
        case 6: return (value%1000000/100000);
        case 5: return (value%100000/10000);
        case 4: return (value%10000/1000);
        case 3: return (value%1000/100);
        case 2: return (value%100/10);
        case 1: return (value%10);
    }
    return 0;


}

char*
GetDate(char * temp)
{
    char year[10];
    char months[]="JanFebMarAprMayJunJulAugSepOctNovDec";

    temp[0] = months[(game.TimeElapsed%12)*3];
    temp[1] = months[(game.TimeElapsed%12)*3+1];
    temp[2] = months[(game.TimeElapsed%12)*3+2];
    temp[3] = ' ';

    sprintf(year, "%ld", (long)(game.TimeElapsed/12)+2000);
    temp[4] = year[0];
    temp[5] = year[1];
    temp[6] = year[2];
    temp[7] = year[3];
    temp[8] = (char)0;

    return (char*)temp;
}

void *
arIndex(char *ary, int addit, int key)
{
    while (*(int *)ary) {
        if (key == *(int *)ary)
            return (ary);
        ary += addit;
    }
    return (0);
}
