/*! \file
 * \brief utilities for unix - common
 */

#if !defined(_NIX_UTILS_H)
#define _NIX_UTILS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <sys/types.h>

/*!
 * \brief find the item in the space concerned
 * \param space the haystack to search
 * \param space_len the size of the haystack
 * \param item the needle to find
 * \param item_len the size of the needle
 * \return NULL if the string could not be found, otherwise it's location.
 */
void * inMem(const char *space, size_t space_len, const char *item,
    size_t item_len);

#if defined(__cplusplus)
}
#endif

#endif /* _NIX_UTILS_H */
