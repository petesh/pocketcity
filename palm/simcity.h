#if !defined(_SIMCITY_H)
#define	_SIMCITY_H

#include <zakdef.h>

/* global vars from simcity.c */
#define MK4(A,B,C,D)	(((A) << 12) | ((B) << 8) | ((C) << 4) | (D))
#define	SGTYP   'DATA'
#define	TILEDBTYPE 'tidb'
#define TBMP	'Tbmp'
#define TVER	'tver'
#define TSTR	'tSTR'
#define	SGNAME  "PCitySave"

extern MemPtr worldPtr;

UInt32 GetPositionClicked(void);
/* For the 'dialogs' */
void PauseGame(void);
void ResumeGame(void);
UInt8 IsGamePlaying(void);
/*
 * For the 'is there a game in progress'.
 * Faffing around with save games causes games to not be in progress.
 * Needed as separate concept from the Pause / Resume for autosave purposes.
 */
void SetGameInProgress(void);
void SetGameNotInProgress(void);
UInt8 IsGameInProgress(void);

void ClearNewROM(void);
void SetNewROM(void);
UInt8 IsNewROM(void);

void ClearDirectBmps(void);
void SetDirectBmps(void);
UInt8 IsDirectBmps(void);
void ResGetString(UInt16 index, char *string, UInt16 length);

#endif
