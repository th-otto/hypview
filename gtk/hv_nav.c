#include "hv_gtk.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void GotoPage(DOCUMENT *doc, hyp_nodenr num, long line, gboolean calc)
{
	WINDOW_DATA *win = doc->window;

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

			/* if old document is not used anymore...    */
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

void HistoryPopup(DOCUMENT *old_doc, int button)
{
	/* YYY */
	UNUSED(old_doc);
	UNUSED(button);
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

void GoThisButton(DOCUMENT *doc, enum toolbutton obj)
{
	WINDOW_DATA *win = doc->window;
	HYP_DOCUMENT *hyp = doc->data;
	hyp_nodenr new_node = HYP_NOINDEX;

	if (obj == TO_NEXT)
		new_node = hyp->indextable[doc->displayed_node->number]->next;
	else if (obj == TO_PREVIOUS)
		new_node = hyp->indextable[doc->displayed_node->number]->previous;
	else if (obj == TO_HOME)
		new_node = hyp->indextable[doc->displayed_node->number]->toc_index;

	if (!hypnode_valid(hyp, new_node))
		return;
	
	/* already displaying this page? */
	if (new_node == doc->getNodeProc(doc))
		return;

	/* is node a text page? */
	if (!HYP_NODE_IS_TEXT(hyp->indextable[new_node]->type))
		return;

	if (obj == TO_HOME)
		AddHistoryEntry(win);
	GotoPage(doc, new_node, 0, FALSE);
}
