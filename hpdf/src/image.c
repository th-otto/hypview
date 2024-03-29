/*
 * << Haru Free PDF Library >> -- hpdf_image.c
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
#include <string.h>


/*---------------------------------------------------------------------------*/

static HPDF_STATUS LoadJpegHeader(HPDF_Image image, HPDF_Stream stream)
{
	HPDF_UINT16 tag;
	HPDF_UINT16 height;
	HPDF_UINT16 width;
	HPDF_BYTE precision;
	HPDF_BYTE num_components;
	const char *color_space_name;
	HPDF_UINT len;
	HPDF_STATUS ret;
	HPDF_Array array;

	len = 2;
	if (HPDF_Stream_Read(stream, &tag, &len) != HPDF_OK)
		return HPDF_Error_GetCode(stream->error);

	HPDF_UInt16Swap(&tag);
	if (tag != 0xFFD8)
		return HPDF_INVALID_JPEG_DATA;

	/* find SOF record */
	for (;;)
	{
		HPDF_UINT16 size;

		len = 2;
		if (HPDF_Stream_Read(stream, &tag, &len) != HPDF_OK)
			return HPDF_Error_GetCode(stream->error);

		HPDF_UInt16Swap(&tag);

		len = 2;
		if (HPDF_Stream_Read(stream, &size, &len) != HPDF_OK)
			return HPDF_Error_GetCode(stream->error);

		HPDF_UInt16Swap(&size);

		if (tag == 0xFFC0 || tag == 0xFFC1 || tag == 0xFFC2 || tag == 0xFFC9)
		{

			len = 1;
			if (HPDF_Stream_Read(stream, &precision, &len) != HPDF_OK)
				return HPDF_Error_GetCode(stream->error);

			len = 2;
			if (HPDF_Stream_Read(stream,  &height, &len) != HPDF_OK)
				return HPDF_Error_GetCode(stream->error);

			HPDF_UInt16Swap(&height);

			len = 2;
			if (HPDF_Stream_Read(stream, &width, &len) != HPDF_OK)
				return HPDF_Error_GetCode(stream->error);

			HPDF_UInt16Swap(&width);

			len = 1;
			if (HPDF_Stream_Read(stream, &num_components, &len) != HPDF_OK)
				return HPDF_Error_GetCode(stream->error);

			break;
		} else if ((tag | 0x00FFU) != 0xFFFFU)
			/* lost marker */
			return HPDF_SetError(image->error, HPDF_UNSUPPORTED_JPEG_FORMAT, 0);

		if (HPDF_Stream_Seek(stream, size - 2, HPDF_SEEK_CUR) != HPDF_OK)
			return HPDF_Error_GetCode(stream->error);
	}

	if (HPDF_Dict_AddNumber(image, "Height", height) != HPDF_OK)
		return HPDF_Error_GetCode(stream->error);

	if (HPDF_Dict_AddNumber(image, "Width", width) != HPDF_OK)
		return HPDF_Error_GetCode(stream->error);

	/* classification of RGB and CMYK is less than perfect
	 * YCbCr and YCCK are classified into RGB or CMYK.
	 *
	 * It is necessary to read APP14 data to distinguish colorspace perfectly.

	 */
	switch (num_components)
	{
	case 1:
		color_space_name = HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_GRAY];
		break;
	case 3:
		color_space_name = HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_RGB];
		break;
	case 4:
		array = HPDF_Array_New(image->mmgr);
		if (!array)
			return HPDF_Error_GetCode(stream->error);

		ret = HPDF_Dict_Add(image, "Decode", array);
		if (ret != HPDF_OK)
			return HPDF_Error_GetCode(stream->error);

		ret += HPDF_Array_Add(array, HPDF_Number_New(image->mmgr, 1));
		ret += HPDF_Array_Add(array, HPDF_Number_New(image->mmgr, 0));
		ret += HPDF_Array_Add(array, HPDF_Number_New(image->mmgr, 1));
		ret += HPDF_Array_Add(array, HPDF_Number_New(image->mmgr, 0));
		ret += HPDF_Array_Add(array, HPDF_Number_New(image->mmgr, 1));
		ret += HPDF_Array_Add(array, HPDF_Number_New(image->mmgr, 0));
		ret += HPDF_Array_Add(array, HPDF_Number_New(image->mmgr, 1));
		ret += HPDF_Array_Add(array, HPDF_Number_New(image->mmgr, 0));

		if (ret != HPDF_OK)
			return HPDF_Error_GetCode(stream->error);

		color_space_name = HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_CMYK];

		break;
	default:
		return HPDF_SetError(image->error, HPDF_UNSUPPORTED_JPEG_FORMAT, 0);
	}

	if (HPDF_Dict_Add(image, "ColorSpace", HPDF_Name_New(image->mmgr, color_space_name)) != HPDF_OK)
		return HPDF_Error_GetCode(stream->error);

	if (HPDF_Dict_Add(image, "BitsPerComponent", HPDF_Number_New(image->mmgr, precision)) != HPDF_OK)
		return HPDF_Error_GetCode(stream->error);

	return HPDF_OK;
}


HPDF_Image HPDF_Image_LoadJpegImage(HPDF_MMgr mmgr, HPDF_Stream jpeg_data, HPDF_Xref xref)
{
	HPDF_Dict image;
	HPDF_STATUS ret = HPDF_OK;

	image = HPDF_DictStream_New(mmgr, xref);
	if (!image)
		return NULL;

	image->header.obj_class |= HPDF_OSUBCLASS_XOBJECT;

	/* add required elements */
	image->filter = HPDF_STREAM_FILTER_DCT_DECODE;
	ret += HPDF_Dict_AddName(image, "Type", "XObject");
	ret += HPDF_Dict_AddName(image, "Subtype", "Image");
	if (ret != HPDF_OK)
		return NULL;

	if (LoadJpegHeader(image, jpeg_data) != HPDF_OK)
		return NULL;

	if (HPDF_Stream_Seek(jpeg_data, 0, HPDF_SEEK_SET) != HPDF_OK)
		return NULL;

	for (;;)
	{
		HPDF_BYTE buf[HPDF_STREAM_BUF_SIZ];
		HPDF_UINT len = HPDF_STREAM_BUF_SIZ;
		HPDF_STATUS ret = HPDF_Stream_Read(jpeg_data, buf, &len);

		if (ret != HPDF_OK)
		{
			if (ret == HPDF_STREAM_EOF)
			{
				if (len > 0)
				{
					ret = HPDF_Stream_Write(image->stream, buf, len);
					if (ret != HPDF_OK)
						return NULL;
				}
				break;
			} else
			{
				return NULL;
			}
		}

		if (HPDF_Stream_Write(image->stream, buf, len) != HPDF_OK)
			return NULL;
	}

	return image;
}


HPDF_Image HPDF_Image_LoadJpegImageFromMem(HPDF_MMgr mmgr, const HPDF_BYTE *buf, HPDF_UINT size, HPDF_Xref xref)
{
	HPDF_Stream jpeg_data;
	HPDF_Image image;

	jpeg_data = HPDF_MemStream_New(mmgr, size);
	if (!HPDF_Stream_Validate(jpeg_data))
	{
		HPDF_RaiseError(mmgr->error, HPDF_INVALID_STREAM, 0);
		return NULL;
	}

	if (HPDF_Stream_Write(jpeg_data, buf, size) != HPDF_OK)
	{
		HPDF_Stream_Free(jpeg_data);
		return NULL;
	}

	image = HPDF_Image_LoadJpegImage(mmgr, jpeg_data, xref);

	/* destroy file stream */
	HPDF_Stream_Free(jpeg_data);

	return image;
}


HPDF_Image HPDF_Image_LoadRawImage(
	HPDF_MMgr mmgr,
	HPDF_Stream raw_data,
	HPDF_Xref xref,
	HPDF_UINT width,
	HPDF_UINT height,
	HPDF_ColorSpace color_space)
{
	HPDF_Dict image;
	HPDF_STATUS ret = HPDF_OK;
	HPDF_UINT size;

	if (color_space != HPDF_CS_DEVICE_GRAY && color_space != HPDF_CS_DEVICE_RGB && color_space != HPDF_CS_DEVICE_CMYK)
	{
		HPDF_SetError(mmgr->error, HPDF_INVALID_COLOR_SPACE, 0);
		return NULL;
	}

	image = HPDF_DictStream_New(mmgr, xref);
	if (!image)
		return NULL;

	image->header.obj_class |= HPDF_OSUBCLASS_XOBJECT;
	ret += HPDF_Dict_AddName(image, "Type", "XObject");
	ret += HPDF_Dict_AddName(image, "Subtype", "Image");
	if (ret != HPDF_OK)
		return NULL;

	if (color_space == HPDF_CS_DEVICE_GRAY)
	{
		size = width * height;
		ret = HPDF_Dict_AddName(image, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_GRAY]);
	} else if (color_space == HPDF_CS_DEVICE_CMYK)
	{
		size = width * height * 4;
		ret = HPDF_Dict_AddName(image, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_CMYK]);
	} else
	{
		size = width * height * 3;
		ret = HPDF_Dict_AddName(image, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_RGB]);
	}

	if (ret != HPDF_OK)
		return NULL;

	if (HPDF_Dict_AddNumber(image, "Width", width) != HPDF_OK)
		return NULL;

	if (HPDF_Dict_AddNumber(image, "Height", height) != HPDF_OK)
		return NULL;

	if (HPDF_Dict_AddNumber(image, "BitsPerComponent", 8) != HPDF_OK)
		return NULL;

	if (HPDF_Stream_WriteToStream(raw_data, image->stream, 0, NULL) != HPDF_OK)
		return NULL;

	if (image->stream->size != size)
	{
		HPDF_SetError(image->error, HPDF_INVALID_IMAGE, 0);
		return NULL;
	}

	return image;
}


HPDF_Image HPDF_Image_LoadRawImageFromMem(
	HPDF_MMgr mmgr,
	const HPDF_BYTE *buf,
	HPDF_Xref xref,
	HPDF_UINT width,
	HPDF_UINT height,
	HPDF_ColorSpace color_space,
	HPDF_UINT bits_per_component)
{
	HPDF_Image image;
	HPDF_STATUS ret = HPDF_OK;
	HPDF_UINT size = 0;

	if (color_space != HPDF_CS_DEVICE_GRAY && color_space != HPDF_CS_DEVICE_RGB && color_space != HPDF_CS_DEVICE_CMYK)
	{
		HPDF_SetError(mmgr->error, HPDF_INVALID_COLOR_SPACE, 0);
		return NULL;
	}

	if (bits_per_component != 1 && bits_per_component != 2 && bits_per_component != 4 && bits_per_component != 8)
	{
		HPDF_SetError(mmgr->error, HPDF_INVALID_IMAGE, 0);
		return NULL;
	}

	image = HPDF_DictStream_New(mmgr, xref);
	if (!image)
		return NULL;

	image->header.obj_class |= HPDF_OSUBCLASS_XOBJECT;
	ret += HPDF_Dict_AddName(image, "Type", "XObject");
	ret += HPDF_Dict_AddName(image, "Subtype", "Image");
	if (ret != HPDF_OK)
		return NULL;

	switch (color_space)
	{
	case HPDF_CS_DEVICE_GRAY:
		size = ((width * bits_per_component + 7) / 8) * height;
		ret = HPDF_Dict_AddName(image, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_GRAY]);
		break;
	case HPDF_CS_DEVICE_RGB:
		size = ((width * bits_per_component + 7) / 8) * height;
		size *= 3;
		ret = HPDF_Dict_AddName(image, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_RGB]);
		break;
	case HPDF_CS_DEVICE_CMYK:
		size = ((width * bits_per_component + 7) / 8) * height;
		size *= 4;
		ret = HPDF_Dict_AddName(image, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_CMYK]);
		break;
	default:
		break;
	}

	if (ret != HPDF_OK)
		return NULL;

	if (HPDF_Dict_AddNumber(image, "Width", width) != HPDF_OK)
		return NULL;

	if (HPDF_Dict_AddNumber(image, "Height", height) != HPDF_OK)
		return NULL;

	if (HPDF_Dict_AddNumber(image, "BitsPerComponent", bits_per_component) != HPDF_OK)
		return NULL;

	if (HPDF_Stream_Write(image->stream, buf, size) != HPDF_OK)
		return NULL;

	return image;
}


HPDF_BOOL HPDF_Image_Validate(HPDF_Image image)
{
	HPDF_Name subtype;

	if (!image)
		return HPDF_FALSE;

	if (image->header.obj_class != (HPDF_OSUBCLASS_XOBJECT | HPDF_OCLASS_DICT))
	{
		HPDF_RaiseError(image->error, HPDF_INVALID_IMAGE, 0);
		return HPDF_FALSE;
	}

	subtype = (HPDF_Name) HPDF_Dict_GetItem(image, "Subtype", HPDF_OCLASS_NAME);
	if (!subtype || strcmp(subtype->value, "Image") != 0)
	{
		HPDF_RaiseError(image->error, HPDF_INVALID_IMAGE, 0);
		return HPDF_FALSE;
	}

	return HPDF_TRUE;
}


HPDF_STATUS HPDF_Image_GetSize(HPDF_Image image, HPDF_UINT *x, HPDF_UINT *y)
{
	HPDF_Number width;
	HPDF_Number height;

	*x = 0;
	*y = 0;

	if (!HPDF_Image_Validate(image))
		return HPDF_INVALID_IMAGE;

	width = (HPDF_Number) HPDF_Dict_GetItem(image, "Width", HPDF_OCLASS_NUMBER);
	height = (HPDF_Number) HPDF_Dict_GetItem(image, "Height", HPDF_OCLASS_NUMBER);

	if (width && height)
	{
		*x = width->value;
		*y = height->value;
	}

	return HPDF_OK;
}


HPDF_UINT HPDF_Image_GetBitsPerComponent(HPDF_Image image)
{
	HPDF_Number n;

	if (!HPDF_Image_Validate(image))
		return 0;

	n = (HPDF_Number) HPDF_Dict_GetItem(image, "BitsPerComponent", HPDF_OCLASS_NUMBER);

	if (!n)
		return 0;

	return n->value;
}


const char *HPDF_Image_GetColorSpace(HPDF_Image image)
{
	HPDF_Name n;

	n = (HPDF_Name) HPDF_Dict_GetItem(image, "ColorSpace", HPDF_OCLASS_NAME);

	if (!n)
	{
		HPDF_Array a;

		HPDF_Error_Reset(image->error);

		a = (HPDF_Array) HPDF_Dict_GetItem(image, "ColorSpace", HPDF_OCLASS_ARRAY);

		if (a)
		{
			n = (HPDF_Name) HPDF_Array_GetItem(a, 0, HPDF_OCLASS_NAME);
		}
	}

	if (!n)
	{
		HPDF_CheckError(image->error);
		return NULL;
	}

	return n->value;
}


HPDF_UINT HPDF_Image_GetWidth(HPDF_Image image)
{
	HPDF_UINT width, height;
	HPDF_Image_GetSize(image, &width, &height);
	return width;
}


HPDF_UINT HPDF_Image_GetHeight(HPDF_Image image)
{
	HPDF_UINT width, height;
	HPDF_Image_GetSize(image, &width, &height);
	return height;
}


HPDF_STATUS HPDF_Image_SetMask(HPDF_Image image, HPDF_BOOL mask)
{
	HPDF_Boolean image_mask;

	if (!HPDF_Image_Validate(image))
		return HPDF_INVALID_IMAGE;

	if (mask && HPDF_Image_GetBitsPerComponent(image) != 1)
		return HPDF_SetError(image->error, HPDF_INVALID_BIT_PER_COMPONENT, 0);

	image_mask = (HPDF_Boolean) HPDF_Dict_GetItem(image, "ImageMask", HPDF_OCLASS_BOOLEAN);
	if (!image_mask)
	{
		HPDF_STATUS ret;

		image_mask = HPDF_Boolean_New(image->mmgr, HPDF_FALSE);

		if ((ret = HPDF_Dict_Add(image, "ImageMask", image_mask)) != HPDF_OK)
			return ret;
	}

	image_mask->value = mask;
	return HPDF_OK;
}


HPDF_STATUS HPDF_Image_SetMaskImage(HPDF_Image image, HPDF_Image mask_image)
{
	if (!HPDF_Image_Validate(image))
		return HPDF_INVALID_IMAGE;

	if (!HPDF_Image_Validate(mask_image))
		return HPDF_INVALID_IMAGE;

	if (HPDF_Image_SetMask(mask_image, HPDF_TRUE) != HPDF_OK)
		return HPDF_CheckError(image->error);

	return HPDF_Dict_Add(image, "Mask", mask_image);
}


HPDF_STATUS HPDF_Image_SetColorMask(
	HPDF_Image image,
	HPDF_UINT rmin,
	HPDF_UINT rmax,
	HPDF_UINT gmin,
	HPDF_UINT gmax,
	HPDF_UINT bmin,
	HPDF_UINT bmax)
{
	HPDF_Array array;
	const char *name;
	HPDF_STATUS ret = HPDF_OK;

	if (!HPDF_Image_Validate(image))
		return HPDF_INVALID_IMAGE;

	if (HPDF_Dict_GetItem(image, "ImageMask", HPDF_OCLASS_BOOLEAN))
		return HPDF_RaiseError(image->error, HPDF_INVALID_OPERATION, 0);

	if (HPDF_Image_GetBitsPerComponent(image) != 8)
		return HPDF_RaiseError(image->error, HPDF_INVALID_BIT_PER_COMPONENT, 0);

	name = HPDF_Image_GetColorSpace(image);
	if (!name || strcmp(HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_RGB], name) != 0)
		return HPDF_RaiseError(image->error, HPDF_INVALID_COLOR_SPACE, 0);

	/* Each integer must be in the range 0 to 2^BitsPerComponent - 1 */
	if (rmax > 255 || gmax > 255 || bmax > 255)
		return HPDF_RaiseError(image->error, HPDF_INVALID_PARAMETER, 0);

	array = HPDF_Array_New(image->mmgr);
	if (!array)
		return HPDF_CheckError(image->error);

	ret += HPDF_Dict_Add(image, "Mask", array);
	ret += HPDF_Array_AddNumber(array, rmin);
	ret += HPDF_Array_AddNumber(array, rmax);
	ret += HPDF_Array_AddNumber(array, gmin);
	ret += HPDF_Array_AddNumber(array, gmax);
	ret += HPDF_Array_AddNumber(array, bmin);
	ret += HPDF_Array_AddNumber(array, bmax);

	if (ret != HPDF_OK)
		return HPDF_CheckError(image->error);

	return HPDF_OK;
}


HPDF_STATUS HPDF_Image_AddSMask(HPDF_Image image, HPDF_Image smask)
{

	const char *name;

	if (!HPDF_Image_Validate(image))
		return HPDF_INVALID_IMAGE;
	if (!HPDF_Image_Validate(smask))
		return HPDF_INVALID_IMAGE;

	if (HPDF_Dict_GetItem(image, "SMask", HPDF_OCLASS_BOOLEAN))
		return HPDF_RaiseError(image->error, HPDF_INVALID_OPERATION, 0);

	name = HPDF_Image_GetColorSpace(smask);
	if (!name || strcmp(HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_GRAY], name) != 0)
		return HPDF_RaiseError(smask->error, HPDF_INVALID_COLOR_SPACE, 0);

	return HPDF_Dict_Add(image, "SMask", smask);
}


HPDF_STATUS HPDF_Image_SetColorSpace(HPDF_Image image, HPDF_Array colorspace)
{
	if (!HPDF_Image_Validate(image))
		return HPDF_INVALID_IMAGE;

	return HPDF_Dict_Add(image, "ColorSpace", colorspace);
}


HPDF_STATUS HPDF_Image_SetRenderingIntent(HPDF_Image image, const char *intent)
{
	if (!HPDF_Image_Validate(image))
		return HPDF_INVALID_IMAGE;

	return HPDF_Dict_AddName(image, "Intent", intent);
}
