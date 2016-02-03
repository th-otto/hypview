#include "hv_gtk.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void GotoPage(WINDOW_DATA *win, hyp_nodenr num, long line, gboolean calc)
{
	DOCUMENT *doc = win->data;
	if (doc->gotoNodeProc(win, NULL, num))
	{
		if (calc)
		{
		}
		doc->start_line = line;
	}
	ReInitWindow(win, FALSE);
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

			hypdoc_unref(old_doc);
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

static void history_selected(GtkWidget *w, WINDOW_DATA *win)
{
	DOCUMENT *old_doc = win->data;
	void *psel = g_object_get_data(G_OBJECT(w), "item-num");
	int sel = (int)(intptr_t)psel;
	int i;

	if (sel >= 0)
	{
		DOCUMENT *new_doc;
		hyp_nodenr page;
		long line;

		/* Find selected entry */
		i = 0;
		while ((new_doc = RemoveHistoryEntry(win, &page, &line)) != NULL)
		{
			hypdoc_unref(old_doc);
			win->data = old_doc = new_doc;
			if (sel == i)
				break;
			i++;
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

/*** ---------------------------------------------------------------------- ***/

void HistoryPopup(WINDOW_DATA *win, int button, guint32 event_time)
{
	int i;
	GtkWidget *menu;
	struct popup_pos popup_pos;
	HISTORY *entry = win->history;

	if (!win->m_buttons[TO_HISTORY])
		return;
	
	menu = gtk_menu_new();
	if (g_object_is_floating(menu))
		g_object_ref_sink(menu);
	
	i = 0;
	while (entry)
	{
		GtkWidget *item = gtk_menu_item_new_with_label(entry->title);
		g_object_set_data(G_OBJECT(item), "item-num", (void *)(intptr_t)i);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(history_selected), win);
		gtk_widget_show(item);
		gtk_menu_append(menu, item);
		i++;
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
	
	popup_pos.window = win;
	popup_pos.obj = TO_HISTORY;
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, position_popup, &popup_pos, button, event_time);
	gtk_widget_unref(menu);
}


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/


/****** Module dependend	****/

static void GotoDocPage(WINDOW_DATA *win, hyp_nodenr page)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;

	if (hypnode_valid(hyp, page) &&
		(win->displayed_node == NULL ||
		 page != win->displayed_node->number))
	{
		AddHistoryEntry(win, doc);
		GotoPage(win, page, 0, FALSE);
	}
}

/*** ---------------------------------------------------------------------- ***/

void GotoHelp(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	GotoDocPage(win, hyp->help_page);
}

/*** ---------------------------------------------------------------------- ***/

void GotoIndex(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	GotoDocPage(win, hyp->index_page);
}

/*** ---------------------------------------------------------------------- ***/

void GotoCatalog(WINDOW_DATA *win)
{
	char *filename = path_subst(gl_profile.viewer.catalog_file);
	OpenFileInWindow(win, filename, NULL, HYP_NOINDEX, FALSE, FALSE, FALSE);
	g_free(filename);
}

/*** ---------------------------------------------------------------------- ***/

void GotoDefaultFile(WINDOW_DATA *win)
{
	char *filename = path_subst(gl_profile.viewer.default_file);
	OpenFileInWindow(win, filename, hyp_default_main_node_name, HYP_NOINDEX, TRUE, FALSE, FALSE);
	g_free(filename);
}

/*** ---------------------------------------------------------------------- ***/

void GoThisButton(WINDOW_DATA *win, enum toolbutton obj)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	hyp_nodenr new_node = HYP_NOINDEX;
	hyp_nodenr current_node = doc->getNodeProc(win);
	gboolean add_to_hist = FALSE;
	
	switch (obj)
	{
	case TO_NEXT:
		new_node = hyp->indextable[current_node]->next;
		break;
	case TO_NEXT_PHYS:
		new_node = current_node + 1;
		while (hypnode_valid(hyp, new_node) && !(HYP_NODE_IS_TEXT(hyp->indextable[new_node]->type)))
			new_node++;
		break;
	case TO_PREV:
		new_node = hyp->indextable[current_node]->previous;
		break;
	case TO_PREV_PHYS:
		new_node = current_node - 1;
		while (hypnode_valid(hyp, new_node) && !(HYP_NODE_IS_TEXT(hyp->indextable[new_node]->type)))
			new_node--;
		break;
	case TO_LAST:
		new_node = hyp->last_text_page;
		break;
	case TO_FIRST:
		new_node = hyp->first_text_page;
		break;
	case TO_HOME:
		add_to_hist = TRUE;
		new_node = hyp->indextable[current_node]->toc_index;
		break;
	case TO_BACK:
		GoBack(win);
		break;
	case TO_KATALOG:
		GotoCatalog(win);
		break;
	case TO_INDEX:
		GotoIndex(win);
		break;
	case TO_HELP:
		GotoHelp(win);
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
