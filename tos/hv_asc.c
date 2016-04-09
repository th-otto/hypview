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

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void AsciiDisplayPage(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	FMT_ASCII *ascii = (FMT_ASCII *)doc->data;
	long line = win->docsize.y / win->y_raster;
	_WORD x, y;
	_WORD end_y;
	unsigned char *line_buffer;

	WindowCalcScroll(win);

	x = (_WORD)(win->scroll.g_x - win->docsize.x);
	y = win->scroll.g_y;

	vswr_mode(vdi_handle, MD_TRANS);
	vst_color(vdi_handle, viewer_colors.text);
	vst_effects(vdi_handle, 0);

	end_y = y + win->scroll.g_h;
	while (line < ascii->lines && y < end_y)
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

/*** ---------------------------------------------------------------------- ***/

void AsciiPrep(WINDOW_DATA *win, HYP_NODE *node)
{
	DOCUMENT *doc = win->data;
	FMT_ASCII *ascii = (FMT_ASCII *)doc->data;
	UNUSED(node);
	win->docsize.w = ascii->columns * win->x_raster;
	win->docsize.h = ascii->lines * win->y_raster;
}

/*** ---------------------------------------------------------------------- ***/

void AsciiGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos)
{
	DOCUMENT *doc = win->data;
	FMT_ASCII *ascii = (FMT_ASCII *)doc->data;
	long line = (y + win->docsize.y) / win->y_raster;
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
	if (line * win->y_raster >= win->docsize.h)
	{
		pos->line = win->docsize.h / win->y_raster;
		pos->y = win->docsize.h;
		pos->offset = 0;
		pos->x = 0;
		return;
	}

	x += (short) (win->docsize.x);

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
