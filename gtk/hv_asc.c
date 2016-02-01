#include "hv_gtk.h"
#include "hv_ascii.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void AsciiDisplayPage(WINDOW_DATA *win)
{
	UNUSED(win);
	/* nothing to do */
}

/*** ---------------------------------------------------------------------- ***/

void AsciiGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos)
{
	DOCUMENT *doc = win->data;
	
	if (doc->type != HYP_FT_ASCII)
	{
		HYP_DBG(("Illegal call for this document type"));
		return;
	}

	/* not needed in GTK */
	UNUSED(x);
	UNUSED(y);
	UNUSED(pos);
}
