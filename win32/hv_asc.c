#include "hv_defs.h"
#include "hv_ascii.h"
#include "hypdebug.h"
#include "w_draw.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void AsciiDisplayPage(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	FMT_ASCII *ascii = (FMT_ASCII *)doc->data;
	long topline = win->docsize.y / win->y_raster;
	_WORD x, y;
	_WORD end_y;
	unsigned char *line_buffer;
	HDC hdc = win->draw_hdc;
	HFONT oldfont;

	WindowCalcScroll(win);

	x = (_WORD)(win->scroll.g_x - win->docsize.x);
	y = win->scroll.g_y;

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, viewer_colors.text);
	oldfont = (HFONT)SelectObject(hdc, (HGDIOBJ)win->fonts[HYP_TXT_NORMAL]);
	
	end_y = y + win->scroll.g_h;
	if (doc->type == HYP_FT_ASCII)
	{
		long line = topline;
		
		while (line < ascii->lines && y < end_y)
		{
			line_buffer = AsciiGetTextLine(ascii->line_ptr[line], ascii->line_ptr[line + 1]);
			line++;
			if (line_buffer != NULL)
			{
				char *txt = hyp_conv_to_utf8(ascii->charset, line_buffer, STR0TERM);
				wchar_t *wtext = hyp_utf8_to_wchar(txt, STR0TERM, NULL);
				size_t len = wcslen(wtext);
				TextOutW(hdc, x, y, wtext, len);
				g_free(wtext);
				g_free(txt);
				g_free(line_buffer);
			}
			
			y += win->y_raster;
		}
	} else
	{
		const unsigned char *src = ascii->start;
		const unsigned char *end = src + ascii->length;
		long line = 0;
		
		while (src < end && line < ascii->lines)
		{
			int n = (int)min(gl_profile.viewer.binary_columns, end - src);
			if (line >= win->docsize.y)
			{
				char *txt = hyp_conv_to_utf8(HYP_CHARSET_BINARY, src, n);
				wchar_t *wtext = hyp_utf8_to_wchar(txt, STR0TERM, NULL);
				size_t len = wcslen(wtext);
				TextOutW(hdc, x, y, wtext, len);
				g_free(wtext);
				g_free(txt);
				y += win->y_raster;
			}
			src += n;
			line++;
		}
	}
	
	SelectObject(hdc, (HGDIOBJ)oldfont);
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
	int width, oldwidth;
	HDC hdc;
	HFONT oldfont;

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

	x += (win->docsize.x);

	hdc = GetDC(win->textwin);
	w_inithdc(hdc);
	oldfont = (HFONT)SelectObject(hdc, (HGDIOBJ)win->fonts[HYP_TXT_NORMAL]);
	i = 0;
	width = 0;
	temp = AsciiGetTextLine(ascii->line_ptr[line], ascii->line_ptr[line + 1]);
	if (temp != NULL)
	{
		src = temp;
	
		while (*src)
		{
			src++;
			i++;
			oldwidth = width;
			{
				int w, h;
				wchar_t *wtext = hyp_utf8_to_wchar((const char *)temp, i, NULL);
				W_TextExtent(hdc, wtext, &w, &h);
				g_free(wtext);
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
		g_free(temp);
	}
	
	pos->line = line;
	pos->y = line * win->y_raster;
	pos->offset = i;
	pos->x = width;

	SelectObject(hdc, (HGDIOBJ)oldfont);
	ReleaseDC(win->textwin, hdc);
}
