/*
 * HypView - (c)      - 2019 Thorsten Otto
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, see <http://www.gnu.org/licenses/>.
 */

#define GDK_DISABLE_DEPRECATION_WARNINGS
#include "hv_gtk.h"
#include "hypdebug.h"

/*
 * filters for file chooser.
 * Filters are separated by newlines.
 * In each filter, the display string is separated from the extension list by '|'.
 * If a display string is not defined, the extension list is used.
 * An extension list may specify several wildcard specifications separated by spaces.
 */
static char const hypertext_file_filter[] = N_("*.hyp *.HYP|Hypertext files (*.hyp)\n*.*|All files (*.*)\n");
static char const text_file_filter[] = N_("*.txt *.TXT|Text files (*.txt)\n*.*|All files (*.*)\n");
static char const stg_file_filter[] = N_("*.stg *.STG|ST-Guide files (*.stg)\n*.*|All files (*.*)\n");
#ifdef WITH_PDF
static char const pdf_file_filter[] = N_("*.pdf *.PDF|PDF files (*.pdf)\n*.*|All files (*.*)\n");
#endif


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

void hv_file_selector_add_filter(GtkWidget *selector, const char *filter)
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

/*** ---------------------------------------------------------------------- ***/

void hv_file_selector_add_hypfilter(GtkWidget *selector)
{
	hv_file_selector_add_filter(selector, _(hypertext_file_filter));
}

/*** ---------------------------------------------------------------------- ***/

static gboolean choose_file(GtkWidget *parent, char **name, gboolean save, const char *title, const char *filter)
{
	GtkWidget *selector;
	int resp;
	gboolean res = FALSE;
	
	if (parent)
		parent = gtk_widget_get_toplevel(parent);

	selector = gtk_file_chooser_dialog_new(title,
		GTK_WINDOW(parent),
		save ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		save ? GTK_STOCK_SAVE : GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	g_object_set_data(G_OBJECT(selector), "hypview_window_type", NO_CONST("fileselector"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(selector), save ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(selector), TRUE);
	if (save)
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(selector), TRUE);
		
	if (!empty(*name))
	{
		const char *base = hyp_basename(*name);
		if (*base == '*')
		{
			char *dir = g_strndup(*name, base - *name);
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(selector), dir);
			g_free(dir);
		} else if (save)
		{
			char *dir = g_strndup(*name, base - *name);
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(selector), dir);
			g_free(dir);
			gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(selector), base);
		} else
		{
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(selector), *name);
		}
	}
	
	/*
	 * create list of filters
	 */
	if (!empty(filter))
		hv_file_selector_add_filter(selector, filter);
	
	g_signal_connect(G_OBJECT(selector), "response", G_CALLBACK(choose_file_response), NULL);
	gtk_window_set_modal(GTK_WINDOW(selector), TRUE);

	gtk_window_set_transient_for(GTK_WINDOW(selector), GTK_WINDOW(parent));
	gtk_widget_show(GTK_WIDGET(selector));
	resp = gtk_dialog_run(GTK_DIALOG(selector));
	if (IsResponseOk(resp))
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
	GtkWidget *parent = GTK_WIDGET(win);
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
	
	if (choose_file(parent, &name, FALSE, _("Open Hypertext..."), _(hypertext_file_filter)))
	{
		hv_recent_add(name);
		win = OpenFileInWindow(win, name, NULL, 0, TRUE, FALSE, FALSE);
	}
	g_free(name);
	return win;
}

/*** ---------------------------------------------------------------------- ***/

char *SelectFileSave(WINDOW_DATA *win, hyp_filetype type)
{
	DOCUMENT *doc = win->data;
	char *filepath;
	GtkWidget *parent = GTK_WIDGET(win);
	const char *defext;
	const char *filter;
	const char *title;
	
	switch (type)
	{
	case HYP_FT_ASCII:
	case HYP_FT_CHEADER:
		defext = HYP_EXT_TXT;
		filter = _(text_file_filter);
		title = _("Save ASCII text as");
		break;
	case HYP_FT_STG:
	case HYP_FT_GUIDE:
		defext = HYP_EXT_STG;
		filter = _(stg_file_filter);
		title = _("Recompile to");
		break;
	case HYP_FT_PDF:
#ifdef WITH_PDF
		defext = HYP_EXT_PDF;
		filter = _(pdf_file_filter);
		title = _("Recompile to PDF");
		break;
#else
		return NULL;
#endif
	case HYP_FT_NONE:
	case HYP_FT_UNKNOWN:
	case HYP_FT_LOADERROR:
	case HYP_FT_BINARY:
	case HYP_FT_HYP:
	case HYP_FT_REF:
	case HYP_FT_RSC:
	case HYP_FT_IMAGE:
	case HYP_FT_HTML:
	case HYP_FT_XML:
	case HYP_FT_HTML_XML:
	case HYP_FT_TREEVIEW:
	default:
		return NULL;
	}
	
	if (gl_profile.output.output_dir)
	{
		char *name = replace_ext(hyp_basename(doc->path), NULL, defext);
		filepath = g_build_filename(gl_profile.output.output_dir, name, NULL);
		g_free(name);
	} else
	{
		filepath = replace_ext(doc->path, NULL, defext);
	}
	
	if (choose_file(parent, &filepath, TRUE, title, filter))
	{
		g_free(gl_profile.output.output_dir);
		gl_profile.output.output_dir = hyp_path_get_dirname(filepath);
		HypProfile_SetChanged();
	} else
	{
		g_free(filepath);
		filepath = NULL;
	}
	return filepath;
}
