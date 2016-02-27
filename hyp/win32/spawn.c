#include "hypdefs.h"
#include <stdlib.h>
#include <sys/types.h>

#include <limits.h>
#include <errno.h>
#include "stat_.h"


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
	retval = _wspawnvp(mode, new_argv[0], (const wchar_t *const *)new_argv);
	for (i = 0; i < argc; i++)
		g_free(new_argv[i]);
	g_free(new_argv);
	return retval;
}
