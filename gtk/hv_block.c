#include "hv_gtk.h"
#include "hypdebug.h"
#include <sys/wait.h>

static volatile int remarker_pid = -1;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static gboolean check_remarker(void *userdata)
{
	UNUSED(userdata);
	if (remarker_pid > 0)
	{
		int retval = 0;
		int ret;
		
		if ((ret = waitpid(remarker_pid, &retval, WNOHANG)) >= 0)
		{
			if (ret > 0 && WIFEXITED(retval))
				remarker_pid = -1;
		} else if (errno == ECHILD)
		{
			remarker_pid = -1;
		}
	}
	if (remarker_pid < 0)
		return FALSE; /* remove timeout handler */
	return TRUE; /* keep checking for process to finish */
}

/*** ---------------------------------------------------------------------- ***/

int StartRemarker(GtkHypviewWindow *win, remarker_mode mode, gboolean quiet)
{
	char *path;
	const char *argv[5];
	int argc = 0;
	
	if (remarker_pid > 0 || mode == remarker_check)
		return remarker_pid;
	
	path = path_subst(gl_profile.remarker.path);
	
	if (empty(path))
	{
		if (!quiet)
			show_message(GTK_WIDGET(win), _("Error"), _("No path to REMARKER configured"), FALSE);
	} else
	{
		char *nodename = NULL;
		
		argv[argc++] = path;
		if (mode == remarker_startup)
		{
			argv[argc++] = "-t";
		} else if (mode == remarker_top && win)
		{
			DOCUMENT *doc = win->data;
			argv[argc++] = "-r";
			argv[argc++] = doc->path;
			if (doc->type == HYP_FT_HYP)
			{
				HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
				nodename = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[win->displayed_node->number]->name, STR0TERM);
				argv[argc++] = nodename;
			}
		}
		argv[argc] = NULL;
		if ((remarker_pid = hyp_utf8_spawnvp(P_NOWAIT, 1, argv)) < 0)
		{
			if (!quiet)
			{
				char *str = g_strdup_printf(_("Can not execute\n'%s'\n%s"), path, hyp_utf8_strerror(errno));
				show_message(GTK_WIDGET(win), _("Error"), str, FALSE);
				g_free(str);
			}
		} else
		{
			g_timeout_add(500, check_remarker, NULL);
		}
		g_free(nodename);
	}
	g_free(path);
	return remarker_pid;
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
		gl_profile.viewer.use_xfont = !gl_profile.viewer.use_xfont && gl_profile.viewer.xfont_name != NULL;
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(gtk_action_group_get_action(win->action_group, "altfont")), gl_profile.viewer.use_xfont);
		break;
	case CO_SELECT_FONT:
		SelectFont(win);
		break;
	case CO_REMARKER:
		StartRemarker(win, remarker_top, FALSE);
		ToolbarUpdate(win, FALSE);
		break;
	case CO_PRINT:
		HYP_DBG(("NYI: print"));
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

static void paste_clipboard(GtkClipboard *clipboard, const char *txt, void *userdata)
{
	WINDOW_DATA *win = (WINDOW_DATA *)userdata;
	const char *path = "$CLIPBRD";
	DOCUMENT *doc;
	DOCUMENT *prev_doc;
	gboolean add_to_hist = TRUE;
	FMT_ASCII *ascii;
	gboolean new_window;
	size_t len;
	
	UNUSED(clipboard);
	if (txt == NULL)
		return;
	len = strlen(txt);
	ascii = (FMT_ASCII *)g_malloc(sizeof(FMT_ASCII) + len);
	if (ascii == NULL)
		return;
	memcpy(ascii->start, txt, len);
	ascii->start[len] = '\0';
	
	/* create a new document */
	doc = g_new0(DOCUMENT, 1);

	doc->path = g_strdup(path);
	doc->buttons.load = TRUE;
	doc->type = HYP_FT_UNKNOWN;
	doc->ref_count = 1;
	
	new_window = FALSE;
	if (!win)
	{
		win = gtk_hypview_window_new(doc, FALSE);
		new_window = TRUE;
		add_to_hist = FALSE;
		prev_doc = NULL;
	} else
	{
		prev_doc = win->data;
		win->data = doc;
	}
	
	ascii->length = len;
	if (AsciiCalcLines(doc, ascii) < 0)
	{
		g_free(doc);
		win->data = prev_doc;
		if (new_window)
			gtk_widget_destroy(GTK_WIDGET(win));
		return;
	}
	if (add_to_hist)
		AddHistoryEntry(win, prev_doc);
	ReInitWindow(win, TRUE);
	hv_win_open(win);
}

/*** ---------------------------------------------------------------------- ***/

void BlockPaste(WINDOW_DATA *win, gboolean new_window)
{
	GtkClipboard *clipboard = gtk_widget_get_clipboard(GTK_WIDGET(win->text_view), GDK_SELECTION_CLIPBOARD);
	
	if (new_window)
		win = NULL;
	gtk_clipboard_request_text(clipboard, paste_clipboard, win);
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
