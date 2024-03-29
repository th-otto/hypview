/*
 * HypView - (c) 2001 - 2006 Philipp Donze
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
#include "hypdebug.h"
#include "hypview.h"


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * Initialize and activate the autolocator.
 * Returns position of next character
 */
static char *AutolocatorInit(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	char *ptr;
	
	/* memory already allocated? */
	if (win->autolocator == NULL)
	{
		int maxlen = win->toolbar[TO_SEARCH].ob_spec.tedinfo->te_txtlen;
		ptr = g_new(char, maxlen);
		if (ptr == NULL)
		{
			form_alert(1, rs_string(DI_MEMORY_ERROR));
			return NULL;
		}
		win->autolocator = ptr;
		ptr[maxlen - 1] = 0;
		*ptr = 0;
	} else
	{
		/* to end of string */
		ptr = win->autolocator;
		while (*ptr)
			ptr++;
	}

	/* search box already active? */
	if (!doc->buttons.searchbox)
		doc->buttons.searchbox = TRUE;

	return ptr;
}

/*** ---------------------------------------------------------------------- ***/

static void autolocator_redraw(WINDOW_DATA *win, _WORD obj, GRECT *tbar)
{
	GRECT r;
	_WORD ret;
	
	wind_update(BEG_UPDATE);
	ret = wind_get_grect(win->whandle, WF_FIRSTXYWH, &r);
	while (ret != 0 && r.g_w > 0 && r.g_h > 0)
	{
		if (rc_intersect(tbar, &r))
			objc_draw_grect(win->toolbar, obj, MAX_DEPTH, &r);
		ret = wind_get_grect(win->whandle, WF_NEXTXYWH, &r);
	}
	wind_update(END_UPDATE);
}

/*** ---------------------------------------------------------------------- ***/

/* Update the autolocator and start a search */
static void AutolocatorUpdate(WINDOW_DATA *win, long start_line)
{
	DOCUMENT *doc = win->data;
	GRECT tbar;
	long line = start_line;

	if (!doc->buttons.searchbox)
		return;

	/* draw toolbar with new text */
	wind_get_grect(win->whandle, WF_WORKXYWH, &tbar);
	tbar.g_h = win->y_offset;

	win->toolbar[TO_STRNOTFOUND].ob_flags |= OF_HIDETREE;

	/* enlarge box to avoid leftovers from backspacing */
	win->toolbar[TO_SEARCHBOX].ob_width = tbar.g_w;

	autolocator_redraw(win, TO_BACKGRND, &tbar);

	/* if autolocator is not empty... */
	if (*win->autolocator)
	{
		graf_mouse(BUSY_BEE, NULL);
		line = doc->autolocProc(win, start_line, win->autolocator, FALSE, FALSE);
		graf_mouse(ARROW, NULL);
	}

	if (line >= 0)
	{
		long topline = hv_win_topline(win);
		if (line != topline)
		{
			win->docsize.y = line * win->y_raster;
			SendRedraw(win);
			SetWindowSlider(win);
		}
	} else
	{
		win->toolbar[TO_STRNOTFOUND].ob_flags &= ~OF_HIDETREE;
		Cconout(7);
		autolocator_redraw(win, TO_SEARCHBOX, &tbar);
	}
}

/*** ---------------------------------------------------------------------- ***/

/* add a new character to the Autolocator and start search */
gboolean AutolocatorKey(WINDOW_DATA *win, short kbstate, short ascii)
{
	DOCUMENT *doc = win->data;
	char *ptr;
	long line = hv_win_topline(win);

	if (!ascii)
		return FALSE;

	ptr = AutolocatorInit(win);
	doc->autolocator_dir = 1;
	if (ptr == NULL)
		return FALSE;
	
	if (ascii == 8)						/* Backspace */
	{
		if (ptr > win->autolocator)
			ptr--;
		*ptr = 0;
	} else if (ascii == 13)				/* Return */
	{
		if (kbstate & KbSHIFT)
		{
			doc->autolocator_dir = 0;
			line--;
		} else
		{
			line++;
		}
	} else if (ascii == 27)				/* Escape */
	{
		if (ptr > win->autolocator)
		{
			ptr = win->autolocator;
			*ptr = 0;
		} else
		{
			RemoveSearchBox(win);
		}
	} else if (ascii == ' ')
	{
		/* ignore space at start of string */
		if (ptr != win->autolocator)
			*ptr++ = ' ';
		*ptr = 0;
	} else if (ascii > ' ')
	{
		int maxlen = win->toolbar[TO_SEARCH].ob_spec.tedinfo->te_txtlen;
		if (ptr - win->autolocator < (maxlen - 1))
		{
			*ptr++ = ascii;
			*ptr = 0;
		} else
		{
			--ptr;
			*ptr = ascii;
		}
	}

	ToolbarUpdate(win, FALSE);
	AutolocatorUpdate(win, line);

	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/* insert contents of clipboard in autolocator. */
void AutoLocatorPaste(WINDOW_DATA *win)
{
	int ret;
	char *scrap_file;

	if ((scrap_file = GetScrapPath(FALSE)) == NULL)
	{
		HYP_DBG(("No clipboard defined"));
		return;
	}

	ret = open(scrap_file, O_RDONLY | O_BINARY);
	g_free(scrap_file);
	if (ret >= 0)
	{
		ssize_t error;
		char c, *ptr;

		ptr = AutolocatorInit(win);
		if (ptr != NULL)
		{
			error = read(ret, &c, 1);
			while (error == 1)
			{
				if (c == ' ')
				{
					/* ignore spaces at start */
					if (ptr != win->autolocator)
						*ptr++ = c;
					*ptr = 0;
				} else if (c == 0x0d || c == 0x0a)
				{
					break;
				} else if (c > ' ')
				{
					int maxlen = win->toolbar[TO_SEARCH].ob_spec.tedinfo->te_txtlen;
					if ((ptr - win->autolocator) >= (maxlen - 1))
						break;
					*ptr++ = c;
					*ptr = 0;
				}
				error = read(ret, &c, 1);
			}
		}
		close(ret);

		ToolbarUpdate(win, FALSE);
		AutolocatorUpdate(win, hv_win_topline(win));
	}
}
