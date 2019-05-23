#include "hv_defs.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

gboolean OpenPopup(WINDOW_DATA *parentwin, hyp_nodenr num, hyp_lineno line, int x, int y)
{
	DOCUMENT *doc = parentwin->data;
	DOCUMENT *newdoc = hypdoc_ref(doc);
	WINDOW_DATA *win;
	gboolean ret = FALSE;
	
	if (parentwin->popup)
		macos_destroy_window(parentwin->popup);
	win = macos_hypview_window_new(newdoc, TRUE);
	if (win != NULL)
	{
		win->parentwin = parentwin;

		newdoc->start_line = line;
		if (newdoc->gotoNodeProc(win, NULL, num))
		{
			parentwin->popup = win;
			ReInitWindow(win, FALSE);
			/* MoveWindow(win, x, y, win->docsize.w + win->x_margin_left + win->x_margin_right, win->docsize.h + win->y_margin_top + win->y_margin_bottom, TRUE); */
			hv_win_open(win);
			ret = TRUE;
		} else
		{
			macos_destroy_window(win);
		}
	}
	return ret;
}
