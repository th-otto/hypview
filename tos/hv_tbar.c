/*
 * HypView - (c) 2001 - 2006 Philipp Donze
 *               2006 -      Philipp Donze & Odd Skancke
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hv_defs.h"
#include "hypview.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void toolbar_redraw(WINDOW_DATA *win)
{
	GRECT tbar;
	GRECT r;
	_WORD ret;
	
	wind_get_grect(win->whandle, WF_WORKXYWH, &tbar);
	tbar.g_h = win->y_offset;
	wind_update(BEG_UPDATE);
	ret = wind_get_grect(win->whandle, WF_FIRSTXYWH, &r);
	while (ret != 0 && r.g_w > 0 && r.g_h > 0)
	{
		if (rc_intersect(&tbar, &r))
			objc_draw_grect(win->toolbar, TO_BACKGRND, MAX_DEPTH, &r);
		ret = wind_get_grect(win->whandle, WF_NEXTXYWH, &r);
	}
	wind_update(END_UPDATE);
}

/*** ---------------------------------------------------------------------- ***/

static void EnableObj(OBJECT *tree, _WORD obj, gboolean enable)
{
	if (enable)
		tree[obj].ob_state &= ~OS_DISABLED;
	else
		tree[obj].ob_state |= OS_DISABLED;
}

/*** ---------------------------------------------------------------------- ***/

static void ShowObj(OBJECT *tree, _WORD obj, gboolean show)
{
	if (show)
		tree[obj].ob_flags &= ~OF_HIDETREE;
	else
		tree[obj].ob_flags |= OF_HIDETREE;
}

/*** ---------------------------------------------------------------------- ***/

void ToolbarUpdate(WINDOW_DATA *win, gboolean redraw)
{
	DOCUMENT *doc = win->data;
	OBJECT *toolbar = win->toolbar;
	
	/* autolocator active? */
	if (doc->buttons.searchbox && win->autolocator != NULL)
	{
		ShowObj(toolbar, TO_BUTTONBOX, FALSE);
		ShowObj(toolbar, TO_SEARCHBOX, TRUE);
		toolbar[TO_SEARCH].ob_spec.tedinfo->te_ptext = win->autolocator;
		return;
	}
	ShowObj(toolbar, TO_SEARCHBOX, FALSE);
	ShowObj(toolbar, TO_BUTTONBOX, TRUE);
	
	doc->buttons.back = TRUE;
	doc->buttons.history = TRUE;
	doc->buttons.bookmarks = TRUE;
	doc->buttons.menu = TRUE;
	doc->buttons.info = TRUE;
	doc->buttons.remarker_running = StartRemarker(win, remarker_check, TRUE) >= 0;
	
	if (win->history == NULL)
	{
		doc->buttons.back = FALSE;
		doc->buttons.history = FALSE;
	}
	
	EnableObj(toolbar, TO_BOOKMARKS, doc->buttons.bookmarks);
	EnableObj(toolbar, TO_REFERENCES, doc->buttons.references);
	EnableObj(toolbar, TO_MENU, doc->buttons.menu);
	ShowObj(toolbar, TO_REMARKER, doc->buttons.remarker_running);
	EnableObj(toolbar, TO_REMARKER, doc->buttons.remarker);
	EnableObj(toolbar, TO_LOAD, doc->buttons.load);
	EnableObj(toolbar, TO_INFO, doc->buttons.info);
	EnableObj(toolbar, TO_BACK, doc->buttons.back);
	EnableObj(toolbar, TO_HISTORY, doc->buttons.history);

	/* is there a catalog file? */
	EnableObj(toolbar, TO_CATALOG, !empty(gl_profile.viewer.catalog_file));

	/* next buttons are type specific */
	EnableObj(toolbar, TO_PREV, doc->buttons.previous);
	EnableObj(toolbar, TO_HOME, doc->buttons.home);
	EnableObj(toolbar, TO_NEXT, doc->buttons.next);
#ifdef TO_NEXT_PHYS
	EnableObj(toolbar, TO_NEXT_PHYS, doc->buttons.nextphys);
#endif
#ifdef TO_PREV_PHYS
	EnableObj(toolbar, TO_PREV_PHYS, doc->buttons.prevphys);
#endif
#ifdef TO_FIRST
	EnableObj(toolbar, TO_FIRST, doc->buttons.first);
#endif
#ifdef TO_LAST
	EnableObj(toolbar, TO_LAST, doc->buttons.last);
#endif

	EnableObj(toolbar, TO_INDEX, doc->buttons.index);
	EnableObj(toolbar, TO_REFERENCES, doc->buttons.references);
	EnableObj(toolbar, TO_HELP, doc->buttons.help);
	EnableObj(toolbar, TO_SAVE, doc->buttons.save);

	if (redraw)
	{
		toolbar_redraw(win);
	}	
}

/*** ---------------------------------------------------------------------- ***/

static void position_popup(WINDOW_DATA *win, _WORD obj, _WORD *x, _WORD *y)
{
	GRECT toolbar;
	
	wind_get_grect(win->whandle, WF_WORKXYWH, &toolbar);
	win->toolbar->ob_x = toolbar.g_x;
	win->toolbar->ob_y = toolbar.g_y;
	objc_offset(win->toolbar, obj, x, y);
	if (win->x_offset)
		*x = toolbar.g_x + win->x_offset;
	if (win->y_offset)
		*y = toolbar.g_y + win->y_offset;
}

/*** ---------------------------------------------------------------------- ***/

/* Handle mouse click on toolbar */
void ToolbarClick(WINDOW_DATA *win, short obj)
{
	_BOOL redraw_button = FALSE;

	if (!obj)
		RemoveSearchBox(win);
	else if ((win->toolbar[obj].ob_state & OS_DISABLED) && obj != TO_REMARKER)
		return;

	CheckFiledate(win);		/* Check if file has changed */
	
	switch (obj)
	{
	case TO_LOAD:
		SelectFileLoad(win);
		break;
	case TO_SAVE:
		SelectFileSave(win);
		break;
	case TO_INDEX:
		GotoIndex(win);
		break;
	case TO_CATALOG:
		GotoCatalog(win);
		break;
	case TO_REFERENCES:
		{
			_WORD x, y;
			
			position_popup(win, obj, &x, &y);
			HypExtRefPopup(win, x, y);
		}
		break;
	case TO_HELP:
		GotoHelp(win);
		break;
	case TO_HISTORY:
		{
			_WORD x, y;
			
			position_popup(win, obj, &x, &y);
			HistoryPopup(win, x, y);
		}
		break;
	case TO_BACK:
		GoBack(win);
		break;
	case TO_NEXT:
	case TO_PREV:
	case TO_HOME:
	case TO_NEXT_PHYS:
	case TO_PREV_PHYS:
	case TO_FIRST:
	case TO_LAST:
		GoThisButton(win, obj);
		break;
	case TO_BOOKMARKS:
		{
			_WORD x, y;
			
			position_popup(win, obj, &x, &y);
			MarkerPopup(win, x, y);
		}
		break;
	case TO_INFO:
		DocumentInfos(win);
		break;
	case TO_MENU:
		{
			_WORD x, y, num;
			OBJECT *tree = rs_tree(CONTEXT);
			
			position_popup(win, obj, &x, &y);
			tree[0].ob_x = x;
			tree[0].ob_y = y;

			num = popup_select(tree, 0, 0);
			BlockOperation(win, num);
		}
		break;
	case TO_REMARKER:
		BlockOperation(win, CO_REMARKER);
		break;
	default:
		break;
	}
	win->toolbar[obj].ob_state &= ~OS_SELECTED;
	if (redraw_button)
		objc_draw_grect(win->toolbar, obj, 1, (GRECT *)&win->toolbar[0].ob_x);
}

/*** ---------------------------------------------------------------------- ***/

void RemoveSearchBox(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	
	/* Is the autolocator/search box displayed? */
	if (doc->buttons.searchbox && win->autolocator)
	{
		doc->buttons.searchbox = FALSE;	/* disable it */
		*win->autolocator = 0;			/* clear autolocator string */

		ToolbarUpdate(win, TRUE);		/* update toolbar */
	}
}
