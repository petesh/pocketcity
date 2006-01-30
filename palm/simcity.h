/*!
 * \file
 * \brief the interface routines to the main program.
 *
 * This is a platform specific file, and is used to splice out a bunch of
 * includes as well.
 *
 */
#if !defined(_SIMCITY_H)
#define	_SIMCITY_H

#include <PalmOS.h>
#include <zakdef.h>

/* global vars from simcity.c */
#define	MK4(A, B, C, D)	(((A) << 12) | ((B) << 8) | ((C) << 4) | (D))
#define	SGTYP   'PCsg'
#define	SGSTRING	"PCsg"
#define	TILEDBTYPE 'tidb'
#define	TBMP	'Tbmp'
#define	TVER	'tver'
#define	TSTR	'tSTR'
#define	SGNAME  "Pocket City Saves"

UInt32 GetPositionClicked(void);

void Clear35ROM(void);
void Set35ROM(void);
UInt16 Is35ROM(void);

void Clear40ROM(void);
void Set40ROM(void);
UInt16 Is40ROM(void);

void ClearScaleModes(void);
void SetScaleModes(void);
UInt16 IsScaleModes(void);

void clearProblemFlags(void);
void ResGetString(UInt16 index, char *string, UInt16 length);

void GotoForm(Int16 n);

#endif
