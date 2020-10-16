/*
 * << Haru Free PDF Library >> -- hpdf_number.c
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


HPDF_Number HPDF_Number_New(HPDF_MMgr mmgr, HPDF_INT32 value)
{
	HPDF_Number obj = (HPDF_Number) HPDF_GetMem(mmgr, sizeof(HPDF_Number_Rec));

	if (obj)
	{
		OBJ_SET_NEW(obj, HPDF_OCLASS_NUMBER);
		obj->value = value;
	}

	return obj;
}


HPDF_STATUS HPDF_Number_Write(HPDF_Number obj, HPDF_Stream stream)
{
	return HPDF_Stream_WriteInt(stream, obj->value);
}


void HPDF_Number_SetValue(HPDF_Number obj, HPDF_INT32 value)
{
	obj->value = value;
}
