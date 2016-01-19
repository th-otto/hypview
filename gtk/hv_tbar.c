#include "hv_gtk.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void toolbar_redraw(DOCUMENT *doc)
{
	WINDOW_DATA *win = doc->window;
	gtk_widget_queue_draw(win->toolbar);
}

/*** ---------------------------------------------------------------------- ***/

void ToolbarUpdate(DOCUMENT *doc, gboolean redraw)
{
	WINDOW_DATA *win = doc->window;
	
	/* autolocator active? */
	if (doc->buttons.searchbox && doc->autolocator != NULL)
	{
		gtk_widget_show(win->searchbox);
		gtk_entry_set_text(GTK_ENTRY(win->searchentry), doc->autolocator);
		return;
	}
	gtk_widget_hide(win->searchbox);
	
	doc->buttons.back = TRUE;
	doc->buttons.history = TRUE;
	doc->buttons.memory = TRUE;
	doc->buttons.menu = TRUE;
	doc->buttons.info = TRUE;
	
	if (CountWindowHistoryEntries(win) == 0)
	{
		doc->buttons.back = FALSE;
		doc->buttons.history = FALSE;
	}
	
	gtk_action_set_sensitive(gtk_action_group_get_action(win->action_group, "bookmarks"), doc->buttons.memory);
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

	if (redraw)
	{
		toolbar_redraw(doc);
	}	
}

/*** ---------------------------------------------------------------------- ***/

void position_popup(GtkMenu *menu, gint *xret, gint *yret, gboolean *push_in, void *data)
{
	struct popup_pos *pos = (struct popup_pos *)data;
	int x, y;
	int wx, wy;
	DOCUMENT *doc = pos->doc;
	WINDOW_DATA *win = doc->window;
	GtkAllocation a;
	
	UNUSED(menu);
	gtk_widget_get_allocation(win->m_buttons[pos->obj], &a);
	gtk_widget_translate_coordinates(win->m_buttons[pos->obj], win->hwnd, 0, a.height, &x, &y);
	gdk_window_get_origin(gtk_widget_get_window(win->hwnd), &wx, &wy);
	*xret = x + wx;
	*yret = y + wy;
	*push_in = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/* Handle mouse click on toolbar */
void ToolbarClick(DOCUMENT *doc, enum toolbutton obj, int button, guint32 event_time)
{
	WINDOW_DATA *win = doc->window;
	
	if ((int)obj < 0)
		RemoveSearchBox(doc);
	else if (!win->m_buttons[obj] || !gtk_widget_get_sensitive(win->m_buttons[obj]))
		return;

	if (gl_profile.viewer.check_time)
		CheckFiledate(doc);		/* Check if file has changed */
	
	switch (obj)
	{
	case TO_LOAD:
		SelectFileLoad(win);
		break;
	case TO_SAVE:
		SelectFileSave(doc);
		break;
	case TO_INDEX:
		GotoIndex(doc);
		break;
	case TO_KATALOG:
		GotoCatalog(win);
		break;
	case TO_REFERENCES:
		HypExtRefPopup(doc, button, event_time);
		break;
	case TO_HELP:
		GotoHelp(doc);
		break;
	case TO_HISTORY:
		HistoryPopup(doc, button, event_time);
		break;
	case TO_BACK:
		GoBack(doc);
		break;
	case TO_NEXT:
	case TO_PREV:
	case TO_NEXT_PHYS:
	case TO_PREV_PHYS:
	case TO_FIRST:
	case TO_LAST:
	case TO_HOME:
		GoThisButton(doc, obj);
		break;
	case TO_MEMORY:
		MarkerPopup(doc, button, event_time);
		break;
	case TO_INFO:
		ProgrammInfos(doc);
		break;
	default:
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void RemoveSearchBox(DOCUMENT *doc)
{
	/* Is the autolocator/search box displayed? */
	if (doc->buttons.searchbox)
	{
		doc->buttons.searchbox = FALSE;	/* disable it */
		*doc->autolocator = 0;			/* clear autolocator string */

		ToolbarUpdate(doc, TRUE);	/* update toolbar */
	}
}
