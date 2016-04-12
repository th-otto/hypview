#include "hv_gtk.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void hv_init(void)
{
	char *skin;
	
	/* load toolbar/skin */
	skin = gl_profile.viewer.skin_path;
	if (!empty(skin))
	{
	}

	/* load markers */
	MarkerInit();
	RecentInit();
}	

/*** ---------------------------------------------------------------------- ***/

void hv_exit(void)
{
	MarkerSaveToDisk(gl_profile.viewer.marken_save_ask);
	/* RecentSaveToDisk(); */
	RecentExit();
}
