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

void UIDisasterNotify(disaster_t disaster);
void UIProblemNotify(problem_t problem);

void UISystemErrorNotify(syserror_t error);
void UIDisplayError1(char *message);

/*! \brief Initialize the drawing area */
void UIInitDrawing(void);
/*! \brief finish up using the drawing area */
void UIFinishDrawing(void);

/*! \brief unlock the screen, allow any paused repainting to happen */
void UIUnlockScreen(void);

/*! \brief lock the screen, pause any painting operations to happen */
void UILockScreen(void);

/*! \brief UI post load game handler */
void UIPostLoadGame(void);

/*!
 * \brief initialize the graphics
 * \todo fix this routine.
 */
void UIInitGraphic(void);
void UIGotoForm(Int16);

/*! \brief Draw the amount of credits on screen. */
void UIPaintCredits(void);
/*! \brief Draw the population on screen. */
void UIPaintPopulation(void);
/*! \brief Draw the date on screen.  */
void UIPaintDate(void);
/*! \brief Draw the map position on screen */
void UIPaintLocation(void);
/*! \brief Update the build icon on screen */
void UIPaintBuildIcon(void);
/*! \brief Draw the speed icon on screen */
void UIPaintSpeed(void);

/*! \brief Paint the desires on the display */
void UIPaintDesires(void);

/*! \brief draw the play area */
void UIPaintPlayArea(void);

/*! \brief Setup the graphics */
void UISetUpGraphic(void);

void UICheckMoney(void);
void UIScrollDisplay(dirType direction);
void _UIDrawRect(Int16 nTop, Int16 nLeft, Int16 nHeight, Int16 nWidth);

/*!
 * \brief draw a field
 * \param xpos the horizontal position
 * \param ypos the vertical position
 * \param nGraphic the item to paint
 */
void UIPaintField(UInt16 xpos, UInt16 ypos, welem_t nGraphic);

/*!
 * \brief Draw a special object
 * \param xpos the horizontal position
 * \param ypos the vertical position
 * \param i the item to draw
 */
void UIPaintSpecialObject(UInt16 xpos, UInt16 ypos, Int8 i);

/*!
 * \brief draw a special unit on the screen
 * \param xpos the horizontal location on screen
 * \param ypos the vertical location on screen
 * \param i the unit do draw
 */
void UIPaintSpecialUnit(UInt16 xpos, UInt16 ypos, Int8 i);

/*!
 * \brief draw a cursor at the location specified
 * \param xpos the xposition
 * \param ypos the y position
 */
void UIPaintCursor(UInt16 xpos, UInt16 ypos);

/*!
 * \brief Draw a power loss overlay on the screen at the specified location
 * \param xpos the xposition
 * \param ypos the y position
 */
void UIPaintPowerLoss(UInt16 xpos, UInt16 ypos);

/*!
 * \brief Draw a water loss overlay on the screen at the specified location
 * \param xpos the x position
 * \param ypos the y position
 */
void UIPaintWaterLoss(UInt16 xpos, UInt16 ypos);

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

/*!
 * \brief update an individual position on the map
 *
 * this would consist of painting it's location, as well as the power and
 * water items for that location. It is the map version of UIDrawField.
 * \param xpos the x position
 * \param ypos the y position
 * \param elem the item to paint on the map
 */
void UIPaintMapField(UInt16 xpos, UInt16 ypos, welem_t elem);

/*!
 * \brief update a map status position
 * \param xpos the x position on the map
 * \param ypos the y position on the map
 * \param world the world pointer
 * \param status the status of the zone
 */
void UIPaintMapStatus(UInt16 xpos, UInt16 ypos, welem_t world, selem_t status);

/*!
 * \brief check if the map location is being clipped by the game
 * \param xpos the x position on the map
 * \param ypos the y position on the map
 * \return true if the point on the map is clipped, and therefore not visible
 */
Int8 UIClipped(UInt16 xpos, UInt16 ypos);

/*!
 * \brief map being resized, do UI related elements
 */
void UIMapResize(void);

/*!
 * \brief update a map location
 *
 * Caused when the content on the map changes
 * \param xpos the xposition on the map that changed
 * \param ypos the y position on the map that changed
 */
void UIUpdateMap(UInt16 xpos, UInt16 ypos);

#if defined(__cplusplus)
}
#endif

#endif /* _UI_H */
