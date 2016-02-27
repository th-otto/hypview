#include "hv_defs.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static char *find_file(WINDOW_DATA *win, const char *path)
{
	char *real_path;
	char *dir;
	char *filename;
	
	filename = path_subst(path);
	/* if we already have a window... */
	if (win)
	{
		DOCUMENT *doc = win->data;
		struct stat s;
		
		/* ...search for file in path of window */
		dir = hyp_path_get_dirname(doc->path);
		real_path = g_build_filename(dir, filename, NULL);
		g_free(dir);
		
		if (hyp_utf8_stat(real_path, &s) == 0)
		{
			g_free(filename);
			return real_path;
		}
		g_free(real_path);
	}
	real_path = hyp_find_file(filename);
	g_free(filename);
	return real_path;
}

/*** ---------------------------------------------------------------------- ***/

/* open a file in the same window */
WINDOW_DATA *OpenFileInWindow(WINDOW_DATA *win, const char *path, const char *chapter, hyp_nodenr node, gboolean find_default, int new_window, gboolean no_message)
{
	DOCUMENT *doc = NULL;
	char *real_path;
	gboolean add_to_hist = TRUE;
	HWND errorwin = win ? win->hwnd : NULL;
	
	/* done if we don't have a name */
	if (empty(path))
		return NULL;

	/* if path starts with "*:\", remove it */
	if (path[0] == '*' && path[1] == ':' && G_IS_DIR_SEPARATOR(path[2]))
		path += 3;

	if ((real_path = find_file(win, path)) == NULL)
	{
		if (!no_message)
			FileError(path, _("not found"));
		return NULL;
	}

	/* only load file if neccessary */
	if (new_window >= FORCE_NEW_WINDOW)
	{
		win = NULL;
	} else if (new_window || win == NULL)
	{
		DOCUMENT *doc2;
		GSList *l;
		WINDOW_DATA *win2;
		
		/* is there a window for that file already? */
		win = NULL;
		for (l = all_list; l; l = l->next)
		{
			win2 = (WINDOW_DATA *)l->data;
			doc2 = win2->data;
			if (filename_cmp(doc2->path, real_path) == 0)
			{
				if (win == NULL)
					win = win2;
				doc = hypdoc_ref(doc2);
				break;
			}
		}
	} else if (win != NULL)
	{
		DOCUMENT *doc2;
		GSList *l;
		WINDOW_DATA *win2;
		
		/* is there a window for that file already? */
		for (l = all_list; l; l = l->next)
		{
			win2 = (WINDOW_DATA *)l->data;
			doc2 = win2->data;
			if (filename_cmp(doc2->path, real_path) == 0)
			{
				doc = hypdoc_ref(doc2);
				break;
			}
		}
	}

	/* load and initialize hypertext file if neccessary */
	if (doc == NULL)
	{
		doc = HypOpenFile(real_path, FALSE);
	}
	g_free(real_path);
	
	if (doc != NULL)
	{
		gboolean found = FALSE;
		DOCUMENT *prev_doc;
		
		if (!doc->data)
		{
			int ret;

			/* (re-)load file */
			ret = hyp_utf8_open(doc->path, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
			if (ret >= 0)
			{
				LoadFile(doc, ret, FALSE);
				hyp_utf8_close(ret);
			} else
			{
				FileErrorErrno(hyp_basename(doc->path));
				found = TRUE; /* do not issue another error */
			}
		}

		new_window = 0;
		if (!win)
		{
			win = gtk_hypview_window_new(doc, FALSE);
			new_window = 1;
			add_to_hist = FALSE;
			prev_doc = NULL;
		} else
		{
			prev_doc = win->data;
			win->data = doc;
		}
		doc->start_line = 0;
		if (add_to_hist)
			AddHistoryEntry(win, prev_doc);
		if (doc->gotoNodeProc(win, chapter, node))
		{
			found = TRUE;
			ReInitWindow(win, FALSE);
			hv_win_open(win);
		} else if (find_default)
		{
			doc->gotoNodeProc(win, NULL, HYP_NOINDEX);
			if (chapter && strcmp(chapter, hyp_default_main_node_name) == 0)
				found = TRUE;
			ReInitWindow(win, FALSE);
			hv_win_open(win);
		} else
		{
			if (new_window)
			{
				DestroyWindow(win->hwnd);
			}
			win = NULL;
		}
		if (!found && !no_message)
		{
			char *str;
			char *name;
			gboolean converror = FALSE;
			
			if (chapter)
				name = hyp_utf8_to_charset(hyp_get_current_charset(), chapter, STR0TERM, &converror);
			else
				name = g_strdup(hyp_default_main_node_name);
			str = g_strdup_printf(_("%s: could not find\n'%s'"), gl_program_name, name);
			show_message(errorwin, _("Error"), str, FALSE);
			g_free(name);
			g_free(str);
		}
		hypdoc_unref(prev_doc);
	} else
	{
		win = NULL;
	}

	return win;
}

/*** ---------------------------------------------------------------------- ***/

/* Verifies the current documents file modification time/date and reloads the
 * document if necessary. This behaviour is enabled by the CHECK_TIME option. */
void CheckFiledate(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	struct stat st;
	int ret;
	
	if (!gl_profile.viewer.check_time)
		return;
	
	if (rpl_stat(doc->path, &st) == 0)
	{
		/* Modification time or date has changed ? */
		if (st.st_mtime != doc->mtime)
		{
			hyp_nodenr node;
			long lineno = 0;
			int ref_count = 0;
			
			node = doc->getNodeProc(win);	/* Remember current node */
			lineno = hv_win_topline(win);
			if (doc->data && doc->type == HYP_FT_HYP)
			{
				HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
				ref_count = hyp->ref_count;
				hyp->ref_count = 1;
				doc->data = hyp_unref(hyp);
			} else
			{
				doc->closeProc(doc);			/* Close document */
			}
							
			/* Reload file */
			ret = hyp_utf8_open(doc->path, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
			if (ret >= 0)
			{
				LoadFile(doc, ret, FALSE);
				hyp_utf8_close(ret);
				if (doc->data && doc->type == HYP_FT_HYP)
				{
					HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
					hyp->ref_count = ref_count;
				}
			} else
			{
				FileErrorErrno(hyp_basename(doc->path));
			}

			/* jump to previously active node */
			doc->gotoNodeProc(win, NULL, node);
			
			doc->start_line = lineno;
			ReInitWindow(win, FALSE);
		}
	}
}
