/*!
 * \file
 * \brief back-end code for savegames
 *
 * Deals with the savegames being loaded and saved.
 * Operates on .pdb files in a read-only manner (palmos savegames)
 * Operates on .cty files in a read-write manner (OS-native). These can
 * be beamed to/from PalmOS devices.
 *
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
#include <sections.h>
#include <pack.h>
#include <handler.h>
#include <logging.h>
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
	char *flags;	/*!< world pointer of the savegame */
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

static Int16
read_int16(int fd)
{
	Int8 by[2];
	read(fd, by, 2);
	return (by[0] << 8 | by[1]);
}

static Int32
read_int32(int fd)
{
	Int8 by[4];
	read(fd, by, 4);
	return (by[0] << 24 | by[1] << 16 | by[2] << 8 | by[3]);
}

*/

static void
write_int8(int fd, Int8 value)
{
	write(fd, &value, 1);
}

static void
write_int16(int fd, Int16 value)
{
	write_int8(fd, (value >> 8) & 0xff);
	write_int8(fd, value & 0xff);
}

static void
write_int32(int fd, Int32 value)
{
	write_int16(fd, (value >> 16) & 0xffff);
	write_int16(fd, value & 0xffff);
}

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

/*!
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
 * I am completely convinced that the creation of this code could be automated
 * in order to eliminate the potential confusion every time the structure
 * changes (e.g. using the stabs information to generate this)
 */
static char *
read_palmstructure(char *mem, GameStruct *new, char **map, char **flags)
{
	int i;
	int j;
	char *ptr = (char *)new;
	size_t map_size;
	UInt32 foo;

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

	new->tax = *mem++;
	new->gameLoopSeconds = *mem++;
	new->diff_disaster = *mem++;
	new->gas_bits = *mem++;
	bcopy(mem, new->cityname, CITYNAMELEN);
	mem += CITYNAMELEN;
	for (i = 0; i < ue_tail; i++)
		new->upkeep[i] = *mem++;
	new->gridsToUpdate = *mem++;

	mem = mapm_int16(mem, (char *)&new->desires[de_evaluation]);
	mem = mapm_int16(mem, (char *)&new->desires[de_residential]);
	mem = mapm_int16(mem, (char *)&new->desires[de_commercial]);
	mem = mapm_int16(mem, (char *)&new->desires[de_industrial]);
	printf("stats starting at %p\n", mem);
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
	printf("units starting at %p\n", mem);
	i = 0;
	while (i < NUM_OF_UNITS) {
		mem = mapm_int16(mem, (char *)&new->units[i].x);
		mem = mapm_int16(mem, (char *)&new->units[i].y);
		mem = mapm_int16(mem, (char *)&new->units[i].active);
		mem = mapm_int16(mem, (char *)&new->units[i].type);
		i++;
	}
	printf("objects starting at %p\n", mem);
	i = 0;
	while (i < NUM_OF_OBJECTS) {
		mem = mapm_int16(mem, (char *)&new->objects[i].x);
		mem = mapm_int16(mem, (char *)&new->objects[i].y);
		mem = mapm_int16(mem, (char *)&new->objects[i].dir);
		mem = mapm_int16(mem, (char *)&new->objects[i].active);
		i++;
	}
	mem = mapm_int32(mem, (char *)&foo);
	printf("%lx\n", (long)foo);
	map_size = sizeof (welem_t) * new->mapx * new->mapy;
	ptr = malloc(map_size);
	*map = ptr;
	printf("map starting from: %p for [0x%x]%d\n", mem, map_size, map_size);
	bcopy(mem, *map, new->mapx * new->mapy);
	mem += new->mapx * new->mapy;
	mem = mapm_int32(mem, (char *)&foo);
	printf("%lx\n", (long)foo);
	printf("status starting from: %p\n", mem);
	//for (i = 0; i < new->mapx * new->mapy; ptr++, i++) {
	//	mem = mapm_int8(mem, ptr);
	//}
	map_size = sizeof (selem_t) * new->mapx * new->mapy;
	ptr = malloc(map_size);
	*flags = ptr;
	UnpackBits(mem, ptr, 2, new->mapx * new->mapy);
	for (i = 0; i < new->mapx * new->mapy; i++) {
		clearWorldFlags(i, PAINTEDBIT);
	}
	return (mem);
}

static void
write_palmstructure(GameStruct *new, char *map, char *flags, int fd)
{
	int i;
	int j;
	size_t compres_size;
	char *compres_buf;

	write(fd, new->version, 4);
	write_int8(fd, new->mapx);
	write_int8(fd, new->mapy);
	write_int8(fd, new->map_xpos);
	write_int8(fd, new->map_ypos);
	write_int32(fd, new->credits);
	write_int32(fd, new->TimeElapsed);
	write_int8(fd, new->tax);
	write_int8(fd, new->gameLoopSeconds);
	write_int8(fd, new->diff_disaster);
	write_int8(fd, new->gas_bits);
	write(fd, new->cityname, CITYNAMELEN);
	for (i = 0; i < ue_tail; i++)
		write_int8(fd, new->upkeep[i]);
	write_int8(fd, new->gridsToUpdate);
	write_int16(fd, new->desires[de_evaluation]);
	write_int16(fd, new->desires[de_residential]);
	write_int16(fd, new->desires[de_commercial]);
	write_int16(fd, new->desires[de_industrial]);
	i = 0;
	while (i < st_tail) {
		for (j = 0; j < STATS_COUNT; j++) {
			write_int16(fd, new->statistics[i].last_ten[j]);
		}
		for (j = 0; j < STATS_COUNT; j++) {
			write_int16(fd, new->statistics[i].last_century[j]);
		}
		i++;
	}

	i = 0;
	while (i < NUM_OF_UNITS) {
		write_int16(fd, new->units[i].x);
		write_int16(fd, new->units[i].y);
		write_int16(fd, new->units[i].active);
		write_int16(fd, new->units[i].type);
		i++;
	}
	i = 0;
	while (i < NUM_OF_OBJECTS) {
		write_int16(fd, new->objects[i].x);
		write_int16(fd, new->objects[i].y);
		write_int16(fd, new->objects[i].dir);
		write_int16(fd, new->objects[i].active);
		i++;
	}

	write_int32(fd, 0xffffffff);
	/* Now we write the map */
	(void) write(fd, map, new->mapx * new->mapy);
	compres_size = (new->mapx * new->mapy +
	    (((sizeof (selem_t) * 8) / 2) - 1)) / ((sizeof (selem_t) * 8) / 2);
	compres_buf = malloc(compres_size);
	PackBits(flags, compres_buf, 2, new->mapx * new->mapy);
	write_int32(fd, 0xffffffff);
	(void) write(fd, compres_buf, compres_size);
	free(compres_buf);
}

savegame_t *
savegame_open(char *filename)
{
	struct stat st;
	savegame_t *rv = (savegame_t *)calloc(1, sizeof (savegame_t));
	int fd = open(filename, O_RDONLY, 0666);
	char *buf, *buf2, *buf3;
	size_t size;

	if (rv == NULL) {
		perror("calloc");
		return (NULL);
	}

	if (fd == -1) {
		perror("open");
		free(rv);
		return (NULL);
	}

	if (-1 == fstat(fd, &st)) {
		perror("fstat");
		close(fd);
		free(rv);
		return (NULL);
	}
	size = st.st_size;

	buf = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		perror("mmap");
		close(fd);
		free(rv);
		return (NULL);
	}
	close(fd);

	if (0 == memcmp(buf, SAVEGAMEVERSION, 4)) { /* Native Savegame */
		rv->gamecount = 1;
		rv->games = calloc(1, sizeof (struct embedded_savegame));
		read_palmstructure(buf, &rv->games[0].gs,
			&rv->games[0].world, &rv->games[0].flags);
		munmap(buf, st.st_size);
		close(fd);
		return(rv);
	}

	/* It may be a palmos savegame structure ... this would be a .pdb */

	buf2 = inMem(buf, size, SAVEGAMEVERSION, 4);
	if (buf2 == NULL) {
		free(rv);
		return (NULL);
	}
	
	while (buf2 != NULL) {
		rv->gamecount++;
		rv->games = realloc(rv->games,
		    rv->gamecount * sizeof (struct embedded_savegame));
		buf3 = buf2;
		buf2 = read_palmstructure(buf2,
		    &rv->games[rv->gamecount - 1].gs,
		    &rv->games[rv->gamecount - 1].world,
		    &rv->games[rv->gamecount - 1].flags);
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
		free(sg->games[index].flags);
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
savegame_getcity(savegame_t *sg, int item, GameStruct *gs, char **map,
    char **flags)
{
	size_t newl = 0;
	if (sg == NULL || item >= sg->gamecount)
		return (-1);
	bcopy(&sg->games[item].gs, gs, sizeof (GameStruct));
	newl = (sizeof (welem_t)) * gs->mapx * gs->mapy;
	*map = (char *)realloc (*map, newl);
	bcopy(sg->games[item].world, *map, newl);
	newl = (sizeof (selem_t)) * gs->mapx * gs->mapy;
	*flags = (char *)realloc(*map, newl);
	bcopy(sg->games[item].flags, *map, newl);
	return (0);
}

int
savegame_setcity(savegame_t *sg, int item, GameStruct *gs, char *map,
    char *flags)
{
	size_t newl;

	if (map == NULL || sg == NULL || item >= sg->gamecount)
		return (-1);
	bcopy(gs, &sg->games[item].gs, sizeof (GameStruct));
	newl = sizeof (welem_t) * gs->mapx * gs->mapy;
	sg->games[item].world = realloc(sg->games[item].world, newl);
	bcopy(map, sg->games[item].world, newl);
	newl = sizeof (selem_t) * gs->mapx * gs->mapy;
	sg->games[item].flags = realloc(sg->games[item].flags, newl);
	bcopy(flags, sg->games[item].flags, newl);
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
	if (worldPtr != NULL) {
		free(worldPtr);
		worldPtr = NULL;
	}
	if (flagPtr != NULL) {
		free(flagPtr);
		flagPtr = NULL;
	}
	savegame_getcity(sg, 0, &game, (char **)&worldPtr,
	    (char **)&flagPtr);
	savegame_close(sg);
	return (0);
}

int
save_defaultfilename()
{
	return(save_filename(getCityFileName(), &game, worldPtr, flagPtr));
}

int
save_filename(char *sel, GameStruct *gs, char *world, char *flags)
{
	int fd;//, ret;
	//ssize_t worldsize = gs->mapx * gs->mapy * (sizeof (welem_t));
	//ssize_t flagssize = gs->mapx * gs->mapy * (sizeof (selem_t));
	//char *nbuf = NULL;
	//ssize_t compres_size;

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
	
	write_palmstructure(gs, world, flags, fd);
	/*
	ret = write(fd, (void*)gs, sizeof (GameStruct));
	if (ret == -1) {
		perror("write game"); / * TODO: make this nicer * /
		return (-1);
	} else if (ret != sizeof (GameStruct)) {
		WriteLog("Whoops, couldn't write full length of game\n");
		return (-1);
	}

	/ * and now the great worldPtr :D * /
	ret = write(fd, (void*)world, (size_t)worldsize);
	if (ret == -1) {
		perror("write world"); / * TODO: make this nicer * /
		return (-1);
	} else if (ret != worldsize) {
		WriteLog("Whoops, couldn't write full length of world\n");
		return (-1);
	}

	compres_size = (gs->mapx * gs->mapy +  (sizeof (selem_t))) /
	    (sizeof (selem_t) / 2);
	nbuf = malloc(compres_size);
	PackBits(worldFlags, nbuf, 2, gs->mapx * gs->mapy);
	ret = write(fd, (void *)nbuf, (size_t)compres_size);
	if (ret == -1) {
		perror("write world flags"); / * TODO: make this nicer * /
		free(nbuf);
		return (-1);
	} else if (ret != compres_size) {
		WriteLog("Whoops, couldn't write the full length of world\n");
		free(nbuf);
		return (-1);
	}
	free(nbuf);
	*/
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
