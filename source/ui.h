/*! \file
 * \brief the user interface routines that need defining in any implementation
 *
 * These are all the functions that need implementing if you want to 
 * make the game work.
 */
#if !defined(_UI_H)
#define	_UI_H

#include <zakdef.h>
#include <build.h>

#define	MSG_QUESTION_REALLY_QUIT	1

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef DEBUG
void WriteLog(char *, ...);
#else
#if defined(__cplusplus)
inline void WriteLog(char *, ...) {}
#else
#define	WriteLog(...)
#endif
#endif

void UIDisplayError(erdiType nError);
void UIDisplayError1(char *message);

/*! \brief Initialize the drawing area */
void UIInitDrawing(void);
/*! \brief finish up using the drawing area */
void UIFinishDrawing(void);

/*! \brief unlock the screen, allow any paused repainting to happen */
void UIUnlockScreen(void);

/*! \brief lock the screen, cause any waiting paing operations to happen */
void UILockScreen(void);

/*! \brief Draw the border around the playing area */
void UIDrawBorder(void);

/*! \brief set up the graphics XXX: FIXME */
void UISetUpGraphic(void);
void UIDrawCredits(void);
void UIUpdateBuildIcon(void);
void UIGotoForm(Int16);
void UIDrawPop(void);
void UIPaintDesires(void);

void UICheckMoney(void);
void UIScrollMap(dirType direction);
void _UIDrawRect(Int16 nTop, Int16 nLeft, Int16 nHeight, Int16 nWidth);


void UIDrawField(Int16 xpos, Int16 ypos, UInt8 nGraphic);
void UIDrawSpecialObject(Int16 i, Int16 xpos, Int16 ypos);
void UIDrawSpecialUnit(Int16 i, Int16 xpos, Int16 ypos);
void UIDrawCursor(Int16 xpos, Int16 ypos);
void UIDrawPowerLoss(Int16 xpos, Int16 ypos);
void UIDrawWaterLoss(Int16 xpos, Int16 ypos);
BuildCode UIGetSelectedBuildItem(void);

Int16 InitWorld(void);
Int16 ResizeWorld(UInt32 size);
UInt8 LockedWorld(void);
void LockWorld(void);
void UnlockWorld(void);
UInt8 GetWorld(UInt32 pos);
void SetWorld(UInt32 pos, UInt8 value);
UInt8 LockedWorldFlags(void);

void PurgeWorld(void);

UInt8 GetWorldFlags(UInt32 pos);
void SetWorldFlags(UInt32 pos, UInt8 value);
void OrWorldFlags(UInt32 pos, UInt8 value);
void AndWorldFlags(UInt32 pos, UInt8 value);
void MapHasJumped(void);


UInt32 GetRandomNumber(UInt32 max);
void UISetTileSize(Int16 size);

#if defined(__cplusplus)
}
#endif

#endif /* _UI_H */
