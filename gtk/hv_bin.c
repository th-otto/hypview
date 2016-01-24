#include "hv_gtk.h"
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
	long line = -1;

	if (doc->type != HYP_FT_BINARY)
	{
		HYP_DBG(("Illegal call for this document type"));
		return;
	}

	/* if line was not found in document... */
	if (line < 0)
	{
		pos->line = 0;
		pos->y = 0;
		pos->offset = 0;
		pos->x = 0;
		return;
	}
	if (line >= doc->lines)
	{
		pos->line = doc->lines;
		pos->y = doc->lines * win->y_raster;
		pos->offset = 0;
		pos->x = 0;
		return;
	}
	/* YYY */
	UNUSED(x);
	UNUSED(y);
}
