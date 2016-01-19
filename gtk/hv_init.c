#include "hv_gtk.h"
#include "hypdebug.h"

const char *sel_font_name;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void hv_init(void)
{
	char *skin;
	
	sel_font_name = gl_profile.viewer.font_name;
	
	/* load toolbar/skin */
	skin = gl_profile.viewer.skin_path;
	if (!empty(skin))
	{
	}

	/* load markers */
	MarkerInit();
}	

/*** ---------------------------------------------------------------------- ***/

void hv_exit(void)
{
	MarkerSaveToDisk();
}
