/*! \file
 * \brief contains the global functions for the 'nix platform
 */

#include <zakdef.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <strings.h>
#include <nix_utils.h>

int
searchForFile(Char *file, UInt16 length, Char *path)
{
	char *buffer;
	char *atp = strdup(path);
	char *tat = atp;
	char *cap;
	struct stat sbuf;
	size_t max_path = pathconf("/", _PC_PATH_MAX);

	buffer = malloc(max_path + 1);

	do {
		cap = strchr(atp, ':');
		if (cap != NULL)
			*cap = '\0';
		snprintf(buffer, max_path, "%s/%s", atp, file);
		if (stat(buffer, &sbuf) != -1) {
			strlcpy(file, buffer, length);
			free(tat);
			free(buffer);
			return (1);
		}
		if (cap != NULL)
			atp = cap + 1;
	} while (cap != NULL);
	if (tat != NULL) free(tat);
	free(buffer);
	return (0);
}

