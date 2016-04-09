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

void BinaryDisplayPage(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	FMT_ASCII *ascii = (FMT_ASCII *)doc->data;
	short x, y;
	char *line_buffer;
	const unsigned char *src = ascii->start;
	const unsigned char *end = src + ascii->length;

	WindowCalcScroll(win);

	line_buffer = g_new(char, gl_profile.viewer.binary_columns + 1);
	if (line_buffer == NULL)
		return;
	x = (_WORD)(win->scroll.g_x - win->docsize.x);
	y = win->scroll.g_y;

	src += (win->docsize.y / win->y_raster) * gl_profile.viewer.binary_columns;
	end = min(end, src + ((unsigned long) win->scroll.g_h / win->y_raster) * gl_profile.viewer.binary_columns);

	vswr_mode(vdi_handle, MD_TRANS);
	vst_color(vdi_handle, viewer_colors.text);
	vst_effects(vdi_handle, 0);

	while (src < end)
	{
		short i, n;
		char *dst;

		dst = line_buffer;
		n = i = (short) min(gl_profile.viewer.binary_columns, end - src);
		while (i--)
		{
			if (*src)
				*dst++ = *src++;
			else
			{
				*dst++ = ' ';
				src++;
			}
		}
		*dst = 0;
		v_gtextn(vdi_handle, x, y, line_buffer, n);
		y += win->y_raster;
	}
	g_free(line_buffer);
}

/*** ---------------------------------------------------------------------- ***/

void BinaryGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos)
{
	DOCUMENT *doc = win->data;
	FMT_ASCII *ascii = (FMT_ASCII *)doc->data;
	long line = (y + win->docsize.y) / win->y_raster;
	_WORD i;
	const unsigned char *start;
	_WORD ext[8];
	short width = 0;

	if (doc->type != HYP_FT_BINARY)
	{
		HYP_DBG(("Illegal call for this document type"));
		return;
	}

	/* if line was not found in document... */
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

	start = ascii->start + line * gl_profile.viewer.binary_columns;
	vst_effects(vdi_handle, 0);

	i = 0;
	while (i < gl_profile.viewer.binary_columns)
	{
		i++;

		vqt_extentn(vdi_handle, (const char *)start, i, ext);

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

	pos->line = line;
	pos->y = line * win->y_raster;
	pos->offset = i;
	pos->x = width;
}
