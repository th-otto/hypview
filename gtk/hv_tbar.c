#include "hv_gtk.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void toolbar_redraw(WINDOW_DATA *win)
{
	if (win->toolbar)
		gtk_widget_queue_draw(win->toolbar);
}

/*** ---------------------------------------------------------------------- ***/

void ToolbarUpdate(WINDOW_DATA *win, gboolean redraw)
{
	DOCUMENT *doc = win->data;
	
	/* autolocator active? */
	if (doc->buttons.searchbox && win->searchbox)
	{
		const char *search = gtk_entry_get_text(GTK_ENTRY(win->searchentry));
		if (!empty(search))
		{
			gtk_widget_show(win->searchbox);
			return;
		}
	}
	if (win->searchbox)
		gtk_widget_hide(win->searchbox);
	
	doc->buttons.back = TRUE;
	doc->buttons.history = TRUE;
	doc->buttons.bookmarks = TRUE;
	doc->buttons.menu = TRUE;
	doc->buttons.info = TRUE;
	doc->buttons.save = TRUE;
	doc->buttons.remarker_running = StartRemarker(win, remarker_check, TRUE) >= 0;
	doc->buttons.remarker = TRUE;
	
	if (win->history == NULL)
	{
		doc->buttons.back = FALSE;
		doc->buttons.history = FALSE;
	}
	
	if (win->action_group)
	{
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "bookmarks"), doc->buttons.bookmarks);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "open"), doc->buttons.load);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "info"), doc->buttons.info);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "back"), doc->buttons.back);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "history"), doc->buttons.history);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "clearstack"), doc->buttons.history);
	
		/* is there a catalog file? */
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "catalog"), !empty(gl_profile.viewer.catalog_file));
	
		/* next buttons are type specific */
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "prevlogpage"), doc->buttons.previous);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "prevphyspage"), doc->buttons.prevphys);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "toc"), doc->buttons.home);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "nextlogpage"), doc->buttons.next);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "nextphyspage"), doc->buttons.nextphys);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "firstpage"), doc->buttons.first);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "lastpage"), doc->buttons.last);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "index"), doc->buttons.index);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "xref"), doc->buttons.references);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "help"), doc->buttons.help);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "save"), doc->buttons.save);
		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "recompile"), doc->buttons.save && doc->type == HYP_FT_HYP);

		gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "remarker"), doc->buttons.remarker);
	}
	if (win->m_buttons[TO_REMARKER])
	{
		if (doc->buttons.remarker_running)
			gtk_widget_show(win->m_buttons[TO_REMARKER]);
		else
			gtk_widget_hide(win->m_buttons[TO_REMARKER]);
	}
	
	if (redraw)
	{
		toolbar_redraw(win);
	}	
}

/*** ---------------------------------------------------------------------- ***/

void position_popup(GtkMenu *menu, gint *xret, gint *yret, gboolean *push_in, void *data)
{
	struct popup_pos *pos = (struct popup_pos *)data;
	int x, y;
	int wx, wy;
	WINDOW_DATA *win = pos->window;
	GtkAllocation a;
	
	UNUSED(menu);
	gtk_widget_get_allocation(win->m_buttons[pos->obj], &a);
	gtk_widget_translate_coordinates(win->m_buttons[pos->obj], GTK_WIDGET(win), 0, a.height, &x, &y);
	gdk_window_get_origin(gtk_widget_get_window(GTK_WIDGET(win)), &wx, &wy);
	*xret = x + wx;
	*yret = y + wy;
	*push_in = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/* Handle mouse click on toolbar */
void ToolbarClick(WINDOW_DATA *win, enum toolbutton obj, int button, guint32 event_time)
{
	if ((int)obj < 0)
		RemoveSearchBox(win);
	else if (!win->m_buttons[obj] || !gtk_widget_get_sensitive(win->m_buttons[obj]))
		return;

	CheckFiledate(win);		/* Check if file has changed */
	
	switch (obj)
	{
	case TO_LOAD:
		SelectFileLoad(win);
		break;
	case TO_SAVE:
		BlockOperation(win, CO_SAVE);
		break;
	case TO_INDEX:
		GotoIndex(win);
		break;
	case TO_CATALOG:
		GotoCatalog(win);
		break;
	case TO_REFERENCES:
		HypExtRefPopup(win, button, event_time);
		break;
	case TO_HELP:
		GotoHelp(win);
		break;
	case TO_HISTORY:
		HistoryPopup(win, button, event_time);
		break;
	case TO_BACK:
		GoBack(win);
		break;
	case TO_NEXT:
	case TO_PREV:
	case TO_NEXT_PHYS:
	case TO_PREV_PHYS:
	case TO_FIRST:
	case TO_LAST:
	case TO_HOME:
		GoThisButton(win, obj);
		break;
	case TO_BOOKMARKS:
		MarkerPopup(win, button, event_time);
		break;
	case TO_INFO:
		DocumentInfos(win);
		break;
	case TO_REMARKER:
		BlockOperation(win, CO_REMARKER);
		break;
	case TO_MAX:
	default:
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void RemoveSearchBox(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;

	/* Is the autolocator/search box displayed? */
	if (doc->buttons.searchbox && win->searchentry)
	{
		doc->buttons.searchbox = FALSE;	/* disable it */
		gtk_entry_set_text(GTK_ENTRY(win->searchentry), "");			/* clear autolocator string */

		ToolbarUpdate(win, TRUE);	/* update toolbar */
	}
}
