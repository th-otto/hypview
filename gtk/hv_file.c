#define GDK_DISABLE_DEPRECATION_WARNINGS

#include "hv_gtk.h"
#include "hypdebug.h"


static char *find_file(WINDOW_DATA *win, const char *path)
{
	int ret;
	char *real_path;
	char *dir;
	char *filename;
	
	filename = path_subst(path);
	/*  Falls schon eine Datei/ein Fenster geoeffnet wurde  */
	if (win)
	{
		DOCUMENT *doc = win->data;

		/* search for file in path of window */
		dir = g_path_get_dirname(doc->path);
		real_path = g_build_filename(dir, filename, NULL);
		g_free(dir);
		
		ret = hyp_utf8_open(real_path, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (ret >= 0)
		{
			hyp_utf8_close(ret);
			g_free(filename);
			return real_path;
		}
		g_free(real_path);
	}
	real_path = hyp_find_file(filename);
	g_free(filename);
	return real_path;
}


/*
 * open a file in a new window
 */
WINDOW_DATA *OpenFileNewWindow(const char *path, const char *chapter, hyp_nodenr node, _BOOL find_default)
{
	DOCUMENT *doc = NULL;
	char *real_path;
	WINDOW_DATA *win = NULL;
	
	/* done if we don't have a name */
	if (empty(path))
		return NULL;

	/* if path starts with "*:\", remove it */
	if (path[0] == '*' && path[1] == ':' && G_IS_DIR_SEPARATOR(path[2]))
		path += 3;

	if ((real_path = find_file(NULL, path)) != NULL)
	{
		/* load and initialize hypertext file */
		doc = HypOpenFile(real_path, FALSE);
		if (doc != NULL)
		{
			win = hv_win_new(doc, FALSE);
			if (doc->gotoNodeProc(doc, chapter, node) ||
				(find_default && doc->gotoNodeProc(doc, NULL, HYP_NOINDEX)))
			{
			} else
			{
				gtk_widget_destroy(win->hwnd);
				win = NULL;
			}
			if (win == NULL)
				HypCloseFile(doc);
		}
		g_free(real_path);
	}
	
	if (doc == NULL)
		FileError(path, _("not found"));

	return win;
}


/* open a file in the same window */
WINDOW_DATA *OpenFileSameWindow(WINDOW_DATA *win, const char *path, const char *chapter, gboolean new_window, gboolean no_message)
{
	DOCUMENT *doc = NULL;
	char *real_path;
	
	/* done if we don't have a name */
	if (empty(path))
		return NULL;

	/* if path starts with "*:\", remove it */
	if (path[0] == '*' && path[1] == ':' && G_IS_DIR_SEPARATOR(path[2]))
		path += 3;

	if ((real_path = find_file(win, path)) == NULL)
	{
		FileError(path, _("not found"));
		return NULL;
	}

	/* only load file if neccessary */
	if (new_window)
	{
		DOCUMENT *doc2;
		GSList *l;
		
		/* is there a window for that file already? */
		win = NULL;
		for (l = all_list; l; l = l->next)
		{
			win = (WINDOW_DATA *)l->data;
			doc2 = win->data;
			if (filename_cmp(doc2->path, real_path) == 0)
			{
				AddHistoryEntry(win);
				doc = doc2;
				break;
			}
		}
	} else if (win != NULL)
	{
		DOCUMENT *prev_doc = win->data;

		AddHistoryEntry(win);

		/* is that file already loaded in this window? */
		if (filename_cmp(prev_doc->path, real_path) == 0)
		{
			doc = prev_doc;
			win->data = prev_doc->next;
			prev_doc->next = NULL;
		} else
		{
			prev_doc->closeProc(prev_doc);

			doc = prev_doc->next;
			while (doc)
			{
				if (filename_cmp(doc->path, real_path) == 0)
				{
					prev_doc->next = doc->next;
					doc->next = NULL;
					break;
				}
				prev_doc = doc;
				doc = doc->next;
			}
		}
	}

	/* load and initialize hypertext file if neccessary */
	if (doc == NULL)
		doc = HypOpenFile(real_path, FALSE);
	g_free(real_path);
	
	if (doc != NULL)
	{
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
			}
		}

		if (!win)
			win = hv_win_new(doc, FALSE);
		if (doc->gotoNodeProc(doc, chapter, HYP_NOINDEX))
		{
			/* no window already? */
			if (!win)
			{
				win = hv_win_new(doc, FALSE);
			} else
			{
				doc->window = win;
				gtk_widget_show(win->hwnd);
				doc->next = win->data;
				ReInitWindow(doc);
				gtk_window_present(GTK_WINDOW(win->hwnd));
			}
		} else
		{
			doc->gotoNodeProc(doc, NULL, HYP_NOINDEX);
			if (win)
			{
				doc->next = win->data;
				win->data = doc;
				doc->window = win;
				gtk_widget_show(win->hwnd);
				ReInitWindow(doc);
				gtk_window_present(GTK_WINDOW(win->hwnd));
			}
			if (!no_message)
			{
				char *str;
				char *name;
				gboolean converror = FALSE;
				
				if (chapter)
					name = hyp_utf8_to_charset(hyp_get_current_charset(), chapter, STR0TERM, &converror);
				else
					name = g_strdup(hyp_default_main_node_name);
				str = g_strdup_printf(_("%s: could not find\n'%s'"), gl_program_name, name);
				show_message(_("Error"), str, FALSE);
				g_free(name);
				g_free(str);
			}
		
		}
	} else
	{
		win = NULL;
	}

	return win;
}


/* Verifies the current documents file modification time/date and reloads the
 * document if necessary. This behaviour is enabled by the CHECK_TIME option. */
void CheckFiledate(DOCUMENT *doc)
{
	struct stat st;
	int ret;
	
	if (rpl_stat(doc->path, &st) == 0)
	{
		/* Modification time or date has changed ? */
		if (st.st_mtime != doc->mtime)
		{
			hyp_nodenr node;

			node = doc->getNodeProc(doc);	/* Remember current node */
			doc->closeProc(doc);		/* Close document */

			/* Reload file */
			ret = hyp_utf8_open(doc->path, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
			if (ret >= 0)
			{
				LoadFile(doc, ret, FALSE);
				hyp_utf8_close(ret);
			} else
			{
				FileErrorErrno(hyp_basename(doc->path));
			}

			/* jump to previously active node */
			doc->gotoNodeProc(doc, NULL, node);

			ReInitWindow(doc);
		}
	}
}


void HypDeleteIfLast(DOCUMENT *doc, HYP_DOCUMENT *hyp)
{
	WINDOW_DATA *ptr;
	GSList *l;
	
	/*
	 * check if file is still in use by another window
	 */
	for (l = all_list; l; l = l->next)
	{
		ptr = (WINDOW_DATA *)l->data;
		if (ptr->data &&
			ptr->data != doc &&
			((DOCUMENT *)ptr->data)->data == hyp)
			return;
	}
	hyp_delete(hyp);
}
