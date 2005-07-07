/*!
 * \file
 * \brief Palm application configuration.
 *
 * This is information that is saved with the application upon termination.
 * It contains information that is not considered
 * suitable for storing with an actual game, but is instead a game based
 * option.
 */
#if !defined(_APPCONFIG_H_)
#define	_APPCONFIG_H_

#include <PalmTypes.h>
#include <sections.h>

/*!
 * \brief the button events
 *
 * The order here matches the string table StrID_Popups in game.rcp
 * As well as the BuildCodes in build.h. If you change one you need to
 * change all
 */
typedef enum skeyEvent {
	keIgnore = 0,
	keUp,
	keDown,
	keLeft,
	keRight,
	kePopup,
	keMap,
	keBudget,
	kePopulation,
	kePassthrough,
	keToolBulldozer,
	keToolResidential,
	keToolCommercial,
	keToolIndustrial,
	keToolRoad,
	keToolRail,
	keToolCoalPlant,
	keToolNuclearPlant,
	keToolPowerLine,
	keToolWaterPump,
	keToolWaterPipe,
	keToolTree,
	keToolLake,
	keToolFireStation,
	keToolPoliceStation,
	keToolArmyBase,
	keToolQuery,
	keUnitFire,
	keUnitPolice,
	keUnitArmy,
#ifdef SONY_CLIE
	keJogUp,
	keJogDown,
	keJogRelease,
#endif /* SONY_CLIE */
	keEnd /* unused */
} keyEvent;

/*! \brief the keys */
typedef enum buttonKeys {
	BkCalendar = 0,
	BkAddress,
	BkHardUp,
	BkHardDown,
	BkToDo,
	BkMemo,
	BkCalc, /* This is not a real key! */
	BkFind, /* This is not a real key! */
#if defined(PALM_HIGH) || defined(SONY_CLIE)
	BkHardLeft,
	BkHardRight,
	BkRockerCenter,
#if defined(SONY_CLIE)
	BkJogUp,
	BkJogDown,
	BkJogRelease,
#endif /* SONY_CLIE */
#endif /* HIRES */
	BkEnd /* Unused */
} ButtonKey;

/*! \brief the palm application configuration */
typedef struct _PalmAppConfig_01 {
	keyEvent	keyOptions[BkEnd];
} PalmAppConfig_01_t;

/*! \brief the palm application configuration */
typedef PalmAppConfig_01_t PlatformAppConfig;

#if defined(PALM_HIGH) || defined(SONY_HIGH)
#define HIRESKEY , keLeft, keRight, keIgnore
#else
#define HIRESKEY
#endif

#if defined(SONY_CLIE)
#define	SONYEV , keJogUp, keJogDown, keJogRelease
#else
#define	SONYEV
#endif

/*! \brief Palm Default application configuration */
#define	DEFAULT_APPCONFIG { \
	{ kePassthrough, keLeft, keUp, keDown, keRight, kePassthrough, \
		kePopup, keMap HIRESKEY SONYEV } \
}

#endif /* _APPCONFIG_H_ */
