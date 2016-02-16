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
#include "hypdebug.h"


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
	ptr->x_raster = 1;
	ptr->y_raster = 1;
#if USE_TOOLBAR
	ptr->toolbar = NULL;
	ptr->x_offset = 0;
	ptr->y_offset = 0;
#endif
	ptr->data = (DOCUMENT *)user_data;

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

			winsize.g_w =
				(short) min(DL_WIN_XADD(ptr) + ptr->docsize.w,
							winsize.g_w - (winsize.g_w - DL_WIN_XADD(ptr)) % ptr->x_raster);
			winsize.g_h =
				(short) min(DL_WIN_YADD(ptr) + ptr->docsize.h,
							winsize.g_h - (winsize.g_h - DL_WIN_YADD(ptr)) % ptr->y_raster);

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
	
	if (win->proc(win, WIND_OPENSIZE, &open_size))
	{
		GRECT real_worksize;
		GRECT winsize;

		wind_calc_grect(WC_WORK, win->kind, &open_size, &winsize);
		real_worksize.g_x = winsize.g_x;
		real_worksize.g_y = winsize.g_y;
		real_worksize.g_w = (short) min(32000L, win->docsize.w + DL_WIN_XADD(win));
		real_worksize.g_h = (short) min(32000L, win->docsize.h + DL_WIN_YADD(win));
		rc_intersect(&real_worksize, &winsize);
		wind_calc_grect(WC_BORDER, win->kind, &winsize, &open_size);
	}

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


gboolean ScrollWindow(WINDOW_DATA *win, WP_UNIT rel_x, WP_UNIT rel_y)
{
	WP_UNIT oy = win->docsize.y;
	WP_UNIT ox = win->docsize.x;
	MFDB screen = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	_WORD pxy[8];
	WP_UNIT abs_rel_x, abs_rel_y;
	gboolean move_screen = TRUE;
	GRECT work, box, redraw;
	_WORD ret;
	
	WP_UNIT lines_width;
	WP_UNIT lines_height;

	work = win->scroll;
	
	/* restrict rectangle to screen */
	wind_get_grect(DESK, WF_WORKXYWH, &box);
	rc_intersect(&box, &work);

	lines_width = (win->scroll.g_w / win->x_raster) * win->x_raster;
	lines_height = (win->scroll.g_h / win->y_raster) * win->y_raster;
	
	rel_x *= win->x_raster;
	rel_y *= win->y_raster;
	
	if (rel_x < 0)
	{
		win->docsize.x = max(win->docsize.x + rel_x, 0);
	} else
	{
		win->docsize.x = min(win->docsize.x + rel_x, win->docsize.w - lines_width);
		win->docsize.x = max(0, win->docsize.x);
	}

	if (rel_y < 0)
	{
		win->docsize.y = max(win->docsize.y + rel_y, 0);
	} else
	{
		win->docsize.y = min(win->docsize.y + rel_y, win->docsize.h - lines_height);
		win->docsize.y = max(0, win->docsize.y);
	}
	
	if (ox == win->docsize.x && oy == win->docsize.y)
	{
		SetWindowSlider(win);
		return FALSE;
	}

	rel_x = win->docsize.x - ox;
	rel_y = win->docsize.y - oy;

	abs_rel_x = abs(rel_x);
	abs_rel_y = abs(rel_y);
	if (abs_rel_x >= lines_width || abs_rel_y >= lines_height)
	{
		move_screen = FALSE;
	}

	graf_mouse(M_OFF, NULL);
	wind_update(BEG_UPDATE);
	work = win->scroll;
	ret = wind_get_grect(win->whandle, WF_FIRSTXYWH, &box);

	while (ret != 0 && box.g_w && box.g_h)
	{
		if (rc_intersect(&work, &box))
		{
			if (!move_screen || abs_rel_x >= box.g_w || abs_rel_y >= box.g_h)
			{
				win->proc(win, WIND_REDRAW, (void *) &box);
			} else
			{
				pxy[0] = box.g_x;
				pxy[2] = box.g_x + box.g_w - 1;
				pxy[4] = box.g_x;
				pxy[6] = box.g_x + box.g_w - 1;

				if (rel_x > 0)
				{
					pxy[0] += (_WORD)rel_x;
					pxy[6] -= (_WORD)rel_x;
				} else if (rel_x < 0)
				{
					pxy[2] += (_WORD)rel_x;
					pxy[4] -= (_WORD)rel_x;
				}

				pxy[1] = box.g_y;
				pxy[3] = box.g_y + box.g_h - 1;
				pxy[5] = box.g_y;
				pxy[7] = box.g_y + box.g_h - 1;
				if (rel_y > 0)
				{
					pxy[1] += (_WORD)rel_y;
					pxy[7] -= (_WORD)rel_y;
				} else if (rel_y < 0)
				{
					pxy[3] += (_WORD)rel_y;
					pxy[5] -= (_WORD)rel_y;
				}
#if OPEN_VDI_WORKSTATION
				vro_cpyfm(win->vdi_handle, 3, pxy, &screen, &screen);
#elif USE_GLOBAL_VDI
				vro_cpyfm(vdi_handle, 3, pxy, &screen, &screen);
#endif

				redraw = box;

				if (rel_x > 0)
				{
					redraw.g_x += box.g_w - (_WORD)rel_x;
					redraw.g_w = (_WORD)rel_x;
					box.g_w -= (_WORD)rel_x;
				} else if (rel_x < 0)
				{
					redraw.g_w = -(_WORD)rel_x;
					box.g_x -= (_WORD)rel_x;
					box.g_w += (_WORD)rel_x;
				}

				if (rel_x)
					win->proc(win, WIND_REDRAW, (void *) &redraw);

				redraw = box;

				if (rel_y > 0)
				{
					redraw.g_y += box.g_h - (_WORD)rel_y;
					redraw.g_h = (_WORD)rel_y;
				} else if (rel_y < 0)
				{
					redraw.g_h = -(_WORD)rel_y;
				}

				if (rel_y)
					win->proc(win, WIND_REDRAW, (void *) &redraw);
			}
		}

		ret = wind_get_grect(win->whandle, WF_NEXTXYWH, &box);
	}
	SetWindowSlider(win);
	wind_update(END_UPDATE);
	graf_mouse(M_ON, NULL);

	return TRUE;
}


void WindowEvents(WINDOW_DATA *win, EVNT *event)
{
	if (win->status & WIS_ICONIFY)
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

		wind_get_grect(win->whandle, WF_WORKXYWH, &toolbar);

		if (win->toolbar)
		{
			win->toolbar->ob_x = toolbar.g_x;
			win->toolbar->ob_y = toolbar.g_y;

			if (win->y_offset)
			{
				win->toolbar->ob_width = toolbar.g_w;
				toolbar.g_h = DL_WIN_YADD(win);
			} else if (win->x_offset)
			{
				win->toolbar->ob_height = toolbar.g_h;
				toolbar.g_h = DL_WIN_XADD(win);
			}

			win->proc(win, WIND_TBUPDATE, &toolbar);

			/* click i toolbar? */
			if ((event->mx >= toolbar.g_x) && (event->my >= toolbar.g_y) &&
				(event->mx < toolbar.g_x + toolbar.g_w) && (event->my < toolbar.g_y + toolbar.g_h))
			{
				_WORD num;

				/* which object? */
				num = objc_find(win->toolbar, 0, MAX_DEPTH, event->mx, event->my);

				/*
				 * check for legal (>=0), selectable, and active object
				 * with exit or touchexit flag
				 */
				if ((num >= 0) && (win->toolbar[num].ob_flags & OF_SELECTABLE) &&
					!(win->toolbar[num].ob_state & OS_DISABLED) &&
					(win->toolbar[num].ob_flags & (OF_EXIT | OF_TOUCHEXIT)))
				{
					if ((win->toolbar[num].ob_flags & OF_TOUCHEXIT) ||
						((win->toolbar[num].ob_flags & OF_EXIT) && graf_watchbox(win->toolbar, num, OS_SELECTED, 0)))
					{
						evnt_timer_gemlib(10);
						win->proc(win, WIND_TBCLICK, &num);
					}
				}
			} else
			{
				win->proc(win, WIND_CLICK, event);
			}
		} else
		{
			win->proc(win, WIND_CLICK, event);
		}
#else
		win->proc(win, WIND_CLICK, event);
#endif
	}
	if (event->mwhich & MU_KEYBD)
	{
		ConvertKeypress(&event->key, &event->kstate);
		win->proc(win, WIND_KEYPRESS, event);
	}
	if (event->mwhich & MU_MESAG)
	{
		if (event->msg[3] != win->whandle)	/* message for a different window?*/
			return;

		event->mwhich &= ~MU_MESAG;
		switch (event->msg[0])
		{
		case WM_REDRAW:
			{
				GRECT box;
				_WORD ret;
				
				wind_update(BEG_UPDATE);
				WindowCalcScroll(win);
				if (win->status & WIS_ICONIFY)
				{
					box = win->work;
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

					toolbar = win->work;
					if (win->toolbar)
					{
						win->toolbar->ob_x = toolbar.g_x;
						win->toolbar->ob_y = toolbar.g_y;
						if (win->y_offset)	/* horizontal toolbar? */
						{
							win->toolbar->ob_width = toolbar.g_w;	/* adjust object width */
							toolbar.g_h = win->y_offset;
						}
						if (win->x_offset)	/* vertical toolbar? */
						{
							win->toolbar->ob_height = toolbar.g_h;	/* adjust object height */
							toolbar.g_w = win->x_offset;
						}

						win->proc(win, WIND_TBUPDATE, &toolbar);
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
							if (win->x_offset)	/* vertical toolbar? */
							{
								box.g_x += temp_tbar.g_w;
								box.g_w -= temp_tbar.g_w;
							}
							if (win->y_offset)	/* horizontal toolbar? */
							{
								box.g_y += temp_tbar.g_h;
								box.g_h -= temp_tbar.g_h;
							}
						}
						if (rc_intersect((GRECT *) & event->msg[4], &temp_tbar))
							objc_draw_grect(win->toolbar, 0, MAX_DEPTH, &temp_tbar);
#endif
						if (rc_intersect((GRECT *) & event->msg[4], &box))
							win->proc(win, WIND_REDRAW, (void *) &box);
						ret = wind_get_grect(event->msg[3], WF_NEXTXYWH, &box);
					}
					graf_mouse(M_ON, NULL);
				}
				wind_update(END_UPDATE);
			}
			break;
			
		case WM_TOPPED:
			if (win->proc(win, WIND_TOPPED, event))
				wind_set_int(event->msg[3], WF_TOP, 0);
			break;
			
		case WM_CLOSED:
			RemoveWindow(win);
			break;

		case WM_FULLED:
			{
				GRECT big;

				if (win->status & WIS_FULL)
				{
					wind_get_grect(event->msg[3], WF_CURRXYWH, &big);

					win->proc(win, WIND_FULLED, &win->last);
					graf_shrinkbox_grect(&win->last, &big);
					wind_set_grect(event->msg[3], WF_CURRXYWH, &win->last);

					win->status &= ~WIS_FULL;
					SetWindowSlider(win);
				} else
				{
					WP_UNIT rel_x, rel_y;

					wind_get_grect(event->msg[3], WF_CURRXYWH, &win->last);

					win->proc(win, WIND_FULLED, &win->full);
					graf_growbox_grect(&win->last, &win->full);
					wind_set_grect(event->msg[3], WF_CURRXYWH, &win->full);

					win->status |= WIS_FULL;

					WindowCalcScroll(win);
					rel_x = ((win->docsize.w - win->docsize.x) - (win->scroll.g_w - DL_WIN_XADD(win)) / win->x_raster);
					rel_y = ((win->docsize.h - win->docsize.y) - (win->scroll.g_h - DL_WIN_YADD(win)) / win->y_raster);
					if ((rel_x < 0) || (rel_y < 0))
					{
						rel_x = min(rel_x, 0);
						rel_y = min(rel_y, 0);
						ScrollWindow(win, rel_x, rel_y);
					} else
					{
						SetWindowSlider(win);
					}
				}
			}
			break;
			
		case WM_ARROWED:
			{
				WP_UNIT rel_x = 0, rel_y = 0;

				WindowCalcScroll(win);

				switch (event->msg[4])
				{
				case WA_UPPAGE:
					rel_y = -win->scroll.g_h / win->y_raster;
					break;
				case WA_DNPAGE:
					rel_y = win->scroll.g_h / win->y_raster;
					break;
				case WA_LFPAGE:
					rel_x = -win->scroll.g_w / win->x_raster;
					break;
				case WA_RTPAGE:
					rel_x = win->scroll.g_w / win->x_raster;
					break;
				case WA_UPLINE:
					rel_y = -win->y_speed;
					break;
				case WA_DNLINE:
					rel_y = win->y_speed;
					break;
				case WA_LFLINE:
					rel_x = -win->x_speed;
					break;
				case WA_RTLINE:
					rel_x = win->x_speed;
					break;
				}
				ScrollWindow(win, rel_x, rel_y);
			}
			break;

		case WM_HSLID:
			{
				_WORD rel_x;

				WindowCalcScroll(win);
				rel_x =
					(short) (((event->msg[4] * (win->docsize.w - win->scroll.g_w) / 1000) - win->docsize.x) / win->x_raster);
				ScrollWindow(win, rel_x, 0);
			}
			break;

		case WM_VSLID:
			{
				_WORD rel_y;

				WindowCalcScroll(win);
				rel_y =
					(short) (((event->msg[4] * (win->docsize.h - win->scroll.g_h) / 1000) - win->docsize.y) / win->y_raster);
				ScrollWindow(win, 0, rel_y);
			}
			break;

		case WM_SIZED:
			{
				GRECT area;
				GRECT old_scroll;
				
				WindowCalcScroll(win);
				old_scroll = win->scroll;
				win->proc(win, WIND_SIZED, &event->msg[4]);
				wind_set_grect(event->msg[3], WF_CURRXYWH, (GRECT *) &event->msg[4]);
				win->status &= ~WIS_FULL;

				WindowCalcScroll(win);
				SetWindowSlider(win);
				/*
				 * the window manager might optimize away redraw events
				 * if the window was made smaller, or send us only newly visible
				 * areas. But we may need to clear the margins at the right/left,
				 * and text might become visible that was previously cleared out
				 * by the margins.
				 * Send redraws for the affected areas.
				 */
#if 0
				/* left */
				area.g_x = win->scroll.g_x - win->x_margin_left;
				area.g_y = win->scroll.g_y - win->y_margin_top;
				area.g_w = win->x_margin_left * 2;
				area.g_h = win->scroll.g_h + win->y_margin_top + win->y_margin_bottom;
				SendRedrawArea(win, &area);
#endif
				/* right */
				area.g_x = win->scroll.g_x + win->scroll.g_w;
				area.g_y = win->scroll.g_y - win->y_margin_top;
				area.g_w = win->x_margin_right;
				area.g_h = win->scroll.g_h + win->y_margin_top + win->y_margin_bottom;
				SendRedrawArea(win, &area);
				if (win->scroll.g_w > old_scroll.g_w)
				{
					area.g_x = old_scroll.g_x + old_scroll.g_w - win->x_margin_right;
					area.g_y = old_scroll.g_y - win->y_margin_top;
					SendRedrawArea(win, &area);
				}
#if 0
				/* top */
				area.g_x = win->scroll.g_x - win->x_margin_left;
				area.g_y = win->scroll.g_y - win->y_margin_top;
				area.g_w = win->scroll.g_w + win->x_margin_left + win->x_margin_right;
				area.g_h = win->y_margin_top * 2;
				SendRedrawArea(win, &area);
#endif
				/* bottom */
				area.g_x = win->scroll.g_x - win->x_margin_left;
				area.g_y = win->scroll.g_y + win->scroll.g_h;
				area.g_w = win->scroll.g_w + win->x_margin_left + win->x_margin_right;
				area.g_h = win->y_margin_bottom;
				SendRedrawArea(win, &area);
				if (win->scroll.g_h > old_scroll.g_h)
				{
					area.g_x = old_scroll.g_x - win->x_margin_left;
					area.g_y = old_scroll.g_y + old_scroll.g_h - win->y_margin_bottom;
					SendRedrawArea(win, &area);
				}
			}
			break;

		case WM_MOVED:
			win->proc(win, WIND_MOVED, &event->msg[4]);
			wind_set_grect(event->msg[3], WF_CURRXYWH, (GRECT *) & event->msg[4]);
			win->status &= ~WIS_FULL;
			SetWindowSlider(win);
			break;

		case WM_NEWTOP:
			win->proc(win, WIND_NEWTOP, NULL);
			break;

		case WM_UNTOPPED:
			win->proc(win, WIND_UNTOPPED, NULL);
			break;

		case WM_ONTOP:
			win->proc(win, WIND_ONTOP, NULL);
			break;

		case WM_BOTTOMED:
			if (win->proc(win, WIND_BOTTOM, event))
				wind_set_int(event->msg[3], WF_BOTTOM, 0);
			break;

		case WM_ICONIFY:
			IconifyWindow(win, (GRECT *) & event->msg[4]);
			win->proc(win, WIND_ICONIFY, (void *) &event->msg[4]);
			break;

		case WM_UNICONIFY:
			UniconifyWindow(win);
			win->proc(win, WIND_UNICONIFY, (void *) &event->msg[4]);
			break;

		case WM_ALLICONIFY:
			AllIconify(event->msg[3], (GRECT *) & event->msg[4]);
			win->proc(win, WIND_ALLICONIFY, (void *) &event->msg[4]);
			break;

		default:
			event->mwhich |= MU_MESAG;
			break;
		}
	}
}

void SetWindowSlider(WINDOW_DATA *win)
{
	GRECT winsize;
	long temp;

	if (win->status & WIS_ICONIFY)
		return;

	wind_get_grect(win->whandle, WF_WORKXYWH, &winsize);

	temp = win->docsize.w - (winsize.g_w - DL_WIN_XADD(win));
	if (temp > 0)
	{
		wind_set_int(win->whandle, WF_HSLIDE, (short) (win->docsize.x * 1000L / temp));
		wind_set_int(win->whandle, WF_HSLSIZE, (short) ((winsize.g_w - DL_WIN_XADD(win)) * 1000L / win->docsize.w));
	} else
	{
		wind_set_int(win->whandle, WF_HSLIDE, 1000);
		wind_set_int(win->whandle, WF_HSLSIZE, 1000);
	}
	temp = win->docsize.h - (winsize.g_h - DL_WIN_YADD(win));
	if (temp > 0)
	{
		wind_set_int(win->whandle, WF_VSLIDE, (short) (win->docsize.y * 1000L / temp));
		wind_set_int(win->whandle, WF_VSLSIZE, (short) ((winsize.g_h - DL_WIN_YADD(win)) * 1000L / win->docsize.h));
	} else
	{
		wind_set_int(win->whandle, WF_VSLIDE, 1000);
		wind_set_int(win->whandle, WF_VSLSIZE, 1000);
	}
}


void ResizeWindow(WINDOW_DATA *win, WP_UNIT max_w, WP_UNIT max_h)
{
	GRECT wind;

	wind_get_grect(DESK, WF_WORKXYWH, &wind);
	wind_calc_grect(WC_WORK, win->kind, &wind, &wind);
	wind.g_w = (_WORD)min(DL_WIN_XADD(win) + max_w, wind.g_w);
	wind.g_h = (_WORD)min(DL_WIN_YADD(win) + max_h, wind.g_h);
	win->docsize.w = max_w;
	win->docsize.h = max_h;

	wind_calc_grect(WC_BORDER, win->kind, &wind, &win->full);
}


void IconifyWindow(WINDOW_DATA *win, GRECT *r)
{
	GRECT current_size;
	GRECT new_pos;

	if (win->status & WIS_ICONIFY)
		return;

	wind_get_grect(win->whandle, WF_CURRXYWH, &current_size);
	wind_close(win->whandle);
	wind_set_grect(win->whandle, WF_ICONIFY, r);
	wind_set_grect(win->whandle, WF_UNICONIFYXYWH, &current_size);
	wind_get_grect(win->whandle, WF_CURRXYWH, &new_pos);
	if (new_pos.g_x != -1)
		graf_shrinkbox_grect(&new_pos, &current_size);

	wind_get_grect(win->whandle, WF_WORKXYWH, &new_pos);
	dial_library_tree[DI_ICON].ob_x = (new_pos.g_w - dial_library_tree[DI_ICON].ob_width) >> 1;
	dial_library_tree[DI_ICON].ob_y = (new_pos.g_h - dial_library_tree[DI_ICON].ob_height) >> 1;

	wind_open_grect(win->whandle, r);
	win->status |= WIS_ICONIFY;
	CycleItems();
}


void UniconifyWindow(WINDOW_DATA *win)
{
	GRECT small;
	GRECT current_size;

	if (!(win->status & WIS_ICONIFY))
		return;

	wind_get_grect(win->whandle, WF_CURRXYWH, &small);

	if ((CHAIN_DATA *) win == iconified_list[0])
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
		win->status &= ~WIS_ALLICONIFY;
	}

	wind_get_grect(win->whandle, WF_UNICONIFY, &current_size);
	graf_growbox_grect(&small, &current_size);
	wind_set_int(win->whandle, WF_TOP, 0);
	wind_set_grect(win->whandle, WF_UNICONIFY, &current_size);
	win->status &= ~WIS_ICONIFY;
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
	WINDOW_DATA *win = (WINDOW_DATA *) all_list;

	while (win)
	{
		if ((win->type == WIN_WINDOW) && (win->whandle == handle) && (win->status & WIS_OPEN))
			return win;
		win = win->next;
	}
	return NULL;
}

WINDOW_DATA *find_window_by_whandle(short handle)
{
	WINDOW_DATA *win = (WINDOW_DATA *) all_list;

	while (win)
	{
		if ((win->type == WIN_WINDOW) && (win->whandle == handle))
			return win;
		win = win->next;
	}
	return NULL;
}

WINDOW_DATA *find_window_by_proc(HNDL_WIN proc)
{
	WINDOW_DATA *win = (WINDOW_DATA *) all_list;

	while (win)
	{
		if ((win->type == WIN_WINDOW) && win->owner == gl_apid && (win->proc == proc))
			return win;
		win = win->next;
	}
	return NULL;
}

WINDOW_DATA *find_window_by_data(void *data)
{
	WINDOW_DATA *win = (WINDOW_DATA *) all_list;

	while (win)
	{
		if ((win->type == WIN_WINDOW) && (win->data == data))
			return win;
		win = win->next;
	}
	return NULL;
}

int count_window(void)
{
	int i;
	WINDOW_DATA *win = (WINDOW_DATA *) all_list;

	i = 0;
	while (win)
	{
		if (win->type == WIN_WINDOW && win->owner == gl_apid)
			i++;
		win = win->next;
	}
	return i;
}
