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
#include "hpdf/image.h"
#include <string.h>

#ifdef HAVE_PNG
#include <png.h>
#include <string.h>

/* small hack: define png_int_t if *not* using our private version of pnglib */
#ifndef __gcc_double_t_defined
typedef int png_int_t;
#endif


static void PngReadFunc(png_structp png_ptr, png_bytep data, png_size_t length)
{
	HPDF_UINT len = (HPDF_UINT)length;
	HPDF_Stream stream = (HPDF_Stream) png_get_io_ptr(png_ptr);

	HPDF_Stream_Read(stream, data, &len);
}


/*---------------------------------------------------------------------------*/

static void PngErrorFunc(png_structp png_ptr, const char *msg)
{
	char error_number[16];
	int i;
	HPDF_STATUS detail_no;
	HPDF_Error error;

	/* pick out error-number from error message */
	memset(error_number, 0, 16);

	for (i = 0; i < 15; i++)
	{
		error_number[i] = *(msg + i);
		if (*(msg + i + 1) == ' ')
			break;
	}

	error = (HPDF_Error) png_get_error_ptr(png_ptr);
	detail_no = (HPDF_STATUS) HPDF_AToI(error_number);
	HPDF_SetError(error, HPDF_LIBPNG_ERROR, detail_no);
}


static HPDF_STATUS ReadPngData_Interlaced(HPDF_Dict image, png_structp png_ptr, png_infop info_ptr)
{
	png_uint_32 len = (png_uint_32)png_get_rowbytes(png_ptr, info_ptr);
	png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
	png_bytep *row_pointers = (png_bytep *) HPDF_GetMem(image->mmgr, height * sizeof(png_bytep));

	if (row_pointers)
	{
		png_uint_32 i;

		memset(row_pointers, 0, height * sizeof(png_bytep));
		for (i = 0; i < height; i++)
		{
			row_pointers[i] = (png_bytep) HPDF_GetMem(image->mmgr, len);

			if (image->error->error_no != HPDF_OK)
				break;
		}

		if (image->error->error_no == HPDF_OK)
		{
			png_read_image(png_ptr, row_pointers);
			if (image->error->error_no == HPDF_OK)
			{							/* add this line */
				for (i = 0; i < height; i++)
				{
					if (HPDF_Stream_Write(image->stream, row_pointers[i], len) != HPDF_OK)
						break;
				}
			}
		}

		/* clean up */
		for (i = 0; i < height; i++)
		{
			HPDF_FreeMem(image->mmgr, row_pointers[i]);
		}

		HPDF_FreeMem(image->mmgr, row_pointers);
	}

	return image->error->error_no;
}


static HPDF_STATUS ReadPngData(HPDF_Dict image, png_structp png_ptr, png_infop info_ptr)
{
	png_uint_32 len = (png_uint_32)png_get_rowbytes(png_ptr, info_ptr);
	png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
	png_bytep buf_ptr = (png_bytep) HPDF_GetMem(image->mmgr, len);

	if (buf_ptr)
	{
		png_uint_32 i;

		for (i = 0; i < height; i++)
		{
			png_read_rows(png_ptr, (png_byte **) &buf_ptr, NULL, 1);
			if (image->error->error_no != HPDF_OK)
				break;

			if (HPDF_Stream_Write(image->stream, buf_ptr, len) != HPDF_OK)
				break;
		}

		HPDF_FreeMem(image->mmgr, buf_ptr);
	}

	return image->error->error_no;
}


static HPDF_STATUS ReadTransparentPaletteData(
	HPDF_Dict image,
	png_structp png_ptr,
	png_infop info_ptr,
	png_bytep smask_data,
	png_bytep trans,
	int num_trans)
{
	HPDF_STATUS ret = HPDF_OK;
	png_uint_32 i, j;
	png_bytep *row_ptr;
	png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
	png_uint_32 width = png_get_image_width(png_ptr, info_ptr);

	row_ptr = (png_bytep *) HPDF_GetMem(image->mmgr, height * sizeof(png_bytep));
	if (!row_ptr)
	{
		return HPDF_FAILED_TO_ALLOC_MEM;
	} else
	{
		png_uint_32 len = (png_uint_32)png_get_rowbytes(png_ptr, info_ptr);

		for (i = 0; i < height; i++)
		{
			row_ptr[i] = (png_bytep) HPDF_GetMem(image->mmgr, len);
			if (row_ptr[i] == NULL)
			{
				while (i != 0)
				{
					--i;
					HPDF_FreeMem(image->mmgr, row_ptr[i]);
				}
				HPDF_FreeMem(image->mmgr, row_ptr);
				return HPDF_FAILED_TO_ALLOC_MEM;
			}
		}
	}

	png_read_image(png_ptr, row_ptr);
	if (image->error->error_no != HPDF_OK)
	{
		ret = HPDF_INVALID_PNG_IMAGE;
		goto Error;
	}

	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			smask_data[width * j + i] = (row_ptr[j][i] < num_trans) ? trans[row_ptr[j][i]] : 0xFF;
		}

		if (HPDF_Stream_Write(image->stream, row_ptr[j], width) != HPDF_OK)
		{
			ret = HPDF_FILE_IO_ERROR;
			goto Error;
		}
	}

  Error:
	for (i = 0; i < height; i++)
	{
		HPDF_FreeMem(image->mmgr, row_ptr[i]);
	}

	HPDF_FreeMem(image->mmgr, row_ptr);
	return ret;
}


static HPDF_STATUS ReadTransparentPngData(HPDF_Dict image, png_structp png_ptr, png_infop info_ptr, png_bytep smask_data)
{
	HPDF_STATUS ret = HPDF_OK;
	HPDF_INT row_len;
	png_uint_32 i, j;
	png_bytep *row_ptr, row;
	png_byte color_type;
	png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
	png_uint_32 width = png_get_image_width(png_ptr, info_ptr);

	color_type = png_get_color_type(png_ptr, info_ptr);

	if (!(color_type & PNG_COLOR_MASK_ALPHA))
	{
		return HPDF_INVALID_PNG_IMAGE;
	}

	row_ptr = (png_bytep *) HPDF_GetMem(image->mmgr, height * sizeof(png_bytep));
	if (!row_ptr)
	{
		return HPDF_FAILED_TO_ALLOC_MEM;
	} else
	{
		png_uint_32 len = (png_uint_32)png_get_rowbytes(png_ptr, info_ptr);

		for (i = 0; i < height; i++)
		{
			row_ptr[i] = (png_bytep) HPDF_GetMem(image->mmgr, len);
			if (!row_ptr[i])
			{
				while (i != 0)
				{
					--i;
					HPDF_FreeMem(image->mmgr, row_ptr[i]);
				}
				HPDF_FreeMem(image->mmgr, row_ptr);
				return HPDF_FAILED_TO_ALLOC_MEM;
			}
		}
	}

	png_read_image(png_ptr, row_ptr);
	if (image->error->error_no != HPDF_OK)
	{
		ret = HPDF_INVALID_PNG_IMAGE;
		goto Error;
	}

	switch (color_type)
	{
	case PNG_COLOR_TYPE_RGB_ALPHA:
		row_len = 3 * width * sizeof(png_byte);
		for (j = 0; j < height; j++)
		{
			row = row_ptr[j];
			for (i = 0; i < width; i++)
			{
				memmove(row + (3 * i), row + (4 * i), 3);
				smask_data[width * j + i] = row[4 * i + 3];
			}

			if (HPDF_Stream_Write(image->stream, row, row_len) != HPDF_OK)
			{
				ret = HPDF_FILE_IO_ERROR;
				goto Error;
			}
		}
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		row_len = width * sizeof(png_byte);
		for (j = 0; j < height; j++)
		{
			row = row_ptr[j];
			for (i = 0; i < width; i++)
			{
				row[i] = row[2 * i];
				smask_data[width * j + i] = row[2 * i + 1];
			}

			if (HPDF_Stream_Write(image->stream, row, row_len) != HPDF_OK)
			{
				ret = HPDF_FILE_IO_ERROR;
				goto Error;
			}
		}
		break;
	default:
		ret = HPDF_INVALID_PNG_IMAGE;
		goto Error;
	}

  Error:
	for (i = 0; i < (HPDF_UINT) height; i++)
	{
		HPDF_FreeMem(image->mmgr, row_ptr[i]);
	}

	HPDF_FreeMem(image->mmgr, row_ptr);
	return ret;
}


static HPDF_STATUS CreatePallet(HPDF_Dict image, png_structp png_ptr, png_infop info_ptr)
{
	png_int_t num_pl = 0;
	png_color *src_pl = NULL;
	HPDF_BYTE *ppallet;
	HPDF_BYTE *p;
	HPDF_INT i;
	HPDF_Array array;

	/* png_get_PLTE does not call PngErrorFunc even if it failed.
	 * so we call HPDF_Set_Error to set error-code.
	 */
	if (png_get_PLTE(png_ptr, info_ptr, (png_color **) &src_pl, &num_pl) != PNG_INFO_PLTE)
		return HPDF_SetError(image->error, HPDF_LIBPNG_ERROR, HPDF_CANNOT_GET_PALETTE);


	/* make a pallet array for indexed image. */
	ppallet = (HPDF_BYTE *) HPDF_GetMem(image->mmgr, num_pl * 3);
	if (!ppallet)
		return image->error->error_no;

	p = ppallet;
	for (i = 0; i < num_pl; i++, src_pl++)
	{
		*p++ = src_pl->red;
		*p++ = src_pl->green;
		*p++ = src_pl->blue;
	}

	array = HPDF_Array_New(image->mmgr);
	if (array)
	{
		HPDF_Binary b;

		HPDF_Dict_Add(image, "ColorSpace", array);

		HPDF_Array_AddName(array, HPDF_COLORSPACE_NAMES[HPDF_CS_INDEXED]);
		HPDF_Array_AddName(array, HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_RGB]);
		HPDF_Array_AddNumber(array, num_pl - 1);

		b = HPDF_Binary_New(image->mmgr, ppallet, num_pl * 3);
		if (b)
			HPDF_Array_Add(array, b);
	}

	HPDF_FreeMem(image->mmgr, ppallet);

	return image->error->error_no;
}

#define HPDF_PNG_BYTES_TO_CHECK 8


static HPDF_STATUS PngBeforeWrite(HPDF_Dict obj);
static HPDF_STATUS PngAfterWrite(HPDF_Dict obj);


static HPDF_STATUS LoadPngData(HPDF_Dict image, HPDF_Xref xref, HPDF_Stream png_data, HPDF_BOOL delayed_loading)
{
	HPDF_STATUS ret = HPDF_OK;
	png_uint_32 width, height;
	int bit_depth, color_type;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;

	/* create read_struct. */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, image->error, PngErrorFunc, PngErrorFunc);

	if (png_ptr == NULL)
	{
		return HPDF_SetError(image->error, HPDF_FAILED_TO_ALLOC_MEM, 0);
	}

	/* create info-struct */
	info_ptr = png_create_info_struct(png_ptr);

	if (info_ptr == NULL)
	{
		HPDF_SetError(image->error, HPDF_FAILED_TO_ALLOC_MEM, 0);
		goto Exit;
	}

	png_set_sig_bytes(png_ptr, HPDF_PNG_BYTES_TO_CHECK);
	png_set_read_fn(png_ptr, (void *) png_data, PngReadFunc);

	/* reading info structure. */
	png_read_info(png_ptr, info_ptr);
	if (image->error->error_no != HPDF_OK)
	{
		goto Exit;
	}

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	/* 16bit images are not supported. */
	if (bit_depth == 16)
	{
		png_set_strip_16(png_ptr);
	}

	png_read_update_info(png_ptr, info_ptr);
	if (image->error->error_no != HPDF_OK)
	{
		goto Exit;
	}

	/* check palette-based images for transparent areas and load them immediately if found */
	if (xref && (PNG_COLOR_TYPE_PALETTE & color_type))
	{
		png_bytep trans;
		int num_trans;
		HPDF_Dict smask;
		png_bytep smask_data;

		if (!png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) ||
			!png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL))
		{
			goto no_transparent_color_in_palette;
		}

		smask = HPDF_DictStream_New(image->mmgr, xref);
		if (!smask)
		{
			ret = HPDF_FAILED_TO_ALLOC_MEM;
			goto Exit;
		}

		smask->header.obj_class |= HPDF_OSUBCLASS_XOBJECT;
		ret = HPDF_Dict_AddName(smask, "Type", "XObject");
		ret += HPDF_Dict_AddName(smask, "Subtype", "Image");
		ret += HPDF_Dict_AddNumber(smask, "Width", width);
		ret += HPDF_Dict_AddNumber(smask, "Height", height);
		ret += HPDF_Dict_AddName(smask, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_GRAY]);
		ret += HPDF_Dict_AddNumber(smask, "BitsPerComponent", bit_depth);

		if (ret != HPDF_OK)
		{
			HPDF_Dict_Free(smask);
			ret = HPDF_INVALID_PNG_IMAGE;
			goto Exit;
		}

		smask_data = (png_bytep) HPDF_GetMem(image->mmgr, width * height);
		if (!smask_data)
		{
			HPDF_Dict_Free(smask);
			ret = HPDF_FAILED_TO_ALLOC_MEM;
			goto Exit;
		}

		if (ReadTransparentPaletteData(image, png_ptr, info_ptr, smask_data, trans, num_trans) != HPDF_OK)
		{
			HPDF_FreeMem(image->mmgr, smask_data);
			HPDF_Dict_Free(smask);
			ret = HPDF_INVALID_PNG_IMAGE;
			goto Exit;
		}

		if (HPDF_Stream_Write(smask->stream, smask_data, width * height) != HPDF_OK)
		{
			HPDF_FreeMem(image->mmgr, smask_data);
			HPDF_Dict_Free(smask);
			ret = HPDF_FILE_IO_ERROR;
			goto Exit;
		}
		HPDF_FreeMem(image->mmgr, smask_data);


		ret += CreatePallet(image, png_ptr, info_ptr);
		ret += HPDF_Dict_AddNumber(image, "Width", width);
		ret += HPDF_Dict_AddNumber(image, "Height", height);
		ret += HPDF_Dict_AddNumber(image, "BitsPerComponent", bit_depth);
		ret += HPDF_Dict_Add(image, "SMask", smask);

		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return HPDF_OK;
	}

  no_transparent_color_in_palette:

	/* read images with alpha channel right away
	   we have to do this because image transparent mask must be added to the Xref */
	if (xref && (PNG_COLOR_MASK_ALPHA & color_type))
	{
		HPDF_Dict smask;
		png_bytep smask_data;

		smask = HPDF_DictStream_New(image->mmgr, xref);
		if (!smask)
		{
			ret = HPDF_FAILED_TO_ALLOC_MEM;
			goto Exit;
		}

		smask->header.obj_class |= HPDF_OSUBCLASS_XOBJECT;
		ret = HPDF_Dict_AddName(smask, "Type", "XObject");
		ret += HPDF_Dict_AddName(smask, "Subtype", "Image");
		ret += HPDF_Dict_AddNumber(smask, "Width", width);
		ret += HPDF_Dict_AddNumber(smask, "Height", height);
		ret += HPDF_Dict_AddName(smask, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_GRAY]);
		ret += HPDF_Dict_AddNumber(smask, "BitsPerComponent", bit_depth);

		if (ret != HPDF_OK)
		{
			HPDF_Dict_Free(smask);
			ret = HPDF_INVALID_PNG_IMAGE;
			goto Exit;
		}

		smask_data = (png_bytep) HPDF_GetMem(image->mmgr, width * height);
		if (!smask_data)
		{
			HPDF_Dict_Free(smask);
			ret = HPDF_FAILED_TO_ALLOC_MEM;
			goto Exit;
		}

		if (ReadTransparentPngData(image, png_ptr, info_ptr, smask_data) != HPDF_OK)
		{
			HPDF_FreeMem(image->mmgr, smask_data);
			HPDF_Dict_Free(smask);
			ret = HPDF_INVALID_PNG_IMAGE;
			goto Exit;
		}

		if (HPDF_Stream_Write(smask->stream, smask_data, width * height) != HPDF_OK)
		{
			HPDF_FreeMem(image->mmgr, smask_data);
			HPDF_Dict_Free(smask);
			ret = HPDF_FILE_IO_ERROR;
			goto Exit;
		}
		HPDF_FreeMem(image->mmgr, smask_data);

		if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		{
			ret += HPDF_Dict_AddName(image, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_GRAY]);
		} else
		{
			ret += HPDF_Dict_AddName(image, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_RGB]);
		}
		ret += HPDF_Dict_AddNumber(image, "Width", width);
		ret += HPDF_Dict_AddNumber(image, "Height", height);
		ret += HPDF_Dict_AddNumber(image, "BitsPerComponent", bit_depth);
		ret += HPDF_Dict_Add(image, "SMask", smask);

		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return HPDF_OK;
	}

	/* if the image has color palette, copy the pallet of the image to
	 * create color map.
	 */
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		ret = CreatePallet(image, png_ptr, info_ptr);
	else if (color_type == PNG_COLOR_TYPE_GRAY)
		ret = HPDF_Dict_AddName(image, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_GRAY]);
	else
		ret = HPDF_Dict_AddName(image, "ColorSpace", HPDF_COLORSPACE_NAMES[HPDF_CS_DEVICE_RGB]);

	if (ret != HPDF_OK)
		goto Exit;

	/* read image-data
	 * if the image is interlaced, read whole image at once.
	 * if delayed_loading is HPDF_TRUE, the data does not load this phase.
	 */
	if (delayed_loading)
	{
		image->before_write_fn = PngBeforeWrite;
		image->after_write_fn = PngAfterWrite;
	} else
	{
		if (png_get_interlace_type(png_ptr, info_ptr) != PNG_INTERLACE_NONE)
			ret = ReadPngData_Interlaced(image, png_ptr, info_ptr);
		else
			ret = ReadPngData(image, png_ptr, info_ptr);

		if (ret != HPDF_OK)
			goto Exit;
	}

	/* setting the info of the image. */
	if (HPDF_Dict_AddNumber(image, "Width", width) != HPDF_OK)
		goto Exit;

	if (HPDF_Dict_AddNumber(image, "Height", height) != HPDF_OK)
		goto Exit;

	if (HPDF_Dict_AddNumber(image, "BitsPerComponent", bit_depth) != HPDF_OK)
		goto Exit;

	/* clean up */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return HPDF_OK;

  Exit:
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	if (ret != HPDF_OK)
	{
		return ret;
	}
	return image->error->error_no;
}


static HPDF_STATUS PngBeforeWrite(HPDF_Dict obj)
{
	HPDF_STATUS ret;
	png_byte header[HPDF_PNG_BYTES_TO_CHECK];
	HPDF_UINT len = HPDF_PNG_BYTES_TO_CHECK;
	HPDF_Stream png_data;
	HPDF_String s;

	HPDF_MemStream_FreeData(obj->stream);

	s = (HPDF_String) HPDF_Dict_GetItem(obj, "_FILE_NAME", HPDF_OCLASS_STRING);
	if (!s)
		return HPDF_SetError(obj->error, HPDF_MISSING_FILE_NAME_ENTRY, 0);

	png_data = HPDF_FileReader_New(obj->mmgr, (const char *) (s->value));
	if (!HPDF_Stream_Validate(png_data))
		return obj->error->error_no;

	memset(header, 0x00, HPDF_PNG_BYTES_TO_CHECK);
	ret = HPDF_Stream_Read(png_data, header, &len);
	if (ret != HPDF_OK || png_sig_cmp(header, (png_size_t) 0, HPDF_PNG_BYTES_TO_CHECK))
	{
		HPDF_Stream_Free(png_data);
		return HPDF_SetError(obj->error, HPDF_INVALID_PNG_IMAGE, 0);
	}

	if ((ret = LoadPngData(obj, NULL, png_data, HPDF_FALSE)) != HPDF_OK)
	{
		HPDF_Stream_Free(png_data);
		return ret;
	}

	HPDF_Stream_Free(png_data);

	return HPDF_OK;
}


static HPDF_STATUS PngAfterWrite(HPDF_Dict obj)
{
	HPDF_MemStream_FreeData(obj->stream);

	return HPDF_OK;
}


HPDF_Image HPDF_Image_LoadPngImage(HPDF_MMgr mmgr, HPDF_Stream png_data, HPDF_Xref xref, HPDF_BOOL delayed_loading)
{
	HPDF_STATUS ret;
	HPDF_Dict image;
	png_byte header[HPDF_PNG_BYTES_TO_CHECK];
	HPDF_UINT len = HPDF_PNG_BYTES_TO_CHECK;

	memset(header, 0x00, HPDF_PNG_BYTES_TO_CHECK);
	ret = HPDF_Stream_Read(png_data, header, &len);
	if (ret != HPDF_OK || png_sig_cmp(header, (png_size_t) 0, HPDF_PNG_BYTES_TO_CHECK))
	{
		HPDF_SetError(mmgr->error, HPDF_INVALID_PNG_IMAGE, 0);
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

	if (LoadPngData(image, xref, png_data, delayed_loading) != HPDF_OK)
		return NULL;

	return image;
}
#endif /* HAVE_PNG */
