#include "hv_defs.h"
#include "hv_ascii.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void BinaryDisplayPage(WINDOW_DATA *win)
{
	/* nothing to do */
	UNUSED(win);
}

/*** ---------------------------------------------------------------------- ***/

void BinaryGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos)
{
	DOCUMENT *doc = win->data;

	if (doc->type != HYP_FT_BINARY)
	{
		HYP_DBG(("Illegal call for this document type"));
		return;
	}

	/* not needed */
	UNUSED(x);
	UNUSED(y);
	UNUSED(pos);
}
