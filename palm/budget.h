/*!
 * \file
 * \brief interface to the budget ui of the palm application.
 *
 * We only expose the handler routine, to allow the handler to be hooked in
 * within the main application event handling loop.
 */
#if !defined(_BUDGET_H_)
#define	_BUDGET_H_

#include <PalmTypes.h>
#include <Event.h>
#include <sections.h>

/*!
 * \brief Handler for the budget form.
 *
 * \param event the event that was received
 * \return true if event was dealt with.
 */
Boolean hBudget(EventPtr event) MAP_SECTION;

#endif /* _BUDGET_H_ */
