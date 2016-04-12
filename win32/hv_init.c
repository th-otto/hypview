#include "hv_defs.h"
#include "hypdebug.h"
#include "w_draw.h"

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
	w_init_brush();
}	

/*** ---------------------------------------------------------------------- ***/

void hv_exit(void)
{
	MarkerSaveToDisk(gl_profile.viewer.marken_save_ask);
	/* RecentSaveToDisk(); */
	RecentExit();
	w_exit_brush();
}
