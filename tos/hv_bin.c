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

void BinaryDisplayPage(DOCUMENT *doc)
{
	FMT_ASCII *ascii = doc->data;
	short x, y;
	char line_buffer[1024];
	const unsigned char *src = ascii->start;
	const unsigned char *end = src + ascii->length;
	WINDOW_DATA *win = doc->window;

	wind_get_grect(win->whandle, WF_WORKXYWH, &win->work);

	x = win->work.g_x - (short) (win->docsize.x) * font_cw;
	y = win->work.g_y + win->y_offset;

	src += win->docsize.y * gl_profile.viewer.binary_columns;
	end = min(end, src + ((unsigned long) win->work.g_h / win->y_raster) * gl_profile.viewer.binary_columns);

	vswr_mode(vdi_handle, MD_TRANS);
	vst_color(vdi_handle, G_BLACK);
	vst_effects(vdi_handle, 0);

	while (src < end)
	{
		short i;
		char *dst;

		dst = line_buffer;
		i = (short) min(gl_profile.viewer.binary_columns, end - src + 1);
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
		v_gtext(vdi_handle, x, y, line_buffer);
		y += win->y_raster;
	}
}

/*** ---------------------------------------------------------------------- ***/

void BinaryGetCursorPosition(DOCUMENT *doc, int x, int y, TEXT_POS *pos)
{
	WINDOW_DATA *win = doc->window;
	FMT_ASCII *ascii = doc->data;
	long line = y / win->y_raster + win->docsize.y;
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
	if (line >= doc->lines)
	{
		pos->line = doc->lines;
		pos->y = doc->lines * win->y_raster;
		pos->offset = 0;
		pos->x = 0;
		return;
	}

	x += (short) (font_cw * win->docsize.x);

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
