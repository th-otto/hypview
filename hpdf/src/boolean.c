/*
 * << Haru Free PDF Library >> -- hpdf_boolean.c
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
#include "hpdf/objects.h"
#include <string.h>


HPDF_Boolean HPDF_Boolean_New(HPDF_MMgr mmgr, HPDF_BOOL value)
{
	HPDF_Boolean obj = (HPDF_Boolean) HPDF_GetMem(mmgr, sizeof(HPDF_Boolean_Rec));

	if (obj)
	{
		OBJ_SET_NEW(obj, HPDF_OCLASS_BOOLEAN);
		obj->value = value;
	}

	return obj;
}


HPDF_STATUS HPDF_Boolean_Write(HPDF_Boolean obj, HPDF_Stream stream)
{
	HPDF_STATUS ret;

	ret = HPDF_Stream_WriteStr(stream, obj->value ? "true" : "false");

	return ret;
}
