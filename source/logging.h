/*! \file
 * \brief the user interface routines that need defining in any implementation
 *
 * These are all the functions that need implementing if you want to 
 * make the game work.
 */
#if !defined(_LOGGING_H)
#define	_LOGGING_H

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef DEBUG
/*!
 * \brief write output to the console
 * \param s the string for formatting
 */
void WriteLog(char *s, ...);
/*!
 * \brief write Xtended output to the console
 * \param s the string for formatting
 */
void WriteLogX(char *s, ...);
#else
#if defined(__cplusplus) || defined(_MSVC)
static void WriteLog(char *s, ...) {}
static void WriteLogX(char *s, ...) {}
#else
#define	WriteLog(...)
#define WriteLogX(...)
#endif
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _LOGGING_H */
