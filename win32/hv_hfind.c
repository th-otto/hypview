#include "hv_defs.h"
#include "resource.rh"

static gboolean can_search_again;
static volatile int HypfindID = -1;
static UINT_PTR hypfind_timer;

/*** ---------------------------------------------------------------------- ***/

static char *get_search_str(HWND dialog)
{
	wchar_t str[256];
	int len;
	
	if ((len = GetWindowTextW(GetDlgItem(dialog, IDC_HYPFIND_TEXT), str, 256)) <= 0)
		return NULL;
	return hyp_wchar_to_utf8(str, len);
}

/*** ---------------------------------------------------------------------- ***/

static void hypfind_text(WINDOW_DATA *win, HWND dialog)
{
	DOCUMENT *doc = win->data;
	long line = hv_win_topline(win);
	long start_line = line;
	char *search = get_search_str(dialog);
	
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
		MessageBeep(MB_ICONWARNING);
	}
	g_free(search);
}

/*** ---------------------------------------------------------------------- ***/

static void hypfind_page(WINDOW_DATA *win, HWND dialog)
{
	DOCUMENT *doc = win->data;
	char *name = get_search_str(dialog);
	if (empty(name))
		return;
	OpenFileInWindow(win, doc->path, name, HYP_NOINDEX, TRUE, FALSE, FALSE);
	g_free(name);
}

/*** ---------------------------------------------------------------------- ***/

static void hypfind_search_allref(WINDOW_DATA *win, HWND dialog)
{
	char *name = get_search_str(dialog);
	if (empty(name))
		return;
	search_allref(win, name, FALSE);
	g_free(name);
}

/*** ---------------------------------------------------------------------- ***/

static void CALLBACK check_hypfind(HWND hwnd, UINT msg, UINT_PTR id, DWORD dwtime)
{
	WINDOW_DATA *win = hwnd ? (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA) : 0;
	
	UNUSED(msg);
	UNUSED(id);
	UNUSED(dwtime);
	if (HypfindID > 0)
	{
		DWORD exitCode;
		
		if (GetExitCodeProcess((HANDLE)HypfindID, &exitCode))
		{
			CloseHandle((HANDLE)HypfindID);
			HypfindID = -1;
			if (exitCode == 0)
			{
				OpenFileInWindow(NULL, HYP_FILENAME_HYPFIND, NULL, HYP_NOINDEX, FALSE, TRUE, FALSE);
			} else
			{
				char *str = g_strdup_printf(_("HypFind exited with code %d"), (int)exitCode);
				show_message(win ? win->hwnd : NULL, _("Error"), str, FALSE);
				g_free(str);
			}
		}
	}
	if (HypfindID < 0)
	{
		if (hypfind_timer != 0)
		{
			KillTimer(hwnd, hypfind_timer);
			hypfind_timer = 0;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

static void hypfind_run_hypfind(WINDOW_DATA *win, HWND dialog, gboolean all_hyp)
{
	DOCUMENT *doc = win->data;
	const char *argv[8];
	int argc = 0;
	char *name;
	char *filename;
	
	if (empty(gl_profile.general.hypfind_path))
	{
		show_message(win ? win->hwnd : NULL, _("Error"), _("No path to HypFind configured"), FALSE);
		return;
	}
	name = get_search_str(dialog);
	if (empty(name))
		return;
	filename = path_subst(gl_profile.general.hypfind_path);
	argv[argc++] = filename;
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
		hypfind_timer = SetTimer(win ? win->hwnd : NULL, 0, 50, check_hypfind);
	} else
	{
		char *str = g_strdup_printf(_("Can not execute\n'%s'\n%s"), filename, hyp_utf8_strerror(errno));
		show_message(win ? win->hwnd : NULL, _("Error"), str, FALSE);
		g_free(str);
	}
	g_free(filename);
	g_free(name);
}

/*** ---------------------------------------------------------------------- ***/

void Hypfind(WINDOW_DATA *win, gboolean again)
{
	HWND dialog = 0;
	int resp = -1;
	
	/*
	 * background process still running?
	 */
	if (HypfindID != -1)
		return;

	if (again && can_search_again)
	{
		hypfind_text(win, dialog);
		return;
	}

	can_search_again = FALSE;
	switch (resp)
	{
	case 1:
		hypfind_text(win, dialog);
		break;
	case 2:
		hypfind_page(win, dialog);
		break;
	case 3:
		hypfind_search_allref(win, dialog);
		break;
	case 4:
		hypfind_run_hypfind(win, dialog, FALSE);
		break;
	case 5:
		hypfind_run_hypfind(win, dialog, TRUE);
		break;
	}
}
