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
	
	if (!doc->buttons.searchbox)
		return;
	/* NYI: autolocator paste */
}
