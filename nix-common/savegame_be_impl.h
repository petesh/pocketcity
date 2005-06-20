#if !defined(_SAVEGAME_BE_IMPL_H)
#define _SAVEGAME_BE_IMPL_H

#define _SAVEGAME_BE_IMPL

#include <zakdef.h>

/*!
 * \brief an embedded savegame inside a savegame
 */
struct embedded_savegame {
	GameStruct gs;	/*!< game structure of the savegame */
	char *world;	/*!< world pointer of the savegame */
	char *flags;	/*!< world pointer of the savegame */
};

/*! \brief a savegame structure */
struct save_tag {
	int gamecount; /*!< count of savegames in this structure */
	struct embedded_savegame *games; /*!< the games */
};

typedef struct save_tag savegame_t;

#endif /* _SAVEGAME_BE_IMPL_H */
