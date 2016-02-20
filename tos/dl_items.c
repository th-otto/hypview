/*
 * HypView - (c)      - 2006 Philipp Donze
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
#include <mint/arch/nf_ops.h>


OBJECT *iconified_tree;

CHAIN_DATA *iconified_list[MAX_ICONIFY_PLACE] = { NULL };
short iconified_count = 0;
CHAIN_DATA *all_list = NULL;
short modal_items = -1;
_WORD modal_stack[MAX_MODALRECURSION];


#define setmsg(a,d,e,f,g,h) \
	msg[0] = a; \
	msg[1] = gl_apid; \
	msg[2] = 0; \
	msg[3] = d; \
	msg[4] = e; \
	msg[5] = f; \
	msg[6] = g; \
	msg[7] = h

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void add_item(CHAIN_DATA *item)
{
	CHAIN_DATA *ptr = all_list;

	if (ptr)
	{
		while (ptr->next)
			ptr = ptr->next;
		ptr->next = item;
		item->previous = ptr;
	} else
	{
		all_list = item;
		item->previous = NULL;
	}
	item->next = NULL;
}

/*** ---------------------------------------------------------------------- ***/

void remove_item(CHAIN_DATA *item)
{
	CHAIN_DATA *ptr = all_list;

	while (ptr)
	{
		if (ptr == item)
		{
			if (ptr->next)
				ptr->next->previous = ptr->previous;
			if (ptr->previous)
				ptr->previous->next = ptr->next;
			else
				all_list = ptr->next;
			ptr = NULL;
		} else
		{
			ptr = ptr->next;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void FlipIconify(void)
{
	CHAIN_DATA *ptr;
	_WORD msg[8];
	_WORD top;
	
	if (!has_iconify)
		return;

	wind_get_int(DESK, WF_TOP, &top);
	ptr = find_ptr_by_whandle(top);
	if (!ptr)
		return;
	setmsg(WM_ICONIFY, top, -1, -1, -1, -1);
	if (ptr->status & WIS_ICONIFY)
	{
		msg[0] = WM_UNICONIFY;
		wind_get_grect(msg[3], WF_UNICONIFY, (GRECT *) & msg[4]);
	}

	appl_write(gl_apid, 16, msg);
}

/*** ---------------------------------------------------------------------- ***/

void AllIconify(short handle, GRECT *r)
{
	CHAIN_DATA *ptr;
	GRECT big, small;

	ptr = find_ptr_by_whandle(handle);
	if (!ptr)
		return;

	if (iconified_list[0])
	{
		handle = iconified_list[0]->whandle;
		wind_get_grect(handle, WF_CURRXYWH, &small);
	} else
	{
		if (!(ptr->status & WIS_ICONIFY))
		{
			wind_get_grect(handle, WF_CURRXYWH, &big);

			switch (ptr->type)
			{
			case WIN_DIALOG:
				dialog_set_iconify(((DIALOG_DATA *) ptr)->dial, r);
				break;
			case WIN_WINDOW:
				wind_close(ptr->whandle);
				wind_set_grect(ptr->whandle, WF_ICONIFY, r);
				wind_set_str(handle, WF_NAME, iconified_name);
				wind_set_grect(ptr->whandle, WF_UNICONIFYXYWH, &big);
				wind_open_grect(ptr->whandle, r);
				break;
			}

			ptr->status |= WIS_ALLICONIFY | WIS_ICONIFY;
		} else
		{
			ptr->status |= WIS_ALLICONIFY;
		}
		
		wind_get_grect(ptr->whandle, WF_CURRXYWH, r);
		graf_shrinkbox_grect(r, &big);
		iconified_list[iconified_count++] = (CHAIN_DATA *) ptr;

		small = *r;
	}

	wind_set_top(handle);
	{
		_WORD hndl = 0,
			pid,
			opn,
			above,
			below;

		while (wind_get(hndl, WF_OWNER, &pid, &opn, &above, &below))
		{
			ptr = find_ptr_by_whandle(hndl);
			if (ptr && !(ptr->status & WIS_ALLICONIFY) &&
				(ptr->status & WIS_OPEN) && (iconified_count < MAX_ICONIFY_PLACE))
			{
				if ((ptr->status & WIS_OPEN))
					ptr->status |= WIS_MFCLOSE;
			}
			hndl = above;
		}
		while ((ptr = find_ptr_by_status((WIS_OPEN | WIS_MFCLOSE), (WIS_OPEN | WIS_MFCLOSE))) != NULL)
		{
			ptr->status &= ~(WIS_OPEN | WIS_MFCLOSE);
			ptr->status |= WIS_ALLICONIFY;

			wind_get_grect(ptr->whandle, WF_CURRXYWH, &ptr->last);
			wind_close(ptr->whandle);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void CycleItems(void)
{
#if 0
	_WORD handle = 0,
		our = -1,
		opn,
		owner,
		dummy;

	if (modal_items >= 0)
		return;

	while (wind_get(handle, WF_OWNER, &owner, &opn, &handle, &dummy))
	{
		if (opn && owner == gl_apid)
			our = handle;
	}
	if (our != -1)
	{
		SendTopped(our);
	}
#endif
#if 1
	_WORD j = 0;
	_WORD owner;
	_WORD *stack_list;
	_WORD hi, lo;
	_WORD msg[8];
	_WORD dummy;
	
	if (modal_items >= 0)
		return;

	if (wind_get(0, WF_M_WINDLIST, &hi, &lo, &dummy, &dummy))
	{
		stack_list = (_WORD *) (((_LONG)hi << 16) | lo);
		if (stack_list)
		{
			while (stack_list[j])
				j++;

			for (; j > 0; j--)
			{
				wind_get_int(stack_list[j], WF_OWNER, &owner);
				if (owner == gl_apid)
				{
					msg[3] = stack_list[j];
					appl_write(gl_apid, 16, msg);
					break;
				}
			}
		}
	} else
	{
		_WORD handle;
		CHAIN_DATA *start_ptr, *ptr;

		wind_get_int(DESK, WF_TOP, &handle);
		start_ptr = find_ptr_by_whandle(handle);
		if (!start_ptr)
			return;
		ptr = start_ptr;
		do
		{
			ptr = ptr->next;
			if (!ptr)
				ptr = all_list;

			if (ptr->status & WIS_OPEN)
			{
				SendTopped(ptr->whandle);
			}
		} while ((ptr != start_ptr) && !(ptr->status & WIS_OPEN));
	}
#endif
}

/*** ---------------------------------------------------------------------- ***/

void RemoveItems(void)
{
	while (all_list)
	{
		switch (all_list->type)
		{
		case WIN_WINDOW:
			RemoveWindow((WINDOW_DATA *) all_list);
			break;
		case WIN_DIALOG:
			RemoveDialog((DIALOG_DATA *) all_list);
			break;
		case WIN_FILESEL:
			RemoveFileselector((FILESEL_DATA *) all_list);
			break;
		case WIN_FONTSEL:
			RemoveFontselector((FONTSEL_DATA *) all_list);
			break;
		}
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#if USE_MENU
static short menu_enabled = TRUE;

static void SetMenu(short enable)
{
	short title,
	 width,
	 last;

	if (menu_tree)
	{
		title = menu_tree[3].ob_next;
		if (enable == ((menu_tree[title].ob_state & DISABLED) != 0))
		{
			width = menu_tree[3].ob_width;
			do
			{
				width += menu_tree[title].ob_width;
				menu_tnormal(menu_tree, title, TRUE);
				menu_ienable(menu_tree, title | 0x8000, enable);
				last = title;
				title = menu_tree[title].ob_next;
			} while (title != 2);
			menu_ienable(menu_tree, last + 3, enable);
			if (enable)
				menu_tree[2].ob_width = width;
			else
				menu_tree[2].ob_width = menu_tree[3].ob_width;

			menu_enabled = enable;
		}
	}
}
#endif

/*** ---------------------------------------------------------------------- ***/

void ModalItem(void)
{
	wind_get_int(DESK, WF_TOP, &modal_stack[++modal_items]);
#if USE_MENU
	SetMenu(FALSE);
#endif
}

/*** ---------------------------------------------------------------------- ***/

void ItemEvent(EVNT *event)
{
	_WORD whandle = 0, top_window;
	CHAIN_DATA *ptr;

	wind_get_int(DESK, WF_TOP, &top_window);

	if (modal_items >= 0)
	{
		whandle = modal_stack[modal_items];
		if ((event->mwhich & MU_MESAG) && ((event->msg[0] == WM_REDRAW) || (event->msg[0] == WM_MOVED)))
			whandle = event->msg[3];
		else if ((event->mwhich & MU_BUTTON) && (top_window != whandle))
			wind_set_top(whandle);
	} else
	{
		if (event->mwhich & MU_MESAG)
		{
			if ((event->msg[0] >= WM_REDRAW && event->msg[0] <= WM_REPOSED) ||
				event->msg[0] == AP_DRAGDROP ||
				event->msg[0] == WM_WHEEL ||
				event->msg[0] == WM_SHADED ||
				event->msg[0] == WM_UNSHADED)
				whandle = event->msg[3];
		} else if (event->mwhich & MU_BUTTON)
		{
			whandle = wind_find(event->mx, event->my);
		} else
		{
			whandle = top_window;
		}
	}

	ptr = find_ptr_by_whandle(whandle);
	if (ptr && (ptr->status & WIS_OPEN))
	{
		if (ptr->type == WIN_WINDOW)
			WindowEvents((WINDOW_DATA *) ptr, event);
		else if (ptr->type == WIN_DIALOG)
			DialogEvents((DIALOG_DATA *) ptr, event);
		else if (ptr->type == WIN_FILESEL)
			FileselectorEvents((FILESEL_DATA *) ptr, event);
		else if (ptr->type == WIN_FONTSEL)
			FontselectorEvents((FONTSEL_DATA *) ptr, event);
	}
#if USE_MENU
	if ((modal_items < 0) && !menu_enabled)
		SetMenu(TRUE);
#endif
}

/*** ---------------------------------------------------------------------- ***/

CHAIN_DATA *find_ptr_by_whandle(short handle)
{
	CHAIN_DATA *ptr = all_list;

	while (ptr)
	{
		if (ptr->whandle == handle)
			return ptr;
		ptr = ptr->next;
	}
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

CHAIN_DATA *find_ptr_by_type(_WORD type)
{
	CHAIN_DATA *ptr = all_list;

	while (ptr)
	{
		if (ptr->type == type)
			return ptr;
		ptr = ptr->next;
	}
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

CHAIN_DATA *find_ptr_by_status(short mask, short status)
{
	CHAIN_DATA *ptr = all_list;

	while (ptr)
	{
		if ((ptr->status & mask) == status)
			break;
		ptr = ptr->next;
	}
	return ptr;
}
