/*! \file
 * \brief compiler pragmas to support compiling on various compilers
 *
 * currently supports: gcc, sun cc
 */

#ifndef _COMPILERPRAGMAS_H
#define	_COMPILERPRAGMAS_H

#if defined(__cplusplus)
extern "C" {
#endif

/*! \brief LOCAL variables are static variables */
#define	LOCAL static
/*! \brief GLOBAL variables are normal variables */
#define GLOBAL

#if defined(__GNUC__)
#if defined(__cplusplus)
/*! \brief clear the parameter because it's invalid for c++ */
#define __attribute__(X)
#endif

#if (defined(_WIN32) || defined(__CYGWIN__))
/*
#if defined(PCITY_DLL)
#define EXPORT  __declspec(dllexport)
#else
#define EXPORT  __declspec(dllimport)
#endif
*/
#define EXPORT
#else
#define EXPORT
#endif

#else
#if defined(__SUNPRO_C)

/*! \brief clear the parameter because it's invalid for sun cc */
#define	__attribute__(X)
#define EXPORT

#else
#if defined(_MSVC)

#define __attribute__(X)
#define EXPORT __declspec(dllexport)

#else

#if defined(__MWERKS__)
#define __attribute__(X)
#define EXPORT
#else

/* It's probably going to be the following, but you need to make sure */
#warning "Please add compiler details to compilerpragmas.h"
#define __attribute__(X)
#define EXPORT

#endif
#endif
#endif
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _COMPILERPRAGMAS_H */
