#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif

int hypmain(int argc, const char **argv);
int utf8_main(int argc, const char **argv);

#ifdef __WIN32__
#undef _argc
#undef __argc
#undef _argv
#undef __argv
#undef __wargv
static int __argc = 0;
static char **__argv = 0;
typedef struct {
  int newmode;
} _startupinfo;
EXTERN_C_BEG
extern void __getmainargs (int *, char ***, char ***, int, _startupinfo *);
extern void __wgetmainargs (int *, wchar_t ***, wchar_t ***, int, _startupinfo *);
EXTERN_C_END

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
	wchar_t **dummy_environ;
	_startupinfo start_info;
	int i;
	wchar_t **__wargv = 0;
	int ret;
	
	(void) hInstance;
	(void) hPrevInstance;
	(void) lpszCmdLine;
	(void) cmdShow;

	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
	
	start_info.newmode = 0;
	__wgetmainargs(&__argc, &__wargv, &dummy_environ, 0, &start_info);
	_crtinit();
	__argv = g_new(char *, __argc + 1);
	for (i = 0; i < __argc; i++)
		__argv[i] = hyp_wchar_to_utf8(__wargv[i], STR0TERM);
	__argv[i] = NULL;
#undef main
#define main utf8_main
	ret = utf8_main(__argc, (const char **)(const void **)__argv);
	for (i = 0; i < __argc; i++)
		g_free(__argv[i]);
	g_free(__argv[i]);
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
