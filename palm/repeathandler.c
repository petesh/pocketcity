/*!
 * \file
 * \brief utility code for repeat handlers
 *
 * Code to deal with the spinners and the fields that they relate to.
 */
#include <PalmOS.h>
#include <StringMgr.h>
#include <Form.h>
#include <MemoryMgr.h>
#include <Field.h>
#include <repeathandler.h>
#include <palmutils.h>
#include <mem_compat.h>

#define BUTTONMAPLEN	(sizeof (buttonmappings) / sizeof (buttonmappings[0]))

buttonmapping_t *
getSpinnerFieldIndex(buttonmapping_t *mapping, UInt16 buttonControl,
    Boolean isButton)
{
	ErrFatalDisplayIf(mapping == NULL, "Bad Function call");

	while (mapping->down != 0 && mapping->up != 0) {
		if (isButton) {
			if (buttonControl == mapping->down ||
			    buttonControl == mapping->up)
				return (mapping);
		} else {
			if (buttonControl == mapping->field)
				return (mapping);
		}
		mapping++;
	}
	return (NULL);
}

buttonmapping_t *
processRepeater(buttonmapping_t *map, UInt16 control,
    Boolean isButton, bmPostHandler post_handle)
{
	buttonmapping_t *bm = getSpinnerFieldIndex(map, control,
	    isButton);
	FieldPtr fp;
	MemHandle mh;
	MemPtr mp;
	Int32 fld;
	Boolean limited = false;

	if (bm == NULL) return (NULL);

	fp = (FieldPtr)GetObjectPtr(FrmGetActiveForm(), bm->field);
	mh = FldGetTextHandle(fp);

	FldSetTextHandle(fp, NULL);
	mp = MemHandleLock(mh);

	fld = StrAToI(mp);

	if (isButton) {
		if (control == bm->down)
			fld--;
		else
			fld++;
	}
	if (fld < bm->min) {
		limited = true;
		fld = bm->min;
	} else if (fld > bm->max) {
		limited = true;
		fld = bm->max;
	}

	if (isButton || (!isButton && limited))
		StrPrintF(mp, "%ld", fld);

	MemHandleUnlock(mh);
	FldSetTextHandle(fp, mh);
	FldDrawField(fp);

	if (post_handle != NULL)
		post_handle(control, bm, fld);

	return (bm);
}
