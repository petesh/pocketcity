/*! \file
 * \brief utilities for unix - common
 */

#if !defined(_NIX_UTILS_H)
#define _NIX_UTILS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <zakdef.h>

/*!
 * \brief search for a file using the path
 * \param file <inout> the name of the file to find
 * \param length the maximum length of the file name to fill.
 * \param path the path to search for the file
 * \return true if the file is found, false otherwise
 */
int searchForFile(Char *file, UInt16 length, Char *path);

#if defined(__cplusplus)
}
#endif

#endif /* _NIX_UTILS_H */
