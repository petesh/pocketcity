/*!
 * \file
 * \brief Platform level config file - palm.
 *
 * This file contains all the definitions appropriate to the palm platform.
 */
#if !defined(_CONFIG_H)
#define	_CONFIG_H

/*! We are on the palm platform */
#define	PALM
#include <ErrorMgr.h>
#include <PalmTypes.h>

/*!
 * \brief define an assertion.
 * 
 * Assertions work in the same manner as other platforms, terminating the
 * program if they arenot found. The purpose is to find programming errors,
 * not user input errors. You should only assert things such as pre/post
 * conditions and invariants.
 * \param X the item to assert
 */
#define	assert(X)	ErrFatalDisplayIf(!(X), #X)

#endif /* _CONFIG_H */
