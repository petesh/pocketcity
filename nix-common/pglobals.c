/*! \file
 * \brief contains the global functions for the 'nix platform
 */

#include <zakdef.h>
#include <time.h>
 
char *
getMonthString(UInt16 month, Char *string, UInt16 length)
{
	struct tm tm;
	tm.tm_mon = month;
	strftime(string, length, "%b", &tm);
	return (string);
}
