#define GDK_DISABLE_DEPRECATION_WARNINGS

#include "hv_gtk.h"
#include "gdkkeysyms.h"

#include "../icons/hypview.h"

char const gl_program_name[] = "HypView";
char const gl_compile_date[12] = __DATE__;

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

/******************************************************************************/
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

static void NOINLINE LoadConfig(void)
{
	HypProfile_Load();
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

static gboolean NOINLINE ParseCommandLine(int *argc, char ***argv)
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
		write_console(msg, FALSE, FALSE, TRUE);
		g_free(msg);
	}
	g_option_context_free(context);
	
	if (retval == FALSE)
	{
		char *msg = g_strdup_printf("%s: %s\n", gl_program_name, error && error->message ? error->message : _("error parsing command line"));
		write_console(msg, TRUE, TRUE, FALSE);
		g_free(msg);
		g_clear_error(&error);
		return FALSE;
	}
	
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void show_version(void)
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
	int exit_status = EXIT_SUCCESS;
	gboolean threads_entered = FALSE;
	
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

	LoadConfig();

	if (!ParseCommandLine(&argc, &argv))
		return EXIT_FAILURE;
	
	if (bShowHelp)
	{
		/* already handled in ParseCommandLine() */
	} else if (bShowVersion)
	{
		show_version();
	} else
	{
		WINDOW_DATA *win = NULL;
		
		hv_init();
	
		if (!init_gtk())
			return EXIT_FAILURE;

		if (GetNumColors() < 16 || gl_profile.viewer.background_color >= 16)
			gl_profile.viewer.background_color = G_WHITE;
		if (GetNumColors() < 16 || gl_profile.viewer.text_color >= 16)
			gl_profile.viewer.text_color = G_BLACK;
		if (gl_profile.viewer.background_color == gl_profile.viewer.text_color)
			gl_profile.viewer.background_color = gl_profile.viewer.text_color ^ 1;

		GDK_THREADS_ENTER();
		threads_entered = TRUE;
		
		Help_Init();
		
		if (argc <= 1)
		{
			if (gl_profile.viewer.startup == 1 &&
				!empty(gl_profile.viewer.default_file))			/* Default-Hypertext angegeben? */
			{
				char *filename = path_subst(gl_profile.viewer.default_file);
				win = OpenFileNewWindow(filename, NULL, HYP_NOINDEX, FALSE);
				g_free(filename);
			} else if (gl_profile.viewer.startup == 2 &&
				!empty(gl_profile.viewer.last_file))
			{
				char *filename = path_subst(gl_profile.viewer.last_file);
				win = OpenFileNewWindow(filename, NULL, HYP_NOINDEX, FALSE);
				g_free(filename);
			}
			if (win == NULL)
				win = SelectFileLoad(NULL);						/* Datei per Fileselector erfragen */
		} else
		{
			/* ...diese Datei (inkl. Kapitel) laden */
			win = OpenFileNewWindow(argv[1], (argc > 2 ? argv[2] : NULL), HYP_NOINDEX, TRUE);
		}
		
		if (win == NULL)
		{
			exit_status = EXIT_FAILURE;
		} else
		{
			if (!empty(geom_arg))
			{
				hv_win_set_geometry(win, geom_arg);
			}
			hv_win_open(win);
		}
	}
	g_freep(&geom_arg);
	
	if (toplevels_open_except(NULL) != 0)
	{
		gtk_main();	
	}
	
	Help_Exit();

	hv_exit();
	HypProfile_Delete();

	if (threads_entered)
	{
		GDK_THREADS_LEAVE();
	}	
	
	x_free_resources();

	return exit_status;
}
