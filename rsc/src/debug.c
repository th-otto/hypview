#include "config.h"
#include <portab.h>
#include <stdarg.h>
#include <errno.h>
#include "nls.h"
#include "debug.h"
#if defined(OS_ATARI)
#include <mint/arch/nf_ops.h>
#endif

static errout_handler error_handler;
static void *error_data;

/*** ---------------------------------------------------------------------- ***/

void erroutv(const char *format, va_list args)
{
	if (error_handler)
	{
		error_handler(error_data, format, args);
	} else
	{
		vfprintf(stderr, format, args);
	}
}

void errout(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	erroutv(format, args);
	va_end(args);
}

/*** ---------------------------------------------------------------------- ***/

errout_handler set_errout_handler(errout_handler handler, void *data)
{
	errout_handler old = error_handler;
	error_handler = handler;
	error_data = data;
	return old;
}

/*** ---------------------------------------------------------------------- ***/

void debugoutv(const char *format, va_list args)
{
	if (error_handler)
	{
		error_handler(error_data, format, args);
	} else
	{
#if defined(OS_ATARI)
		nf_debugvprintf(format, args);
#else
		vfprintf(stderr, format, args);
#endif
	}
}

void debugout(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	debugoutv(format, args);
	va_end(args);
}
