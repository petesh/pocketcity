/*! \file
 * \brief utilities for unix - common
 */

#if !defined(_NIX_UTILS_H)
#define _NIX_UTILS_H

#if defined(__cplusplus)
extern "C" {
#endif

void * inMem(const char *space, size_t space_len, const char *item,
    size_t item_len);

#if defined(__cplusplus)
}
#endif

#endif /* _NIX_UTILS_H */
