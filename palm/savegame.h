#if !defined(_SAVEGAME_H)
#define	_SAVEGAME_H

#include <sections.h>

/* event handlers for forms */
Boolean hFiles(EventPtr event) SAVE_SECTION;
Boolean hFilesNew(EventPtr event) SAVE_SECTION;

/* save/load framework */
int UILoadAutoGame(void) SAVE_SECTION;
void UISaveAutoGame(void) SAVE_SECTION;
void UISaveMyCity(void) SAVE_SECTION;
void UIClearAutoSaveSlot(void) SAVE_SECTION;

#endif /* _SAVEGAME_H */
