#include "hv_defs.h"
#include "hypdebug.h"
#include "resource.rh"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void hv_update_menu(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	HMENU menu = GetMenu(win->hwnd);
	
	if (!win->is_popup)
	{
		CheckMenuObj(win, IDM_OPT_ALTFONT, gl_profile.viewer.use_xfont);
		CheckMenuObj(win, IDM_OPT_EXPANDSPACES, gl_profile.viewer.expand_spaces);
		
		EnableMenuObj(menu, IDM_NAV_BOOKMARKSMENU, doc->buttons.memory);
		EnableMenuObj(menu, IDM_FILE_OPEN, doc->buttons.load);
		EnableMenuObj(menu, IDM_FILE_SAVE, doc->buttons.save);
		EnableMenuObj(menu, IDM_FILE_REMARKER, doc->buttons.remarker);
		EnableMenuObj(menu, IDM_FILE_INFO, doc->buttons.info);
		EnableMenuObj(menu, IDM_NAV_BACK, doc->buttons.back);
		EnableMenuObj(menu, IDM_NAV_CLEARSTACK, doc->buttons.history);
		EnableMenuObj(menu, IDM_FILE_CATALOG, !empty(gl_profile.viewer.catalog_file));
		EnableMenuObj(menu, IDM_NAV_PREV, doc->buttons.previous);
		EnableMenuObj(menu, IDM_NAV_PREVPHYS, doc->buttons.prevphys);
		EnableMenuObj(menu, IDM_NAV_TOC, doc->buttons.home);
		EnableMenuObj(menu, IDM_NAV_NEXT, doc->buttons.next);
		EnableMenuObj(menu, IDM_NAV_NEXTPHYS, doc->buttons.nextphys);
		EnableMenuObj(menu, IDM_NAV_FIRST, doc->buttons.first);
		EnableMenuObj(menu, IDM_NAV_LAST, doc->buttons.last);
		EnableMenuObj(menu, IDM_NAV_INDEX, doc->buttons.index);
		EnableMenuObj(menu, IDM_NAV_HELP, doc->buttons.help);
	}
}

/*** ---------------------------------------------------------------------- ***/

void hv_update_menus(void)
{
	WINDOW_DATA *win;
	GSList *l;
	
	for (l = all_list; l; l = l->next)
	{
		win = (WINDOW_DATA *)l->data;
		hv_update_menu(win);
	}
}

/*** ---------------------------------------------------------------------- ***/

static void ApplyFont(void)
{
	WINDOW_DATA *win;
	DOCUMENT *doc;
	GSList *l;
	
	hv_update_menus();
	/* adjust all open documents and windows */
	for (l = all_list; l; l = l->next)
	{
		win = (WINDOW_DATA *)l->data;
		/* if (win->type == WIN_WINDOW) */
		{
			gboolean ret;
			long topline;
			
			doc = win->data;
			/* reload page or file */

			topline = hv_win_topline(win);
			ret = doc->gotoNodeProc(win, NULL, doc->getNodeProc(win));
			
			if (ret)
			{
				doc->start_line = topline;

				ReInitWindow(win, TRUE);
			}
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void SwitchFont(WINDOW_DATA *win)
{
	UNUSED(win);
	gl_profile.viewer.use_xfont = gl_profile.viewer.use_xfont && gl_profile.viewer.xfont_name != NULL;
	ApplyFont();
	HypProfile_SetChanged();
}

/*** ---------------------------------------------------------------------- ***/

void SelectFont(WINDOW_DATA *win)
{
	UNUSED(win);
	/* NYI */
}
