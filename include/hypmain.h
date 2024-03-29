#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif

int hypmain(int argc, const char **argv);
int utf8_main(int argc, const char **argv);

#if defined(__WIN32__) && !defined(__CYGWIN__)

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
	for (argc = 0; argv[argc] != NULL; argc++)
		;
#undef main
#define main utf8_main
	ret = utf8_main(argc, (const char **)(const void **)argv);
	for (argc = 0; argv[argc] != NULL; argc++)
		g_free(argv[argc]);
	g_free(argv);
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
	
	static _DTA mydta;
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
		__argv[i] = hyp_conv_to_utf8(HYP_CHARSET_ATARI, argv[i], STR0TERM);
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


#if defined(__LINUX_GLIBC_WRAP_H)

/* ugly hack to get __libc_start_main versioned */

#if __GLIBC_PREREQ(2, 34)

#define STR_(s) #s
#define STR(s)  STR_(s)
#include <dlfcn.h>

#ifdef __UCLIBC__
#define __libc_start_main       __uClibc_main
#endif

int __libc_start_main(
        int (*main)(int,char**,char**), int ac, char **av,
        int (*init)(void), void (*fini)(void),
        void (*rtld_fini)(void), void *stack_end);
int __libc_start_main(
        int (*main)(int,char**,char**), int ac, char **av,
        int (*init)(void), void (*fini)(void),
        void (*rtld_fini)(void), void *stack_end)
{
	typeof(__libc_start_main) *real_lsm;
	if ((*(void**)&real_lsm = dlsym(RTLD_NEXT, STR(__libc_start_main))) != 0)
		return real_lsm(main, ac, av, init, fini, rtld_fini, stack_end);
	fputs("BUG: dlsym error\n", stderr);
	return 1;
}
#undef STR
#undef STR_
#endif
#endif


int main(int argc, const char **argv)
{
	int ret;
	
	_crtinit();

#if defined(__TOS__) || defined(__atarist__)
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
