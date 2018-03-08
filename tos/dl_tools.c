/*
 * HypView - (c)      - 2006 Philipp Donze
 *               2006 -      Philipp Donze & Odd Skancke
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
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hv_defs.h"


WINDOW_DATA *get_first_window(void)
{
	WINDOW_DATA *win;
	_WORD whandle = 0;
	
	wind_get_int(0, WF_TOP, &whandle);
	if (whandle > 0 &&
		(win = find_window_by_whandle(whandle)) != NULL &&
		win->type == WIN_WINDOW &&
		win->owner == gl_apid &&
		(win->status & WIS_OPEN))
		return win;
	win = (WINDOW_DATA *) all_list;
	while (win)
	{
		if (win->type == WIN_WINDOW &&
			win->owner == gl_apid &&
			(win->status & WIS_OPEN))
			return win;
		win = win->next;
	}
	return NULL;
}
