#include "hypdefs.h"
#include "hypdebug.h"

#ifdef NO_UTF8

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

int hyp_utf8_fprintf(FILE *fp, const char *format, ...)
{
	va_list args;
	int res;
	
	va_start(args, format);
	res = vfprintf(fp, format, args);
	va_end(args);
	return res;
}

/*** ---------------------------------------------------------------------- ***/

HYP_CHARSET hyp_default_charset(HYP_OS os)
{
	UNUSED(os);
	return HYP_CHARSET_ATARI;
}

#endif
