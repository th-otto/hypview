#include "hv_gtk.h"
#include "picture.h"

typedef guint32 cairo_pixel_t;

#define XMAX_PLANES 32

#define RGBA(r,g,b,a)\
	(((cairo_pixel_t)((unsigned char)(a)) << 24) | \
	 ((cairo_pixel_t)((unsigned char)(r)) << 16) | \
	 ((cairo_pixel_t)((unsigned char)(g)) <<  8) | \
	 ((cairo_pixel_t)((unsigned char)(b))     ))
#define RGB(r,g,b) RGBA(r, g, b, ALPHA_OPAQUE)
#define ALPHA_OPAQUE 0xff

static const cairo_user_data_key_t gdk_pixbuf_cairo_data_key = { 0 };


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void cairo_destroy_pixbuf_data(void *data)
{
	g_free(data);
}

/*** ---------------------------------------------------------------------- ***/

#if 0
/* cairo surface -> gdk pixbuf */
static inline guint8 unpremult_alpha(guint8 src, guint8 alpha)
{
	return alpha ? ((src * 255 + alpha / 2) / alpha) : 0;
}

/*** ---------------------------------------------------------------------- ***/

void convert_bgra_to_rgba(const cairo_pixel_t *src, guint8 *dst, int width, int height)
{
	const cairo_pixel_t *src_pixel = src;
	guint8 *dst_pixel = dst;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			guint8 a = (pixel & 0xff000000) >> 24;
			guint8 r = (pixel & 0x00ff0000) >> 16;
			guint8 g = (pixel & 0x0000ff00) >>  8;
			guint8 b = (pixel & 0x000000ff) >>  0;
			dst_pixel[0] = unpremult_alpha(r, a);
			dst_pixel[1] = unpremult_alpha(g, a);
			dst_pixel[2] = unpremult_alpha(b, a);
			dst_pixel[3] = a;

			dst_pixel += 4;
			src_pixel += 1;
		}
	}
}
#endif

/*** ---------------------------------------------------------------------- ***/

static inline guint8 premult_alpha(guint8 src, guint8 alpha)
{
    int temp = (alpha * src) + 0x80;
    return ((temp + (temp >> 8)) >> 8);
}

/*** ---------------------------------------------------------------------- ***/

/* gdk pixbuf -> cairo surface */
cairo_surface_t *convert_pixbuf_to_cairo(GdkPixbuf *pixbuf)
{
	int width, height, src_stride, dst_stride;
	guint8 const *src_pixel;
	guint8 const *src_pixels;
	cairo_format_t format;
	cairo_pixel_t *dst_pixel;
	guint8 *data;
	guint8 r, g, b, a;
	gboolean has_alpha;
	cairo_surface_t *surf;
	int x, y;
	
	g_return_val_if_fail(GDK_IS_PIXBUF(pixbuf), NULL);
	g_return_val_if_fail(gdk_pixbuf_get_colorspace(pixbuf) == GDK_COLORSPACE_RGB, NULL);
	g_return_val_if_fail(gdk_pixbuf_get_n_channels(pixbuf) == 3 || gdk_pixbuf_get_n_channels(pixbuf) == 4, NULL);
	g_return_val_if_fail(gdk_pixbuf_get_bits_per_sample(pixbuf) == 8, NULL);

	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	src_stride = gdk_pixbuf_get_rowstride(pixbuf);
	src_pixels = gdk_pixbuf_get_pixels(pixbuf);
	has_alpha = gdk_pixbuf_get_has_alpha(pixbuf);
	
	if (gdk_pixbuf_get_n_channels(pixbuf) == 4)
	{
		format = has_alpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24;
		dst_stride = cairo_format_stride_for_width(format, width);
		data = g_new(guint8, height * dst_stride);
		for (y = 0; y < height; y++)
		{
			dst_pixel = (cairo_pixel_t *)(data + y * dst_stride);
			src_pixel = src_pixels + y * src_stride;
			for (x = 0; x < width; x++)
			{
 				r = src_pixel[0];
				g = src_pixel[1];
				b = src_pixel[2];
				if (has_alpha)
				{
					a = src_pixel[3];
					r = premult_alpha(r, a);
					g = premult_alpha(g, a);
					b = premult_alpha(b, a);
				} else
				{
					a = ALPHA_OPAQUE;
				}
				dst_pixel[0] = RGBA(r, g, b, a);
	
				dst_pixel += 1;
				src_pixel += 4;
			}
		}
	} else
	{
		format = CAIRO_FORMAT_RGB24;
		dst_stride = cairo_format_stride_for_width(format, width);
		data = g_new(guint8, height * dst_stride);
		for (y = 0; y < height; y++)
		{
			dst_pixel = (cairo_pixel_t *)(data + y * dst_stride);
			src_pixel = src_pixels + y * src_stride;
			for (x = 0; x < width; x++)
			{
 				r = src_pixel[0];
				g = src_pixel[1];
				b = src_pixel[2];
				dst_pixel[0] = RGB(r, g, b);
	
				dst_pixel += 1;
				src_pixel += 3;
			}
		}
	}
	surf = cairo_image_surface_create_for_data(data, format, width, height, dst_stride);
	if (cairo_surface_set_user_data(surf, &gdk_pixbuf_cairo_data_key, (void *)data, cairo_destroy_pixbuf_data) != CAIRO_STATUS_SUCCESS)
	{
		cairo_surface_finish(surf);
		g_free(data);
	}
	return surf;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

_WORD GetNumColors(void)
{
	GdkVisual *visual = gdk_visual_get_system();
	int depth = gdk_visual_get_depth(visual);
	return depth <= 1 ? 2 : depth <= 4 ? 16 : depth <= 8 ? 256 : 32767;
}

/*** ---------------------------------------------------------------------- ***/

_WORD GetNumPlanes(void)
{
	GdkVisual *visual = gdk_visual_get_system();
	return gdk_visual_get_depth(visual);
}

/*** ---------------------------------------------------------------------- ***/

void W_Release_Bitmap(void **pdata, _WORD width, _WORD height, _WORD planes)
{
	GdkPixbuf *data = *pdata;
	
	UNUSED(width);
	UNUSED(height);
	UNUSED(planes);
	if (data != NULL)
	{
		/*
		 * image was converted to pixbuf
		 */
		gdk_pixbuf_unref(data);
		*pdata = NULL;
	}
}

/*** ---------------------------------------------------------------------- ***/

void gdk_pixbuf_make_transparent(GdkPixbuf *pixbuf)
{
	int x, y;
	guchar r, g, b;
	guchar *src, *ptr;
	size_t rowstride;
	int width, height;
	
	g_return_if_fail(GDK_IS_PIXBUF(pixbuf));
	g_return_if_fail(gdk_pixbuf_get_colorspace(pixbuf) == GDK_COLORSPACE_RGB);
	g_return_if_fail(gdk_pixbuf_get_n_channels(pixbuf) == 4);
	g_return_if_fail(gdk_pixbuf_get_bits_per_sample(pixbuf) == 8);
	g_return_if_fail(gdk_pixbuf_get_has_alpha(pixbuf));

	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	if (width <= 0 || height <= 0)
		return;
	src = gdk_pixbuf_get_pixels(pixbuf);
	r = src[0];
	g = src[1];
	b = src[2];
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	for (y = 0; y < height; y++)
	{
		ptr = src;
		for (x = 0; x < width; x++)
		{
			if (ptr[0] == r && ptr[1] == g && ptr[2] == b)
				ptr[3] = 0;
			ptr += 4;
		}
		src += rowstride;
	}
}

/*** ---------------------------------------------------------------------- ***/

void gdk_pixbuf_make_opaque(GdkPixbuf *pixbuf)
{
	int x, y;
	guchar *src, *ptr;
	size_t rowstride;
	int width, height;
	
	g_return_if_fail(GDK_IS_PIXBUF(pixbuf));
	g_return_if_fail(gdk_pixbuf_get_colorspace(pixbuf) == GDK_COLORSPACE_RGB);
	g_return_if_fail(gdk_pixbuf_get_n_channels(pixbuf) == 4);
	g_return_if_fail(gdk_pixbuf_get_bits_per_sample(pixbuf) == 8);
	g_return_if_fail(gdk_pixbuf_get_has_alpha(pixbuf));

	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	if (width <= 0 || height <= 0)
		return;
	src = gdk_pixbuf_get_pixels(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	for (y = 0; y < height; y++)
	{
		ptr = src;
		for (x = 0; x < width; x++)
		{
			ptr[3] = 0xff;
			ptr += 4;
		}
		src += rowstride;
	}
}

/*** ---------------------------------------------------------------------- ***/

gboolean W_Fix_Bitmap(void **pdata, _WORD _width, _WORD _height, _WORD _planes)
{
	int width = _width;
	int height = _height;
	int planes = _planes;
	GdkPixbuf *pixbuf;
	guint8 *src = *pdata;
	guint8 *dst;
	int dststride;
	int srcstride;
	int x, planesize, pos;
	int i;
	PALETTE pal;
	guint16 back[XMAX_PLANES];
	guint8 *plane_ptr[XMAX_PLANES];
	int np;
	int pixel;
	guint16 color;
	
	pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
	if (pixbuf == NULL)
	{
		*pdata = NULL;
		return FALSE;
	}
	
	srcstride = ((width + 15) >> 4) << 1;
	planesize = srcstride * height;
	dststride = gdk_pixbuf_get_rowstride(pixbuf);
	pic_stdpalette(pal, planes);
	dst = gdk_pixbuf_get_pixels(pixbuf);
	
	for (i = 0; i < planes; i++)
		plane_ptr[i] = &src[i * planesize];
	
	pos = 0;
	width <<= 2; /* we write 4 bytes per picel */
	for (x = 0; x < planesize; x += 2)
	{
		for (np = 0; np < planes; np++)
			back[np] = (plane_ptr[np][x] << 8) | plane_ptr[np][x + 1];
		
		for (pixel = 0; pixel < 16; pixel++)
		{
			color = 0;
			for (np = 0; np < planes; np++)
			{
				color |= ((back[np] & 0x8000) >> (15 - np));
				back[np] <<= 1;
			}
			if (pos < width)
			{
				dst[pos++] = pal[color].r;
				dst[pos++] = pal[color].g;
				dst[pos++] = pal[color].b;
				dst[pos++] = 0xff; /* alpha */
			}
		}
		if (pos >= width)
		{
			pos = 0;
			dst += dststride;
		}
	}

	if (gl_profile.viewer.transparent_pics)
	{
		gdk_pixbuf_make_transparent(pixbuf);
	}	
	*pdata = pixbuf;
	/* return value indicates wether data was converted inplace */
	return FALSE;
}
