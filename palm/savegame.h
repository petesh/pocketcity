// event handlers for forms
extern Boolean hFiles(EventPtr event);
extern Boolean hFilesNew(EventPtr event);


// save/load framework
extern void UISaveGameToIndex(void);
extern int UILoadAutoGame(void);
extern void UISaveGame(UInt16 index);
extern void UIClearAutoSaveSlot(void);
