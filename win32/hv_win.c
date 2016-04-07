#include "hv_defs.h"
#include "hypdebug.h"
#include "windebug.h"
#include "resource.rh"
#include "w_draw.h"

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
	indexof(TO_INDEX), indexof(TO_CATALOG), indexof(TO_REFERENCES), indexof(TO_HELP), TB_SEPARATOR,
	indexof(TO_INFO), indexof(TO_LOAD), indexof(TO_SAVE), TB_SEPARATOR,
	indexof(TO_REMARKER), TB_ENDMARK
#undef indexof
};

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
		doc = win->data;
		if (win->popup)
		{
			WINDOW_DATA *pop = win->popup;
			win->popup = NULL;
			DestroyWindow(pop->hwnd);
		}
		if (win->parentwin)
			win->parentwin->popup = NULL;
		toolbar_status_exit(win);
		hypdoc_unref(doc);
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

static LRESULT CALLBACK mainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WINDOW_DATA *win;
	
	win32debug_msg_print(stderr, "mainWndProc", hwnd, message, wParam, lParam);
	switch (message)
	{
	case WM_NCCREATE:
		win = (WINDOW_DATA *)(((CREATESTRUCT *)lParam)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (DWORD_PTR)win);
		win->hwnd = hwnd;
		break;

	case WM_CREATE:
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		hv_update_menu(win);
		break;
	
	case WM_CLOSE:
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		win32_hypview_window_finalize(win);
		win32debug_msg_end("mainWndProc", message, "FALSE");
		return FALSE;
	
	case WM_MOVE:
	case WM_SIZE:
		{
			RECT r;
			
			win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			GetWindowRect(hwnd, &r);
			gl_profile.viewer.win_x = r.left;
			gl_profile.viewer.win_y = r.top;
			gl_profile.viewer.win_w = r.right - r.left;
			gl_profile.viewer.win_h = r.bottom - r.top;
			HypProfile_SetChanged();
			WindowCalcScroll(win);
			SetWindowSlider(win);
		}
		break;
	
	case WM_INITMENUPOPUP:
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		{
			MENUITEMINFO info;
			
			memset(&info, 0, sizeof(info));
			info.cbSize = sizeof(info);
			info.fMask = MIIM_ID;
			GetMenuItemInfo((HMENU)wParam, LOWORD(lParam), TRUE, &info);
			switch (info.wID)
			{
			case IDM_FILE_RECENTMENU:
				RecentUpdate(win);
				break;
			case IDM_NAV_BOOKMARKSMENU:
				MarkerUpdate(win);
				break;
			}
		}
		break;
		
	case WM_COMMAND:
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		switch (LOWORD(wParam))
		{
		case IDM_FILE_OPEN:
			SelectFileLoad(win);
			break;
		case IDM_FILE_SAVE:
			SelectFileSave(win);
			break;
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
			on_bookmark_selected(win, LOWORD(wParam) - IDM_NAV_BOOKMARK_1);
			break;
		case IDM_NAV_HISTORYMENU:
			HistoryPopup(win, 1);
			break;
		
		case IDM_OPT_SELECTFONT:
			SelectFont(win);
			break;
		case IDM_OPT_SELECTCOLORS:
			hv_config_colors(win);
			break;
		case IDM_OPT_OUTPUT:
			HYP_DBG(("NYI: on_output_settings"));
			break;
		case IDM_OPT_ALTFONT:
			gl_profile.viewer.use_xfont = !gl_profile.viewer.use_xfont;
			SwitchFont(win);
			break;
		case IDM_OPT_EXPANDSPACES:
			gl_profile.viewer.expand_spaces = !gl_profile.viewer.expand_spaces;
			hv_update_menu(win);
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
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		win32debug_msg_end("mainWndProc", message, "FALSE");
		return FALSE;
	
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			GRECT gr;
			
			win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			W_BeginPaint(hwnd, &ps, &gr);
			W_EndPaint(hwnd, &ps);
			win32debug_msg_end("mainWndProc", message, "TRUE");
		}
		return TRUE;

	default:
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
		}
		break;
	}

	win32debug_msg_end("mainWndProc", message, "DefWindowProc");
	
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*** ---------------------------------------------------------------------- ***/

static LRESULT CALLBACK textWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WINDOW_DATA *win;
	
	win32debug_msg_print(stderr, "textWndProc", hwnd, message, wParam, lParam);
	switch (message)
	{
	case WM_NCCREATE:
		win = (WINDOW_DATA *)(((CREATESTRUCT *)lParam)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (DWORD_PTR)win);
		win->textwin = hwnd;
		break;

	case WM_DESTROY:
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		win32debug_msg_end("textWndProc", message, "FALSE");
		win->textwin = NULL;
		return FALSE;
	
	case WM_ERASEBKGND:
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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
			
			win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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
			win32debug_msg_end("textWndProc", message, "TRUE");
		}
		return TRUE;
	}
	
	win32debug_msg_end("textWndProc", message, "DefWindowProc");
	
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
	const char *name = gl_profile.viewer.use_xfont ? gl_profile.viewer.xfont_name : gl_profile.viewer.font_name;
	HDC hdc = GetDC(win->textwin);
	TEXTMETRIC tm;
	HFONT oldfont;
	int i;
	
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
	}
	SelectObject(hdc, (HGDIOBJ)oldfont);
	ReleaseDC(win->textwin, hdc);
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *win32_hypview_window_new(DOCUMENT *doc, gboolean popup)
{
	static int registered;
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
		
		wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wndclass.lpfnWndProc = mainWndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = inst;
		wndclass.hIcon = LoadIcon(inst, MAKEINTRESOURCE(IDI_MAINFRAME));
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
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

		toolbar_register_classes(inst);
		
		registered = TRUE;
	}
	win = g_new0(WINDOW_DATA, 1);
	if (win == NULL)
		return NULL;
	
	win->data = doc;
	win->is_popup = popup;
	win->title = g_strdup(doc->path);
	
	if (popup)
	{
		style = WS_POPUP | WS_BORDER | WS_CLIPCHILDREN;
	} else
	{
		win->td = toolbar_init(win, tb_defs, (int)(sizeof(tb_defs) / sizeof(tb_defs[0])), tb_entries, (int)(sizeof(tb_entries) / sizeof(tb_entries[0])));
		exstyle = WS_EX_OVERLAPPEDWINDOW;
		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_HSCROLL | WS_VSCROLL | WS_SIZEBOX | WS_CLIPCHILDREN;
		menu = LoadMenuExW(inst, MAKEINTRESOURCEW(IDR_MAIN_MENU));
	}
	x = gl_profile.viewer.win_x;
	y = gl_profile.viewer.win_y;
	w = gl_profile.viewer.win_w;
	h = gl_profile.viewer.win_h;
	if (default_geometry)
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
	
	win->y_margin_top = gl_profile.viewer.text_yoffset;
	win->x_margin_left = gl_profile.viewer.text_xoffset;
	win->x_margin_right = gl_profile.viewer.text_xoffset;
	
	hv_set_font(win);
	hv_set_title(win, win->title);

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
	UNUSED(win);
	/* NYI */
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

void hv_set_title(WINDOW_DATA *win, const char *title)
{
	wchar_t *wtitle = hyp_utf8_to_wchar(title, STR0TERM, NULL);
	SetWindowTextW(win->hwnd, wtitle);
	g_free(wtitle);
}

/*** ---------------------------------------------------------------------- ***/

void SendRedraw(WINDOW_DATA *win)
{
	InvalidateRect(win->textwin, NULL, TRUE);
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
	UNUSED(win);
	UNUSED(line);
	/* NYI */
}

/*** ---------------------------------------------------------------------- ***/

void SetWindowSlider(WINDOW_DATA *win)
{
	SCROLLINFO si;
	WP_UNIT pos, range;
	
	pos = win->docsize.x;
	range = win->docsize.w;
	if (pos != win->scrollhpos || range != win->scrollhsize)
	{
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE|SIF_RANGE|SIF_POS;
		si.nMin = 0;
		si.nMax = range - 1;
		if (si.nMax < 0)
			si.nMax = 0;
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
		si.fMask = SIF_PAGE|SIF_RANGE|SIF_POS;
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
	
	win->hovering_over_link = FALSE;
	SetClassLongPtr(win->hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));
	hv_set_font(win);
	if (prep)
		doc->prepNode(win, win->displayed_node);
	hv_set_title(win, win->title);
	
	/* adjust window size to new dimensions */
	if (gl_profile.viewer.adjust_winsize)
	{
	}

	if (doc->start_line)
	{
		hv_win_scroll_to_line(win, doc->start_line);
	}
	
	SetWindowSlider(win);
	ToolbarUpdate(win, FALSE);
	SendRedraw(win);
}
