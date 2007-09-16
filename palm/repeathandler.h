/*!
 * \file
 * \brief functions that deal with repeat items being clicked
 *
 * This contains the routines that are used to deal with changes in repeater
 * items being passed onto related fields.
 */

#include <PalmOS.h>

#include <sections.h>

#if !defined(_REPEATHANDLER_H_)
#define	_REPEATHANDLER_H_

/*! \brief the button mapping structure */
typedef struct buttonmapping_tag {
	UInt16		down; /*!< repeat button for down */
	UInt16		up; /*!< repeat button for up */
	UInt16		field; /*!< field value */
	Int16		min; /*!< minimum value to set field to */
	Int16		max; /*!< maximum value to set field to */
	UInt32		special1; /*!< special field one (custom use) */
	UInt32		special2; /*!< special field two (custom use) */
} buttonmapping_t;

/*! \brief handler for post change operation */
typedef void (*bmPostHandler)(UInt16 button, buttonmapping_t *mapping,
    Int32 newValue);

/*!
 * \brief get a spinner field index
 * \param map the list of mapping items
 * \param buttonControl the button/spinner that was checked
 * \param isButton is it a button (or a spinner)
 * \return the item that was found or null.
 */
buttonmapping_t *getSpinnerFieldIndex(buttonmapping_t *map,
    UInt16 buttonControl, Boolean isButton) REPEATH_SECTION;

/*!
 * \brief process a repeat button being pressed
 * \param map the list of items to process
 * \param control the control that was clicked
 * \param isButton is it a button
 * \param post_handle function to call after processing value change
 * \return the map item that was picked.
 */
buttonmapping_t *processRepeater(buttonmapping_t *map, UInt16 control,
    Boolean isButton, bmPostHandler post_handle) REPEATH_SECTION;

#endif
