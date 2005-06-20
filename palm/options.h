/*!
 * \file
 * \brief interface to the options forms
 *
 * The only things exposed are the form handling routines. Everything
 * else is internal to the forms.
 */
#include <PalmTypes.h>
#include <PalmOS.h>

#include <sections.h>

#if !defined(_OPTIONS_H_)
#define	_OPTIONS_H_

/*!
 * \brief Handler for the main options dialog.
 * \param event the event that happened
 * \return true if event was dealt with
 */
Boolean hOptions(EventPtr event) MAP_SECTION;
/*!
 * \brief handler for the button configuration form
 * \param event the event to deal with
 * \return whether the event was dealt with or not.
 *
 * This form allows the user to configure the buttons on the PalmOs device.
 */
Boolean hButtonConfig(EventPtr event) MAP_SECTION;

#endif /* _OPTIONS_H_ */
