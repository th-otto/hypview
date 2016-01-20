#define GDK_DISABLE_DEPRECATION_WARNINGS
#include "hv_gtk.h"

/*
 * filters for file chooser.
 * Filters are separated by newlines.
 * In each filter, the display string is separated from the extension list by '|'.
 * If a display string is not defined, the extension list is used.
 * An extension list may specify several wildcard specifications separated by spaces.
 */
#define IDS_SELECT_HYPERTEXT _("*.hyp|Hypertext files (*.hyp)\n*.*|All files (*.*)\n")
#define IDS_SELECT_TEXTFILES _("*.txt|Text files (*.txt)\n*.*|All files (*.*)\n")


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void choose_file_response(GtkWidget *dialog, gint response_id, gpointer user_data)
{
	UNUSED(dialog);
	UNUSED(user_data);
	
	switch (response_id)
	{
	case GTK_RESPONSE_OK:
	case GTK_RESPONSE_APPLY:
	case GTK_RESPONSE_ACCEPT:
	case GTK_RESPONSE_YES:
		break;
	case GTK_RESPONSE_CANCEL:
	case GTK_RESPONSE_CLOSE:
	case GTK_RESPONSE_NO:
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

static gboolean filter_exe(const GtkFileFilterInfo *filter_info, gpointer userdata)
{
	GtkFileChooserDialog *selector = GTK_FILE_CHOOSER_DIALOG(userdata);
	GFile *file;
	gboolean ret = FALSE;
	
	UNUSED(selector);
	
	if (!(filter_info->contains & GTK_FILE_FILTER_FILENAME))
		return FALSE;
	if (g_pattern_match_simple("*.exe", filter_info->filename))
		return TRUE;
	if (g_pattern_match_simple("*.cmd", filter_info->filename))
		return TRUE;
	if (g_pattern_match_simple("*.bat", filter_info->filename))
		return TRUE;
	if (g_pattern_match_simple("*.desktop", filter_info->filename))
		return TRUE;
	file = g_file_new_for_path(filter_info->filename);
	if (file != NULL)
	{
		GFileInfo *fileinfo = g_file_query_info(file, "access::*", G_FILE_QUERY_INFO_NONE, NULL, NULL);
		if (fileinfo != NULL)
		{
			if (g_file_info_has_attribute(fileinfo, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE))
				ret = TRUE;
			g_object_unref(fileinfo);
		}
		g_object_unref(file);
	}
	return ret;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean choose_file(GtkWidget *parent, char **name, gboolean must_exist, const char *title, const char *filter)
{
	GtkFileChooserDialog *selector;
	int resp;
	gboolean save = !must_exist;
	gboolean res = FALSE;
	
	if (parent)
		parent = gtk_widget_get_toplevel(parent);

	selector = GTK_FILE_CHOOSER_DIALOG(
		gtk_file_chooser_dialog_new(title,
		GTK_WINDOW(parent),
		save ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		save ? GTK_STOCK_SAVE : GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL));
	g_object_set_data(G_OBJECT(selector), "hypview_window_type", NO_CONST("fileselector"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(selector), save ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(selector), TRUE);
	
	if (!empty(*name))
	{
		const char *base = hyp_basename(*name);
		if (*base == '*')
		{
			char *dir = g_strndup(*name, base - *name);
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(selector), dir);
			g_free(dir);
		} else
		{
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(selector), *name);
		}
	}
	
	/*
	 * create list of filters
	 */
	if (!empty(filter))
	{
		char **filterlist = g_strsplit(filter, "\n", 0);
		int i, j;
		char *displayname;
		GtkFileFilter *filefilter;
		char **patterns;
		
		for (i = 0; filterlist != NULL && filterlist[i] != NULL; i++)
		{
			if (empty(filterlist[i]))
				continue;
			filefilter = gtk_file_filter_new();
			displayname = strchr(filterlist[i], '|');
			if (displayname != NULL)
			{
				*displayname++ = '\0';
				gtk_file_filter_set_name(filefilter, displayname);
			} else
			{
				gtk_file_filter_set_name(filefilter, filterlist[i]);
			}
			patterns = g_strsplit(filterlist[i], " ", 0);
			for (j = 0; patterns != NULL && patterns[j] != NULL; j++)
			{
				if (strcmp(patterns[j], "*.*") == 0)
					strcpy(patterns[j], "*");
				if (strcmp(patterns[j], "*.exe") == 0)
					gtk_file_filter_add_custom(filefilter, GTK_FILE_FILTER_FILENAME, filter_exe, selector, NULL);
				else
					gtk_file_filter_add_pattern(filefilter, patterns[j]);
			}
			g_strfreev(patterns);
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(selector), filefilter);
		}
		g_strfreev(filterlist);
	}
	
	g_signal_connect(G_OBJECT(selector), "response", G_CALLBACK(choose_file_response), NULL);
	gtk_window_set_modal(GTK_WINDOW(selector), TRUE);

	gtk_window_set_transient_for(GTK_WINDOW(selector), GTK_WINDOW(parent));
	gtk_widget_show(GTK_WIDGET(selector));
	resp = gtk_dialog_run(GTK_DIALOG(selector));
	if (resp == GTK_RESPONSE_ACCEPT ||
		resp == GTK_RESPONSE_OK ||
		resp == GTK_RESPONSE_YES ||
		resp == GTK_RESPONSE_APPLY)
	{
		g_free(*name);
		*name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(selector));
		res = TRUE;
	}
	gtk_widget_destroy(GTK_WIDGET(selector));
		  	
	return res;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

WINDOW_DATA *SelectFileLoad(WINDOW_DATA *win)
{
	char *name;
	char *subst;
	GtkWidget *parent = win ? win->hwnd : NULL;
	char **paths;
	
	if (win)
	{
		DOCUMENT *doc = (DOCUMENT *)win->data;
		name = g_strdup(doc->path);
	} else
	{
		subst = path_subst(gl_profile.general.path_list);
		paths = g_strsplit(subst, G_SEARCHPATH_SEPARATOR_S, 0);
		name = g_build_filename(paths[0], "*.hyp", NULL);
		g_strfreev(paths);
		g_free(subst);
	}
	
	if (choose_file(parent, &name, TRUE, _("Open Hypertext..."), IDS_SELECT_HYPERTEXT))
	{
		win = OpenFileInWindow(win, name, NULL, HYP_NOINDEX, FALSE, FALSE, FALSE);
	}
	g_free(name);
	return win;
}

/*** ---------------------------------------------------------------------- ***/

void SelectFileSave(DOCUMENT *doc)
{
	char **paths;
	char *filepath;
	char *subst;
	GtkWidget *parent = doc ? ((WINDOW_DATA *)(doc->window))->hwnd : NULL;

	subst = path_subst(gl_profile.general.path_list);
	paths = g_strsplit(subst, G_SEARCHPATH_SEPARATOR_S, 0);
	filepath = replace_ext(doc->path, NULL, ".txt");

	if (choose_file(parent, &filepath, FALSE, _("Save ASCII text as"), IDS_SELECT_TEXTFILES))
	{
		int ret;

		ret = hyp_utf8_open(filepath, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (ret >= 0)
		{
			hyp_utf8_close(ret);
			if (ask_yesno(GTK_WINDOW(parent), _("This file exists already.\nDo you want to replace it?")))
				ret = -1;
		}
		if (ret < 0)
			BlockAsciiSave(doc, filepath);
	}
	g_free(filepath);
	g_strfreev(paths);
	g_free(subst);
}
