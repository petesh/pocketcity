/*!
 * \file
 * \brief Platform level config file - palm
 *
 * This file contains all the #define commands for the palm platform.
 */
#if !defined(_CONFIG_H_)
#define	_CONFIG_H_

#define	PALM
#include <ErrorMgr.h>
#define	assert(X)	ErrFatalDisplayIf(!(X), #X)

#endif /* _CONFIG_H_ */
