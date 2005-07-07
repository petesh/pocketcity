/*!
 * \file
 * \brief interface to beaming functions
 */

#if !defined(_BEAM_H_)
#define _BEAM_H_

#include <PalmTypes.h>

#include <appconfig.h>
#include <sections.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * \brief beam a city record
 * \param map_ptr a read only pointer to the city record
 * \return -1 if the beaming failed
 */
int BeamSend(UInt8 *map_ptr) BEAM_SECTION;

/*!
 * \brief register to receive city records
 */
void BeamRegister(void) BEAM_SECTION;

/*!
 * \brief receive the city
 * \param ptr the exchange manager data handle
 *
 * Must be in the first code section, which is the reason for not having the
 * section attribute.
 */
Err BeamReceive(ExgSocketType *ptr);

#if defined(__cplusplus)
}
#endif

#endif /* _BEAM_H_ */
