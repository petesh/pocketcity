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
void WriteLog(char *s, ...);
void WriteLogX(char *s, ...);
#else
#if defined(__cplusplus) || defined(_MSVC)
static void WriteLog(char *s, ...) {}
static void WriteLogX(char *s, ...) {}
#else
#define	WriteLog(...)
#define WriteLogX(...)
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

/*! \brief lock the screen, pause any painting operations to happen */
void UILockScreen(void);

/*! \brief Draw the border around the playing area */
void UIDrawBorder(void);

/*! \brief set up the graphics XXX: FIXME */
void UISetUpGraphic(void);
void UIDrawCredits(void);
void UIGotoForm(Int16);
void UIDrawPop(void);
void UIDrawDate(void);
void UIDrawLoc(void);
void UIDrawBuildIcon(void);
void UIDrawSpeed(void);
void UIPaintDesires(void);

void UICheckMoney(void);
void UIScrollDisplay(dirType direction);
void _UIDrawRect(Int16 nTop, Int16 nLeft, Int16 nHeight, Int16 nWidth);

void UIDrawPlayArea(void);
void UIDrawField(Int16 xpos, Int16 ypos, welem_t nGraphic);
void UIDrawSpecialObject(Int16 xpos, Int16 ypos, Int8 i);
void UIDrawSpecialUnit(Int16 xpos, Int16 ypos, Int8 i);
void UIDrawCursor(Int16 xpos, Int16 ypos);
void UIDrawPowerLoss(Int16 xpos, Int16 ypos);
void UIDrawWaterLoss(Int16 xpos, Int16 ypos);
BuildCode UIGetSelectedBuildItem(void);

void MapHasJumped(void);

UInt32 GetRandomNumber(UInt32 max);

void UIUpdateMap(UInt16 xpos, UInt16 ypos);

typedef enum {
	lz_world = 0, /*!< lock the world zone */
	lz_growable, /*!< lock the growable memory zone */
	lz_end /*!< the end/guard entry for the memory zones */
} lockZone;

void LockZone(lockZone zone);
void UnlockZone(lockZone zone);
void ReleaseZone(lockZone zone);

#if defined(__cplusplus)
}
#endif

#endif /* _UI_H */
