/*
 * HypView - (c)      - 2019 Thorsten Otto
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
 * along with HypView; if not, see <http://www.gnu.org/licenses/>.
 */

#include "hv_gtk.h"
#include "hv_ascii.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void AsciiDisplayPage(WINDOW_DATA *win)
{
	UNUSED(win);
	/* nothing to do */
}

/*** ---------------------------------------------------------------------- ***/

void AsciiPrep(WINDOW_DATA *win, HYP_NODE *node)
{
	DOCUMENT *doc = win->data;
	FMT_ASCII *ascii = (FMT_ASCII *)doc->data;
	long line;
	GtkTextBuffer *text_buffer = win->text_buffer;
	GtkTextIter iter;
	
	UNUSED(node);
	/*
	 * clear buffer
	 */
	hv_win_reset_text(win);
	g_free(win->title);
	win->title = g_strdup(doc->path);
	hv_set_title(win, win->title);
	gtk_text_buffer_get_iter_at_offset(text_buffer, &iter, 0);

	if (doc->type == HYP_FT_ASCII)
	{
		for (line = 0; line < ascii->lines; line++)
		{
			const unsigned char *src = ascii->line_ptr[line];
			size_t n = ascii->line_ptr[line + 1] - ascii->line_ptr[line];
			char *txt;
			
			if (n > 0 && src[n - 1] == 0x0a)
				n--;
			if (n > 0 && src[n - 1] == 0x0d)
				n--;
			txt = hyp_conv_to_utf8(ascii->charset, src, n);
			gtk_text_buffer_insert(text_buffer, &iter, txt, -1);
			g_free(txt);
			gtk_text_buffer_insert(text_buffer, &iter, "\n", 1);
		}
	} else
	{
		const unsigned char *src = ascii->start;
		const unsigned char *end = src + ascii->length;
		
		while (src < end)
		{
			int n = (int)min(gl_profile.viewer.binary_columns, end - src);
			char *txt = hyp_conv_to_utf8(HYP_CHARSET_BINARY, src, n);
			gtk_text_buffer_insert(text_buffer, &iter, txt, -1);
			g_free(txt);
			gtk_text_buffer_insert(text_buffer, &iter, "\n", 1);
			src += n;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void AsciiGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos)
{
	DOCUMENT *doc = win->data;
	
	if (doc->type != HYP_FT_ASCII)
	{
		HYP_DBG(("Illegal call for this document type"));
		return;
	}

	/* not needed in GTK */
	UNUSED(x);
	UNUSED(y);
	UNUSED(pos);
}
