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

#include "hpdf/conf.h"
#include "hpdf/utils.h"
#include "hpdf.h"
#include "hpdf/annotat.h"
#include "hpdf/destinat.h"
#include "hpdf/3dmeasur.h"
#include "hpdf/exdata.h"
#include "hpdf/u3d.h"
#include <string.h>

#if defined(__PUREC__) || defined(__APPLE__)
# define PAGES_ALL
#endif


/*----------------------------------------------------------------------------*/

typedef struct _HPDF_PageSizeValue
{
	HPDF_REAL x;
	HPDF_REAL y;
} HPDF_PageSizeValue;

extern const HPDF_PageSizeValue HPDF_PREDEFINED_PAGE_SIZES[HPDF_PAGE_SIZE_EOF];

#if defined(PAGES_ALL) || defined(PAGES_SIZES)
const HPDF_PageSizeValue HPDF_PREDEFINED_PAGE_SIZES[HPDF_PAGE_SIZE_EOF] = {
	{ 612.0, 792.0 },		/* HPDF_PAGE_SIZE_LETTER */
	{ 612.0, 1008.0 },		/* HPDF_PAGE_SIZE_LEGAL */
	{ 841.89, 1190.551 },	/* HPDF_PAGE_SIZE_A3 */
	{ 595.276, 841.89 },	/* HPDF_PAGE_SIZE_A4 */
	{ 419.528, 595.276 },	/* HPDF_PAGE_SIZE_A5 */
	{ 708.661, 1000.63 },	/* HPDF_PAGE_SIZE_B4 */
	{ 498.898, 708.661 },	/* HPDF_PAGE_SIZE_B5 */
	{ 522.0, 756.0 },		/* HPDF_PAGE_SIZE_EXECUTIVE */
	{ 288.0, 432.0 },		/* HPDF_PAGE_SIZE_US4x6 */
	{ 288.0, 576.0 },		/* HPDF_PAGE_SIZE_US4x8 */
	{ 360.0, 504.0 },		/* HPDF_PAGE_SIZE_US5x7 */
	{ 297.0, 684.0 },		/* HPDF_PAGE_SIZE_COMM10 */
	{ 396.0, 612.0 },		/* HPDF_PAGE_SIZE_HALF_LETTER */
	{ 792.0, 1224.0 },		/* HPDF_PAGE_SIZE_DOUBLE_LETTER */
	{ 1080.0, 1638.0 },		/* HPDF_PAGE_SIZE_BROADSHEET */
};
#endif

#if defined(PAGES_ALL) || defined(PAGES_SIZES)
const char *const HPDF_COLORSPACE_NAMES[HPDF_CS_EOF] = {
	"DeviceGray",
	"DeviceRGB",
	"DeviceCMYK",
	"CalGray",
	"CalRGB",
	"Lab",
	"ICCBased",
	"Separation",
	"DeviceN",
	"Indexed",
	"Pattern",
};
#endif

/*----------------------------------------------------------------------------*/
/*----- HPDF_Pages -----------------------------------------------------------*/


#if defined(PAGES_ALL) || defined(PAGES_NEW)
static HPDF_UINT GetPageCount(HPDF_Dict pages)
{
	HPDF_UINT i;
	HPDF_UINT count = 0;
	HPDF_Array kids = (HPDF_Array) HPDF_Dict_GetItem(pages, "Kids", HPDF_OCLASS_ARRAY);

	if (!kids)
		return 0;

	for (i = 0; i < kids->list->count; i++)
	{
		void *obj = HPDF_Array_GetItem(kids, i, HPDF_OCLASS_DICT);
		HPDF_Obj_Header *header = (HPDF_Obj_Header *) obj;

		if (header->obj_class == (HPDF_OCLASS_DICT | HPDF_OSUBCLASS_PAGES))
			count += GetPageCount((HPDF_Dict) obj);
		else if (header->obj_class == (HPDF_OCLASS_DICT | HPDF_OSUBCLASS_PAGE))
			count += 1;
	}

	return count;
}


static HPDF_STATUS Pages_BeforeWrite(HPDF_Dict obj)
{
	HPDF_Array kids = (HPDF_Array) HPDF_Dict_GetItem(obj, "Kids", HPDF_OCLASS_ARRAY);
	HPDF_Number count = (HPDF_Number) HPDF_Dict_GetItem(obj, "Count", HPDF_OCLASS_NUMBER);
	HPDF_STATUS ret;

	if (!kids)
		return HPDF_SetError(obj->error, HPDF_PAGES_MISSING_KIDS_ENTRY, 0);

	if (count)
	{
		count->value = GetPageCount(obj);
	} else
	{
		count = HPDF_Number_New(obj->mmgr, GetPageCount(obj));
		if (!count)
			return HPDF_Error_GetCode(obj->error);

		if ((ret = HPDF_Dict_Add(obj, "Count", count)) != HPDF_OK)
			return ret;
	}

	return HPDF_OK;
}


HPDF_Pages HPDF_Pages_New(HPDF_MMgr mmgr, HPDF_Pages parent, HPDF_Xref xref)
{
	HPDF_STATUS ret = HPDF_OK;
	HPDF_Pages pages;

	pages = HPDF_Dict_New(mmgr);
	if (!pages)
		return NULL;

	pages->header.obj_class |= HPDF_OSUBCLASS_PAGES;
	pages->before_write_fn = Pages_BeforeWrite;

	if (HPDF_Xref_Add(xref, pages) != HPDF_OK)
		return NULL;

	/* add required elements */
	ret |= HPDF_Dict_AddName(pages, "Type", "Pages");
	ret |= HPDF_Dict_Add(pages, "Kids", HPDF_Array_New(pages->mmgr));
	ret |= HPDF_Dict_Add(pages, "Count", HPDF_Number_New(pages->mmgr, 0));

	if (ret == HPDF_OK && parent)
		ret |= HPDF_Pages_AddKids(parent, pages);

	if (ret != HPDF_OK)
		return NULL;

	return pages;
}
#endif


#if defined(PAGES_ALL) || defined(PAGES_ADDKIDS)
HPDF_STATUS HPDF_Pages_AddKids(HPDF_Pages parent, HPDF_Dict kid)
{
	HPDF_Array kids;
	HPDF_STATUS ret;

	if (HPDF_Dict_GetItem(kid, "Parent", HPDF_OCLASS_DICT))
		return HPDF_SetError(parent->error, HPDF_PAGE_CANNOT_SET_PARENT, 0);

	if ((ret = HPDF_Dict_Add(kid, "Parent", parent)) != HPDF_OK)
		return ret;

	kids = (HPDF_Array) HPDF_Dict_GetItem(parent, "Kids", HPDF_OCLASS_ARRAY);
	if (!kids)
		return HPDF_SetError(parent->error, HPDF_PAGES_MISSING_KIDS_ENTRY, 0);

	if (kid->header.obj_class == (HPDF_OCLASS_DICT | HPDF_OSUBCLASS_PAGE))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) kid->attr;

		attr->parent = parent;
	}

	return HPDF_Array_Add(kids, kid);
}
#endif


#if defined(PAGES_ALL) || defined(PAGES_INSERTBEFORE)
HPDF_STATUS HPDF_Page_InsertBefore(HPDF_Page page, HPDF_Page target)
{
	HPDF_Page parent;
	HPDF_Array kids;
	HPDF_STATUS ret;
	HPDF_PageAttr attr;

	if (!target)
		return HPDF_INVALID_PARAMETER;

	attr = (HPDF_PageAttr) target->attr;
	parent = attr->parent;

	if (!parent)
		return HPDF_PAGE_CANNOT_SET_PARENT;

	if (HPDF_Dict_GetItem(page, "Parent", HPDF_OCLASS_DICT))
		return HPDF_SetError(parent->error, HPDF_PAGE_CANNOT_SET_PARENT, 0);

	if ((ret = HPDF_Dict_Add(page, "Parent", parent)) != HPDF_OK)
		return ret;

	kids = (HPDF_Array) HPDF_Dict_GetItem(parent, "Kids", HPDF_OCLASS_ARRAY);
	if (!kids)
		return HPDF_SetError(parent->error, HPDF_PAGES_MISSING_KIDS_ENTRY, 0);

	attr = (HPDF_PageAttr) page->attr;
	attr->parent = parent;

	return HPDF_Array_Insert(kids, target, &page->header);
}
#endif


#if defined(PAGES_ALL) || defined(PAGES_VALIDATE)
HPDF_BOOL HPDF_Pages_Validate(HPDF_Pages pages)
{
	HPDF_Obj_Header *header = (HPDF_Obj_Header *) pages;

	if (!pages || header->obj_class != (HPDF_OCLASS_DICT | HPDF_OSUBCLASS_PAGES))
		return HPDF_FALSE;

	return HPDF_TRUE;
}
#endif


/*----------------------------------------------------------------------------*/
/*----- HPDF_Page ------------------------------------------------------------*/


#if defined(PAGES_ALL) || defined(PAGE_NEW)
static void Page_OnFree(HPDF_Dict obj)
{
	HPDF_PageAttr attr = (HPDF_PageAttr) obj->attr;

	if (attr)
	{
		if (attr->gstate)
			HPDF_GState_Free(obj->mmgr, attr->gstate);

		HPDF_FreeMem(obj->mmgr, attr);
	}
}


static HPDF_STATUS Page_BeforeWrite(HPDF_Dict obj)
{
	HPDF_STATUS ret;
	HPDF_Page page = (HPDF_Page) obj;
	HPDF_PageAttr attr = (HPDF_PageAttr) obj->attr;

	if (attr->gmode == HPDF_GMODE_PATH_OBJECT)
	{
		if ((ret = HPDF_Page_EndPath(page)) != HPDF_OK)
			return ret;
	}

	if (attr->gmode == HPDF_GMODE_TEXT_OBJECT)
	{
		if ((ret = HPDF_Page_EndText(page)) != HPDF_OK)
			return ret;
	}

	if (attr->gstate)
	{
		while (attr->gstate->prev)
		{
			if ((ret = HPDF_Page_GRestore(page)) != HPDF_OK)
				return ret;
		}
	}

	return HPDF_OK;
}


static HPDF_STATUS AddResource(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_OK;
	HPDF_Dict resources;
	HPDF_Array procset;

	resources = HPDF_Dict_New(page->mmgr);
	if (!resources)
		return HPDF_Error_GetCode(page->error);

	/* although ProcSet-entry is obsolete, add it to resource for
	 * compatibility
	 */

	ret |= HPDF_Dict_Add(page, "Resources", resources);

	procset = HPDF_Array_New(page->mmgr);
	if (!procset)
		return HPDF_Error_GetCode(page->error);

	if (HPDF_Dict_Add(resources, "ProcSet", procset) != HPDF_OK)
		return HPDF_Error_GetCode(resources->error);

	ret |= HPDF_Array_Add(procset, HPDF_Name_New(page->mmgr, "PDF"));
	ret |= HPDF_Array_Add(procset, HPDF_Name_New(page->mmgr, "Text"));
	ret |= HPDF_Array_Add(procset, HPDF_Name_New(page->mmgr, "ImageB"));
	ret |= HPDF_Array_Add(procset, HPDF_Name_New(page->mmgr, "ImageC"));
	ret |= HPDF_Array_Add(procset, HPDF_Name_New(page->mmgr, "ImageI"));

	if (ret != HPDF_OK)
		return HPDF_Error_GetCode(procset->error);

	return HPDF_OK;
}


HPDF_Page HPDF_Page_New(HPDF_MMgr mmgr, HPDF_Xref xref)
{
	HPDF_STATUS ret;
	HPDF_PageAttr attr;
	HPDF_Page page;
	HPDF_Box box;

	page = HPDF_Dict_New(mmgr);
	if (!page)
		return NULL;

	page->header.obj_class |= HPDF_OSUBCLASS_PAGE;
	page->free_fn = Page_OnFree;
	page->before_write_fn = Page_BeforeWrite;

	attr = (HPDF_PageAttr) HPDF_GetMem(page->mmgr, sizeof(HPDF_PageAttr_Rec));
	if (!attr)
	{
		HPDF_Dict_Free(page);
		return NULL;
	}

	page->attr = attr;
	memset(attr, 0, sizeof(HPDF_PageAttr_Rec));
	attr->gmode = HPDF_GMODE_PAGE_DESCRIPTION;
	attr->cur_pos.x = 0;
	attr->cur_pos.y = 0;
	attr->text_pos.x = 0;
	attr->text_pos.y = 0;

	ret = HPDF_Xref_Add(xref, page);
	if (ret != HPDF_OK)
		return NULL;

	attr->gstate = HPDF_GState_New(page->mmgr, NULL);
	attr->contents = HPDF_DictStream_New(page->mmgr, xref);

	if (!attr->gstate || !attr->contents)
		return NULL;

	attr->stream = attr->contents->stream;
	attr->xref = xref;

	/* add required elements */
	ret |= HPDF_Dict_AddName(page, "Type", "Page");
	HPDF_ToBox(&box, 0, 0, HPDF_PREDEFINED_PAGE_SIZES[HPDF_DEF_PAGE_SIZE].x, HPDF_PREDEFINED_PAGE_SIZES[HPDF_DEF_PAGE_SIZE].y);
	ret |= HPDF_Dict_Add(page, "MediaBox", HPDF_Box_Array_New(page->mmgr, &box));
	ret |= HPDF_Dict_Add(page, "Contents", attr->contents);

	ret |= AddResource(page);

	if (ret != HPDF_OK)
		return NULL;

	return page;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CHECKSTATE)
HPDF_STATUS HPDF_Page_CheckState(HPDF_Page page, HPDF_UINT mode)
{
	HPDF_PageAttr attr;

	if (!page)
		return HPDF_INVALID_OBJECT;

	if (page->header.obj_class != (HPDF_OSUBCLASS_PAGE | HPDF_OCLASS_DICT))
		return HPDF_INVALID_PAGE;

	attr = (HPDF_PageAttr) page->attr;
	if (!(attr->gmode & mode))
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_GMODE, attr->gmode);

	return HPDF_OK;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETINHERITABLEITEM)
static const char *const HPDF_INHERITABLE_ENTRIES[4] = {
	"Resources",
	"MediaBox",
	"CropBox",
	"Rotate"
};


void *HPDF_Page_GetInheritableItem(HPDF_Page page, const char *key, HPDF_UINT16 obj_class)
{
	HPDF_BOOL chk = HPDF_FALSE;
	int i;
	void *obj;

	/* check whether the specified key is valid */
	for (i = 0; i < (int)(sizeof(HPDF_INHERITABLE_ENTRIES) / sizeof(HPDF_INHERITABLE_ENTRIES[0])); i++)
	{
		if (strcmp(key, HPDF_INHERITABLE_ENTRIES[i]) == 0)
		{
			chk = HPDF_TRUE;
			break;
		}
	}

	/* the key is not inheritable */
	if (!chk)
	{
		HPDF_SetError(page->error, HPDF_INVALID_PARAMETER, 0);
		return NULL;
	}

	obj = HPDF_Dict_GetItem(page, key, obj_class);

	/* if resources of the object is NULL, search resources of parent
	 * pages recursivly
	 */
	if (!obj)
	{
		HPDF_Pages pages = (HPDF_Pages) HPDF_Dict_GetItem(page, "Parent", HPDF_OCLASS_DICT);

		while (pages)
		{
			obj = HPDF_Dict_GetItem(page, key, obj_class);

			if (obj)
				break;

			pages = (HPDF_Pages) HPDF_Dict_GetItem(pages, "Parent", HPDF_OCLASS_DICT);
		}
	}

	return obj;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETLOCALFONTNAME)
const char *HPDF_Page_GetLocalFontName(HPDF_Page page, HPDF_Font font)
{
	HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;
	const char *key;

	/* whether check font-resource exists.  when it does not exists,
	 * create font-resource
	 * 2006.07.21 Fixed a problem which may cause a memory leak.
	 */
	if (!attr->fonts)
	{
		HPDF_Dict resources;
		HPDF_Dict fonts;

		resources = (HPDF_Dict) HPDF_Page_GetInheritableItem(page, "Resources", HPDF_OCLASS_DICT);
		if (!resources)
			return NULL;

		fonts = HPDF_Dict_New(page->mmgr);
		if (!fonts)
			return NULL;

		if (HPDF_Dict_Add(resources, "Font", fonts) != HPDF_OK)
			return NULL;

		attr->fonts = fonts;
	}

	/* search font-object from font-resource */
	key = HPDF_Dict_GetKeyByObj(attr->fonts, font);
	if (!key)
	{
		/* if the font is not registered in font-resource, register font to
		 * font-resource.
		 */
		char fontName[HPDF_LIMIT_MAX_NAME_LEN + 1];

		strcpy(fontName, "F");
		HPDF_IToA(fontName + 1, attr->fonts->list->count + 1, fontName + HPDF_LIMIT_MAX_NAME_LEN);

		if (HPDF_Dict_Add(attr->fonts, fontName, font) != HPDF_OK)
			return NULL;

		key = HPDF_Dict_GetKeyByObj(attr->fonts, font);
	}

	return key;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETLOCALPATTERNNAME)
const char *HPDF_Page_GetLocalPatternName(HPDF_Page page, HPDF_Pattern pattern)
{
	HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;
	const char *key;

	if (!attr->patterns)
	{
		HPDF_Dict resources;
		HPDF_Dict patterns;

		resources = (HPDF_Dict) HPDF_Page_GetInheritableItem(page, "Resources", HPDF_OCLASS_DICT);
		if (!resources)
			return NULL;

		patterns = HPDF_Dict_New(page->mmgr);
		if (!patterns)
			return NULL;

		if (HPDF_Dict_Add(resources, "Pattern", patterns) != HPDF_OK)
			return NULL;

		attr->patterns = patterns;
	}

	/* search font-object from font-resource */
	key = HPDF_Dict_GetKeyByObj(attr->patterns, pattern);
	if (!key)
	{
		/* if the pattern is not registered in pattern-resource, register pattern to
		 * pattern-resource.
		 */
		char patternName[HPDF_LIMIT_MAX_NAME_LEN + 1];

		strcpy(patternName, "P");
		HPDF_IToA(patternName + 1, attr->patterns->list->count + 1, patternName + HPDF_LIMIT_MAX_NAME_LEN);

		if (HPDF_Dict_Add(attr->patterns, patternName, pattern) != HPDF_OK)
			return NULL;

		key = HPDF_Dict_GetKeyByObj(attr->patterns, pattern);
	}

	return key;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETLOCALCOLORSPACENAME)
const char *HPDF_Page_GetLocalColorspaceName(HPDF_Page page, HPDF_PatternColorspace colorspace)
{
	HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;
	const char *key;

	if (!attr->colorspaces)
	{
		HPDF_Dict resources;
		HPDF_Dict colorspaces;

		resources = (HPDF_Dict) HPDF_Page_GetInheritableItem(page, "Resources", HPDF_OCLASS_DICT);
		if (!resources)
			return NULL;

		colorspaces = HPDF_Dict_New(page->mmgr);
		if (!colorspaces)
			return NULL;

		if (HPDF_Dict_Add(resources, "ColorSpace", colorspaces) != HPDF_OK)
			return NULL;

		attr->colorspaces = colorspaces;
	}

	/* search font-object from font-resource */
	key = HPDF_Dict_GetKeyByObj(attr->colorspaces, colorspace);
	if (!key)
	{
		/* if the colorspace is not registered in colorspace-resource, register colorspace to
		 * colorspace-resource.
		 */
		char colorspaceName[HPDF_LIMIT_MAX_NAME_LEN + 1];

		strcpy(colorspaceName, "Cs");
		HPDF_IToA(colorspaceName + 2, attr->colorspaces->list->count + 1, colorspaceName + HPDF_LIMIT_MAX_NAME_LEN);

		if (HPDF_Dict_Add(attr->colorspaces, colorspaceName, colorspace) != HPDF_OK)
			return NULL;

		key = HPDF_Dict_GetKeyByObj(attr->colorspaces, colorspace);
	}

	return key;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETMEDIABOX)
void HPDF_Page_GetMediaBox(HPDF_Page page, HPDF_Box *media_box)
{
	media_box->left = 0;
	media_box->bottom = 0;
	media_box->right = 0;
	media_box->top = 0;

	if (HPDF_Page_Validate(page))
	{
		HPDF_Array array = (HPDF_Array) HPDF_Page_GetInheritableItem(page, "MediaBox", HPDF_OCLASS_ARRAY);

		if (array)
		{
			HPDF_Real r;

			r = (HPDF_Real) HPDF_Array_GetItem(array, 0, HPDF_OCLASS_REAL);
			if (r)
				media_box->left = r->value;

			r = (HPDF_Real) HPDF_Array_GetItem(array, 1, HPDF_OCLASS_REAL);
			if (r)
				media_box->bottom = r->value;

			r = (HPDF_Real) HPDF_Array_GetItem(array, 2, HPDF_OCLASS_REAL);
			if (r)
				media_box->right = r->value;

			r = (HPDF_Real) HPDF_Array_GetItem(array, 3, HPDF_OCLASS_REAL);
			if (r)
				media_box->top = r->value;

			HPDF_CheckError(page->error);
		} else
		{
			HPDF_RaiseError(page->error, HPDF_PAGE_CANNOT_FIND_OBJECT, 0);
		}
	}
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEXOBJECTFROMIMAGE)
HPDF_XObject HPDF_Page_CreateXObjectFromImage(HPDF_Doc pdf, HPDF_Page page, const HPDF_Rect *rect, HPDF_Image image, HPDF_Boolean zoom)
{
	HPDF_Dict resource;
	HPDF_Dict fromxobject;
	HPDF_Dict xobject;
	HPDF_STATUS ret = HPDF_OK;
	HPDF_Array procset;
	HPDF_REAL top, bottom;
	HPDF_Array array1;
	HPDF_Array array2;

	fromxobject = HPDF_DictStream_New(pdf->mmgr, pdf->xref);
	if (!fromxobject)
		return NULL;

	fromxobject->header.obj_class |= HPDF_OSUBCLASS_XOBJECT;

	/* add required elements */
	fromxobject->filter = HPDF_STREAM_FILTER_FLATE_DECODE;

	resource = HPDF_Dict_New(page->mmgr);
	if (!resource)
		return NULL;

	/* althoth ProcSet-entry is obsolete, add it to resouce for
	 * compatibility
	 */

	ret |= HPDF_Dict_Add(fromxobject, "Resources", resource);

	procset = HPDF_Array_New(page->mmgr);
	if (!procset)
		return NULL;

	ret |= HPDF_Dict_Add(resource, "ProcSet", procset);
	ret |= HPDF_Array_Add(procset, HPDF_Name_New(page->mmgr, "PDF"));
	ret |= HPDF_Array_Add(procset, HPDF_Name_New(page->mmgr, "ImageC"));

	xobject = HPDF_Dict_New(page->mmgr);
	if (!xobject)
		return NULL;

	if (HPDF_Dict_Add(resource, "XObject", xobject) != HPDF_OK)
		return NULL;

	if (HPDF_Dict_Add(xobject, "Im1", image) != HPDF_OK)
		return NULL;

	array1 = HPDF_Array_New(page->mmgr);
	if (!array1)
		return NULL;

	if (HPDF_Dict_Add(fromxobject, "BBox", array1) != HPDF_OK)
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

	ret |= HPDF_Array_AddReal(array1, rect->left);
	ret |= HPDF_Array_AddReal(array1, bottom);
	ret |= HPDF_Array_AddReal(array1, rect->right);
	ret |= HPDF_Array_AddReal(array1, top);

	array2 = HPDF_Array_New(page->mmgr);
	if (!array2)
		return NULL;

	if (HPDF_Dict_Add(fromxobject, "Matrix", array2) != HPDF_OK)
		return NULL;

	ret |= HPDF_Array_AddReal(array2, 1.0);
	ret |= HPDF_Array_AddReal(array2, 0.0);
	ret |= HPDF_Array_AddReal(array2, 0.0);
	ret |= HPDF_Array_AddReal(array2, 1.0);
	ret |= HPDF_Array_AddReal(array2, 0.0);
	ret |= HPDF_Array_AddReal(array2, 0.0);

	if (HPDF_Dict_AddNumber(fromxobject, "FormType", 1) != HPDF_OK)
		return NULL;

	if (HPDF_Dict_AddName(fromxobject, "Subtype", "Form") != HPDF_OK)
		return NULL;

	if (HPDF_Dict_AddName(fromxobject, "Type", "XObject") != HPDF_OK)
		return NULL;

	if (HPDF_Stream_WriteStr(fromxobject->stream, "q") != HPDF_OK)
		return NULL;
	if (HPDF_Stream_WriteChar(fromxobject->stream, 0x0A) != HPDF_OK)
		return NULL;

	if (zoom)
	{
		if (HPDF_Stream_WriteReal(fromxobject->stream, rect->right - rect->left) != HPDF_OK)
			return NULL;
		if (HPDF_Stream_WriteStr(fromxobject->stream, " 0 0 ") != HPDF_OK)
			return NULL;
		if (HPDF_Stream_WriteReal(fromxobject->stream, top - bottom) != HPDF_OK)
			return NULL;
		if (HPDF_Stream_WriteStr(fromxobject->stream, " 0 0 cm") != HPDF_OK)
			return NULL;
	} else
	{
		if (HPDF_Stream_WriteStr(fromxobject->stream, "1.0 0 0 1.0 0 0 cm") != HPDF_OK)
			return NULL;
	}


	if (HPDF_Stream_WriteChar(fromxobject->stream, 0x0A) != HPDF_OK)
		return NULL;
	if (HPDF_Stream_WriteStr(fromxobject->stream, "/Im1 Do") != HPDF_OK)
		return NULL;
	if (HPDF_Stream_WriteChar(fromxobject->stream, 0x0A) != HPDF_OK)
		return NULL;
	if (HPDF_Stream_WriteStr(fromxobject->stream, "Q") != HPDF_OK)
		return NULL;

	return fromxobject;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEXOBJECTASWHITERECT)
HPDF_XObject HPDF_Page_CreateXObjectAsWhiteRect(HPDF_Doc pdf, HPDF_Page page, const HPDF_Rect *rect)
{
	HPDF_Dict resource;
	HPDF_Dict fromxobject;
	HPDF_Dict xobject;
	HPDF_STATUS ret = HPDF_OK;
	HPDF_Array procset;
	HPDF_REAL bottom, top;
	HPDF_Array array1;
	HPDF_Array array2;

	fromxobject = HPDF_DictStream_New(pdf->mmgr, pdf->xref);
	if (!fromxobject)
		return NULL;

	fromxobject->header.obj_class |= HPDF_OSUBCLASS_XOBJECT;

	/* add required elements */
	fromxobject->filter = HPDF_STREAM_FILTER_FLATE_DECODE;

	resource = HPDF_Dict_New(page->mmgr);
	if (!resource)
		return NULL;

	/* althoth ProcSet-entry is obsolete, add it to resouce for
	 * compatibility
	 */

	ret |= HPDF_Dict_Add(fromxobject, "Resources", resource);

	procset = HPDF_Array_New(page->mmgr);
	if (!procset)
		return NULL;

	ret |= HPDF_Dict_Add(resource, "ProcSet", procset);
	ret |= HPDF_Array_Add(procset, HPDF_Name_New(page->mmgr, "PDF"));
	ret |= HPDF_Array_Add(procset, HPDF_Name_New(page->mmgr, "ImageC"));

	xobject = HPDF_Dict_New(page->mmgr);
	if (!xobject)
		return NULL;

	if (HPDF_Dict_Add(resource, "XObject", xobject) != HPDF_OK)
		return NULL;

	array1 = HPDF_Array_New(page->mmgr);
	if (!array1)
		return NULL;

	if (HPDF_Dict_Add(fromxobject, "BBox", array1) != HPDF_OK)
		return NULL;

	if (rect->top < rect->bottom)
	{
		bottom = rect->top;
		top = rect->bottom;
	} else
	{
		top = rect->top;
		bottom = rect->bottom;
	}

	ret |= HPDF_Array_AddReal(array1, 0.0);
	ret |= HPDF_Array_AddReal(array1, 0.0);
	ret |= HPDF_Array_AddReal(array1, rect->right - rect->left);
	ret |= HPDF_Array_AddReal(array1, top - bottom);

	array2 = HPDF_Array_New(page->mmgr);
	if (!array2)
		return NULL;

	if (HPDF_Dict_Add(fromxobject, "Matrix", array2) != HPDF_OK)
		return NULL;

	ret |= HPDF_Array_AddReal(array2, 1.0);
	ret |= HPDF_Array_AddReal(array2, 0.0);
	ret |= HPDF_Array_AddReal(array2, 0.0);
	ret |= HPDF_Array_AddReal(array2, 1.0);
	ret |= HPDF_Array_AddReal(array2, 0.0);
	ret |= HPDF_Array_AddReal(array2, 0.0);

	if (HPDF_Dict_AddNumber(fromxobject, "FormType", 1) != HPDF_OK)
		return NULL;

	if (HPDF_Dict_AddName(fromxobject, "Subtype", "Form") != HPDF_OK)
		return NULL;

	if (HPDF_Dict_AddName(fromxobject, "Type", "XObject") != HPDF_OK)
		return NULL;

	if (HPDF_Stream_WriteStr(fromxobject->stream, "1 g") != HPDF_OK)
		return NULL;
	if (HPDF_Stream_WriteChar(fromxobject->stream, 0x0A) != HPDF_OK)
		return NULL;
	if (HPDF_Stream_WriteStr(fromxobject->stream, "0 0 ") != HPDF_OK)
		return NULL;

	if (HPDF_Stream_WriteReal(fromxobject->stream, rect->right - rect->left) != HPDF_OK)
		return NULL;
	if (HPDF_Stream_WriteStr(fromxobject->stream, " ") != HPDF_OK)
		return NULL;
	if (HPDF_Stream_WriteReal(fromxobject->stream, top - bottom) != HPDF_OK)
		return NULL;
	if (HPDF_Stream_WriteStr(fromxobject->stream, " re") != HPDF_OK)
		return NULL;

	if (HPDF_Stream_WriteChar(fromxobject->stream, 0x0A) != HPDF_OK)
		return NULL;
	if (HPDF_Stream_WriteStr(fromxobject->stream, "f") != HPDF_OK)
		return NULL;

	return fromxobject;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETXOBJECTNAME)
const char *HPDF_Page_GetXObjectName(HPDF_Page page, HPDF_XObject xobj)
{
	HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;
	const char *key;

	if (!attr->xobjects)
	{
		HPDF_Dict resources;
		HPDF_Dict xobjects;

		resources = (HPDF_Dict) HPDF_Page_GetInheritableItem(page, "Resources", HPDF_OCLASS_DICT);
		if (!resources)
			return NULL;

		xobjects = HPDF_Dict_New(page->mmgr);
		if (!xobjects)
			return NULL;

		if (HPDF_Dict_Add(resources, "XObject", xobjects) != HPDF_OK)
			return NULL;

		attr->xobjects = xobjects;
	}

	/* search xobject-object from xobject-resource */
	key = HPDF_Dict_GetKeyByObj(attr->xobjects, xobj);
	if (!key)
	{
		/* if the xobject is not registered in xobject-resource, register
		 * xobject to xobject-resource.
		 */
		char xobj_name[HPDF_LIMIT_MAX_NAME_LEN + 1];

		strcpy(xobj_name, "X");
		HPDF_IToA(xobj_name + 1, attr->xobjects->list->count + 1, xobj_name + HPDF_LIMIT_MAX_NAME_LEN);

		if (HPDF_Dict_Add(attr->xobjects, xobj_name, xobj) != HPDF_OK)
			return NULL;

		key = HPDF_Dict_GetKeyByObj(attr->xobjects, xobj);
	}

	return key;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETEXTGSTATENAME)
const char *HPDF_Page_GetExtGStateName(HPDF_Page page, HPDF_ExtGState state)
{
	HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;
	const char *key;

	if (!attr->ext_gstates)
	{
		HPDF_Dict resources;
		HPDF_Dict ext_gstates;

		resources = (HPDF_Dict) HPDF_Page_GetInheritableItem(page, "Resources", HPDF_OCLASS_DICT);
		if (!resources)
			return NULL;

		ext_gstates = HPDF_Dict_New(page->mmgr);
		if (!ext_gstates)
			return NULL;

		if (HPDF_Dict_Add(resources, "ExtGState", ext_gstates) != HPDF_OK)
			return NULL;

		attr->ext_gstates = ext_gstates;
	}

	/* search ext_gstate-object from ext_gstate-resource */
	key = HPDF_Dict_GetKeyByObj(attr->ext_gstates, state);
	if (!key)
	{
		/* if the ext-gstate is not registered in ext-gstate resource, register
		 *  to ext-gstate resource.
		 */
		char ext_gstate_name[HPDF_LIMIT_MAX_NAME_LEN + 1];

		strcpy(ext_gstate_name, "E");
		HPDF_IToA(ext_gstate_name + 1, attr->ext_gstates->list->count + 1, ext_gstate_name + HPDF_LIMIT_MAX_NAME_LEN);

		if (HPDF_Dict_Add(attr->ext_gstates, ext_gstate_name, state) != HPDF_OK)
			return NULL;

		key = HPDF_Dict_GetKeyByObj(attr->ext_gstates, state);
	}

	return key;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_ADDANNOTATION)
HPDF_STATUS HPDF_Page_AddAnnotation(HPDF_Page page, HPDF_Annotation annot)
{
	HPDF_Array array;
	HPDF_STATUS ret = HPDF_OK;

	/* find "Annots" entry */
	array = (HPDF_Array) HPDF_Dict_GetItem(page, "Annots", HPDF_OCLASS_ARRAY);

	if (!array)
	{
		array = HPDF_Array_New(page->mmgr);
		if (!array)
			return HPDF_Error_GetCode(page->error);

		ret = HPDF_Dict_Add(page, "Annots", array);
		if (ret != HPDF_OK)
			return ret;
	}

	if ((ret = HPDF_Array_Add(array, annot)) != HPDF_OK)
		return ret;

	/* Add Parent to the annotation  */
	ret = HPDF_Dict_Add(annot, "P", page);

	return ret;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_TEXTWIDTH)
HPDF_REAL HPDF_Page_TextWidth(HPDF_Page page, const char *text)
{
	HPDF_PageAttr attr;
	HPDF_TextWidth tw;
	HPDF_REAL ret = 0;
	HPDF_UINT len = HPDF_StrLen(text, HPDF_LIMIT_MAX_STRING_LEN + 1);

	if (!HPDF_Page_Validate(page) || len == 0)
		return 0;

	attr = (HPDF_PageAttr) page->attr;

	/* no font exists */
	if (!attr->gstate->font)
	{
		HPDF_RaiseError(page->error, HPDF_PAGE_FONT_NOT_FOUND, 0);
		return 0;
	}

	HPDF_Font_TextWidth(attr->gstate->font, (const HPDF_BYTE *) text, len, &tw);

	ret += attr->gstate->word_space * tw.numspace;
	ret += tw.width * attr->gstate->font_size / 1000;
	ret += attr->gstate->char_space * tw.numchars;

	HPDF_CheckError(page->error);

	return ret;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_MEASURETEXT)
HPDF_UINT HPDF_Page_MeasureText(HPDF_Page page, const char *text, HPDF_REAL width, HPDF_BOOL wordwrap, HPDF_REAL *real_width)
{
	HPDF_PageAttr attr;
	HPDF_UINT len = HPDF_StrLen(text, HPDF_LIMIT_MAX_STRING_LEN + 1);
	HPDF_UINT ret;

	if (!HPDF_Page_Validate(page) || len == 0)
		return 0;

	attr = (HPDF_PageAttr) page->attr;

	/* no font exists */
	if (!attr->gstate->font)
	{
		HPDF_RaiseError(page->error, HPDF_PAGE_FONT_NOT_FOUND, 0);
		return 0;
	}

	ret = HPDF_Font_MeasureText(attr->gstate->font, (const HPDF_BYTE *) text, len, width,
								attr->gstate->font_size, attr->gstate->char_space,
								attr->gstate->word_space, wordwrap, real_width);

	HPDF_CheckError(page->error);

	return ret;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETWIDTH)
HPDF_REAL HPDF_Page_GetWidth(HPDF_Page page)
{
	HPDF_Box box;
	HPDF_Page_GetMediaBox(page, &box);
	return box.right;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETHEIGHT)
HPDF_REAL HPDF_Page_GetHeight(HPDF_Page page)
{
	HPDF_Box box;
	HPDF_Page_GetMediaBox(page, &box);
	return box.top;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETCURRENTFONT)
HPDF_Font HPDF_Page_GetCurrentFont(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->font;
	}
	return NULL;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETCURRENTFONTSIZE)
HPDF_REAL HPDF_Page_GetCurrentFontSize(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		if (attr->gstate->font)
			return attr->gstate->font_size;
	}
	return 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETTRANSMATRIX)
void HPDF_Page_GetTransMatrix(HPDF_Page page, HPDF_TransMatrix *matrix)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		*matrix = attr->gstate->trans_matrix;
		return;
	}

	matrix->a = 1;
	matrix->b = 0;
	matrix->c = 0;
	matrix->d = 1;
	matrix->x = 0;
	matrix->y = 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETLINEWIDTH)
HPDF_REAL HPDF_Page_GetLineWidth(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->line_width;
	}
	return HPDF_DEF_LINEWIDTH;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETLINECAP)
HPDF_LineCap HPDF_Page_GetLineCap(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->line_cap;
	}
	return HPDF_DEF_LINECAP;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETLINEJOIN)
HPDF_LineJoin HPDF_Page_GetLineJoin(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->line_join;
	}
	return HPDF_DEF_LINEJOIN;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETMITERLIMIT)
HPDF_REAL HPDF_Page_GetMiterLimit(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->miter_limit;
	}
	return HPDF_DEF_MITERLIMIT;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETDASH)
void HPDF_Page_GetDash(HPDF_Page page, HPDF_DashMode *mode)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		*mode = attr->gstate->dash_mode;
		return;
	}

	mode->ptn[0] = 0;
	mode->ptn[1] = 0;
	mode->ptn[2] = 0;
	mode->ptn[3] = 0;
	mode->ptn[4] = 0;
	mode->ptn[5] = 0;
	mode->ptn[6] = 0;
	mode->ptn[7] = 0;
	mode->num_ptn = 0;
	mode->phase = 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETFLAT)
HPDF_REAL HPDF_Page_GetFlat(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->flatness;
	}
	return HPDF_DEF_FLATNESS;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETWORDSPACE)
HPDF_REAL HPDF_Page_GetWordSpace(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->word_space;
	}
	return HPDF_DEF_WORDSPACE;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETCHARSPACE)
HPDF_REAL HPDF_Page_GetCharSpace(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->char_space;
	}
	return HPDF_DEF_CHARSPACE;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETHORIZONTALSCALING)
HPDF_REAL HPDF_Page_GetHorizontalScaling(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->h_scaling;
	}
	return HPDF_DEF_HSCALING;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETTEXTLEADING)
HPDF_REAL HPDF_Page_GetTextLeading(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->text_leading;
	}
	return HPDF_DEF_LEADING;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETTEXTRENDERINGMODE)
HPDF_TextRenderingMode HPDF_Page_GetTextRenderingMode(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->rendering_mode;
	}
	return HPDF_DEF_RENDERING_MODE;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETTEXTRISE)
HPDF_REAL HPDF_Page_GetTextRise(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->text_rise;
	}
	return HPDF_DEF_RISE;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETRGBFILL)
void HPDF_Page_GetRGBFill(HPDF_Page page, HPDF_RGBColor *color)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		if (attr->gstate->cs_fill == HPDF_CS_DEVICE_RGB)
		{
			*color = attr->gstate->rgb_fill;
			return;
		}
	}

	color->r = color->g = color->b = 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETRGBSTROKE)
void HPDF_Page_GetRGBStroke(HPDF_Page page, HPDF_RGBColor *color)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		if (attr->gstate->cs_stroke == HPDF_CS_DEVICE_RGB)
		{
			*color = attr->gstate->rgb_stroke;
			return;
		}
	}

	color->r = color->g = color->b = 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETCMYKFILL)
void HPDF_Page_GetCMYKFill(HPDF_Page page, HPDF_CMYKColor *color)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		if (attr->gstate->cs_fill == HPDF_CS_DEVICE_CMYK)
		{
			*color = attr->gstate->cmyk_fill;
			return;
		}
	}

	color->c = color->m = color->y = color->k = 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETCMYKSTROKE)
void HPDF_Page_GetCMYKStroke(HPDF_Page page, HPDF_CMYKColor *color)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		if (attr->gstate->cs_stroke == HPDF_CS_DEVICE_CMYK)
		{
			*color = attr->gstate->cmyk_stroke;
			return;
		}
	}

	color->c = color->m = color->y = color->k = 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETGRAYFILL)
HPDF_REAL HPDF_Page_GetGrayFill(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		if (attr->gstate->cs_fill == HPDF_CS_DEVICE_GRAY)
			return attr->gstate->gray_fill;
	}

	return 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETGRAYSTROKE)
HPDF_REAL HPDF_Page_GetGrayStroke(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		if (attr->gstate->cs_stroke == HPDF_CS_DEVICE_GRAY)
			return attr->gstate->gray_stroke;
	}

	return 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETSTROKINGCOLORSPACE)
HPDF_ColorSpace HPDF_Page_GetStrokingColorSpace(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
		return ((HPDF_PageAttr) page->attr)->gstate->cs_stroke;

	return HPDF_CS_EOF;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETFILLINGCOLORSPACE)
HPDF_ColorSpace HPDF_Page_GetFillingColorSpace(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
		return ((HPDF_PageAttr) page->attr)->gstate->cs_fill;

	return HPDF_CS_EOF;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETTEXTMATRIX)
void HPDF_Page_GetTextMatrix(HPDF_Page page, HPDF_TransMatrix *matrix)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		*matrix = attr->text_matrix;
		return;
	}

	matrix->a = 1;
	matrix->b = 0;
	matrix->c = 0;
	matrix->d = 1;
	matrix->x = 0;
	matrix->y = 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETGSTATEDEPTH)
HPDF_UINT HPDF_Page_GetGStateDepth(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gstate->depth;
	}
	return 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETGMODE)
HPDF_UINT16 HPDF_Page_GetGMode(HPDF_Page page)
{
	if (HPDF_Page_Validate(page))
	{
		HPDF_PageAttr attr = (HPDF_PageAttr) page->attr;

		return attr->gmode;
	}

	return 0;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETCURRENTPOS)
HPDF_STATUS HPDF_Page_GetCurrentPos(HPDF_Page page, HPDF_Point *pos)
{
	HPDF_PageAttr attr;

	pos->x = 0;
	pos->y = 0;
	if (!HPDF_Page_Validate(page))
		return HPDF_INVALID_PAGE;

	attr = (HPDF_PageAttr) page->attr;

	if (attr->gmode & HPDF_GMODE_PATH_OBJECT)
		*pos = attr->cur_pos;

	return HPDF_OK;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_GETCURRENTTEXTPOS)
HPDF_STATUS HPDF_Page_GetCurrentTextPos(HPDF_Page page, HPDF_Point *pos)
{
	HPDF_PageAttr attr;

	pos->x = 0;
	pos->y = 0;
	if (!HPDF_Page_Validate(page))
		return HPDF_INVALID_PAGE;

	attr = (HPDF_PageAttr) page->attr;

	if (attr->gmode & HPDF_GMODE_TEXT_OBJECT)
		*pos = attr->text_pos;

	return HPDF_OK;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_SETBOXVALUE)
HPDF_STATUS HPDF_Page_SetBoxValue(HPDF_Page page, const char *name, HPDF_UINT index, HPDF_REAL value)
{
	HPDF_Real r;
	HPDF_Array array;

	if (!HPDF_Page_Validate(page))
		return HPDF_INVALID_PAGE;

	array = (HPDF_Array) HPDF_Page_GetInheritableItem(page, name, HPDF_OCLASS_ARRAY);
	if (!array)
		return HPDF_SetError(page->error, HPDF_PAGE_CANNOT_FIND_OBJECT, 0);

	r = (HPDF_Real) HPDF_Array_GetItem(array, index, HPDF_OCLASS_REAL);
	if (!r)
		return HPDF_SetError(page->error, HPDF_PAGE_INVALID_INDEX, index);

	r->value = value;

	return HPDF_OK;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_SETROTATE)
HPDF_STATUS HPDF_Page_SetRotate(HPDF_Page page, HPDF_UINT16 angle)
{
	HPDF_Number n;
	HPDF_STATUS ret = HPDF_OK;

	if (!HPDF_Page_Validate(page))
		return HPDF_INVALID_PAGE;

	if ((angle % 90) != 0)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_ROTATE_VALUE, angle);

	n = (HPDF_Number) HPDF_Page_GetInheritableItem(page, "Rotate", HPDF_OCLASS_NUMBER);

	if (!n)
		ret = HPDF_Dict_AddNumber(page, "Rotate", angle);
	else
		n->value = angle;

	return ret;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_SETZOOM)
HPDF_STATUS HPDF_Page_SetZoom(HPDF_Page page, HPDF_REAL zoom)
{
	HPDF_STATUS ret = HPDF_OK;

	if (!HPDF_Page_Validate(page))
	{
		return HPDF_INVALID_PAGE;
	}

	if (zoom < 0.08 || zoom > 32)
	{
		return HPDF_RaiseError(page->error, HPDF_INVALID_PARAMETER, 0);
	}

	ret = HPDF_Dict_AddReal(page, "PZ", zoom);
	return ret;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_SETWIDTH)
HPDF_STATUS HPDF_Page_SetWidth(HPDF_Page page, HPDF_REAL value)
{
	if (value < HPDF_MIN_PAGE_SIZE || value > HPDF_MAX_PAGE_SIZE)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_SIZE, value);

	if (HPDF_Page_SetBoxValue(page, "MediaBox", 2, value) != HPDF_OK)
		return HPDF_CheckError(page->error);

	return HPDF_OK;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_SETHEIGHT)
HPDF_STATUS HPDF_Page_SetHeight(HPDF_Page page, HPDF_REAL value)
{
	if (value < HPDF_MIN_PAGE_SIZE || value > HPDF_MAX_PAGE_SIZE)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_SIZE, value);

	if (HPDF_Page_SetBoxValue(page, "MediaBox", 3, value) != HPDF_OK)
		return HPDF_CheckError(page->error);

	return HPDF_OK;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_SETSIZE)
HPDF_STATUS HPDF_Page_SetSize(HPDF_Page page, HPDF_PageSizes size, HPDF_PageDirection direction)
{
	HPDF_STATUS ret = HPDF_OK;

	if (!HPDF_Page_Validate(page))
		return HPDF_INVALID_PAGE;

	if ((unsigned int)size >= HPDF_PAGE_SIZE_EOF)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_SIZE, size);

	if (direction == HPDF_PAGE_LANDSCAPE)
	{
		ret |= HPDF_Page_SetHeight(page, HPDF_PREDEFINED_PAGE_SIZES[size].x);
		ret |= HPDF_Page_SetWidth(page, HPDF_PREDEFINED_PAGE_SIZES[size].y);
	} else if (direction == HPDF_PAGE_PORTRAIT)
	{
		ret |= HPDF_Page_SetHeight(page, HPDF_PREDEFINED_PAGE_SIZES[size].y);
		ret |= HPDF_Page_SetWidth(page, HPDF_PREDEFINED_PAGE_SIZES[size].x);
	} else
	{
		ret = HPDF_SetError(page->error, HPDF_PAGE_INVALID_DIRECTION, direction);
	}

	if (ret != HPDF_OK)
		return HPDF_CheckError(page->error);

	return HPDF_OK;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_VALIDATE)
HPDF_BOOL HPDF_Page_Validate(HPDF_Page page)
{
	HPDF_Obj_Header *header = (HPDF_Obj_Header *) page;

	if (!page || !page->attr)
		return HPDF_FALSE;

	if (header->obj_class != (HPDF_OCLASS_DICT | HPDF_OSUBCLASS_PAGE))
		return HPDF_FALSE;

	return HPDF_TRUE;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEDESTINATION)
HPDF_Destination HPDF_Page_CreateDestination(HPDF_Page page)
{
	HPDF_PageAttr attr;
	HPDF_Destination dst;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	dst = HPDF_Destination_New(page->mmgr, page, attr->xref);
	if (!dst)
		HPDF_CheckError(page->error);

	return dst;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATE3DANNOT)
HPDF_Annotation HPDF_Page_Create3DAnnot(HPDF_Page page, const HPDF_Rect *rect, HPDF_BOOL tb, HPDF_BOOL np, HPDF_U3D u3d, HPDF_Image ap)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	annot = HPDF_3DAnnot_New(page->mmgr, attr->xref, rect, tb, np, u3d, ap);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATETEXTANNOT)
HPDF_Annotation HPDF_Page_CreateTextAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *text, HPDF_Encoder encoder)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	if (encoder && !HPDF_Encoder_Validate(encoder))
	{
		HPDF_RaiseError(page->error, HPDF_INVALID_ENCODER, 0);
		return NULL;
	}

	annot = HPDF_MarkupAnnot_New(page->mmgr, attr->xref, rect, text, encoder, HPDF_ANNOT_TEXT);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEFREETEXTANNOT)
HPDF_Annotation HPDF_Page_CreateFreeTextAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *text, HPDF_Encoder encoder)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	if (encoder && !HPDF_Encoder_Validate(encoder))
	{
		HPDF_RaiseError(page->error, HPDF_INVALID_ENCODER, 0);
		return NULL;
	}

	annot = HPDF_MarkupAnnot_New(page->mmgr, attr->xref, rect, text, encoder, HPDF_ANNOT_FREE_TEXT);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATELINEANNOT)
HPDF_Annotation HPDF_Page_CreateLineAnnot(HPDF_Page page, const char *text, HPDF_Encoder encoder)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;
	HPDF_Rect rect = { 0, 0, 0, 0 };

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	if (encoder && !HPDF_Encoder_Validate(encoder))
	{
		HPDF_RaiseError(page->error, HPDF_INVALID_ENCODER, 0);
		return NULL;
	}

	annot = HPDF_MarkupAnnot_New(page->mmgr, attr->xref, &rect, text, encoder, HPDF_ANNOT_LINE);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEWIDGETANNOT)
HPDF_Annotation HPDF_Page_CreateWidgetAnnot(HPDF_Page page, const HPDF_Rect *rect)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	annot = HPDF_WidgetAnnot_New(page->mmgr, attr->xref, rect);

	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEWIDGETANNOT_WHITEONLYWHILEPRINT)
HPDF_Annotation HPDF_Page_CreateWidgetAnnot_WhiteOnlyWhilePrint(HPDF_Doc pdf, HPDF_Page page, const HPDF_Rect *rect)
{
	HPDF_XObject fxobj;
	HPDF_Annotation annot;
	HPDF_Dict appearence;
	HPDF_Dict mk;
	HPDF_STATUS ret = HPDF_OK;
	HPDF_Array array_bg;

	annot = HPDF_Page_CreateWidgetAnnot(page, rect);

	fxobj = HPDF_Page_CreateXObjectAsWhiteRect(pdf, page, rect);
	if (!fxobj)
		return NULL;

	appearence = HPDF_Dict_New(annot->mmgr);
	if (!appearence)
		return NULL;

	ret = HPDF_Dict_Add(annot, "AP", appearence);
	if (ret != HPDF_OK)
		return NULL;

	ret = HPDF_Dict_Add(appearence, "N", fxobj);
	if (ret != HPDF_OK)
		return NULL;

	mk = HPDF_Dict_New(annot->mmgr);
	if (!mk)
		return NULL;

	ret = HPDF_Dict_Add(annot, "MK", mk);
	if (ret != HPDF_OK)
		return NULL;

	array_bg = HPDF_Array_New(annot->mmgr);
	if (!array_bg)
		return NULL;

	if (HPDF_Dict_Add(mk, "BG", array_bg) != HPDF_OK)
		return NULL;

	ret = HPDF_Array_AddReal(array_bg, 1.0);
	ret |= HPDF_Array_AddReal(array_bg, 1.0);
	ret |= HPDF_Array_AddReal(array_bg, 1.0);

	ret |= HPDF_Dict_AddName(annot, "FT", "Btn");
	if (ret != HPDF_OK)
		return NULL;

	ret = HPDF_Dict_AddNumber(annot, "F", 36);
	if (ret != HPDF_OK)
		return NULL;

	ret = HPDF_Dict_Add(annot, "T", HPDF_String_New(annot->mmgr, "Blind", NULL));
	if (ret != HPDF_OK)
		return NULL;

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATELINKANNOT)
HPDF_Annotation HPDF_Page_CreateLinkAnnot(HPDF_Page page, const HPDF_Rect *rect, HPDF_Destination dst)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	if (dst)
	{
		if (!HPDF_Destination_Validate(dst))
		{
			HPDF_RaiseError(page->error, HPDF_INVALID_DESTINATION, 0);
			return NULL;
		}
	}

	annot = HPDF_LinkAnnot_New(page->mmgr, attr->xref, rect, dst);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEGOTOANNOT)
HPDF_Annotation HPDF_Page_CreateGoToAnnot(HPDF_Page page, const HPDF_Rect *rect, HPDF_Destination dst)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	if (dst)
	{
		if (!HPDF_Destination_Validate(dst))
		{
			HPDF_RaiseError(page->error, HPDF_INVALID_DESTINATION, 0);
			return NULL;
		}
	}

	annot = HPDF_GoToLinkAnnot_New(page->mmgr, attr->xref, rect, dst);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEGOTORANNOT)
HPDF_Annotation HPDF_Page_CreateGoToRAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *file, const char *destname, HPDF_BOOL newwindow)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	annot = HPDF_GoToRLinkAnnot_New(page->mmgr, attr->xref, rect, file, destname, newwindow);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATENAMEDANNOT)
HPDF_Annotation HPDF_Page_CreateNamedAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *type)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	annot = HPDF_NamedLinkAnnot_New(page->mmgr, attr->xref, rect, type);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATELAUNCHANNOT)
HPDF_Annotation HPDF_Page_CreateLaunchAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *file, const char *args, const char *type)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	annot = HPDF_LaunchLinkAnnot_New(page->mmgr, attr->xref, rect, file, args, type);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEURILINKANNOT)
HPDF_Annotation HPDF_Page_CreateURILinkAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *uri)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_StrLen(uri, HPDF_LIMIT_MAX_STRING_LEN) <= 0)
	{
		HPDF_RaiseError(page->error, HPDF_INVALID_URI, 0);
		return NULL;
	}

	annot = HPDF_URILinkAnnot_New(page->mmgr, attr->xref, rect, uri);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATECIRCLEANNOT)
HPDF_Annotation HPDF_Page_CreateCircleAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *text, HPDF_Encoder encoder)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	if (encoder && !HPDF_Encoder_Validate(encoder))
	{
		HPDF_RaiseError(page->error, HPDF_INVALID_ENCODER, 0);
		return NULL;
	}

	annot = HPDF_MarkupAnnot_New(page->mmgr, attr->xref, rect, text, encoder, HPDF_ANNOT_CIRCLE);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATESQUAREANNOT)
HPDF_Annotation HPDF_Page_CreateSquareAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *text, HPDF_Encoder encoder)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	if (encoder && !HPDF_Encoder_Validate(encoder))
	{
		HPDF_RaiseError(page->error, HPDF_INVALID_ENCODER, 0);
		return NULL;
	}

	annot = HPDF_MarkupAnnot_New(page->mmgr, attr->xref, rect, text, encoder, HPDF_ANNOT_SQUARE);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATE3DVIEW)
HPDF_Dict HPDF_Page_Create3DView(HPDF_Page page, HPDF_U3D u3d, HPDF_Annotation annot3d, const char *name)
{
	HPDF_PageAttr attr;
	HPDF_Dict view;

	HPDF_UNUSED(annot3d);

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	view = HPDF_3DView_New(page->mmgr, attr->xref, u3d, name);
	if (!view)
	{
		HPDF_CheckError(page->error);
	}
	return view;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATETEXTMARKUPANNOT)
HPDF_Annotation HPDF_Page_CreateTextMarkupAnnot(
	HPDF_Page page,
	const HPDF_Rect *rect,
	const char *text,
	HPDF_Encoder encoder,
	HPDF_AnnotType subType)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	if (encoder && !HPDF_Encoder_Validate(encoder))
	{
		HPDF_RaiseError(page->error, HPDF_INVALID_ENCODER, 0);
		return NULL;
	}

	annot = HPDF_MarkupAnnot_New(page->mmgr, attr->xref, rect, text, encoder, subType);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEHIGHLIGHTANNOT)
HPDF_Annotation HPDF_Page_CreateHighlightAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *text, HPDF_Encoder encoder)
{
	return HPDF_Page_CreateTextMarkupAnnot(page, rect, text, encoder, HPDF_ANNOT_HIGHLIGHT);
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATESQUIGGLYANNOT)
HPDF_Annotation HPDF_Page_CreateSquigglyAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *text, HPDF_Encoder encoder)
{
	return HPDF_Page_CreateTextMarkupAnnot(page, rect, text, encoder, HPDF_ANNOT_SQUIGGLY);
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEUNDERLINEANNOT)
HPDF_Annotation HPDF_Page_CreateUnderlineAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *text, HPDF_Encoder encoder)
{
	return HPDF_Page_CreateTextMarkupAnnot(page, rect, text, encoder, HPDF_ANNOT_UNDERLINE);
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATESTRIKEOUTANNOT)
HPDF_Annotation HPDF_Page_CreateStrikeOutAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *text, HPDF_Encoder encoder)
{
	return HPDF_Page_CreateTextMarkupAnnot(page, rect, text, encoder, HPDF_ANNOT_STRIKE_OUT);
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEPOPUPANNOT)
HPDF_Annotation HPDF_Page_CreatePopupAnnot(HPDF_Page page, const HPDF_Rect *rect, HPDF_Annotation parent)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	annot = HPDF_PopupAnnot_New(page->mmgr, attr->xref, rect, parent);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATESTAMPANNOT)
HPDF_Annotation HPDF_Page_CreateStampAnnot(
	HPDF_Page page,
	const HPDF_Rect *rect,
	HPDF_StampAnnotName name,
	const char *text,
	HPDF_Encoder encoder)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	annot = HPDF_StampAnnot_New(page->mmgr, attr->xref, rect, name, text, encoder);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEPROJECTIONANNOT)
HPDF_Annotation HPDF_Page_CreateProjectionAnnot(HPDF_Page page, const HPDF_Rect *rect, const char *text, HPDF_Encoder encoder)
{
	HPDF_PageAttr attr;
	HPDF_Annotation annot;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	annot = HPDF_ProjectionAnnot_New(page->mmgr, attr->xref, rect, text, encoder);
	if (annot)
	{
		if (HPDF_Page_AddAnnotation(page, annot) != HPDF_OK)
		{
			HPDF_CheckError(page->error);
			annot = NULL;
		}
	} else
	{
		HPDF_CheckError(page->error);
	}

	return annot;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATE3DC3DMEASURE)
HPDF_3DMeasure HPDF_Page_Create3DC3DMeasure(HPDF_Page page, const HPDF_Point3D *firstanchorpoint, const HPDF_Point3D *textanchorpoint)
{
	HPDF_PageAttr attr;
	HPDF_Annotation measure;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	measure = HPDF_3DC3DMeasure_New(page->mmgr, attr->xref, firstanchorpoint, textanchorpoint);
	if (!measure)
		HPDF_CheckError(page->error);

	return measure;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATEPD33DMEASURE)
HPDF_3DMeasure HPDF_Page_CreatePD33DMeasure(
	HPDF_Page page,
	const HPDF_Point3D *annotationPlaneNormal,
	const HPDF_Point3D *firstAnchorPoint,
	const HPDF_Point3D *secondAnchorPoint,
	const HPDF_Point3D *leaderLinesDirection,
	const HPDF_Point3D *measurementValuePoint,
	const HPDF_Point3D *textYDirection,
	HPDF_REAL value,
	const char *unitsString)
{
	HPDF_PageAttr attr;
	HPDF_Annotation measure;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	measure = HPDF_PD33DMeasure_New(page->mmgr,
									attr->xref,
									annotationPlaneNormal,
									firstAnchorPoint,
									secondAnchorPoint,
									leaderLinesDirection, measurementValuePoint, textYDirection, value, unitsString);
	if (!measure)
		HPDF_CheckError(page->error);

	return measure;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_CREATE3DANNOTEXDATA)
HPDF_ExData HPDF_Page_Create3DAnnotExData(HPDF_Page page)
{
	HPDF_PageAttr attr;
	HPDF_Annotation exData;

	if (!HPDF_Page_Validate(page))
		return NULL;

	attr = (HPDF_PageAttr) page->attr;

	exData = HPDF_3DAnnotExData_New(page->mmgr, attr->xref);
	if (!exData)
		HPDF_CheckError(page->error);

	return exData;
}
#endif


#if defined(PAGES_ALL) || defined(PAGE_SETFILTER)
void HPDF_Page_SetFilter(HPDF_Page page, HPDF_UINT filter)
{
	HPDF_PageAttr attr;

	attr = (HPDF_PageAttr) page->attr;
	attr->contents->filter = filter;
}
#endif
