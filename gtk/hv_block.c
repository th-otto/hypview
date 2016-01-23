#include "hv_gtk.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void StartRemarker(gboolean quiet)
{
	char *path = path_subst(gl_profile.remarker.path);
	const char *argv[2];
	
	if (empty(path))
	{
		if (!quiet)
			show_message(_("Error"), _("No path to REMARKER configured"), FALSE);
	} else
	{
		argv[0] = path;
		argv[1] = NULL;
		if (hyp_utf8_spawnvp(P_NOWAIT, 1, argv) < 0 && !quiet)
		{
			char *str = g_strdup_printf(_("Can not execute\n'%s'\n%s"), path, hyp_utf8_strerror(errno));
			show_message(_("Error"), str, FALSE);
			g_free(str);
		}
	}
	g_free(path);
}

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
		Hypfind(doc, FALSE);
		break;
	case CO_SEARCH_AGAIN:
		Hypfind(doc, TRUE);
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
		StartRemarker(FALSE);
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
	int handle;
	WINDOW_DATA *win = doc->window;
	GtkTextIter start, end;
	char *txt;
	size_t len, ret;
	
	handle = hyp_utf8_open(path, O_WRONLY | O_TRUNC | O_CREAT, HYP_DEFAULT_FILEMODE);
	if (handle < 0)
	{
		FileErrorErrno(path);
	} else
	{
		if (!gtk_text_buffer_get_has_selection(win->text_buffer))
			gtk_text_view_select_all(win->text_view);
		gtk_text_buffer_get_selection_bounds(win->text_buffer, &start, &end);
		txt = gtk_text_buffer_get_text(win->text_buffer, &start, &end, FALSE);
		len = txt ? strlen(txt) : 0;
		ret = hyp_utf8_write(handle, txt, len);
		if (ret != len)
		{
			HYP_DBG(("Error %s writing file. Abort.", strerror(errno)));
		}
		if (len != 0 && txt[len - 1] != '\n')
			write(handle, "\n", 1);
		g_free(txt);
		hyp_utf8_close(handle);
	}
}
