#include "hv_gtk.h"
#include "hv_ascii.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void AsciiDisplayPage(DOCUMENT *doc)
{
	UNUSED(doc);
	/* nothing to do */
}

/*** ---------------------------------------------------------------------- ***/

void AsciiGetCursorPosition(DOCUMENT *doc, int x, int y, TEXT_POS *pos)
{
	long line = -1;
	WINDOW_DATA *win = doc->window;
	
	if (doc->type != HYP_FT_ASCII)
	{
		HYP_DBG(("Illegal call for this document type"));
		return;
	}

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
