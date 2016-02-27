#include "hv_defs.h"
#include "picture.h"

static _WORD screen_w, screen_h;
static _WORD screen_colors;
static _WORD screen_planes;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void init_screen(void)
{
	HDC hDC;
	_WORD bits;

	if (screen_w == 0)
	{
		screen_w = (_WORD) GetSystemMetrics(SM_CXSCREEN);
		screen_h = (_WORD) GetSystemMetrics(SM_CYSCREEN);

		hDC = GetDC(NULL);
		screen_planes = GetDeviceCaps(hDC, PLANES);
		bits = GetDeviceCaps(hDC, BITSPIXEL);
		screen_planes *= bits;
		screen_colors = GetDeviceCaps(hDC, NUMCOLORS);
		if ((screen_planes * bits) >= 16)
			screen_colors = 32766;		/* more than we need */

		ReleaseDC(NULL, hDC);
	}
}

/*** ---------------------------------------------------------------------- ***/

GLOBAL _WORD GetNumPlanes(_VOID)
{
	init_screen();
	return screen_planes;
}

/*** ---------------------------------------------------------------------- ***/

_WORD GetNumColors(void)
{
	init_screen();
	return screen_colors;
}

/*** ---------------------------------------------------------------------- ***/

void W_Release_Bitmap(void **pdata, _WORD width, _WORD height, _WORD planes)
{
	/* NYI */
	UNUSED(pdata);
	UNUSED(width);
	UNUSED(height);
	UNUSED(planes);
}

/*** ---------------------------------------------------------------------- ***/

gboolean W_Fix_Bitmap(void **pdata, _WORD width, _WORD height, _WORD planes)
{
	/* NYI */
	UNUSED(pdata);
	UNUSED(width);
	UNUSED(height);
	UNUSED(planes);
	*pdata = NULL;
	return FALSE;
}
