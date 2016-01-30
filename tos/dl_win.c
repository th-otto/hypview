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


#if USE_TOOLBAR
#define DL_WIN_XADD(ptr)	(ptr->x_offset + ptr->x_margin_left + ptr->x_margin_right)
#define DL_WIN_YADD(ptr)	(ptr->y_offset + ptr->y_margin_top + ptr->y_margin_bottom)
#else
#define DL_WIN_XADD(ptr)	0
#define DL_WIN_YADD(ptr)	0
#endif

static GRECT const small = { 0, 0, 0, 0 };


#define setmsg(a,b,c,d,e,f,g,h) \
	msg[0] = a; \
	msg[1] = b; \
	msg[2] = c; \
	msg[3] = d; \
	msg[4] = e; \
	msg[5] = f; \
	msg[6] = g; \
	msg[7] = h


WINDOW_DATA *CreateWindow(HNDL_WIN proc, short kind, const char *title, WP_UNIT max_w, WP_UNIT max_h, void *user_data)
{
	WINDOW_DATA *ptr;
	GRECT screen;

	ptr = g_new0(WINDOW_DATA, 1);
	if (ptr == NULL)
	{
		form_alert(1, rs_string(DI_MEMORY_ERROR));
		return NULL;
	}
	if (!has_iconify)
		kind &= ~SMALLER;				/* filter out SMALLER */
	ptr->type = WIN_WINDOW;
	ptr->status = 0;
	ptr->proc = proc;
	ptr->owner = gl_apid;
	ptr->title = g_strdup(title);
	ptr->kind = kind;
	ptr->docsize.w = max_w;
	ptr->docsize.h = max_h;
	ptr->docsize.x = 0;
	ptr->docsize.y = 0;
	ptr->x_speed = 1;
	ptr->y_speed = 1;
	ptr->x_margin_left = 0;
	ptr->x_margin_right = 0;
	ptr->y_margin_top = 0;
	ptr->y_margin_bottom = 0;
#if USE_LOGICALRASTER
	ptr->x_raster = 1;
	ptr->y_raster = 1;
#endif
#if USE_TOOLBAR
	ptr->toolbar = NULL;
	ptr->x_offset = 0;
	ptr->y_offset = 0;
#endif
	ptr->data = user_data;

	wind_get_grect(0, WF_WORKXYWH, &screen);
	ptr->whandle = wind_create_grect(ptr->kind, &screen);
	if (ptr->whandle >= 0)
	{
		short ok = TRUE;

#if OPEN_VDI_WORKSTATION
		_WORD i;
		_WORD workin[16];

		for (i = 0; i < 10; workin[i++] = 1)
			;
		workin[10] = 2;
		for (i = 11; i < (_WORD)(sizeof(workin) / sizeof(workin[0])); i++)
			workin[i] = 0;
		ptr->vdi_handle = aes_handle;

		v_opnvwk(workin, &ptr->vdi_handle, ptr->workout);
		if (ptr->vdi_handle)
			vq_extnd(ptr->vdi_handle, 1, ptr->ext_workout);
		else
			ok = FALSE;
#endif
		if (ok && proc(ptr, WIND_INIT, NULL))
		{
			GRECT winsize;

			wind_calc_grect(WC_WORK, ptr->kind, &screen, &winsize);

#if USE_LOGICALRASTER
			winsize.g_w =
				(short) min(DL_WIN_XADD(ptr) + ptr->docsize.w * ptr->x_raster,
							winsize.g_w - (winsize.g_w - DL_WIN_XADD(ptr)) % ptr->x_raster);
			winsize.g_h =
				(short) min(DL_WIN_YADD(ptr) + ptr->docsize.h * ptr->y_raster,
							winsize.g_h - (winsize.g_h - DL_WIN_YADD(ptr)) % ptr->y_raster);
#else
			winsize.g_w = min(DL_WIN_XADD(ptr) + ptr->docsize.w, winsize.g_w);
			winsize.g_h = min(DL_WIN_YADD(ptr) + ptr->docsize.h, winsize.g_h);
#endif

			wind_calc_grect(WC_BORDER, ptr->kind, &winsize, &ptr->full);
		}

		if (!ok)
		{
			proc(ptr, WIND_EXIT, NULL);
#if OPEN_VDI_WORKSTATION
			if (ptr->vdi_handle)
				v_clsvwk(ptr->vdi_handle);
#endif
			wind_delete(ptr->whandle);
			g_free(ptr);
			ptr = NULL;
		}
		if (ptr)
			add_item((CHAIN_DATA *) ptr);
	} else
	{
		g_free(ptr);
		ptr = NULL;
	}
	return ptr;
}


void OpenWindow(WINDOW_DATA *win)
{
	GRECT open_size;
	
	open_size = win->full;

	if (win->whandle < 0)
		return;
	
#if USE_LOGICALRASTER
	if (win->proc(win, WIND_OPENSIZE, &open_size))
	{
		GRECT real_worksize;
		GRECT winsize;

		wind_calc_grect(WC_WORK, win->kind, &open_size, &winsize);
		real_worksize.g_x = winsize.g_x;
		real_worksize.g_y = winsize.g_y;
		real_worksize.g_w = (short) min(32000L, win->docsize.w * win->x_raster + DL_WIN_XADD(win));
		real_worksize.g_h = (short) min(32000L, win->docsize.h * win->y_raster + DL_WIN_YADD(win));
		rc_intersect(&real_worksize, &winsize);
		wind_calc_grect(WC_BORDER, win->kind, &winsize, &open_size);
	}
#else
	win->proc(win, WIND_OPENSIZE, &open_size);
#endif

	graf_growbox_grect(&small, &open_size);
	if (wind_open_grect(win->whandle, &open_size))
	{
		win->status = WIS_OPEN;
		hv_set_title(win, win->title);
		win->proc(win, WIND_OPEN, NULL);
		SetWindowSlider(win);
	}
}


void CloseWindow(WINDOW_DATA *ptr)
{
	if (!ptr)
		return;
	if (ptr->status & WIS_ICONIFY)
	{
		EVNT event;

		memset(&event, 0, sizeof(event));
		event.mwhich = MU_MESAG;
		event.msg[0] = WM_UNICONIFY;
		event.msg[1] = gl_apid;
		event.msg[2] = 0;
		event.msg[3] = ptr->whandle;
		wind_get_grect(ptr->whandle, WF_UNICONIFY, (GRECT *) & event.msg[4]);
		DoEventDispatch(&event);
	}
	if (ptr->status & WIS_ALLICONIFY)
	{
		GRECT sm;
		short i, copy = FALSE;

		wind_get_grect(iconified_list[0]->whandle, WF_CURRXYWH, &sm);

		graf_growbox_grect(&sm, &ptr->last);
		wind_open_grect(ptr->whandle, &ptr->last);
		ptr->status &= ~WIS_ALLICONIFY;
		ptr->status |= WIS_OPEN;
		for (i = 0; i < iconified_count; i++)
		{
			if (copy)
				iconified_list[i - 1] = iconified_list[i];
			if (iconified_list[i] == (CHAIN_DATA *) ptr)
				copy = TRUE;
		}
		iconified_count--;
	}
	if (ptr->status & WIS_OPEN)
	{
		if (ptr->owner == gl_apid && ptr->proc(ptr, WIND_CLOSE, NULL))
		{
			ptr->status &= ~WIS_OPEN;
			wind_get_grect(ptr->whandle, WF_CURRXYWH, &ptr->last);
			wind_close(ptr->whandle);
			graf_shrinkbox_grect(&small, &ptr->last);
			if (modal_items >= 0)
				modal_items--;
		} else
		{
			ptr->status &= ~WIS_OPEN;
			ptr->whandle = -1;
		}
	}
}

void CloseAllWindows(void)
{
	WINDOW_DATA *ptr = (WINDOW_DATA *) all_list;

	while (ptr)
	{
		if (ptr->type == WIN_WINDOW)
			CloseWindow(ptr);
		ptr = ptr->next;
	}
}

void RemoveWindow(WINDOW_DATA *ptr)
{
	GRECT big;
	if (ptr)
	{
		if (ptr->owner == gl_apid)
		{
			if (ptr->status & WIS_OPEN)
			{
				ptr->proc(ptr, WIND_CLOSE, NULL);
				if (ptr->whandle > 0)
				{
					wind_get_grect(ptr->whandle, WF_CURRXYWH, &big);
					wind_close(ptr->whandle);
					graf_shrinkbox_grect(&small, &big);
				}
			}
			ptr->proc(ptr, WIND_EXIT, NULL);
#if OPEN_VDI_WORKSTATION
			v_clsvwk(ptr->vdi_handle);
#endif
			if (ptr->whandle > 0)
				wind_delete(ptr->whandle);
			if (modal_items >= 0)
				modal_items--;
		}
		remove_item((CHAIN_DATA *) ptr);
		g_free(ptr->autolocator);
		g_free(ptr);
	}
}


gboolean ScrollWindow(WINDOW_DATA *ptr, _WORD *r_x, _WORD *r_y)
{
	WP_UNIT oy = ptr->docsize.y;
	WP_UNIT ox = ptr->docsize.x;
	MFDB screen = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	_WORD pxy[8] = { -99 };
	_WORD rel_x, rel_y;
	_WORD abs_rel_x, abs_rel_y, move_screen = TRUE;
	GRECT work, box, redraw;
	_WORD ret;
	
#if USE_LOGICALRASTER
	short lines_width,
	 lines_height;
#endif

	if (r_x)
		rel_x = *r_x;
	else
		rel_x = 0;

	if (r_y)
		rel_y = *r_y;
	else
		rel_y = 0;

	WindowCalcScroll(ptr);
	work = ptr->scroll;
	
	/* restrict rectangle to screen */
	wind_get_grect(DESK, WF_WORKXYWH, &box);
	rc_intersect(&box, &work);

#if USE_LOGICALRASTER
	lines_width = work.g_w / ptr->x_raster;
	lines_height = work.g_h / ptr->y_raster;

	if (rel_x < 0)
		ptr->docsize.x = max(ptr->docsize.x + rel_x, 0);
	else
	{
		ptr->docsize.x = min(ptr->docsize.x + rel_x, ptr->docsize.w - lines_width);
		ptr->docsize.x = max(0, ptr->docsize.x);
	}

	if (rel_y < 0)
		ptr->docsize.y = max(ptr->docsize.y + rel_y, 0);
	else
	{
		ptr->docsize.y = min(ptr->docsize.y + rel_y, ptr->docsize.h - lines_height);
		ptr->docsize.y = max(0, ptr->docsize.y);
	}
#else
	if (rel_x < 0)
		ptr->docsize.x = max(ptr->docsize.x + rel_x, 0);
	else
	{
		ptr->docsize.x = min(ptr->docsize.x + rel_x, ptr->docsize.w - work.g_w);
		ptr->docsize.x = max(0, ptr->docsize.x);
	}

	if (rel_y < 0)
		ptr->docsize.y = max(ptr->docsize.y + rel_y, 0);
	else
	{
		ptr->docsize.y = min(ptr->docsize.y + rel_y, ptr->docsize.h - work.g_h);
		ptr->docsize.y = max(0, ptr->docsize.y);
	}
#endif

	if ((ox == ptr->docsize.x) && (oy == ptr->docsize.y))
	{
		SetWindowSlider(ptr);
		return FALSE;
	}

	rel_x = (short) (ptr->docsize.x - ox);
	rel_y = (short) (ptr->docsize.y - oy);
	if (r_x)
		*r_x = rel_x;
	if (r_y)
		*r_y = rel_y;

#if USE_LOGICALRASTER
	if ((abs(ptr->docsize.x - ox) >= lines_width) || (abs(ptr->docsize.y - oy) >= lines_height))
	{
		move_screen = FALSE;
		abs_rel_x = abs(rel_x);
		abs_rel_y = abs(rel_y);
	} else
	{
		rel_x = rel_x * ptr->x_raster;
		rel_y = rel_y * ptr->y_raster;
		abs_rel_x = abs(rel_x);
		abs_rel_y = abs(rel_y);
	}
#else
	abs_rel_x = abs(rel_x);
	abs_rel_y = abs(rel_y);
	if (abs_rel_x >= work.g_w || abs_rel_y >= work.g_h)
		move_screen = FALSE;
#endif

	graf_mouse(M_OFF, NULL);
	wind_update(BEG_UPDATE);
	ret = wind_get_grect(ptr->whandle, WF_FIRSTXYWH, &box);

	while (ret != 0 && box.g_w && box.g_h)
	{
		if (rc_intersect(&work, &box))
		{
			if (!move_screen || abs_rel_x >= box.g_w || abs_rel_y >= box.g_h)
			{
				ptr->proc(ptr, WIND_REDRAW, (void *) &box);
			} else
			{
				pxy[0] = box.g_x;
				pxy[2] = box.g_x + box.g_w - 1;
				pxy[4] = box.g_x;
				pxy[6] = box.g_x + box.g_w - 1;

				if (rel_x > 0)
				{
					pxy[0] += rel_x;
					pxy[6] -= rel_x;
				} else if (rel_x < 0)
				{
					pxy[2] += rel_x;
					pxy[4] -= rel_x;
				}

				pxy[1] = box.g_y;
				pxy[3] = box.g_y + box.g_h - 1;
				pxy[5] = box.g_y;
				pxy[7] = box.g_y + box.g_h - 1;
				if (rel_y > 0)
				{
					pxy[1] += rel_y;
					pxy[7] -= rel_y;
				} else if (rel_y < 0)
				{
					pxy[3] += rel_y;
					pxy[5] -= rel_y;
				}
#if OPEN_VDI_WORKSTATION
				vro_cpyfm(ptr->vdi_handle, 3, pxy, &screen, &screen);
#elif USE_GLOBAL_VDI
				vro_cpyfm(vdi_handle, 3, pxy, &screen, &screen);
#endif

				redraw = box;

				if (rel_x > 0)
				{
					redraw.g_x += box.g_w - rel_x;
					redraw.g_w = rel_x;
					box.g_w -= rel_x;
				} else if (rel_x < 0)
				{
					redraw.g_w = -rel_x;
					box.g_x -= rel_x;
					box.g_w += rel_x;
				}

				if (rel_x)
					ptr->proc(ptr, WIND_REDRAW, (void *) &redraw);

				redraw = box;

				if (rel_y > 0)
				{
					redraw.g_y += box.g_h - rel_y;
					redraw.g_h = rel_y;
				} else if (rel_y < 0)
				{
					redraw.g_h = -rel_y;
				}

				if (rel_y)
					ptr->proc(ptr, WIND_REDRAW, (void *) &redraw);
			}
		}

		ret = wind_get_grect(ptr->whandle, WF_NEXTXYWH, &box);
	}
	SetWindowSlider(ptr);
	wind_update(END_UPDATE);
	graf_mouse(M_ON, NULL);

	return TRUE;
}


void WindowEvents(WINDOW_DATA *ptr, EVNT *event)
{
	if (ptr->status & WIS_ICONIFY)
	{
		if (event->mwhich & MU_KEYBD)
			event->mwhich &= ~MU_KEYBD;
		if (event->mwhich & MU_BUTTON)
			event->mwhich &= ~MU_BUTTON;
		if (event->mwhich & MU_MESAG)
		{
			switch (event->msg[0])
			{
			case WM_REDRAW:
			case WM_UNICONIFY:
			case WM_MOVED:
				break;
			case WM_TOPPED:
				wind_set_int(event->msg[3], WF_TOP, 0);
			default:
				event->mwhich &= ~MU_MESAG;
				break;
			}
		}
	}

	if (event->mwhich & MU_BUTTON)
	{
#if USE_TOOLBAR
		GRECT toolbar;

		wind_get_grect(ptr->whandle, WF_WORKXYWH, &toolbar);

		if (ptr->toolbar)
		{
			ptr->toolbar->ob_x = toolbar.g_x;
			ptr->toolbar->ob_y = toolbar.g_y;

			if (ptr->y_offset)
			{
				ptr->toolbar->ob_width = toolbar.g_w;
				toolbar.g_h = DL_WIN_YADD(ptr);
			} else if (ptr->x_offset)
			{
				ptr->toolbar->ob_height = toolbar.g_h;
				toolbar.g_h = DL_WIN_XADD(ptr);
			}

			ptr->proc(ptr, WIND_TBUPDATE, &toolbar);

			/* click i toolbar? */
			if ((event->mx >= toolbar.g_x) && (event->my >= toolbar.g_y) &&
				(event->mx < toolbar.g_x + toolbar.g_w) && (event->my < toolbar.g_y + toolbar.g_h))
			{
				_WORD num;

				/* which object? */
				num = objc_find(ptr->toolbar, 0, MAX_DEPTH, event->mx, event->my);

				/*
				 * check for legal (>=0), selectable, and active object
				 * with exit or touchexit flag
				 */
				if ((num >= 0) && (ptr->toolbar[num].ob_flags & OF_SELECTABLE) &&
					!(ptr->toolbar[num].ob_state & OS_DISABLED) &&
					(ptr->toolbar[num].ob_flags & (OF_EXIT | OF_TOUCHEXIT)))
				{
					if ((ptr->toolbar[num].ob_flags & OF_TOUCHEXIT) ||
						((ptr->toolbar[num].ob_flags & OF_EXIT) && graf_watchbox(ptr->toolbar, num, OS_SELECTED, 0)))
					{
						evnt_timer_gemlib(10);
						ptr->proc(ptr, WIND_TBCLICK, &num);
					}
				}
			} else
			{
				ptr->proc(ptr, WIND_CLICK, event);
			}
		} else
		{
			ptr->proc(ptr, WIND_CLICK, event);
		}
#else
		ptr->proc(ptr, WIND_CLICK, event);
#endif
	}
	if (event->mwhich & MU_KEYBD)
	{
		ConvertKeypress(&event->key, &event->kstate);
		ptr->proc(ptr, WIND_KEYPRESS, event);
	}
	if (event->mwhich & MU_MESAG)
	{
		if (event->msg[3] != ptr->whandle)	/* message for a different window?*/
			return;

		event->mwhich &= ~MU_MESAG;
		switch (event->msg[0])
		{
		case WM_REDRAW:
			{
				GRECT box;
				_WORD ret;
				
				wind_update(BEG_UPDATE);
				if (ptr->status & WIS_ICONIFY)
				{
					wind_get_grect(event->msg[3], WF_WORKXYWH, &box);
					dial_library_tree[0].ob_x = box.g_x;
					dial_library_tree[0].ob_y = box.g_y;
					ret = wind_get_grect(event->msg[3], WF_FIRSTXYWH, &box);
					graf_mouse(M_OFF, NULL);
					while (ret != 0 && box.g_w && box.g_h)
					{
						if (rc_intersect((GRECT *) & event->msg[4], &box))
							objc_draw_grect(dial_library_tree, 0, 1, &box);
						ret = wind_get_grect(event->msg[3], WF_NEXTXYWH, &box);
					}
					graf_mouse(M_ON, NULL);
				} else
				{
#if USE_TOOLBAR
					GRECT toolbar;

					wind_get_grect(event->msg[3], WF_WORKXYWH, &toolbar);
					if (ptr->toolbar)
					{
						ptr->toolbar->ob_x = toolbar.g_x;
						ptr->toolbar->ob_y = toolbar.g_y;
						if (ptr->y_offset)	/* horizontal toolbar? */
						{
							ptr->toolbar->ob_width = toolbar.g_w;	/* adjust object width */
							toolbar.g_h = ptr->y_offset;
						}
						if (ptr->x_offset)	/* vertical toolbar? */
						{
							ptr->toolbar->ob_height = toolbar.g_h;	/* adjust object height */
							toolbar.g_w = ptr->x_offset;
						}

						ptr->proc(ptr, WIND_TBUPDATE, &toolbar);
					} else
					{
						toolbar.g_w = 0;
						toolbar.g_h = 0;
					}
#endif
					ret = wind_get_grect(event->msg[3], WF_FIRSTXYWH, &box);

					graf_mouse(M_OFF, NULL);
					while (ret != 0 && box.g_w && box.g_h)
					{
#if USE_TOOLBAR
						GRECT temp_tbar;

						temp_tbar = toolbar;
						if (rc_intersect(&box, &temp_tbar))
						{
							if (ptr->x_offset)	/* vertical toolbar? */
							{
								box.g_x += temp_tbar.g_w;
								box.g_w -= temp_tbar.g_w;
							}
							if (ptr->y_offset)	/* horizontall Toolbar? */
							{
								box.g_y += temp_tbar.g_h;
								box.g_h -= temp_tbar.g_h;
							}
						}
						if (rc_intersect((GRECT *) & event->msg[4], &temp_tbar))
							objc_draw_grect(ptr->toolbar, 0, MAX_DEPTH, &temp_tbar);
#endif
						if (rc_intersect((GRECT *) & event->msg[4], &box))
							ptr->proc(ptr, WIND_REDRAW, (void *) &box);
						ret = wind_get_grect(event->msg[3], WF_NEXTXYWH, &box);
					}
					graf_mouse(M_ON, NULL);
				}
				wind_update(END_UPDATE);
			}
			break;
			
		case WM_TOPPED:
			if (ptr->proc(ptr, WIND_TOPPED, event))
				wind_set_int(event->msg[3], WF_TOP, 0);
			break;
			
		case WM_CLOSED:
			RemoveWindow(ptr);
			break;

		case WM_FULLED:
			{
				GRECT big;

				if (ptr->status & WIS_FULL)
				{
					wind_get_grect(event->msg[3], WF_CURRXYWH, &big);

					ptr->proc(ptr, WIND_FULLED, &ptr->last);
					graf_shrinkbox_grect(&ptr->last, &big);
					wind_set_grect(event->msg[3], WF_CURRXYWH, &ptr->last);

					ptr->status &= ~WIS_FULL;
					SetWindowSlider(ptr);
				} else
				{
					GRECT win;
					_WORD rel_x, rel_y;

					wind_get_grect(event->msg[3], WF_CURRXYWH, &ptr->last);

					ptr->proc(ptr, WIND_FULLED, &ptr->full);
					graf_growbox_grect(&ptr->last, &ptr->full);
					wind_set_grect(event->msg[3], WF_CURRXYWH, &ptr->full);

					ptr->status |= WIS_FULL;

					wind_get_grect(event->msg[3], WF_WORKXYWH, &win);
#if USE_LOGICALRASTER
					rel_x = (short) ((ptr->docsize.w - ptr->docsize.x) - (win.g_w - DL_WIN_XADD(ptr)) / ptr->x_raster);
					rel_y = (short) ((ptr->docsize.h - ptr->docsize.y) - (win.g_h - DL_WIN_YADD(ptr)) / ptr->y_raster);
#else
					rel_x = ptr->docsize.w - ptr->docsize.x - (win.g_w - DL_WIN_XADD(ptr));
					rel_y = ptr->docsize.h - ptr->docsize.y - (win.g_h - DL_WIN_YADD(ptr));
#endif
					if ((rel_x < 0) || (rel_y < 0))
					{
						rel_x = min(rel_x, 0);
						rel_y = min(rel_y, 0);
						ScrollWindow(ptr, &rel_x, &rel_y);
					} else
					{
						SetWindowSlider(ptr);
					}
				}
			}
			break;
			
		case WM_ARROWED:
			{
				_WORD rel_x = 0, rel_y = 0;
				GRECT win;

				wind_get_grect(event->msg[3], WF_WORKXYWH, &win);

#if USE_TOOLBAR
				/* take toolbar into account */
				win.g_w -= DL_WIN_XADD(ptr);
				win.g_h -= DL_WIN_YADD(ptr);
#endif

				switch (event->msg[4])
				{
#if USE_LOGICALRASTER
				case WA_UPPAGE:
					rel_y = -win.g_h / ptr->y_raster;
					break;
				case WA_DNPAGE:
					rel_y = win.g_h / ptr->y_raster;
					break;
				case WA_LFPAGE:
					rel_x = -win.g_w / ptr->x_raster;
					break;
				case WA_RTPAGE:
					rel_x = win.g_w / ptr->x_raster;
					break;
#else
				case WA_UPPAGE:
					rel_y = -win.g_h;
					break;
				case WA_DNPAGE:
					rel_y = win.g_h;
					break;
				case WA_LFPAGE:
					rel_x = -win.g_w;
					break;
				case WA_RTPAGE:
					rel_x = win.g_w;
					break;
#endif
				case WA_UPLINE:
					rel_y = -ptr->y_speed;
					break;
				case WA_DNLINE:
					rel_y = ptr->y_speed;
					break;
				case WA_LFLINE:
					rel_x = -ptr->x_speed;
					break;
				case WA_RTLINE:
					rel_x = ptr->x_speed;
					break;
				}
				ScrollWindow(ptr, &rel_x, &rel_y);
			}
			break;

		case WM_HSLID:
			{
				_WORD rel_x;
				GRECT win;

				wind_get_grect(event->msg[3], WF_WORKXYWH, &win);
#if USE_LOGICALRASTER
				rel_x =
					(short) ((event->msg[4] * (ptr->docsize.w - (win.g_w - DL_WIN_XADD(ptr)) / ptr->x_raster)) / 1000 -
							 ptr->docsize.x);
#else
				rel_x =
					(short) (((long) event->msg[4] * (ptr->docsize.w - win.g_w + DL_WIN_XADD(ptr))) / 1000L - ptr->docsize.x);
#endif
				ScrollWindow(ptr, &rel_x, NULL);
			}
			break;

		case WM_VSLID:
			{
				_WORD rel_y;
				GRECT win;

				wind_get_grect(event->msg[3], WF_WORKXYWH, &win);
#if USE_LOGICALRASTER
				rel_y =
					(short) ((event->msg[4] * (ptr->docsize.h - (win.g_h - DL_WIN_YADD(ptr)) / ptr->y_raster)) / 1000 - ptr->docsize.y);
#else
				rel_y =
					(short) (((long) event->msg[4] * (ptr->docsize.h - win.g_h + DL_WIN_YADD(ptr))) / 1000L - ptr->docsize.y);
#endif
				ScrollWindow(ptr, NULL, &rel_y);
			}
			break;

		case WM_SIZED:
			{
				_WORD rel_x, rel_y;

				ptr->proc(ptr, WIND_SIZED, &event->msg[4]);
				wind_set_grect(event->msg[3], WF_CURRXYWH, (GRECT *) & event->msg[4]);
				ptr->status &= ~WIS_FULL;

				WindowCalcScroll(ptr);
#if USE_LOGICALRASTER
				rel_x = (short) ((ptr->docsize.w - ptr->docsize.x) - ptr->scroll.g_w / ptr->x_raster);
				rel_y = (short) ((ptr->docsize.h - ptr->docsize.y) - ptr->scroll.g_h / ptr->y_raster);
#else
				rel_x = ptr->docsize.w - ptr->docsize.x - ptr->scroll.g_w;
				rel_y = ptr->docsize.h - ptr->docsize.y - ptr->scroll.g_h;
#endif
				if (rel_x < 0 || rel_y < 0)
				{
					rel_x = min(rel_x, 0);
					rel_y = min(rel_y, 0);
					ScrollWindow(ptr, &rel_x, &rel_y);
				} else
				{
					SetWindowSlider(ptr);
				}
			}
			break;

		case WM_MOVED:
			ptr->proc(ptr, WIND_MOVED, &event->msg[4]);
			wind_set_grect(event->msg[3], WF_CURRXYWH, (GRECT *) & event->msg[4]);
			ptr->status &= ~WIS_FULL;
			SetWindowSlider(ptr);
			break;

		case WM_NEWTOP:
			ptr->proc(ptr, WIND_NEWTOP, NULL);
			break;

		case WM_UNTOPPED:
			ptr->proc(ptr, WIND_UNTOPPED, NULL);
			break;

		case WM_ONTOP:
			ptr->proc(ptr, WIND_ONTOP, NULL);
			break;

		case WM_BOTTOMED:
			if (ptr->proc(ptr, WIND_BOTTOM, event))
				wind_set_int(event->msg[3], WF_BOTTOM, 0);
			break;

		case WM_ICONIFY:
			IconifyWindow(ptr, (GRECT *) & event->msg[4]);
			ptr->proc(ptr, WIND_ICONIFY, (void *) &event->msg[4]);
			break;

		case WM_UNICONIFY:
			UniconifyWindow(ptr);
			ptr->proc(ptr, WIND_UNICONIFY, (void *) &event->msg[4]);
			break;

		case WM_ALLICONIFY:
			AllIconify(event->msg[3], (GRECT *) & event->msg[4]);
			ptr->proc(ptr, WIND_ALLICONIFY, (void *) &event->msg[4]);
			break;

		default:
			event->mwhich |= MU_MESAG;
			break;
		}
	}
}

void SetWindowSlider(WINDOW_DATA * ptr)
{
	GRECT win;
	long temp;

	if (ptr->status & WIS_ICONIFY)
		return;

	wind_get_grect(ptr->whandle, WF_WORKXYWH, &win);

#if USE_LOGICALRASTER
	temp = ptr->docsize.w - (win.g_w - DL_WIN_XADD(ptr)) / ptr->x_raster;
#else
	temp = ptr->docsize.w - (win.g_w - DL_WIN_XADD(ptr));
#endif
	if (temp > 0)
	{
		wind_set_int(ptr->whandle, WF_HSLIDE, (short) (ptr->docsize.x * 1000L / temp));
#if USE_LOGICALRASTER
		wind_set_int(ptr->whandle, WF_HSLSIZE, (short) ((win.g_w - DL_WIN_XADD(ptr)) / ptr->x_raster * 1000L / ptr->docsize.w));
#else
		wind_set_int(ptr->whandle, WF_HSLSIZE, (short) ((win.g_w - DL_WIN_XADD(ptr)) * 1000L / ptr->docsize.w));
#endif
	} else
	{
		wind_set_int(ptr->whandle, WF_HSLIDE, 1000);
		wind_set_int(ptr->whandle, WF_HSLSIZE, 1000);
	}
#if USE_LOGICALRASTER
	temp = ptr->docsize.h - (win.g_h - DL_WIN_YADD(ptr)) / ptr->y_raster;
#else
	temp = ptr->docsize.h - (win.g_h - DL_WIN_YADD(ptr));
#endif
	if (temp > 0)
	{
		wind_set_int(ptr->whandle, WF_VSLIDE, (short) (ptr->docsize.y * 1000L / temp));
#if USE_LOGICALRASTER
		wind_set_int(ptr->whandle, WF_VSLSIZE, (short) ((win.g_h - DL_WIN_YADD(ptr)) * 1000L / ptr->y_raster / ptr->docsize.h));
#else
		wind_set_int(ptr->whandle, WF_VSLSIZE, (short) ((win.g_h - DL_WIN_YADD(ptr)) * 1000L / ptr->docsize.h));
#endif
	} else
	{
		wind_set_int(ptr->whandle, WF_VSLIDE, 1000);
		wind_set_int(ptr->whandle, WF_VSLSIZE, 1000);
	}
}


void ResizeWindow(WINDOW_DATA *ptr, WP_UNIT max_w, WP_UNIT max_h)
{
	GRECT wind;

	wind_get_grect(DESK, WF_WORKXYWH, &wind);
	wind_calc_grect(WC_WORK, ptr->kind, &wind, &wind);
#if USE_LOGICALRASTER
	wind.g_w = (short) min(DL_WIN_XADD(ptr) + max_w * ptr->x_raster, wind.g_w - (wind.g_w - DL_WIN_XADD(ptr)) % ptr->x_raster);
	wind.g_h = (short) min(DL_WIN_YADD(ptr) + max_h * ptr->y_raster, wind.g_h - (wind.g_h - DL_WIN_YADD(ptr)) % ptr->y_raster);
#else
	wind.g_w = min(DL_WIN_XADD(ptr) + max_w, wind.g_w);
	wind.g_h = min(DL_WIN_YADD(ptr) + max_h, wind.g_h);
#endif
	ptr->docsize.w = max_w;
	ptr->docsize.h = max_h;

	wind_calc_grect(WC_BORDER, ptr->kind, &wind, &ptr->full);
}


void IconifyWindow(WINDOW_DATA * ptr, GRECT * r)
{
	GRECT current_size;
	GRECT new_pos;

	if (ptr->status & WIS_ICONIFY)
		return;

	wind_get_grect(ptr->whandle, WF_CURRXYWH, &current_size);
	wind_close(ptr->whandle);
	wind_set_grect(ptr->whandle, WF_ICONIFY, r);
	wind_set_grect(ptr->whandle, WF_UNICONIFYXYWH, &current_size);
	wind_get_grect(ptr->whandle, WF_CURRXYWH, &new_pos);
	if (new_pos.g_x != -1)
		graf_shrinkbox_grect(&new_pos, &current_size);

	wind_get_grect(ptr->whandle, WF_WORKXYWH, &new_pos);
	dial_library_tree[DI_ICON].ob_x = (new_pos.g_w - dial_library_tree[DI_ICON].ob_width) >> 1;
	dial_library_tree[DI_ICON].ob_y = (new_pos.g_h - dial_library_tree[DI_ICON].ob_height) >> 1;

	wind_open_grect(ptr->whandle, r);
	ptr->status |= WIS_ICONIFY;
	CycleItems();
}


void UniconifyWindow(WINDOW_DATA *ptr)
{
	GRECT small;
	GRECT current_size;

	if (!(ptr->status & WIS_ICONIFY))
		return;

	wind_get_grect(ptr->whandle, WF_CURRXYWH, &small);

	if ((CHAIN_DATA *) ptr == iconified_list[0])
	{
		CHAIN_DATA *ptr2;

		while (--iconified_count > 0)
		{
			ptr2 = iconified_list[iconified_count];
			graf_growbox_grect(&small, &ptr2->last);
			wind_open_grect(ptr2->whandle, &ptr2->last);
			ptr2->status &= ~WIS_ALLICONIFY;
			ptr2->status |= WIS_OPEN;
		};
		iconified_list[0] = NULL;
		iconified_count = 0;
		ptr->status &= ~WIS_ALLICONIFY;
	}

	wind_get_grect(ptr->whandle, WF_UNICONIFY, &current_size);
	graf_growbox_grect(&small, &current_size);
	wind_set_int(ptr->whandle, WF_TOP, 0);
	wind_set_grect(ptr->whandle, WF_UNICONIFY, &current_size);
	ptr->status &= ~WIS_ICONIFY;
}

#if USE_TOOLBAR
void DrawToolbar(WINDOW_DATA * win)
{
	if (!win->toolbar)
		return;
	if (win->status & WIS_ICONIFY)
		return;
	if (win->status & WIS_OPEN)
	{
		GRECT toolbar, box;
		_WORD ret;
		
		wind_get_grect(win->whandle, WF_WORKXYWH, &toolbar);

		win->toolbar->ob_x = toolbar.g_x;
		win->toolbar->ob_y = toolbar.g_y;
		if (win->y_offset)				/* horizontal toolbar? */
		{
			win->toolbar->ob_width = toolbar.g_w;	/* adjust object width */
			toolbar.g_h = win->y_offset;
		}
		if (win->x_offset)				/* vertical toolbar? */
		{
			win->toolbar->ob_height = toolbar.g_h;	/* adjust object height */
			toolbar.g_w = win->x_offset;
		}

		win->proc(win, WIND_TBUPDATE, &toolbar);

		wind_update(BEG_UPDATE);
		ret = wind_get_grect(win->whandle, WF_FIRSTXYWH, &box);
		graf_mouse(M_OFF, NULL);
		while (ret != 0 && box.g_w && box.g_h)
		{
			GRECT temp_tbar;

			temp_tbar = toolbar;
			if (rc_intersect(&box, &temp_tbar))
				objc_draw_grect(win->toolbar, 0, MAX_DEPTH, &temp_tbar);
			ret = wind_get_grect(win->whandle, WF_NEXTXYWH, &box);
		}
		graf_mouse(M_ON, NULL);
		wind_update(END_UPDATE);
	}
}
#endif

WINDOW_DATA *find_openwindow_by_whandle(short handle)
{
	WINDOW_DATA *ptr = (WINDOW_DATA *) all_list;

	while (ptr)
	{
		if ((ptr->type == WIN_WINDOW) && (ptr->whandle == handle) && (ptr->status & WIS_OPEN))
			return (ptr);
		ptr = ptr->next;
	}
	return NULL;
}

WINDOW_DATA *find_window_by_whandle(short handle)
{
	WINDOW_DATA *ptr = (WINDOW_DATA *) all_list;

	while (ptr)
	{
		if ((ptr->type == WIN_WINDOW) && (ptr->whandle == handle))
			return ptr;
		ptr = ptr->next;
	}
	return NULL;
}

WINDOW_DATA *find_window_by_proc(HNDL_WIN proc)
{
	WINDOW_DATA *ptr = (WINDOW_DATA *) all_list;

	while (ptr)
	{
		if ((ptr->type == WIN_WINDOW) && (ptr->proc == proc))
			return (ptr);
		ptr = ptr->next;
	}
	return NULL;
}

WINDOW_DATA *find_window_by_data(void *data)
{
	WINDOW_DATA *ptr = (WINDOW_DATA *) all_list;

	while (ptr)
	{
		if ((ptr->type == WIN_WINDOW) && (ptr->data == data))
			return (ptr);
		ptr = ptr->next;
	}
	return NULL;
}

int count_window(void)
{
	int i;
	WINDOW_DATA *ptr = (WINDOW_DATA *) all_list;

	i = 0;
	while (ptr)
	{
		if (ptr->type == WIN_WINDOW && ptr->owner == gl_apid)
			i++;
		ptr = ptr->next;
	}
	return i;
}
