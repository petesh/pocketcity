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

static void
read_palmstructure(GameStruct *new, int fd)
{
	int i;

	bzero(new, sizeof (*new));
	read(fd, new->version, 4);
	new->mapsize = read_int8(fd);
	read_int8(fd);
	/* skip visibles 2xint16 */
	read_int32(fd);
	new->map_xpos = read_int16(fd);
	new->map_ypos = read_int16(fd);
	/* skip cursor */
	read_int32(fd);
	new->credits = read_int32(fd);
	/* build count */
	i = 0;
	while (i++ < 20)
		read_int32(fd);
	new->TimeElapsed = read_int32(fd);
	new->tax = read_int8(fd);
	new->auto_bulldoze = read_int8(fd);
	/* hole for name ?? */
	read_int16(fd);
	read(fd, new->cityname, CITYNAMELEN);
	new->upkeep[0] = read_int8(fd);
	new->upkeep[1] = read_int8(fd);
	new->upkeep[2] = read_int8(fd);
	new->diff_disaster = read_int8(fd);
	i = 0;
	while (i < 10) {
		new->units[i].x = read_int16(fd);
		new->units[i].y = read_int16(fd);
		new->units[i].active = read_int16(fd);
		new->units[i].type = read_int16(fd);
		i++;
	}
	i = 0;
	while (i < 10) {
		new->objects[i].x = read_int16(fd);
		new->objects[i].y = read_int16(fd);
		new->objects[i].dir = read_int16(fd);
		new->objects[i].active = read_int16(fd);
		i++;
	}
}

static void
write_palmstructure(GameStruct *new, int fd)
{
	int i;

	write(fd, new->version, 4);
	write_int8(fd, new->mapsize);
	write_int8(fd, 0);
	write_int32(fd, 0); /* visibles */
	write_int16(fd, new->map_xpos);
	write_int16(fd, new->map_ypos);
	write_int32(fd, 0); /* cursor */
	write_int32(fd, new->credits);
	i = 0;
	while(i++ < 20)
		write_int32(fd, 0);
	write_int32(fd, new->credits);
	write_int8(fd, new->tax);
	write_int8(fd, new->auto_bulldoze);
	write_int16(fd, 0);
	write(fd, new->cityname, CITYNAMELEN);
	write_int8(fd, new->upkeep[0]);
	write_int8(fd, new->upkeep[1]);
	write_int8(fd, new->upkeep[2]);
	write_int8(fd, new->diff_disaster);
	i = 0;
	while(i++ < 10) {
		write_int16(fd, new->units[i].x);
		write_int16(fd, new->units[i].y);
		write_int16(fd, new->units[i].active);
		write_int16(fd, new->units[i].type);
		i++;
	}
	i = 0;
	while (i++ < 10) {
		write_int16(fd, new->objects[i].x);
		write_int16(fd, new->objects[i].y);
		write_int16(fd, new->objects[i].dir);
		write_int16(fd, new->objects[i].active);
		i++;
	}

}

int
open_filename(char *sel, int palm)
{
	int fd, ret;
	char tempversion[4];

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
	ret = read(fd, (void *)worldPtr, GetMapMul());
	if (ret == -1) {
		perror("read world"); /* TODO: make this nicer */
		return (-1);
	} else if (ret != GetMapMul()) {
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
		write_palmstructure(&game, fd);
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
	ret = write(fd, (void*)worldPtr, GetMapMul());
	if (ret == -1) {
		perror("write world"); /* TODO: make this nicer */
		return (-1);
	} else if (ret != GetMapMul()) {
		WriteLog("Whoops, couldn't write full length of world\n");
		return (-1);
	}

	if (close(fd) == -1) {
		perror("close"); /* TODO: make this nicer */
		return (-1);
	}
	return (0);
}
