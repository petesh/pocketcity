/*! \file
 * \brief utilities for unix - common
 */

#if !defined(_NIX_UTILS_H)
#define _NIX_UTILS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <zakdef.h>

int searchForFile(Char *file, UInt16 length, Char *path);

#if defined(__cplusplus)
}
#endif

#endif /* _NIX_UTILS_H */
