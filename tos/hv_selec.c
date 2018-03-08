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


void SelectAll(DOCUMENT *doc)
{
	WINDOW_DATA *win = doc->window;

	BlockSelectAll(doc, &doc->selection);
	SendRedraw(win);
}

void MouseSelection(DOCUMENT *doc, EVNTDATA *m_data)
{
	WINDOW_DATA *win = doc->window;
	GRECT work;
	_WORD clip_rect[4];
	short x, y;
	short ox, oy;
	TEXT_POS start, end, new;
	_WORD scroll_x = 0, scroll_y = 0;

	wind_update(BEG_UPDATE);
	wind_update(BEG_MCTRL);
	wind_get_grect(win->whandle, WF_WORKXYWH, &work);

	/* VDI Fuell-Attribute fr XOR-Flaechen */
	vsf_interior(vdi_handle, FIS_SOLID);
	vsf_color(vdi_handle, G_BLACK);
	vsf_perimeter(vdi_handle, FALSE);
	vswr_mode(vdi_handle, MD_XOR);

	/* Clipping Rechteck festlegen */
	clip_rect[0] = work.g_x;
	clip_rect[1] = work.g_y + win->y_offset;
	clip_rect[2] = work.g_x + work.g_w - 1;
	clip_rect[3] = work.g_y + work.g_h - 1;
	vs_clip(vdi_handle, TRUE, clip_rect);	/* clipping ON */

	/* Wird SHIFT gedrueckt? */
	if ((m_data->kstate & KbSHIFT) && doc->selection.valid)
	{
		start = doc->selection.start;
		end = doc->selection.end;
#if 1
		if (end.y < (win->docsize.y * win->y_raster))
			oy = -win->y_raster;
		else if (end.y > (win->docsize.y * win->y_raster) + (work.g_h - win->y_offset))
			oy = work.g_h + win->y_raster;
		else
			oy = end.y - (win->docsize.y * win->y_raster);
		ox = end.x - (short)(win->docsize.x * font_cw);
#else
		if (end.line < win->docsize.y)
			oy = -font_ch;
		else if (end.line > win->docsize.y + (work.g_h - win->y_offset) / font_ch)
			oy = work.g_h + font_ch;
		else
			oy = (short)(end.line - win->docsize.y) * font_ch;

		ox = end.x - (short)(win->docsize.x * font_cw);
#endif
		goto shift_entry;
	} else
	{
		/* Falls noch eine Selektion aktiv ist: Neu zeichnen = loeschen */
		if (doc->selection.valid)
			DrawSelection(doc);

		x = m_data->x - work.g_x;
		y = m_data->y - work.g_y - win->y_offset;
		y -= y % font_ch;

		doc->getCursorProc(doc, x, y, &start);
		end = start;
	}

	for (;;)
	{
		ox = end.x - (short)(win->docsize.x * font_cw);
		oy = end.y - (short)(win->docsize.y * font_ch);

		graf_mkstate(&m_data->x, &m_data->y, &m_data->bstate, &m_data->kstate);

		{
			_WORD nbs, nmy, nmx;
			
			graf_mkstate(&nmx, &nmy, &nbs, &m_data->kstate);
			while (m_data->x == nmx && m_data->y == nmy && m_data->bstate == nbs)
			{
				if (nmy > work.g_y + work.g_h || nmy < work.g_y + win->y_offset ||
					nmx > work.g_x + work.g_w || nmx < work.g_x)
					break;
				graf_mkstate(&nmx, &nmy, &nbs, &m_data->kstate);
			}
			m_data->x = nmx;
			m_data->y = nmy;
			m_data->bstate = nbs;
		}
		/* Falls Maustaste losgelassen wurde: sofort Schleife verlassen */
		if ((m_data->bstate & 1) == 0)
			break;

	  shift_entry:
		/* Mauskoordinaten relativ zum echten Arbeitsbereich */
		x = m_data->x - work.g_x;
		y = m_data->y - work.g_y - win->y_offset;

		/* Maus ausserhalb des Arbeitsbereichs? */
		if (x < 0)
		{
			x = -1;
			scroll_x = -1;
		} else if (x >= work.g_w)
		{
			x = work.g_w;
			scroll_x = 1;
		} else
		{
			scroll_x = 0;
		}

		if (y < 0)
		{
			y = -font_ch;

			scroll_y = -win->y_speed;
			scroll_y *= 1 - ((m_data->y - work.g_y - win->y_offset) >> 6);
		} else if (y >= work.g_h - win->y_offset)
		{
			y = work.g_h - win->y_offset;
			scroll_y = win->y_speed;
			scroll_y *= 1 + ((m_data->y - work.g_y - work.g_h) >> 6);
		} else
		{
			scroll_y = 0;
		}
		
		/* Muss gescrollt werden? */
		if (scroll_x || scroll_y)
		{
			/* Versuche zu scrollen */
			if (ScrollWindow(win, &scroll_x, &scroll_y))
			{
				/* VDI Fll-Attribute reinitialisieren */
				vsf_interior(vdi_handle, FIS_SOLID);
				vsf_color(vdi_handle, G_BLACK);
				vsf_perimeter(vdi_handle, FALSE);
				vswr_mode(vdi_handle, MD_XOR);
				vs_clip(vdi_handle, TRUE, clip_rect);

				if (scroll_y)			/* wurde in y-Richtung gescrollt? */
				{
					oy -= scroll_y * font_ch;
				}

				if (scroll_x)			/* wurde in x-Richtung gescrollt? */
					ox -= scroll_x * font_cw;
			}
		}

		/* Berechne Cursor-Position im Text */
		doc->getCursorProc(doc, x, y, &new);

		x = (short)(new.x - win->docsize.x * font_cw);
		y = new.y - (win->docsize.y * font_ch);
		if (y != oy)
		{
			_WORD xy[4];
			short py1, py2;
			short px1, px2;

			if (y < oy)					/* Selektion zurueck bewegt? */
			{
				px1 = x;
				py1 = y;
				px2 = ox;
				py2 = oy;
			} else						/* Selektion vorwaerts bewegt? */
			{
				px1 = ox;
				py1 = oy;
				px2 = x;
				py2 = y;
			}

			graf_mouse(M_OFF, NULL);

			/* Evtl. bis Zeilen-Ende zeichnen */
			if (px1 < work.g_w - 1)
			{
				xy[0] = work.g_x + px1;
				xy[1] = work.g_y + win->y_offset + py1;
				xy[2] = work.g_x + work.g_w - 1;
				xy[3] = work.g_y + font_ch + win->y_offset + py1 - 1;
				vr_recfl(vdi_handle, xy);
			}

			/* Gefuellte Zeilen Zeichnen */
			py1 += font_ch;

			if (py1 < py2)
			{
				xy[0] = work.g_x;
				xy[1] = work.g_y + win->y_offset + py1;
				xy[2] = work.g_x + work.g_w - 1;
				xy[3] = work.g_y + win->y_offset + py2 - 1;
				vr_recfl(vdi_handle, xy);
			}
			if (px2 >= 0)					/* ueberhaupt etwas selektiert? */
			{
				/* Vom Zeilen-Anfang an zeichnen */
				xy[0] = work.g_x;
				xy[1] = work.g_y + win->y_offset + py2;
				xy[2] = work.g_x + px2 - 1;
				xy[3] = work.g_y + font_ch + win->y_offset + py2 - 1;
				vr_recfl(vdi_handle, xy);
			}
			graf_mouse(M_ON, NULL);
		} else if (x != ox)
		{
			short px1, px2;
			_WORD xy[4];

			if (x < ox)
			{
				px1 = x;
				px2 = ox;
			} else
			{
				px1 = ox;
				px2 = x;
			}

			graf_mouse(M_OFF, NULL);
			/* Nur die nderung zeichnen */
			xy[0] = work.g_x + px1;
			xy[1] = work.g_y + win->y_offset + oy;
			xy[2] = work.g_x + px2 - 1;
			xy[3] = work.g_y + win->y_offset + oy + font_ch - 1;
			vr_recfl(vdi_handle, xy);
			graf_mouse(M_ON, NULL);
		}


		if (scroll_x || scroll_y)
		{
			/* Kleine Pause, damit das Scrollen auch auf schnellen Rechnern
			   brauchbar ist */
			/* XXX Add some config to this */
			/* evnt_timer(300); */
		}

		end = new;

		if ((start.line == end.line) && (start.offset == end.offset))
			doc->selection.valid = FALSE;
		else
		{
			doc->selection.valid = TRUE;
			if (start.line < end.line)
			{
				doc->selection.start.line = start.line;
				doc->selection.start.y = start.y;
				doc->selection.start.offset = start.offset;
				doc->selection.start.x = start.x;
				doc->selection.end.line = end.line;
				doc->selection.end.y = end.y;
				doc->selection.end.offset = end.offset;
				doc->selection.end.x = end.x;
			} else if (start.line > end.line)
			{
				doc->selection.start.line = end.line;
				doc->selection.start.y = end.y;
				doc->selection.start.offset = end.offset;
				doc->selection.start.x = end.x;
				doc->selection.end.line = start.line;
				doc->selection.end.y = start.y;
				doc->selection.end.offset = start.offset;
				doc->selection.end.x = start.x;
			} else
			{
				doc->selection.start.line = start.line;
				doc->selection.start.y = start.y;
				doc->selection.end.line = start.line;
				doc->selection.end.y = start.y;

				if (start.x < end.x)
				{
					doc->selection.start.offset = start.offset;
					doc->selection.start.x = start.x;
					doc->selection.end.offset = end.offset;
					doc->selection.end.x = end.x;
				} else
				{
					doc->selection.start.offset = end.offset;
					doc->selection.start.x = end.x;
					doc->selection.end.offset = start.offset;
					doc->selection.end.x = start.x;
				}
			}
		}
	}
	
	vs_clip(vdi_handle, FALSE, clip_rect);	/* clipping OFF */
	wind_update(END_MCTRL);
	wind_update(END_UPDATE);
}


void RemoveSelection(DOCUMENT *doc)
{
	WINDOW_DATA *win = doc->window;

	doc->selection.valid = FALSE;				/* invalidate selection */
	SendRedraw(win);
}


void DrawSelection(DOCUMENT *doc)
{
	WINDOW_DATA *win = doc->window;
	GRECT work;
	_WORD xy[4], x1, y1, x2, y2;
	short vis_height;

	/* VDI Fll-Attribute fr XOR-Flchen */
	vsf_interior(vdi_handle, FIS_SOLID);
	vsf_color(vdi_handle, G_BLACK);
	vsf_perimeter(vdi_handle, FALSE);
	vswr_mode(vdi_handle, MD_XOR);

	if (!doc->selection.valid)				/* Keine gueltige Selektion? */
		return;								/* Zeichnen abbrechen */

	wind_get_grect(win->whandle, WF_WORKXYWH, &work);

	/* Fenster-relative Koordinaten der Selektion berechnen */
	x1 = doc->selection.start.x - (short)(win->docsize.x * font_cw);
	y1 = (_WORD) (doc->selection.start.y - (win->docsize.y * font_ch));
	x2 = doc->selection.end.x - (short)(win->docsize.x * font_cw);
	y2 = (_WORD) (doc->selection.end.y - (win->docsize.y * font_ch));

	vis_height = work.g_h - win->y_offset;
	if ((y1 >= vis_height) || (y2 < 0))
		return;

	graf_mouse(M_OFF, NULL);				/* Maus-Cursor deaktivieren */

	/* Anfang und Ende nicht auf der gleichen Zeile */
	if (y1 != y2)
	{
		if (y1 < 0)
		{
			y1 = -font_ch;
			x1 = 0;
		} else if (x1 <= work.g_w)
		{
			/* Vom Blockanfang bis zum Zeilenende zeichnen */
			xy[0] = work.g_x + x1;
			xy[1] = work.g_y + win->y_offset + y1;
			xy[2] = work.g_x + work.g_w - 1;
			xy[3] = xy[1] + font_ch - 1;
			vr_recfl(vdi_handle, xy);
		}
		
		if (y2 > vis_height)
		{
			y2 = work.g_h - win->y_offset;
			x2 = work.g_w;
		} else if (x2 > 0)
		{
			/* Vom Zeilenanfang bis zum Blockende an zeichnen */
			xy[0] = work.g_x;
			xy[1] = work.g_y + win->y_offset + y2;
			xy[2] = work.g_x + x2 - 1;
			xy[3] = xy[1] + font_ch - 1;
			vr_recfl(vdi_handle, xy);
		}

		/* Mehr als nur 1 Zeile Differenz? */
		if (y2 > y1 + font_ch)
		{
			xy[0] = work.g_x;
			xy[1] = work.g_y + win->y_offset + y1 + font_ch;
			xy[2] = work.g_x + work.g_w - 1;
			xy[3] = work.g_y + win->y_offset + y2 - 1;
			vr_recfl(vdi_handle, xy);
		}
	} else	/* Anfang und Ende auf der gleichen Zeile */
	{
		/* Nur die Aenderung zeichnen */
		xy[0] = work.g_x + x1;
		xy[1] = work.g_y + win->y_offset + y1;
		xy[2] = work.g_x + x2 - 1;
		xy[3] = work.g_y + font_ch + win->y_offset + y1 - 1;
		vr_recfl(vdi_handle, xy);
	}

	graf_mouse(M_ON, NULL);
}
