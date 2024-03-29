/*
 * << Haru Free PDF Library >> -- hpdf_null.c
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


HPDF_Null HPDF_Null_New(HPDF_MMgr mmgr)
{
	HPDF_Null obj = (HPDF_Null) HPDF_GetMem(mmgr, sizeof(HPDF_Null_Rec));

	if (obj)
	{
		OBJ_SET_NEW(obj, HPDF_OCLASS_NULL);
	}

	return obj;
}
