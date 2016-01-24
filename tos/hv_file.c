/*
 * HypView - (c) 2001 - 2006 Philipp Donze
 *               2006 -      Philipp Donze & Odd Skancke
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
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hv_defs.h"
#include "hypdebug.h"
#include "hypview.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static char *find_file(WINDOW_DATA *win, const char *path)
{
	int ret;
	char *real_path;
	char *dir;
	char *filename;
	
	filename = path_subst(path);
	/* if we already have a window... */
	if (win)
	{
		DOCUMENT *doc = win->data;

		/* ...search for file in path of window */
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

/*** ---------------------------------------------------------------------- ***/

/* open a file in the same window */
WINDOW_DATA *OpenFileInWindow(WINDOW_DATA *win, const char *path, const char *chapter, hyp_nodenr node, gboolean find_default, int new_window, gboolean no_message)
{
	DOCUMENT *doc = NULL;
	char *real_path;
	gboolean add_to_hist = TRUE;
	
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
	if (new_window > 1)
	{
		win = NULL;
	} else if (new_window || win == NULL)
	{
		DOCUMENT *doc2;
		
		/* is there a window for that file already? */
		win = (WINDOW_DATA *) all_list;
		while (win != NULL)
		{
			if (win->type == WIN_WINDOW)
			{
				doc2 = win->data;
				if (filename_cmp(doc2->path, real_path) == 0)
				{
					doc = hypdoc_ref(doc2);
					break;
				}
			}
			win = win->next;
		}
	} else if (win != NULL)
	{
		DOCUMENT *doc2;
		WINDOW_DATA *win2;
		
		for (win2 = (WINDOW_DATA *)all_list; win2; win2 = win2->next)
		{
			if (win->type == WIN_WINDOW)
			{
				doc2 = win->data;
				/* is that file already loaded in this window? */
				if (filename_cmp(doc2->path, real_path) == 0)
				{
					doc = hypdoc_ref(doc2);
					break;
				}
			}
		}
	}

	/* load and initialize hypertext file if neccessary */
	if (doc == NULL)
		doc = HypOpenFile(real_path, FALSE);
	g_free(real_path);
	
	if (doc != NULL)
	{
		gboolean found = FALSE;
		DOCUMENT *prev_doc = NULL;
		
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
			add_to_hist = FALSE;
		if (add_to_hist)
			AddHistoryEntry(win, prev_doc);
		if (doc->gotoNodeProc(win, chapter, node))
		{
			found = TRUE;
			/* no window already? */
			if (!win)
			{
				win = OpenWindow(HelpWindow, NAME | CLOSER | FULLER | MOVER | SIZER | UPARROW | DNARROW |
						   VSLIDE | LFARROW | RTARROW | HSLIDE | SMALLER, doc->path, -1, -1, doc);
				new_window = 1;
				win->data = doc;
			} else
			{
				prev_doc = win->data;
				win->data = doc;
				if (win->status & WIS_ICONIFY)
					UniconifyWindow(win);
				ReInitWindow(win);
				wind_set_int(win->whandle, WF_TOP, 0);
			}
		} else if (find_default)
		{
			doc->gotoNodeProc(win, NULL, HYP_NOINDEX);
			if (win)
			{
				prev_doc = win->data;
				win->data = doc;
				if (win->status & WIS_ICONIFY)
					UniconifyWindow(win);
				ReInitWindow(win);
				wind_set_int(win->whandle, WF_TOP, 0);
			}
		} else
		{
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
			str = g_strdup_printf(rs_string(WARN_NORESULT), name);
			form_alert(1, str);
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

			graf_mouse(BUSY_BEE, NULL);	/* We are busy... */

			node = doc->getNodeProc(doc);	/* Remember current node */
			lineno = win->docsize.y;
			if (doc->data && doc->type == HYP_FT_HYP)
			{
				HYP_DOCUMENT *hyp = doc->data;
				ref_count = hyp->ref_count;
				hyp->ref_count = 1;
				doc->data = hyp_unref(hyp);
			} else
			{
				doc->closeProc(doc);		/* Close document */
			}
			
			/* Reload file */
			ret = hyp_utf8_open(doc->path, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
			if (ret >= 0)
			{
				LoadFile(doc, ret, FALSE);
				hyp_utf8_close(ret);
				if (doc->data && doc->type == HYP_FT_HYP)
				{
					HYP_DOCUMENT *hyp = doc->data;
					hyp->ref_count = ref_count;
				}
			} else
			{
				FileErrorErrno(hyp_basename(doc->path));
			}

			/* jump to previously active node */
			doc->gotoNodeProc(win, NULL, node);

			win->docsize.y = lineno;
			ReInitWindow(win);
			graf_mouse(ARROW, NULL);	/* We are done. */
		}
	}
}
