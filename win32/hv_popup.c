#include "hv_defs.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void OpenPopup(WINDOW_DATA *parentwin, hyp_nodenr num, int x, int y)
{
	DOCUMENT *doc = parentwin->data;
	DOCUMENT *newdoc = hypdoc_ref(doc);
	WINDOW_DATA *win;
	
	if (parentwin->popup)
		DestroyWindow(parentwin->popup->hwnd);
	win = gtk_hypview_window_new(newdoc, TRUE);
	if (win != NULL)
	{
		win->parentwin = parentwin;

		if (newdoc->gotoNodeProc(win, NULL, num))
		{
			char *geom;
			
			parentwin->popup = win;
			geom = g_strdup_printf("+%d+%d", x, y);
			hv_win_set_geometry(geom);
			g_free(geom);
			ReInitWindow(win, FALSE);
			hv_win_open(win);
			hv_win_set_geometry(NULL);
		} else
		{
			DestroyWindow(win->hwnd);
		}
	}
}
