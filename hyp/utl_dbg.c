#include "hypdefs.h"
#include "hypdebug.h"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

void hyp_debug(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	if (nf_debugvprintf(fmt, args) >= 0)
	{
		(void) nf_debugprintf("\n");
	} else
	{
		vfprintf(stderr, fmt, args);
		fputs("\n", stderr);
	}
	va_end(args);
}
