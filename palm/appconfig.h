#if !defined(_APPCONFIG_H)
#define	_APPCONFIG_H

#include <PalmTypes.h>
#include <sections.h>

/*
 * if you add in here don't forget the entries in game.rcp for the
 * configuration screen.
 */
typedef enum ButtonEvent {
	BeIgnore = 0,
	BeUp,
	BeDown,
	BeLeft,
	BeRight,
	BePopup,
	BeMap,
	BeBudget,
	BePopulation,
	BePassthrough,
#ifdef SONY_CLIE
	BeJogUp,
	BeJogDown,
	BeJogRelease,
#endif
	BeEnd /* unused */
} ButtonEvent;

typedef enum buttonKeys {
	BkCalendar = 0,
	BkAddress,
	BkHardUp,
	BkHardDown,
	BkToDo,
	BkMemo,
	BkCalc,
	BkFind,
#ifdef SONY_CLIE
	BkJogUp,
	BkJogDown,
	BkJogRelease,
#endif
	BkEnd /* Unused */
} ButtonKey;

typedef struct _PalmAppConfig_01 {
	ButtonKey	keyOptions[BeEnd];
} PalmAppConfig_01_t;

typedef PalmAppConfig_01_t PlatformAppConfig;

#ifdef SONY_CLIE
#define	SONYEV , BeJogUp, BeJogDown, BeJogRelease
#else
#define	SONYEV
#endif

#define	DEFAULT_APPCONFIG { \
	{ BePassthrough, BeLeft, BeUp, BeDown, BeRight, BePassthrough, \
		BePopup, BeMap SONYEV } \
}

#endif /* _APPCONFIG_H */
