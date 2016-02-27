#include "hv_defs.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/* add a new character to the Autolocator and start search */
gboolean AutolocatorKey(WINDOW_DATA *win, WPARAM wparam, LPARAM lparam)
{
	UNUSED(win);
	UNUSED(wparam);
	UNUSED(lparam);
	/* NYI */
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/* insert contents of clipboard in autolocator. */
void AutoLocatorPaste(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	
	if (!doc->buttons.searchbox)
		return;
	/* NYI */
}
