extern int mapsize;
extern int visible_x;
extern int visible_y;
extern int map_xpos;
extern int map_ypos;
extern int cursor_xpos;
extern int cursor_ypos;
extern long signed int credits;
extern long unsigned int BuildCount[10];
extern long unsigned int TimeElapsed;
extern int tax;
extern int TILE_SIZE;
extern short updatePowerGrid;
extern short unsigned int SIM_GAME_LOOP_SECONDS;


extern void LongToString(signed long value, char* out);
extern char* GetDate(char * temp);
extern void UIDrawPop(void);
extern void UIDoTaxes(void);

