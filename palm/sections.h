#if !defined(_SECTIONS_H_)
#define	_SECTIONS_H_

#define	MAP_SECTION __attribute__((section("lard")))
#define	OTHER_SECTION MAP_SECTION
#define	BUDGET_SECTION MAP_SECTION
#define SAVE_SECTION MAP_SECTION
#define BUILD_SECTION MAP_SECTION

#endif /* _SECTIONS_H_ */
