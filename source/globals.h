#include "zakdef.h"

extern char mapsize;
extern int visible_x;
extern int visible_y;
extern char map_xpos;
extern char map_ypos;
extern char cursor_xpos;
extern char cursor_ypos;
extern long signed int credits;
extern long unsigned int BuildCount[20];
extern long unsigned int TimeElapsed;
extern int tax;
extern int TILE_SIZE;
extern short updatePowerGrid;
extern short unsigned int SIM_GAME_LOOP_SECONDS;
extern char cityname[20];


extern void LongToString(signed long value, char* out);
extern char* GetDate(char * temp);
extern void UIDrawPop(void);
extern void UIDoTaxes(void);



extern DefenceUnit    units[10];
extern MoveableObject objects[10];
