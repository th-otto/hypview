#include "hv_defs.h"

#define IDS_SELECT_HYPERTEXT _("*.hyp|Hypertext files (*.hyp)\n*.*|All files (*.*)\n")
#define IDS_SELECT_TEXTFILES _("*.txt|Text files (*.txt)\n*.*|All files (*.*)\n")

#define G_SEARCHPATH_SEPARATOR_S ";"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static gboolean choose_file(HWND parent, char **name, gboolean must_exist, const char *title, const char *filter)
{
	/* NYI */
	UNUSED(parent);
	UNUSED(must_exist);
	g_free(*name);
	*name = g_strdup("c:/atari/hyp/st-guide.hyp");
	UNUSED(title);
	UNUSED(filter);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *SelectFileLoad(WINDOW_DATA *win)
{
	char *name;
	char *subst;
	HWND parent = win ? win->hwnd : NULL;
	char **paths;
	
	if (win)
	{
		DOCUMENT *doc = (DOCUMENT *)win->data;
		name = g_strdup(doc->path);
	} else
	{
		subst = path_subst(gl_profile.general.path_list);
		paths = g_strsplit(subst, G_SEARCHPATH_SEPARATOR_S, 0);
		name = g_build_filename(paths[0], "*.hyp", NULL);
		g_strfreev(paths);
		g_free(subst);
	}
	
	if (choose_file(parent, &name, TRUE, _("Open Hypertext..."), IDS_SELECT_HYPERTEXT))
	{
		hv_recent_add(name);
		win = OpenFileInWindow(win, name, hyp_default_main_node_name, HYP_NOINDEX, TRUE, FALSE, FALSE);
	}
	g_free(name);
	return win;
}

/*** ---------------------------------------------------------------------- ***/

void SelectFileSave(WINDOW_DATA *win)
{
	/* NYI */
	UNUSED(win);
}
