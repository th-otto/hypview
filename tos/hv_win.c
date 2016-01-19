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


#define setmsg(a,b,c,d,e,f,g,h) \
	msg[0] = a; \
	msg[1] = b; \
	msg[2] = c; \
	msg[3] = d; \
	msg[4] = e; \
	msg[5] = f; \
	msg[6] = g; \
	msg[7] = h

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void SendClose(_WORD whandle)
{
	_WORD msg[8];
	setmsg(WM_CLOSED, gl_apid, 0, whandle, 0, 0, 0, 0);
	appl_write(gl_apid, 16, msg);
}

/*** ---------------------------------------------------------------------- ***/

void SendCloseWindow(WINDOW_DATA *win)
{
	SendClose(win->whandle);
}

/*** ---------------------------------------------------------------------- ***/

void SendTopped(_WORD whandle)
{
	_WORD msg[8];
	setmsg(WM_TOPPED, gl_apid, 0, whandle, 0, 0, 0, 0);
	appl_write(gl_apid, 16, msg);
}

/*** ---------------------------------------------------------------------- ***/

void SendRedraw(WINDOW_DATA *win)
{
	_WORD msg[8];
	msg[0] = WM_REDRAW;
	msg[1] = gl_apid;
	msg[2] = 0;
	msg[3] = win->whandle;
	wind_get_grect(win->whandle, WF_WORKXYWH, (GRECT *)&msg[4]);
	appl_write(gl_apid, 16, msg);
}

/*** ---------------------------------------------------------------------- ***/

void hv_set_title(WINDOW_DATA *win, const char *wintitle)
{
	char *title;

	title = hyp_utf8_to_charset(hyp_get_current_charset(), wintitle, STR0TERM, NULL);
	strncpy(win->titlebuf, title ? title : "", sizeof(win->titlebuf));
	win->titlebuf[sizeof(win->titlebuf) - 1] = '\0';
	g_free(title);
	wind_set_str(win->whandle, WF_NAME, win->titlebuf);
}

/*** ---------------------------------------------------------------------- ***/

static void calc_opensize(WINDOW_DATA *win, GRECT *curr)
{
	GRECT screen;
	_WORD minw, minh;

	minw = win->x_offset + font_cw + win->x_margin_left + win->x_margin_right;
	minh = win->y_offset + font_ch + win->y_margin_top + win->y_margin_bottom;

	wind_get_grect(DESK, WF_WORKXYWH, &screen);

	if (gl_profile.viewer.adjust_winsize)
	{
		wind_calc_grect(WC_WORK, win->kind, curr, curr);
		if (curr->g_w < toolbar_tree[TO_SEARCHBOX].ob_width)
			curr->g_w = toolbar_tree[TO_SEARCHBOX].ob_width;

		if (curr->g_h < minh)
			curr->g_h = minh;
		if (curr->g_w < minw)
			curr->g_w = minw;
		
		curr->g_w = ((curr->g_w - minw + win->x_raster - 1) / win->x_raster) * win->x_raster + minw;
		curr->g_h = ((curr->g_h - minh + win->y_raster - 1) / win->y_raster) * win->y_raster + minh;
		
		wind_calc_grect(WC_BORDER, win->kind, curr, curr);

		if (curr->g_x + curr->g_w > screen.g_x + screen.g_w)
		{
			_WORD val = curr->g_x - screen.g_x + curr->g_w - screen.g_w;

			curr->g_w -= (val / win->x_raster) * win->x_raster;
		}

		if (curr->g_y + curr->g_h > screen.g_y + screen.g_h)
		{
			_WORD val = curr->g_y - screen.g_y + curr->g_h - screen.g_h;

			curr->g_h -= (val / win->y_raster) * win->y_raster;
		}
	} else
	{
		wind_calc_grect(WC_WORK, win->kind, curr, curr);

		curr->g_w = ((curr->g_w - minw + win->x_raster - 1) / win->x_raster) * win->x_raster + minw;
		curr->g_h = ((curr->g_h - minh + win->y_raster - 1) / win->y_raster) * win->y_raster + minh;
		wind_calc_grect(WC_BORDER, win->kind, curr, curr);
	}
}

/*** ---------------------------------------------------------------------- ***/

void WindowCalcScroll(WINDOW_DATA *win)
{
	wind_get_grect(win->whandle, WF_WORKXYWH, &win->work);
	win->scroll.g_x = win->work.g_x + win->x_offset + win->x_margin_left;
	win->scroll.g_y = win->work.g_y + win->y_offset + win->y_margin_top;
	win->scroll.g_w = win->work.g_w - win->x_offset - win->x_margin_left - win->x_margin_right;
	win->scroll.g_h = win->work.g_h - win->y_offset - win->y_margin_top - win->y_margin_top;
}

/*** ---------------------------------------------------------------------- ***/

void ReInitWindow(DOCUMENT *doc)
{
	WINDOW_DATA *win = doc->window;
	_WORD visible_lines;
	GRECT curr;
	
	win->data = doc;
	win->title = doc->window_title;
	win->x_raster = font_cw;
	win->y_raster = font_ch;
	hv_set_title(win, win->title);
	doc->selection.valid = FALSE;

	/* window size: at least 5 columns and 1 line */
	ResizeWindow(win, max(doc->columns, 5), max(doc->lines, 1));

	wind_get_grect(win->whandle, WF_CURRXYWH, &curr);

	/* adjust window size to new dimensions */
	if (gl_profile.viewer.adjust_winsize)
	{
		GRECT screen;

		wind_get_grect(DESK, WF_WORKXYWH, &screen);

		curr.g_h = screen.g_h;
	}
	calc_opensize(win, &curr);
	wind_set_grect(win->whandle, WF_CURRXYWH, &curr);

	WindowCalcScroll(win);
	visible_lines = (win->scroll.g_h + win->y_raster - 1) / win->y_raster;

	win->docsize.y = min(doc->lines - visible_lines, doc->start_line);
	win->docsize.y = max(0, win->docsize.y);
	win->docsize.x = 0;

	if (!gl_profile.viewer.intelligent_fuller)
		wind_get_grect(0, WF_WORKXYWH, &win->full);

	SetWindowSlider(win);
	ToolbarUpdate(doc, FALSE);
	SendRedraw(win);
}

/*** ---------------------------------------------------------------------- ***/

gboolean HelpWindow(WINDOW_DATA *win, _WORD obj, void *data)
{
	DOCUMENT *doc = (DOCUMENT *) win->data;

	switch (obj)
	{
	case WIND_INIT:
		/* use font size for raster */
		win->x_raster = font_cw;
		win->y_raster = font_ch;

		win->y_margin_top = gl_profile.viewer.text_offset;
		win->x_margin_left = font_cw;
		
		/* install toolbar */
		win->toolbar = toolbar_tree;
		win->x_offset = 0;
		win->y_offset = toolbar_tree[0].ob_height + 2;

		win->title = doc->window_title;

		/* window size: at least 5 columns and 1 line */
		win->docsize.w = max(doc->columns, 5);
		win->docsize.h = max(doc->lines, 1);

		win->docsize.x = 0;
		win->docsize.y = doc->start_line;

		DhstAddFile(doc->path);
		break;
		
	case WIND_EXIT:
		if (doc->popup)					/* if a popup is still open... */
			RemoveWindow(doc->popup);	/* ... close it */
		HypCloseFile(doc);
		RemoveAllHistoryEntries(win);
		win = NULL;
		break;
		
	case WIND_OPEN:
		graf_mouse(ARROW, NULL);

		SendAV_ACCWINDOPEN(win->whandle);

		if (!gl_profile.viewer.intelligent_fuller)
			wind_get_grect(0, WF_WORKXYWH, &win->full);
		break;
		
	case WIND_OPENSIZE:	/* initial window size */
		{
			GRECT *open_size = (GRECT *) data;
			GRECT screen;
	
			wind_get_grect(DESK, WF_WORKXYWH, &screen);
	
			if (!gl_profile.viewer.adjust_winsize)
				*open_size = screen;
	
			/* default X-coordinate specified? */
			if (gl_profile.viewer.win_x && (gl_profile.viewer.win_x <= screen.g_x + screen.g_w - 50))
				open_size->g_x = gl_profile.viewer.win_x;
	
			/* default Y-Koordinate specified? */
			if (gl_profile.viewer.win_y && (gl_profile.viewer.win_y <= screen.g_y + screen.g_h - 70))
				open_size->g_y = gl_profile.viewer.win_y;

			if (gl_profile.viewer.win_w != 0 && (gl_profile.viewer.win_w < open_size->g_w))
				open_size->g_w = gl_profile.viewer.win_w;
			
			if (gl_profile.viewer.win_h != 0 && (gl_profile.viewer.win_h < open_size->g_h))
				open_size->g_h = gl_profile.viewer.win_h;
			
			/* window width or height specified? */
			if (!gl_profile.viewer.adjust_winsize)
			{
				if (gl_profile.viewer.win_w == 0)
				{
					_WORD maxw = HYP_STGUIDE_DEFAULT_LINEWIDTH * font_cw + win->x_margin_left + win->x_margin_right;
					wind_calc_grect(WC_WORK, win->kind, open_size, open_size);
					if (open_size->g_w > maxw)
						open_size->g_w = maxw;
					wind_calc_grect(WC_BORDER, win->kind, open_size, open_size);
				}
			}
			calc_opensize(win, open_size);
		}
		return FALSE;
		
	case WIND_CLOSE:
		SendAV_ACCWINDCLOSED(win->whandle);
		break;
		
	case WIND_REDRAW:
		{
			_WORD pxy[4];
			GRECT *box = (GRECT *) data;
	
			pxy[0] = box->g_x;
			pxy[1] = box->g_y;
			pxy[2] = box->g_x + box->g_w - 1;
			pxy[3] = box->g_y + box->g_h - 1;
	
			vsf_color(vdi_handle, gl_profile.viewer.background_color);
			vsf_interior(vdi_handle, FIS_SOLID);
			vswr_mode(vdi_handle, MD_REPLACE);
	
			vs_clip(vdi_handle, TRUE, pxy);	/* clipping ON */
			vr_recfl(vdi_handle, pxy);		/* clear beackground */
	
			doc->displayProc(doc);
			DrawSelection(doc);
	
			vs_clip(vdi_handle, FALSE, pxy);	/* clipping OFF */
		}
		break;
		
	case WIND_SIZED:		/* window size has changed */
		{
			GRECT *out = (GRECT *) data;
			GRECT in;
			
			wind_calc_grect(WC_WORK, win->kind, out, &in);	/* calculate working area */
			
			in.g_w -= (in.g_w - win->x_offset - win->x_margin_left - win->x_margin_right) % win->x_raster;	/* align width to raster */
			in.g_h -= (in.g_h - win->y_offset - win->y_margin_top - win->y_margin_bottom) % win->y_raster;	/* align height to raster */
			
			wind_calc_grect(WC_BORDER, win->kind, &in, out);	/* calculate window frame */
		}
		break;
		
	case WIND_FULLED:		/* fuller activated */
		{
			GRECT *out = (GRECT *) data;
			GRECT in;
			GRECT screen;
			_WORD minw, minh;
			
			wind_get_grect(DESK, WF_WORKXYWH, &screen);
	
			wind_calc_grect(WC_WORK, win->kind, out, &in);	/* calculate working area */
	
			/* if window was enlarged, prevent toolbar from disappearing */
			if (((win->status & WIS_FULL) == 0) && in.g_w < toolbar_tree[TO_SEARCHBOX].ob_width)
			{
				in.g_w = toolbar_tree[TO_SEARCHBOX].ob_width;
			}
			
			minw = win->x_offset + win->x_margin_left + win->x_margin_right;
			minh = win->y_offset + win->y_margin_top + win->y_margin_bottom;
			in.g_w -= (in.g_w - minw) % win->x_raster;	/* align window width */
			in.g_h -= (in.g_h - minh) % win->y_raster;	/* align window height */
	
			wind_calc_grect(WC_BORDER, win->kind, &in, out);	/* calculate window frame */
	
			if (gl_profile.viewer.intelligent_fuller && ((win->status & WIS_FULL) == 0))
			{
				out->g_x = win->last.g_x;
				out->g_y = win->last.g_y;
				if (out->g_x + out->g_w > screen.g_x + screen.g_w)
					out->g_x -= out->g_x + out->g_w - (screen.g_x + screen.g_w);
				else if (out->g_x < screen.g_x)
					out->g_x = screen.g_x;
				if (out->g_y + out->g_h > screen.g_y + screen.g_h)
					out->g_y -= out->g_y + out->g_h - (screen.g_y + screen.g_h);
				else if (out->g_y < screen.g_y)
					out->g_y = screen.g_y;
			}
		}
		break;
		
	case WIND_KEYPRESS:
		{
			EVNT *event = (EVNT *) data;
			_WORD scan = (event->key >> 8) & 0xff;
			_WORD ascii = event->key & 0xff;
	
			WindowCalcScroll(win);
	
			event->mwhich &= ~MU_KEYBD;
	
			if ((event->kstate & KbSHIFT) && (event->kstate & KbCTRL))
			{
				if (scan == KbUP || scan == KbLEFT)
					ToolbarClick(doc, TO_PREV);
				else if (scan == KbDOWN || scan == KbRIGHT)
					ToolbarClick(doc, TO_NEXT);
				else if (ascii == 'V')
					BlockPaste(win, !gl_profile.viewer.clipbrd_new_window);
				else
					event->mwhich |= MU_KEYBD;
			} else if (event->kstate & KbSHIFT)
			{
				_WORD val;
				
				switch (scan)
				{
				case KbLEFT:
					val = -win->scroll.g_w / win->x_raster;
					ScrollWindow(win, &val, NULL);
					break;
				case KbRIGHT:
					val = win->scroll.g_w / win->x_raster;
					ScrollWindow(win, &val, NULL);
					break;
				case KbUP:
					val = -win->scroll.g_h / win->y_raster;
					ScrollWindow(win, NULL, &val);
					break;
				case KbDOWN:
					val = win->scroll.g_h / win->y_raster;
					ScrollWindow(win, NULL, &val);
					break;
				case KbHOME:
				case KbEND:
					win->docsize.x = 0;
					win->docsize.y = win->docsize.h - win->scroll.g_h / win->y_raster;
					SetWindowSlider(win);
					SendRedraw(win);
					break;
				case KbPAGEUP:
					val = -(win->scroll.g_h) / win->y_raster;
					ScrollWindow(win, NULL, &val);
					break;
				case KbPAGEDOWN:
					val = (win->scroll.g_h) / win->y_raster;
					ScrollWindow(win, NULL, &val);
					break;
				case KbF11:
				case KbF12:
				case KbF13:
				case KbF14:
				case KbF15:
				case KbF16:
				case KbF17:
				case KbF18:
				case KbF19:
				case KbF20:
					MarkerSave(doc, scan - KbF11);
					break;
				default:
					event->mwhich |= MU_KEYBD;
					break;
				}
			} else if (event->kstate & KbALT)
			{
				switch (ascii)
				{
				case 'D':
					{
						char *filename = path_subst(gl_profile.viewer.default_file);
						OpenFileSameWindow(win, filename, NULL, FALSE, FALSE);
						g_free(filename);
					}
					break;
				case 'E':
					RemoveAllHistoryEntries(win);
					ToolbarUpdate(doc, TRUE);
					break;
				case 'K':
					{
						char *filename = path_subst(gl_profile.viewer.catalog_file);
						OpenFileSameWindow(win, filename, NULL, FALSE, FALSE);
						g_free(filename);
					}
					break;
				case 'R':
					BlockOperation(doc, CO_REMARKER);
					break;
				case 'T':
					GoThisButton(doc, TO_HOME);
					break;
				case 'X':
					GotoIndex(doc);
					break;
				case 'Z':
					BlockOperation(doc, CO_SELECT_FONT);
					break;
				}	
			} else if (event->kstate & KbCTRL)
			{
				if (ascii == 'A')
					BlockOperation(doc, CO_SELECT_ALL);
				else if (ascii == 'C')
					BlockOperation(doc, CO_COPY);
				else if (ascii == 'I')
					ProgrammInfos(doc);
				else if (ascii == 'F')
					BlockOperation(doc, CO_SEARCH);
				else if (ascii == 'O')
					ToolbarClick(doc, TO_LOAD);
				else if (ascii == 'V')
				{
					if (doc->buttons.searchbox)
						AutoLocatorPaste(doc);
					else
						BlockOperation(doc, CO_PASTE);
				} else if (ascii == 'Z')
				{
					BlockOperation(doc, CO_SWITCH_FONT);
				} else if (scan == KbUP)
				{
					_WORD val = -win->scroll.g_h / win->y_raster;
	
					ScrollWindow(win, NULL, &val);
				} else if (scan == KbDOWN)
				{
					_WORD val = win->scroll.g_h / win->y_raster;
	
					ScrollWindow(win, NULL, &val);
				} else if (scan == KbLEFT)
					ToolbarClick(doc, TO_PREV);
				else if (scan == KbRIGHT)
					ToolbarClick(doc, TO_NEXT);
				else if (scan >= KbF1 && scan <= KbF10)
					MarkerShow(doc, scan - KbF1, TRUE);
				else
					event->mwhich |= MU_KEYBD;
			} else if (event->kstate & KbNUM)
			{
				if (ascii == '-')
					ToolbarClick(doc, TO_PREV);
				else if (ascii == '+')
					ToolbarClick(doc, TO_NEXT);
				else
					event->mwhich |= MU_KEYBD;
			} else if (event->kstate == 0)
			{
				_WORD val;
				
				switch (scan)
				{
				case KbLEFT:
					val = -win->x_speed;
					ScrollWindow(win, &val, NULL);
					break;
				case KbRIGHT:
					val = win->x_speed;
					ScrollWindow(win, &val, NULL);
					break;
				case KbUP:
					val = -win->y_speed;
					ScrollWindow(win, NULL, &val);
					break;
				case KbDOWN:
					val = win->y_speed;
					ScrollWindow(win, NULL, &val);
					break;
				case KbPAGEUP:
					val = -(win->scroll.g_h) / win->y_raster;
					ScrollWindow(win, NULL, &val);
					break;
				case KbPAGEDOWN:
					val = (win->scroll.g_h) / win->y_raster;
					ScrollWindow(win, NULL, &val);
					break;
				case KbHOME:
					if (win->docsize.y)
					{
						win->docsize.x = 0;
						win->docsize.y = 0;
						SetWindowSlider(win);
						SendRedraw(win);
					}
					break;
				case KbEND:
					win->docsize.x = 0;
					win->docsize.y = win->docsize.h - (win->scroll.g_h) / win->y_raster;
					SetWindowSlider(win);
					SendRedraw(win);
					break;
				case KbHELP:
					ToolbarClick(doc, TO_HELP);
					break;
				case KbUNDO:
					ToolbarClick(doc, TO_BACK);
					break;
				case KbF1:
				case KbF2:
				case KbF3:
				case KbF4:
				case KbF5:
				case KbF6:
				case KbF7:
				case KbF8:
				case KbF9:
				case KbF10:
					MarkerShow(doc, scan - KbF1, FALSE);
					break;
				default:
					if ((ascii == 27) && !(doc->buttons.searchbox))
						ToolbarClick(doc, TO_BACK);
					else
						event->mwhich |= MU_KEYBD;
					break;
				}
			} else
			{
				event->mwhich |= MU_KEYBD;
			}
	
			if (event->mwhich & MU_KEYBD)
			{
				if (AutolocatorKey(doc, event->kstate, ascii))
					event->mwhich &= ~MU_KEYBD;
			}
		}
		break;
		
	case WIND_DRAGDROPFORMAT:
		{
			_WORD i;
			long *format = data;
	
			for (i = 0; i < MAX_DDFORMAT; i++)
				format[i] = 0L;
	
			format[0] = 0x41524753L;		/* 'ARGS' */
		}
		break;
		
	case WIND_DRAGDROP:
		{
			char *ptr = hyp_conv_to_utf8(hyp_get_current_charset(), data, STR0TERM);
			char *chapter;
			
			chapter = ParseData(ptr);
			if (chapter != ptr && *chapter == '\'')
				ParseData(chapter);
			OpenFileSameWindow(win, ptr, chapter, FALSE, FALSE);
			g_free(ptr);
		}
		break;
		
	case WIND_CLICK:
		{
			EVNT *event = (EVNT *) data;
	
			if (doc->popup)					/* Popup active? */
				SendCloseWindow(doc->popup);
	
			RemoveSearchBox(doc);
	
			if (event->mbutton & 1)			/* left button */
			{
				graf_mkstate(&event->mx, &event->my, &event->mbutton, &event->kstate);
	
				if (gl_profile.viewer.check_time)
					CheckFiledate(doc);
	
				if ((event->mbutton & 1) ||		/* button still pressed? */
					(event->kstate & KbSHIFT))	/* or shift pressee? */
				{
					EVNTDATA d;
					d.x = event->mx;
					d.y = event->my;
					d.bstate = event->mbutton;
					d.kstate = event->kstate;
					MouseSelection(doc, &d);
				} else
				{
					EVNTDATA d;
					d.x = event->mx;
					d.y = event->my;
					d.bstate = event->mbutton;
					d.kstate = event->kstate;
					RemoveSelection(doc);
					doc->clickProc(doc, &d);
				}
			} else if (event->mbutton & 2)	/* right button */
			{
				_WORD num;
				OBJECT *tree = rs_tree(CONTEXT);
				
				num = popup_select(tree, event->mx, event->my);
				BlockOperation(doc, num);
			}
		}
		break;
		
	case WIND_TBUPDATE:
		ToolbarUpdate(doc, FALSE);
		break;
		
	case WIND_TBCLICK:
		ToolbarClick(doc, *(_WORD *) data);
		break;
		
	case WIND_ICONIFY:
		hv_set_title(win, hyp_basename(doc->path));
		break;
		
	case WIND_UNICONIFY:
		hv_set_title(win, win->title);
		break;
		
	case WIND_TOPPED:
		if (doc->popup != NULL)					/* popup active? */
		{
			_WORD top;
			WINDOW_DATA *popup = doc->popup;
			
			wind_get_int(DESK, WF_TOP, &top);

			if (top != popup->whandle)
			{
				wind_set_int(win->whandle, WF_TOP, 0);
				wind_set_int(popup->whandle, WF_TOP, 0);
			} else
			{
				SendCloseWindow(popup);
				wind_set_int(win->whandle, WF_TOP, 0);
			}
		} else
		{
			wind_set_int(win->whandle, WF_TOP, 0);
		}
		if (gl_profile.viewer.check_time)
			CheckFiledate(doc);
		return FALSE;
	}
	return TRUE;
}
