/*!
 * \file
 * \brief interface to the savegame code
 *
 * exports the handlers and a couple of convenience functions
 */
#if !defined(_SAVEGAME_H)
#define	_SAVEGAME_H

#include <Event.h>
#include <sections.h>

/*!
 * \brief Handler for the list of cities dialog.
 * \param event event to react to
 * \return true if event was dealt with, false otherwise.
 */
Boolean hFiles(EventPtr event) SAVE_SECTION;

/*!
 * \brief Handler for the new file form.
 *
 * \param event the event received
 * \return true if handled.
 */
Boolean hFilesNew(EventPtr event) SAVE_SECTION;

/*! \brief Load the game form the auto-save slot */
int UILoadAutoGame(void) SAVE_SECTION;
/*! \brief Save the autosave game. */
void UISaveAutoGame(void) SAVE_SECTION;
/*! \brief save the city that is currently being used. */
void UISaveMyCity(void) SAVE_SECTION;
/*! \brief Delete the game stored in the auto-save slot */
void UIClearAutoSaveSlot(void) SAVE_SECTION;

#endif /* _SAVEGAME_H */
