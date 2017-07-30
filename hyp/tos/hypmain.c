#include "hypdefs.h"

/* ------------------------------------------------------------------------- */

#if defined(__TOS__) || defined(__atarist__)

const char *_argv0;

char *g_ttp_get_bindir(void)
{
	return g_strdup(_argv0);
}
#endif
