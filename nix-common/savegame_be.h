
#if !defined(_SAVEGAME_BE_H)
#define _SAVEGAME_BE_H

#if defined(__cplusplus)
extern "C" {
#endif

int load_filename(char *name, int palm);
int load_defaultfilename(int palm);
int save_filename(char *name, int palm);
int save_defaultfilename(int palm);
void NewGame(void);
void setCityFileName(char *name);
char *getCityFileName(void);

#if defined(__cplusplus)
}
#endif

#endif /* _SAVEGAME_BE_H */
