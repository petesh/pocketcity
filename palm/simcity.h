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
#include <sections.h>

/* global vars from simcity.c */
#define	MK4(A, B, C, D)	(((A) << 12) | ((B) << 8) | ((C) << 4) | (D))
#define	SGTYP   'PCsg'
#define	SGSTRING	"PCsg"
#define	TILEDBTYPE 'tidb'
#define	TBMP	'Tbmp'
#define	TVER	'tver'
#define	TSTR	'tSTR'
#define	SGNAME  "Pocket City Saves"

UInt32 GetPositionClicked(void) SIMCITY_SECTION;

void Clear35ROM(void) SIMCITY_SECTION;
void Set35ROM(void) SIMCITY_SECTION;
UInt16 Is35ROM(void) SIMCITY_SECTION;

void Clear40ROM(void) SIMCITY_SECTION;
void Set40ROM(void) SIMCITY_SECTION;
UInt16 Is40ROM(void) SIMCITY_SECTION;

void ClearScaleModes(void) SIMCITY_SECTION;
void SetScaleModes(void) SIMCITY_SECTION;
UInt16 IsScaleModes(void) SIMCITY_SECTION;

void clearProblemFlags(void) SIMCITY_SECTION;

/*!
 * \brief get a resource string
 * \param index the index into the resstrings to get the string from
 * \param string the destination buffer to fill
 * \param length the maximum length of string to fill
 */
void ResGetString(UInt16 index, char *string, UInt16 length) SIMCITY_SECTION;

void GotoForm(Int16 n) SIMCITY_SECTION;

#endif
