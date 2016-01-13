#include "hv_gtk.h"
#include "picture.h"

#define XMAX_PLANES 32


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
	
	*pdata = pixbuf;
	/* return value indicates wether data was converted inplace */
	return FALSE;
}
