/*! \file
 * \brief contains the global functions for the 'nix platform
 */

#include <zakdef.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <nix_utils.h>

/*!
 * \brief search for a file using the path
 * \param file <inout> the name of the file to find
 * \param filelen the maximum length of the file name to fill.
 * \param path the path to search for the file
 * \return true if the file is found, false otherwise
 */
int
searchForFile(Char *file, UInt16 length, Char *path)
{
	char buffer[MAXPATHLEN];
	char *atp = strdup(path);
	char *tat = atp;
	char *cap;
	struct stat sbuf;

	while (atp != NULL) {
		cap = strchr(atp, ':');
		if (cap != NULL)
			*cap = '\0';
		snprintf(buffer, MAXPATHLEN - 1, "%s/%s", atp, file);
		if (stat(buffer, &sbuf) != -1) {
			strlcpy(file, buffer, length);
			free(tat);
			return (1);
		}
		if (cap != NULL)
			atp = cap + 1;
	}
	if (tat != NULL) free(tat);
	return (0);
}

