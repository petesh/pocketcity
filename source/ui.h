#if !defined(_UI_H)
#define _UI_H

#include <zakdef.h>

#define MSG_QUESTION_REALLY_QUIT        1

#if defined(__cplusplus)
extern "C" {
#endif

int UIDisplayError(erdiType nError);
void UIInitDrawing(void);
void UIFinishDrawing(void);
void UIUnlockScreen(void);
void UILockScreen(void);
void UIDrawBorder(void);
void UISetUpGraphic(void);
void UIDrawCredits(void);
void UIUpdateBuildIcon(void);
void UIGotoForm(int);
void UIDrawPop(void);

void UICheckMoney(void);
void UIScrollMap(dirType direction);
void _UIDrawRect(int nTop,int nLeft,int nHeight,int nWidth);


void UIDrawField(int xpos, int ypos, unsigned char nGraphic);
void UIDrawSpecialObject(int i, int xpos, int ypos);
void UIDrawSpecialUnit(int i, int xpos, int ypos);
void UIDrawCursor(int xpos, int ypos);
void UIDrawPowerLoss(int xpos, int ypos);
void UIDrawWaterLoss(int xpos, int ypos);
unsigned char UIGetSelectedBuildItem(void);


int InitWorld(void);
int ResizeWorld(unsigned long size);
void LockWorld(void);
void UnlockWorld(void);
unsigned char GetWorld(unsigned long pos);
void SetWorld(unsigned long pos, unsigned char value);
void LockWorldFlags(void);
void UnlockWorldFlags(void);

unsigned char GetWorldFlags(unsigned long pos);
void SetWorldFlags(unsigned long pos, unsigned char value);
void OrWorldFlags(unsigned long pos, unsigned char value);
void AndWorldFlags(unsigned long pos, unsigned char value);
void MapHasJumped(void);


unsigned long GetRandomNumber(unsigned long max);
void UISetTileSize(int size);
#ifdef DEBUG
void UIWriteLog(char*);
#else
#define UIWriteLog(x)
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _UI_H */
