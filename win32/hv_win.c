#include "hv_defs.h"
#include "hypdebug.h"
#include "resource.rh"

static char *default_geometry;

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

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void hv_win_destroy_images(WINDOW_DATA *win)
{
	GSList *l;
	
	for (l = win->image_childs; l; l = l->next)
	{
		struct hyp_gfx *gfx = (struct hyp_gfx *)l->data;
		if (gfx->surf)
		{
			/* NYI */
			gfx->surf = NULL;
		}
	}
	g_slist_free(win->image_childs);
	win->image_childs = NULL;
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
		hypdoc_unref(doc);
		if (!win->is_popup)
		{
			RemoveAllHistoryEntries(win);
			all_list = g_slist_remove(all_list, win);
		}
		hv_win_destroy_images(win);
		g_freep(&win->title);
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
			if (WriteProfile(win) == FALSE)
				return FALSE;
		}
		DestroyWindow(win->hwnd);
		return TRUE;

	case WM_DESTROY:
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		win32_hypview_window_finalize(win);
		return FALSE;
	
	case WM_MOVE:
	case WM_SIZE:
		{
			RECT r;
			
			GetWindowRect(hwnd, &r);
			gl_profile.viewer.win_x = r.left;
			gl_profile.viewer.win_y = r.top;
			gl_profile.viewer.win_w = r.right - r.left;
			gl_profile.viewer.win_h = r.bottom - r.top;
			HypProfile_SetChanged();
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
			GoThisButton(win, TO_KATALOG);
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
		return 0;
	
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

	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*** ---------------------------------------------------------------------- ***/

DOCUMENT *hypwin_doc(WINDOW_DATA *win)
{
	return win->data;
}

/*** ---------------------------------------------------------------------- ***/

static void set_font_attributes(WINDOW_DATA *win)
{
	UNUSED(win);
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *gtk_hypview_window_new(DOCUMENT *doc, gboolean popup)
{
	static int registered;
	WINDOW_DATA *win;
	DWORD style;
	DWORD exstyle = 0;
	HWND hwnd;
	HMENU menu = 0;
	HWND parent = 0;
	int x, y, w, h;
	
	if (!registered)
	{
		WNDCLASS wndclass;

		wndclass.style = CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wndclass.lpfnWndProc = mainWndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = GetInstance();
		wndclass.hIcon = LoadIcon(GetInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = "hypview";

		RegisterClass(&wndclass);
	}
	win = g_new0(WINDOW_DATA, 1);
	
	win->data = doc;
	win->is_popup = popup;
	win->title = g_strdup(doc->path);
	
	if (popup)
	{
		style = WS_POPUP | WS_BORDER;
	} else
	{
		exstyle = WS_EX_OVERLAPPEDWINDOW;
		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_HSCROLL | WS_VSCROLL | WS_SIZEBOX;
		menu = LoadMenuExW(GetInstance(), MAKEINTRESOURCEW(IDR_MAIN_MENU));
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
		GetInstance(),
		(LPVOID)win);
	if (!hwnd)
	{
		g_free(win);
		return NULL;
	}
	
	set_font_attributes(win);
	hv_set_title(win, win->title);
	
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
	InvalidateRect(win->hwnd, NULL, TRUE);
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

void ReInitWindow(WINDOW_DATA *win, gboolean prep)
{
	DOCUMENT *doc = win->data;
	
	win->hovering_over_link = FALSE;
	SetClassLongPtr(win->hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));
	set_font_attributes(win);
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
	
	ToolbarUpdate(win, FALSE);
	SendRedraw(win);
}
