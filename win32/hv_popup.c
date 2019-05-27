/*
 * HypView - (c)      - 2019 Thorsten Otto
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, see <http://www.gnu.org/licenses/>.
 */

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
		DestroyWindow(parentwin->popup->hwnd);
	win = win32_hypview_window_new(newdoc, TRUE);
	if (win != NULL)
	{
		win->parentwin = parentwin;

		newdoc->start_line = line;
		if (newdoc->gotoNodeProc(win, NULL, num))
		{
			parentwin->popup = win;
			ReInitWindow(win, FALSE);
			MoveWindow(win->hwnd, x, y, win->docsize.w + win->x_margin_left + win->x_margin_right, win->docsize.h + win->y_margin_top + win->y_margin_bottom, TRUE);
			hv_win_open(win);
			ret = TRUE;
		} else
		{
			DestroyWindow(win->hwnd);
		}
	}
	return ret;
}
