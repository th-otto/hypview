#include "hypdefs.h"
#include "hypdebug.h"


void hyp_debug(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	nf_debugvprintf(fmt, args);
	va_end(args);
	fputs("\n", stderr);
	nf_debugprintf("\n");
}
