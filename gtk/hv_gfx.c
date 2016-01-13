#include "hv_gtk.h"

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

gboolean W_Fix_Bitmap(void **pdata, _WORD width, _WORD height, _WORD planes)
{
	/* YYY */
	UNUSED(pdata);
	UNUSED(width);
	UNUSED(height);
	UNUSED(planes);
	return FALSE;
}
