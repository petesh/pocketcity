#if !defined (_SAVEGAME_H)
#define _SAVEGAME_H

/* event handlers for forms */
Boolean hFiles(EventPtr event);
Boolean hFilesNew(EventPtr event);

/* save/load framework */
int UILoadAutoGame(void);
void UISaveAutoGame(void);
void UISaveMyCity(void);
void UIClearAutoSaveSlot(void);

#endif /* _SAVEGAME_H */
