void SetUpGraphic(void);
void RedrawAllFields(void);
void ScrollMap(int direction);
void MoveCursor(int direction);
void DrawField(int xpos, int ypos);
void DrawCross(int xpos, int ypos);
unsigned char GetSpecialGraphicNumber(long unsigned int pos, int nType);
void DrawFieldWithoutInit(int xpos, int ypos);
void Goto(int x, int y);

int IsRoad(unsigned char x);
int IsZone(unsigned char x, zoneType nType);
int CarryPower(unsigned char x);
int CarryWater(unsigned char x);
int IsPowerLine(unsigned char x);
