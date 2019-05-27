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
#include "windebug.h"
#include "resource.rh"

#define IsModifierKey(keycode) \
	(keycode == VK_LSHIFT || \
	 keycode == VK_RSHIFT || \
	 keycode == VK_LCONTROL || \
	 keycode == VK_RCONTROL || \
	 keycode == VK_LMENU || \
	 keycode == VK_RMENU || \
	 keycode == VK_NUMLOCK || \
	 keycode == VK_SCROLL)

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * Initialize and activate the autolocator.
 * Returns position of next character
 */
static void AutolocatorInit(DOCUMENT *doc)
{
	/* search box already active? */
	if (!doc->buttons.searchbox)
		doc->buttons.searchbox = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/* Update the autolocator and start a search */
static void AutolocatorUpdate(WINDOW_DATA *win, long start_line)
{
	DOCUMENT *doc = win->data;
	long line = start_line;
	char *search;
	
	if (!doc->buttons.searchbox)
		return;

	ShowWindow(GetDlgItem(win->searchbox, IDC_SEARCH_NOTFOUND), SW_HIDE);

	/* if autolocator is not empty... */
	search = DlgGetText(win->searchbox, IDC_SEARCH_ENTRY);
	if (!empty(search))
	{
		line = doc->autolocProc(win, start_line, search, FALSE, FALSE);
	}
	g_free(search);
	
	if (line >= 0)
	{
		long topline = hv_win_topline(win);
		if (line != topline)
		{
			hv_win_scroll_to_line(win, line);
		}
	} else
	{
		ShowWindow(GetDlgItem(win->searchbox, IDC_SEARCH_NOTFOUND), SW_SHOW);
		MessageBeep(MB_ICONWARNING);
	}
}

/*** ---------------------------------------------------------------------- ***/

/* add a new character to the Autolocator and start search */
gboolean AutolocatorKey(WINDOW_DATA *win, unsigned int message, WPARAM wparam, LPARAM lparam)
{
	unsigned int keycode;
	DOCUMENT *doc = win->data;
	long line = hv_win_topline(win);
	wchar_t *wstr;
	char *str;
	
	keycode = MapVirtualKey((lparam >> 16) & 0xff, MAPVK_VSC_TO_VK_EX);
	if (IsModifierKey(keycode))
		return FALSE;

	AutolocatorInit(doc);
	doc->autolocator_dir = 1;
	
	switch (message)
	{
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
		switch (keycode)
		{
		case VK_BACK:
			wstr = DlgGetTextW(win->searchbox, IDC_SEARCH_ENTRY);
			if (wstr && *wstr)
			{
				wstr[wcslen(wstr) - 1] = L'\0';
				DlgSetTextW(win->searchbox, IDC_SEARCH_ENTRY, wstr);
			}
			g_free(wstr);
			break;
		case VK_RETURN:
			if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
			{
				doc->autolocator_dir = 0;
				line--;
			} else
			{
				line++;
			}
			break;
		case VK_ESCAPE:
			wstr = DlgGetTextW(win->searchbox, IDC_SEARCH_ENTRY);
			if (wstr && *wstr)
			{
				DlgSetTextW(win->searchbox, IDC_SEARCH_ENTRY, L"");
			} else
			{
				RemoveSearchBox(win);
			}
			g_free(wstr);
			break;
		case VK_SPACE:
			/* ignore space at start of string */
			str = DlgGetText(win->searchbox, IDC_SEARCH_ENTRY);
			if (str && *str)
			{
				char *newtxt = g_strconcat(str, " ", NULL);
				DlgSetText(win->searchbox, IDC_SEARCH_ENTRY, newtxt);
				g_free(newtxt);
			}
			g_free(str);
			break;
		}
		break;
	case WM_CHAR:
	case WM_UNICHAR:
		if (wparam >= ' ')
		{
			char *newtxt;
			char buf[HYP_UTF8_CHARMAX + 1];
			
			str = DlgGetText(win->searchbox, IDC_SEARCH_ENTRY);
			hyp_unichar_to_utf8(buf, wparam);
			newtxt = g_strconcat(str, buf, NULL);
			DlgSetText(win->searchbox, IDC_SEARCH_ENTRY, newtxt);
			g_free(newtxt);
			g_free(str);
		}
		break;
	}

	ToolbarUpdate(win, FALSE);
	AutolocatorUpdate(win, line);

	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/* insert contents of clipboard in autolocator. */
void AutoLocatorPaste(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	HANDLE hClipData;
	wchar_t *Text;
	long line = hv_win_topline(win);
	
	if (!doc->buttons.searchbox)
		return;

	if (OpenClipboard(win->hwnd))
	{
		hClipData = GetClipboardData(CF_UNICODETEXT);
		if (hClipData != NULL)
		{
			if ((Text = (wchar_t *)GlobalLock(hClipData)) != NULL)
			{
				size_t len = GlobalSize(hClipData) / sizeof(wchar_t);
				char *txt;
				char *p;
				char *newtxt;
				char *str;
				
				if (len > 0 && Text[len - 1] == 0)
					len--;
				txt = hyp_wchar_to_utf8(Text, len);

				p = strchr(txt, 0x0d);
				if (p == NULL)
					p = strchr(txt, 0x0a);
				if (p != NULL)
					*p = '\0';
				
				GlobalUnlock(hClipData);
				
				str = DlgGetText(win->searchbox, IDC_SEARCH_ENTRY);
				/* ignore spaces at start */
				if (empty(str))
					g_strchug(txt);
				newtxt = g_strconcat(str, txt, NULL);
				DlgSetText(win->searchbox, IDC_SEARCH_ENTRY, newtxt);

				g_free(newtxt);
				g_free(str);
				g_free(txt);

				AutolocatorInit(doc);
				doc->autolocator_dir = 1;
				ToolbarUpdate(win, TRUE);
				AutolocatorUpdate(win, line);
			}
		}
		CloseClipboard();
	}
}
