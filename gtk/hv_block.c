#include "hv_gtk.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void gtk_text_view_select_all(GtkWidget *text_view)
{
	GtkTextIter start_iter, end_iter;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	
	gtk_text_buffer_get_bounds(buffer, &start_iter, &end_iter);
    gtk_text_buffer_select_range(buffer, &start_iter, &end_iter);
}

/*** ---------------------------------------------------------------------- ***/

void BlockOperation(DOCUMENT *doc, enum blockop num)
{
	/* YYY */
	WINDOW_DATA *win = doc->window;

	switch (num)
	{
	case CO_SAVE:
		SelectFileSave(doc);
		break;
	case CO_BACK:
		GoBack(doc);
		break;
	case CO_COPY:
		BlockCopy(doc);
		break;
	case CO_PASTE:
		if (doc->buttons.searchbox)
			AutoLocatorPaste(doc);
		else
			BlockPaste(win, gl_profile.viewer.clipbrd_new_window);
		break;
	case CO_SELECT_ALL:
		gtk_text_view_select_all(win->text_view);
		break;
	case CO_SEARCH:
		Hypfind(doc, TRUE);
		break;
	case CO_SEARCH_AGAIN:
		Hypfind(doc, FALSE);
		break;
	case CO_DELETE_STACK:
		RemoveAllHistoryEntries(win);
		ToolbarUpdate(doc, TRUE);
		break;
	case CO_SWITCH_FONT:
		SwitchFont(doc);
		break;
	case CO_SELECT_FONT:
		SelectFont(doc);
		break;
	case CO_REMARKER:
		printf("NYI: remarker\n");
		break;
	case CO_PRINT:
		printf("NYI: print\n");
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void BlockSelectAll(DOCUMENT *doc, BLOCK *b)
{
	WINDOW_DATA *win = doc->window;
	gtk_text_view_select_all(win->text_view);
	UNUSED(b);
}

/*** ---------------------------------------------------------------------- ***/

void BlockCopy(DOCUMENT *doc)
{
	WINDOW_DATA *win = doc->window;
	GtkClipboard *clipboard = gtk_widget_get_clipboard(win->text_view, GDK_SELECTION_CLIPBOARD);
	if (!gtk_text_buffer_get_has_selection(win->text_buffer))
		gtk_text_view_select_all(win->text_view);
	gtk_text_buffer_copy_clipboard(win->text_buffer, clipboard);
}

/*** ---------------------------------------------------------------------- ***/

void BlockPaste(WINDOW_DATA *win, gboolean new_window)
{
	/* YYY */
	GtkClipboard *clipboard = gtk_widget_get_clipboard(GTK_WIDGET(win->text_view), GDK_SELECTION_CLIPBOARD);
	UNUSED(clipboard);
	UNUSED(new_window);
	printf("NYI: paste\n");
}

/*** ---------------------------------------------------------------------- ***/

void BlockAsciiSave(DOCUMENT *doc, const char *path)
{
	/* YYY */
	UNUSED(doc);
	UNUSED(path);
}
