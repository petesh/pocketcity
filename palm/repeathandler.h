#include <PalmOS.h>
#include <sections.h>

typedef struct buttonmapping_tag {
	UInt16		down; /*!< repeat button for down */
	UInt16		up; /*!< repeat button for up */
	UInt16		field; /*!< field value */
	Int16		min; /*!< minimum value to set field to */
	Int16		max; /*!< maximum value to set field to */
	UInt32		special1; /*!< special field one (custom use) */
	UInt32		special2; /*!< special field two (custom use) */
} buttonmapping_t;

typedef void (*bmPostHandler)(UInt16 button, buttonmapping_t *mapping,
    UInt32 newValue);

buttonmapping_t *getSpinnerFieldIndex(buttonmapping_t *map,
    UInt16 buttonControl, Boolean isButton) OTHER_SECTION;

buttonmapping_t *processRepeater(buttonmapping_t *map, UInt16 control,
    Boolean isButton, bmPostHandler post_handle) OTHER_SECTION;
