#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <main.h>
#include <globals.h>
#include <handler.h>
#include <ui.h>
#include <simulation.h>
#include <sys/types.h>
#include <inttypes.h>
#include <strings.h>
#include <mem_compat.h>

/*
static Int8
read_int8(int fd)
{
	Int8 rv;
	read(fd, &rv, 1);
	return (rv);
}

static void
write_int8(int fd, Int8 value)
{
	write(fd, &value, 1);
}

static Int16
read_int16(int fd)
{
	Int8 by[2];
	read(fd, by, 2);
	return (by[0] << 8 | by[1]);
}

static void
write_int16(int fd, Int16 value)
{
	write_int8(fd, (value >> 8) & 0xff);
	write_int8(fd, value & 0xff);
}

static Int32
read_int32(int fd)
{
	Int8 by[4];
	read(fd, by, 4);
	return (by[0] << 24 | by[1] << 16 | by[2] << 8 | by[3]);
}

static void
write_int32(int fd, Int32 value)
{
	write_int16(fd, (value >> 16) & 0xffff);
	write_int16(fd, value & 0xffff);
}
*/

static void *
map_int8(int fd, void *val)
{
	(void) read(fd, val, 1);
	return ((char *)val + 1);
}

static void *
map_int16(int fd, void *val)
{
	Int8 by[2];
	read(fd, by, 2);
	*(Int16 *)val = (by[0] << 8 | by[1]);
	return ((char *)val + 2);
}

static void *
map_int32(int fd, void *val)
{
	Int8 by[4];
	read(fd, by, 4);
	*(Int32 *)val = (by[0] << 24 | by[1] << 16 | by[2] << 8 | by[3]);
	return ((char *)val + 4);
}

static void
read_palmstructure(GameStruct *new, int fd)
{
	int i;
	int j;
	void *ptr = new;

	bzero(new, sizeof (*new));
	for (i = 0; i < 4; i++)
		ptr = map_int8(fd, ptr);
	ptr = map_int8(fd, ptr);
	ptr = map_int8(fd, ptr);
	ptr = map_int16(fd, ptr);
	ptr = map_int16(fd, ptr);
	ptr = map_int32(fd, ptr);
	ptr = map_int32(fd, ptr);
	ptr = map_int8(fd, ptr);
	ptr = map_int8(fd, ptr);
	ptr = map_int8(fd, ptr);
	ptr = map_int8(fd, ptr);
	read(fd, ptr, CITYNAMELEN);
	ptr = (char *)ptr + CITYNAMELEN;
	ptr = map_int8(fd, ptr);
	ptr = map_int8(fd, ptr);
	ptr = map_int8(fd, ptr);
	ptr = map_int16(fd, ptr);
	ptr = map_int8(fd, ptr);
	ptr = map_int8(fd, ptr);
	i = 0;
	while (i < st_tail) {
		for (j = 0; j < STATS_COUNT; j++) {
			ptr = map_int16(fd, ptr);
		}
		for (j = 0; j < STATS_COUNT; j++) {
			ptr = map_int16(fd, ptr);
		}
		i++;
	}
	i = 0;
	while (i < NUM_OF_UNITS) {
		ptr = map_int16(fd, ptr);
		ptr = map_int16(fd, ptr);
		ptr = map_int16(fd, ptr);
		ptr = map_int16(fd, ptr);
		i++;
	}
	i = 0;
	while (i < NUM_OF_OBJECTS) {
		ptr = map_int16(fd, ptr);
		ptr = map_int16(fd, ptr);
		ptr = map_int16(fd, ptr);
		ptr = map_int16(fd, ptr);
		i++;
	}
}

/*
static void
write_palmstructure(GameStruct *new, int fd)
{
	int i;
	int j;

	write(fd, new->gsi.version, 4);
	write_int8(fd, new->gsi.mapx);
	write_int8(fd, new->gsi.mapy);
	write_int16(fd, new->gsi.map_xpos);
	write_int16(fd, new->gsi.map_ypos);
	write_int32(fd, new->gsi.credits);
	write_int32(fd, new->gsi.TimeElapsed);
	write_int8(fd, new->gsi.tax);
	write_int8(fd, new->gsi.gameLoopSeconds);
	write_int8(fd, new->gsi.diff_disaster);
	write_int8(fd, new->gsi.auto_bulldoze);
	write(fd, new->gsi.cityname, CITYNAMELEN);
	write_int8(fd, new->gsi.upkeep[0]);
	write_int8(fd, new->gsi.upkeep[1]);
	write_int8(fd, new->gsi.upkeep[2]);
	write_int16(fd, new->gsi.evaluation);
	write_int8(fd, new->gsi.c_units);
	write_int8(fd, new->gsi.c_objects);
	i = 0;
	while (i < si_tail) {
		for (j = 0; j < STATS_COUNT; j++) {
			write_int16(fd, new->gsi.statistics[i].last_ten[j]);
		}
		for (j = 0; j < STATS_COUNT; j++) {
			write_int16(fd, new->gsi.statistics[i].last_century[j]);
		}
		i++;
	}
	i = 0;
	while (i < new->gsi.c_units) {
		 write_int16(fd, new->units[i].x);
		 write_int16(fd, new->units[i].y);
		 write_int16(fd, new->units[i].active);
		 write_int16(fd, new->units[i].type);
		i++;
	}
	i = 0;
	while (i < new->gsi.c_objects) {
		 write_int16(fd, new->objects[i].x);
		 write_int16(fd, new->objects[i].y);
		 write_int16(fd, new->objects[i].dir);
		 write_int16(fd, new->objects[i].active);
		i++;
	}
}
*/

int
open_filename(char *sel, int palm)
{
	int fd, ret;
	char tempversion[4];
	int worldsize = WorldSize();

	WriteLog("Opening save game from %s\n", sel);

	fd = open(sel, O_RDONLY);
	if (fd == -1) {
		perror("open"); /* TODO: make this nicer */
		return (-1);
	}
	/* first of all, check the savegame version */
	ret = read(fd, (void*)tempversion, 4);
	if (ret == -1) {
		perror("read version"); /* TODO: make this nicer */
		return (-1);
	}
	if (strncmp(tempversion, SAVEGAMEVERSION, 4) != 0) {
		WriteLog("Wrong save game format - aborting\n");
		return (-1);
	}
	/* version was ok, rewind the file */
	lseek(fd, 0, SEEK_SET);

	/* God, I love to read all my vars at one time */
	/* using a struct :D ... unfortunately horribly unportable */
	if (!palm) {
		ret = read(fd, (void *)&game, sizeof (GameStruct));
	if (ret == -1) {
		perror("read game"); /* TODO: make this nicer */
		return (-1);
		} else if (ret != sizeof (GameStruct)) {
			WriteLog("Oops, couldn't read full length of game\n");
			return (-1);
		}
	} else {
		read_palmstructure(&game, fd);
		/* make sure of position */
		lseek(fd, 294, SEEK_SET);
	}

	/* and now the great worldPtr :D */
	ret = read(fd, (void *)worldPtr, worldsize);
	if (ret == -1) {
		perror("read world"); /* TODO: make this nicer */
		return (-1);
	} else if (ret != worldsize) {
		WriteLog("Oops, couldn't read full length of world\n");
		return (-1);
	}

	if (close(fd) == -1) {
		perror("close"); /* TODO: make this nicer */
		return (-1);
	}

	return (0);
}

int
save_filename(char *sel, int palm)
{
	int fd, ret;
	int worldsize = WorldSize();

	if (sel == NULL) {
		return (-1);
	}
	WriteLog("Saving game as %s...\n", sel);

	fd = open(sel,
	    O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("open"); /* TODO: make this nicer */
		return (-1);
	}
	/* God, I love to write all my vars at one time */
	/* using a struct :D */
	if (palm != 0) {
		//write_palmstructure(&game, fd);
		return (0);
	}
	ret = write(fd, (void*)&game, sizeof (GameStruct));
	if (ret == -1) {
		perror("write game"); /* TODO: make this nicer */
		return (-1);
	} else if (ret != sizeof (GameStruct)) {
		WriteLog("Whoops, couldn't write full length of game\n");
		return (-1);
	}

	/* and now the great worldPtr :D */
	ret = write(fd, (void*)worldPtr, worldsize);
	if (ret == -1) {
		perror("write world"); /* TODO: make this nicer */
		return (-1);
	} else if (ret != worldsize) {
		WriteLog("Whoops, couldn't write full length of world\n");
		return (-1);
	}

	if (close(fd) == -1) {
		perror("close"); /* TODO: make this nicer */
		return (-1);
	}
	return (0);
}
