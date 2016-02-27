#include "hv_defs.h"
#include "hypdebug.h"
#include "resource.rh"

static volatile int remarker_pid = -1;
static UINT_PTR remarker_timer;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void CALLBACK check_remarker(HWND hwnd, UINT msg, UINT_PTR id, DWORD dwtime)
{
	UNUSED(hwnd);
	UNUSED(msg);
	UNUSED(id);
	UNUSED(dwtime);
	if (remarker_pid > 0)
	{
		DWORD ret;
		
		if (GetExitCodeProcess((HANDLE)remarker_pid, &ret))
		{
			CloseHandle((HANDLE)remarker_pid);
			remarker_pid = -1;
		}
	}
	if (remarker_pid < 0)
	{
		if (remarker_timer != 0)
		{
			KillTimer(NULL, remarker_timer);
			remarker_timer = 0;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

int StartRemarker(WINDOW_DATA *win, remarker_mode mode, gboolean quiet)
{
	char *path;
	const char *argv[5];
	int argc = 0;
	
	if (remarker_pid > 0 || mode == remarker_check)
		return remarker_pid;
	
	path = path_subst(gl_profile.remarker.path);
	
	if (empty(path))
	{
		if (!quiet)
			show_message(win ? win->hwnd : NULL, _("Error"), _("No path to REMARKER configured"), FALSE);
	} else
	{
		char *nodename = NULL;
		
		argv[argc++] = path;
		if (mode == remarker_startup)
		{
			argv[argc++] = "-t";
		} else if (mode == remarker_top && win)
		{
			DOCUMENT *doc = win->data;
			argv[argc++] = "-r";
			argv[argc++] = doc->path;
			if (doc->type == HYP_FT_HYP)
			{
				HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
				nodename = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[win->displayed_node->number]->name, STR0TERM);
				argv[argc++] = nodename;
			}
		}
		argv[argc] = NULL;
		if ((remarker_pid = hyp_utf8_spawnvp(P_NOWAIT, 1, argv)) < 0)
		{
			if (!quiet)
			{
				char *str = g_strdup_printf(_("Can not execute\n'%s'\n%s"), path, hyp_utf8_strerror(errno));
				show_message(win ? win->hwnd : NULL, _("Error"), str, FALSE);
				g_free(str);
			}
		} else
		{
			remarker_timer = SetTimer(NULL, 0, 50, check_remarker);
		}
		g_free(nodename);
	}
	g_free(path);
	return remarker_pid;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void SelectAll(WINDOW_DATA *win)
{
	UNUSED(win);
	/* NYI */
}

/*** ---------------------------------------------------------------------- ***/

void BlockOperation(WINDOW_DATA *win, enum blockop num)
{
	DOCUMENT *doc = win->data;

	switch (num)
	{
	case CO_SAVE:
		SelectFileSave(win);
		break;
	case CO_BACK:
		GoBack(win);
		break;
	case CO_COPY:
		BlockCopy(win);
		break;
	case CO_PASTE:
		if (doc->buttons.searchbox)
			AutoLocatorPaste(win);
		else
			BlockPaste(win, gl_profile.viewer.clipbrd_new_window);
		break;
	case CO_SELECT_ALL:
		SelectAll(win);
		break;
	case CO_SEARCH:
		Hypfind(win, FALSE);
		break;
	case CO_SEARCH_AGAIN:
		Hypfind(win, TRUE);
		break;
	case CO_DELETE_STACK:
		RemoveAllHistoryEntries(win);
		ToolbarUpdate(win, TRUE);
		break;
	case CO_SWITCH_FONT:
		gl_profile.viewer.use_xfont = !gl_profile.viewer.use_xfont;
		SwitchFont(win);
		break;
	case CO_SELECT_FONT:
		SelectFont(win);
		break;
	case CO_REMARKER:
		StartRemarker(win, remarker_top, FALSE);
		ToolbarUpdate(win, FALSE);
		break;
	case CO_PRINT:
		HYP_DBG(("NYI: print"));
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void BlockSelectAll(WINDOW_DATA *win, BLOCK *b)
{
	SelectAll(win);
	UNUSED(b);
}

/*** ---------------------------------------------------------------------- ***/

void BlockCopy(WINDOW_DATA *win)
{
	UNUSED(win);
	/* NYI */
}

/*** ---------------------------------------------------------------------- ***/

void BlockPaste(WINDOW_DATA *win, gboolean new_window)
{
	UNUSED(win);
	UNUSED(new_window);
	/* NYI */
}

/*** ---------------------------------------------------------------------- ***/

void BlockAsciiSave(WINDOW_DATA *win, const char *path)
{
	UNUSED(win);
	UNUSED(path);
	/* NYI */
}
