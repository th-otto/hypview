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
 * open a file in a new windows
 */
WINDOW_DATA *OpenFileNewWindow(const char *path, const char *chapter, hyp_nodenr node, _BOOL find_default)
{
	DOCUMENT *doc = NULL;
	char *real_path;
	WINDOW_DATA *win = NULL;
	
	/* done if we don't have a name */
	if (empty(path))
		return NULL;

	graf_mouse(BUSY_BEE, NULL);

	/* if path starts with "*:\", remove it */
	if (path[0] == '*' && path[1] == ':' && G_IS_DIR_SEPARATOR(path[2]))
		path += 3;

	if ((real_path = find_file(NULL, path)) != NULL)
	{
		/* load and initialize hypertext file */
		doc = HypOpenFile(real_path, FALSE);
		if (doc != NULL)
		{
			if (doc->gotoNodeProc(doc, chapter, node) ||
				(find_default && doc->gotoNodeProc(doc, NULL, HYP_NOINDEX)))
			{
				/*  neues Fenster anlegen?  */
				win = doc->window = OpenWindow(HelpWindow, NAME | CLOSER | FULLER | MOVER | SIZER |
						   UPARROW | DNARROW | VSLIDE | LFARROW | RTARROW | HSLIDE | SMALLER, doc->path, -1, -1, doc);
			}
			if (win == NULL)
				HypCloseFile(doc);
		}
		g_free(real_path);
	}
	
	graf_mouse(ARROW, NULL);
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

		/* is there a window for that file already? */
		win = (WINDOW_DATA *) all_list;
		while (win != NULL)
		{
			if (win->type == WIN_WINDOW)
			{
				doc2 = win->data;
				if (filename_cmp(doc2->path, real_path) == 0)
				{
					AddHistoryEntry(win);
					doc = doc2;
					break;
				}
			}
			win = win->next;
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

		if (doc->gotoNodeProc(doc, chapter, 0))
		{
			/* no window already? */
			if (!win)
			{
				win = doc->window = OpenWindow(HelpWindow, NAME | CLOSER | FULLER | MOVER | SIZER | UPARROW | DNARROW |
						   VSLIDE | LFARROW | RTARROW | HSLIDE | SMALLER, doc->path, -1, -1, doc);
			} else
			{
				doc->window = win;
				if (win->status & WIS_ICONIFY)
					UniconifyWindow(win);
				doc->next = win->data;
				ReInitWindow(doc);
				wind_set_int(win->whandle, WF_TOP, 0);
			}
		} else
		{
			if (win)
			{
				doc->next = win->data;
				win->data = doc;
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
				str = g_strdup_printf(rs_string(WARN_NORESULT), name);
				form_alert(1, str);
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

			graf_mouse(BUSY_BEE, NULL);	/*  We are busy... */

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
			graf_mouse(ARROW, NULL);	/* We are done. */
		}
	}
}


void HypDeleteIfLast(DOCUMENT *doc, HYP_DOCUMENT *hyp)
{
	WINDOW_DATA *ptr;
	
	/*
	 * check if file is still in use by another window
	 */
	ptr = (WINDOW_DATA *) all_list;
	while (ptr)
	{
		if (ptr->type == WIN_WINDOW &&
			ptr->proc == HelpWindow &&
			ptr->data &&
			ptr->data != doc &&
			((DOCUMENT *)ptr->data)->data == hyp)
			return;
		ptr = ptr->next;
	}
	hyp_delete(hyp);
}
