#include "hv_gtk.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void ApplyFont(void)
{
	WINDOW_DATA *win;
	DOCUMENT *doc;
	GSList *l;
	
	/* adjust all open documents and windows */
	win = (WINDOW_DATA *) all_list;

	for (l = all_list; l; l = l->next)
	{
		win = l->data;
		/* if (win->type == WIN_WINDOW) */
		{
			gboolean ret;
			long topline;
			
			doc = win->data;
			/* reload page or file */

			topline = hv_win_topline(win);
			ret = doc->gotoNodeProc(win, NULL, doc->getNodeProc(doc));
			
			if (ret)
			{
				doc->start_line = topline;

				ReInitWindow(win);
			}
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void SwitchFont(WINDOW_DATA *win)
{
	UNUSED(win);
	if (sel_font_name == gl_profile.viewer.font_name && gl_profile.viewer.xfont_name != 0)
	{
		sel_font_name = gl_profile.viewer.xfont_name;
	} else
	{
		sel_font_name = gl_profile.viewer.font_name;
	}
	ApplyFont();
}

/*** ---------------------------------------------------------------------- ***/

void SelectFont(WINDOW_DATA *win)
{
	UNUSED(win);
	printf("NYI: SelectFont\n");
}
