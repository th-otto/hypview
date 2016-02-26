#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif

int hypmain(int argc, const char **argv);
int utf8_main(int argc, const char **argv);

#ifdef __WIN32__

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
	int ret;
	int argc;
	char **argv;
	
	(void) hInstance;
	(void) hPrevInstance;
	(void) lpszCmdLine;
	(void) cmdShow;

	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
	
	_crtinit();
	argv = g_win32_get_command_line();
	argc = g_strv_length(argv);
#undef main
#define main utf8_main
	ret = utf8_main(argc, (const char **)(const void **)argv);
	g_strfreev(argv);
	_crtexit();
#undef _crtinit
#undef _crtexit
#define _crtinit()
#define _crtexit()
	return ret;
}
#endif

#if defined(__TOS__) || defined(__atarist__)
#undef _argc
#undef __argc
#undef _argv
#undef __argv
#undef main
static int __argc = 0;
static char **__argv = 0;
int main(int argc, const char **argv)
{
	int i;
	int ret;
	gboolean converror = FALSE;
	
	static DTA mydta;
	Fsetdta(&mydta);
	
	Pdomain(1); /* DOM_MINT */

#ifdef do_appl_init
	gl_apid = appl_init();
	if (gl_apid < 0)
		return 1;
	acc_memsave = !_app && _AESnumapps == 1;
#endif

	_mallocChunkSize(0);
	
	__argc = argc;
	__argv = g_new(char *, argc + 1);
	for (i = 0; i < argc; i++)
		__argv[i] = hyp_conv_charset(HYP_CHARSET_ATARI, HYP_CHARSET_UTF8, argv[i], STR0TERM, &converror);
	__argv[i] = NULL;
#undef main
#define main utf8_main
	ret = utf8_main(__argc, (const char **)(const void **)__argv);
	for (i = 0; i < __argc; i++)
		g_free(__argv[i]);
	g_free(__argv);
	_crtexit();
#undef _crtinit
#undef _crtexit
#define _crtinit()
#define _crtexit()
	return ret;
}
#endif


int main(int argc, const char **argv)
{
	int ret;
	
	_crtinit();

#ifdef __TOS__
	_argv0 = argv[0];
	g_tos_get_bindir = g_ttp_get_bindir;
#endif

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, xs_get_locale_dir());
	textdomain(GETTEXT_PACKAGE);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif

#undef main
#define main hypmain
	ret = main(argc, argv);

#ifdef ENABLE_NLS
	xs_locale_exit();
#endif

	_crtexit();

	return ret;
}
