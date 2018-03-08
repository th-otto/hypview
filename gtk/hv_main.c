#include "hv_gtk.h"

#include "../icons/hypview.h"

typedef struct _shell_dialog
{
	char *m_geometry;

	char *m_file_source;
	char **mru_list;
	int n_mru;
	int max_mru;
	
	/* path to our help file */
	char *hypview_helpfile;
	
	GtkWidget *hwnd;
	GtkWidget *history_menu;
	GtkWidget *main_hbox;
	GtkWidget *main_vbox;
	GtkWidget *text_window;
	GtkWidget *text_view;
	
	GtkTooltips *tooltips;
} SHELL_DIALOG;

char const gl_program_name[] = "hypview";
char const gl_compile_date[12] = __DATE__;
static gboolean have_console;
static int gtk_inited;
static SHELL_DIALOG *global_sd;

static char *geom_arg;
static gboolean bShowVersion;
static gboolean bShowHelp;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

char *gl_program_version(void)
{
	return g_strdup(HYPVIEW_VERSION);
}

/* ------------------------------------------------------------------------- */

static gboolean NOINLINE add_mru(SHELL_DIALOG *sd, const char *filename)
{
	int i, j;
	char *tmp;
	char **new_mru;
	
	if (!empty(filename))
	{
		for (i = 0; i < sd->n_mru; i++)
		{
			if (filename_cmp(filename, sd->mru_list[i]) == 0)
			{
				tmp = sd->mru_list[i];
				for (j = i-1; j >= 0; j--)
					sd->mru_list[j+1] = sd->mru_list[j];
				sd->mru_list[0] = tmp;
				return TRUE;
			}
		}
		tmp = g_strdup(filename);
		if (tmp == NULL)
			return FALSE;
		if (sd->max_mru != 0 && sd->n_mru >= sd->max_mru)
		{
			--sd->n_mru;
			g_freep(&sd->mru_list[sd->n_mru]);
		} else
		{
			if (sd->mru_list == NULL)
			{
				new_mru = g_new(char *, 1);
			} else
			{
				new_mru = g_renew(char *, sd->mru_list, sd->n_mru + 1);
			}
			if (new_mru == NULL)
			{
				g_free(tmp);
				return FALSE;
			}
			sd->mru_list = new_mru;
		}
		for (i = sd->n_mru - 1; i >= 0; i--)
			sd->mru_list[i + 1] = sd->mru_list[i];
		sd->mru_list[0] = tmp;
		++sd->n_mru;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean on_history_open(GtkWidget *widget, gpointer user_data)
{
	SHELL_DIALOG *sd = (SHELL_DIALOG *)user_data;
	UNUSED(widget);
	UNUSED(sd);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void NOINLINE reset_mru(SHELL_DIALOG *sd)
{
	int i;
	GtkWidget *item;
	
	gtk_container_foreach(GTK_CONTAINER(sd->history_menu), (GtkCallback)gtk_widget_destroy, NULL);
	for (i = 0; i < sd->n_mru; i++)
	{
		item = gtk_menu_item_new_with_label(sd->mru_list[i]);
		gtk_widget_show(item);
		gtk_container_add(GTK_CONTAINER(sd->history_menu), item);
		g_signal_connect((gpointer)item, "activate", G_CALLBACK(on_history_open), (gpointer) sd);
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void check_console(void)
{
#ifdef G_PLATFORM_WIN32
	CONSOLE_SCREEN_BUFFER_INFO ci;
	
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &ci))
	{
#ifdef G_OS_UNIX
		/* G_PLATFORM_WIN32 + G_OS_UNIX = CYGWIN */
		have_console = TRUE;
#endif
	} else
	{
		have_console = TRUE;
	}
#endif /* G_PLATFORM_WIN32 */

#ifdef G_OS_UNIX
	have_console = isatty(0);
#endif
}

/*** ---------------------------------------------------------------------- ***/

static int toplevels_open_except(GtkWidget *top)
{
	GList *tops, *l;
	int num_open;
	
	tops = gtk_window_list_toplevels();
	for (l = tops, num_open = 0; l != NULL; l = l->next)
		if (l->data != top)
		{
			if (gtk_widget_get_visible(GTK_WIDGET(l->data)) &&
				g_object_get_data(G_OBJECT(l->data), "hypview_window_type") != NULL)
			{
				num_open++;
			}
		}
	g_list_free(tops);
	return num_open;
}

/*** ---------------------------------------------------------------------- ***/

void check_toplevels(GtkWidget *toplevel)
{
	int num_open;
	
	if ((num_open = toplevels_open_except(toplevel)) == 0)
	{
		gtk_main_quit();
	}
}

/*** ---------------------------------------------------------------------- ***/

static void message_destroyed(GtkWidget *w, gpointer user_data)
{
	UNUSED(user_data);
	check_toplevels(w);
}

/*** ---------------------------------------------------------------------- ***/

static GtkWidget *show_message(SHELL_DIALOG *sd, const char *title, const char *text, gboolean big)
{
	GtkWidget *dialog, *vbox, *label;
	GtkWidget *button;
	GtkWidget *scroll;
	
	dialog = gtk_dialog_new();
	g_object_set_data(G_OBJECT(dialog), "hypview_window_type", NO_CONST("message"));
	gtk_signal_connect(GTK_OBJECT(dialog), "destroy", GTK_SIGNAL_FUNC(message_destroyed), sd);
	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox, TRUE, TRUE, 0);

	label = gtk_label_new(text);
	gtk_label_set_selectable(GTK_LABEL(label), TRUE);
	if (big)
	{
		scroll = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll), label);
		gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	} else
	{
		gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);
	}
	
	button = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_CANCEL);
	gtk_signal_connect_object(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), dialog);
	
	gtk_button_box_set_layout(GTK_BUTTON_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))), GTK_BUTTONBOX_SPREAD);

	gtk_widget_set_can_default(button, TRUE);
	gtk_widget_grab_default(button);

	if (big)
		gtk_window_set_default_size(GTK_WINDOW(dialog), 640, 400);
	gtk_widget_show_all(dialog);

	return dialog;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean init_gtk(SHELL_DIALOG *sd);

/*
 * write a message to stdout/stderr if we are attached to a tty,
 * otherwise pop up a dialog
 */
static void write_console(SHELL_DIALOG *sd, const char *s, gboolean to_stderr, gboolean big)
{
	if (have_console || sd == NULL || !init_gtk(sd))
	{
		char *desc;
		
		fflush(stdout);
		fflush(stderr);
		desc = g_locale_from_utf8(s, -1, NULL, NULL, NULL);
		if (desc)
			fputs(desc, to_stderr ? stderr : stdout);
		else
			fputs(s, to_stderr ? stderr : stdout);
		g_free(desc);
	} else
	{
		show_message(sd, to_stderr ? _("Error") : _("Warning"), s, big);
	}
}

/*** ---------------------------------------------------------------------- ***/

static gboolean init_gtk(SHELL_DIALOG *sd)
{
	int argc;
	char *argv[6];
	char **argvp;
	
	if (gtk_inited == 0)
	{
		gtk_set_locale();
		gdk_threads_init();
		argc = 0;
		argv[argc++] = NO_CONST(gl_program_name);
		argv[argc] = NULL;
		argvp = argv;
		if (!gtk_init_check(&argc, &argvp))
		{
			char *msg;
			
			gtk_inited = 2;
			msg = g_strdup_printf(_("%s: error initializing GTK (wrong display?)\n"), gl_program_name);
			write_console(sd, msg, TRUE, FALSE);
			g_free(msg);
		} else
		{
			gtk_inited = 1;
		}
	}
	return gtk_inited == 1;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void FileError(const char *path, const char *msg)
{
	char *filename;
	char *str;

	filename = hyp_path_get_basename(path);
	str = g_strdup_printf(_("File <%s>\n%s"), path, msg);
	write_console(global_sd, str, TRUE, FALSE);
	g_free(str);
	g_free(filename);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static SHELL_DIALOG *NOINLINE ShellDialog_New(void)
{
	SHELL_DIALOG *sd;

	sd = g_new0(SHELL_DIALOG, 1);
	if (sd == NULL)
		return NULL;
	
	sd->m_geometry = NULL;

	sd->mru_list = NULL;
	sd->n_mru = 0;
	sd->max_mru = 0;
	
	sd->hypview_helpfile = g_strdup(_("hypview_en.chm"));

	return sd;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void NOINLINE ChooseHypertext(SHELL_DIALOG *sd)
{
	if (choose_file(sd->hwnd, &sd->m_file_source, TRUE, _("Open Hypertext..."), IDS_SELECT_HYPERTEXT))
	{
		add_mru(sd, sd->m_file_source);
		reset_mru(sd);
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

GdkPixbuf *app_icon(void)
{
	return gdk_pixbuf_new_from_inline(-1, hypview_icon_data, FALSE, NULL);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/* NYI */
static void About(GtkWidget *parent)
{
	UNUSED(parent);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void NOINLINE ShellDialog_Delete(SHELL_DIALOG *sd)
{
	int i;
	
	if (sd != NULL)
	{
		if (sd->mru_list != NULL)
		{
			for (i = 0; i < sd->n_mru; i++)
				g_freep(&sd->mru_list[i]);
			g_free(sd->mru_list);
		}
		sd->mru_list = NULL;
		g_freep(&sd->m_file_source);
		g_freep(&sd->hypview_helpfile);
		HypProfile_Delete();

		if (sd->tooltips != NULL)
			gtk_object_unref(GTK_OBJECT(sd->tooltips));
		sd->tooltips = NULL;

		g_free(sd);
		global_sd = NULL;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void shell_destroyed(GtkWidget *w, void *userdata)
{
	SHELL_DIALOG *sd = (SHELL_DIALOG *)userdata;

	UNUSED(w);
	Help_Exit();
	ShellDialog_Delete(sd);
}

/*** ---------------------------------------------------------------------- ***/

static void quit_force(GtkWidget *w, void *userdata)
{
	SHELL_DIALOG *sd = (SHELL_DIALOG *)userdata;

	UNUSED(w);
	gtk_widget_destroy(sd->hwnd);
	check_toplevels(NULL);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static gboolean NOINLINE ReadProfile(SHELL_DIALOG *sd)
{
	int i;
	char *path;
	char name[80];
	Profile *inifile;
	
	HypProfile_Load();
	inifile = gl_profile.profile;
	
	sd->m_geometry = g_strdup_printf("%dx%d+%d+%d",
		gl_profile.viewer.win_w,
		gl_profile.viewer.win_h,
		gl_profile.viewer.win_x,
		gl_profile.viewer.win_y);
	Profile_ReadString(inifile, "Shell", "Source", &sd->m_file_source);

	i = 0;
	for (;;)
	{
		++i;
		sprintf(name, "Source%d", i);
		path = NULL;
		if (!Profile_ReadString(inifile, "MRU", name, &path))
			break;
		add_mru(sd, path);
		g_free(path);
	}
	return TRUE;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static gboolean NOINLINE WriteProfile(SHELL_DIALOG *sd)
{
	int i;
	char name[40];
	gboolean ret;
	Profile *inifile;
	
	inifile = gl_profile.profile;
	
	Profile_WriteString(inifile, "Shell", "Source", sd->m_file_source);

	for (i = 0; i < sd->n_mru; i++)
	{
		sprintf(name, "Source%d", sd->n_mru - i);
		Profile_WriteString(inifile, "MRU", name, sd->mru_list[i]);
	}
	
	ret = HypProfile_Save(TRUE);
	
	if (ret == FALSE)
	{
		char *msg = g_strdup_printf(_("Can't write Settings:\n%s\n%s\nQuit anyway?"), Profile_GetFilename(inifile), g_strerror(errno));
		show_dialog(sd->hwnd, GTK_STOCK_DIALOG_ERROR, msg, quit_force, sd);
		g_free(msg);
		return FALSE;
	}
	return TRUE;
}
	
/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void on_select_source(GtkWidget *widget, gpointer user_data)
{
	SHELL_DIALOG *sd = (SHELL_DIALOG *)user_data;
	UNUSED(widget);
	ChooseHypertext(sd);
}

/*** ---------------------------------------------------------------------- ***/

static void on_help_contents(GtkWidget *widget, gpointer user_data)
{
	SHELL_DIALOG *sd = (SHELL_DIALOG *)user_data;
	UNUSED(widget);
	Help_Contents(sd->hwnd, sd->hypview_helpfile);
}

/*** ---------------------------------------------------------------------- ***/

static void on_help_index(GtkWidget *widget, gpointer user_data)
{
	SHELL_DIALOG *sd = (SHELL_DIALOG *)user_data;
	UNUSED(widget);
	Help_Index(sd->hwnd, sd->hypview_helpfile);
}

/*** ---------------------------------------------------------------------- ***/

static void on_about(GtkWidget *widget, gpointer user_data)
{
	SHELL_DIALOG *sd = (SHELL_DIALOG *)user_data;
	UNUSED(widget);
	About(sd->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

static void on_quit(GtkWidget *widget, gpointer user_data)
{
	SHELL_DIALOG *sd = (SHELL_DIALOG *)user_data;
	if (!WriteProfile(sd))
		return;
	quit_force(widget, sd);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean wm_toplevel_close_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	UNUSED(event);
	on_quit(widget, user_data);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void create_shell_dialog(SHELL_DIALOG *sd)
{
	GtkWidget *vbox, *hbox, *vbox2, *hbox2;
	GtkWidget *menubar;
	GtkWidget *submenu;
	GtkWidget *historymenu;
	GtkWidget *item;
	GtkWidget *image;
	GtkAccelGroup *accel_group;
	GdkPixbuf *icon;
	
	sd->tooltips = gtk_tooltips_new();
	gtk_object_ref(GTK_OBJECT(sd->tooltips));
	gtk_object_sink(GTK_OBJECT(sd->tooltips));
	
	accel_group = gtk_accel_group_new ();
	
	sd->hwnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_object_set_data(G_OBJECT(sd->hwnd), "shell-dialog", sd);
	g_object_set_data(G_OBJECT(sd->hwnd), "hypview_window_type", NO_CONST("shell-window"));
	gtk_window_set_title(GTK_WINDOW(sd->hwnd), _("HypView"));
	icon = app_icon();
	gtk_window_set_icon(GTK_WINDOW(sd->hwnd), icon);
	gdk_pixbuf_unref(icon);
	gtk_window_set_role(GTK_WINDOW(sd->hwnd), "udoshell");
	
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add (GTK_CONTAINER (sd->hwnd), vbox);
 	
	menubar = gtk_menu_bar_new();
	gtk_widget_show(menubar);
	gtk_box_pack_start(GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

	item = gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(menubar), item);
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);

	item = gtk_image_menu_item_new_with_mnemonic(_("Open Hypertext..."));
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	gtk_widget_add_accelerator(item, "activate", accel_group, GDK_O, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	image = gtk_image_new_from_stock("gtk-open", GTK_ICON_SIZE_MENU);
	gtk_widget_show(image);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
	g_signal_connect((gpointer)item, "activate", G_CALLBACK(on_select_source), (gpointer) sd);
 
	item = gtk_menu_item_new_with_mnemonic(_("Open _Recent"));
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	historymenu = sd->history_menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), historymenu);

	item = gtk_separator_menu_item_new();
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	gtk_widget_set_sensitive(item, FALSE);

	item = gtk_image_menu_item_new_from_stock("gtk-quit", accel_group);
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	g_signal_connect((gpointer)item, "activate", G_CALLBACK(on_quit), (gpointer) sd);

	item = gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(menubar), item);
	submenu = gtk_menu_new ();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);

	item = gtk_image_menu_item_new_with_mnemonic (_("_Contents"));
	gtk_widget_show (item);
	gtk_container_add (GTK_CONTAINER (submenu), item);
	image = gtk_image_new_from_stock ("gtk-info", GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	g_signal_connect((gpointer)item, "activate", G_CALLBACK(on_help_contents), (gpointer) sd);

	item = gtk_image_menu_item_new_with_mnemonic (_("_Index"));
	gtk_widget_show (item);
	gtk_container_add (GTK_CONTAINER (submenu), item);
	image = gtk_image_new_from_stock ("gtk-index", GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	g_signal_connect((gpointer)item, "activate", G_CALLBACK(on_help_index), (gpointer) sd);

	item = gtk_separator_menu_item_new ();
	gtk_widget_show (item);
	gtk_container_add (GTK_CONTAINER (submenu), item);
	gtk_widget_set_sensitive (item, FALSE);

	item = gtk_image_menu_item_new_with_mnemonic (_("_About"));
	gtk_widget_show (item);
	gtk_container_add (GTK_CONTAINER (submenu), item);
	image = gtk_image_new_from_stock ("gtk-about", GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	g_signal_connect((gpointer)item, "activate", G_CALLBACK(on_about), (gpointer) sd);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

	vbox2 = sd->main_vbox = gtk_vbox_new (TRUE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

	hbox2 = sd->main_hbox = gtk_hbox_new (TRUE, 0);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox2, TRUE, TRUE, 0);

	sd->text_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sd->text_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sd->text_window), GTK_SHADOW_IN);
	gtk_widget_show(sd->text_window);
	sd->text_view = gtk_text_view_new();
	gtk_widget_show(sd->text_view);
	gtk_container_add(GTK_CONTAINER(sd->text_window), sd->text_view);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(sd->text_view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(sd->text_view), GTK_WRAP_NONE); 
	gtk_widget_set_can_focus(sd->text_view, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox2), sd->text_window, TRUE, TRUE, 0);
	
	reset_mru(sd);
	
	gtk_window_add_accel_group (GTK_WINDOW (sd->hwnd), accel_group);

	gtk_signal_connect(GTK_OBJECT(sd->hwnd), "delete_event", GTK_SIGNAL_FUNC(wm_toplevel_close_cb), (gpointer) sd);
	gtk_signal_connect(GTK_OBJECT(sd->hwnd), "destroy", GTK_SIGNAL_FUNC(shell_destroyed), (gpointer) sd);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static GOptionEntry const options[] = {
	{ "geometry", 0, 0, G_OPTION_ARG_STRING, &geom_arg, N_("Sets the client geometry of the main window"), N_("GEOMETRY") },
	{ "version", 0, 0, G_OPTION_ARG_NONE, &bShowVersion, N_("Show version information and exit"), NULL },
	{ "help", '?', G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_NONE, &bShowHelp, N_("Show help information and exit"), NULL },
	
	{ NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static gboolean NOINLINE ParseCommandLine(SHELL_DIALOG *sd, int *argc, char ***argv)
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
	context = g_option_context_new(_("[FILE]"));
	main_group = g_option_group_new(NULL, NULL, NULL, sd, NULL);
	g_option_context_set_main_group(context, main_group);
	g_option_context_set_summary(context, _("GTK Shell for HypView"));
	g_option_context_add_group(context, gtk_group);
	g_option_context_add_main_entries(context, options, GETTEXT_PACKAGE);
	
	/*
	 * disable automatic handling of --help from Glib because
	 * - the short option 'h' conflicts with our short option for --html
	 * - we may want to redirect the message to a dialog box
	 */
	g_option_context_set_help_enabled(context, FALSE);
	
	retval = g_option_context_parse(context, argc, argv, &error);
	if (bShowHelp)
	{
		char *msg = g_option_context_get_help(context, FALSE, NULL);
		write_console(sd, msg, FALSE, TRUE);
		g_free(msg);
	}
	g_option_context_free(context);
	
	if (retval == FALSE)
	{
		char *msg = g_strdup_printf("%s: %s\n", gl_program_name, error && error->message ? error->message : _("error parsing command line"));
		write_console(sd, msg, TRUE, FALSE);
		g_free(msg);
		g_clear_error(&error);
		return FALSE;
	}
	
	if (!empty(geom_arg))
	{
		g_free(sd->m_geometry);
		sd->m_geometry = geom_arg;
		geom_arg = NULL;
	}
	g_freep(&geom_arg);
	
	/*
	 * copy global options back to status var
	 */
	
	if (*argc > 1)
	{
		g_free(sd->m_file_source);
		sd->m_file_source = g_strdup((*argv)[1]);
	}
	
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void show_version(SHELL_DIALOG *sd)
{
	char *url = g_strdup_printf(_("%s is Open Source (see %s for further information).\n"), gl_program_name, HYP_URL);
	char *hyp_version = hyp_lib_version();
	char *msg = g_strdup_printf(
		"HypView GTK Version %s\n"
		"HCP %s\n"
		"%s\n"
		"%s",
		HYPVIEW_VERSION,
		hyp_version,
		HYP_COPYRIGHT,
		url);
	write_console(sd, msg, FALSE, FALSE);
	g_free(msg);
	g_free(hyp_version);
	g_free(url);
}

/*** ---------------------------------------------------------------------- ***/

static void fix_fds(void)
{
	/* Bad Things Happen if stdin, stdout, and stderr have been closed
	   (as by the `sh incantation "attraction >&- 2>&-").  When you do
	   that, the X connection gets allocated to one of these fds, and
	   then some random library writes to stderr, and random bits get
	   stuffed down the X pipe, causing "Xlib: sequence lost" errors.
	   So, we cause the first three file descriptors to be open to
	   /dev/null if they aren't open to something else already.  This
	   must be done before any other files are opened (or the closing
	   of that other file will again free up one of the "magic" first
	   three FDs.)

	   We do this by opening /dev/null three times, and then closing
	   those fds, *unless* any of them got allocated as #0, #1, or #2,
	   in which case we leave them open.  Gag.

	   Really, this crap is technically required of *every* X program,
	   if you want it to be robust in the face of "2>&-".
	 */
#ifdef G_OS_WIN32
#define NULL_DEV "nul"
#endif
#ifdef G_OS_UNIX
#define NULL_DEV "/dev/null"
#endif

	int fd0 = open(NULL_DEV, O_RDWR);
	int fd1 = open(NULL_DEV, O_RDWR);
	int fd2 = open(NULL_DEV, O_RDWR);

	if (fd0 > 2)
		close(fd0);
	if (fd1 > 2)
		close(fd1);
	if (fd2 > 2)
		close(fd2);
}

/*** ---------------------------------------------------------------------- ***/

#if defined(G_OS_WIN32) && defined(_MSC_VER)
static void myInvalidParameterHandler(const wchar_t *expression,
   const wchar_t *function,
   const wchar_t *file,
   unsigned int line, 
   uintptr_t pReserved)
{
	if (function && file)
		wprintf(L"Invalid parameter detected in function %s. File: %s Line: %d\n", function, file, line);
	if (expression)
		wprintf(L"Expression: %s\n", expression);
	(void) pReserved;
}
#endif /* G_OS_WIN32 */

/*** ---------------------------------------------------------------------- ***/

int main(int argc, char **argv)
{
	SHELL_DIALOG *sd;
	
	check_console();
	fix_fds();

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif

#ifdef G_OS_WIN32
	g_win32_get_windows_version();
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#ifdef _MSC_VER
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_set_invalid_parameter_handler(myInvalidParameterHandler);
#endif
#endif
	
#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, xs_get_locale_dir());
	textdomain(GETTEXT_PACKAGE);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif

	sd = global_sd = ShellDialog_New();
	if (sd == NULL)
		return 1;

	if (!ReadProfile(sd))
		return 1;
	if (!ParseCommandLine(sd, &argc, &argv))
		return 1;
	
	if (bShowHelp)
	{
		/* already handled in ParseCommandLine() */
	} else if (bShowVersion)
	{
		show_version(sd);
	} else
	{
		if (!init_gtk(sd))
			return 1;

		Help_Init();
		
		add_mru(sd, sd->m_file_source);
		create_shell_dialog(sd);
	
		if (sd->hwnd == NULL)
		{
			gtk_exit(1);
		}
		
		gtk_window_parse_geometry(GTK_WINDOW(sd->hwnd), sd->m_geometry);
		
		gtk_window_present(GTK_WINDOW(sd->hwnd));
	}
	
	if (toplevels_open_except(NULL) != 0)
	{
		GDK_THREADS_ENTER();
		
		gtk_main();
		
		GDK_THREADS_LEAVE();
	}
	
	return 0;
}
