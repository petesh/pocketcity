/*! \file
 * \brief interface to the routines to pack and unpack bitfield arrays
 */

#if !defined(_PACK_H_)
#define _PACK_H_

#if defined (__cplusplus)
extern "C" {
#endif

#include <appconfig.h>
#include <sections.h>

void PackBits(void *src, void *dest, UInt8 nbits, UInt32 count) PACK_SECTION;
void UnpackBits(void *src, void *dest, UInt8 nbits, UInt32 count) PACK_SECTION;

#if defined (__cplusplus)
}
#endif

#endif
