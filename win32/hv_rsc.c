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


void ShowResource(WINDOW_DATA *win, const char *path, _UWORD treenr)
{
	char *str = g_strdup_printf(_("Display of resource files not implemented."));
	UNUSED(path);
	UNUSED(treenr);
	show_message(win ? win->hwnd : NULL, _("Error"), str, FALSE);
	g_free(str);
}
