#include "hv_defs.h"
#include "w_draw.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static char *pagename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;

	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	return hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
}

/*** ---------------------------------------------------------------------- ***/

void HypGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos)
{
	DOCUMENT *doc = win->data;
	HYP_NODE *node;
	long line;
	long i;
	unsigned char textattr = 0;
	int x_pos = 0;
	int width = 0;
	const unsigned char *src;
	const unsigned char *end;
	const unsigned char *textstart;
	HYP_DOCUMENT *hyp;
	char *temp;
	HDC hdc;
	HFONT oldfont;
	
	if (doc->type != HYP_FT_HYP)
	{
		pos->line = 0;
		pos->y = 0;
		pos->offset = 0;
		pos->x = 0;
		return;
	}

	hyp = (HYP_DOCUMENT *)doc->data;
	node = win->displayed_node;

	line = (y + win->docsize.y) / win->y_raster;
	if (node->line_ptr == NULL || line < 0)
	{
		pos->line = 0;
		pos->y = 0;
		pos->offset = 0;
		pos->x = 0;
		return;
	}

	if (line >= (win->docsize.h / win->y_raster))
	{
		pos->line = line = win->docsize.h / win->y_raster;
		pos->y = line * win->y_raster;
		pos->offset = 0;
		pos->x = 0;
		return;
	}

	src = node->line_ptr[line];

	/* reset text effects */
	hdc = GetDC(win->textwin);
	oldfont = (HFONT)SelectObject(hdc, (HGDIOBJ)win->fonts[HYP_TXT_NORMAL]);
	textstart = src;
	end = node->line_ptr[line + 1];
	temp = g_strdup("");
	
#define TEXTOUT(str, len) \
	{ \
		int w, h; \
		wchar_t *wtext = hyp_utf8_to_wchar(str, len, NULL); \
		W_TextExtent(hdc, wtext, &w, &h); \
		g_free(wtext); \
		width = w; \
	}

	while (src < end && *src)
	{
		if (*src == HYP_ESC)					/* ESC-sequence ?? */
		{
			/* unwritten data? */
			if (src > textstart)
			{
				size_t len = (size_t)(src - textstart);
				char *s = hyp_conv_to_utf8(hyp->comp_charset, textstart, len);
				char *newt;
				
				/* output remaining data */
				TEXTOUT(s, STR0TERM);
				newt = g_strconcat(temp, s, NULL);
				g_free(temp);
				temp = newt;
				g_free(s);
				if (x_pos + width >= x)
					goto go_back;

				x_pos += width;
			}
			src++;

			switch (*src)
			{
			case HYP_ESC_ESC:					/* ESC */
				textstart = src;
				src++;
				break;
			case HYP_ESC_LINK:
			case HYP_ESC_LINK_LINE:
			case HYP_ESC_ALINK:
			case HYP_ESC_ALINK_LINE:
				{
					hyp_nodenr dest_page;	/* index of target page */
					char *str;
					char *newt;
					size_t len;
					
					if (*src == HYP_ESC_LINK_LINE || *src == HYP_ESC_ALINK_LINE)		/* skip line number */
						src += 2;

					dest_page = DEC_255(&src[1]);
					src += 3;

					/* calculate width of link text honoring text effects */
					SelectObject(hdc, (HGDIOBJ)win->fonts[(gl_profile.colors.link_effect | textattr) & HYP_TXT_MASK]);

					/* get link text */
					if (*src <= HYP_STRLEN_OFFSET)		/* Kein Text angegeben */
					{
						if (hypnode_valid(hyp, dest_page))
						{
							str = pagename(hyp, dest_page);
						} else
						{
							str = hyp_invalid_page(dest_page);
						}
						src++;
					} else
					{
						len = *src - HYP_STRLEN_OFFSET;

						src++;
						str = hyp_conv_to_utf8(hyp->comp_charset, src, len);
						src += len;
					}

					newt = g_strconcat(temp, str, NULL);
					g_free(temp);
					temp = newt;
					TEXTOUT(str, STR0TERM);
					g_free(str);
					if (x_pos + width >= x)
						goto go_back;

					x_pos += width;

					SelectObject(hdc, (HGDIOBJ)win->fonts[(textattr) & HYP_TXT_MASK]);
					textstart = src;
				}
				break;
			
			case HYP_ESC_CASE_TEXTATTR:
				textattr = *src - HYP_ESC_TEXTATTR_FIRST;
				SelectObject(hdc, (HGDIOBJ)win->fonts[(textattr) & HYP_TXT_MASK]);
				src++;
				textstart = src;
				break;
			
			default:
				src = hyp_skip_esc(src - 1);
				textstart = src;
				break;
			}
		} else
		{
			src++;
		}
	}

	if (src > textstart)
	{
		size_t len = (size_t)(src - textstart);
		char *s = hyp_conv_to_utf8(hyp->comp_charset, textstart, len);
		char *newt;
		
		TEXTOUT(s, STR0TERM);
		newt = g_strconcat(temp, s, NULL);
		g_free(temp);
		temp = newt;
		g_free(s);
		if (x_pos + width >= x)
			goto go_back;
		x_pos += width;
	}
	width = 0;

  go_back:
	i = strlen(temp);
	if (*temp && x_pos + width > x)
	{
		char *dst = temp + i - 1;
		int oldwidth;

		oldwidth = x_pos + width;
		width = oldwidth;
		while (dst >= temp)
		{
			i--;
			*dst = 0;
			oldwidth = width;
			TEXTOUT(temp, STR0TERM);
			if (width < x)
			{
				if (x - width > oldwidth - x)
				{
					i++;
					break;
				}
				break;
			}
			dst--;
		}

		pos->x = oldwidth;
	} else
	{
		pos->x = x_pos + width;
	}
	
	if (i == 0)
		pos->x = 0;
	pos->offset = i;
	pos->line = line;
	pos->y = line * win->y_raster;
	g_free(temp);

	SelectObject(hdc, (HGDIOBJ)oldfont);
	ReleaseDC(win->textwin, hdc);
}
