/*
 * << Haru Free PDF Library >> -- hpdf_array.c
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

#ifdef __PUREC__
# define ARRAY_ALL
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_NEW)
HPDF_Array HPDF_Array_New(HPDF_MMgr mmgr)
{
	HPDF_Array obj;

	obj = (HPDF_Array) HPDF_GetMem(mmgr, sizeof(HPDF_Array_Rec));
	if (obj)
	{
		memset(obj, 0, sizeof(HPDF_Array_Rec));
		obj->header.obj_class = HPDF_OCLASS_ARRAY;
		obj->mmgr = mmgr;
		obj->error = mmgr->error;
		obj->list = HPDF_List_New(mmgr, HPDF_DEF_ITEMS_PER_BLOCK);
		if (!obj->list)
		{
			HPDF_FreeMem(mmgr, obj);
			obj = NULL;
		}
	}

	return obj;
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_BOX_NEW)
HPDF_Array HPDF_Box_Array_New(HPDF_MMgr mmgr, const HPDF_Box *box)
{
	HPDF_Array obj;
	HPDF_STATUS ret = HPDF_OK;

	obj = HPDF_Array_New(mmgr);
	if (!obj)
		return NULL;

	ret |= HPDF_Array_Add(obj, HPDF_Real_New(mmgr, box->left));
	ret |= HPDF_Array_Add(obj, HPDF_Real_New(mmgr, box->bottom));
	ret |= HPDF_Array_Add(obj, HPDF_Real_New(mmgr, box->right));
	ret |= HPDF_Array_Add(obj, HPDF_Real_New(mmgr, box->top));

	if (ret != HPDF_OK)
	{
		HPDF_Array_Free(obj);
		return NULL;
	}

	return obj;
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_FREE)
void HPDF_Array_Free(HPDF_Array array)
{
	if (!array)
		return;

	HPDF_Array_Clear(array);

	HPDF_List_Free(array->list);

	array->header.obj_class = 0;

	HPDF_FreeMem(array->mmgr, array);
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_WRITE)
HPDF_STATUS HPDF_Array_Write(HPDF_Array array, HPDF_Stream stream, HPDF_Encrypt e)
{
	HPDF_UINT i;
	HPDF_STATUS ret;

	ret = HPDF_Stream_WriteStr(stream, "[ ");
	if (ret != HPDF_OK)
		return ret;

	for (i = 0; i < array->list->count; i++)
	{
		void *element = HPDF_List_ItemAt(array->list, i);

		ret = HPDF_Obj_Write(element, stream, e);
		if (ret != HPDF_OK)
			return ret;

		ret = HPDF_Stream_WriteChar(stream, ' ');
		if (ret != HPDF_OK)
			return ret;
	}

	ret = HPDF_Stream_WriteChar(stream, ']');

	return ret;
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_ADDNUMBER)
HPDF_STATUS HPDF_Array_AddNumber(HPDF_Array array, HPDF_INT32 value)
{
	HPDF_Number n = HPDF_Number_New(array->mmgr, value);

	if (!n)
		return HPDF_Error_GetCode(array->error);
	else
		return HPDF_Array_Add(array, n);
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_ADDREAL)
HPDF_STATUS HPDF_Array_AddReal(HPDF_Array array, HPDF_REAL value)
{
	HPDF_Real r = HPDF_Real_New(array->mmgr, value);

	if (!r)
		return HPDF_Error_GetCode(array->error);
	else
		return HPDF_Array_Add(array, r);
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_ADDNULL)
HPDF_STATUS HPDF_Array_AddNull(HPDF_Array array)
{
	HPDF_Null n = HPDF_Null_New(array->mmgr);

	if (!n)
		return HPDF_Error_GetCode(array->error);
	else
		return HPDF_Array_Add(array, n);
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_ADDNAME)
HPDF_STATUS HPDF_Array_AddName(HPDF_Array array, const char *value)
{
	HPDF_Name n = HPDF_Name_New(array->mmgr, value);

	if (!n)
		return HPDF_Error_GetCode(array->error);
	else
		return HPDF_Array_Add(array, n);
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_ADD)
HPDF_STATUS HPDF_Array_Add(HPDF_Array array, void *obj)
{
	HPDF_Obj_Header *header;
	HPDF_STATUS ret;

	if (!obj)
	{
		if (HPDF_Error_GetCode(array->error) == HPDF_OK)
			return HPDF_SetError(array->error, HPDF_INVALID_OBJECT, 0);
		else
			return HPDF_INVALID_OBJECT;
	}

	header = (HPDF_Obj_Header *) obj;

	if (header->obj_id & HPDF_OTYPE_DIRECT)
		return HPDF_SetError(array->error, HPDF_INVALID_OBJECT, 0);

	if (array->list->count >= HPDF_LIMIT_MAX_ARRAY)
	{
		HPDF_Obj_Free(array->mmgr, obj);
		return HPDF_SetError(array->error, HPDF_ARRAY_COUNT_ERR, 0);
	}

	if (header->obj_id & HPDF_OTYPE_INDIRECT)
	{
		HPDF_Proxy proxy = HPDF_Proxy_New(array->mmgr, obj);

		if (!proxy)
		{
			HPDF_Obj_Free(array->mmgr, obj);
			return HPDF_Error_GetCode(array->error);
		}

		proxy->header.obj_id |= HPDF_OTYPE_DIRECT;
		obj = proxy;
	} else
	{
		header->obj_id |= HPDF_OTYPE_DIRECT;
	}

	ret = HPDF_List_Add(array->list, obj);
	if (ret != HPDF_OK)
		HPDF_Obj_Free(array->mmgr, obj);

	return ret;
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_ITEMS)
HPDF_UINT HPDF_Array_Items(HPDF_Array array)
{
	return array->list->count;
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_INSERT)
HPDF_STATUS HPDF_Array_Insert(HPDF_Array array, void *target, HPDF_Obj_Header *obj)
{
	HPDF_Obj_Header *header;
	HPDF_STATUS ret;
	HPDF_UINT i;

	if (!obj)
	{
		if (HPDF_Error_GetCode(array->error) == HPDF_OK)
			return HPDF_SetError(array->error, HPDF_INVALID_OBJECT, 0);
		else
			return HPDF_INVALID_OBJECT;
	}

	header = obj;

	if (header->obj_id & HPDF_OTYPE_DIRECT)
	{
		return HPDF_SetError(array->error, HPDF_INVALID_OBJECT, 0);
	}

	if (array->list->count >= HPDF_LIMIT_MAX_ARRAY)
	{
		HPDF_Obj_Free(array->mmgr, obj);

		return HPDF_SetError(array->error, HPDF_ARRAY_COUNT_ERR, 0);
	}

	if (header->obj_id & HPDF_OTYPE_INDIRECT)
	{
		HPDF_Proxy proxy = HPDF_Proxy_New(array->mmgr, obj);

		if (!proxy)
		{
			HPDF_Obj_Free(array->mmgr, obj);
			return HPDF_Error_GetCode(array->error);
		}

		obj = &proxy->header;
		obj->obj_id |= HPDF_OTYPE_DIRECT;
	} else
	{
		header->obj_id |= HPDF_OTYPE_DIRECT;
	}

	/* get the target-object from object-list
	 * consider that the pointer contained in list may be proxy-object.
	 */
	for (i = 0; i < array->list->count; i++)
	{
		void *ptr = HPDF_List_ItemAt(array->list, i);
		void *obj_ptr;

		header = obj;
		if (header->obj_class == HPDF_OCLASS_PROXY)
			obj_ptr = ((HPDF_Proxy) ptr)->obj;
		else
			obj_ptr = ptr;

		if (obj_ptr == target)
		{
			ret = HPDF_List_Insert(array->list, ptr, obj);
			if (ret != HPDF_OK)
				HPDF_Obj_Free(array->mmgr, obj);

			return ret;
		}
	}

	HPDF_Obj_Free(array->mmgr, obj);

	return HPDF_ARRAY_ITEM_NOT_FOUND;
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_GETITEM)
void *HPDF_Array_GetItem(HPDF_Array array, HPDF_UINT index, HPDF_UINT16 obj_class)
{
	void *obj;
	HPDF_Obj_Header *header;

	obj = HPDF_List_ItemAt(array->list, index);

	if (!obj)
	{
		HPDF_SetError(array->error, HPDF_ARRAY_ITEM_NOT_FOUND, 0);
		return NULL;
	}

	header = (HPDF_Obj_Header *) obj;

	if (header->obj_class == HPDF_OCLASS_PROXY)
	{
		obj = ((HPDF_Proxy) obj)->obj;
		header = (HPDF_Obj_Header *) obj;
	}

	if ((header->obj_class & HPDF_OCLASS_ANY) != obj_class)
	{
		HPDF_SetError(array->error, HPDF_ARRAY_ITEM_UNEXPECTED_TYPE, 0);

		return NULL;
	}

	return obj;
}
#endif


#if defined(ARRAY_ALL) || defined(ARRAY_CLEAR)
void HPDF_Array_Clear(HPDF_Array array)
{
	HPDF_UINT i;

	if (!array)
		return;

	for (i = 0; i < array->list->count; i++)
	{
		void *obj = HPDF_List_ItemAt(array->list, i);

		if (obj)
		{
			HPDF_Obj_Free(array->mmgr, obj);
		}
	}

	HPDF_List_Clear(array->list);
}
#endif
