#include "hv_defs.h"
#include "hv_vers.h"
#include "xgetopt.h"
#include "resource.rh"

char const gl_program_name[] = "HypView";
char const gl_compile_date[12] = __DATE__;

static gboolean bShowVersion;
static gboolean bShowHelp;
static const char *geom_arg;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

char *gl_program_version(void)
{
	return g_strdup(HYPVIEW_VERSION);
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

enum {
	OPT_GEOMETRY = 256
};

static struct option const long_options[] = {
	{ "geometry", required_argument, NULL, OPT_GEOMETRY },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	
	{ NULL, no_argument, NULL, 0 }
};
	

static gboolean NOINLINE ParseCommandLine(int *argc, const char ***pargv)
{
	struct _getopt_data *d;
	gboolean retval = TRUE;
	int c;
	const char **argv = *pargv;
	
	bShowVersion = FALSE;
	bShowHelp = FALSE;
	
	getopt_init_r(gl_program_name, &d);
	while ((c = getopt_long_only_r(*argc, argv, "hV?", long_options, NULL, d)) != EOF)
	{
		switch (c)
		{
		case OPT_GEOMETRY:
			geom_arg = getopt_arg_r(d);
			break;
		case 'h':
			bShowHelp = TRUE;
			break;
		case 'V':
			bShowVersion = TRUE;
			break;
		case '?':
			if (getopt_opt_r(d) == '?')
			{
				bShowHelp = TRUE;
			} else
			{
				retval = FALSE;
			}
			break;
		case 0:
			/* option which just sets a var */
			break;
		
		default:
			/* error message already issued */
			retval = FALSE;
			break;
		}
	}

	if (bShowHelp)
	{
		char *msg = g_strdup_printf(_("\
HypView Win32 Version %s\n\
ST-Guide Hypertext File Viewer\n\
\n\
usage: %s [FILE [CHAPTER]]"), HYPVIEW_VERSION, gl_program_name);
		write_console(msg, FALSE, FALSE, TRUE);
		g_free(msg);
	}
	
	if (retval)
	{
		int oind = getopt_ind_r(d);
		*argc = *argc - oind;
		*pargv += oind;
	}
	
	getopt_finish_r(&d);

	return retval;
}

/*** ---------------------------------------------------------------------- ***/

static void show_version(void)
{
	char *url = g_strdup_printf(_("%s is Open Source (see %s for further information).\n"), gl_program_name, HYP_URL);
	char *hyp_version = hyp_lib_version();
	char *msg = g_strdup_printf(
		"HypView Win32 Version %s\n"
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

#include "hypmain.h"

int main(int argc, const char **argv)
{
	int exit_status = EXIT_SUCCESS;
	HACCEL haccel;
	
	check_console();
	
#ifdef G_OS_WIN32
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#ifdef _MSC_VER
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_set_invalid_parameter_handler(myInvalidParameterHandler);
#endif
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
	
		Help_Init();
		
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
		
		if (argc <= 1)
		{
			/* default-hypertext specified? */
			if (gl_profile.viewer.startup == 1 &&
				(!empty(gl_profile.viewer.default_file) || !empty(gl_profile.viewer.catalog_file)))
			{
				char *filename = path_subst(empty(gl_profile.viewer.default_file) ? gl_profile.viewer.catalog_file : gl_profile.viewer.default_file);
				win = OpenFileInWindow(NULL, filename, hyp_default_main_node_name, HYP_NOINDEX, TRUE, TRUE, FALSE);
				g_free(filename);
			} else if (gl_profile.viewer.startup == 2 &&
				!empty(gl_profile.viewer.last_file))
			{
				char *filename = path_subst(gl_profile.viewer.last_file);
				win = OpenFileInWindow(NULL, filename, hyp_default_main_node_name, HYP_NOINDEX, TRUE, TRUE, FALSE);
				g_free(filename);
			}
		} else
		{
			if (argc == 2 && hyp_guess_filetype(argv[1]) != HYP_FT_HYP)
			{
				win = search_allref(win, argv[1], FALSE);
			} else
			{
				/* ...load this file (incl. chapter) */
				win = OpenFileInWindow(NULL, argv[1], (argc > 2 ? argv[2] : hyp_default_main_node_name), HYP_NOINDEX, TRUE, TRUE, FALSE);
			}
		}
		if (win == NULL)
			win = SelectFileLoad(NULL);						/* use file selector */
		
		if (win == NULL)
		{
			exit_status = EXIT_FAILURE;
		} else
		{
			hv_recent_add(win->data->path);
			hv_win_open(win);
			if (gl_profile.remarker.run_on_startup)
				StartRemarker(win, remarker_startup, FALSE);
		}
	}
	
	if (toplevels_open_except(NULL) != 0)
	{
		MSG msg;
		haccel = LoadAccelerators(GetInstance(), MAKEINTRESOURCE(IDR_ACCEL));
		
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (haccel == NULL || !TranslateAccelerator(msg.hwnd, haccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	
	Help_Exit();

	hv_exit();
	HypProfile_Delete();

	x_free_resources();

	return exit_status;
}
