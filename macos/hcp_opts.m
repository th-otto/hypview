#include "../hyp/hcp_opts.c"
#include "hv_vers.h"

char *usage_msg(void)
{
	return g_strdup_printf(_("\
HypView macOS Version %s\n\
ST-Guide Hypertext File Viewer\n\
\n\
usage: %s [FILE [CHAPTER]]"), gl_program_version, gl_program_name);
}


char *version_msg(void)
{
	char *url = g_strdup_printf(_("%s is Open Source (see %s for further information).\n"), gl_program_name, HYP_URL);
	char *hyp_version = hyp_lib_version();
	char *msg = g_strdup_printf(
		"HypView macOS Version %s\n"
		"HCP %s\n"
		"%s\n"
		"%s",
		gl_program_version,
		hyp_version,
		HYP_COPYRIGHT,
		url);
	g_free(hyp_version);
	g_free(url);
	return msg;
}
