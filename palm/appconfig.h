#if !defined(_APPCONFIG_H)
#define _APPCONFIG_H

typedef enum ButtonEvent {
    BeIgnore = 0, BeUp, BeDown, BeLeft, BeRight, BePassthrough
} ButtonEvent;

typedef enum buttonKeys {
    BkCalendar = 0, BkAddress, BkHardUp, BkHardDown, BkToDo, BkMemo
} ButtonKey;

/* enum -> count?? */
#define BKCOUNT 6

typedef struct _PalmAppConfig_01 {
    ButtonKey           keyOptions[BKCOUNT];
} PalmAppConfig_01_t;

typedef PalmAppConfig_01_t PlatformAppConfig;

#define DEFAULT_APPCONFIG       { \
    { BePassthrough, BeLeft, BeUp, BeDown, BeRight, BePassthrough } \
}

#endif /* _APPCONFIG_H */
