/*! \file
 * \brief contains the global functions for the 'nix platform
 */

#include <zakdef.h>
#include <time.h>

/*!
 * \brief get a month as a short string.
 *
 * Uses strftime to get the brief month name for a specific month.
 * \param month the zero offset month (0 = jan)
 * \param string the string that filled in with the month
 * \length the maximum length of the month
 * \return the string passed (useful for printf; etc.)
 */
char *
getMonthString(UInt16 month, Char *string, UInt16 length)
{
	struct tm tm;
	tm.tm_mon = month;
	strftime(string, length, "%b", &tm);
	return (string);
}

