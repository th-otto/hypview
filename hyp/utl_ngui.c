#include "hypdefs.h"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

void W_Release_Bitmap(void **pdata, _WORD width, _WORD height, _WORD planes)
{
	UNUSED(pdata);
	UNUSED(width);
	UNUSED(height);
	UNUSED(planes);
}

/* ------------------------------------------------------------------------- */

gboolean W_Fix_Bitmap(void **pdata, _WORD width, _WORD height, _WORD planes)
{
	UNUSED(pdata);
	UNUSED(width);
	UNUSED(height);
	UNUSED(planes);
	return TRUE;
}

/* ------------------------------------------------------------------------- */

_WORD GetNumPlanes(void)
{
	return 1;
}
