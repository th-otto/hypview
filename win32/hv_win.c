/*
 * HypView - (c) 2019 - 2020 Thorsten Otto
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
#include "w_draw.h"
#include <commctrl.h>
#include "windows.rh"

#ifndef TVN_ITEMCHANGING
  typedef struct tagTVITEMCHANGE {
    NMHDR hdr;
    UINT uChanged;
    HTREEITEM hItem;
    UINT uStateNew;
    UINT uStateOld;
    LPARAM lParam;
  } NMTVITEMCHANGE;
#endif


static char *default_geometry;

static TOOLBAR_ENTRY const tb_entries[] = {
	[ TO_BACK] = { TO_BACK, IDM_NAV_BACK, IDI_BACK, N_("Back one page") },
	[ TO_HISTORY ] = { TO_HISTORY, IDM_NAV_HISTORYMENU, IDI_HISTORY, N_("Show history of pages") },
	[ TO_BOOKMARKS ] = { TO_BOOKMARKS, IDM_NAV_BOOKMARKSMENU, IDI_BOOKMARKS, N_("Show list of bookmarks") },
	[ TO_FIRST ] = { TO_FIRST, IDM_NAV_FIRST, IDI_FIRST, N_("Goto first page") },
	[ TO_PREV_PHYS ] = { TO_PREV_PHYS, IDM_NAV_PREVPHYS, IDI_PREVPHYS, N_("Goto previous physical page") },
	[ TO_PREV ] = { TO_PREV, IDM_NAV_PREV, IDI_PREV, N_("Goto previous page") },
	[ TO_HOME ] = { TO_HOME, IDM_NAV_TOC, IDI_HOME, N_("Go up one page") },
	[ TO_NEXT ] = { TO_NEXT, IDM_NAV_NEXT, IDI_NEXT, N_("Goto next page") },
	[ TO_NEXT_PHYS ] = { TO_NEXT_PHYS, IDM_NAV_NEXTPHYS, IDI_NEXTPHYS, N_("Goto next physical page") },
	[ TO_LAST ] = { TO_LAST, IDM_NAV_LAST, IDI_LAST, N_("Goto last page") },
	[ TO_INDEX ] = { TO_INDEX, IDM_NAV_INDEX, IDI_INDEX, N_("Goto index page") },
	[ TO_TREEVIEW ] = { TO_TREEVIEW, IDM_NAV_TREEVIEW, IDI_TREEVIEW, N_("Tree View") },
	[ TO_CATALOG ] = { TO_CATALOG, IDM_FILE_CATALOG, IDI_CATALOG, N_("Show catalog of hypertexts") },
	[ TO_REFERENCES ] = { TO_REFERENCES, IDM_NAV_XREF, IDI_REFERENCE, N_("Show list of cross references") },
	[ TO_HELP ] = { TO_HELP, IDM_NAV_HELP, IDI_HELP, N_("Show help page") },
	[ TO_INFO ] = { TO_INFO, IDM_FILE_INFO, IDI_INFO, N_("Show info about hypertext") },
	[ TO_LOAD ] = { TO_LOAD, IDM_FILE_OPEN, IDI_LOAD, N_("Load a file") },
	[ TO_SAVE ] = { TO_SAVE, IDM_FILE_SAVE, IDI_SAVE, N_("Save page to file") },
	/* [ TO_MENU ] = { TO_MENU, 0, IDI_MENU, NULL }, */
	[ TO_REMARKER ] = { TO_REMARKER, IDM_FILE_REMARKER, IDI_REMARKER, N_("Start Remarker") },
};

/*
 * note: values from this table are used as indices into the table above;
 * using the ids here only works when tb_entries[i].config_id == i
 */
static int const tb_defs[] = {
#define indexof(i) i
	indexof(TO_BACK), indexof(TO_HISTORY), indexof(TO_BOOKMARKS), TB_SEPARATOR,
	indexof(TO_FIRST), indexof(TO_PREV_PHYS), indexof(TO_PREV), indexof(TO_HOME), indexof(TO_NEXT), indexof(TO_NEXT_PHYS), indexof(TO_LAST), TB_SEPARATOR,
	indexof(TO_INDEX), indexof(TO_TREEVIEW), indexof(TO_CATALOG), indexof(TO_REFERENCES), indexof(TO_HELP), TB_SEPARATOR,
	indexof(TO_INFO), indexof(TO_LOAD), indexof(TO_SAVE), TB_SEPARATOR,
	indexof(TO_REMARKER), TB_ENDMARK
#undef indexof
};

#define K_RSHIFT        0x0001
#define K_LSHIFT        0x0002
#define K_SHIFT			(K_LSHIFT|K_RSHIFT)
#define K_CTRL          0x0004
#define K_ALT           0x0008
#define K_CAPSLOCK		0x0010

#define set_tooltip_text(hwnd, txt) /* NYI */

static HCURSOR regular_cursor;
static HCURSOR hand_cursor;

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

void hv_win_set_geometry(const char *geometry)
{
	g_free(default_geometry);
	default_geometry = g_strdup(geometry);
}

/*** ---------------------------------------------------------------------- ***/

void hv_win_open(WINDOW_DATA *win)
{
	ShowWindow(win->hwnd, SW_SHOW);
	BringWindowToTop(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

void WindowCalcScroll(WINDOW_DATA *win)
{
	RECT r;
	GRECT tgr, sgr;
	int th = 0;
	int sh = 0;
	int sb = 0;
	TOOL_DATA *td;
	STATUS_DATA *sd;
	
	GetClientRect(win->hwnd, &r);
	td = win->td;
	sd = win->sd;
	
	if (td && td->hwnd)
	{
		if (td->visible)
		{
			if (!IsWindowVisible(td->hwnd))
				ShowWindow(td->hwnd, SW_SHOW);
			RectToGrect(&tgr, &r);
			th = td->toolbar_size(td, &tgr);
		} else
		{
			if (IsWindowVisible(td->hwnd))
				ShowWindow(td->hwnd, SW_HIDE);
		}
	}
	
	if (win->searchbox && IsWindowVisible(win->searchbox))
	{
		RECT s;
		GetClientRect(win->searchbox, &s);
		sb = s.bottom - s.top;
	}
	
	if (sd != NULL && sd->hwnd != 0)
	{
		if (sd->visible)
		{
			if (!IsWindowVisible(sd->hwnd))
				ShowWindow(sd->hwnd, SW_SHOW);
		} else
		{
			if (IsWindowVisible(sd->hwnd))
				ShowWindow(sd->hwnd, SW_HIDE);
		}
		RectToGrect(&sgr, &r);
		sh = sd->statusbar_size(sd, &sgr);
	}

	if (td && td->hwnd)
	{
		if (td->ontop)
		{
			MoveWindow(td->hwnd, r.left, r.top, r.right - r.left, th, TRUE);
			r.top += th;
		} else
		{
			r.bottom -= th;
			MoveWindow(td->hwnd, r.left, r.bottom, r.right - r.left, th, TRUE);
		}
	}

	if (win->searchbox && IsWindowVisible(win->searchbox))
	{
		MoveWindow(win->searchbox, r.left, r.top, r.right - r.left, sb, TRUE);
		r.top += sb;
	}
	
	if (sd != NULL && sd->hwnd != 0)
	{
		if (sd->ontop)
		{
			MoveWindow(sd->hwnd, r.left, r.top, r.right - r.left, sh, TRUE);
			r.top += sh;
		} else
		{
			r.bottom -= sh;
			MoveWindow(sd->hwnd, r.left, r.bottom, r.right - r.left, sh, TRUE);
		}
	}

	if (win->textwin)
	{
		MoveWindow(win->textwin, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
		GetClientRect(win->textwin, &r);
		win->scroll.g_x = r.left + win->x_margin_left;
		win->scroll.g_y = r.top + win->y_margin_top;
		win->scroll.g_w = (r.right - r.left) - win->x_margin_left - win->x_margin_right;
		win->scroll.g_h = (r.bottom - r.top) - win->y_margin_top - win->y_margin_bottom;
	}

	if (win->treewin)
	{
		MoveWindow(win->treewin, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
		GetClientRect(win->treewin, &r);
		win->scroll.g_x = r.left + win->x_margin_left;
		win->scroll.g_y = r.top + win->y_margin_top;
		win->scroll.g_w = (r.right - r.left) - win->x_margin_left - win->x_margin_right;
		win->scroll.g_h = (r.bottom - r.top) - win->y_margin_top - win->y_margin_bottom;
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void toolbar_status_exit(WINDOW_DATA *win)
{
	STATUS_DATA *sd;
	TOOL_DATA *td;

	if ((td = win->td) != NULL)
	{
		td->toolbar_close(win);
		td->toolbar_exit(win);
	}
	if ((sd = win->sd) != NULL)
	{
		sd->statusbar_close(win);
		sd->statusbar_exit(win);
	}
}

/*** ---------------------------------------------------------------------- ***/

static void win32_hypview_window_finalize(WINDOW_DATA *win)
{
	DOCUMENT *doc;
	
	if (win != NULL)
	{
		WINDOW_DATA *tree;

		doc = win->data;
		if (win->popup)
		{
			WINDOW_DATA *pop = win->popup;
			win->popup = NULL;
			DestroyWindow(pop->hwnd);
		}
		if (win->rscfile)
		{
			HWND rscfile = win->rscfile;
			win->rscfile = NO_WINDOW;
			DestroyWindow(rscfile);
		}
		if ((tree = HaveTreeview(win)) != NULL)
		{
			win->treeview_window_id = 0;
			DestroyWindow(tree->hwnd);
		}
		if (win->parentwin)
			win->parentwin->popup = NULL;
		toolbar_status_exit(win);
		hypdoc_unref(doc);
		win->data = NULL;
		if (!win->is_popup)
		{
			RemoveAllHistoryEntries(win);
			all_list = g_slist_remove(all_list, win);
		}
		if (win->cliprgn != NULL)
			DeleteObject(win->cliprgn);
		g_freep(&win->title);
		g_free(win);
		check_toplevels(NULL);
	}
}

/*** ---------------------------------------------------------------------- ***/

static void on_quit(WINDOW_DATA *win)
{
	GSList *l;
	
	for (l = all_list; l; l = l->next)
	{
		win = (WINDOW_DATA *)l->data;
		SendCloseWindow(win);
	}
}

/*** ---------------------------------------------------------------------- ***/

static gboolean NOINLINE WriteProfile(WINDOW_DATA *win)
{
	gboolean ret;
	Profile *inifile;
	
	inifile = gl_profile.profile;
	
	ret = HypProfile_Save(TRUE);
	
	if (ret == FALSE)
	{
		char *msg = g_strdup_printf(_("Can't write Settings:\n%s\n%s\nQuit anyway?"), Profile_GetFilename(inifile), hyp_utf8_strerror(errno));
		ret = ask_yesno(win ? win->hwnd : 0, msg);
		g_free(msg);
	}
	return ret;
}

/*** ---------------------------------------------------------------------- ***/

gboolean hv_scroll_window(WINDOW_DATA *win, long xamount, long yamount)
{
	WP_UNIT old_x, old_y;
	
	if (!win->textwin)
		return FALSE;
	old_x = win->docsize.x;
	old_y = win->docsize.y;
	
	win->docsize.y += yamount;
	if (win->docsize.y > (win->docsize.h - win->scroll.g_h))
		win->docsize.y = (win->docsize.h - win->scroll.g_h);
	if (win->docsize.y < 0)
		win->docsize.y = 0;
	win->docsize.y = (win->docsize.y / win->y_raster) * win->y_raster;
	
	win->docsize.x += xamount;
	if (win->docsize.x > (win->docsize.w - win->scroll.g_w))
		win->docsize.x = (win->docsize.w - win->scroll.g_w);
	if (win->docsize.x < 0)
		win->docsize.x = 0;
	win->docsize.x = (win->docsize.x / win->x_raster) * win->x_raster;
	
	if (win->docsize.x == old_x && win->docsize.y == old_y)
		return FALSE;
	SetWindowSlider(win);
	SendRedraw(win);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static int getkeystate(void)
{
	int keystate = 0;

	if (GetKeyState(VK_MENU) & 0x8000)
		keystate |= K_ALT;
	if (GetKeyState(VK_CONTROL) & 0x8000)
		keystate |= K_CTRL;
	if (GetKeyState(VK_LSHIFT) & 0x8000)
		keystate |= K_LSHIFT;
	if (GetKeyState(VK_RSHIFT) & 0x8000)
		keystate |= K_RSHIFT;
	if (GetKeyState(VK_CAPITAL) & 0x8000)
		keystate |= K_CAPSLOCK;
	return keystate;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean key_press_event(WINDOW_DATA *win, unsigned int message, WPARAM wParam, LPARAM lParam)
{
	unsigned int keycode;
	unsigned int keystate;
	gboolean handled = FALSE;
	DOCUMENT *doc = win->data;
	
	keystate = getkeystate();
	switch (message)
	{
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
		keycode = MapVirtualKey((lParam >> 16) & 0xff, MAPVK_VSC_TO_VK_EX);
		if (IsModifierKey(keycode))
			return FALSE;
		
		if (win->is_popup)
		{
			DestroyWindow(win->hwnd);
			return TRUE;
		}
		if (win->popup)
		{
			DestroyWindow(win->popup->hwnd);
			return TRUE;
		}
		handled = TRUE;
		switch (keycode)
		{
		case VK_RETURN:
			/* NYI: find position of selected link */
			break;
		case VK_LEFT:
			if ((keystate & K_SHIFT) && (keystate & K_CTRL))
				GoThisButton(win, TO_PREV);
			else if (keystate & K_SHIFT)
				hv_scroll_window(win, -win->scroll.g_w, 0);
			else if (keystate & K_CTRL)
				GoThisButton(win, TO_PREV);
			else
				hv_scroll_window(win, -win->x_raster, 0);
			break;
		case VK_RIGHT:
			if ((keystate & K_SHIFT) && (keystate & K_CTRL))
				GoThisButton(win, TO_NEXT);
			else if (keystate & K_SHIFT)
				hv_scroll_window(win, win->scroll.g_w, 0);
			else if (keystate & K_CTRL)
				GoThisButton(win, TO_NEXT);
			else
				hv_scroll_window(win, win->x_raster, 0);
			break;
		case VK_UP:
			if ((keystate & K_SHIFT) && (keystate & K_CTRL))
				GoThisButton(win, TO_PREV);
			else if (keystate & K_SHIFT)
				hv_scroll_window(win, 0, -win->scroll.g_h);
			else if (keystate & K_CTRL)
				hv_scroll_window(win, 0, -win->scroll.g_h);
			else
				hv_scroll_window(win, 0, -win->y_raster);
			break;
		case VK_DOWN:
			if ((keystate & K_SHIFT) && (keystate & K_CTRL))
				GoThisButton(win, TO_NEXT);
			else if (keystate & K_SHIFT)
				hv_scroll_window(win, 0, win->scroll.g_h);
			else if (keystate & K_CTRL)
				hv_scroll_window(win, 0, win->scroll.g_h);
			else
				hv_scroll_window(win, 0, win->y_raster);
			break;
		case VK_PRIOR:
			hv_scroll_window(win, 0, -win->scroll.g_h);
			break;
		case VK_NEXT:
			hv_scroll_window(win, 0, win->scroll.g_h);
			break;
		case VK_HOME:
			if ((keystate & K_SHIFT) || (keystate & K_CTRL))
				hv_scroll_window(win, -INT_MAX, -INT_MAX);
			else
				hv_scroll_window(win, -INT_MAX, 0);
			break;
		case VK_END:
			if ((keystate & K_SHIFT) || (keystate & K_CTRL))
				hv_scroll_window(win, -INT_MAX, INT_MAX);
			else
				hv_scroll_window(win, INT_MAX, 0);
			break;
		case VK_SUBTRACT:
			GoThisButton(win, TO_PREV);
			break;
		case VK_ADD:
			GoThisButton(win, TO_NEXT);
			break;
		case VK_DIVIDE:
			GoThisButton(win, TO_PREV_PHYS);
			break;
		case VK_MULTIPLY:
			GoThisButton(win, TO_NEXT_PHYS);
			break;
		case VK_HELP:
			GotoHelp(win);
			break;
		case VK_ESCAPE:
		case VK_BACK:
			if (!win->searchentry || !(doc->buttons.searchbox))
				GoThisButton(win, TO_BACK);
			else
				handled = FALSE;
			break;
		case VK_F1:				/* already handled by accelerators */
		case VK_F2:				/* already handled by accelerators */
		case VK_F3:				/* already handled by accelerators */
		case VK_F4:				/* already handled by accelerators */
		case VK_F5:				/* already handled by accelerators */
		case VK_F6:				/* already handled by accelerators */
		case VK_F7:				/* already handled by accelerators */
		case VK_F8:				/* already handled by accelerators */
		case VK_F9:				/* already handled by accelerators */
		case VK_F10:			/* already handled by accelerators */
		case VK_F11:			/* already handled by accelerators */
		case VK_F12:			/* already handled by accelerators */
		default:
			handled = FALSE;
			break;
		}
		break;
	}
	if (!handled && win->searchentry)
		handled = AutolocatorKey(win, message, wParam, lParam);
	return handled;
}

/*** ---------------------------------------------------------------------- ***/

static void SetWindowCursor(HWND hwnd, HCURSOR cursor)
{
	SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)cursor);
	SetCursor(cursor);
}

/*** ---------------------------------------------------------------------- ***/

static INT_PTR CALLBACK search_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNUSED(hwnd);
	switch (message)
	{
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOREDIT:
		if (GetDlgCtrlID((HWND)lParam) == IDC_SEARCH_NOTFOUND)
		{
			HDC hdcStatic = (HDC) wParam;
			SetTextColor(hdcStatic, RGB(255, 0, 0));
			SetBkColor(hdcStatic, RGB(0,0,0));
			SetBkMode(hdcStatic, TRANSPARENT);
			return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
		}
		break;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean hv_commdlg_help(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNUSED(hwnd);
	UNUSED(wParam);
	if (message == commdlg_help)
	{
		DWORD *p = (DWORD *)lParam;
		
		if (p == NULL)
		{
			HYP_DBG(("HELP %x %lx", wParam, lParam));
		} else if (*p == sizeof(CHOOSECOLORW))
		{
			Help_Show(NULL, "colorselector");
		} else if (*p == sizeof(CHOOSEFONTW))
		{
			Help_Show(NULL, "fontselector");
		} else if (*p == sizeof(FINDREPLACEW))
		{
			Help_Show(NULL, "findselector");
		} else if (*p == sizeof(OPENFILENAMEW) || *p == OPENFILENAME_SIZE_VERSION_400)
		{
			Help_Show(NULL, "fileselector");
		} else if (*p == sizeof(PAGESETUPDLGW))
		{
			Help_Show(NULL, "pagesetupselector");
		} else if (*p == sizeof(PRINTDLGW))
		{
			Help_Show(NULL, "printerselector");
		} else
		{
			HYP_DBG(("HELP %x %lx: %ld", wParam, lParam, *p));
		}
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean hv_show_item(WINDOW_DATA *win, HTREEITEM hItem)
{
	if (hItem)
	{
		TVITEMW item;
		LINK_INFO linkinfo;
		HYP_DOCUMENT *hyp;
		INDEX_ENTRY *entry;
		WINDOW_DATA *target;
		
		item.hItem = hItem;
		item.mask = TVIF_HANDLE;
		if (SendMessage(win->treewin, TVM_GETITEMW, 0, (LPARAM)&item))
		{
			linkinfo.dest_page = item.lParam;
			target = hv_link_targetwin(win, win->treeview_parent);
			if (target == NULL)
				return FALSE;
			hyp = (HYP_DOCUMENT *)target->data->data;
			entry = hyp->indextable[linkinfo.dest_page];
			linkinfo.link_type = HYP_ESC_LINK_LINE;
			linkinfo.dst_type = (hyp_indextype) entry->type;
			linkinfo.tip = NULL;
			linkinfo.line_nr = 0;
			linkinfo.window_id = win->treeview_parent;
			HypClick(target, &linkinfo);
			return TRUE;
		}
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static LRESULT CALLBACK mainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WINDOW_DATA *win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	
	win32debug_msg_print(stderr, "mainWndProc", hwnd, message, wParam, lParam);
	switch (message)
	{
	case WM_NCCREATE:
		win = (WINDOW_DATA *)(((CREATESTRUCT *)lParam)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (DWORD_PTR)win);
		win->hwnd = hwnd;
		{
			HINSTANCE inst = GetInstance();
			win->searchbox = CreateDialogIndirect(inst, LoadDialog(inst, MAKEINTRESOURCEW(IDD_SEARCHBOX)), hwnd, search_dialog);
			win->searchentry = GetDlgItem(win->searchbox, IDC_SEARCH_ENTRY);
		}
		win->scrollvsize = -1;
		win->scrollhsize = -1;
		break;

	case WM_CREATE:
		hv_update_winmenu(win);
		break;
	
	case WM_CLOSE:
		if (toplevels_open_except(win) == 0)
		{
			RecentSaveToDisk();
			if (WriteProfile(win) == FALSE)
				return FALSE;
		}
		DestroyWindow(win->hwnd);
		win32debug_msg_end("mainWndProc", message, "TRUE");
		return TRUE;

	case WM_DESTROY:
		win32_hypview_window_finalize(win);
		win32debug_msg_end("mainWndProc", message, "FALSE");
		return FALSE;
	
	case WM_WINDOWPOSCHANGED:
		{
			RECT r;
			
			if (!win->is_popup)
			{
				GetWindowRect(hwnd, &r);
				gl_profile.viewer.win_x = r.left;
				gl_profile.viewer.win_y = r.top;
				gl_profile.viewer.win_w = r.right - r.left;
				gl_profile.viewer.win_h = r.bottom - r.top;
				HypProfile_SetChanged();
				WindowCalcScroll(win);
				win->scrollvsize = -1;
				win->scrollhsize = -1;
				SetWindowSlider(win);
			}
		}
		win32debug_msg_end("mainWndProc", message, "FALSE");
		return FALSE;
	
	case WM_INITMENUPOPUP:
		{
			HMENU submenu = (HMENU)wParam;
			
			if (submenu == win->bookmarks_menu)
				MarkerUpdate(win);
			else if (submenu == win->recent_menu)
				RecentUpdate(win);
		}
		break;
		
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_FILE_OPEN:
			SelectFileLoad(win);
			break;
		case IDM_FILE_SAVE:
			{
				char *filename;
				gboolean selection_only = FALSE;
				DOCUMENT *doc = win->data;
				
				if (win->selection.valid)
					selection_only = TRUE;
				filename = SelectFileSave(win, HYP_FT_ASCII);
				if (filename)
				{
					if (doc->type == HYP_FT_HYP && !selection_only)
						hv_recompile((HYP_DOCUMENT *)doc->data, filename, HYP_FT_ASCII);
					else
						BlockAsciiSave(win, filename);
					g_free(filename);
				}
			}
			break;
		case IDM_FILE_RECOMPILE:
			{
				char *filename;
				DOCUMENT *doc = win->data;
				
				if (doc->type == HYP_FT_HYP)
				{
					filename = SelectFileSave(win, HYP_FT_STG);
					if (filename)
					{
						hv_recompile((HYP_DOCUMENT *)doc->data, filename, HYP_FT_STG);
						g_free(filename);
					}
				}
			}
			break;
#ifdef WITH_PDF
		case IDM_FILE_SAVEPDF:
			{
				char *filename;
				DOCUMENT *doc = win->data;
				
				if (doc->type == HYP_FT_HYP)
				{
					filename = SelectFileSave(win, HYP_FT_PDF);
					if (filename)
					{
						hv_recompile((HYP_DOCUMENT *)doc->data, filename, HYP_FT_PDF);
						g_free(filename);
					}
				}
			}
			break;
#endif
		case IDM_FILE_CATALOG:
			GoThisButton(win, TO_CATALOG);
			break;
		case IDM_FILE_DEFAULT:
			GotoDefaultFile(win);
			break;
		case IDM_FILE_REMARKER:
			BlockOperation(win, CO_REMARKER);
			break;
		case IDM_FILE_INFO:
			DocumentInfos(win);
			break;
		case IDM_FILE_CLOSE:
			SendCloseWindow(win);
			break;
		case IDM_FILE_QUIT:
			on_quit(win);
			break;
		case IDM_FILE_RECENTMENU:
			RecentUpdate(win);
			break;
		case IDM_FILE_RECENT_1:
		case IDM_FILE_RECENT_2:
		case IDM_FILE_RECENT_3:
		case IDM_FILE_RECENT_4:
		case IDM_FILE_RECENT_5:
		case IDM_FILE_RECENT_6:
		case IDM_FILE_RECENT_7:
		case IDM_FILE_RECENT_8:
		case IDM_FILE_RECENT_9:
		case IDM_FILE_RECENT_10:
			on_recent_selected(win, LOWORD(wParam) - IDM_FILE_RECENT_1);
			break;
		case IDM_EDIT_SELECTALL:
			BlockOperation(win, CO_SELECT_ALL);
			break;
		case IDM_EDIT_COPY:
			BlockOperation(win, CO_COPY);
			break;
		case IDM_EDIT_PASTE:
			BlockOperation(win, CO_PASTE);
			break;
		case IDM_EDIT_FIND:
			BlockOperation(win, CO_SEARCH);
			break;
		case IDM_EDIT_FINDNEXT:
			BlockOperation(win, CO_SEARCH_AGAIN);
			break;
		
		case IDM_NAV_PREV:
			GoThisButton(win, TO_PREV);
			break;
		case IDM_NAV_NEXT:
			GoThisButton(win, TO_NEXT);
			break;
		case IDM_NAV_PREVPHYS:
			GoThisButton(win, TO_PREV_PHYS);
			break;
		case IDM_NAV_NEXTPHYS:
			GoThisButton(win, TO_NEXT_PHYS);
			break;
		case IDM_NAV_FIRST:
			GoThisButton(win, TO_FIRST);
			break;
		case IDM_NAV_LAST:
			GoThisButton(win, TO_LAST);
			break;
		case IDM_NAV_TOC:
			GoThisButton(win, TO_HOME);
			break;
		case IDM_NAV_INDEX:
			GotoIndex(win);
			break;
		case IDM_NAV_TREEVIEW:
			ShowTreeview(win);
			break;
		case IDM_NAV_XREF:
			HypExtRefPopup(win, 1);
			break;
		case IDM_NAV_HELP:
			GotoHelp(win);
			break;
		case IDM_NAV_BACK:
			GoThisButton(win, TO_BACK);
			break;
		case IDM_NAV_CLEARSTACK:
			BlockOperation(win, CO_DELETE_STACK);
			break;
		case IDM_NAV_BOOKMARKSMENU:
			MarkerUpdate(win);
			MarkerPopup(win, 1);
			break;
		case IDM_NAV_BOOKMARK_1:
		case IDM_NAV_BOOKMARK_2:
		case IDM_NAV_BOOKMARK_3:
		case IDM_NAV_BOOKMARK_4:
		case IDM_NAV_BOOKMARK_5:
		case IDM_NAV_BOOKMARK_6:
		case IDM_NAV_BOOKMARK_7:
		case IDM_NAV_BOOKMARK_8:
		case IDM_NAV_BOOKMARK_9:
		case IDM_NAV_BOOKMARK_10:
		case IDM_NAV_BOOKMARK_11:
		case IDM_NAV_BOOKMARK_12:
			on_bookmark_selected(win, LOWORD(wParam) - IDM_NAV_BOOKMARK_1);
			break;
		case IDM_NAV_HISTORYMENU:
			HistoryPopup(win, TO_HISTORY, 1);
			break;
		
		case IDM_OPT_SELECTFONT:
			SelectFont(win);
			break;
		case IDM_OPT_SELECTCOLORS:
			hv_config_colors(win);
			break;
		case IDM_OPT_OUTPUT:
			hv_config_output(win);
			break;
		case IDM_OPT_ALTFONT:
			gl_profile.viewer.use_xfont = !gl_profile.viewer.use_xfont;
			HypProfile_SetChanged();
			SwitchFont(win, FALSE);
			break;
		case IDM_OPT_EXPANDSPACES:
			gl_profile.viewer.expand_spaces = !gl_profile.viewer.expand_spaces;
			HypProfile_SetChanged();
			hv_update_winmenu(win);
			{
				DOCUMENT *doc = win->data;
				if (doc && doc->prepNode)
				{
					doc->start_line = hv_win_topline(win);
					ReInitWindow(win, TRUE);
				}
			}
			break;
		case IDM_OPT_SCALEBITMAPS:
			gl_profile.viewer.scale_bitmaps = !gl_profile.viewer.scale_bitmaps;
			hv_update_menus();
			break;
		case IDM_OPT_PREFERENCES:
			hv_preferences(win);
			break;

		case IDM_HELP_CONTENTS:
			Help_Contents(win);
			break;
		case IDM_HELP_INDEX:
			Help_Index(win);
			break;
		case IDM_HELP_ABOUT:
			About(hwnd);
			break;
		}
		win32debug_msg_end("mainWndProc", message, "0");
		return 0;
	
	case WM_ERASEBKGND:
		win32debug_msg_end("mainWndProc", message, "FALSE");
		return FALSE;
	
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			GRECT gr;
			
			W_BeginPaint(hwnd, &ps, &gr);
			W_EndPaint(hwnd, &ps);
			win32debug_msg_end("mainWndProc", message, "TRUE");
		}
		return TRUE;

	case WM_VSCROLL:
	case WM_HSCROLL:
		{
			long xamount = 0;
			long yamount = 0;
			SCROLLINFO si;
			
			if (win->textwin)
				UpdateWindow(win->textwin);
			if (win->treewin)
				UpdateWindow(win->treewin);

			si.cbSize = sizeof(si);
			si.fMask = SIF_PAGE|SIF_RANGE|SIF_POS|SIF_TRACKPOS;
			
			if (message == WM_HSCROLL)
			{
				switch (LOWORD(wParam))
				{
				case SB_TOP:
					xamount = (-win->docsize.x);
					break;
				case SB_BOTTOM:
					xamount = win->docsize.w - win->scroll.g_w - win->docsize.x;
					break;
				case SB_LINEUP:
					xamount = -win->x_raster;
					break;
				case SB_LINEDOWN:
					xamount = win->x_raster;
					break;
				case SB_PAGEUP:
					xamount = -win->scroll.g_w;
					break;
				case SB_PAGEDOWN:
					xamount = win->scroll.g_w;
					break;

				case SB_THUMBTRACK:
					GetScrollInfo(win->hwnd, SB_HORZ, &si);
					xamount = si.nTrackPos - win->docsize.x;
					break;
				case SB_THUMBPOSITION:
					GetScrollInfo(win->hwnd, SB_HORZ, &si);
					xamount = si.nPos - win->docsize.x;
					break;
				}
			} else
			{
				switch (LOWORD(wParam))
				{
				case SB_TOP:
					xamount = -win->docsize.x;
					yamount = -win->docsize.y;
					break;
				case SB_BOTTOM:
					xamount = -win->docsize.x;
					yamount = win->docsize.h - win->scroll.g_h - win->docsize.y;
					break;
				case SB_LINEUP:
					yamount = -win->y_raster;
					break;
				case SB_LINEDOWN:
					yamount = win->y_raster;
					break;
				case SB_PAGEUP:
					yamount = -win->scroll.g_h;
					break;
				case SB_PAGEDOWN:
					yamount = win->scroll.g_h;
					break;

				case SB_THUMBTRACK:
					GetScrollInfo(win->hwnd, SB_VERT, &si);
					yamount = si.nTrackPos - win->docsize.y;
					break;
				case SB_THUMBPOSITION:
					GetScrollInfo(win->hwnd, SB_VERT, &si);
					yamount = si.nPos - win->docsize.y;
					break;
				}
			}

			hv_scroll_window(win, xamount, yamount);
		}
		return 0;

	case WM_MOUSEWHEEL:
		{
			int amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

			if (win->textwin)
			{
				UpdateWindow(win->textwin);

				hv_scroll_window(win, 0, -amount * win->y_raster);
			}
		}
		win32debug_msg_end("textWndProc", message, "0");
		return 0;
	
	case WM_MOUSEHWHEEL:
		{
			int amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

			if (win->textwin)
			{
				UpdateWindow(win->textwin);

				hv_scroll_window(win, -amount * win->x_raster, 0);
			}
		}
		win32debug_msg_end("textWndProc", message, "0");
		return 0;
	
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	case WM_CHAR:
		if (key_press_event(win, message, wParam, lParam))
		{
			win32debug_msg_end("mainWndProc", message, "0");
			return 0;
		}
		break;
	
	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			
			switch (hdr->code)
			{
			case NM_DBLCLK:
				return 1;
			case TVN_GETINFOTIPW:
				{
					NMTVGETINFOTIPW *tip = (NMTVGETINFOTIPW *)lParam;
					WINDOW_DATA *target;
					char *text;
					wchar_t *str;
					hyp_nodenr node;
					HYP_DOCUMENT *hyp;
					INDEX_ENTRY *entry;
					
					target = hv_link_targetwin(win, win->treeview_parent);
					if (target == NULL)
						return FALSE;
					node = tip->lParam;
					hyp = (HYP_DOCUMENT *)target->data->data;
					entry = hyp->indextable[node];
					text = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
					str = hyp_utf8_to_wchar(text, STR0TERM, NULL);
#ifdef __CYGWIN__
					wcscpy(tip->pszText, str);
#else
					wcscpy_s(tip->pszText, tip->cchTextMax, str);
#endif
					g_free(str);
					g_free(text);
				}
				return TRUE;
			case TVN_GETINFOTIPA:
				{
					NMTVGETINFOTIPA *tip = (NMTVGETINFOTIPA *)lParam;
					WINDOW_DATA *target;
					char *str;
					hyp_nodenr node;
					HYP_DOCUMENT *hyp;
					INDEX_ENTRY *entry;

					target = hv_link_targetwin(win, win->treeview_parent);
					if (target == NULL)
						return FALSE;
					node = tip->lParam;
					hyp = (HYP_DOCUMENT *)target->data->data;
					entry = hyp->indextable[node];
					str = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), entry->name, entry->length - SIZEOF_INDEX_ENTRY, NULL);
#ifdef __CYGWIN__
					strncpy(tip->pszText, str, tip->cchTextMax);
#else
					strcpy_s(tip->pszText, tip->cchTextMax, str);
#endif
					g_free(str);
				}
				return TRUE;
			case NM_CLICK:
				/*
				 * some sources recommend to use TVN_ITEMCHANGED instead.
				 * But that message is also send when navigating with the
				 * cursors keys, not only when clicking the item.
				 */
				{
					TVHITTESTINFO info;
					DWORD pos = GetMessagePos();
					
					info.pt.x = LOWORD(pos);
					info.pt.y = HIWORD(pos);
					ScreenToClient(win->treewin, &info.pt);
					info.flags = 0;
					info.hItem = 0;
					SendMessage(win->treewin, TVM_HITTEST, 0, (LPARAM)&info);
					hv_show_item(win, info.hItem);
				}
				return 0;
			case NM_RETURN:
				{
					HTREEITEM item;
					item = (HTREEITEM)SendMessage(win->treewin, TVM_GETNEXTITEM, TVGN_CARET, 0);
					printf("return %p\n", item);
					hv_show_item(win, item);
				}
				return 0;
			case TVN_ITEMCHANGEDA:
			case TVN_ITEMCHANGEDW:
				{
#if 0
					NMTVITEMCHANGE *item = (NMTVITEMCHANGE *)lParam;
					printf("itemchange %x %x\n", item->uStateOld, item->uStateNew);
#endif
				}
				return 0;
			}
		}
		break;

	default:
		hv_commdlg_help(hwnd, message, wParam, lParam);
		break;
	}

	win32debug_msg_end("mainWndProc", message, "DefWindowProc");
	
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*** ---------------------------------------------------------------------- ***/

static void set_cursor_if_appropriate(WINDOW_DATA *win, int x, int y)
{
	LINK_INFO info;
	gboolean hovering = FALSE;
	
	if (!win->textwin)
		return;
	if (HypFindLink(win, x, y, &info, FALSE))
	{
		if (info.tip)
		{
			set_tooltip_text(win->textwin, info.tip);
			g_free(info.tip);
		}
		hovering = TRUE;
	}
	if (hovering != win->hovering_over_link)
	{
		win->hovering_over_link = hovering;
		if (hovering)
		{
			SetWindowCursor(win->textwin, hand_cursor);
		} else
		{
			SetWindowCursor(win->textwin, regular_cursor);
			set_tooltip_text(win->textwin, NULL);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * if the link was from a different window
 * like the treeview, find the window were the
 * actual hypertext is displayed.
 * Only use that if it is still displaying the
 * same hypertext
 */
WINDOW_DATA *hv_link_targetwin(WINDOW_DATA *win, unsigned int window_id)
{
	if (window_id != 0)
	{
		GSList *l;
		WINDOW_DATA *parentwin;

		for (l = all_list; l; l = l->next)
		{
			parentwin = (WINDOW_DATA *)l->data;
			if (parentwin->window_id == window_id)
			{
				DOCUMENT *doc1, *doc2;

				doc1 = win->data;
				doc2 = parentwin->data;
				win = parentwin;
				if (doc2->type != HYP_FT_HYP ||
					strcmp(doc1->path, doc2->path) != 0)
					win = NULL;
				break;
			}
		}
	}
	return win;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean follow_if_link(WINDOW_DATA *win, int x, int y)
{
	LINK_INFO info;
	
	if (HypFindLink(win, x, y, &info, !gl_profile.viewer.refonly))
	{
		g_free(info.tip);
		if (win->is_popup)
		{
			WINDOW_DATA *parentwin;
			GSList *l;;
			
			for (l = all_list; l; l = l->next)
			{
				parentwin = (WINDOW_DATA *)l->data;
				if (parentwin->popup == win)
				{
					HypClick(parentwin, &info);
					break;
				}
			}
		} else
		{
			HypClick(win, &info);
		}
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean on_button_release(WINDOW_DATA *win, int x, int y, int button)
{
	gboolean found = FALSE;
	
	if (button == 1)
	{
		CheckFiledate(win);
		
		/* we shouldn't follow a link if the user has selected something */
		if (win->selection.valid)
			return FALSE;
	
		found = follow_if_link(win, x, y);
	}

	if (win->is_popup)
	{
		DestroyWindow(win->hwnd);
		found = TRUE;
	}
	
	return found;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean on_button_press(WINDOW_DATA *win, int x, int y, int button)
{
	DOCUMENT *doc = win->data;
	
	RemoveSearchBox(win);
	
	if (button == 1)
	{
		CheckFiledate(win);
		if (win->textwin)
			MouseSelection(win, x, y, (getkeystate() & K_SHIFT) != 0);
	} else if (button == 2 && !win->popup && win->textwin)
	{
		if (gl_profile.viewer.rightback)
		{
			GoThisButton(win, TO_BACK);
		} else
		{
			HMENU menu = LoadMenuW(GetInstance(), MAKEINTRESOURCEW(IDR_CONTEXT_MENU));
			HMENU popup = GetSubMenu(menu, 0);
			UINT flags = TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_NOANIMATION;
			POINT p;
			
			SetForegroundWindow(win->hwnd);
			p.x = x;
			p.y = y;
			ClientToScreen(win->textwin, &p);
			hv_update_menu(popup, doc);
			TrackPopupMenu(popup, flags, p.x, p.y, 0, win->hwnd, NULL);
			PostMessage(win->hwnd, WM_NULL, 0, 0);
		}
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static LRESULT CALLBACK textWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WINDOW_DATA *win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	
	win32debug_msg_print(stderr, "textWndProc", hwnd, message, wParam, lParam);
	switch (message)
	{
	case WM_NCCREATE:
		win = (WINDOW_DATA *)(((CREATESTRUCT *)lParam)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (DWORD_PTR)win);
		win->textwin = hwnd;
		break;

	case WM_DESTROY:
		win->textwin = NULL;
		win32debug_msg_end("textWndProc", message, "FALSE");
		return FALSE;
	
	case WM_ERASEBKGND:
		{
			RECT r;
			HBRUSH brush = CreateSolidBrush(viewer_colors.background);
			GetClientRect(hwnd, &r);
			FillRect((HDC)wParam, &r, brush);
			DeleteObject(brush);
		}
		win32debug_msg_end("textWndProc", message, "TRUE");
		return TRUE;
	
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			GRECT gr;
			DOCUMENT *doc;
			
			doc = (DOCUMENT *) win->data;
			win->draw_hdc = W_BeginPaint(hwnd, &ps, &gr);
			if (win->cliprgn == NULL)
				win->cliprgn = CreateRectRgn(win->scroll.g_x, win->scroll.g_y, win->scroll.g_x + win->scroll.g_w, win->scroll.g_y + win->scroll.g_h);
			else
				SetRectRgn(win->cliprgn, win->scroll.g_x, win->scroll.g_y, win->scroll.g_x + win->scroll.g_w, win->scroll.g_y + win->scroll.g_h);
			SelectClipRgn(win->draw_hdc, win->cliprgn);
			doc->displayProc(win);
			DrawSelection(win);
			W_EndPaint(hwnd, &ps);
			win->draw_hdc = NULL;
		}
		win32debug_msg_end("textWndProc", message, "TRUE");
		return TRUE;

	case WM_MOUSEWHEEL:
		{
			int amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

			UpdateWindow(win->textwin);

			hv_scroll_window(win, 0, -amount * win->y_raster);
		}
		win32debug_msg_end("textWndProc", message, "0");
		return 0;
	
	case WM_MOUSEHWHEEL:
		{
			int amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

			UpdateWindow(win->textwin);

			hv_scroll_window(win, -amount * win->x_raster, 0);
		}
		win32debug_msg_end("textWndProc", message, "0");
		return 0;
	
	case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			set_cursor_if_appropriate(win, x, y);
		}
		break;
	
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			int button = message == WM_LBUTTONDOWN ? 1 : message == WM_RBUTTONDOWN ? 2 : 3;
			on_button_press(win, x, y, button);
		}
		break;
	
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			int button = message == WM_LBUTTONUP ? 1 : message == WM_RBUTTONUP ? 2 : 3;
			on_button_release(win, x, y, button);
		}
		break;
	}
	
	win32debug_msg_end("textWndProc", message, "DefWindowProc");
	
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*** ---------------------------------------------------------------------- ***/

static LRESULT CALLBACK treeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WINDOW_DATA *win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	win32debug_msg_print(stderr, "treeWndProc", hwnd, message, wParam, lParam);
	switch (message)
	{
	case WM_NCCREATE:
		win = (WINDOW_DATA *)(((CREATESTRUCT *)lParam)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (DWORD_PTR)win);
		win->treewin = hwnd;
		break;

	case WM_DESTROY:
		win->treewin = NULL;
		win32debug_msg_end("treeWndProc", message, "FALSE");
		return FALSE;

	case WM_ERASEBKGND:
		{
			RECT r;
			HBRUSH brush = CreateSolidBrush(viewer_colors.background);
			GetClientRect(hwnd, &r);
			FillRect((HDC)wParam, &r, brush);
			DeleteObject(brush);
		}
		win32debug_msg_end("treeWndProc", message, "TRUE");
		return TRUE;

	case WM_PAINT:
		/* nothing to do; the treeview control will repaint itself */
		break;

	case WM_MOUSEWHEEL:
		{
			int amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

			UpdateWindow(win->treewin);

			hv_scroll_window(win, 0, -amount * win->y_raster);
		}
		win32debug_msg_end("treeWndProc", message, "0");
		return 0;

	case WM_MOUSEHWHEEL:
		{
			int amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

			UpdateWindow(win->treewin);

			hv_scroll_window(win, -amount * win->x_raster, 0);
		}
		win32debug_msg_end("treeWndProc", message, "0");
		return 0;

	case WM_MOUSEMOVE:
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		break;
	}

	win32debug_msg_end("treeWndProc", message, "DefWindowProc");

	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*** ---------------------------------------------------------------------- ***/

DOCUMENT *hypwin_doc(WINDOW_DATA *win)
{
	return win->data;
}

/*** ---------------------------------------------------------------------- ***/

void hv_set_font(WINDOW_DATA *win)
{
	const char *name;
	HDC hdc;
	TEXTMETRIC tm;
	HFONT oldfont;
	int i;
	
	if (!win->textwin)
		return;
	name = gl_profile.viewer.use_xfont && gl_profile.viewer.xfont_name ? gl_profile.viewer.xfont_name : gl_profile.viewer.font_name;
	hdc = GetDC(win->textwin);
	for (i = 0; i <= HYP_TXT_MASK; i++)
	{
		if (win->fonts[i])
			DeleteObject(win->fonts[i]);
	}
	W_FontCreate(name, win->fonts);
	oldfont = (HFONT)SelectObject(hdc, win->fonts[HYP_TXT_NORMAL]);
	if (GetTextMetrics(hdc, &tm))
	{
		win->x_raster = tm.tmAveCharWidth;
		win->y_raster = tm.tmHeight + tm.tmExternalLeading;
	} else
	{
		hyp_debug("GetTextMetrics: %s\n", win32_errstring(GetLastError()));
		/* to avoid divisions by zero */
		win->x_raster = HYP_PIC_FONTW;
		win->y_raster = HYP_PIC_FONTH;
	}
	SelectObject(hdc, (HGDIOBJ)oldfont);
	ReleaseDC(win->textwin, hdc);
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *win32_hypview_window_new(DOCUMENT *doc, gboolean popup, gboolean treeview)
{
	static int registered;
	static unsigned int window_id;
	WINDOW_DATA *win;
	DWORD style;
	DWORD exstyle = 0;
	HWND hwnd;
	HMENU menu = 0;
	HWND parent = 0;
	int x, y, w, h;
	HINSTANCE inst = GetInstance();
	
	if (!registered)
	{
		WNDCLASSA wndclass;
		
		regular_cursor = LoadCursor(NULL, IDC_ARROW);
		hand_cursor = LoadCursor(NULL, IDC_HAND);
		
		wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wndclass.lpfnWndProc = mainWndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = inst;
		wndclass.hIcon = LoadIcon(inst, MAKEINTRESOURCE(IDI_MAINFRAME));
		wndclass.hCursor = regular_cursor;
		wndclass.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = "hypview";
		if (RegisterClassA(&wndclass) == 0)
			hyp_debug("can't register %s window class: %s", "main", win32_errstring(GetLastError()));

		wndclass.style = CS_DBLCLKS | CS_PARENTDC | CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = textWndProc;
		wndclass.hIcon = NULL;
		wndclass.hbrBackground = NULL;
		wndclass.lpszClassName = "hyptext";
		if (RegisterClassA(&wndclass) == 0)
			hyp_debug("can't register %s window class: %s", "text", win32_errstring(GetLastError()));

		wndclass.style = CS_DBLCLKS | CS_PARENTDC | CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = treeWndProc;
		wndclass.hIcon = NULL;
		wndclass.hbrBackground = NULL;
		wndclass.lpszClassName = "hyptree";
		if (RegisterClassA(&wndclass) == 0)
			hyp_debug("can't register %s window class: %s", "text", win32_errstring(GetLastError()));

		toolbar_register_classes(inst);
		
		registered = TRUE;
	}
	win = g_new0(WINDOW_DATA, 1);
	if (win == NULL)
		return NULL;
	
	win->data = doc;
	win->is_popup = popup;
	win->title = g_strdup(doc->path);
	win->window_id = ++window_id;
	win->treeview_window_id = 0;
	win->treeview_parent = 0;
	/* to avoid divisions by zero */
	win->x_raster = HYP_PIC_FONTW;
	win->y_raster = HYP_PIC_FONTH;
	
	if (popup)
	{
		style = WS_POPUP | WS_BORDER | WS_CLIPCHILDREN;
	} else if (treeview)
	{
		exstyle = WS_EX_OVERLAPPEDWINDOW;
		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_SIZEBOX | WS_CLIPCHILDREN;
	} else
	{
		MENUITEMINFO info;
		
		win->td = toolbar_init(win, tb_defs, (int)(sizeof(tb_defs) / sizeof(tb_defs[0])), tb_entries, (int)(sizeof(tb_entries) / sizeof(tb_entries[0])));
		exstyle = WS_EX_OVERLAPPEDWINDOW;
		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_HSCROLL | WS_VSCROLL | WS_SIZEBOX | WS_CLIPCHILDREN;
		menu = LoadMenuExW(inst, MAKEINTRESOURCEW(IDR_MAIN_MENU));
		memset(&info, 0, sizeof(info));
		info.cbSize = sizeof(info);
		info.fMask = MIIM_ID | MIIM_SUBMENU;
		GetMenuItemInfo(menu, IDM_NAV_BOOKMARKSMENU, FALSE, &info);
		win->bookmarks_menu = info.hSubMenu;
		GetMenuItemInfo(menu, IDM_FILE_RECENTMENU, FALSE, &info);
		win->recent_menu = info.hSubMenu;
	}
	x = gl_profile.viewer.win_x;
	y = gl_profile.viewer.win_y;
	w = gl_profile.viewer.win_w;
	h = gl_profile.viewer.win_h;
	if (default_geometry && !popup)
	{
		gtk_XParseGeometry(default_geometry, &x, &y, &w, &h);
		hv_win_set_geometry(NULL);
	}
	if (w == 0)
		w = CW_USEDEFAULT;
	if (h == 0)
		h = CW_USEDEFAULT;
		
	hwnd = CreateWindowExA(
		exstyle,
		"hypview",
		NULL,
		style,
		x, y, w, h,
		parent,
		menu,
		inst,
		(LPVOID)win);
	if (hwnd == 0)
	{
		g_free(win);
		hyp_debug("can't create %s window: %s", "main", win32_errstring(GetLastError()));
		return NULL;
	}

	if (win->td && win->td->visible)
		win->td->toolbar_open(win);

	if (treeview)
	{
		win->treewin = CreateWindowExW(
			0,
			WC_TREEVIEWW,
			NULL,
			WS_CHILD|WS_VISIBLE|TVS_HASLINES|TVS_HASBUTTONS|TVS_LINESATROOT|TVS_INFOTIP,
			x, y, w, h,
			hwnd,
			NULL,
			inst,
			(LPVOID)win);
		if (win->treewin == 0)
		{
			g_free(win);
			hyp_debug("can't create %s window: %s", "tree", win32_errstring(GetLastError()));
			return NULL;
		}
	} else
	{
		CreateWindowExA(
			0,
			"hyptext",
			NULL,
			WS_CHILD|WS_VISIBLE,
			x, y, w, h,
			hwnd,
			NULL,
			inst,
			(LPVOID)win);
		if (win->textwin == 0)
		{
			g_free(win);
			hyp_debug("can't create %s window: %s", "text", win32_errstring(GetLastError()));
			return NULL;
		}
	}
	
	win->y_margin_top = gl_profile.viewer.text_yoffset;
	win->y_margin_bottom = gl_profile.viewer.text_yoffset;
	win->x_margin_left = gl_profile.viewer.text_xoffset;
	win->x_margin_right = gl_profile.viewer.text_xoffset;
	
	hv_set_font(win);
	hv_set_title(win->hwnd, win->title);

	WindowCalcScroll(win);
	SetWindowSlider(win);
	
	if (!popup)
	{
		all_list = g_slist_prepend(all_list, win);
	}
	
	return win;
}

/*** ---------------------------------------------------------------------- ***/

long hv_win_topline(WINDOW_DATA *win)
{
	if (!win->textwin)
		return 0;
	return win->docsize.y / win->y_raster;
}

/*** ---------------------------------------------------------------------- ***/

void hv_set_title(HWND hwnd, const char *title)
{
	wchar_t *wtitle = hyp_utf8_to_wchar(title, STR0TERM, NULL);
	SetWindowTextW(hwnd, wtitle);
	g_free(wtitle);
}

/*** ---------------------------------------------------------------------- ***/

void SendRedraw(WINDOW_DATA *win)
{
	if (win->textwin)
		InvalidateRect(win->textwin, NULL, TRUE);
	if (win->treewin)
		InvalidateRect(win->treewin, NULL, TRUE);
}

/*** ---------------------------------------------------------------------- ***/

void SendCloseWindow(WINDOW_DATA *win)
{
	if (win)
		SendClose(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

void SendClose(HWND w)
{
	if (w)
	{
		PostMessage(w, WM_CLOSE, 0, 0);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * scroll window such that <line> is displayed at the top
 */
void hv_win_scroll_to_line(WINDOW_DATA *win, long line)
{
	if (!win->textwin)
		return;
	hv_scroll_window(win, 0, line * win->y_raster - win->docsize.y);
}

/*** ---------------------------------------------------------------------- ***/

void SetWindowSlider(WINDOW_DATA *win)
{
	SCROLLINFO si;
	WP_UNIT pos, range;
	
	if (!win->textwin)
		return;
	pos = win->docsize.x;
	range = win->docsize.w - 1;
	if (range < 0)
		range = 0;
	if (pos != win->scrollhpos || range != win->scrollhsize)
	{
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE|SIF_RANGE|SIF_POS|SIF_TRACKPOS;
		si.nMin = 0;
		si.nMax = range;
		si.nPage = win->scroll.g_w;
		si.nPos = pos;
		si.nTrackPos = si.nPos;
		SetScrollInfo(win->hwnd, SB_HORZ, &si, TRUE);
	}
	win->scrollhpos = pos;
	win->scrollhsize = range;

	pos = win->docsize.y;
	range = win->docsize.h - 1;
	if (range < 0)
		range = 0;
	if (pos != win->scrollvpos || range != win->scrollvsize)
	{
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE|SIF_RANGE|SIF_POS|SIF_TRACKPOS;
		si.nMin = 0;
		si.nMax = range;
		si.nPage = win->scroll.g_h;
		si.nPos = pos;
		si.nTrackPos = si.nPos;
		SetScrollInfo(win->hwnd, SB_VERT, &si, TRUE);
	}
	win->scrollvpos = pos;
	win->scrollvsize = range;
}

/*** ---------------------------------------------------------------------- ***/

void ReInitWindow(WINDOW_DATA *win, gboolean prep)
{
	DOCUMENT *doc = win->data;
	int visible_lines;
	
	win->hovering_over_link = FALSE;
	if (win->textwin)
	{
		SetWindowCursor(win->textwin, regular_cursor);
		hv_set_font(win);
	}
	if (prep)
		doc->prepNode(win, win->displayed_node);
	hv_set_title(win->hwnd, win->title);
	win->selection.valid = FALSE;
	
	/* adjust window size to new dimensions */
	if (gl_profile.viewer.adjust_winsize)
	{
	}

	visible_lines = (win->scroll.g_h + win->y_raster - 1) / win->y_raster;

	win->docsize.y = min(win->docsize.h - visible_lines * win->y_raster, doc->start_line * win->y_raster);
	win->docsize.y = max(0, win->docsize.y);
	win->docsize.x = 0;

	SetWindowSlider(win);
	ToolbarUpdate(win, TRUE);
	SendRedraw(win);
}
