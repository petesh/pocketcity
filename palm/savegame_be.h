/*!
 * \file
 * \brief this is the interface to the savegame back-end
 *
 * Contains all the interfaces that are exported to the front-end
 */
#if !defined(_SAVEGAME_BE_H)
#define	_SAVEGAME_BE_H

#include <palmutils.h>
#include <sections.h>

/*! \brief Load the autosave city. */
int LoadAutoSave(void) SAVE_SECTION;
/*! \brief Clear the autosave name */
void DeleteAutoSave(void) SAVE_SECTION;
/*! \brief set the autosave name */
void SetAutoSave(char *name) SAVE_SECTION;

/*!
 * \brief save the city that is currently being used by the name passed
 * \param name the name of the city
 */
void SaveGameByName(char *name) SAVE_SECTION;

/*!
 * \brief Delete a city from the DB by name
 * \param name the name of the city to delete
 */
void DeleteGameByName(char *name) SAVE_SECTION;

/*!
 * \brief Load a game by name
 * \param name the name of the city
 * \return 0 if the city was loaded, -1 otherwise
 */
int  LoadGameByName(char *name) SAVE_SECTION;

/*!
 * \brief Create a new save game slot.
 * \param name name of the city.
 *
 * Save the city into it using a special save-game mode that says it is to
 * be reconfigured
 */
void CreateNewSaveGame(char *name) SAVE_SECTION;

/*!
 * \brief Check if a saved city by this name exists
 * \param name the name of the city
 * \return true if game exists
 */
int GameExists(char *name) SAVE_SECTION;

/*!
 * \brief rename a city
 * \param oldname the old name of the city
 * \param newname the new name of the city
 * \return -1 if database could not be fount, 0 if not changes, 1 otherwise
 */
int RenameCity(char *oldname, char *newname) SAVE_SECTION;

/*!
 * \brief copy a city
 * \param name the name of the old city
 * \return true if the city was copied
 */
int CopyCity(char *name) SAVE_SECTION;

/*! \brief Reset the viewable elements of the volatile game configuration. */
void ResetViewable(void) SAVE_SECTION;

/*!
 * \brief Get The City Names
 * \param count the count returned
 * \return a NULL terminated array of the city names
 */
char **CityNames(int *count) SAVE_SECTION;

/*!
 * \brief release the list of city names returned using cityNames
 * \param names the names of the cities
 */
void FreeCityNames(char **names) SAVE_SECTION;

#endif /* _SAVEGAME_BE_H */
