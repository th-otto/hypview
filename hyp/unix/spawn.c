#include "hypdefs.h"
#include <stdlib.h>
#include <sys/types.h>

#include <limits.h>
#include <errno.h>
#include <sys/wait.h>



int hyp_utf8_spawnvp(int mode, int argc, const char *const *argv)
{
	char **new_argv;
	int retval;
	int i;
	pid_t pid;
	
	new_argv = g_new(char *, argc + 1);
	if (new_argv == NULL)
		return -1;
	for (i = 0; i < argc; i++)
		new_argv[i] = hyp_utf8_to_charset(hyp_get_current_charset(), argv[i], STR0TERM, NULL);
	new_argv[i] = NULL;
	if (mode != P_OVERLAY)
	{
		pid = fork();
		if (pid < 0)
			return -1;
	} else
	{
		pid = 0;
	}
	if (pid == 0)
	{
		execvp(new_argv[0], new_argv);
		_exit(-1);
	}
	retval = 0;
	for (i = 0; i < argc; i++)
		g_free(new_argv[i]);
	if (mode == P_WAIT)
	{
		while (waitpid(pid, &retval, 0) > 0 && !WIFEXITED(retval))
		{
		}
		retval = WEXITSTATUS(retval);
	}
	return retval;
}
