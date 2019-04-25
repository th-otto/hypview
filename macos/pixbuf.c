/* GdkPixbuf library - Image creation from in-memory buffers
 *
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Author: Federico Mena-Quintero <federico@gimp.org>
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
#include "pixbuf.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * gdk_pixbuf_new_from_data:
 * @data: (array): Image data in 8-bit/sample packed format
 * @colorspace: Colorspace for the image data
 * @has_alpha: Whether the data has an opacity channel
 * @bits_per_sample: Number of bits per sample
 * @width: Width of the image in pixels, must be > 0
 * @height: Height of the image in pixels, must be > 0
 * @rowstride: Distance in bytes between row starts
 * @destroy_fn: (scope async) (allow-none): Function used to free the data when the pixbuf's reference count
 * drops to zero, or %NULL if the data should not be freed
 * @destroy_fn_data: (closure): Closure data to pass to the destroy notification function
 * 
 * Creates a new #GdkPixbuf out of in-memory image data.  Currently only RGB
 * images with 8 bits per sample are supported.
 *
 * Since you are providing a pre-allocated pixel buffer, you must also
 * specify a way to free that data.  This is done with a function of
 * type #GdkPixbufDestroyNotify.  When a pixbuf created with is
 * finalized, your destroy notification function will be called, and
 * it is its responsibility to free the pixel array.
 *
 * See also gdk_pixbuf_new_from_bytes().
 *
 * Return value: (transfer full): A newly-created #GdkPixbuf structure with a reference count of 1.
 **/
GdkPixbuf *gdk_pixbuf_new_from_data(const uint8_t *data, GdkColorspace colorspace, gboolean has_alpha,
									int bits_per_sample, int width, int height, int rowstride,
									GdkPixbufDestroyNotify destroy_fn, void *destroy_fn_data)
{
	GdkPixbuf *pixbuf;

	/* Only 8-bit/sample RGB buffers are supported for now */

	if (data == NULL)
		return NULL;
	if (colorspace != GDK_COLORSPACE_RGB)
		return NULL;
	if (bits_per_sample != 8)
		return NULL;
	if (width <= 0)
		return NULL;
	if (height <= 0)
		return NULL;

	pixbuf = (GdkPixbuf *)g_malloc(sizeof(*pixbuf));
	if (pixbuf == NULL)
		return NULL;
	pixbuf->refcount = 1;
	
	pixbuf->colorspace = colorspace;
	pixbuf->n_channels = has_alpha ? 4 : 3;
	pixbuf->bits_per_sample = bits_per_sample;
	pixbuf->width = width;
	pixbuf->height = height;
	pixbuf->rowstride = rowstride;
	pixbuf->pixels.c = data;
	pixbuf->destroy_fn = destroy_fn;
	pixbuf->destroy_fn_data = destroy_fn_data;
	pixbuf->has_alpha = has_alpha;

	return pixbuf;
}


GdkPixbuf *gdk_pixbuf_ref(GdkPixbuf *pixbuf)
{
	assert(pixbuf);
	++pixbuf->refcount;
	return pixbuf;
}


void gdk_pixbuf_unref(GdkPixbuf *pixbuf)
{
	assert(pixbuf);
	assert(pixbuf->refcount > 0);
	if (--pixbuf->refcount == 0)
	{
		if (pixbuf->pixels.d && pixbuf->destroy_fn)
        {
			(* pixbuf->destroy_fn) (pixbuf->pixels.d, pixbuf->destroy_fn_data);
		}
		g_free(pixbuf);

		pixbuf = NULL;
	}
}


static GdkPixbuf *gdk_pixbuf_copydata(const GdkPixbuf *pixbuf, const uint8_t *src)
{
	size_t size = pixbuf->height * pixbuf->rowstride;
	GdkPixbuf *new_pixbuf;
	uint8_t *data;
	
	data = (uint8_t *)g_malloc(size);
	if (data == NULL)
		return NULL;
	if (src != NULL)
		memcpy(data, src, size);
	new_pixbuf = gdk_pixbuf_new_from_data(
		data,
		pixbuf->colorspace,
		pixbuf->has_alpha,
		pixbuf->bits_per_sample,
		pixbuf->width,
		pixbuf->height,
		pixbuf->rowstride,
		(GdkPixbufDestroyNotify) g_free,
		data);
	return new_pixbuf;
}


/**
 * gdk_pixbuf_add_alpha:
 * @pixbuf: A #GdkPixbuf.
 * @substitute_color: Whether to set a color to zero opacity.  If this
 * is %FALSE, then the (@r, @g, @b) arguments will be ignored.
 * @r: Red value to substitute.
 * @g: Green value to substitute.
 * @b: Blue value to substitute.
 *
 * Takes an existing pixbuf and adds an alpha channel to it.
 * If the existing pixbuf already had an alpha channel, the channel
 * values are copied from the original; otherwise, the alpha channel
 * is initialized to 255 (full opacity).
 *
 * If @substitute_color is %TRUE, then the color specified by (@r, @g, @b) will be
 * assigned zero opacity. That is, if you pass (255, 255, 255) for the
 * substitute color, all white pixels will become fully transparent.
 *
 * Return value: (transfer full): A newly-created pixbuf with a reference count of 1.
 **/
GdkPixbuf *mygdk_pixbuf_add_alpha(const GdkPixbuf *pixbuf, gboolean substitute_color, uint8_t r, uint8_t g, uint8_t b)
{
	GdkPixbuf *new_pixbuf;
	int x, y;
	const uint8_t *src_pixels;
	uint8_t *ret_pixels;
	const uint8_t *src;
	uint8_t *dest;

	assert(pixbuf);
	assert(pixbuf->colorspace == GDK_COLORSPACE_RGB);
	assert(pixbuf->n_channels == 3 || pixbuf->n_channels == 4);
	assert(pixbuf->bits_per_sample == 8);

	src_pixels = gdk_pixbuf_read_pixels(pixbuf);

	if (pixbuf->has_alpha)
	{
		new_pixbuf = gdk_pixbuf_copydata(pixbuf, src_pixels);
		if (!new_pixbuf)
			return NULL;

		if (!substitute_color)
			return new_pixbuf;
	} else
	{
		new_pixbuf = gdk_pixbuf_copydata(pixbuf, NULL);
	}

	if (!new_pixbuf)
		return NULL;

	ret_pixels = gdk_pixbuf_get_pixels(new_pixbuf);

	for (y = 0; y < pixbuf->height; y++, src_pixels += pixbuf->rowstride, ret_pixels += new_pixbuf->rowstride)
	{
		uint8_t tr, tg, tb;

		src = src_pixels;
		dest = ret_pixels;

		if (pixbuf->has_alpha)
		{
			/*
			 * Just subst color, we already copied everything
			 * else
			 */
			for (x = 0; x < pixbuf->width; x++)
			{
				if (src[0] == r && src[1] == g && src[2] == b)
					dest[3] = 0;
				src += 4;
				dest += 4;
			}
		} else
		{
			for (x = 0; x < pixbuf->width; x++)
			{
				tr = *dest++ = *src++;
				tg = *dest++ = *src++;
				tb = *dest++ = *src++;

				if (substitute_color && tr == r && tg == g && tb == b)
					*dest++ = 0;
				else
					*dest++ = 255;
			}
		}
	}

	return new_pixbuf;
}
