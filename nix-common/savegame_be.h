
#if !defined(_SAVEGAME_BE_H)
#define _SAVEGAME_BE_H

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(_SAVEGAME_BE_IMPL)
/* Opaque to outside world */
typedef void savegame_t;
#endif

/*!
 * \brief open a savegame file, breaking it into games
 * \param filename the name of the file to open
 * \return the savegame, or NULL if the file could not be processed.
 */
savegame_t *savegame_open(char *filename);
/*!
 * \brief close a previously created savegame structure
 * \param sg the gamestructure from an savegame_open call
 */
void savegame_close(savegame_t *sg);
/*!
 * \brief get the count of the cities in a savegame structure
 * \param sg the savegame structure
 * \return the count of the cities, or -1 on error
 */
int savegame_citycount(savegame_t *sg);
/*!
 * \brief get the name of a city in the savegame structure
 * \param sg the savegame structure allocated from savegame_open
 * \param item the savegame who's name you want to access
 * \return a pointer to the savegame, or NULL on error
 */
char *savegame_getcityname(savegame_t *sg, int item);
/*!
 * \brief get a city structure from the savegame structure
 * \param sg the savegame structure allocated from savegame_open
 * \param item the index of the savegame to access
 * \param gs the game structure to populate with the world
 * \param map the reference to a pointer that will be allocated and filled
 *        with the map. It will be realloced into.
 * \return 0 if the item was copied safely, -1 otherwise
 *
 * the map parameter will have it's value overwritten, so if you've allocated
 * a pointer into it you need to free it beforehand.
 */
int savegame_getcity(savegame_t *sg, int item, GameStruct *gs, char **map);
/*!
 * \brief store the contents of a city into the savegame structure
 * \param sg the savegame structure
 * \param gs the game structure to store
 * \param map the map to store
 * \return 0 on success, -1 on error
 */
int savegame_setcity(savegame_t *sg, int item, GameStruct *gs, char *map);

/*!
 * \brief save the game into the file specified
 * \param name the name of the file to save the game to
 * \param gs the structure to save
 * \param world the world to save
 * \return 0 if the file was saved successfully, -1 otherwise
 */
int save_game(char *name, GameStruct *gs, char *world);
/*!
 * \brief save the default savegame file and map
 * \return -1 if something went wrong, 0 otherwise
 */
int save_defaultfilename(void);
/*!
 * \brief load the default file name into the default game structures
 * \return -1 if something went wrong, 0 otherwise
 */
int load_defaultfilename(void);

/*!
 * \brief save a filename using the gamestruct and worldptr passed
 * \param sel the name of the file to save
 * \param gs the gamestruct to save
 * \param world the worldpointer to save
 * \return -1 on error, 0 otherwise.
 */
int save_filename(char *sel, GameStruct *gs, char *world);

/*!
 * \brief Initialize the game structures for a new game
 */
void NewGame(void);

/*!
 * \brief set the city file name
 * \return -1 if the name was rejected (ended in .pdb)
 */
int setCityFileName(char *name);
/*!
 * \brief get the name of default city file
 * \return NULL if it's not set.
 */
char *getCityFileName(void);

#if defined(__cplusplus)
}
#endif

#endif /* _SAVEGAME_BE_H */
