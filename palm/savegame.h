#if !defined(_SAVEGAME_H)
#define	_SAVEGAME_H

#include <sections.h>

/* event handlers for forms */
Boolean hFiles(EventPtr event) MAP_SECTION;
Boolean hFilesNew(EventPtr event) MAP_SECTION;

/* save/load framework */
int UILoadAutoGame(void);
void UISaveAutoGame(void);
void UISaveMyCity(void);
void UIClearAutoSaveSlot(void);

#endif /* _SAVEGAME_H */
