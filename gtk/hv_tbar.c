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
	
	if (win->m_buttons[TO_MEMORY])
		gtk_widget_set_sensitive(win->m_buttons[TO_MEMORY], doc->buttons.memory);
	if (win->m_buttons[TO_REFERENCES])
		gtk_widget_set_sensitive(win->m_buttons[TO_REFERENCES], doc->buttons.references);
#if 0
	if (win->m_buttons[TO_MENU])
		gtk_widget_set_sensitive(win->m_buttons[TO_MENU], doc->buttons.menu);
#endif
	if (win->m_buttons[TO_LOAD])
		gtk_widget_set_sensitive(win->m_buttons[TO_LOAD], doc->buttons.load);
	if (win->m_buttons[TO_INFO])
		gtk_widget_set_sensitive(win->m_buttons[TO_INFO], doc->buttons.info);
	if (win->m_buttons[TO_BACK])
		gtk_widget_set_sensitive(win->m_buttons[TO_BACK], doc->buttons.back);
	if (win->m_buttons[TO_HISTORY])
		gtk_widget_set_sensitive(win->m_buttons[TO_HISTORY], doc->buttons.history);

	/* is there a catalog file?*/
	if (win->m_buttons[TO_KATALOG])
		gtk_widget_set_sensitive(win->m_buttons[TO_KATALOG], !empty(gl_profile.viewer.catalog_file));

	/* next buttons are type specific */
	if (win->m_buttons[TO_PREVIOUS])
		gtk_widget_set_sensitive(win->m_buttons[TO_PREVIOUS], doc->buttons.previous);
	if (win->m_buttons[TO_HOME])
	 	gtk_widget_set_sensitive(win->m_buttons[TO_HOME], doc->buttons.home);
	if (win->m_buttons[TO_NEXT])
		gtk_widget_set_sensitive(win->m_buttons[TO_NEXT], doc->buttons.next);
	if (win->m_buttons[TO_INDEX])
		gtk_widget_set_sensitive(win->m_buttons[TO_INDEX], doc->buttons.index);
	if (win->m_buttons[TO_REFERENCES])
		gtk_widget_set_sensitive(win->m_buttons[TO_REFERENCES], doc->buttons.references);
	if (win->m_buttons[TO_HELP])
		gtk_widget_set_sensitive(win->m_buttons[TO_HELP], doc->buttons.help);
	if (win->m_buttons[TO_SAVE])
		gtk_widget_set_sensitive(win->m_buttons[TO_SAVE], doc->buttons.save);

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
	
	UNUSED(menu);
	gtk_widget_translate_coordinates(win->m_buttons[pos->obj], win->hwnd, 0, 0, &x, &y);
	gdk_window_get_origin(gtk_widget_get_window(win->hwnd), &wx, &wy);
	*xret = x + wx;
	*yret = y + wy;
	*push_in = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/* Handle mouse click on toolbar */
void ToolbarClick(DOCUMENT *doc, enum toolbutton obj, int button)
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
		{
			char *filename = path_subst(gl_profile.viewer.catalog_file);
			OpenFileSameWindow(win, filename, NULL, FALSE, FALSE);
			g_free(filename);
		}
		break;
	case TO_REFERENCES:
		HypExtRefPopup(doc, button);
		break;
	case TO_HELP:
		GotoHelp(doc);
		break;
	case TO_HISTORY:
		HistoryPopup(doc, button);
		break;
	case TO_BACK:
		GoBack(doc);
		break;
	case TO_NEXT:
	case TO_PREVIOUS:
	case TO_HOME:
		GoThisButton(doc, obj);
		break;
	case TO_MEMORY:
		MarkerPopup(doc, button);
		break;
	case TO_INFO:
		ProgrammInfos(doc);
		break;
#if 0
	case TO_MENU:
		break;
#endif
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
