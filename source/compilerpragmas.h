
#ifndef _COMPILERPRAGMAS_H
#define	_COMPILERPRAGMAS_H

/*
 * compiler pragmas to support compiling on various compilers
 * currently supports: gcc, sun cc
 */

#ifdef __cplusplus
extern "C" {
#endif

#define	LOCAL static

#if defined(__GNUC__)

#else
#if defined(__SUNPRO_C)

#define	__attribute__(X)

#else

#error "Unsupported compiler ... please add support into compilerpragmas"
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* _COMPILERPRAGMAS_H */
