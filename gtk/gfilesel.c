#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
/* avoid warnings from G_TYPE_* macros */
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wc++-compat"
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#pragma GCC diagnostic warning "-Wcast-qual"
#pragma GCC diagnostic warning "-Wstrict-prototypes"
#pragma GCC diagnostic warning "-Wc++-compat"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif

#undef ENABLE_NLS

#define _(x) x
#define N_(x) x
#define NC_(Context, String) String

#ifndef GETTEXT_PACKAGE
#define GETTEXT_PACKAGE "gtk+"
#endif

char const gl_program_name[] = "gfilesel";

static char *geom_arg;
static char *caption;
static char *title;
static char *startdir;
static char *filter;
static char *filterindex_str;
static char *parentid;
static char *iconname;
static gboolean multiple;
static gboolean bNokeep;
static gboolean saving;
static gboolean donull;
static gboolean uri;
static const char *fignore;
static gboolean bShowVersion;
static gboolean bShowHelp;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static GOptionEntry const options[] = {
	{ "geometry", 0, 0, G_OPTION_ARG_STRING, &geom_arg, N_("Sets the client geometry of the main window"), NC_("option", "GEOMETRY") },
	{ "caption", 0, 0, G_OPTION_ARG_STRING, &caption, N_("The name of the application"), NC_("option", "TITLE") },
	{ "prompt", 0, 0, G_OPTION_ARG_STRING, &title, N_("The caption of the dialog widget"), NC_("option", "TITLE") },
	{ "title", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_STRING, &title, N_("The caption of the dialog widget"), NC_("option", "TITLE") },
	{ "icon", 0, 0, G_OPTION_ARG_STRING, &iconname, N_("The application icon"), NC_("option", "FILE") },
	{ "location", 0, 0, G_OPTION_ARG_STRING, &startdir, N_("The directory to start in"), NC_("option", "DIR") },
	{ "filter", 0, 0, G_OPTION_ARG_STRING, &filter, N_("Sets the filter(s) to be used"), NC_("option", "FILTER") },
	{ "filterindex", 0, 0, G_OPTION_ARG_STRING, &filterindex_str, N_("Sets the filter to start with"), NC_("option", "INDEX") },
	{ "parent", 0, 0, G_OPTION_ARG_STRING, &parentid, N_("ID of the parent window"), NC_("option", "WINDOW-ID") },
	{ "multiple", 0, 0, G_OPTION_ARG_NONE, &multiple, N_("Allow multiple filenames to be selected"), NULL },
	{ "save", 0, 0, G_OPTION_ARG_NONE, &saving, N_("create a \"Save As\" type dialog"), NULL },
	{ "uri", 0, 0, G_OPTION_ARG_NONE, &uri, N_("print filenames as URI"), NULL },
	{ "nokeep", 0, 0, G_OPTION_ARG_NONE, &bNokeep, N_("Sets whether the filename/url should be kept when changing directories"), NULL },
	{ "null", '0', 0, G_OPTION_ARG_NONE, &donull, N_("Use null character instead of newline to separate filenames"), NULL },
	{ "version", 0, 0, G_OPTION_ARG_NONE, &bShowVersion, N_("Show version information and exit"), NULL },
	{ "help", '?', G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_NONE, &bShowHelp, N_("Show help information and exit"), NULL },
	
	{ NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};


static gboolean ParseCommandLine(int *argc, char ***argv)
{
	GOptionContext *context;
	GOptionGroup *gtk_group;
	GError *error = NULL;
	gboolean retval;
	GOptionGroup *main_group;
	
	/*
	 * Glib's option parser requires global variables,
	 * copy options read from INI file there.
	 */
	bShowVersion = FALSE;
	bShowHelp = FALSE;
	
	gtk_group = gtk_get_option_group(FALSE);
	context = g_option_context_new(_("[FILE [CHAPTER]]"));
	main_group = g_option_group_new(NULL, NULL, NULL, NULL, NULL);
	g_option_context_set_main_group(context, main_group);
	g_option_context_set_summary(context, _("GTK Fileselector"));
	g_option_context_add_group(context, gtk_group);
	g_option_context_add_main_entries(context, options, GETTEXT_PACKAGE);
	
	g_option_context_set_help_enabled(context, FALSE);
	
	retval = g_option_context_parse(context, argc, argv, &error);
	if (bShowHelp)
	{
		char *msg = g_option_context_get_help(context, FALSE, NULL);
		fprintf(stderr, "%s\n", msg);
		g_free(msg);
	}
	g_option_context_free(context);
	
	if (retval == FALSE)
	{
		char *msg = g_strdup_printf("%s: %s", gl_program_name, error && error->message ? error->message : _("error parsing command line"));
		fprintf(stdout, "%s\n", msg);
		g_free(msg);
		g_clear_error(&error);
		return FALSE;
	}
	
	return TRUE;
}


static void show_version(void)
{
}


static GtkWidget *create_foreign_gtk(const char *windowid)
{
	GtkWidget *toplevel = 0;
	GdkWindow *parent;
	GdkNativeWindow window;

	window = strtoul(windowid, NULL, 0);	

	parent = gdk_window_foreign_new(window);
	if (parent != NULL)
	{
		toplevel = gtk_window_new(GTK_WINDOW_POPUP);
		gtk_widget_realize(toplevel);
		gtk_widget_set_window(toplevel, parent);
	}
	if (toplevel == NULL)
	{
		fprintf(stderr, _("can't create GDK window for window-id %s\n"), windowid);
	}
	return toplevel;
}


/*** ---------------------------------------------------------------------- ***/

static gboolean filter_exe(const GtkFileFilterInfo *filter_info, gpointer userdata)
{
	GtkFileChooserDialog *selector = GTK_FILE_CHOOSER_DIALOG(userdata);
	GFile *file;
	gboolean ret = FALSE;
	
	(void) selector;
	
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


static gboolean filter_fignore(const GtkFileFilterInfo *filter_info, gpointer userdata)
{
	GtkFileChooserDialog *selector = GTK_FILE_CHOOSER_DIALOG(userdata);
	gboolean ret = TRUE;
	char **patterns;
	int i;
	
	(void) selector;
	if (!(filter_info->contains & GTK_FILE_FILTER_FILENAME))
		return FALSE;

	patterns = g_strsplit(fignore, ":", 0);
	for (i = 0; patterns[i] != NULL && ret; i++)
		if (g_str_has_suffix(filter_info->filename, patterns[i]))
			ret = FALSE;
	g_strfreev(patterns);
	return ret;
}
	
static void set_ignore(GtkFileFilter *filefilter, GtkWidget *selector)
{
	gtk_file_filter_add_custom(filefilter, GTK_FILE_FILTER_FILENAME, filter_fignore, selector, NULL);
}


int main (int argc, char* argv[])
{
	int i;
	GtkWidget *parent = NULL;
	GtkFileChooserAction action;
	GtkWidget *selector;
	int resp;
	GSList *filenames, *l;
	char *str;
	gboolean didOutput;
	
	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-session") == 0)
			exit(EXIT_SUCCESS);
	}

	setvbuf(stdout, NULL, _IONBF, 0);

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, xs_get_locale_dir());
	textdomain(GETTEXT_PACKAGE);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif

	if (!ParseCommandLine(&argc, &argv))
		return EXIT_FAILURE;
	
	if (bShowHelp)
	{
		return EXIT_SUCCESS;
	}
	if (bShowVersion)
	{
		show_version();
		return EXIT_SUCCESS;
	}
	
#if !GTK_CHECK_VERSION(3, 0, 0)
	gtk_set_locale();
#endif
	gdk_threads_init();
	gtk_init(&argc, &argv);
	GDK_THREADS_ENTER();
	
	if (startdir == NULL)
		startdir = g_get_current_dir();
	
	if (parentid != NULL)
		parent = create_foreign_gtk(parentid);
	
	if (title == NULL)
		title = g_strdup(saving ? _("Save As") : _("Open"));
	if (caption != NULL)
		title = g_strconcat(caption, " - ", title, NULL);
	
	if (saving)
	{
		multiple = FALSE;
		action = GTK_FILE_CHOOSER_ACTION_SAVE;
	} else
	{
		action = GTK_FILE_CHOOSER_ACTION_OPEN;
	}
	
	if (fignore == NULL)
	{
		fignore = getenv("FIGNORE");
		if (fignore == NULL)
			fignore = ".o:.lo:.a:~";
	}
	
	selector = gtk_file_chooser_dialog_new(title,
		GTK_WINDOW(parent),
		action,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		saving ? GTK_STOCK_SAVE : GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(selector), !uri);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(selector), startdir);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(selector), multiple);
	
	if (iconname)
	{
		char *filename = g_strconcat("/usr/share/pixmaps/", iconname, NULL);
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
		if (pixbuf)
			gtk_window_set_icon(GTK_WINDOW(selector), pixbuf);
	}
	
	/*
	 * create list of filters
	 */
	if (filter != NULL)
	{
		char **filterlist = g_strsplit(filter, "\n", 0);
		int i, j;
		char *displayname;
		GtkFileFilter *filefilter;
		char **patterns;
		GtkFileFilter *selectedfilter = NULL;
		int filterindex = 0;
		
		if (filterindex_str)
			filterindex = (int)strtol(filterindex_str, NULL, 0);
		if (filterindex < 0 || filterindex >= (gint)g_strv_length(filterlist))
			filterindex = 0;
		
		for (i = 0; filterlist != NULL && filterlist[i] != NULL; i++)
		{
			if (filterlist[i] == NULL || filterlist[i][0] == '\0')
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
				else if (strcmp(patterns[j], "*") == 0)
					set_ignore(filefilter, selector);
				else
					gtk_file_filter_add_pattern(filefilter, patterns[j]);
			}
			g_strfreev(patterns);
			g_object_set_data(G_OBJECT(filefilter), "num", GINT_TO_POINTER(i));
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(selector), filefilter);
			if (i == filterindex)
				selectedfilter = filefilter;
		}
		g_strfreev(filterlist);
		if (selectedfilter)
			gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(selector), selectedfilter);
	} else
	{
		GtkFileFilter *filefilter;

		filterindex_str = NULL;
		filefilter = gtk_file_filter_new();
		set_ignore(filefilter, selector);
		gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(selector), filefilter);
	}

	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(selector), GTK_WINDOW(parent));

	g_signal_connect_swapped(G_OBJECT(selector), "response", G_CALLBACK(gtk_widget_hide), selector);
	gtk_window_set_modal(GTK_WINDOW(selector), TRUE);
	
	gtk_widget_show_all(selector);
	gtk_window_present(GTK_WINDOW(selector));
	resp = gtk_dialog_run(GTK_DIALOG(selector));
	
	if (resp == GTK_RESPONSE_OK ||
		resp == GTK_RESPONSE_YES ||
		resp == GTK_RESPONSE_APPLY ||
		resp == GTK_RESPONSE_ACCEPT)
	{
		if (uri)
			filenames = gtk_file_chooser_get_uris(GTK_FILE_CHOOSER(selector));
		else
			filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(selector));
		
		didOutput = FALSE;
		for (l = filenames; l; l = l->next)
		{
			str = (char *)l->data;
			if (str && *str)
			{
				if (donull)
				{
					write(1, str, strlen(str) + 1);
				} else
				{
					puts(str);
				}
				didOutput = TRUE;
			}
		}
	
		if (filterindex_str && didOutput)
		{
			GtkFileFilter *filefilter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(selector));
			int filterindex = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(filefilter), "num"));
			str = g_strdup_printf("filter: %d", filterindex);
			if (donull)
			{
				write(1, str, strlen(str) + 1);
			} else
			{
				puts(str);
			}
		}
	}
	GDK_THREADS_LEAVE();
			
	return 0;
}
