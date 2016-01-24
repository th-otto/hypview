/*
 * HypView - (c)      - 2006 Philipp Donze
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
#include "hypview.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void GotoPage(WINDOW_DATA *win, hyp_nodenr num, long line, gboolean calc)
{
	DOCUMENT *doc = win->data;

	graf_mouse(BUSY_BEE, NULL);
	if (doc->gotoNodeProc(win, NULL, num))
	{
		if (calc)
		{
			/*
			 * when activated by a link, we have to calculate the correct "line".
			 * Important if page contains images.
			 */
			long y;
			HYP_NODE *node;
	
			node = doc->displayed_node;
	
			y = HypGetLineY(node, line);
			line = y / win->y_raster;
		}
		doc->start_line = line;
	}
	graf_mouse(ARROW, NULL);
	ReInitWindow(win);
}

/*** ---------------------------------------------------------------------- ***/

void GoBack(WINDOW_DATA *win)
{
	DOCUMENT *old_doc = win->data;
	DOCUMENT *new_doc;
	hyp_nodenr page;
	long line;

	if ((new_doc = RemoveHistoryEntry(win, &page, &line)) != NULL)
	{
		/* changing file? */
		{
			int ret;

			/* if old document is not used anymore... */
			hypdoc_unref(old_doc);		/* ...close document */
			win->data = new_doc;
			
			/* load new file */
			if (new_doc->data == NULL)
			{
				ret = hyp_utf8_open(new_doc->path, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
				if (ret >= 0)
				{
					LoadFile(new_doc, ret, FALSE);
					hyp_utf8_close(ret);
				} else
				{
					FileErrorErrno(hyp_basename(new_doc->path));
				}
			}
		}

		if (new_doc->type >= 0)			/* known filetype? */
			GotoPage(win, page, line, FALSE);	/* switch to requested page */
		else
			FileError(hyp_basename(new_doc->path), _("format not recognized"));
	}
}

/*** ---------------------------------------------------------------------- ***/

void HistoryPopup(WINDOW_DATA *win, short x, short y)
{
	DOCUMENT *old_doc = win->data;
	OBJECT *tree = rs_tree(EMPTYPOPUP);
	short i, h;
	short sel;
	size_t len = 0;
	HISTORY *entry = win->history;

	i = tree[ROOT].ob_head;
	h = 0;
	while (entry && i != ROOT)
	{
		tree[i].ob_flags = OF_SELECTABLE | OF_TOUCHEXIT;
		tree[i].ob_spec.free_string = entry->title;
		len = max(strlen(entry->title) + 1, len);
		h = max(h, tree[i].ob_y + tree[i].ob_height);
		i = tree[i].ob_next;
		entry = entry->next;
	}

	if (h == 0)
	{
		/*
		 * no history entries found? we should not get here,
		 * the toolbar entry should not have been selectable
		 */
		return;
	}
	
	/* Hide unused entries */
	while (i != ROOT)
	{
		tree[i].ob_flags = OF_HIDETREE;
		i = tree[i].ob_next;
	}

	len = len * pwchar;

	tree[EM_BACK].ob_x = x;
	tree[EM_BACK].ob_y = y;
	tree[EM_BACK].ob_width = (_WORD) len;
	tree[EM_BACK].ob_height = h;

	/* set same width for all entries */
	for (i = tree[ROOT].ob_head; i != ROOT; i = tree[i].ob_next)
	{
		tree[i].ob_width = (_WORD) len;
		tree[i].ob_height = phchar;
	}

	sel = popup_select(tree, 0, 0);

	/* reset strings; the popup is used also by other routines */
	for (i = tree[ROOT].ob_head; i != ROOT; i = tree[i].ob_next)
	{
		tree[i].ob_flags = OF_SELECTABLE;
		tree[i].ob_spec.free_string = NULL;
	}

	if (sel > 0)
	{
		DOCUMENT *new_doc;
		hyp_nodenr page;
		long line;

		/* Find selected entry */
		i = tree[ROOT].ob_head;
		while ((new_doc = RemoveHistoryEntry(win, &page, &line)) != NULL && i != ROOT)
		{
			hypdoc_unref(old_doc);
			win->data = old_doc = new_doc;
			if (sel == i)
				break;
			i = tree[i].ob_next;
		}

		new_doc = win->data;
		if (!new_doc->data)
		{
			int ret;

			/* open new document */
			ret = hyp_utf8_open(new_doc->path, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
			if (ret >= 0)
			{
				LoadFile(new_doc, ret, FALSE);
				hyp_utf8_close(ret);
			} else
			{
				FileErrorErrno(hyp_basename(new_doc->path));
			}
		}

		if (new_doc->type >= 0)			/* known file type ? */
			GotoPage(win, page, line, FALSE);	/* jump to page */
		else
			FileError(hyp_basename(new_doc->path), _("format not recognized"));
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/****** Module dependend	****/

static void GotoDocPage(WINDOW_DATA *win, hyp_nodenr page)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = doc->data;

	if (hypnode_valid(hyp, page) &&
		(doc->displayed_node == NULL ||
		 page != doc->displayed_node->number))
	{
		AddHistoryEntry(win, doc);
		GotoPage(win, page, 0, FALSE);
	} else
	{
		/* short visual feedback */
		graf_mouse(BUSY_BEE, NULL);
		evnt_timer_gemlib(10);
		graf_mouse(ARROW, NULL);
	}
}

/*** ---------------------------------------------------------------------- ***/

void GotoHelp(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = doc->data;
	GotoDocPage(win, hyp->help_page);
}

/*** ---------------------------------------------------------------------- ***/

void GotoIndex(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = doc->data;
	GotoDocPage(win, hyp->index_page);
}

/*** ---------------------------------------------------------------------- ***/

void GoThisButton(WINDOW_DATA *win, short obj)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = doc->data;
	hyp_nodenr new_node = HYP_NOINDEX;
	hyp_nodenr current_node = doc->getNodeProc(doc);
	gboolean add_to_hist = FALSE;

	switch (obj)
	{
	case TO_NEXT:
		new_node = hyp->indextable[current_node]->next;
		break;
#ifdef TO_NEXT_PHYS
	case TO_NEXT_PHYS:
		new_node = current_node + 1;
		while (hypnode_valid(hyp, new_node) && !(HYP_NODE_IS_TEXT(hyp->indextable[new_node]->type)))
			new_node++;
		break;
#endif
	case TO_PREV:
		new_node = hyp->indextable[current_node]->previous;
		break;
#ifdef TO_PREV_PHYS
	case TO_PREV_PHYS:
		new_node = current_node - 1;
		while (hypnode_valid(hyp, new_node) && !(HYP_NODE_IS_TEXT(hyp->indextable[new_node]->type)))
			new_node--;
		break;
#endif
#ifdef TO_LAST
	case TO_LAST:
		new_node = hyp->last_text_page;
		break;
#endif
#ifdef TO_FIRST
	case TO_FIRST:
		new_node = hyp->first_text_page;
		break;
#endif
	case TO_HOME:
		add_to_hist = TRUE;
		new_node = hyp->indextable[current_node]->toc_index;
		break;
	default:
		new_node = HYP_NOINDEX;
		break;
	}
	
	if (!hypnode_valid(hyp, new_node))
		return;
	
	/* already displaying this page? */
	if (new_node == current_node)
		return;

	/* is node a text page? */
	if (!HYP_NODE_IS_TEXT(hyp->indextable[new_node]->type))
		return;

	if (add_to_hist)
		AddHistoryEntry(win, doc);
	GotoPage(win, new_node, 0, FALSE);
}

/*** ---------------------------------------------------------------------- ***/

void GotoDefaultFile(WINDOW_DATA *win)
{
	char *filename = path_subst(gl_profile.viewer.default_file);
	OpenFileInWindow(win, filename, NULL, HYP_NOINDEX, FALSE, FALSE, FALSE);
	g_free(filename);
}

/*** ---------------------------------------------------------------------- ***/

void GotoCatalog(WINDOW_DATA *win)
{
	char *filename = path_subst(gl_profile.viewer.catalog_file);
	OpenFileInWindow(win, filename, NULL, HYP_NOINDEX, FALSE, FALSE, FALSE);
	g_free(filename);
}
