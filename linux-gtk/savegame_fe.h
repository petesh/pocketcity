/*!
 * \file
 * \brief interface to the savegame front end
 *
 * These are the only routines that need to be communicated through
 */
#include <gtk/gtk.h>

#if !defined(_SAVEGAME_FE_H)
#define	_SAVEGAME_FE_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief handle a new game menu choice */
void newgame_handler(void);
/*! \brief handle a savegame as menu choice */
void savegameas_handler(void);
/*! \brief handle a save choice */
void savegame_handler(void);
/*! \brief handle an open choice */
void opengame_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* _SAVEGAME_FE_H */
