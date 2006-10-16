/*!
 * \file
 * \brief mapping for sections for functions
 *
 * This is the declaration of what section a function exists in.
 *
 * If you declare it's interface you need to use the appropriate section
 * otherwise you get 'mystery' crashes in the aplm application as is calls
 * into data or another completely different function.
 */
#if !defined(_SECTIONS_H_)
#define	_SECTIONS_H_

#define	LARD_SECTION __attribute__((section("lard")))
#define	DISTRIBUTION_SECTION __attribute__((section("lard2")))
#define SECTION_1 LARD_SECTION
#define SECTION_2 LARD_SECTION
#define SECTION_3 LARD_SECTION

#if defined LOGGING
#define DRAWING_SECTION SECTION_1
#else
#define DRAWING_SECTION
#endif

#define	MAP_SECTION SECTION_1
#define	BUDGET_SECTION SECTION_1
#define	SAVE_SECTION SECTION_1
#define	HIRES_SECTION SECTION_1

#define	BUILD_SECTION SECTION_2
#define	DISASTER_SECTION SECTION_2
#define	CONFIG_SECTION SECTION_2
#define	REPEATH_SECTION SECTION_2

#define	QUERY_SECTION SECTION_3
#define	PACK_SECTION SECTION_3
#define	BEAM_SECTION SECTION_3
#define	MINIMAP_SECTION SECTION_3

#if defined(LOGGING)
#define SIMCITY_SECTION SECTION_3
#else
#define SIMCITY_SECTION
#endif

#if defined(DEBUG)
#define	SONY_SECTION LARD_SECTION
#else
#define	SONY_SECTION
#endif

#endif /* _SECTIONS_H_ */
