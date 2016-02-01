#include "hv_defs.h"
#include "hypdebug.h"

extern const char *_argv0;
extern const char *__env_argv;

/* selected font */
_WORD font_cw, font_ch;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

char *g_gem_get_bindir(void)
{
	char *retval;
	
	retval = NULL;
	if (!_app)
		return g_strdup("*:\\hypview");
	
#ifdef __PUREC__
	/*
	 * if ARGV protocol was used, use that.
	 * shel_read might read the application name of
	 * the calling APP when that app didnt use shel_write().
	 *
	 * Found no way so far to figure that out when using gemlib,
	 * the startup code trashes the env string, so getenv("ARGV")
	 * doesnt work.
	 */
	if (__env_argv != NULL)
	{
		retval = g_strdup(_argv0);
	} else
#endif
	{
		size_t allocsize = 65400ul;
		_WORD res;
		char *cmd, *tail;
		
		/* unfortunately, there is no restriction on the args to shel_read() */
		cmd = g_new(char, allocsize);
		*cmd = '\0';
		tail = g_new(char, allocsize);
		*tail = '\0';
		res = shel_read(cmd, tail);
		g_free(tail);
		if (res == 0 || *cmd == '\0')
		{
			/*
			 * failed, try argv.
			 * This does not work, if app was started
			 * from old DESKTOP that did not use ARGV protokoll,
			 * because in this case we only get the arguments
			 * in the command line but not the program name.
			 */
			g_free(cmd);
			retval = g_strdup(_argv0);
		} else
		{
			retval = cmd;
		}
	}
	return retval;
}
