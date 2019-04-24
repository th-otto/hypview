#include "hv_defs.h"

static gboolean can_search_again;
static char *last_search;
static volatile pid_t HypfindID = -1;
static CFRunLoopTimerRef hypfind_timer;

/*** ---------------------------------------------------------------------- ***/

static void hypfind_text(WINDOW_DATA *win, const char *search)
{
	DOCUMENT *doc = win->data;
	long line = hv_win_topline(win);
	long start_line = line;
	
	if (empty(search))
		return;
	doc->autolocator_dir = 1;
	if (!empty(search))
	{
		line = doc->autolocProc(win, start_line, search, gl_profile.viewer.find_casesensitive, gl_profile.viewer.find_word);
	}
	if (line >= 0)
	{
		if (line != start_line)
		{
			can_search_again = TRUE;
			hv_win_scroll_to_line(win, line);
		}
	} else
	{
		NSBeep();
	}
}

/*** ---------------------------------------------------------------------- ***/

#if 0
static void hypfind_page(WINDOW_DATA *win, const char *name)
{
	DOCUMENT *doc = win->data;
	if (empty(name))
		return;
	/*
	 * FIXME: ST-Guide search for all pages here where the string is part of the name
	 */
	OpenFileInWindow(win, doc->path, name, HYP_NOINDEX, TRUE, FALSE, FALSE);
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static void hypfind_search_allref(WINDOW_DATA *win, const char *name)
{
	if (empty(name))
		return;
	search_allref(win, name, FALSE);
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static void check_hypfind(CFRunLoopTimerRef timer, void *info)
{
	WINDOW_DATA *win = (WINDOW_DATA *)info;
	
	if (HypfindID > 0)
	{
		int ret;
		
		if (waitpid(HypfindID, &ret, WNOHANG) < 0)
		{
			HypfindID = -1;
		} else if (WIFEXITED(ret))
		{
			HypfindID = -1;
			if (WEXITSTATUS(ret) == 0)
			{
				OpenFileInWindow(NULL, HYP_FILENAME_HYPFIND, NULL, HYP_NOINDEX, FALSE, TRUE, FALSE);
			} else
			{
				char *str = g_strdup_printf(_("HypFind exited with code %d"), (int)WEXITSTATUS(ret));
				/* FIXME: win might have gone in the meantime */
				show_message(win, _("Error"), str, FALSE);
				g_free(str);
			}
		}
	}
	if (HypfindID < 0)
	{
		if (hypfind_timer != 0)
		{
			CFRunLoopTimerInvalidate(hypfind_timer);
			hypfind_timer = 0;
		}
	}
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static void hypfind_run_hypfind(WINDOW_DATA *win, gboolean all_hyp, const char *name)
{
	DOCUMENT *doc = win->data;
	const char *argv[9];
	int argc = 0;
	char *filename;
	
	if (empty(gl_profile.general.hypfind_path))
	{
		show_message(win, _("Error"), _("No path to HypFind configured"), FALSE);
		return;
	}
	if (empty(name))
		return;
	filename = path_subst(gl_profile.general.hypfind_path);
	argv[argc++] = filename;
	argv[argc++] = "-q";
	argv[argc++] = "-p";
	argv[argc++] = name;
	if (gl_profile.viewer.find_casesensitive)
		argv[argc++] = "-I";
	if (gl_profile.viewer.find_word)
		argv[argc++] = "-w";
		
	if (!all_hyp)
	{
		argv[argc++] = doc->path;
	}
	argv[argc] = NULL;
	HypfindID = hyp_utf8_spawnvp(P_NOWAIT, argc, argv);
	if (HypfindID > 0)
	{
		CFRunLoopTimerContext context;
		
		context.version = 0;
		context.info = win;
		context.retain = 0;
		context.release = 0;
		context.copyDescription = 0;
		hypfind_timer = CFRunLoopTimerCreate(NULL, 0.0, 0.2, 0, 0, check_hypfind, &context);
		CFRunLoopAddTimer(CFRunLoopGetCurrent(), hypfind_timer, kCFRunLoopCommonModes);
	} else
	{
		char *str = g_strdup_printf(_("Can not execute\n'%s'\n%s"), filename, hyp_utf8_strerror(errno));
		show_message(win, _("Error"), str, FALSE);
		g_free(str);
	}
	g_free(filename);
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static void on_apply(WINDOW_DATA *win)
{
	char *search;

	gl_profile.viewer.find_casesensitive = DlgGetButton(win, IDC_HYPFIND_CASE);
	gl_profile.viewer.find_word = DlgGetButton(win, IDC_HYPFIND_WORD);
	search = DlgGetText(win, IDC_HYPFIND_STRING);
	g_free(last_search);
	last_search = search;
	HypProfile_SetChanged();
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static INT_PTR CALLBACK hypfind_dialog(WINDOW_DATA *win, UINT message, WPARAM wParam, LPARAM lParam)
{
	WINDOW_DATA *win;
	WORD notifyCode;

	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_INITDIALOG:
		win = (WINDOW_DATA *)lParam;
		SetWindowLongPtr(win, GWLP_USERDATA, (DWORD_PTR)win);
		CenterWindow(win);
		DlgSetText(win, IDC_HYPFIND_STRING, fixnull(last_search));
		DlgSetButton(win, IDC_HYPFIND_CASE, gl_profile.viewer.find_casesensitive);
		DlgSetButton(win, IDC_HYPFIND_WORD, gl_profile.viewer.find_word);
		return TRUE;
	case WM_CLOSE:
		EndDialog(win, IDCANCEL);
		DestroyWindow(win);
		return TRUE;
	
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(win, GWLP_USERDATA);
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(win, IDCANCEL);
			DestroyWindow(win);
			return TRUE;
		case IDC_HYPFIND_TEXT:
			on_apply(win);
			EndDialog(win, IDOK);
			DestroyWindow(win);
			hypfind_text(win, last_search);
			return TRUE;
		case IDC_HYPFIND_PAGES:
			on_apply(win);
			EndDialog(win, IDOK);
			DestroyWindow(win);
			hypfind_page(win, last_search);
			return TRUE;
		case IDC_HYPFIND_REF:
			on_apply(win);
			EndDialog(win, IDOK);
			DestroyWindow(win);
			hypfind_search_allref(win, last_search);
			return TRUE;
		case IDC_HYPFIND_ALL_PAGE:
			on_apply(win);
			EndDialog(win, IDOK);
			DestroyWindow(win);
			hypfind_run_hypfind(win, FALSE, last_search);
			return TRUE;
		case IDC_HYPFIND_ALL_HYP:
			on_apply(win);
			EndDialog(win, IDOK);
			DestroyWindow(win);
			hypfind_run_hypfind(win, TRUE, last_search);
			return TRUE;
		case IDHELP:
			if (notifyCode == BN_CLICKED)
			{
				Help_Contents(win);
			}
			break;
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

void Hypfind(WINDOW_DATA *win, gboolean again)
{
	/*
	 * background process still running?
	 */
	if (HypfindID > 0)
		return;
	hypfind_timer = 0;
	if (win == NULL)
	{
		g_freep(&last_search);
		return;
	}
	
	if (again && can_search_again)
	{
		hypfind_text(win, last_search);
		return;
	}

	can_search_again = FALSE;
#if 0
	DialogBoxExW(NULL, MAKEINTRESOURCEW(IDD_HYPFIND), win, hypfind_dialog, (LPARAM)win);
#endif
}
