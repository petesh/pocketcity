/*! \file
 * \brief Packing and unpacking routines
 *
 * pack and unpack bit field arrays.
 */

#include <pack.h>
#include <appconfig.h>

#define ANDUPTO(X) \
	((1<<((X)-1)) | (1<<((X)-2)) | (1<<((X)-3)) | (1<<((X)-4)) | \
	(1<<((X)-5)) | (1<<((X)-6)) | (1<<((X)-7)) | (1<<((X)-8)))

/*!
 * \brief pack from src->dest count of the nbits-lowest order bits
 *
 * \param src source memory
 * \param dest destination memory
 * \param nbits number of bits to pack
 * \param count count of entries to pack
 */
void
PackBits(void *src, void *dest, UInt8 nbits, UInt32 count)
{
	UInt8 *dest2 = (UInt8 *)dest;
	UInt8 *src2 = (UInt8 *)src;
	UInt8 offset = 8;

	*dest2 = 0;
	while (--count) {
		UInt8 val = *src2 & ANDUPTO(nbits);
		src2++;
		if (offset == 0) { /* it's a byte */
			dest2++;
			*dest2 = 0;
			offset = 8;
		}
		if (offset > nbits) { /* have more space available */
			offset -= nbits;
			*dest2 |= val << (offset);
		} else {
			UInt8 lam = nbits - offset;
			*dest2 |= val >> lam;
			offset = 8 - lam;
			dest2++;
			*dest2 = val << offset;
		}
	}
}

/*!
 * \brief unpack from src->dest the number of bits into the count specified
 * \param src source memory to unpack from
 * \param dest dest memory to unpack into
 * \param nbits number of bits in this element
 * \param count number of elements
 *
 * Process: shift the current value right by offset bits and and with the mask
 * This works providing we have more bits left in the field than bits we want
 * to shift by.
 */
void
UnpackBits(void *src, void *dest, UInt8 nbits, UInt32 count)
{
	UInt8 *dest2 = (UInt8 *)dest;
	UInt8 *src2 = (UInt8 *)src;
	UInt8 offset = 8;
	UInt8 and = ANDUPTO(nbits);

	while (count--) {
		UInt8 val;
		if (offset == 0) {
			src2++;
			offset = 8;
		}
		if (offset >= nbits) {
			offset -= nbits;
			val = (*src2 >> offset) & and;
		} else {
			/* we have offset remaining bits */
			UInt8 lam = nbits - offset;
			val = (*src2 & ANDUPTO(offset)) << lam;
			offset = 8 - lam;
			src2++;
			val |= (*src2 >> offset) & ANDUPTO(lam);
		}
		*((UInt8 *)dest2) = val;
		dest2++;
	}
}

