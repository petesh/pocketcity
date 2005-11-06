/*!
 * \file
 * \brief interface routines to the simulation
 */
#if !defined(_DISTRIBUTION_H_)
#define	_DISTRIBUTION_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <zakdef.h>
#include <compilerpragmas.h>

/*! \brief the bit associated with power in the worldflags */
#define	POWEREDBIT	((unsigned char)0x01)
/*! \brief the bit associated with water in the worldflags */
#define	WATEREDBIT	((unsigned char)0x02)
/*! \brief the number of fits saved into the savegame */
#define	SAVEDBITS	(2)

/*! \brief the bit associated with scratch/unvisited in the worldflags */
#define	SCRATCHBIT	((unsigned char)0x80)
/*! \brief the bit associated with knowing if the field has been painted */
#define	PAINTEDBIT	((unsigned char)0x40)

/*! \brief Up Direction */
#define	DIR_UP		(1<<0)
/*! \brief Down Direction */
#define	DIR_DOWN	(1<<1)
/*! \brief Vertical Direction */
#define	DIR_VER		((DIR_UP) | (DIR_DOWN))
/*! \brief Left Direction */
#define	DIR_LEFT	(1<<2)
/*! \brief Right Direction */
#define	DIR_RIGHT	(1<<3)
/*! \brief Horizontal Direction */
#define	DIR_HOR		((DIR_LEFT) | (DIR_RIGHT))
/*! \brief All Directions */
#define	DIR_ALL		((DIR_HOR) | (DIR_VER))

#define	getScratch(i) (getWorldFlags(i) & SCRATCHBIT)
#define	setScratch(i) orWorldFlags((i), SCRATCHBIT)
#define	unsetScratch(i) andWorldFlags((i), (selem_t)~SCRATCHBIT)
#define	clearScratch() { \
	UInt32 XXX = 0; \
	for (; XXX < MapMul(); XXX++) unsetScratch(XXX); \
}

/*!
 * \brief Do a grid distribution for the grid(s) specified
 * \param gridonly the grid to do
 */
EXPORT void Sim_Distribute_Specific(Int16 gridonly);

#if defined(__cplusplus)
}
#endif

#endif /* _DISTRIBUTION_H_ */
