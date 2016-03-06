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

void ToolbarUpdate(WINDOW_DATA *win, gboolean redraw)
{
	DOCUMENT *doc = win->data;
	OBJECT *toolbar = win->toolbar;
	
	/* autolocator active? */
	if (doc->buttons.searchbox && win->autolocator != NULL)
	{
		toolbar[TO_BUTTONBOX].ob_flags |= OF_HIDETREE;
		toolbar[TO_SEARCHBOX].ob_flags &= ~OF_HIDETREE;
		toolbar[TO_SEARCH].ob_spec.tedinfo->te_ptext = win->autolocator;
		return;
	}
	toolbar[TO_SEARCHBOX].ob_flags |= OF_HIDETREE;
	toolbar[TO_BUTTONBOX].ob_flags &= ~OF_HIDETREE;
	
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
	
	if (doc->buttons.bookmarks)
		toolbar[TO_BOOKMARKS].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_BOOKMARKS].ob_state |= OS_DISABLED;

	if (doc->buttons.references)
		toolbar[TO_REFERENCES].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_REFERENCES].ob_state |= OS_DISABLED;

	if (doc->buttons.menu)
		toolbar[TO_MENU].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_MENU].ob_state |= OS_DISABLED;
	
	if (doc->buttons.remarker_running)
		toolbar[TO_REMARKER].ob_flags &= ~OF_HIDETREE;
	else
		toolbar[TO_REMARKER].ob_flags |= OF_HIDETREE;
	
	if (doc->buttons.remarker)
		toolbar[TO_REMARKER].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_REMARKER].ob_state |= OS_DISABLED;
	
	if (doc->buttons.load)
		toolbar[TO_LOAD].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_LOAD].ob_state |= OS_DISABLED;

	if (doc->buttons.info)
		toolbar[TO_INFO].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_INFO].ob_state |= OS_DISABLED;

	if (doc->buttons.back)
		toolbar[TO_BACK].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_BACK].ob_state |= OS_DISABLED;

	if (doc->buttons.history)
		toolbar[TO_HISTORY].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_HISTORY].ob_state |= OS_DISABLED;

	/* is there a catalog file? */
	if (!empty(gl_profile.viewer.catalog_file))
		toolbar[TO_CATALOG].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_CATALOG].ob_state |= OS_DISABLED;

	/* next buttons are type specific */
	if (doc->buttons.previous)
		toolbar[TO_PREV].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_PREV].ob_state |= OS_DISABLED;

	if (doc->buttons.home)
		toolbar[TO_HOME].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_HOME].ob_state |= OS_DISABLED;

	if (doc->buttons.next)
		toolbar[TO_NEXT].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_NEXT].ob_state |= OS_DISABLED;

	if (doc->buttons.index)
		toolbar[TO_INDEX].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_INDEX].ob_state |= OS_DISABLED;

	if (doc->buttons.references)
		toolbar[TO_REFERENCES].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_REFERENCES].ob_state |= OS_DISABLED;

	if (doc->buttons.help)
		toolbar[TO_HELP].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_HELP].ob_state |= OS_DISABLED;

	if (doc->buttons.save)
		toolbar[TO_SAVE].ob_state &= ~OS_DISABLED;
	else
		toolbar[TO_SAVE].ob_state |= OS_DISABLED;

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
