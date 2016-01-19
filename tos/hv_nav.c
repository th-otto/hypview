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

void GotoPage(DOCUMENT *doc, hyp_nodenr num, long line, gboolean calc)
{
	WINDOW_DATA *win = doc->window;

	graf_mouse(BUSY_BEE, NULL);
	if (doc->gotoNodeProc(doc, NULL, num))
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
	ReInitWindow(doc);
}

/*** ---------------------------------------------------------------------- ***/

void GoBack(DOCUMENT *old_doc)
{
	WINDOW_DATA *win = old_doc->window;
	DOCUMENT *new_doc;
	hyp_nodenr page;
	long line;

	new_doc = old_doc;
	if (RemoveHistoryEntry(&new_doc, &page, &line))
	{
		/* changing file? */
		if (new_doc != old_doc)
		{
			int ret;

			/* if old document is not used anymore... */
			if (!CountDocumentHistoryEntries(old_doc))
			{
				win->data = old_doc->next;
				HypCloseFile(old_doc);		/* ...close document */
			} else
			{
				DOCUMENT *prev_doc = old_doc;

				old_doc->closeProc(old_doc);

				/* place new document at top of list */
				while (prev_doc->next != new_doc)
					prev_doc = prev_doc->next;
				prev_doc->next = new_doc->next;
				new_doc->next = old_doc;
			}

			/* load new file */
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

		if (new_doc->type >= 0)			/* known filetype? */
			GotoPage(new_doc, page, line, FALSE);	/* switch to requested page */
		else
			FileError(hyp_basename(new_doc->path), _("format not recognized"));
	}
}

/*** ---------------------------------------------------------------------- ***/

void HistoryPopup(DOCUMENT *old_doc, short x, short y)
{
	WINDOW_DATA *win = old_doc->window;
	OBJECT *tree = rs_tree(EMPTYPOPUP);
	short i, h;
	short sel;
	size_t len = 0;
	HISTORY *entry = history;

	i = tree[ROOT].ob_head;
	h = 0;
	while (entry && i != ROOT)
	{
		if (entry->win == win)
		{
			tree[i].ob_flags = OF_SELECTABLE | OF_TOUCHEXIT;
			tree[i].ob_spec.free_string = entry->title;
			len = max(strlen(entry->title) + 1, len);
			h = max(h, tree[i].ob_y + tree[i].ob_height);
			i = tree[i].ob_next;
		}
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
		HISTORY *selected_entry = history;
		DOCUMENT *new_doc = old_doc;
		hyp_nodenr page;
		long line;

		/* Find selected entry */
		i = tree[ROOT].ob_head;
		while (selected_entry && i != ROOT)
		{
			if (selected_entry->win == win)
			{
				if (sel == i)
					break;
				i = tree[i].ob_next;
			}
			selected_entry = selected_entry->next;
		}

		/* Release previous document */
		old_doc->closeProc(old_doc);

		/* Remove unnecessary history entries */
		entry = history;
		for (;;)
		{
			if (entry->win == win)
			{
				RemoveHistoryEntry(&new_doc, &page, &line);

				/* Switch document? -> close old document */
				if (new_doc != old_doc)
				{
					if (!CountDocumentHistoryEntries(old_doc))
					{
						win->data = old_doc->next;
						HypCloseFile(old_doc);	/* close document */
					} else
					{
						DOCUMENT *prev_doc = old_doc;

						/* add new document at the beginning */
						while (prev_doc->next != new_doc)
							prev_doc = prev_doc->next;
						prev_doc->next = new_doc->next;
						new_doc->next = old_doc;
					}
					old_doc = new_doc;
				}

				if (entry == selected_entry)
					break;
				entry = history;
			} else
			{
				entry = entry->next;
			}
		}

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
			GotoPage(new_doc, page, line, FALSE);	/* jump to page */
		else
			FileError(hyp_basename(new_doc->path), _("format not recognized"));
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/****** Module dependend	****/

static void GotoDocPage(DOCUMENT *doc, hyp_nodenr page)
{
	WINDOW_DATA *win = doc->window;
	HYP_DOCUMENT *hyp = doc->data;

	if (hypnode_valid(hyp, page) &&
		(doc->displayed_node == NULL ||
		 page != doc->displayed_node->number))
	{
		AddHistoryEntry(win);
		GotoPage(doc, page, 0, FALSE);
	} else
	{
		/* short visual feedback */
		graf_mouse(BUSY_BEE, NULL);
		evnt_timer_gemlib(10);
		graf_mouse(ARROW, NULL);
	}
}

/*** ---------------------------------------------------------------------- ***/

void GotoHelp(DOCUMENT *doc)
{
	HYP_DOCUMENT *hyp = doc->data;
	GotoDocPage(doc, hyp->help_page);
}

/*** ---------------------------------------------------------------------- ***/

void GotoIndex(DOCUMENT *doc)
{
	HYP_DOCUMENT *hyp = doc->data;
	GotoDocPage(doc, hyp->index_page);
}

/*** ---------------------------------------------------------------------- ***/

void GoThisButton(DOCUMENT *doc, short obj)
{
	WINDOW_DATA *win = doc->window;
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
		AddHistoryEntry(win);
	GotoPage(doc, new_node, 0, FALSE);
}
