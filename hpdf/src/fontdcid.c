/*
 * << Haru Free PDF Library >> -- hpdf_fontdef_cid.c
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
#include "hpdf/fontdef.h"
#include <string.h>

/*----------------------------------------------------------------------*/
/*----- HPDF_CIDFontDef ------------------------------------------------*/

static void HPDF_CIDFontDef_FreeWidth(HPDF_FontDef fontdef)
{
	HPDF_CIDFontDefAttr attr = (HPDF_CIDFontDefAttr) fontdef->attr;
	HPDF_UINT i;

	for (i = 0; i < attr->widths->count; i++)
	{
		HPDF_CID_Width *w = (HPDF_CID_Width *) HPDF_List_ItemAt(attr->widths, i);

		HPDF_FreeMem(fontdef->mmgr, w);
	}

	HPDF_List_Free(attr->widths);
	attr->widths = NULL;

	fontdef->valid = HPDF_FALSE;
}


static void HPDF_CIDFontDef_FreeFunc(HPDF_FontDef fontdef)
{
	HPDF_CIDFontDefAttr attr = (HPDF_CIDFontDefAttr) fontdef->attr;

	HPDF_CIDFontDef_FreeWidth(fontdef);
	HPDF_FreeMem(fontdef->mmgr, attr);
}


HPDF_FontDef HPDF_CIDFontDef_New(HPDF_MMgr mmgr, const char *name, HPDF_FontDef_InitFunc init_fn)
{
	HPDF_FontDef fontdef;
	HPDF_CIDFontDefAttr fontdef_attr;

	if (!mmgr)
		return NULL;

	fontdef = (HPDF_FontDef) HPDF_GetMem(mmgr, sizeof(HPDF_FontDef_Rec));
	if (!fontdef)
		return NULL;

	memset(fontdef, 0, sizeof(HPDF_FontDef_Rec));
	fontdef->sig_bytes = HPDF_FONTDEF_SIG_BYTES;
	strcpy(fontdef->base_font, name);
	fontdef->mmgr = mmgr;
	fontdef->error = mmgr->error;
	fontdef->type = HPDF_FONTDEF_TYPE_UNINITIALIZED;
	fontdef->free_fn = HPDF_CIDFontDef_FreeFunc;
	fontdef->init_fn = init_fn;
	fontdef->valid = HPDF_FALSE;
	fontdef_attr = (HPDF_CIDFontDefAttr) HPDF_GetMem(mmgr, sizeof(HPDF_CIDFontDefAttr_Rec));
	if (!fontdef_attr)
	{
		HPDF_FreeMem(fontdef->mmgr, fontdef);
		return NULL;
	}

	fontdef->attr = fontdef_attr;
	memset(fontdef_attr, 0, sizeof(HPDF_CIDFontDefAttr_Rec));

	fontdef_attr->widths = HPDF_List_New(mmgr, HPDF_DEF_CHAR_WIDTHS_NUM);
	if (!fontdef_attr->widths)
	{
		HPDF_FreeMem(fontdef->mmgr, fontdef);
		HPDF_FreeMem(fontdef->mmgr, fontdef_attr);
		return NULL;
	}

	fontdef->missing_width = 500;
	fontdef_attr->DW = 1000;
	fontdef_attr->DW2[0] = 880;
	fontdef_attr->DW2[1] = -1000;

	return fontdef;
}


HPDF_INT16 HPDF_CIDFontDef_GetCIDWidth(HPDF_FontDef fontdef, HPDF_UINT16 cid)
{
	HPDF_CIDFontDefAttr attr = (HPDF_CIDFontDefAttr) fontdef->attr;
	HPDF_UINT i;

	for (i = 0; i < attr->widths->count; i++)
	{
		HPDF_CID_Width *w = (HPDF_CID_Width *) HPDF_List_ItemAt(attr->widths, i);

		if (w->cid == cid)
			return w->width;
	}

	/* Not found in pdf_cid_width array. */
	return attr->DW;
}


HPDF_STATUS HPDF_CIDFontDef_AddWidth(HPDF_FontDef fontdef, const HPDF_CID_Width *widths)
{
	HPDF_CIDFontDefAttr attr = (HPDF_CIDFontDefAttr) fontdef->attr;

	while (widths->cid != 0xFFFF)
	{
		HPDF_CID_Width *w = (HPDF_CID_Width *) HPDF_GetMem(fontdef->mmgr, sizeof(HPDF_CID_Width));
		HPDF_STATUS ret;

		if (!w)
			return fontdef->error->error_no;

		w->cid = widths->cid;
		w->width = widths->width;

		if ((ret = HPDF_List_Add(attr->widths, w)) != HPDF_OK)
		{
			HPDF_FreeMem(fontdef->mmgr, w);

			return ret;
		}

		widths++;
	}

	return HPDF_OK;
}


HPDF_STATUS HPDF_CIDFontDef_ChangeStyle(HPDF_FontDef fontdef, HPDF_BOOL bold, HPDF_BOOL italic)
{
	if (!fontdef || !fontdef->attr)
		return HPDF_INVALID_FONTDEF_DATA;

	if (bold)
	{
		fontdef->stemv *= 2;
		fontdef->flags |= HPDF_FONT_FOURCE_BOLD;
	}

	if (italic)
	{
		fontdef->italic_angle -= 11;
		fontdef->flags |= HPDF_FONT_ITALIC;
	}

	return HPDF_OK;
}
