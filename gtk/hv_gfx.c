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
	/* YYY */
	UNUSED(pdata);
	UNUSED(width);
	UNUSED(height);
	UNUSED(planes);
}

/*** ---------------------------------------------------------------------- ***/

void W_Fix_Bitmap(void **pdata, _WORD width, _WORD height, _WORD planes)
{
	/* YYY */
	UNUSED(pdata);
	UNUSED(width);
	UNUSED(height);
	UNUSED(planes);
}
