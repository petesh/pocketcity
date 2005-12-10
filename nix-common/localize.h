/*!
 * \file
 * Just a few localization stubs to make it easier to process.
 */
#if !defined(_LOCALIZE_H)
#define _LOCALIZE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <libintl.h>

#define	_(S)	gettext(S)

#if defined(__cplusplus)
}
#endif

#endif
