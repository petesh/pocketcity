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
/*!
 * \brief write output to the console
 * \param s the string for formatting
 */
void WriteLog(char *s, ...);
/*!
 * \brief write Xtended output to the console
 * \param s the string for formatting
 */
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

/*!
 * \brief set up the graphics
 * \todo fix this routine.
 */
void UISetUpGraphic(void);
/*! \brief Draw the amount of credits on screen. */
void UIDrawCredits(void);
void UIGotoForm(Int16);
/*! \brief Draw the population on screen. */
void UIDrawPop(void);
/*! \brief Draw the date on screen.  */
void UIDrawDate(void);
/*! \brief Draw the map position on screen */
void UIDrawLoc(void);
/*! \brief Update the build icon on screen */
void UIDrawBuildIcon(void);
/*! \brief Draw the speed icon on screen */
void UIDrawSpeed(void);
void UIPaintDesires(void);

void UICheckMoney(void);
void UIScrollDisplay(dirType direction);
void _UIDrawRect(Int16 nTop, Int16 nLeft, Int16 nHeight, Int16 nWidth);

/*! \brief draw the play area */
void UIDrawPlayArea(void);

/*!
 * \brief draw a field
 * \param xpos the horizontal position
 * \param ypos the vertical position
 * \param nGraphic the item to paint
 */
void UIDrawField(Int16 xpos, Int16 ypos, welem_t nGraphic);

/*!
 * \brief Draw a special object
 * \param xpos the horizontal position
 * \param ypos the vertical position
 * \param i the item to draw
 */
void UIDrawSpecialObject(Int16 xpos, Int16 ypos, Int8 i);

/*!
 * \brief draw a special unit on the screen
 * \param xpos the horizontal location on screen
 * \param ypos the vertical location on screen
 * \param i the unit do draw
 */
void UIDrawSpecialUnit(Int16 xpos, Int16 ypos, Int8 i);

/*!
 * \brief draw a cursor at the location specified
 * \param xpos the xposition
 * \param ypos the y position
 */
void UIDrawCursor(Int16 xpos, Int16 ypos);

/*!
 * \brief Draw a power loss overlay on the screen at the specified location
 * \param xpos the xposition
 * \param ypos the y position
 */
void UIDrawPowerLoss(Int16 xpos, Int16 ypos);

/*!
 * \brief Draw a water loss overlay on the screen at the specified location
 * \param xpos the x position
 * \param ypos the y position
 */
void UIDrawWaterLoss(Int16 xpos, Int16 ypos);

/*!
 * \brief get the selected item to build
 * \return the item that is selected.
 */
BuildCode UIGetSelectedBuildItem(void);

/*!
 * \brief the map has jumped to another location
 */
void MapHasJumped(void);

/*!
 * \brief get a random number between 0 and max
 * \param max the maximum limit of the value to obtain
 * \return the random number
 */
UInt32 GetRandomNumber(UInt32 max);

void UIUpdateMap(UInt16 xpos, UInt16 ypos);

/*! \brief the zones to lock/unlock */
typedef enum {
	lz_world = 0, /*!< lock the world zone */
	lz_growable, /*!< lock the growable memory zone */
	lz_end /*!< the end/guard entry for the memory zones */
} lockZone;

/*!
 * \brief lock the zone specified for writing
 * \param zone the zone to lock
 */
void LockZone(lockZone zone);

/*!
 * \brief unlock the zone specified. don't write to it anymore
 * \param zone the zone to unlock
 */
void UnlockZone(lockZone zone);

/
void ReleaseZone(lockZone zone);

#if defined(__cplusplus)
}
#endif

#endif /* _UI_H */
