/*
 * << Haru Free PDF Library >> -- hpdf_objects.c
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

void HPDF_Obj_Free(HPDF_MMgr mmgr, void *obj)
{
	HPDF_Obj_Header *header;

	if (!obj)
		return;

	header = (HPDF_Obj_Header *) obj;

	if (!(header->obj_id & HPDF_OTYPE_INDIRECT))
		HPDF_Obj_ForceFree(mmgr, obj);
}


void HPDF_Obj_ForceFree(HPDF_MMgr mmgr, void *obj)
{
	HPDF_Obj_Header *header;

	if (!obj)
		return;

	header = (HPDF_Obj_Header *) obj;

	switch (header->obj_class & HPDF_OCLASS_ANY)
	{
	case HPDF_OCLASS_STRING:
		HPDF_String_Free((HPDF_String) obj);
		break;
	case HPDF_OCLASS_BINARY:
		HPDF_Binary_Free((HPDF_Binary) obj);
		break;
	case HPDF_OCLASS_ARRAY:
		HPDF_Array_Free((HPDF_Array) obj);
		break;
	case HPDF_OCLASS_DICT:
		HPDF_Dict_Free((HPDF_Dict) obj);
		break;
	default:
		HPDF_FreeMem(mmgr, obj);
		break;
	}
}


HPDF_STATUS HPDF_Obj_Write(void *obj, HPDF_Stream stream, HPDF_Encrypt e)
{
	HPDF_Obj_Header *header = (HPDF_Obj_Header *) obj;

	if (header->obj_id & HPDF_OTYPE_HIDDEN)
	{
		return HPDF_OK;
	}

	if (header->obj_class == HPDF_OCLASS_PROXY)
	{
		char buf[HPDF_SHORT_BUF_SIZ];
		char *pbuf = buf;
		char *eptr = buf + HPDF_SHORT_BUF_SIZ - 1;
		HPDF_Proxy p = (HPDF_Proxy) obj;

		header = (HPDF_Obj_Header *) p->obj;

		pbuf = HPDF_IToA(pbuf, header->obj_id & 0x00FFFFFFUL, eptr);
		*pbuf++ = ' ';
		pbuf = HPDF_IToA(pbuf, header->gen_no, eptr);
		HPDF_StrCpy(pbuf, " R", eptr);

		return HPDF_Stream_WriteStr(stream, buf);
	}

	return HPDF_Obj_WriteValue(obj, stream, e);
}


HPDF_STATUS HPDF_Obj_WriteValue(void *obj, HPDF_Stream stream, HPDF_Encrypt e)
{
	HPDF_Obj_Header *header;
	HPDF_STATUS ret;

	header = (HPDF_Obj_Header *) obj;

	switch (header->obj_class & HPDF_OCLASS_ANY)
	{
	case HPDF_OCLASS_NAME:
		ret = HPDF_Name_Write((HPDF_Name) obj, stream);
		break;
	case HPDF_OCLASS_NUMBER:
		ret = HPDF_Number_Write((HPDF_Number) obj, stream);
		break;
	case HPDF_OCLASS_REAL:
		ret = HPDF_Real_Write((HPDF_Real) obj, stream);
		break;
	case HPDF_OCLASS_STRING:
		ret = HPDF_String_Write((HPDF_String) obj, stream, e);
		break;
	case HPDF_OCLASS_BINARY:
		ret = HPDF_Binary_Write((HPDF_Binary) obj, stream, e);
		break;
	case HPDF_OCLASS_ARRAY:
		ret = HPDF_Array_Write((HPDF_Array) obj, stream, e);
		break;
	case HPDF_OCLASS_DICT:
		ret = HPDF_Dict_Write((HPDF_Dict) obj, stream, e);
		break;
	case HPDF_OCLASS_BOOLEAN:
		ret = HPDF_Boolean_Write((HPDF_Boolean) obj, stream);
		break;
	case HPDF_OCLASS_NULL:
		ret = HPDF_Stream_WriteStr(stream, "null");
		break;
	default:
		ret = HPDF_ERR_UNKNOWN_CLASS;
		break;
	}

	return ret;
}


HPDF_Proxy HPDF_Proxy_New(HPDF_MMgr mmgr, void *obj)
{
	HPDF_Proxy p = (HPDF_Proxy) HPDF_GetMem(mmgr, sizeof(HPDF_Proxy_Rec));

	if (p)
	{
		OBJ_SET_NEW(p, HPDF_OCLASS_PROXY);
		p->obj = obj;
	}

	return p;
}
