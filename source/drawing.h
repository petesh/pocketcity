extern void                SetUpGraphic(void);
extern void                RedrawAllFields(void);
extern void                ScrollMap(int direction);
extern void                MoveCursor(int direction);
extern void                DrawField(int xpos, int ypos);
extern void                DrawCross(int xpos, int ypos);
extern unsigned char       GetSpecialGraphicNumber(long unsigned int pos, int nType);
extern void                DrawFieldWithoutInit(int xpos, int ypos);
extern void Goto(int x, int y);

extern int IsRoad(unsigned char x);
extern int IsZone(unsigned char x, int nType);
extern int CarryPower(unsigned char x);
extern int CarryWater(unsigned char x);
extern int IsPowerLine(unsigned char x);
