/*!
 * \file
 * \brief this just nulls-out the *_SECTION in function definitions
 *
 * This it to allow the code to compile in a non-segmented architecture
 */
#if !defined(_SECTIONS_H_)
#define _SECTIONS_H_

#define MAP_SECTION
#define OTHER_SECTION
#define BUDGET_SECTION
#define SAVE_SECTION
#define BUILD_SECTION
#define DISASTER_SECTION
#define PACK_SECTION

#endif /* _SECTIONS_H_ */

