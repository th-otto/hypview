#include "hypdefs.h"

#if 0
_WORD font_cw = HYP_PIC_FONTW;
_WORD font_ch = HYP_PIC_FONTH;
#endif

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

/* ------------------------------------------------------------------------- */

#ifdef __TOS__

const char *_argv0;

char *g_ttp_get_bindir(void)
{
	return g_strdup(_argv0);
}
#endif
