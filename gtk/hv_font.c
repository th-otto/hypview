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
			
			doc = win->data;
			/* reload page or file */

			win->docsize.y = hv_win_topline(win);
			ret = doc->gotoNodeProc(doc, NULL, doc->getNodeProc(doc));
			
			if (ret)
			{
				doc->start_line = win->docsize.y;

				ReInitWindow(doc);
			}
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void SwitchFont(DOCUMENT *doc)
{
	UNUSED(doc);
	if (sel_font_name == gl_profile.viewer.font_name && gl_profile.viewer.xfont_name != 0)
	{
		sel_font_name = gl_profile.viewer.xfont_name;
	} else
	{
		sel_font_name = gl_profile.viewer.font_name;
	}
	ApplyFont();
}
