/*!
 * \file
 * \brief back-end code for savegames
 *
 * Deals with the savegames being loaded and saved.
 * Can perform read-only work on palmOS games directly, but performs
 * read/write on native games.
 *
 * The code needs to be made more platform-independent, as it is it depends
 * a lot on the endianness of the machine you're running it on.
 * \todo change the savegame format to XML for the linux platforms.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <main.h>
#include <globals.h>
#include <handler.h>
#include <ui.h>
#include <simulation.h>
#include <inttypes.h>
#include <strings.h>
#include <mem_compat.h>
#include <stringsearch.h>

/*!
 * \brief an embedded savegame inside a savegame
 */
struct embedded_savegame {
	GameStruct gs;	/*!< game structure of the savegame */
	char *world;	/*!< world pointer of the savegame */
};

/*! \brief a savegame structure */
struct save_tag {
	int gamecount; /*!< count of savegames in this structure */
	struct embedded_savegame *games; /*!< the games */
};
typedef struct save_tag savegame_t;

#define _SAVEGAME_BE_IMPL
#include <savegame_be.h>

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

/*
static char *
mapm_int8(char *mem, char *val)
{
	*val = *mem;
	return (mem + 1);
}
*/

/*
 * \brief map a memory element that's 16 bits long
 * \param mem the memory pointer to start at
 * \param val the value to fill
 * \return pointer to after the read structure
 */
static unsigned char *
mapm_int16(unsigned char *mem, unsigned char *val)
{
	*(Int16 *)val = (mem[0] << 8 | mem[1]);
	return (mem + 2);
}

/*
 * \brief map a memory element that's 32bits long
 * \param mem the memory to map
 * \param val the value to populate with the value
 * \return pointer to just after the data
 */
static unsigned char *
mapm_int32(unsigned char *mem, unsigned char *val)
{
	*(Int32 *)val = (mem[0] << 24 | mem[1] << 16 | mem[2] << 8 | mem[3]);
	return (mem + 4);
}

/*
 * \brief read the palm game structure and the map.
 * \param mem the pointer to the start of the memory of the game
 * \param new the pointer to the gamestructure to fill with the data
 * \param map pointer to the pointer that will be filled with the map
 * \return the pointer to just after the structure.
 *
 * This routine reads in the palm structure and it's corresponding map from
 * a mem-mapped copy of the file. If you change the savegame structure you
 * need to make sure that the reading routines here match the declarations
 * in the GameStruct structure, otherwise you read in garbage.
 */
static char *
read_palmstructure(char *mem, GameStruct *new, char **map)
{
	int i;
	int j;
	char *ptr = (char *)new;
	size_t map_size;

	printf("starting from: %p\n", mem);
	bzero(new, sizeof (*new));

	/* savegame identifier */
	for (i = 0; i < 4; i++)
		*(ptr + i) = *mem++;

	new->mapx = *mem++;
	new->mapy = *mem++;

	new->map_xpos = *mem++;
	new->map_ypos = *mem++;

	mem = mapm_int32(mem, (char *)&new->credits);
	mem = mapm_int32(mem, (char *)&new->TimeElapsed);

	new->tax + *mem++;
	new->gameLoopSeconds = *mem++;
	new->diff_disaster = *mem++;
	new->auto_bulldoze = *mem++;
	bcopy(mem, new->cityname, CITYNAMELEN);
	mem += CITYNAMELEN;
	for (i = 0; i < ue_tail; i++)
		new->upkeep[i] = *mem++;
	new->gridsToUpdate = *mem++;

	mem = mapm_int16(mem, (char *)&new->evaluation);
	i = 0;
	while (i < st_tail) {
		for (j = 0; j < STATS_COUNT; j++) {
			mem = mapm_int16(mem,
			    (char *)&new->statistics[i].last_ten[j]);
		}
		for (j = 0; j < STATS_COUNT; j++) {
			mem = mapm_int16(mem,
			    (char *)&new->statistics[i].last_century[j]);
		}
		i++;
	}
	i = 0;
	while (i < NUM_OF_UNITS) {
		mem = mapm_int16(mem, (char *)&new->units[i].x);
		mem = mapm_int16(mem, (char *)&new->units[i].y);
		mem = mapm_int16(mem, (char *)&new->units[i].active);
		new->units[i].type = *mem++;
		mem += 1;
		i++;
	}
	i = 0;
	while (i < NUM_OF_OBJECTS) {
		mem = mapm_int16(mem, (char *)&new->objects[i].x);
		mem = mapm_int16(mem, (char *)&new->objects[i].y);
		mem = mapm_int16(mem, (char *)&new->objects[i].dir);
		mem = mapm_int16(mem, (char *)&new->objects[i].active);
		i++;
	}
	map_size = (sizeof (welem_t) + sizeof (selem_t)) *
	    new->mapx * new->mapy;
	ptr = malloc(map_size);
	*map = ptr;
	printf("map starting from: %p for %d\n", mem, map_size);
	for (i = 0; i < new->mapx * new->mapy; ptr+=2, i++) {
		mem = mapm_int16(mem, ptr);
	}
	return (mem);
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

savegame_t *
savegame_open(char *filename)
{
	struct stat st;
	savegame_t *rv = (savegame_t *)calloc(1, sizeof (savegame_t));
	int fd = open(filename, O_RDONLY, 0666);
	char *buf, *buf2, *buf3;
	size_t size;
	char sbuf[4];

	if (rv == NULL) {
		perror("calloc");
		return (NULL);
	}

	if (fd == -1) {
		perror("open");
		return (NULL);
	}

	read(fd, sbuf, 4);
	if (0 == memcmp(sbuf, SAVEGAMEVERSION, 4)) {
		size_t worldsize;
		lseek(fd, SEEK_SET, 0);
		rv->gamecount = 1;
		rv->games = calloc(1, sizeof (struct embedded_savegame));
		read(fd, &rv->games[0].gs, sizeof (GameStruct));
		worldsize = (rv->games[0].gs.mapx * rv->games[0].gs.mapy) *
		    (sizeof (selem_t) + sizeof (welem_t));
		rv->games[0].world = calloc(1, worldsize);
		read(fd, (void *)worldPtr, worldsize);
		close(fd);
		return(rv);
	}

	/* It's a palmos savegame structure */

	if (-1 == fstat(fd, &st)) {
		perror("fstat");
		close(fd);
		return (NULL);
	}
	size = st.st_size;

	buf = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return (NULL);
	}
	close(fd);
	buf2 = inMem(buf, size, SAVEGAMEVERSION, 4);
	while (buf2 != NULL) {
		rv->gamecount++;
		rv->games = realloc(rv->games,
		    rv->gamecount * sizeof (struct embedded_savegame));
		buf3 = buf2;
		buf2 = read_palmstructure(buf2,
		    &rv->games[rv->gamecount - 1].gs,
		    &rv->games[rv->gamecount - 1].world);
		size -= (buf2 - buf3);
		buf2 = inMem(buf2, size, SAVEGAMEVERSION, 4);
	}
	munmap(buf, st.st_size);
	return (rv);
}

void
savegame_close(savegame_t *sg)
{
	int index;

	for (index = 0; index < sg->gamecount; index++) {
		free(sg->games[index].world);
	}
	free(sg->games);
	free(sg);
}

char *
savegame_getcityname(savegame_t *sg, int item)
{
	if (sg == NULL || item >= sg->gamecount)
		return (NULL);
	else
		return (sg->games[item].gs.cityname);
}

int
savegame_getcity(savegame_t *sg, int item, GameStruct *gs, char **map)
{
	size_t newl = 0;
	if (sg == NULL || item >= sg->gamecount)
		return (-1);
	bcopy(&sg->games[item].gs, gs, sizeof (GameStruct));
	newl = (sizeof (welem_t) + sizeof (selem_t)) * gs->mapx * gs->mapy;
	*map = (char *)realloc (*map, newl);
	bcopy(sg->games[item].world, *map, newl);
	return (0);
}

int
savegame_setcity(savegame_t *sg, int item, GameStruct *gs, char *map)
{
	size_t newl;

	if (map == NULL || sg == NULL || item >= sg->gamecount)
		return (-1);
	bcopy(gs, &sg->games[item].gs, sizeof (GameStruct));
	newl = sizeof (selem_t) + sizeof (welem_t) * gs->mapx * gs->mapy;
	sg->games[item].world = realloc(sg->games[item].world, newl);
	bcopy(map, sg->games[item].world, newl);
	return (0);
}

int
savegame_citycount(savegame_t *sg)
{
	if (sg == NULL) return (-1);
	return (sg->gamecount);
}

int
load_defaultfilename(void)
{
	savegame_t *sg = savegame_open(getCityFileName());
	if (sg == NULL)
		return (-1);
	if (worldPtr != NULL) free(worldPtr);
	savegame_getcity(sg, 0, &game, (char **)&worldPtr);
	savegame_close(sg);
	return (0);
}

int
save_defaultfilename()
{
	return(save_filename(getCityFileName(), &game, worldPtr));
}

int
save_filename(char *sel, GameStruct *gs, char *world)
{
	int fd, ret;
	ssize_t worldsize = gs->mapx * gs->mapy *
	    (sizeof (selem_t) + sizeof (welem_t));

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

	ret = write(fd, (void*)gs, sizeof (GameStruct));
	if (ret == -1) {
		perror("write game"); /* TODO: make this nicer */
		return (-1);
	} else if (ret != sizeof (GameStruct)) {
		WriteLog("Whoops, couldn't write full length of game\n");
		return (-1);
	}

	/* and now the great worldPtr :D */
	ret = write(fd, (void*)world, (size_t)worldsize);
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

void
NewGame(void)
{
	InitGameStruct();
	setDifficultyLevel(0);
	setDisasterLevel(getDifficultyLevel() + 1);
	setMapSize(100, 100);
	ConfigureNewGame();
}

static char *cityfile;

char *
getCityFileName(void)
{
	return (cityfile);
}

int
setCityFileName(char *newName)
{
	if (strcasecmp(newName + strlen(newName) - 4, ".pdb") == 0)
		return (-1);
	if (cityfile != NULL)
		free(cityfile);
	cityfile = strdup(newName);
	return (0);
}
