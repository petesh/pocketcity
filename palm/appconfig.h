#if !defined(_APPCONFIG_H)
#define _APPCONFIG_H

typedef enum ButtonEvent {
    BeIgnore = 0,
    BeUp,
    BeDown,
    BeLeft,
    BeRight,
    BePopup,
    BeMap,
#ifdef SONY_CLIE
    BeJogUp,
    BeJogDown,
    BeJogRelease,
#endif
    BePassthrough
} ButtonEvent;

typedef enum buttonKeys {
    BkCalendar = 0,
    BkAddress,
    BkHardUp,
    BkHardDown,
    BkToDo,
    BkMemo,
#ifdef SONY_CLIE
    BkJogUp,
    BkJogDown,
    BkJogRelease,
#endif
    BkCalc,
    BkFind,
    BkEnd /* Unused */
} ButtonKey;

/* enum -> count?? */
#ifdef SONY_CLIE
#define BKCOUNT 11
#else
#define BKCOUNT 8
#endif

typedef struct _PalmAppConfig_01 {
    ButtonKey           keyOptions[BKCOUNT];
} PalmAppConfig_01_t;

typedef PalmAppConfig_01_t PlatformAppConfig;

#ifdef SONY_CLIE
#define SONYEV BeJogUp, BeJogDown, BeJogRelease,
#else
#define SONYEV
#endif

#define DEFAULT_APPCONFIG       { \
    { BePassthrough, BeLeft, BeUp, BeDown, BeRight, BePassthrough, \
      SONYEV BePopup, BeMap } \
}

#endif /* _APPCONFIG_H */