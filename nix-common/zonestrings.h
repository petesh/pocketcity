/*!
 * \file
 * interface to the zonestrings file
 */

#if !defined(_ZONESTRINGS_H)
#define _ZONESTRINGS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <zakdef.h>

int getFieldString(welem_t world, char *dest, int destlen);
int getFieldValue(welem_t world, char *dest, int destlen);
int getFieldDensity(welem_t world, char *dest, int destlen);

#if defined(__cplusplus)
}
#endif

#endif
