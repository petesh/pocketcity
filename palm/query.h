/*!
 * \file
 * \brief interface to the query form
 *
 * Only exports the handler for the form so it can be hooked by the
 * application handler
 */
#if !defined(_QUERY_H)
#define	_QUERY_H

#include <PalmTypes.h>
#include <sections.h>
#include <Form.h>

/*!
 * \brief Handler for the query form
 * \param event the event to deal with
 * \return wheterht eh event was handled or not
 */
Boolean hQuery(EventPtr event) MAP_SECTION;

#endif /* _QUERY_H */
