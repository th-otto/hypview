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
	int pipefds[2];
	int count, err;
	
	new_argv = g_new(char *, argc + 1);
	if (new_argv == NULL)
		return -1;
	for (i = 0; i < argc; i++)
		new_argv[i] = hyp_utf8_to_charset(hyp_get_current_charset(), argv[i], STR0TERM, NULL);
	new_argv[i] = NULL;
	pipefds[0] = pipefds[1] = -1;
	if (mode != P_OVERLAY)
	{
		if (pipe(pipefds))
			return -1;
		if (fcntl(pipefds[1], F_SETFD, fcntl(pipefds[1], F_GETFD) | FD_CLOEXEC))
			return -1;
		pid = fork();
		if (pid < 0)
			return -1;
	} else
	{
		pid = 0;
	}
	if (pid == 0)
	{
		if (pipefds[0] >= 0)
			close(pipefds[0]);
		execvp(new_argv[0], new_argv);
		if (mode == P_OVERLAY)
		{
			/* we are still in the parent, because the exec failed */
			return -1;
		} else
		{
			/* we are in the child, report errno to parent */
			write(pipefds[1], &errno, sizeof(int));
			_exit(-1);
		}
	}
	
	for (i = 0; i < argc; i++)
		g_free(new_argv[i]);
	g_free(new_argv);
	
	if (pipefds[1] >= 0)
	{
		close(pipefds[1]);
		while ((count = read(pipefds[0], &err, sizeof(errno))) == -1)
			if (errno != EAGAIN && errno != EINTR)
				break;
		close(pipefds[0]);
		if (count)
		{
			errno = err;
			return -1;
		}
	}
	
	retval = 0;
	if (mode == P_WAIT)
	{
		while (waitpid(pid, &retval, 0) > 0 && !WIFEXITED(retval))
		{
		}
		if (WIFEXITED(retval))
			retval = WEXITSTATUS(retval);
		else if (WIFSIGNALED(retval))
			retval = WTERMSIG(retval) + 128;
	} else if (mode == P_NOWAIT)
	{
		retval = pid;
	}
	return retval;
}
