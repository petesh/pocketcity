
#include "zakdef.h"


#define MSG_ERROR_OUT_OF_MEMORY		0
#define MSG_QUESTION_REALLY_QUIT	1


extern int  UIDisplayError(int nError);
extern void UIInitDrawing(void);
extern void UIFinishDrawing(void);
extern void UIDrawBorder(void);
extern void UISetUpGraphic(void);
extern void UIDrawCredits(void);


extern void UIDrawField(int xpos, int ypos, unsigned char nGraphic);
extern void UIDrawCursor(int xpos, int ypos);
extern void UIDrawPowerLoss(int xpos, int ypos);
extern unsigned char UIGetSelectedBuildItem(void);


extern int InitWorld(void);
extern int ResizeWorld(long unsigned size);
extern void LockWorld(void);
extern void UnlockWorld(void);
extern unsigned char GetWorld(long unsigned int pos);
extern void SetWorld(long unsigned int pos, unsigned char value);
extern void LockWorldFlags(void);
extern void UnlockWorldFlags(void);
extern unsigned char GetWorldFlags(long unsigned int pos);
extern void SetWorldFlags(long unsigned int pos, unsigned char value);


extern unsigned long GetRandomNumber(unsigned long max);



