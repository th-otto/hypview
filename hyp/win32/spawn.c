#include "hypdefs.h"
#include <stdlib.h>
#include <sys/types.h>

#include <limits.h>
#include <errno.h>
#include "stat_.h"
#ifdef __CYGWIN__
#include <process.h>
int _wspawnvp(int mode, const wchar_t *path, const wchar_t * const *argv);
#endif


int hyp_utf8_spawnvp(int mode, int argc, const char *const *argv)
{
	wchar_t **new_argv;
	int retval;
	int i;
	
	new_argv = g_new(wchar_t *, argc + 1);
	if (new_argv == NULL)
		return -1;
	for (i = 0; i < argc; i++)
		new_argv[i] = hyp_utf8_to_wchar(argv[i], STR0TERM, NULL);
	new_argv[i] = NULL;
	if (mode == P_NOWAIT && argc == 1 && is_weblink(argv[0]))
	{
		ShellExecuteW(NULL, L"open", new_argv[0], NULL, NULL, SW_SHOW);
		retval = 0;
	} else
	{
#ifdef __CYGWIN__
		retval = spawnvp(mode, argv[0], argv);
#else
		retval = _wspawnvp(mode, new_argv[0], (const wchar_t *const *)new_argv);
#endif
	}
	for (i = 0; i < argc; i++)
		g_free(new_argv[i]);
	g_free(new_argv);
	return retval;
}
