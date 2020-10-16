/*
 * << Haru Free PDF Library >> -- hpdf_annotation.c
 *
 * URL: http://libharu.org
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 * Copyright (c) 2007-2009 Antony Dovgal <tony@daylessday.org>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */

#include "hpdf/conf.h"
#include "hpdf/utils.h"
#include "hpdf/info.h"
#include "hpdf/annotat.h"
#include "hpdf.h"
#include <string.h>

static const char *const HPDF_ANNOT_TYPE_NAMES[] = {
	"Text",
	"Link",
	"FreeText",
	"Line",
	"Square",
	"Circle",
	"Polygon",
	"PolyLine",
	"Highlight",
	"Underline",
	"Squiggly",
	"StrikeOut",
	"Stamp",
	"Caret",
	"Ink",
	"Popup",
	"FileAttachment",
	"Sound",
	"Movie",
	"Widget",
	"Screen",
	"PrinterMark",
	"TrapNet",
	"Watermark",
	"3D",
	"Redact",
	"Projection",
};

static const char *const HPDF_ANNOT_ICON_NAMES_NAMES[] = {
	"Comment",
	"Key",
	"Note",
	"Help",
	"NewParagraph",
	"Paragraph",
	"Insert"
};

static const char *const HPDF_ANNOT_INTENT_NAMES[] = {
	"FreeTextCallout",
	"FreeTextTypeWriter",
	"LineArrow",
	"LineDimension",
	"PolygonCloud",
	"PolyLineDimension",
	"PolygonDimension"
};

static const char *const HPDF_LINE_ANNOT_ENDING_STYLE_NAMES[] = {
	"None",
	"Square",
	"Circle",
	"Diamond",
	"OpenArrow",
	"ClosedArrow",
	"Butt",
	"ROpenArrow",
	"RClosedArrow",
	"Slash"
};

static const char *const HPDF_LINE_ANNOT_CAP_POSITION_NAMES[] = {
	"Inline",
	"Top"
};

static const char *const HPDF_STAMP_ANNOT_NAME_NAMES[] = {
	"Approved",
	"Experimental",
	"NotApproved",
	"AsIs",
	"Expired",
	"NotForPublicRelease",
	"Confidential",
	"Final",
	"Sold",
	"Departmental",
	"ForComment",
	"TopSecret",
	"Draft",
	"ForPublicRelease"
};

/*----------------------------------------------------------------------------*/
/*------ HPDF_Annotation -----------------------------------------------------*/


HPDF_Annotation HPDF_Annotation_New(HPDF_MMgr mmgr, HPDF_Xref xref, HPDF_AnnotType type, const HPDF_Rect *rect)
{
	HPDF_Annotation annot;
	HPDF_Array array;
	HPDF_STATUS ret = HPDF_OK;
	HPDF_REAL bottom, top;

	annot = HPDF_Dict_New(mmgr);
	if (!annot)
		return NULL;

	if (HPDF_Xref_Add(xref, annot) != HPDF_OK)
		return NULL;

	array = HPDF_Array_New(mmgr);
	if (!array)
		return NULL;

	if (HPDF_Dict_Add(annot, "Rect", array) != HPDF_OK)
		return NULL;

	if (rect->top < rect->bottom)
	{
		top = rect->bottom;
		bottom = rect->top;
	} else
	{
		top = rect->top;
		bottom = rect->bottom;
	}

	ret += HPDF_Array_AddReal(array, rect->left);
	ret += HPDF_Array_AddReal(array, bottom);
	ret += HPDF_Array_AddReal(array, rect->right);
	ret += HPDF_Array_AddReal(array, top);

	ret += HPDF_Dict_AddName(annot, "Type", "Annot");
	ret += HPDF_Dict_AddName(annot, "Subtype", HPDF_ANNOT_TYPE_NAMES[(HPDF_INT) type]);

	if (ret != HPDF_OK)
		return NULL;

	annot->header.obj_class |= HPDF_OSUBCLASS_ANNOTATION;

	return annot;
}


HPDF_STATUS HPDF_Annotation_SetBorderStyle(HPDF_Annotation annot,
	HPDF_BSSubtype subtype,
	HPDF_REAL width,
	HPDF_UINT16 dash_on,
	HPDF_UINT16 dash_off,
	HPDF_UINT16 dash_phase)
{
	HPDF_Dict bs;
	HPDF_Array dash;
	HPDF_STATUS ret;

	bs = HPDF_Dict_New(annot->mmgr);
	if (!bs)
		return HPDF_Error_GetCode(annot->error);

	if ((ret = HPDF_Dict_Add(annot, "BS", bs)) != HPDF_OK)
		return ret;

	if (subtype == HPDF_BS_DASHED)
	{
		dash = HPDF_Array_New(annot->mmgr);
		if (!dash)
			return HPDF_Error_GetCode(annot->error);

		if ((ret = HPDF_Dict_Add(bs, "D", dash)) != HPDF_OK)
			return ret;

		ret += HPDF_Dict_AddName(bs, "Type", "Border");
		ret += HPDF_Array_AddReal(dash, dash_on);
		ret += HPDF_Array_AddReal(dash, dash_off);

		if (dash_phase != 0)
			ret += HPDF_Array_AddReal(dash, dash_off);
	}

	switch (subtype)
	{
	case HPDF_BS_SOLID:
		ret += HPDF_Dict_AddName(bs, "S", "S");
		break;
	case HPDF_BS_DASHED:
		ret += HPDF_Dict_AddName(bs, "S", "D");
		break;
	case HPDF_BS_BEVELED:
		ret += HPDF_Dict_AddName(bs, "S", "B");
		break;
	case HPDF_BS_INSET:
		ret += HPDF_Dict_AddName(bs, "S", "I");
		break;
	case HPDF_BS_UNDERLINED:
		ret += HPDF_Dict_AddName(bs, "S", "U");
		break;
	default:
		return HPDF_SetError(annot->error, HPDF_ANNOT_INVALID_BORDER_STYLE, 0);
	}

	if (width != HPDF_BS_DEF_WIDTH)
		ret += HPDF_Dict_AddReal(bs, "W", width);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(annot->error);

	return HPDF_OK;
}


HPDF_Annotation HPDF_WidgetAnnot_New(HPDF_MMgr mmgr, HPDF_Xref xref, const HPDF_Rect *rect)
{
	HPDF_Annotation annot;

	annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_WIDGET, rect);
	if (!annot)
		return NULL;

	return annot;
}


HPDF_Annotation HPDF_LinkAnnot_New(HPDF_MMgr mmgr, HPDF_Xref xref, const HPDF_Rect *rect, HPDF_Destination dst)
{
	HPDF_Annotation annot;

	annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_LINK, rect);
	if (!annot)
		return NULL;

	if (dst)
	{
		if (HPDF_Dict_Add(annot, "Dest", dst) != HPDF_OK)
			return NULL;
	}

	return annot;
}


static HPDF_Annotation HPDF_ActionAnnot_New(HPDF_MMgr mmgr, HPDF_Xref xref, const HPDF_Rect *rect, const char *type, const char *dst, void *obj)
{
	HPDF_Annotation annot;
	HPDF_Dict action;
	HPDF_STATUS ret;

	annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_LINK, rect);
	if (!annot)
		return NULL;

	/* create action dictionary */
	action = HPDF_Dict_New(mmgr);
	if (!action)
		return NULL;

	ret = HPDF_Dict_Add(annot, "A", action);
	if (ret != HPDF_OK)
		return NULL;

	ret += HPDF_Dict_AddName(action, "S", type);
	if (obj)
		ret += HPDF_Dict_Add(action, dst, obj);

	if (ret != HPDF_OK)
		return NULL;

	return annot;
}


HPDF_Annotation HPDF_URILinkAnnot_New(HPDF_MMgr mmgr, HPDF_Xref xref, const HPDF_Rect *rect, const char *uri)
{
	return HPDF_ActionAnnot_New(mmgr, xref, rect, "URI", "URI", HPDF_String_New(mmgr, uri, NULL));
}


HPDF_Annotation HPDF_GoToLinkAnnot_New(HPDF_MMgr mmgr, HPDF_Xref xref, const HPDF_Rect *rect, HPDF_Destination dst)
{
	return HPDF_ActionAnnot_New(mmgr, xref, rect, "GoTo", "D", dst);
}


HPDF_Annotation HPDF_GoToRLinkAnnot_New(HPDF_MMgr mmgr, HPDF_Xref xref, const HPDF_Rect *rect, const char *file, const char *destname, HPDF_BOOL newwindow)
{
	HPDF_Annotation annot;
	HPDF_String dst = NULL;
	
	if (destname)
		dst = HPDF_String_New(mmgr, destname, NULL);
	annot = HPDF_ActionAnnot_New(mmgr, xref, rect, "GoToR", "D", dst);
	if (annot)
	{
		HPDF_Dict action = (HPDF_Dict) HPDF_Dict_GetItem(annot, "A", HPDF_OCLASS_DICT);
		if (HPDF_Dict_Add(action, "F", HPDF_String_New(mmgr, file, NULL)) != HPDF_OK)
			return NULL;
		if (newwindow)
			if (HPDF_Dict_Add(action, "NewWindow", HPDF_Boolean_New(mmgr, newwindow)) != HPDF_OK)
				return NULL;
	}
	return annot;
}


HPDF_Annotation HPDF_NamedLinkAnnot_New(HPDF_MMgr mmgr, HPDF_Xref xref, const HPDF_Rect *rect, const char *type)
{
	HPDF_Annotation annot;
	HPDF_Dict action;
	HPDF_STATUS ret;

	annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_LINK, rect);
	if (!annot)
		return NULL;

	/* create action dictionary */
	action = HPDF_Dict_New(mmgr);
	if (!action)
		return NULL;

	ret = HPDF_Dict_Add(annot, "A", action);
	if (ret != HPDF_OK)
		return NULL;

	ret += HPDF_Dict_AddName(action, "S", "Named");
	ret += HPDF_Dict_AddName(action, "N", type);

	if (ret != HPDF_OK)
		return NULL;

	return annot;
}


HPDF_Annotation HPDF_LaunchLinkAnnot_New(HPDF_MMgr mmgr, HPDF_Xref xref, const HPDF_Rect *rect, const char *file, const char *args, const char *type)
{
	HPDF_Annotation annot;
	HPDF_Dict win;
	
	annot = HPDF_ActionAnnot_New(mmgr, xref, rect, "Launch", "F", HPDF_String_New(mmgr, file, NULL));
	if (annot)
	{
		HPDF_Dict action = (HPDF_Dict) HPDF_Dict_GetItem(annot, "A", HPDF_OCLASS_DICT);
		if (HPDF_Dict_Add(action, "F", HPDF_String_New(mmgr, file, NULL)) != HPDF_OK)
			return NULL;
		win = HPDF_Dict_New(mmgr);
		if (HPDF_Dict_Add(action, "NewWindow", HPDF_Boolean_New(mmgr, HPDF_TRUE)) != HPDF_OK)
			return NULL;
		if (HPDF_Dict_Add(action, "Win", win) != HPDF_OK)
			return NULL;
		if (HPDF_Dict_Add(win, "F", HPDF_String_New(mmgr, file, NULL)) != HPDF_OK)
			return NULL;
		if (args)
			if (HPDF_Dict_Add(win, "P", HPDF_String_New(mmgr, type, NULL)) != HPDF_OK)
				return NULL;
		if (type)
			if (HPDF_Dict_Add(win, "O", HPDF_String_New(mmgr, type, NULL)) != HPDF_OK)
				return NULL;
	}
	return annot;
}


static HPDF_BOOL CheckSubType(HPDF_Annotation annot, HPDF_AnnotType type)
{
	HPDF_Name subtype;

	if (!HPDF_Annotation_Validate(annot))
		return HPDF_FALSE;

	subtype = (HPDF_Name) HPDF_Dict_GetItem(annot, "Subtype", HPDF_OCLASS_NAME);

	if (!subtype || strcmp(subtype->value, HPDF_ANNOT_TYPE_NAMES[(HPDF_INT) type]) != 0)
	{
		HPDF_RaiseError(annot->error, HPDF_INVALID_ANNOTATION, 0);
		return HPDF_FALSE;
	}

	return HPDF_TRUE;
}


HPDF_STATUS HPDF_LinkAnnot_SetJavaScript(HPDF_Annotation annot, HPDF_JavaScript javascript)
{
	HPDF_Dict action;
	HPDF_STATUS ret;

	if (!CheckSubType(annot, HPDF_ANNOT_LINK))
		return HPDF_INVALID_ANNOTATION;

	/* create action dictionary */
	action = HPDF_Dict_New(annot->mmgr);
	if (!action)
		return HPDF_CheckError(annot->error);

	ret = HPDF_Dict_Add(annot, "A", action);
	if (ret != HPDF_OK)
		return HPDF_CheckError(annot->error);

	ret += HPDF_Dict_Add(action, "JS", javascript);
	ret += HPDF_Dict_AddName(action, "S", "JavaScript");

	if (ret != HPDF_OK)
		return HPDF_CheckError(annot->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_LinkAnnot_SetBorderStyle(HPDF_Annotation annot, HPDF_REAL width, HPDF_UINT16 dash_on, HPDF_UINT16 dash_off)
{
	HPDF_Array array;
	HPDF_STATUS ret;

	if (!CheckSubType(annot, HPDF_ANNOT_LINK))
		return HPDF_INVALID_ANNOTATION;

	if (width < 0)
		return HPDF_RaiseError(annot->error, HPDF_INVALID_PARAMETER, 0);

	array = HPDF_Array_New(annot->mmgr);
	if (!array)
		return HPDF_CheckError(annot->error);

	if ((ret = HPDF_Dict_Add(annot, "Border", array)) != HPDF_OK)
		return HPDF_CheckError(annot->error);

	ret += HPDF_Array_AddNumber(array, 0);
	ret += HPDF_Array_AddNumber(array, 0);
	ret += HPDF_Array_AddReal(array, width);

	if (ret != HPDF_OK)
		return HPDF_CheckError(annot->error);

	if (dash_on && dash_off)
	{
		HPDF_Array dash = HPDF_Array_New(annot->mmgr);

		if (!dash)
			return HPDF_CheckError(annot->error);

		if ((ret = HPDF_Array_Add(array, dash)) != HPDF_OK)
			return HPDF_CheckError(annot->error);

		ret += HPDF_Array_AddNumber(dash, dash_on);
		ret += HPDF_Array_AddNumber(dash, dash_off);

		if (ret != HPDF_OK)
			return HPDF_CheckError(annot->error);
	}

	return HPDF_OK;
}


HPDF_STATUS HPDF_LinkAnnot_SetHighlightMode(HPDF_Annotation annot, HPDF_AnnotHighlightMode mode)
{
	HPDF_STATUS ret;

	if (!CheckSubType(annot, HPDF_ANNOT_LINK))
		return HPDF_INVALID_ANNOTATION;

	switch (mode)
	{
	case HPDF_ANNOT_NO_HIGHLIGHT:
		ret = HPDF_Dict_AddName(annot, "H", "N");
		break;
	case HPDF_ANNOT_INVERT_BORDER:
		ret = HPDF_Dict_AddName(annot, "H", "O");
		break;
	case HPDF_ANNOT_DOWN_APPEARANCE:
		ret = HPDF_Dict_AddName(annot, "H", "P");
		break;
	default:							/* HPDF_ANNOT_INVERT_BOX */
		/* default value */
		HPDF_Dict_RemoveElement(annot, "H");
		ret = HPDF_OK;
		break;
	}

	if (ret != HPDF_OK)
		return HPDF_CheckError(annot->error);

	return ret;
}


HPDF_Annotation HPDF_3DAnnot_New(HPDF_MMgr mmgr,
	HPDF_Xref xref,
	const HPDF_Rect *rect,
	HPDF_BOOL tb,
	HPDF_BOOL np,
	HPDF_U3D u3d,
	HPDF_Image ap)
{
	HPDF_Annotation annot;
	HPDF_Dict action, appearance, stream;
	HPDF_STATUS ret;

	annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_3D, rect);
	if (!annot)
	{
		return NULL;
	}
	/* include the flags */
	HPDF_Dict_AddNumber(annot, "F", 68);
	/*
	 * Bit 3:Print If set, print the annotation when the page is printed.
	 * Bit 7:If set, do not allow the annotation to interact with the user.
	 *      The annotation may be displayed or printed (depending on the settings of the NoView and Print flags)
	 *      but should not respond to mouse clicks or change its appearance in response to mouse motions.
	 */

	HPDF_Dict_Add(annot, "Contents", HPDF_String_New(mmgr, "3D Model", NULL));

	action = HPDF_Dict_New(mmgr);
	if (!action)
	{
		return NULL;
	}

	ret = HPDF_Dict_Add(annot, "3DA", action);
	if (ret != HPDF_OK)
	{
		return NULL;
	}
	/* enable visibility on page open */
	ret += HPDF_Dict_AddName(action, "A", "PO");

	/* enable visibility of ToolBar */
	ret += HPDF_Dict_AddBoolean(action, "TB", tb);

	/* enable visibility of Navigation Panel */
	ret += HPDF_Dict_AddBoolean(action, "NP", np);

	/* Set behavior of Annotation on Disabling */
	ret += HPDF_Dict_AddName(action, "DIS", "U");

	/* Set behavior of Annotation upon activation */
	ret += HPDF_Dict_AddName(action, "AIS", "L");

	if (ret != HPDF_OK)
	{
		return NULL;
	}

	if (HPDF_Dict_Add(annot, "3DD", u3d) != HPDF_OK)
	{
		return NULL;
	}

	appearance = HPDF_Dict_New(mmgr);
	if (!appearance)
	{
		return NULL;
	}

	ret = HPDF_Dict_Add(annot, "AP", appearance);
	if (ret != HPDF_OK)
	{
		return NULL;
	}

	if (ap)
	{
		if (HPDF_Dict_Add(appearance, "N", ap) != HPDF_OK)
			return NULL;
	} else
	{
		stream = HPDF_Dict_New(mmgr);
		if (!stream)
		{
			return NULL;
		}
		ret = HPDF_Dict_Add(appearance, "N", stream);
	}

	if (ret != HPDF_OK)
	{
		return NULL;
	}

	return annot;
}


HPDF_Annotation HPDF_MarkupAnnot_New(HPDF_MMgr mmgr,
	HPDF_Xref xref,
	const HPDF_Rect *rect,
	const char *text,
	HPDF_Encoder encoder, HPDF_AnnotType subtype)
{
	HPDF_Annotation annot;
	HPDF_String s;

	annot = HPDF_Annotation_New(mmgr, xref, subtype, rect);
	if (!annot)
		return NULL;

	s = HPDF_String_New(mmgr, text, encoder);
	if (!s)
		return NULL;

	if (HPDF_Dict_Add(annot, "Contents", s) != HPDF_OK)
		return NULL;

	return annot;
}


HPDF_STATUS HPDF_Annot_SetRGBColor(HPDF_Annotation annot, const HPDF_RGBColor *color)
{
	HPDF_Array cArray;
	HPDF_STATUS ret = HPDF_OK;

	cArray = HPDF_Array_New(annot->mmgr);
	if (!cArray)
		return HPDF_Error_GetCode(annot->error);

	ret += HPDF_Dict_Add(annot, "C", cArray);
	ret += HPDF_Array_AddReal(cArray, color->r);
	ret += HPDF_Array_AddReal(cArray, color->g);
	ret += HPDF_Array_AddReal(cArray, color->b);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(annot->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_Annot_SetCMYKColor(HPDF_Annotation annot, const HPDF_CMYKColor *color)
{
	HPDF_Array cArray;
	HPDF_STATUS ret = HPDF_OK;

	cArray = HPDF_Array_New(annot->mmgr);
	if (!cArray)
		return HPDF_Error_GetCode(annot->error);

	ret += HPDF_Dict_Add(annot, "C", cArray);
	ret += HPDF_Array_AddReal(cArray, color->c);
	ret += HPDF_Array_AddReal(cArray, color->m);
	ret += HPDF_Array_AddReal(cArray, color->y);
	ret += HPDF_Array_AddReal(cArray, color->k);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(annot->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_Annot_SetGrayColor(HPDF_Annotation annot, HPDF_REAL color)
{
	HPDF_Array cArray;
	HPDF_STATUS ret = HPDF_OK;

	cArray = HPDF_Array_New(annot->mmgr);
	if (!cArray)
		return HPDF_Error_GetCode(annot->error);

	ret += HPDF_Dict_Add(annot, "C", cArray);
	ret += HPDF_Array_AddReal(cArray, color);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(annot->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_Annot_SetNoColor(HPDF_Annotation annot)
{
	HPDF_Array cArray;
	HPDF_STATUS ret = HPDF_OK;

	cArray = HPDF_Array_New(annot->mmgr);
	if (!cArray)
		return HPDF_Error_GetCode(annot->error);

	ret = HPDF_Dict_Add(annot, "C", cArray);

	return ret;
}


HPDF_STATUS HPDF_TextAnnot_SetIcon(HPDF_Annotation annot, HPDF_AnnotIcon icon)
{
	if (!CheckSubType(annot, HPDF_ANNOT_TEXT))
		return HPDF_INVALID_ANNOTATION;

	if (icon >= HPDF_ANNOT_ICON_EOF)
		return HPDF_RaiseError(annot->error, HPDF_ANNOT_INVALID_ICON, (HPDF_STATUS) icon);

	if (HPDF_Dict_AddName(annot, "Name", HPDF_ANNOT_ICON_NAMES_NAMES[(HPDF_INT) icon]) != HPDF_OK)
		return HPDF_CheckError(annot->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_TextAnnot_SetOpened(HPDF_Annotation annot, HPDF_BOOL opened)
{
	HPDF_Boolean b;

	if (!CheckSubType(annot, HPDF_ANNOT_TEXT))
		return HPDF_INVALID_ANNOTATION;

	b = HPDF_Boolean_New(annot->mmgr, opened);
	if (!b)
		return HPDF_CheckError(annot->error);

	return HPDF_Dict_Add(annot, "Open", b);
}


HPDF_STATUS HPDF_PopupAnnot_SetOpened(HPDF_Annotation annot, HPDF_BOOL opened)
{
	HPDF_Boolean b;

	if (!CheckSubType(annot, HPDF_ANNOT_POPUP))
		return HPDF_INVALID_ANNOTATION;

	b = HPDF_Boolean_New(annot->mmgr, opened);
	if (!b)
		return HPDF_CheckError(annot->error);

	return HPDF_Dict_Add(annot, "Open", b);
}


HPDF_STATUS HPDF_MarkupAnnot_SetTitle(HPDF_Annotation annot, const char *name)
{
	return HPDF_Dict_Add(annot, "T", HPDF_String_New(annot->mmgr, name, NULL));
}


HPDF_STATUS HPDF_MarkupAnnot_SetSubject(HPDF_Annotation annot, const char *name)
{
	return HPDF_Dict_Add(annot, "Subj", HPDF_String_New(annot->mmgr, name, NULL));
}


HPDF_STATUS HPDF_MarkupAnnot_SetCreationDate(HPDF_Annotation annot, const HPDF_Date *value)
{
	return HPDF_Info_SetInfoDateAttr(annot, HPDF_INFO_CREATION_DATE, value);
}


HPDF_STATUS HPDF_MarkupAnnot_SetTransparency(HPDF_Annotation annot, HPDF_REAL value)
{
	return HPDF_Dict_AddReal(annot, "CA", value);
}


HPDF_STATUS HPDF_MarkupAnnot_SetIntent(HPDF_Annotation annot, HPDF_AnnotIntent intent)
{
	if (HPDF_Dict_AddName(annot, "IT", HPDF_ANNOT_INTENT_NAMES[(HPDF_INT) intent]) != HPDF_OK)
		return HPDF_CheckError(annot->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_MarkupAnnot_SetPopup(HPDF_Annotation annot, HPDF_Annotation popup)
{
	return HPDF_Dict_Add(annot, "Popup", popup);
}


HPDF_STATUS HPDF_MarkupAnnot_SetInteriorRGBColor(HPDF_Annotation annot, const HPDF_RGBColor *color)	/* IC with RGB entry */
{
	HPDF_Array cArray;
	HPDF_STATUS ret = HPDF_OK;

	cArray = HPDF_Array_New(annot->mmgr);
	if (!cArray)
		return HPDF_Error_GetCode(annot->error);

	ret += HPDF_Dict_Add(annot, "IC", cArray);
	ret += HPDF_Array_AddReal(cArray, color->r);
	ret += HPDF_Array_AddReal(cArray, color->g);
	ret += HPDF_Array_AddReal(cArray, color->b);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(annot->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_MarkupAnnot_SetInteriorCMYKColor(HPDF_Annotation annot, const HPDF_CMYKColor *color)	/* IC with CMYK entry */
{
	HPDF_Array cArray;
	HPDF_STATUS ret = HPDF_OK;

	cArray = HPDF_Array_New(annot->mmgr);
	if (!cArray)
		return HPDF_Error_GetCode(annot->error);

	ret += HPDF_Dict_Add(annot, "IC", cArray);
	ret += HPDF_Array_AddReal(cArray, color->c);
	ret += HPDF_Array_AddReal(cArray, color->m);
	ret += HPDF_Array_AddReal(cArray, color->y);
	ret += HPDF_Array_AddReal(cArray, color->k);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(annot->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_MarkupAnnot_SetInteriorGrayColor(HPDF_Annotation annot, HPDF_REAL color)	/* IC with Gray entry */
{
	HPDF_Array cArray;
	HPDF_STATUS ret = HPDF_OK;

	cArray = HPDF_Array_New(annot->mmgr);
	if (!cArray)
		return HPDF_Error_GetCode(annot->error);

	ret += HPDF_Dict_Add(annot, "IC", cArray);
	ret += HPDF_Array_AddReal(cArray, color);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(annot->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_MarkupAnnot_SetInteriorTransparent(HPDF_Annotation annot)	/* IC with No Color entry */
{
	HPDF_Array cArray;
	HPDF_STATUS ret = HPDF_OK;

	cArray = HPDF_Array_New(annot->mmgr);
	if (!cArray)
		return HPDF_Error_GetCode(annot->error);

	ret = HPDF_Dict_Add(annot, "IC", cArray);

	return ret;
}


HPDF_BOOL HPDF_Annotation_Validate(HPDF_Annotation annot)
{
	if (!annot)
		return HPDF_FALSE;

	if (annot->header.obj_class != (HPDF_OSUBCLASS_ANNOTATION | HPDF_OCLASS_DICT))
		return HPDF_FALSE;

	return HPDF_TRUE;
}


HPDF_STATUS HPDF_Annot_Set3DView(HPDF_MMgr mmgr, HPDF_Annotation annot, HPDF_Annotation annot3d, HPDF_Dict view3d)
{
	HPDF_Proxy proxyView3d;
	HPDF_Dict exData = HPDF_Dict_New(mmgr);
	HPDF_STATUS retS = HPDF_OK;

	retS += HPDF_Dict_AddName(exData, "Type", "ExData");
	retS += HPDF_Dict_AddName(exData, "Subtype", "Markup3D");
	retS += HPDF_Dict_Add(exData, "3DA", annot3d);

	proxyView3d = HPDF_Proxy_New(mmgr, view3d);

	retS += HPDF_Dict_Add(exData, "3DV", proxyView3d);
	retS += HPDF_Dict_Add(annot, "ExData", exData);
	return retS;
}


HPDF_Annotation HPDF_PopupAnnot_New(HPDF_MMgr mmgr, HPDF_Xref xref, const HPDF_Rect *rect, HPDF_Annotation parent)
{
	HPDF_Annotation annot;

	annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_POPUP, rect);
	if (!annot)
		return NULL;

	if (HPDF_Dict_Add(annot, "Parent", parent) != HPDF_OK)
		return NULL;

	return annot;
}


HPDF_Annotation HPDF_StampAnnot_New(HPDF_MMgr mmgr,
	HPDF_Xref xref,
	const HPDF_Rect *rect,
	HPDF_StampAnnotName name,
	const char *text,
	HPDF_Encoder encoder)
{
	HPDF_Annotation annot;
	HPDF_String s;

	annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_STAMP, rect);
	if (!annot)
		return NULL;

	if (HPDF_Dict_AddName(annot, "Name", HPDF_STAMP_ANNOT_NAME_NAMES[name]) != HPDF_OK)
		return NULL;

	s = HPDF_String_New(mmgr, text, encoder);
	if (!s)
		return NULL;

	if (HPDF_Dict_Add(annot, "Contents", s) != HPDF_OK)
		return NULL;

	return annot;
}


HPDF_Annotation HPDF_ProjectionAnnot_New(HPDF_MMgr mmgr, HPDF_Xref xref, const HPDF_Rect *rect, const char *text, HPDF_Encoder encoder)
{
	HPDF_Annotation annot;
	HPDF_String s;

	annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_PROJECTION, rect);
	if (!annot)
		return NULL;

	s = HPDF_String_New(mmgr, text, encoder);
	if (!s)
		return NULL;

	if (HPDF_Dict_Add(annot, "Contents", s) != HPDF_OK)
		return NULL;

	return annot;
}


HPDF_STATUS HPDF_TextMarkupAnnot_SetQuadPoints(HPDF_Annotation annot, HPDF_Point lb, HPDF_Point rb, HPDF_Point lt, HPDF_Point rt)	/* l-left, r-right, b-bottom, t-top positions */
{
	HPDF_Array quadPoints;
	HPDF_STATUS ret = HPDF_OK;

	quadPoints = HPDF_Array_New(annot->mmgr);
	if (!quadPoints)
		return HPDF_Error_GetCode(annot->error);

	if ((ret = HPDF_Dict_Add(annot, "QuadPoints", quadPoints)) != HPDF_OK)
		return ret;

	ret += HPDF_Array_AddReal(quadPoints, lb.x);
	ret += HPDF_Array_AddReal(quadPoints, lb.y);
	ret += HPDF_Array_AddReal(quadPoints, rb.x);
	ret += HPDF_Array_AddReal(quadPoints, rb.y);
	ret += HPDF_Array_AddReal(quadPoints, lt.x);
	ret += HPDF_Array_AddReal(quadPoints, lt.y);
	ret += HPDF_Array_AddReal(quadPoints, rt.x);
	ret += HPDF_Array_AddReal(quadPoints, rt.y);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(quadPoints->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_FreeTextAnnot_SetLineEndingStyle(
	HPDF_Annotation annot,
	HPDF_LineAnnotEndingStyle startStyle,
	HPDF_LineAnnotEndingStyle endStyle)
{
	HPDF_Array lineEndStyles;
	HPDF_STATUS ret = HPDF_OK;

	lineEndStyles = HPDF_Array_New(annot->mmgr);
	if (!lineEndStyles)
		return HPDF_Error_GetCode(annot->error);

	if ((ret = HPDF_Dict_Add(annot, "LE", lineEndStyles)) != HPDF_OK)
		return ret;

	ret += HPDF_Array_AddName(lineEndStyles, HPDF_LINE_ANNOT_ENDING_STYLE_NAMES[(HPDF_INT) startStyle]);
	ret += HPDF_Array_AddName(lineEndStyles, HPDF_LINE_ANNOT_ENDING_STYLE_NAMES[(HPDF_INT) endStyle]);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(lineEndStyles->error);

	return HPDF_OK;
}


/* RD entry : this is the difference between Rect and the text annotation rectangle */
HPDF_STATUS HPDF_MarkupAnnot_SetRectDiff(HPDF_Annotation annot, const HPDF_Rect *rect)
{
	HPDF_Array array;
	HPDF_STATUS ret = HPDF_OK;
	HPDF_REAL bottom, top;

	array = HPDF_Array_New(annot->mmgr);
	if (!array)
		return HPDF_Error_GetCode(annot->error);

	if ((ret = HPDF_Dict_Add(annot, "RD", array)) != HPDF_OK)
		return ret;

	if (rect->top < rect->bottom)
	{
		top = rect->bottom;
		bottom = rect->top;
	} else
	{
		top = rect->top;
		bottom = rect->bottom;
	}

	ret += HPDF_Array_AddReal(array, rect->left);
	ret += HPDF_Array_AddReal(array, bottom);
	ret += HPDF_Array_AddReal(array, rect->right);
	ret += HPDF_Array_AddReal(array, top);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(array->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_FreeTextAnnot_SetDefaultStyle(HPDF_Annotation annot, const char *style)
{
	HPDF_String s;
	HPDF_STATUS ret = HPDF_OK;

	s = HPDF_String_New(annot->mmgr, style, NULL);
	if (!s)
		return HPDF_Error_GetCode(annot->error);

	ret = HPDF_Dict_Add(annot, "DS", s);

	return ret;
}


HPDF_STATUS HPDF_FreeTextAnnot_Set3PointCalloutLine(HPDF_Annotation annot, HPDF_Point startPoint, HPDF_Point kneePoint, HPDF_Point endPoint)	/* Callout line will be in default user space */
{
	HPDF_Array clPoints;
	HPDF_STATUS ret = HPDF_OK;

	clPoints = HPDF_Array_New(annot->mmgr);
	if (!clPoints)
		return HPDF_Error_GetCode(annot->error);

	if ((ret = HPDF_Dict_Add(annot, "CL", clPoints)) != HPDF_OK)
		return ret;

	ret += HPDF_Array_AddReal(clPoints, startPoint.x);
	ret += HPDF_Array_AddReal(clPoints, startPoint.y);
	ret += HPDF_Array_AddReal(clPoints, kneePoint.x);
	ret += HPDF_Array_AddReal(clPoints, kneePoint.y);
	ret += HPDF_Array_AddReal(clPoints, endPoint.x);
	ret += HPDF_Array_AddReal(clPoints, endPoint.y);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(clPoints->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_FreeTextAnnot_Set2PointCalloutLine(HPDF_Annotation annot, HPDF_Point startPoint, HPDF_Point endPoint)	/* Callout line will be in default user space */
{
	HPDF_Array clPoints;
	HPDF_STATUS ret = HPDF_OK;

	clPoints = HPDF_Array_New(annot->mmgr);
	if (!clPoints)
		return HPDF_Error_GetCode(annot->error);

	if ((ret = HPDF_Dict_Add(annot, "CL", clPoints)) != HPDF_OK)
		return ret;

	ret += HPDF_Array_AddReal(clPoints, startPoint.x);
	ret += HPDF_Array_AddReal(clPoints, startPoint.y);
	ret += HPDF_Array_AddReal(clPoints, endPoint.x);
	ret += HPDF_Array_AddReal(clPoints, endPoint.y);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(clPoints->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_MarkupAnnot_SetCloudEffect(HPDF_Annotation annot, HPDF_INT cloudIntensity)	/* BE entry */
{
	HPDF_Dict borderEffect;
	HPDF_STATUS ret = HPDF_OK;

	borderEffect = HPDF_Dict_New(annot->mmgr);
	if (!borderEffect)
		return HPDF_Error_GetCode(annot->error);

	ret += HPDF_Dict_Add(annot, "BE", borderEffect);
	ret += HPDF_Dict_AddName(borderEffect, "S", "C");
	ret += HPDF_Dict_AddNumber(borderEffect, "I", cloudIntensity);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(annot->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_LineAnnot_SetPosition(
	HPDF_Annotation annot,
	HPDF_Point startPoint,
	HPDF_LineAnnotEndingStyle startStyle,
	HPDF_Point endPoint,
	HPDF_LineAnnotEndingStyle endStyle)
{
	HPDF_Array lineEndPoints;
	HPDF_Array lineEndStyles;
	HPDF_STATUS ret = HPDF_OK;

	lineEndPoints = HPDF_Array_New(annot->mmgr);
	if (!lineEndPoints)
		return HPDF_Error_GetCode(annot->error);

	if ((ret = HPDF_Dict_Add(annot, "L", lineEndPoints)) != HPDF_OK)
		return ret;

	ret += HPDF_Array_AddReal(lineEndPoints, startPoint.x);
	ret += HPDF_Array_AddReal(lineEndPoints, startPoint.y);
	ret += HPDF_Array_AddReal(lineEndPoints, endPoint.x);
	ret += HPDF_Array_AddReal(lineEndPoints, endPoint.y);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(lineEndPoints->error);

	lineEndStyles = HPDF_Array_New(annot->mmgr);
	if (!lineEndStyles)
		return HPDF_Error_GetCode(annot->error);

	if ((ret = HPDF_Dict_Add(annot, "LE", lineEndStyles)) != HPDF_OK)
		return ret;

	ret += HPDF_Array_AddName(lineEndStyles, HPDF_LINE_ANNOT_ENDING_STYLE_NAMES[(HPDF_INT) startStyle]);
	ret += HPDF_Array_AddName(lineEndStyles, HPDF_LINE_ANNOT_ENDING_STYLE_NAMES[(HPDF_INT) endStyle]);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(lineEndStyles->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_LineAnnot_SetLeader(
	HPDF_Annotation annot,
	HPDF_INT leaderLen,
	HPDF_INT leaderExtLen,
	HPDF_INT leaderOffsetLen)
{
	HPDF_STATUS ret = HPDF_OK;

	ret += HPDF_Dict_AddNumber(annot, "LL", leaderLen);
	ret += HPDF_Dict_AddNumber(annot, "LLE", leaderExtLen);
	ret += HPDF_Dict_AddNumber(annot, "LLO", leaderOffsetLen);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(annot->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_LineAnnot_SetCaption(
	HPDF_Annotation annot,
	HPDF_BOOL showCaption,
	HPDF_LineAnnotCapPosition position,
	HPDF_INT horzOffset,
	HPDF_INT vertOffset)
{
	HPDF_STATUS ret = HPDF_OK;
	HPDF_Array capOffset;

	ret += HPDF_Dict_AddBoolean(annot, "Cap", showCaption);
	ret += HPDF_Dict_AddName(annot, "CP", HPDF_LINE_ANNOT_CAP_POSITION_NAMES[(HPDF_INT) position]);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(annot->error);

	capOffset = HPDF_Array_New(annot->mmgr);
	if (!capOffset)
		return HPDF_Error_GetCode(annot->error);

	if ((ret = HPDF_Dict_Add(annot, "CO", capOffset)) != HPDF_OK)
		return ret;

	ret += HPDF_Array_AddNumber(capOffset, horzOffset);
	ret += HPDF_Array_AddNumber(capOffset, vertOffset);

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(capOffset->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_ProjectionAnnot_SetExData(HPDF_Annotation annot, HPDF_ExData exdata)
{
	HPDF_STATUS ret = HPDF_OK;

	ret = HPDF_Dict_Add(annot, "ExData", exdata);

	return ret;
}