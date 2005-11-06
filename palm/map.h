/*!
 * \file
 * \brief map rendering interface
 *
 * We only expose the handler for the map routine, everything else is internal
 * to the map code
 */
#if !defined(_MAP_H_)
#define	_MAP_H_

#include <PalmTypes.h>
#include <Event.h>

#include <sections.h>

/*!
 * \brief Handler for the map.
 * \param event the event received
 * \return the handle
 */
Boolean hMap(EventPtr event) MAP_SECTION;

#endif /* _MAP_H_ */
