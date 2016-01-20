#include "hv_defs.h"
#define RSC_NAMED_FUNCTIONS 1
#include "lang/en/hypview.rsh"
#include <mint/arch/nf_ops.h>

#define PROGRAM_NAME "HypView"
char const gl_program_name[] = PROGRAM_NAME;
char const gl_compile_date[12] = __DATE__;

/* ------------------------------------------------------------------------- */

char *gl_program_version(void)
{
	return g_strdup(HYPVIEW_VERSION);
}

/*** ---------------------------------------------------------------------- ***/

void GetTextSize(_WORD *width, _WORD *height)
{
	_WORD dummy;
	
	graf_handle(width, height, &dummy, &dummy);
}

/*** ---------------------------------------------------------------------- ***/

static void LoadConfig(void)
{
	HypProfile_Load();
	/*
	 * as long as there are no configuration dialogs,
	 * throw away the contents of the ini file,
	 * and only keep the settings
	 */
	Profile_Delete(gl_profile.profile);
	gl_profile.profile = NULL;
	
	Profile_ValidateColors(GetNumColors());
}

/*** ---------------------------------------------------------------------- ***/

const char *_argv0;
#define g_ttp_get_bindir g_gem_get_bindir
#include "hypmain.h"


int main(int argc, const char **argv)
{
	if (DoAesInit() == FALSE)
		return 1;

	if (DoInitSystem() == FALSE)
	{
		singletos_fail_loop();
		return 1;
	}
	
	LoadConfig();						/* Konfiguration laden */

	hv_init();							/* restliche Initialisierung */

	if (!_app)							/* Als ACC gestartet? */
		menu_register(gl_apid, "  " PROGRAM_NAME);	/* ...im Menu anmelden */
	
	if (argc <= 1)						/* Keine Parameter bergeben? */
	{
		if (_app)						/* Als Programm gestartet? */
		{
			if (!empty(gl_profile.viewer.default_file))			/* default-Hypertext specified? */
			{
				char *filename = path_subst(gl_profile.viewer.default_file);
				if (OpenFileInWindow(NULL, filename, NULL, HYP_NOINDEX, FALSE, TRUE, FALSE) == NULL)
					SelectFileLoad(NULL);						/* use file selector */
				g_free(filename);
			} else
			{
				SelectFileLoad(NULL);							/* use file selector */
			}
		}
	} else
	{
		/* ...load this file (incl. chapter) */
		OpenFileInWindow(NULL, argv[1], (argc > 2 ? argv[2] : NULL), HYP_NOINDEX, TRUE, TRUE, FALSE);
	}

	while (!_app || (!doneFlag && all_list))
	{
		DoEvent();
		if (quitApp)
			break;
	}

	RemoveItems();

	hv_exit();

	DoExitSystem();

	HypProfile_Delete();
	x_free_resources();

	return 0;
}
