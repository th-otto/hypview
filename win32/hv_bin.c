#include "hv_defs.h"
#include "hv_ascii.h"
#include "hypdebug.h"
#include "w_draw.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void BinaryDisplayPage(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	FMT_ASCII *ascii = (FMT_ASCII *)doc->data;
	int x, y;
	char *line_buffer;
	const unsigned char *src = ascii->start;
	const unsigned char *end = src + ascii->length;
	HDC hdc = win->draw_hdc;
	HFONT oldfont;

	WindowCalcScroll(win);

	line_buffer = g_new(char, gl_profile.viewer.binary_columns + 1);
	if (line_buffer == NULL)
		return;
	x = (_WORD)(win->scroll.g_x - win->docsize.x);
	y = win->scroll.g_y;

	src += (win->docsize.y / win->y_raster) * gl_profile.viewer.binary_columns;
	end = min(end, src + ((unsigned long) win->scroll.g_h / win->y_raster + 1) * gl_profile.viewer.binary_columns);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, viewer_colors.text);
	oldfont = (HFONT)SelectObject(hdc, (HGDIOBJ)win->fonts[HYP_TXT_NORMAL]);

	while (src < end)
	{
		int i, n;
		char *dst;
		char *txt;
		wchar_t *wtext;
		
		dst = line_buffer;
		n = i = min(gl_profile.viewer.binary_columns, end - src);
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
		txt = hyp_conv_to_utf8(HYP_CHARSET_BINARY, line_buffer, n);
		wtext = hyp_utf8_to_wchar(txt, STR0TERM, NULL);
		TextOutW(hdc, x, y, wtext, wcslen(wtext));
		g_free(wtext);
		g_free(txt);
		y += win->y_raster;
	}
	g_free(line_buffer);
	
	SelectObject(hdc, (HGDIOBJ)oldfont);
}

/*** ---------------------------------------------------------------------- ***/

void BinaryGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos)
{
	DOCUMENT *doc = win->data;
	FMT_ASCII *ascii = (FMT_ASCII *)doc->data;
	long line = (y + win->docsize.y) / win->y_raster;
	_WORD i;
	const unsigned char *start;
	int width = 0;
	int oldwidth;
	HDC hdc;
	HFONT oldfont;
	
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

	x += (win->docsize.x);

	start = ascii->start + line * gl_profile.viewer.binary_columns;

	hdc = GetDC(win->textwin);
	w_inithdc(hdc);
	oldfont = (HFONT)SelectObject(hdc, (HGDIOBJ)win->fonts[HYP_TXT_NORMAL]);

	i = 0;
	while (i < gl_profile.viewer.binary_columns)
	{
		i++;

		oldwidth = width;
		{
			int w, h;
			char *txt = hyp_conv_to_utf8(HYP_CHARSET_BINARY, start, i);
			wchar_t *wtext = hyp_utf8_to_wchar(txt, STR0TERM, NULL);
			W_TextExtent(hdc, wtext, &w, &h);
			g_free(wtext);
			g_free(txt);
			width = w;
		}

		if (width >= x)
		{
			if (width - x > x - oldwidth)
			{
				i--;
				break;
			}
			break;
		}
	}

	pos->line = line;
	pos->y = line * win->y_raster;
	pos->offset = i;
	pos->x = width;

	SelectObject(hdc, (HGDIOBJ)oldfont);
	ReleaseDC(win->textwin, hdc);
}
