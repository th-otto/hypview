#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>

/*** ---------------------------------------------------------------------- ***/

char *pic_colornameformat(int planes)
{
	char *colors;
	if (planes <= 0)
		colors = g_strdup_printf(_(" unknown-%d"), planes);
	else if (planes <= 8)
		colors = g_strdup_printf("x%d", 1 << planes);
	else if (planes <= 16)
		colors = g_strdup_printf(_(" hicolor-%d"), planes);
	else
		colors = g_strdup_printf(_(" truecolor-%d"), planes);
	return colors;
}

