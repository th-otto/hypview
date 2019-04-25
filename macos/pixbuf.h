/* GdkPixbuf library - GdkPixbuf data structure
 *
 * Copyright (C) 2003 The Free Software Foundation
 *
 * Authors: Mark Crichton <crichton@gimp.org>
 *          Miguel de Icaza <miguel@gnu.org>
 *          Federico Mena-Quintero <federico@gimp.org>
 *          Havoc Pennington <hp@redhat.com>
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

#ifndef GDK_PIXBUF_CORE_H
#define GDK_PIXBUF_CORE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __G_LIB_H__
typedef int gboolean;
#endif

/**
 * SECTION:gdk-pixbuf
 * @Short_description: Information that describes an image.
 * @Title: The GdkPixbuf Structure
 * 
 * The #GdkPixbuf structure contains
 * information that describes an image in memory.
 * 
 * ## Image Data ## {#image-data}
 *
 * Image data in a pixbuf is stored in memory in uncompressed,
 * packed format.  Rows in the image are stored top to bottom, and
 * in each row pixels are stored from left to right.  There may be
 * padding at the end of a row.  The "rowstride" value of a pixbuf,
 * as returned by gdk_pixbuf_get_rowstride(), indicates the number
 * of bytes between rows.
 * 
 * ## put_pixel() Example ## {#put-pixel}
 * 
 * The following code illustrates a simple put_pixel()
 * function for RGB pixbufs with 8 bits per channel with an alpha
 * channel.  It is not included in the gdk-pixbuf library for
 * performance reasons; rather than making several function calls
 * for each pixel, your own code can take shortcuts.
 * 
 * |[<!-- language="C" -->
 * static void
 * put_pixel (GdkPixbuf *pixbuf, int x, int y, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
 * {
 *   int width, height, rowstride, n_channels;
 *   uint8_t *pixels, *p;
 * 
 *   n_channels = gdk_pixbuf_get_n_channels (pixbuf);
 * 
 *   g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
 *   g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
 *   g_assert (gdk_pixbuf_get_has_alpha (pixbuf));
 *   g_assert (n_channels == 4);
 * 
 *   width = gdk_pixbuf_get_width (pixbuf);
 *   height = gdk_pixbuf_get_height (pixbuf);
 * 
 *   g_assert (x >= 0 && x < width);
 *   g_assert (y >= 0 && y < height);
 * 
 *   rowstride = gdk_pixbuf_get_rowstride (pixbuf);
 *   pixels = gdk_pixbuf_get_pixels (pixbuf);
 * 
 *   p = pixels + y * rowstride + x * n_channels;
 *   p[0] = red;
 *   p[1] = green;
 *   p[2] = blue;
 *   p[3] = alpha;
 * }
 * ]|
 * 
 * This function will not work for pixbufs with images that are
 * other than 8 bits per sample or channel, but it will work for
 * most of the pixbufs that GTK+ uses.
 * 
 * If you are doing memcpy() of raw pixbuf data, note that the last row
 * in the pixbuf may not be as wide as the full rowstride, but rather
 * just as wide as the pixel data needs to be. That is, it is unsafe to
 * do `memcpy (dest, pixels, rowstride * height)` to copy a whole pixbuf.
 * Use gdk_pixbuf_copy() instead, or compute the width in bytes of the
 * last row as `width * ((n_channels * bits_per_sample + 7) / 8)`.
 */


/**
 * GdkPixbufAlphaMode:
 * @GDK_PIXBUF_ALPHA_BILEVEL: A bilevel clipping mask (black and white)
 *  will be created and used to draw the image.  Pixels below 0.5 opacity
 *  will be considered fully transparent, and all others will be
 *  considered fully opaque.
 * @GDK_PIXBUF_ALPHA_FULL: For now falls back to #GDK_PIXBUF_ALPHA_BILEVEL.
 *  In the future it will do full alpha compositing.
 * 
 * These values can be passed to
 * gdk_pixbuf_xlib_render_to_drawable_alpha() to control how the alpha
 * channel of an image should be handled.  This function can create a
 * bilevel clipping mask (black and white) and use it while painting
 * the image.  In the future, when the X Window System gets an alpha
 * channel extension, it will be possible to do full alpha
 * compositing onto arbitrary drawables.  For now both cases fall
 * back to a bilevel clipping mask.
 */
typedef enum
{
        GDK_PIXBUF_ALPHA_BILEVEL,
        GDK_PIXBUF_ALPHA_FULL
} GdkPixbufAlphaMode;

/**
 * GdkColorspace:
 * @GDK_COLORSPACE_RGB: Indicates a red/green/blue additive color space.
 * 
 * This enumeration defines the color spaces that are supported by
 * the gdk-pixbuf library.  Currently only RGB is supported.
 */
/* Note that these values are encoded in inline pixbufs
 * as ints, so don't reorder them
 */
typedef enum {
	GDK_COLORSPACE_RGB
} GdkColorspace;

/* All of these are opaque structures */


/**
 * GdkPixbufDestroyNotify:
 * @pixels: (array) (element-type guint8): The pixel array of the pixbuf
 *   that is being finalized.
 * @data: (closure): User closure data.
 * 
 * A function of this type is responsible for freeing the pixel array
 * of a pixbuf.  The gdk_pixbuf_new_from_data() function lets you
 * pass in a pre-allocated pixel array so that a pixbuf can be
 * created from it; in this case you will need to pass in a function
 * of #GdkPixbufDestroyNotify so that the pixel data can be freed
 * when the pixbuf is finalized.
 */
typedef void (* GdkPixbufDestroyNotify) (uint8_t *pixels, void *data);


/**
 * GdkPixbuf:
 * 
 * This is the main structure in the gdk-pixbuf library.  It is
 * used to represent images.  It contains information about the
 * image's pixel data, its color space, bits per sample, width and
 * height, and the rowstride (the number of bytes between the start of
 * one row and the start of the next). 
 */
typedef struct _GdkPixbuf GdkPixbuf;

/* Private part of the GdkPixbuf structure */
struct _GdkPixbuf {
	unsigned int refcount;
    /* GObject parent_instance; */

	/* Color space */
	GdkColorspace colorspace;

	/* Number of channels, alpha included */
	int n_channels;

	/* Bits per channel */
	int bits_per_sample;

	/* Size */
	int width, height;

	/* Offset between rows */
	int rowstride;

	/* The pixel array */
	union {
		uint8_t *d;
		const uint8_t *c;
	} pixels;

	/* Destroy notification function; it is supposed to free the pixel array */
	GdkPixbufDestroyNotify destroy_fn;

	/* User data for the destroy notification function */
	void *destroy_fn_data;

	/* Do we have an alpha channel? */
	unsigned has_alpha : 1;
};


/* Reference counting */
GdkPixbuf *gdk_pixbuf_ref(GdkPixbuf *pixbuf);
void gdk_pixbuf_unref(GdkPixbuf *pixbuf);

/* GdkPixbuf accessors */

#define gdk_pixbuf_get_colorspace(pixbuf) ((pixbuf)->colorspace)
#define gdk_pixbuf_get_n_channels(pixbuf) ((pixbuf)->n_channels)
#define gdk_pixbuf_get_has_alpha(pixbuf) ((pixbuf)->has_alpha)
#define gdk_pixbuf_get_bits_per_sample(pixbuf) ((pixbuf)->bits_per_sample)
#define gdk_pixbuf_get_pixels(pixbuf) ((pixbuf)->pixels.d)
#define gdk_pixbuf_read_pixels(pixbuf) ((pixbuf)->pixels.c)
#define gdk_pixbuf_get_width(pixbuf) ((pixbuf)->width)
#define gdk_pixbuf_get_height(pixbuf) ((pixbuf)->height)
#define gdk_pixbuf_get_rowstride(pixbuf) ((pixbuf)->rowstride)

/* Create a blank pixbuf with an optimal rowstride and a new buffer */

GdkPixbuf *gdk_pixbuf_new (GdkColorspace colorspace, gboolean has_alpha, int bits_per_sample,
			   int width, int height);

int gdk_pixbuf_calculate_rowstride (GdkColorspace colorspace,
				     gboolean      has_alpha,
				     int           bits_per_sample,
				     int           width,
				     int           height);

/* Copy a pixbuf */
GdkPixbuf *gdk_pixbuf_copy (const GdkPixbuf *pixbuf);

/* Simple loading */

GdkPixbuf *gdk_pixbuf_new_from_data (const uint8_t *data,
				     GdkColorspace colorspace,
				     gboolean has_alpha,
				     int bits_per_sample,
				     int width, int height,
				     int rowstride,
				     GdkPixbufDestroyNotify destroy_fn,
				     void *destroy_fn_data);

GdkPixbuf* mygdk_pixbuf_new_from_inline	(uint32_t data_length, const uint8_t *data,
					 gboolean      copy_pixels);

GdkPixbuf *mygdk_pixbuf_add_alpha(const GdkPixbuf *pixbuf, gboolean substitute_color, uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
extern "C" {
#endif

#endif
