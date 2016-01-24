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

static void SelectAll(WINDOW_DATA *win)
{
	GtkTextIter start_iter, end_iter;
	
	gtk_text_buffer_get_bounds(win->text_buffer, &start_iter, &end_iter);
    gtk_text_buffer_select_range(win->text_buffer, &start_iter, &end_iter);
}

/*** ---------------------------------------------------------------------- ***/

void BlockOperation(WINDOW_DATA *win, enum blockop num)
{
	DOCUMENT *doc = win->data;

	switch (num)
	{
	case CO_SAVE:
		SelectFileSave(win);
		break;
	case CO_BACK:
		GoBack(win);
		break;
	case CO_COPY:
		BlockCopy(win);
		break;
	case CO_PASTE:
		if (doc->buttons.searchbox)
			AutoLocatorPaste(win);
		else
			BlockPaste(win, gl_profile.viewer.clipbrd_new_window);
		break;
	case CO_SELECT_ALL:
		SelectAll(win);
		break;
	case CO_SEARCH:
		Hypfind(win, FALSE);
		break;
	case CO_SEARCH_AGAIN:
		Hypfind(win, TRUE);
		break;
	case CO_DELETE_STACK:
		RemoveAllHistoryEntries(win);
		ToolbarUpdate(win, TRUE);
		break;
	case CO_SWITCH_FONT:
		SwitchFont(win);
		break;
	case CO_SELECT_FONT:
		SelectFont(win);
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

void BlockSelectAll(WINDOW_DATA *win, BLOCK *b)
{
	SelectAll(win);
	UNUSED(b);
}

/*** ---------------------------------------------------------------------- ***/

void BlockCopy(WINDOW_DATA *win)
{
	GtkClipboard *clipboard = gtk_widget_get_clipboard(win->text_view, GDK_SELECTION_CLIPBOARD);
	char *txt;
	GtkTextIter start, end;
	
	if (gtk_text_buffer_get_has_selection(win->text_buffer))
		gtk_text_buffer_get_selection_bounds(win->text_buffer, &start, &end);
	else
		gtk_text_buffer_get_bounds(win->text_buffer, &start, &end);
	txt = gtk_text_buffer_get_text(win->text_buffer, &start, &end, FALSE);
	gtk_clipboard_set_text(clipboard, txt, -1);
	g_free(txt);
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

void BlockAsciiSave(WINDOW_DATA *win, const char *path, GtkTextIter *start, GtkTextIter *end)
{
	int handle;
	char *txt;
	size_t len, ret;
	
	handle = hyp_utf8_open(path, O_WRONLY | O_TRUNC | O_CREAT, HYP_DEFAULT_FILEMODE);
	if (handle < 0)
	{
		FileErrorErrno(path);
	} else
	{
		txt = gtk_text_buffer_get_text(win->text_buffer, start, end, FALSE);
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
