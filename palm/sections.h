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
#define	MAP_SECTION LARD_SECTION
#define	BUDGET_SECTION LARD_SECTION
#define SAVE_SECTION LARD_SECTION
#define BUILD_SECTION LARD_SECTION
#define DISASTER_SECTION LARD_SECTION
#define CONFIG_SECTION LARD_SECTION
#define REPEATH_SECTION LARD_SECTION
#define QUERY_SECTION LARD_SECTION
#define HIRES_SECTION LARD_SECTION
#define PACK_SECTION LARD_SECTION

#endif /* _SECTIONS_H_ */
