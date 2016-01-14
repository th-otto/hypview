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

static void history_selected(GtkWidget *w, void *user_data)
{
	DOCUMENT *old_doc = (DOCUMENT *)user_data;
	WINDOW_DATA *win = old_doc->window;
	void *psel = g_object_get_data(G_OBJECT(w), "item-num");
	int sel = (int)(intptr_t)psel;
	int i;
	HISTORY *entry;

	if (sel >= 0)
	{
		HISTORY *selected_entry = history;
		DOCUMENT *new_doc = old_doc;
		hyp_nodenr page;
		long line;

		/* Find selected entry */
		i = 0;
		while (selected_entry)
		{
			if (selected_entry->win == win)
			{
				if (sel == i)
					break;
				i++;
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

						/* add new document at the beginning  */
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

/*** ---------------------------------------------------------------------- ***/

void HistoryPopup(DOCUMENT *old_doc, int button, guint32 event_time)
{
	WINDOW_DATA *win = old_doc->window;
	int i;
	GtkWidget *menu;
	struct popup_pos popup_pos;
	HISTORY *entry = history;

	if (!win->m_buttons[TO_HISTORY])
		return;
	
	menu = gtk_menu_new();
	if (g_object_is_floating(menu))
		g_object_ref_sink(menu);
	
	while (entry)
	{
		if (entry->win == win)
		{
			GtkWidget *item = gtk_menu_item_new_with_label(entry->title);
			g_object_set_data(G_OBJECT(item), "item-num", (void *)(intptr_t)i);
			g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(history_selected), old_doc);
			gtk_widget_show(item);
			gtk_menu_append(menu, item);
			i++;
		}
		entry = entry->next;
	}
	
	if (i == 0)
	{
		/*
		 * no history entries found? we should not get here,
		 * the toolbar entry should not have been selectable
		 */
		gtk_widget_unref(menu);
		return;
	}
	
	popup_pos.doc = old_doc;
	popup_pos.obj = TO_HISTORY;
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, position_popup, &popup_pos, button, event_time);
	gtk_widget_unref(menu);
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
