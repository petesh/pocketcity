/*extern void		DrawGame(GStateHandle gstate, int full);
extern void		RedrawField(unsigned long pos);
extern void		UpdateFieldAtCursor(void);
extern void		UpdateCrossAtCursor(void);
extern int		GetGraphicNumber(int coords);
extern int		GetRoadType(long coords, int power);
extern void		MoveCursor(int direction);
extern void		ScrollMap(int direction);
extern void		DrawCredits(GStateHandle gstate);*/


extern void				SetUpGraphic(void);
extern void				RedrawAllFields(void);
extern void				ScrollMap(int direction);
extern void				MoveCursor(int direction);
extern void				DrawField(int xpos, int ypos);
extern void				DrawCross(int xpos, int ypos);
extern unsigned char	GetSpecialGraphicNumber(long unsigned int pos, int nType);



extern int IsRoad(unsigned char x);
extern int IsZone(unsigned char x, int nType);
extern int CarryPower(unsigned char x);
extern int IsPowerLine(unsigned char x);