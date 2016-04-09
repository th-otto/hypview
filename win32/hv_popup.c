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
	win = win32_hypview_window_new(newdoc, TRUE);
	if (win != NULL)
	{
		win->parentwin = parentwin;

		if (newdoc->gotoNodeProc(win, NULL, num))
		{
			parentwin->popup = win;
			ReInitWindow(win, FALSE);
			MoveWindow(win->hwnd, x, y, win->docsize.w + win->x_margin_left + win->x_margin_right, win->docsize.h + win->y_margin_top + win->y_margin_bottom, TRUE);
			hv_win_open(win);
		} else
		{
			DestroyWindow(win->hwnd);
		}
	}
}
