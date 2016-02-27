#include "hv_defs.h"
#include "resource.rh"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void toolbar_redraw(WINDOW_DATA *win)
{
	if (win->toolbar)
		InvalidateRect(win->toolbar, NULL, TRUE);
}

/*** ---------------------------------------------------------------------- ***/

void EnableMenuObj(HMENU menu, int obj, gboolean enable)
{
	EnableMenuItem(menu, obj, MF_BYCOMMAND | (enable ? MF_ENABLED : (MF_DISABLED|MF_GRAYED)));
}

/*** ---------------------------------------------------------------------- ***/

void CheckMenuObj(WINDOW_DATA *win, int obj, gboolean check)
{
	HMENU menu = GetMenu(win->hwnd);
	CheckMenuItem(menu, obj, MF_BYCOMMAND | (check ? MF_CHECKED : MF_UNCHECKED));
}

/*** ---------------------------------------------------------------------- ***/

static char *get_search_str(WINDOW_DATA *win)
{
	wchar_t str[256];
	int len;
	
	if ((len = GetWindowTextW(win->searchentry, str, 256)) <= 0)
		return NULL;
	return hyp_wchar_to_utf8(str, len);
}

/*** ---------------------------------------------------------------------- ***/

void ToolbarUpdate(WINDOW_DATA *win, gboolean redraw)
{
	DOCUMENT *doc = win->data;
	
	/* autolocator active? */
	if (doc->buttons.searchbox && win->searchbox)
	{
		char *search = get_search_str(win);
		if (!empty(search))
		{
			g_free(search);
			ShowWindow(win->searchbox, SW_SHOW);
			return;
		}
	}
	if (win->searchbox)
		ShowWindow(win->searchbox, SW_HIDE);
	
	doc->buttons.back = TRUE;
	doc->buttons.history = TRUE;
	doc->buttons.memory = TRUE;
	doc->buttons.menu = TRUE;
	doc->buttons.info = TRUE;
	doc->buttons.save = TRUE;
	doc->buttons.remarker_running = StartRemarker(win, remarker_check, TRUE) >= 0;
	doc->buttons.remarker = TRUE;
	
	if (win->history == NULL)
	{
		doc->buttons.back = FALSE;
		doc->buttons.history = FALSE;
	}
	
	if (!win->is_popup)
	{
		hv_update_menu(win);
		EnableWindow(win->m_buttons[TO_REMARKER], doc->buttons.remarker);
		EnableWindow(win->m_buttons[TO_MEMORY], doc->buttons.memory);
		EnableWindow(win->m_buttons[TO_INFO], doc->buttons.info);
		EnableWindow(win->m_buttons[TO_BACK], doc->buttons.back);
		EnableWindow(win->m_buttons[TO_HISTORY], doc->buttons.history);

		EnableWindow(win->m_buttons[TO_KATALOG], !empty(gl_profile.viewer.catalog_file));

		/* next buttons are type specific */
		EnableWindow(win->m_buttons[TO_PREV], doc->buttons.previous);
		EnableWindow(win->m_buttons[TO_PREV_PHYS], doc->buttons.prevphys);
		EnableWindow(win->m_buttons[TO_HOME], doc->buttons.home);
		EnableWindow(win->m_buttons[TO_NEXT], doc->buttons.next);
		EnableWindow(win->m_buttons[TO_NEXT_PHYS], doc->buttons.nextphys);
		EnableWindow(win->m_buttons[TO_FIRST], doc->buttons.first);
		EnableWindow(win->m_buttons[TO_LAST], doc->buttons.last);
		EnableWindow(win->m_buttons[TO_INDEX], doc->buttons.index);
		EnableWindow(win->m_buttons[TO_HELP], doc->buttons.help);
		EnableWindow(win->m_buttons[TO_REFERENCES], doc->buttons.references);
	}
	if (win->m_buttons[TO_REMARKER])
	{
		if (doc->buttons.remarker_running)
			ShowWindow(win->m_buttons[TO_REMARKER], SW_SHOW);
		else
			ShowWindow(win->m_buttons[TO_REMARKER], SW_HIDE);
	}
	
	if (redraw)
	{
		toolbar_redraw(win);
	}	
}

/*** ---------------------------------------------------------------------- ***/

void position_popup(HMENU menu, struct popup_pos *pos, int *xret, int *yret)
{
	WINDOW_DATA *win = pos->window;
	RECT a, r;
	
	UNUSED(menu);
	GetClientRect(win->m_buttons[pos->obj], &a);
	GetWindowRect(win->m_buttons[pos->obj], &r);
	r.top += a.bottom - a.top;
#if _WIN32_WINNT >= 0x601
	{
		SIZE s;
		POINT p;
		s.cx = a.right - a.left;
		s.cy = a.bottom - a.top;
		p.x = r.left;
		p.y = r.top;
		CalculatePopupWindowPosition(&p, &s, TPM_LEFTALIGN, NULL, &r);
	}
#endif
	*xret = r.left;
	*yret = r.top;
}

/*** ---------------------------------------------------------------------- ***/

/* Handle mouse click on toolbar */
void ToolbarClick(WINDOW_DATA *win, enum toolbutton obj, int button)
{
	if ((int)obj < 0)
		RemoveSearchBox(win);
	else if (!win->m_buttons[obj] || !IsWindowEnabled(win->m_buttons[obj]))
		return;

	CheckFiledate(win);		/* Check if file has changed */
	
	switch (obj)
	{
	case TO_LOAD:
		SelectFileLoad(win);
		break;
	case TO_SAVE:
		SelectFileSave(win);
		break;
	case TO_INDEX:
		GotoIndex(win);
		break;
	case TO_KATALOG:
		GotoCatalog(win);
		break;
	case TO_REFERENCES:
		HypExtRefPopup(win, button);
		break;
	case TO_HELP:
		GotoHelp(win);
		break;
	case TO_HISTORY:
		HistoryPopup(win, button);
		break;
	case TO_BACK:
		GoBack(win);
		break;
	case TO_NEXT:
	case TO_PREV:
	case TO_NEXT_PHYS:
	case TO_PREV_PHYS:
	case TO_FIRST:
	case TO_LAST:
	case TO_HOME:
		GoThisButton(win, obj);
		break;
	case TO_MEMORY:
		MarkerPopup(win, button);
		break;
	case TO_INFO:
		DocumentInfos(win);
		break;
	case TO_REMARKER:
		BlockOperation(win, CO_REMARKER);
		break;
	default:
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void RemoveSearchBox(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;

	/* Is the autolocator/search box displayed? */
	if (doc->buttons.searchbox && win->searchentry)
	{
		doc->buttons.searchbox = FALSE;	/* disable it */
		SetWindowTextW(win->searchentry, L"");			/* clear autolocator string */

		ToolbarUpdate(win, TRUE);	/* update toolbar */
	}
}
