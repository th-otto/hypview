/*
 * << Haru Free PDF Library >> -- hpdf_pages.c
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

#ifndef _HPDF_PAGES_H
#define _HPDF_PAGES_H

#include "hpdf/gstate.h"
#include "hpdf/extgstat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/*----- HPDF_Pages -----------------------------------------------------------*/

HPDF_Pages HPDF_Pages_New(HPDF_MMgr mmgr, HPDF_Pages parent, HPDF_Xref xref);


HPDF_BOOL HPDF_Pages_Validate(HPDF_Pages pages);


HPDF_STATUS HPDF_Pages_AddKids(HPDF_Pages parent, HPDF_Dict kid);


HPDF_STATUS HPDF_Page_InsertBefore(HPDF_Page page, HPDF_Page target);


typedef struct _HPDF_PageAttr_Rec
{
	HPDF_Pages parent;
	HPDF_Dict fonts;
	HPDF_Dict xobjects;
	HPDF_Dict ext_gstates;
	HPDF_Dict patterns;
	HPDF_Dict colorspaces;
	HPDF_GState gstate;
	HPDF_Point str_pos;
	HPDF_Point cur_pos;
	HPDF_Point text_pos;
	HPDF_TransMatrix text_matrix;
	HPDF_UINT16 gmode;
	HPDF_Dict contents;
	HPDF_Stream stream;
	HPDF_Xref xref;
} HPDF_PageAttr_Rec;

typedef HPDF_PageAttr_Rec *HPDF_PageAttr;


/*----------------------------------------------------------------------------*/
/*----- HPDF_Page ------------------------------------------------------------*/

HPDF_BOOL HPDF_Page_Validate(HPDF_Page page);


HPDF_Page HPDF_Page_New(HPDF_MMgr mmgr, HPDF_Xref xref);


void *HPDF_Page_GetInheritableItem(HPDF_Page page, const char *key, HPDF_UINT16 obj_class);


const char *HPDF_Page_GetXObjectName(HPDF_Page page, HPDF_XObject xobj);


const char *HPDF_Page_GetLocalFontName(HPDF_Page page, HPDF_Font font);


const char *HPDF_Page_GetLocalPatternName(HPDF_Page page, HPDF_Pattern pattern);


const char *HPDF_Page_GetLocalColorspaceName(HPDF_Page page, HPDF_PatternColorspace colorspace);


const char *HPDF_Page_GetExtGStateName(HPDF_Page page, HPDF_ExtGState gstate);


void HPDF_Page_GetMediaBox(HPDF_Page page, HPDF_Box *box);


HPDF_STATUS HPDF_Page_SetBoxValue(HPDF_Page page, const char *name, HPDF_UINT index, HPDF_REAL value);


void HPDF_Page_SetFilter(HPDF_Page page, HPDF_UINT filter);


HPDF_STATUS HPDF_Page_CheckState(HPDF_Page page, HPDF_UINT mode);


#ifdef __cplusplus
}
#endif

#endif /* _HPDF_PAGES_H */
