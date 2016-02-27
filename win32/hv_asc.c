#include "hv_defs.h"
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

void AsciiPrep(WINDOW_DATA *win, HYP_NODE *node)
{
	UNUSED(win);
	UNUSED(node);
	/* NYI */
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

	UNUSED(x);
	UNUSED(y);
	UNUSED(pos);
}
