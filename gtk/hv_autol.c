#include "hv_gtk.h"
#include "hypdebug.h"
#include "gdkkeysyms.h"


#define IsModifierKey(key) \
	((key) == GDK_KEY_Shift_L || \
	 (key) == GDK_KEY_Shift_R || \
	 (key) == GDK_KEY_Control_L || \
	 (key) == GDK_KEY_Control_R || \
	 (key) == GDK_KEY_Caps_Lock || \
	 (key) == GDK_KEY_Shift_Lock || \
	 (key) == GDK_KEY_Meta_L || \
	 (key) == GDK_KEY_Meta_R || \
	 (key) == GDK_KEY_Alt_L || \
	 (key) == GDK_KEY_Alt_R || \
	 (key) == GDK_KEY_Super_L || \
	 (key) == GDK_KEY_Super_R || \
	 (key) == GDK_KEY_Hyper_L || \
	 (key) == GDK_KEY_Hyper_R || \
	 ((key) >= GDK_KEY_ISO_Lock && (key) <= GDK_KEY_ISO_Last_Group_Lock) || \
	 (key) == GDK_KEY_Scroll_Lock)

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * Initialize and activate the autolocator.
 * Returns position of next character
 */
static void AutolocatorInit(DOCUMENT *doc)
{
	/* search box already active? */
	if (!doc->buttons.searchbox)
		doc->buttons.searchbox = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/* Update the autolocator and start a search */
static void AutolocatorUpdate(DOCUMENT *doc, long start_line)
{
	WINDOW_DATA *win = doc->window;
	long line = start_line;
	const char *search;
	
	if (!doc->buttons.searchbox)
		return;

	gtk_widget_hide(win->strnotfound);

	/* if autolocator is not empty... */
	search = gtk_entry_get_text(GTK_ENTRY(win->searchentry));
	if (!empty(search))
	{
		line = doc->autolocProc(doc, start_line, search);
	}

	if (line >= 0)
	{
		long topline = hv_win_topline(win);
		if (line != topline)
		{
			hv_win_scroll_to_line(win, line);
		}
	} else
	{
		gtk_widget_show(win->strnotfound);
		gdk_display_beep(gtk_widget_get_display(win->hwnd));
	}
}

/*** ---------------------------------------------------------------------- ***/

/* add a new character to the Autolocator and start search */
gboolean AutolocatorKey(DOCUMENT *doc, GdkEventKey *event)
{
	WINDOW_DATA *win = doc->window;
	long line = hv_win_topline(win);
	gint len;
	
	if (!event->keyval)
		return FALSE;
	if (IsModifierKey(event->keyval))
		return FALSE;

	AutolocatorInit(doc);
	doc->autolocator_dir = 1;

	switch (event->keyval)
	{
	case GDK_KEY_BackSpace:			/* Backspace */
		len = gtk_entry_get_text_length(GTK_ENTRY(win->searchentry));
		if (len > 0)
		{
			char *txt = gtk_editable_get_chars(GTK_EDITABLE(win->searchentry), 0, len - 1);
			gtk_entry_set_text(GTK_ENTRY(win->searchentry), txt);
			g_free(txt);
		}
		break;
	case GDK_KEY_KP_Enter:
	case GDK_KEY_Return:			/* Return */
		if (event->state & GDK_SHIFT_MASK)
		{
			doc->autolocator_dir = 0;
			line--;
		} else
		{
			line++;
		}
		break;
	case GDK_KEY_Escape:			/* Escape */
		len = gtk_entry_get_text_length(GTK_ENTRY(win->searchentry));
		if (len > 0)
		{
			gtk_entry_set_text(GTK_ENTRY(win->searchentry), "");
		} else
		{
			RemoveSearchBox(doc);
		}
		break;
	case GDK_KEY_space:
		/* ignore space at start of string */
		len = gtk_entry_get_text_length(GTK_ENTRY(win->searchentry));
		if (len != 0)
		{
			char *txt = gtk_editable_get_chars(GTK_EDITABLE(win->searchentry), 0, -1);
			char *newtxt = g_strconcat(txt, " ", NULL);
			gtk_entry_set_text(GTK_ENTRY(win->searchentry), newtxt);
			g_free(newtxt);
			g_free(txt);
		}
		break;
	default:
		if (event->keyval >= ' ' && event->length > 0)
		{
			char *txt = gtk_editable_get_chars(GTK_EDITABLE(win->searchentry), 0, -1);
			char *newtxt = g_strconcat(txt, event->string, NULL);
			gtk_entry_set_text(GTK_ENTRY(win->searchentry), newtxt);
			g_free(newtxt);
			g_free(txt);
		}
		break;
	}

	ToolbarUpdate(doc, FALSE);
	AutolocatorUpdate(doc, line);

	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/* insert contents of clipboard in autolocator. */
void AutoLocatorPaste(DOCUMENT *doc)
{
	WINDOW_DATA *win = doc->window;
	
	if (!doc->buttons.searchbox)
		return;
	gtk_editable_paste_clipboard(GTK_EDITABLE(win->searchentry));
}
