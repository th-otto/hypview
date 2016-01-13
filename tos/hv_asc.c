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
#include "hv_ascii.h"
#include "hypdebug.h"


void AsciiDisplayPage(DOCUMENT *doc)
{
	WINDOW_DATA *win = doc->window;
	FMT_ASCII *ascii = doc->data;
	long line = win->docsize.y;
	_WORD x, y;
	_WORD end_y;
	unsigned char *line_buffer;

	WindowCalcScroll(win);

	x = (_WORD)(win->scroll.g_x - win->docsize.x * win->y_raster);
	y = win->scroll.g_y;

	vswr_mode(vdi_handle, MD_TRANS);
	vst_color(vdi_handle, gl_profile.viewer.text_color);
	vst_effects(vdi_handle, 0);

	end_y = y + min((unsigned short) (doc->lines - win->docsize.y) * win->y_raster, win->scroll.g_h);
	while (y < end_y)
	{
		line_buffer = AsciiGetTextLine(ascii->line_ptr[line], ascii->line_ptr[line + 1]);
		line++;
		if (line_buffer != NULL)
		{
			v_gtext(vdi_handle, x, y, (const char *)line_buffer);
			g_free(line_buffer);
		}
		
		y += win->y_raster;
	}
}


void AsciiGetCursorPosition(DOCUMENT *doc, int x, int y, TEXT_POS *pos)
{
	WINDOW_DATA *win = doc->window;
	FMT_ASCII *ascii = doc->data;
	long line = y / win->y_raster + win->docsize.y;
	size_t i;
	unsigned char *temp;
	const unsigned char *src;
	_WORD ext[8];
	short width;

	if (doc->type != HYP_FT_ASCII)
	{
		HYP_DBG(("Illegal call for this document type"));
		return;
	}

	if (line < 0)
	{
		pos->line = 0;
		pos->y = 0;
		pos->offset = 0;
		pos->x = 0;
		return;
	}
	if (line >= doc->lines)
	{
		pos->line = doc->lines;
		pos->y = doc->lines * win->y_raster;
		pos->offset = 0;
		pos->x = 0;
		return;
	}

	x += (short) (win->y_raster * win->docsize.x);

	i = 0;
	width = 0;
	temp = AsciiGetTextLine(ascii->line_ptr[line], ascii->line_ptr[line + 1]);
	if (temp != NULL)
	{
		src = temp;
		vst_effects(vdi_handle, 0);
	
		while (*src)
		{
			src++;
			i++;
			vqt_extentn(vdi_handle, (const char *)temp, (_WORD)i, ext);
	
			if (ext[2] >= x)
			{
				if (ext[2] - x > x - width)
				{
					i--;
					break;
				}
				width = (ext[2] + ext[4]) >> 1;
				break;
			}
			width = (ext[2] + ext[4]) >> 1;
		}
		g_free(temp);
	}
	
	pos->line = line;
	pos->y = line * win->y_raster;
	pos->offset = i;
	pos->x = width;
}
