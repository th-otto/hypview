/* GdkPixbuf library - GdkPixdata - functions for inlined pixbuf handling
 * Copyright (C) 1999, 2001 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "hypdefs.h"
#include "pixdata.h"
#include <stdlib.h>
#include <string.h>

#undef g_set_error_literal
#define g_set_error_literal(a, b, c, d)

/**
 * SECTION:inline
 * @Short_description: Functions for inlined pixbuf handling.
 * @Title: Inline data
 * 
 * Using #GdkPixdata, images can be compiled into an application,
 * making it unnecessary to refer to external image files at runtime.
 * GdkPixBuf includes a utility named gdk-pixbuf-csource, which
 * can be used to convert image files into #GdkPixdata structures suitable
 * for inclusion in C sources. To convert the #GdkPixdata structures back 
 * into #GdkPixbufs, use gdk_pixbuf_from_pixdata.
 */

/* --- functions --- */

#define	return_header_corrupt(error)	{ \
  g_set_error_literal (error, GDK_PIXBUF_ERROR, \
                       GDK_PIXBUF_ERROR_CORRUPT_IMAGE, _("Image header corrupt")); \
  return FALSE; \
}
#define	return_invalid_format(error)	{ \
  g_set_error_literal (error, GDK_PIXBUF_ERROR, \
                       GDK_PIXBUF_ERROR_UNKNOWN_TYPE, _("Image format unknown")); \
  return FALSE; \
}
#define	return_pixel_corrupt(error)	{ \
  g_set_error_literal (error, GDK_PIXBUF_ERROR, \
                       GDK_PIXBUF_ERROR_CORRUPT_IMAGE, _("Image pixel data corrupt")); \
  return FALSE; \
}

static inline const uint8_t *get_uint32(const uint8_t *stream, uint32_t *result)
{
	*result = (stream[0] << 24) + (stream[1] << 16) + (stream[2] << 8) + stream[3];
	return stream + 4;
}

/**
 * gdk_pixdata_deserialize:
 * @pixdata: a #GdkPixdata structure to be filled in.
 * @stream_length: length of the stream used for deserialization.
 * @stream: (array length=stream_length): stream of bytes containing a
 *   serialized #GdkPixdata structure.
 *
 * Deserializes (reconstruct) a #GdkPixdata structure from a byte stream.
 * The byte stream consists of a straightforward writeout of the
 * #GdkPixdata fields in network byte order, plus the @pixel_data
 * bytes the structure points to.
 * The @pixdata contents are reconstructed byte by byte and are checked
 * for validity. This function may fail with %GDK_PIXBUF_ERROR_CORRUPT_IMAGE
 * or %GDK_PIXBUF_ERROR_UNKNOWN_TYPE.
 *
 * Return value: Upon successful deserialization %TRUE is returned,
 * %FALSE otherwise.
 **/
gboolean mygdk_pixdata_deserialize(GdkPixdata *pixdata, uint32_t stream_length, const uint8_t *stream)
{
	uint32_t color_type;
	uint32_t sample_width;
	uint32_t encoding;

	if (pixdata == NULL)
		return FALSE;
	if (stream_length != 0 && stream_length < GDK_PIXDATA_HEADER_LENGTH)
		return_header_corrupt(error);
	if (stream == NULL)
		return FALSE;

	/* deserialize header */
	stream = get_uint32(stream, &pixdata->magic);
	stream = get_uint32(stream, & pixdata->length);
	if (pixdata->magic != GDK_PIXBUF_MAGIC_NUMBER || pixdata->length < GDK_PIXDATA_HEADER_LENGTH)
		return_header_corrupt(error);
	stream = get_uint32(stream, &pixdata->pixdata_type);
	stream = get_uint32(stream, &pixdata->rowstride);
	stream = get_uint32(stream, &pixdata->width);
	stream = get_uint32(stream, &pixdata->height);
	if (pixdata->width < 1 || pixdata->height < 1 || pixdata->rowstride < pixdata->width)
		return_header_corrupt(error);
	color_type = pixdata->pixdata_type & GDK_PIXDATA_COLOR_TYPE_MASK;
	sample_width = pixdata->pixdata_type & GDK_PIXDATA_SAMPLE_WIDTH_MASK;
	encoding = pixdata->pixdata_type & GDK_PIXDATA_ENCODING_MASK;
	if ((color_type != GDK_PIXDATA_COLOR_TYPE_RGB &&
		 color_type != GDK_PIXDATA_COLOR_TYPE_RGBA) ||
		sample_width != GDK_PIXDATA_SAMPLE_WIDTH_8 ||
		(encoding != GDK_PIXDATA_ENCODING_RAW && encoding != GDK_PIXDATA_ENCODING_RLE))
		return_invalid_format(error);

	/* deserialize pixel data */
	if (stream_length < pixdata->length - GDK_PIXDATA_HEADER_LENGTH)
		return_pixel_corrupt(error);
	pixdata->pixel_data = (uint8_t *)NO_CONST(stream);

	return TRUE;
}


/**
 * gdk_pixbuf_from_pixdata:
 * @pixdata: a #GdkPixdata to convert into a #GdkPixbuf.
 * @copy_pixels: whether to copy raw pixel data; run-length encoded
 *     pixel data is always copied.
 * 
 * Converts a #GdkPixdata to a #GdkPixbuf. If @copy_pixels is %TRUE or
 * if the pixel data is run-length-encoded, the pixel data is copied into
 * newly-allocated memory; otherwise it is reused.
 *
 * Returns: (transfer full): a new #GdkPixbuf.
 **/
GdkPixbuf *mygdk_pixbuf_from_pixdata(const GdkPixdata *pixdata, gboolean copy_pixels)
{
	uint32_t encoding;
	uint32_t bpp;
	uint8_t *data = NULL;

	if (pixdata == NULL)
		return NULL;
	if (pixdata->width <= 0)
		return NULL;
	if (pixdata->height <= 0)
		return NULL;
	if (pixdata->rowstride < pixdata->width)
		return NULL;
	if ((pixdata->pixdata_type & GDK_PIXDATA_COLOR_TYPE_MASK) != GDK_PIXDATA_COLOR_TYPE_RGB &&
		(pixdata->pixdata_type & GDK_PIXDATA_COLOR_TYPE_MASK) != GDK_PIXDATA_COLOR_TYPE_RGBA)
		return NULL;
	if ((pixdata->pixdata_type & GDK_PIXDATA_SAMPLE_WIDTH_MASK) != GDK_PIXDATA_SAMPLE_WIDTH_8)
		return NULL;
	if ((pixdata->pixdata_type & GDK_PIXDATA_ENCODING_MASK) != GDK_PIXDATA_ENCODING_RAW &&
		(pixdata->pixdata_type & GDK_PIXDATA_ENCODING_MASK) != GDK_PIXDATA_ENCODING_RLE)
		return NULL;
	if (pixdata->pixel_data == NULL)
		return NULL;

	bpp = (pixdata->pixdata_type & GDK_PIXDATA_COLOR_TYPE_MASK) == GDK_PIXDATA_COLOR_TYPE_RGB ? 3 : 4;
	encoding = pixdata->pixdata_type & GDK_PIXDATA_ENCODING_MASK;

	if (encoding == GDK_PIXDATA_ENCODING_RLE)
		copy_pixels = TRUE;

	if (copy_pixels)
	{
		data = g_malloc(pixdata->height * pixdata->rowstride);
		if (!data)
		{
			return NULL;
		}
	}
	if (encoding == GDK_PIXDATA_ENCODING_RLE)
	{
		const uint8_t *rle_buffer = pixdata->pixel_data;
		uint8_t *image_buffer = data;
		uint8_t *image_limit = data + pixdata->rowstride * pixdata->height;
		gboolean check_overrun = FALSE;

		while (image_buffer < image_limit)
		{
			uint32_t length;

			length = *(rle_buffer++);

			if (length & 128)
			{
				length = length - 128;
				check_overrun = image_buffer + length * bpp > image_limit;
				if (check_overrun)
					length = (uint32_t)(image_limit - image_buffer) / bpp;
				if (bpp < 4)			/* RGB */
					do
					{
						memcpy(image_buffer, rle_buffer, 3);
						image_buffer += 3;
					}
					while (--length);
				else					/* RGBA */
					do
					{
						memcpy(image_buffer, rle_buffer, 4);
						image_buffer += 4;
					}
					while (--length);
				rle_buffer += bpp;
			} else
			{
				length *= bpp;
				check_overrun = image_buffer + length > image_limit;
				if (check_overrun)
					length = (uint32_t)(image_limit - image_buffer);
				memcpy(image_buffer, rle_buffer, length);
				image_buffer += length;
				rle_buffer += length;
			}
		}
		if (check_overrun)
		{
			g_free(data);
			g_set_error_literal(error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_CORRUPT_IMAGE, _("Image pixel data corrupt"));
			return NULL;
		}
	} else if (copy_pixels)
	{
		memcpy(data, pixdata->pixel_data, pixdata->rowstride * pixdata->height);
	} else
	{
		data = pixdata->pixel_data;
	}

	return gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB,
									(pixdata->pixdata_type & GDK_PIXDATA_COLOR_TYPE_MASK) ==
									GDK_PIXDATA_COLOR_TYPE_RGBA, 8, pixdata->width, pixdata->height, pixdata->rowstride,
									copy_pixels ? (GdkPixbufDestroyNotify) g_free : NULL, data);
}


/**
 * gdk_pixbuf_new_from_inline:
 * @data_length: Length in bytes of the @data argument or -1 to 
 *    disable length checks
 * @data: (array length=data_length): Byte data containing a
 *    serialized #GdkPixdata structure
 * @copy_pixels: Whether to copy the pixel data, or use direct pointers
 *               @data for the resulting pixbuf
 *
 * Create a #GdkPixbuf from a flat representation that is suitable for
 * storing as inline data in a program. This is useful if you want to
 * ship a program with images, but don't want to depend on any
 * external files.
 *
 * gdk-pixbuf ships with a program called [gdk-pixbuf-csource][gdk-pixbuf-csource],
 * which allows for conversion of #GdkPixbufs into such a inline representation.
 * In almost all cases, you should pass the `--raw` option to
 * `gdk-pixbuf-csource`. A sample invocation would be:
 *
 * |[
 *  gdk-pixbuf-csource --raw --name=myimage_inline myimage.png
 * ]|
 * 
 * For the typical case where the inline pixbuf is read-only static data,
 * you don't need to copy the pixel data unless you intend to write to
 * it, so you can pass %FALSE for @copy_pixels.  (If you pass `--rle` to
 * `gdk-pixbuf-csource`, a copy will be made even if @copy_pixels is %FALSE,
 * so using this option is generally a bad idea.)
 *
 * If you create a pixbuf from const inline data compiled into your
 * program, it's probably safe to ignore errors and disable length checks, 
 * since things will always succeed:
 * |[
 * pixbuf = gdk_pixbuf_new_from_inline (-1, myimage_inline, FALSE, NULL);
 * ]|
 *
 * For non-const inline data, you could get out of memory. For untrusted 
 * inline data located at runtime, you could have corrupt inline data in 
 * addition.
 *
 * Return value: A newly-created #GdkPixbuf structure with a reference,
 *   count of 1, or %NULL if an error occurred.
 **/
GdkPixbuf *mygdk_pixbuf_new_from_inline(uint32_t data_length, const uint8_t *data, gboolean copy_pixels)
{
	GdkPixdata pixdata;

	if (data_length != 0 && data_length != (uint32_t)-1)
		if (data_length <= GDK_PIXDATA_HEADER_LENGTH)
			return NULL;
	if (data == NULL)
		return NULL;

	if (!mygdk_pixdata_deserialize(&pixdata, data_length, data))
		return NULL;

	return mygdk_pixbuf_from_pixdata(&pixdata, copy_pixels);
}
