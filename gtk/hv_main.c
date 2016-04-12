#define GDK_DISABLE_DEPRECATION_WARNINGS

#include "hv_gtk.h"
#include "hypdebug.h"
#include "gdkkeysyms.h"
#include "hv_vers.h"

#include "../icons/hypview.h"

char const gl_program_name[] = "HypView";
char const gl_program_version[] = HYPVIEW_VERSION;
char const gl_compile_date[12] = __DATE__;

static GDesktopAppInfo *appinfo;

/* avoid warnings from G_TYPE_* macros */
#pragma GCC diagnostic ignored "-Wcast-qual"
G_DEFINE_TYPE(HypviewApplication, hypview_application, G_TYPE_APPLICATION)
#pragma GCC diagnostic warning "-Wcast-qual"

static const gchar org_gtk_hypview_xml[] =
  "<node>"
    "<interface name='org.gtk.hypview'>"
      "<method name='ToFront'>"
        "<arg type='o' name='path' direction='in'/>"
        "<arg type='b' name='result' direction='out'/>"
      "</method>"
      "<method name='Close'>"
        "<arg type='o' name='path' direction='in'/>"
        "<arg type='b' name='result' direction='out'/>"
      "</method>"
      "<method name='GetFront'>"
        "<arg type='o' name='path' direction='out'/>"
      "</method>"
      "<method name='Quit'/>"
    "</interface>"
  "</node>";

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

static GOptionEntry const options[] = {
	{ "version", 0, 0, G_OPTION_ARG_NONE, NULL, N_("Show version information and exit"), NULL },
	{ "help", '?', G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_NONE, NULL, N_("Show help information and exit"), NULL },
	{ "geometry", 0, 0, G_OPTION_ARG_STRING, NULL, N_("Sets the client geometry of the main window"), NC_("option", "GEOMETRY") },
	{ "new-window", 0, 0, G_OPTION_ARG_NONE, NULL, N_("Open <FILE> in a new window"), NULL },
	{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, NULL, NULL, N_("[FILE [CHAPTER]]") },
	
	{ NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static gboolean NOINLINE init_options(GApplication *app)
{
	GOptionGroup *gtk_group;
	
	gtk_group = gtk_get_option_group(FALSE);
	g_application_add_option_group(app, gtk_group);
	g_application_add_main_option_entries(app, options);
	
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void show_version(GApplicationCommandLine *command_line)
{
	char *url = g_strdup_printf(_("%s is Open Source (see %s for further information).\n"), gl_program_name, HYP_URL);
	char *hyp_version = hyp_lib_version();
	char *msg = g_strdup_printf(
		"HypView GTK Version %s\n"
		"HCP %s\n"
		"%s\n"
		"%s",
		gl_program_version,
		hyp_version,
		HYP_COPYRIGHT,
		url);
	if (command_line)
		g_application_command_line_print(command_line, "%s\n", msg);
	else
		write_console(msg, FALSE, FALSE, FALSE);
	g_free(msg);
	g_free(hyp_version);
	g_free(url);
}

/*** ---------------------------------------------------------------------- ***/

static void fix_fds(void)
{
	/* Bad Things Happen if stdin, stdout, and stderr have been closed
	   (as by the `sh incantation "hypview >&- 2>&-").  When you do
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

static void hypview_application_init(HypviewApplication *app)
{
	UNUSED(app);
}

/*** ---------------------------------------------------------------------- ***/

static void hypview_application_activate(GApplication *application)
{
	G_APPLICATION_CLASS(hypview_application_parent_class)->activate(application);
	g_application_hold(application);

	if (all_list == NULL)
	{
		hv_init();
		Help_Init();
	}
	
	if (!init_gtk())
	{
		exit(EXIT_FAILURE);
	}
}

/*** ---------------------------------------------------------------------- ***/

static void hypview_application_open(GApplication *application, GFile **files, gint n_files, const gchar *hint)
{
	WINDOW_DATA *win = NULL;
	int i;
		
	UNUSED(application);
	UNUSED(hint);
	
	defer_messages++;
	for (i = 0; i < n_files; i++)
	{
		char *path = g_file_get_path(files[i]);
		win = OpenFileInWindow(NULL, path, hyp_default_main_node_name, HYP_NOINDEX, TRUE, gl_profile.viewer.va_start_newwin, FALSE);
		if (win != NULL)
		{
			hv_recent_add(win->data->path);
			hv_win_open(win);
		}
		g_free(path);
	}
	defer_messages--;
}

/*** ---------------------------------------------------------------------- ***/

static void hypview_application_startup(GApplication *application)
{
	G_APPLICATION_CLASS(hypview_application_parent_class)->startup(application);
}

/*** ---------------------------------------------------------------------- ***/

static void hypview_application_before_emit(GApplication *application, GVariant *platform_data)
{
	const gchar *startup_id;

	UNUSED(application);
	if (!g_variant_lookup(platform_data, "desktop-startup-id", "&s", &startup_id))
		return;
}

/*** ---------------------------------------------------------------------- ***/

static gint hypview_command_line(GApplication *app, GApplicationCommandLine *command_line)
{
	GVariantDict *dict = g_application_command_line_get_options_dict(command_line);
	gboolean bShowVersion = FALSE;
	gboolean bShowHelp = FALSE;
	char **argv = NULL;
	int argc;
	WINDOW_DATA *win = NULL;
	int status = EXIT_SUCCESS;
	
	UNUSED(app);
	g_variant_dict_lookup(dict, "help", "b", &bShowHelp);
	g_variant_dict_lookup(dict, "version", "b", &bShowVersion);
	g_variant_dict_lookup(dict, G_OPTION_REMAINING, "^as", &argv);
	argc = argv ? g_strv_length(argv) : 0;
	
	if (bShowHelp)
	{
		/* char *msg = g_option_context_get_help(context, FALSE, NULL);
		write_console(msg, FALSE, FALSE, TRUE);
		g_free(msg); */
	} else if (bShowVersion)
	{
		show_version(command_line);
	} else
	{
		char *geom_arg = NULL;
		gboolean quiet = FALSE;
		int new_window = TRUE;
		gboolean is_remote = g_application_command_line_get_is_remote(command_line);
		
		g_application_activate(app);

		g_variant_dict_lookup(dict, "geometry", "s", &geom_arg);

		if (!empty(geom_arg))
			gtk_XParseGeometry(geom_arg, &gl_profile.viewer.win_x, &gl_profile.viewer.win_y, &gl_profile.viewer.win_w, &gl_profile.viewer.win_h);
		{
			char *str= g_strdup_printf("%dx%d+%d+%d",
				gl_profile.viewer.win_w,
				gl_profile.viewer.win_h,
				gl_profile.viewer.win_x,
				gl_profile.viewer.win_y);
			hv_win_set_geometry(str);
			g_free(str);
		}
		g_freep(&geom_arg);

		/*
		 * when called remotely, prevent immediate error dialogs from popping up,
		 * because that would prevent sending back the exit status
		 */
		if (is_remote)
		{
			gboolean bNewWindow = FALSE;
			
			g_variant_dict_lookup(dict, "new-window", "b", &bNewWindow);
			
			defer_messages++;
			win = all_list ? (WINDOW_DATA *)all_list->data : NULL;
			new_window = gl_profile.viewer.va_start_newwin;
			if (bNewWindow)
				new_window = FORCE_NEW_WINDOW;
		}
		
		if (argc <= 0)
		{
			if (is_remote && win && new_window < FORCE_NEW_WINDOW)
			{
				/* hv_win_open(win); */
			}
			/* default-hypertext specified? */
			else if (gl_profile.viewer.startup == 1 &&
				(!empty(gl_profile.viewer.default_file) || !empty(gl_profile.viewer.catalog_file)))
			{
				char *filename = path_subst(empty(gl_profile.viewer.default_file) ? gl_profile.viewer.catalog_file : gl_profile.viewer.default_file);
				win = OpenFileInWindow(win, filename, hyp_default_main_node_name, HYP_NOINDEX, TRUE, new_window, quiet);
				g_free(filename);
			} else if (gl_profile.viewer.startup == 2 &&
				!empty(gl_profile.viewer.last_file))
			{
				char *filename = path_subst(gl_profile.viewer.last_file);
				win = OpenFileInWindow(win, filename, hyp_default_main_node_name, HYP_NOINDEX, TRUE, new_window, quiet);
				g_free(filename);
			}
		} else
		{
			char *path = argv[0];
			
			if (argc == 1 && hyp_guess_filetype(path) != HYP_FT_HYP)
			{
				win = search_allref(win, path, FALSE);
			} else
			{
				/* ...load this file (incl. chapter) */
				win = OpenFileInWindow(win, path, (argc >= 2 ? argv[1] : hyp_default_main_node_name), HYP_NOINDEX, TRUE, new_window, quiet);
			}
		}
		
		if (win == NULL && !is_remote)
			win = SelectFileLoad(NULL);						/* use file selector */
		
		if (win == NULL)
		{
			g_application_command_line_set_exit_status(command_line, EXIT_FAILURE);
			status = EXIT_FAILURE;
		} else
		{
			hv_recent_add(win->data->path);
			hv_win_open(win);
			if (gl_profile.remarker.run_on_startup)
				StartRemarker(win, remarker_startup, FALSE);
		}

		if (is_remote)
			defer_messages--;
	}
	
	g_strfreev(argv);
	
	return status;
}

/*** ---------------------------------------------------------------------- ***/

static void do_action(const char *name)
{
	GtkHypviewWindow *win;
	GtkAction *action;
	
	if (all_list == NULL)
		return;
	win = (GtkHypviewWindow *)all_list->data;
	action = gtk_action_group_get_action(win->action_group, name);
	if (action)
		gtk_action_activate(action);
}

/*** ---------------------------------------------------------------------- ***/

static void g_application_impl_method_call(
	GDBusConnection *connection,
	const gchar *sender,
	const gchar *object_path,
	const gchar *interface_name,
	const gchar *method_name,
	GVariant *parameters,
	GDBusMethodInvocation *invocation,
	gpointer user_data)
{
	HypviewApplication *app = (HypviewApplication *)user_data;
	GVariant *result = NULL;
	GtkHypviewWindow *win;
	
	UNUSED(connection);
	UNUSED(sender);
	UNUSED(object_path);
	UNUSED(interface_name);
	UNUSED(method_name);
	UNUSED(app);
	
	if (strcmp(method_name, "ToFront") == 0)
	{
		const char *path = NULL;
		
		win = NULL;
		g_variant_get(parameters, "(&o)", &path);
		if (empty(path))
		{
			if (all_list)
				win = (GtkHypviewWindow *)all_list->data;
		} else
		{
			GSList *l;
			
			for (l = all_list; l; l = l->next)
			{
				win = (GtkHypviewWindow *)l->data;
				
				if (strcmp(path, win->object_path) == 0)
					break;
			}
		}
		if (win)
		{
			hv_win_open(win);
			result = g_variant_new("(b)", TRUE);
		} else
		{
			result = g_variant_new("(b)", FALSE);
		}
	}
	
	if (strcmp(method_name, "Close") == 0)
	{
		const char *path = NULL;
		
		win = NULL;
		g_variant_get(parameters, "(&o)", &path);
		if (empty(path))
		{
			if (all_list)
				win = (GtkHypviewWindow *)all_list->data;
		} else
		{
			GSList *l;
			
			for (l = all_list; l; l = l->next)
			{
				win = (GtkHypviewWindow *)l->data;
				
				if (strcmp(path, win->object_path) == 0)
					break;
			}
		}
		if (win)
		{
			SendCloseWindow(win);
			result = g_variant_new("(b)", TRUE);
		} else
		{
			result = g_variant_new("(b)", FALSE);
		}
	}
	
	if (strcmp(method_name, "GetFront") == 0)
	{
		win = all_list ? (GtkHypviewWindow *)all_list->data : NULL;
		result = g_variant_new("(o)", win ? win->object_path : "");
#if 0
		GVariantBuilder b;
		g_variant_builder_init(&b, G_VARIANT_TYPE_TUPLE);
		g_variant_builder_add_value(&b, g_variant_new_object_path(win ? win->object_path : ""));
		result = g_variant_builder_end(&b);
#endif
	}
	
	if (strcmp(method_name, "Quit") == 0)
	{
		do_action("quit");
	}
	
	g_dbus_method_invocation_return_value(invocation, result);
}

/*** ---------------------------------------------------------------------- ***/

static GVariant *g_application_impl_get_property(
	GDBusConnection *connection,
	const gchar *sender,
	const gchar *object_path,
	const gchar *interface_name,
	const gchar *property_name,
	GError **error,
	gpointer user_data)
{
	HypviewApplication *app = (HypviewApplication *)user_data;

	UNUSED(connection);
	UNUSED(sender);
	UNUSED(object_path);
	UNUSED(interface_name);
	UNUSED(property_name);
	UNUSED(error);
	UNUSED(app);
	
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

static void hypview_application_class_init(GApplicationClass *klass)
{
	klass->before_emit = hypview_application_before_emit;
	klass->startup = hypview_application_startup;
	klass->activate = hypview_application_activate;
	klass->open = hypview_application_open;
	klass->command_line = hypview_command_line;
}

/*** ---------------------------------------------------------------------- ***/

static int gtk_application_run(GApplication *application, int argc, char **argv)
{
	int status;
	char **arguments;

	{
		gint i;

		arguments = g_new (gchar *, argc + 1);
		for (i = 0; i < argc; i++)
			arguments[i] = g_strdup (argv[i]);
		arguments[i] = NULL;
	}

	if (g_get_prgname() == NULL && argc > 0)
	{
		gchar *prgname;

		prgname = g_path_get_basename(argv[0]);
		g_set_prgname(prgname);
		g_free(prgname);
	}

	if (!G_APPLICATION_GET_CLASS(application)->local_command_line(application, &arguments, &status))
	{
		/* the default implementation never returns FALSE */
		ASSERT(0);
		status = EXIT_FAILURE;
	}
	g_strfreev(arguments);
	
	if (toplevels_open_except(NULL) != 0)
		gtk_main();

	if (g_application_get_is_registered(application) && !g_application_get_is_remote(application))
	{
		g_signal_emit_by_name(application, "shutdown", 0);
	}

	return status;
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
	HypviewApplication *app;
	int exit_status = EXIT_SUCCESS;
	struct stat s;
	const char *desktop_filename = "hypview.desktop";
	
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

	HypProfile_Load();

	g_set_prgname(gl_program_name);
	if (stat(desktop_filename, &s) == 0)
		appinfo = g_desktop_app_info_new_from_filename(desktop_filename);
	else
		appinfo = g_desktop_app_info_new(desktop_filename);
	app = (HypviewApplication *)g_object_new(hypview_application_get_type(),
		"application-id", "org.gtk.hypview",
		"flags", G_APPLICATION_HANDLES_OPEN | G_APPLICATION_HANDLES_COMMAND_LINE,
		NULL);

	{
    	GError *error = NULL;

		if (!g_application_register(app, NULL, &error))
		{
			g_printerr("%s\n", error->message);
			g_error_free(error);
			return EXIT_FAILURE;
		}
	}
	
	{
		GError *error = NULL;
		GDBusNodeInfo *info;
		GDBusInterfaceInfo *org_gtk_hypview;
		GDBusConnection *session_bus;
		const char *object_path;
		
		static const GDBusInterfaceVTable vtable = {
			g_application_impl_method_call,
			g_application_impl_get_property,
			NULL, /* set_property */
			{ 0, 0, 0, 0, 0, 0, 0, 0 }
		};
		
		info = g_dbus_node_info_new_for_xml(org_gtk_hypview_xml, &error);
		if (G_UNLIKELY(info == NULL))
		{
			g_printerr("%s", error->message);
			g_error_free(error);
		} else
		{
			org_gtk_hypview = g_dbus_node_info_lookup_interface(info, "org.gtk.hypview");
			g_dbus_interface_info_ref(org_gtk_hypview);
			g_dbus_node_info_unref(info);
			
			session_bus = g_application_get_dbus_connection(app);
			object_path = g_application_get_dbus_object_path(app);
			g_dbus_connection_register_object(session_bus, object_path, org_gtk_hypview, &vtable, app, NULL, &error);
		}
	}
	
	if (!init_options(app))
		return EXIT_FAILURE;
	
	exit_status = gtk_application_run(app, argc, argv);	
	
	Help_Exit();

	hv_exit();
	HypProfile_Delete();

	exit_gtk();
	
	x_free_resources();

	return exit_status;
}
