#include "zakdef.h"

// this is the central game struct
GameStruct game;

unsigned char updatePowerGrid = 1;
unsigned char updateWaterGrid = 1;

int GetCiffer(int number, signed long value);

/* gah - maybe I should find a function that does this for me
 * but I'm too lazy, and this is crossplatform ;)
 */
extern void LongToString(signed long value, char* out)
{
    int move=0;
    int reachednumber;
    int i=0;
    char temp;
    reachednumber=0;

    if (value==0)
    {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    if (value<0)
    {
        out[0] = '-';
        move=1;
    }


    for (i=0; i<10; i++)
    {
        if (reachednumber==0)
        {
            temp = (char)GetCiffer(10-i, value)+0x30;
            if (temp != (char)'0')
            {
                reachednumber=1;
                out[move] = temp;
                move++;
            }
        }
        else
        {
            temp = (char)GetCiffer(10-i, value)+0x30;
            out[move] = temp;
            move++;
        }
    }
    out[move] = '\0';
}

int GetCiffer(int number, signed long value)
{
    if (value < 0) { value = 0-value; }

    switch(number)
    {
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






extern char* GetDate(char * temp)
{
    char year[5];
    char months[]="JanFebMarAprMayJunJulAugSepOctNovDec";

    temp[0] = months[(game.TimeElapsed%12)*3];
    temp[1] = months[(game.TimeElapsed%12)*3+1];
    temp[2] = months[(game.TimeElapsed%12)*3+2];
    temp[3] = ' ';

    LongToString((game.TimeElapsed/12)+2000,(char*)year);
    temp[4] = year[0];
    temp[5] = year[1];
    temp[6] = year[2];
    temp[7] = year[3];
    temp[8] = (char)0;

    return (char*)temp;
}
