#if !defined(_SAVEGAME_BE_H)
#define	_SAVEGAME_BE_H

#include <palmutils.h>
#include <sections.h>

int LoadAutoSave(void) SAVE_SECTION;
void DeleteAutoSave(void) SAVE_SECTION;
void SetAutoSave(char *name) SAVE_SECTION;
void SaveGameByName(char *name) SAVE_SECTION;
void DeleteGameByName(char *name) SAVE_SECTION;
int  LoadGameByName(char *name) SAVE_SECTION;
void CreateNewSaveGame(char *name) SAVE_SECTION;
int GameExists(char *name) SAVE_SECTION;
int RenameCity(char *oldname, char *newname) SAVE_SECTION;
int CopyCity(char *name) SAVE_SECTION;
void ResetViewable(void) SAVE_SECTION;

char **CityNames(int *count) SAVE_SECTION;
void FreeCityNames(char **names) SAVE_SECTION;

#endif /* _SAVEGAME_BE_H */
