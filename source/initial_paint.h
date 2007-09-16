/*!
 * \file
 * \brief Interface to the routines for building.
 *
 * This contains the declarations for all the functions for building
 * that are used by other sections of the simulation
 */

#if !defined(_INITIAL_PAINT_H_)
#define	_INITIAL_PAINT_H_

#include <zakdef.h>
#include <sections.h>
#include <compilerpragmas.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*! \brief Paint the world */
EXPORT void PaintTheWorld(void) BUILD_SECTION;

#if defined(__cplusplus)
}
#endif

#endif /* _INITIAL_PAINT_H_ */
