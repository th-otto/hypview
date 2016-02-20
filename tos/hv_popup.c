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
#include "hypdebug.h"


typedef struct
{
	DOCUMENT doc;
	short x, y;
	void *entry;
	WINDOW_DATA *parentwin;
} POPUP_INFO;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

gboolean PopupWindow(WINDOW_DATA *win, _WORD obj, void *data)
{
	POPUP_INFO *popup = (POPUP_INFO *)win->data;

	switch (obj)
	{
	case WIND_INIT:
		/* use font size for window raster */
		win->x_raster = font_cw;
		win->y_raster = font_ch;

		win->y_margin_top = gl_profile.viewer.text_yoffset;
		win->y_margin_bottom = gl_profile.viewer.text_yoffset;
		win->x_margin_left = gl_profile.viewer.text_xoffset;
		win->x_margin_right = gl_profile.viewer.text_xoffset;
		
		break;
	case WIND_EXIT:
		popup->parentwin->popup = NULL;
		g_free(popup);
		g_free(win->autolocator);
		win->autolocator = NULL;
		break;
	case WIND_OPEN:
		break;
	case WIND_OPENSIZE:	/* initial opening size */
		{
			GRECT *osize = (GRECT *) data;
			GRECT screen;
	
			wind_get_grect(0, WF_WORKXYWH, &screen);
	
			if (popup->x + osize->g_w > screen.g_x + screen.g_w)
				popup->x = screen.g_x + screen.g_w - osize->g_w;
			osize->g_x = max(screen.g_x, popup->x);
			if (popup->y + osize->g_h > screen.g_y + screen.g_h)
				popup->y = screen.g_y + screen.g_h - osize->g_h;
			osize->g_y = max(screen.g_y, popup->y);
			osize->g_w = screen.g_w;
			osize->g_h = screen.g_h;
		}
		break;
	case WIND_CLOSE:
		break;
	case WIND_REDRAW:
		{
			_WORD pxy[4];
			GRECT *box = (GRECT *) data;
	
			pxy[0] = box->g_x;
			pxy[1] = box->g_y;
			pxy[2] = box->g_x + box->g_w - 1;
			pxy[3] = box->g_y + box->g_h - 1;
			vs_clip(vdi_handle, TRUE, pxy);	/* clipping ON */
	
			vsf_color(vdi_handle, viewer_colors.background);
			vsf_interior(vdi_handle, FIS_SOLID);
			vswr_mode(vdi_handle, MD_REPLACE);
			vr_recfl(vdi_handle, pxy);		/* clear background */
	
			HypDisplayPage(win);
	
			vs_clip(vdi_handle, FALSE, pxy);	/* clipping OFF */
		}
		break;
	case WIND_KEYPRESS:
		{
			EVNT *event = (EVNT *) data;
	
			SendCloseWindow(win);
			event->mwhich &= ~MU_KEYBD;
		}
		break;
	case WIND_CLICK:
		{
			EVNT *event = (EVNT *) data;
	
			if (event->mbutton & 1)
			{
				GRECT work;
	
				do
				{
					graf_mkstate(&event->mx, &event->my, &event->mbutton, &event->kstate);
				} while (event->mbutton & 1);
	
				wind_get_grect(win->whandle, WF_WORKXYWH, &work);
				/* GetLink(file,event->mx-work.g_x, event->my - work.g_y-ptr->y_offset); */
				SendCloseWindow(win);
			}
		}
		break;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void OpenPopup(WINDOW_DATA *win, hyp_nodenr num, short x, short y)
{
	DOCUMENT *doc = win->data;
	POPUP_INFO *popup;

	graf_mouse(BUSY_BEE, NULL);

	popup = g_new(POPUP_INFO, 1);

	if (popup != NULL)
	{
		GRECT work;
		char *old_wtitle = win->title;
		gboolean ret;
		
		popup->doc = *doc;
		popup->parentwin = win;
		win->popup = hv_win_new(&popup->doc, TRUE);
		if (win->popup == NULL)
		{
			g_free(popup);
		} else
		{
			ret = doc->gotoNodeProc(win->popup, NULL, num);
			if (ret)
			{
				wind_get_grect(win->whandle, WF_WORKXYWH, &work);
		
				popup->x = x + win->scroll.g_x;
				popup->y = y + win->scroll.g_y;
				popup->entry = win->displayed_node;
				
				if (old_wtitle != win->title)
				{
					g_free(win->title);
					win->title = old_wtitle;
				}
				
				ReInitWindow(win->popup, FALSE);
				hv_win_open(win->popup);
			} else
			{
				RemoveWindow(win->popup);
			}
		}
	}
	
	graf_mouse(ARROW, NULL);
}
