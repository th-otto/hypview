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
#ifndef __GDK_PIXDATA_H__
#define __GDK_PIXDATA_H__

#include "pixbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GDK_PIXBUF_MAGIC_NUMBER
/**
 * GDK_PIXBUF_MAGIC_NUMBER:
 *
 * Magic number for #GdkPixdata structures.
 **/
#define GDK_PIXBUF_MAGIC_NUMBER (0x47646b50)	/* 'GdkP' */
/**
 * GdkPixdataType:
 * @GDK_PIXDATA_COLOR_TYPE_RGB:  each pixel has red, green and blue samples.
 * @GDK_PIXDATA_COLOR_TYPE_RGBA: each pixel has red, green and blue samples 
 *    and an alpha value.
 * @GDK_PIXDATA_COLOR_TYPE_MASK: mask for the colortype flags of the enum.
 * @GDK_PIXDATA_SAMPLE_WIDTH_8: each sample has 8 bits.
 * @GDK_PIXDATA_SAMPLE_WIDTH_MASK: mask for the sample width flags of the enum.
 * @GDK_PIXDATA_ENCODING_RAW: the pixel data is in raw form. 
 * @GDK_PIXDATA_ENCODING_RLE: the pixel data is run-length encoded. Runs may 
 *    be up to 127 bytes long; their length is stored in a single byte 
 *    preceding the pixel data for the run. If a run is constant, its length
 *    byte has the high bit set and the pixel data consists of a single pixel
 *    which must be repeated. 
 * @GDK_PIXDATA_ENCODING_MASK: mask for the encoding flags of the enum.
 *
 * An enumeration containing three sets of flags for a #GdkPixdata struct: 
 * one for the used colorspace, one for the width of the samples and one 
 * for the encoding of the pixel data.  
 **/
	typedef enum
{
	/* colorspace + alpha */
	GDK_PIXDATA_COLOR_TYPE_RGB = 0x01,
	GDK_PIXDATA_COLOR_TYPE_RGBA = 0x02,
	GDK_PIXDATA_COLOR_TYPE_MASK = 0xff,
	/* width, support 8bits only currently */
	GDK_PIXDATA_SAMPLE_WIDTH_8 = 0x01 << 16,
	GDK_PIXDATA_SAMPLE_WIDTH_MASK = 0x0f << 16,
	/* encoding */
	GDK_PIXDATA_ENCODING_RAW = 0x01 << 24,
	GDK_PIXDATA_ENCODING_RLE = 0x02 << 24,
	GDK_PIXDATA_ENCODING_MASK = 0x0f << 24
} GdkPixdataType;

/**
 * GdkPixdata:
 * @magic: magic number. A valid #GdkPixdata structure must have 
 *    #GDK_PIXBUF_MAGIC_NUMBER here.
 * @length: less than 1 to disable length checks, otherwise 
 *    #GDK_PIXDATA_HEADER_LENGTH + length of @pixel_data. 
 * @pixdata_type: information about colorspace, sample width and 
 *    encoding, in a #GdkPixdataType. 
 * @rowstride: Distance in bytes between rows.
 * @width: Width of the image in pixels.
 * @height: Height of the image in pixels.
 * @pixel_data: (array) (element-type guint8): @width x @height pixels, encoded according to @pixdata_type
 *   and @rowstride.
 *
 * A #GdkPixdata contains pixbuf information in a form suitable for 
 * serialization and streaming.
 **/
typedef struct _GdkPixdata GdkPixdata;
struct _GdkPixdata
{
	uint32_t magic;						/* GDK_PIXBUF_MAGIC_NUMBER */
	uint32_t length;					/* <1 to disable length checks, otherwise:
										 * GDK_PIXDATA_HEADER_LENGTH + pixel_data length
										 */
	uint32_t pixdata_type;				/* GdkPixdataType */
	uint32_t rowstride;
	uint32_t width;
	uint32_t height;
	unsigned char *pixel_data;
};

/**
 * GDK_PIXDATA_HEADER_LENGTH:
 *
 * The length of a #GdkPixdata structure without the @pixel_data pointer.
 **/
#define	GDK_PIXDATA_HEADER_LENGTH	(4 + 4 + 4 + 4 + 4 + 4)

#endif

/* the returned stream is plain htonl of GdkPixdata members + pixel_data */
gboolean mygdk_pixdata_deserialize(GdkPixdata * pixdata, uint32_t stream_length, const uint8_t * stream);
GdkPixbuf *mygdk_pixbuf_from_pixdata(const GdkPixdata * pixdata, gboolean copy_pixels);

GdkPixbuf *mygdk_pixbuf_new_from_inline(uint32_t data_length, const unsigned char *data, gboolean copy_pixels);

#ifdef __cplusplus
}
#endif

#endif /* __GDK_PIXDATA_H__ */
