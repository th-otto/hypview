#include "hv_defs.h"
#include "hypdebug.h"
#include "w_draw.h"

#define vst_effects(handle, attr)
#define vsl_ends(handle, beg, end)	

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static long DrawPicture(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	WP_UNIT tx, ty;
	HYP_IMAGE *pic;
	
	pic = (HYP_IMAGE *)AskCache(hyp, gfx->extern_node_index);

	if (pic && pic->decompressed)
	{
		DWORD mode;
		
		if (gfx->x_offset == 0)
		{
			tx = (hyp->line_width * win->x_raster - gfx->pixwidth) / 2;
			if (tx < 0)
				tx = 0;
		} else
		{
			tx = (gfx->x_offset - 1) * win->x_raster;
		}
		
		tx += x;
		ty = y;
		if (gfx->islimage)
		{
			y += ((gfx->pixheight + win->y_raster - 1) / win->y_raster) * win->y_raster;
		}
		if (tx >= win->scroll.g_w || (tx + gfx->pixwidth) <= 0)
			return y;
		if (ty >= win->scroll.g_h || (ty + gfx->pixheight) <= 0)
			return y;

		if (gfx->planes == 1)
		{
			if (gl_profile.viewer.transparent_pics)
				mode = SRCAND;
			else
				mode = SRCCOPY;
			W_Draw_Image(win->draw_hdc, tx + win->scroll.g_x, ty + win->scroll.g_y, gfx->pixwidth, gfx->pixheight, pic->pic.fd_addr, W_PAL_BLACK, W_PAL_WHITE, mode);
		} else
		{
			_WORD scrn_planes = GetNumPlanes();
			GRECT gr;
			
			if (!gl_profile.viewer.transparent_pics)
				mode = SRCCOPY;
			else if (scrn_planes > 8)
				mode = SRCAND;
			else
				mode = SRCPAINT;
			gr.g_x = 0;
			gr.g_y = 0;
			gr.g_w = gfx->pixwidth;
			gr.g_h = gfx->pixheight;
			W_Draw_Picture(win->draw_hdc, tx + win->scroll.g_x, ty + win->scroll.g_y, &gr, &pic->pic, mode);
		}
	}
	return y;
}

/*** ---------------------------------------------------------------------- ***/

static void DrawLine(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y)
{
	POINT xy[2];
	int w, h;
	WP_UNIT tx, ty;
	HDC hdc = win->draw_hdc;
	
	tx = (gfx->x_offset - 1) * win->x_raster;
	w = gfx->width * win->x_raster;
	h = gfx->height * win->y_raster;

	tx += x;
	ty = y;
	
	if ((tx >= win->scroll.g_w && (tx + w) >= win->scroll.g_w) || (tx < 0 && (tx + w) < 0))
		return;
	if (ty >= win->scroll.g_h || (ty + h) < 0)
		return;

	/* rectangle */
	xy[0].x = tx + win->scroll.g_x;
	xy[0].y = ty + win->scroll.g_y;
	xy[1].x = xy[0].x + w;
	xy[1].y = xy[0].y + h;

	if (w)
	{
		if (w < 0)
			xy[1].x++;
		else
			xy[1].x--;
	}
	if (h)
		xy[1].y--;

	vsl_ends(hdc, gfx->begend & ARROWED, (gfx->begend >> 1) & ARROWED);
	W_Lines(hdc, xy, 2, gfx->style, viewer_colors.text);
	vsl_ends(hdc, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

static void DrawBox(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y)
{
	GRECT gr;
	_WORD w, h;
	WP_UNIT tx, ty;
	int fillstyle;
	HDC hdc = win->draw_hdc;
	
	tx = (gfx->x_offset - 1) * win->x_raster;
	w = gfx->width * win->x_raster;
	h = gfx->height * win->y_raster;

	tx += x;
	ty = y;
	
	if (tx >= win->scroll.g_w || (tx + w) <= 0)
		return;
	if (ty >= win->scroll.g_h || (ty + h) <= 0)
		return;

	/* rectangle */
	gr.g_x = (_WORD) (tx + win->scroll.g_x);
	gr.g_y = (_WORD) (ty + win->scroll.g_y);
	gr.g_w = w;
	gr.g_h = h;
	
	if (gfx->style != 0)
	{
		fillstyle = gfx->style;
	} else
	{
		fillstyle = IP_HOLLOW;
	}

	if (gfx->type == HYP_ESC_BOX)
	{
		if (fillstyle != IP_HOLLOW)
			W_Fill_Rect(hdc, &gr, fillstyle, viewer_colors.text);
		W_Rectangle(hdc, &gr, W_PEN_SOLID, viewer_colors.text);
	} else
	{
#if 0
		if (gfx->style != 0)
			v_rfbox(vdi_handle, xy);
		else
			v_rbox(vdi_handle, xy);
#endif
	}
}

/*** ---------------------------------------------------------------------- ***/

static char *pagename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;

	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	return hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
}

/*** ---------------------------------------------------------------------- ***/

static long draw_graphics(WINDOW_DATA *win, struct hyp_gfx *gfx, long lineno, WP_UNIT sx, WP_UNIT sy)
{
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			switch (gfx->type)
			{
			case HYP_ESC_PIC:
				sy = DrawPicture(win, gfx, sx, sy);
				break;
			case HYP_ESC_LINE:
				DrawLine(win, gfx, sx, sy);
				break;
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
				DrawBox(win, gfx, sx, sy);
				break;
			}
		}
		gfx = gfx->next;
	}
	return sy;
}

/*** ---------------------------------------------------------------------- ***/

static long SkipPicture(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	HYP_IMAGE *pic;
	
	pic = (HYP_IMAGE *)AskCache(hyp, gfx->extern_node_index);
	UNUSED(x);
	
	if (pic && pic->decompressed)
	{
		if (gfx->islimage)
		{
			y += ((gfx->pixheight + win->y_raster - 1) / win->y_raster) * win->y_raster;
		}
	}
	return y;
}

/*** ---------------------------------------------------------------------- ***/

static long skip_graphics(WINDOW_DATA *win, struct hyp_gfx *gfx, long lineno, WP_UNIT sx, WP_UNIT sy)
{
	while (gfx != NULL)
	{
		if (gfx->y_offset == lineno)
		{
			switch (gfx->type)
			{
			case HYP_ESC_PIC:
				sy = SkipPicture(win, gfx, sx, sy);
				break;
			case HYP_ESC_LINE:
				break;
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
				break;
			}
		}
		gfx = gfx->next;
	}
	return sy;
}

/*** ---------------------------------------------------------------------- ***/

void HypDisplayPage(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	HYP_NODE *node = win->displayed_node;
	long lineno;
	WP_UNIT x, sx, sy;
	const unsigned char *src, *end, *textstart;
	unsigned char textattr;
	char *str;
	HDC hdc = win->draw_hdc;
	HFONT oldfont;
	
	if (node == NULL)					/* stop if no page loaded */
		return;

	end = node->end;
	
	WindowCalcScroll(win);
	
	sx = -win->docsize.x;
	sy = -win->docsize.y;

	/* standard text color */
	SetTextColor(hdc, viewer_colors.text);
	oldfont = (HFONT)SelectObject(hdc, win->font);
	vst_effects(hdc, 0);

#define TEXTOUT(str) \
	{ \
		int w, h; \
		wchar_t *wtext = hyp_utf8_to_wchar(str, STR0TERM, NULL); \
		size_t len = wcslen(wtext); \
		W_NTextExtent(hdc, wtext, len, &w, &h); \
		if (x < win->scroll.g_w && (x + w) > 0 && sy < win->scroll.g_h && (sy + win->y_raster) > 0) \
			TextOutW(hdc, x + win->scroll.g_x, sy + win->scroll.g_y, wtext, len); \
		x += w; \
		g_free(wtext); \
	}

#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		/* draw remaining text */ \
		char *s; \
		size_t len = (size_t)(src - textstart); \
		s = hyp_conv_to_utf8(hyp->comp_charset, textstart, len); \
		TEXTOUT(s); \
		g_free(s); \
	}

	src = node->start;
	textstart = src;
	textattr = 0;
	lineno = 0;
	x = sx;
	sy = draw_graphics(win, node->gfx, lineno, sx, sy);
	vst_effects(hdc, 0);
	SetBkMode(hdc, TRANSPARENT);
	
	while (src < end)
	{
		if (*src == HYP_ESC)		/* ESC-sequence */
		{
			/* unwritten data? */
			DUMPTEXT();
			src++;

			switch (*src)
			{
			case HYP_ESC_ESC:		/* ESC */
				textstart = src;
				src++;
				break;

			case HYP_ESC_LINK:
			case HYP_ESC_LINK_LINE:
			case HYP_ESC_ALINK:
			case HYP_ESC_ALINK_LINE:
				{
					hyp_nodenr dest_page;	/* index of destination page */
					unsigned char link_type = *src;	/* remember link type */
					COLORREF color;
					hyp_indextype dst_type = HYP_NODE_EOF;
					
					src++;
					if (link_type == HYP_ESC_LINK_LINE || link_type == HYP_ESC_ALINK_LINE)	/* skip destination line number */
						src += 2;

					dest_page = DEC_255(src);
					src += 2;

					/* get link text for output */
					if (*src <= HYP_STRLEN_OFFSET)	/* no text in link: use nodename */
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
						_UWORD len = *src - HYP_STRLEN_OFFSET;
						src++;
						str = hyp_conv_to_utf8(hyp->comp_charset, src, len);
						src += len;
					}
					
					if (hypnode_valid(hyp, dest_page))
					{
						dst_type = (hyp_indextype)hyp->indextable[dest_page]->type;
						switch (dst_type)
						{
						case HYP_NODE_INTERNAL:
							color = viewer_colors.link;
							break;
						case HYP_NODE_POPUP:
							color = viewer_colors.popup;
							break;
						case HYP_NODE_EXTERNAL_REF:
							color = viewer_colors.xref;
							break;
						case HYP_NODE_REXX_COMMAND:
							color = viewer_colors.rx;
							break;
						case HYP_NODE_REXX_SCRIPT:
							color = viewer_colors.rxs;
							break;
						case HYP_NODE_QUIT:
							color = viewer_colors.quit;
							break;
						case HYP_NODE_CLOSE:
							color = viewer_colors.close;
							break;
						case HYP_NODE_SYSTEM_ARGUMENT:
							color = viewer_colors.system;
							break;
						case HYP_NODE_IMAGE:
						case HYP_NODE_EOF:
						default:
							color = viewer_colors.error;
							break;
						}
					} else
					{
						color = viewer_colors.error;
					}
					
					/* set text effects for link text */
					SetTextColor(hdc, color);
					vst_effects(hdc, gl_profile.colors.link_effect | textattr);

					TEXTOUT(str);
					g_free(str);

					SetTextColor(hdc, viewer_colors.text);
					vst_effects(hdc, textattr);
					textstart = src;
				}
				break;
				
			case HYP_ESC_CASE_TEXTATTR:
				textattr = *src - HYP_ESC_TEXTATTR_FIRST;
				vst_effects(hdc, textattr);
				src++;
				textstart = src;
				break;
			
			case HYP_ESC_WINDOWTITLE:
			case HYP_ESC_CASE_DATA:
			case HYP_ESC_EXTERNAL_REFS:
			case HYP_ESC_OBJTABLE:
			case HYP_ESC_PIC:
			case HYP_ESC_LINE:
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
			default:
				src = hyp_skip_esc(--src);
				textstart = src;
				break;
			}
		} else if (*src == HYP_EOL)
		{
			DUMPTEXT();
			++lineno;
			src++;
			textstart = src;
			x = sx;
			sy += win->y_raster;
			sy = draw_graphics(win, node->gfx, lineno, sx, sy);
		} else
		{
			src++;
		}
	}
	DUMPTEXT();
	if (x != sx)
	{
		++lineno;
		sy += win->y_raster;
		sy = draw_graphics(win, node->gfx, lineno, sx, sy);
	}
	
	vst_effects(hdc, 0);
	SelectObject(hdc, (HGDIOBJ)oldfont);
	UNUSED(textattr);
	
#undef TEXTOUT
#undef DUMPTEXT
}

/*** ---------------------------------------------------------------------- ***/

HYP_NODE *hypwin_node(WINDOW_DATA *win)
{
	return win->displayed_node;
}

/*** ---------------------------------------------------------------------- ***/

void HypPrepNode(WINDOW_DATA *win, HYP_NODE *node)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	long lineno;
	WP_UNIT x, sx, sy;
	WP_UNIT max_w;
	const unsigned char *src, *end, *textstart;
	const unsigned char *linestart;
	unsigned char textattr;
	char *str;
	long old_windowline;
	long new_windowline;
	HDC hdc;
	HFONT oldfont;
	
	win->displayed_node = node;
	if (node == NULL)					/* stop if no page loaded */
		return;

	g_free(win->title);
	if (node->window_title)
		win->title = hyp_conv_to_utf8(hyp->comp_charset, node->window_title, STR0TERM);
	else
		win->title = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[node->number]->name, STR0TERM);
	hv_set_title(win, win->title);
	
	StartRemarker(win, remarker_update, TRUE);
	
	hdc = GetDC(win->textwin);
	sx = sy = 0;
	w_inithdc(hdc);
	oldfont = (HFONT)SelectObject(hdc, win->font);

#define TEXTOUT(str) \
	{ \
	int w, h; \
	wchar_t *wtext = hyp_utf8_to_wchar(str, STR0TERM, NULL); \
	W_TextExtent(hdc, wtext, &w, &h); \
	g_free(wtext); \
	x += w; \
	}

#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		/* draw remaining text */ \
		char *s; \
		size_t len = (size_t)(src - textstart); \
		s = hyp_conv_to_utf8(hyp->comp_charset, textstart, len); \
		TEXTOUT(s); \
		g_free(s); \
	}

	src = node->start;
	end = node->end;
	textstart = src;
	textattr = 0;
	lineno = 0;
	x = sx;
	max_w = x;
	old_windowline = sy / win->y_raster;
	sy = skip_graphics(win, node->gfx, lineno, sx, sy);
	vst_effects(hdc, 0);

	g_free(node->line_ptr);
	node->line_ptr = NULL;
	linestart = src;
	
	new_windowline = sy / win->y_raster;
	node->line_ptr = g_renew(const unsigned char *, node->line_ptr, new_windowline + 1);
	while (old_windowline < new_windowline)
		node->line_ptr[old_windowline++] = linestart;

	while (src < end)
	{
		if (*src == HYP_ESC)		/* ESC-sequence */
		{
			/* unwritten data? */
			DUMPTEXT();
			src++;

			switch (*src)
			{
			case HYP_ESC_ESC:		/* ESC */
				textstart = src;
				src++;
				break;

			case HYP_ESC_LINK:
			case HYP_ESC_LINK_LINE:
			case HYP_ESC_ALINK:
			case HYP_ESC_ALINK_LINE:
				{
					hyp_nodenr dest_page;	/* index of destination page */
					unsigned char link_type = *src;	/* remember link type */
					
					src++;
					if (link_type == HYP_ESC_LINK_LINE || link_type == HYP_ESC_ALINK_LINE)	/* skip destination line number */
						src += 2;

					dest_page = DEC_255(src);
					src += 2;

					/* get link text for output */
					if (*src <= HYP_STRLEN_OFFSET)	/* no text in link: use nodename */
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
						_UWORD len = *src - HYP_STRLEN_OFFSET;
						src++;
						str = hyp_conv_to_utf8(hyp->comp_charset, src, len);
						src += len;
					}
					
					/* set text effects for link text */
					vst_effects(hdc, gl_profile.colors.link_effect | textattr);

					TEXTOUT(str);
					g_free(str);

					vst_effects(hdc, textattr);
					textstart = src;
				}
				break;
				
			case HYP_ESC_CASE_TEXTATTR:
				textattr = *src - HYP_ESC_TEXTATTR_FIRST;
				vst_effects(hdc, textattr);
				src++;
				textstart = src;
				break;
			
			case HYP_ESC_WINDOWTITLE:
			case HYP_ESC_CASE_DATA:
			case HYP_ESC_EXTERNAL_REFS:
			case HYP_ESC_OBJTABLE:
			case HYP_ESC_PIC:
			case HYP_ESC_LINE:
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
			default:
				src = hyp_skip_esc(--src);
				textstart = src;
				break;
			}
		} else if (*src == HYP_EOL)
		{
			old_windowline = sy / win->y_raster;
			
			DUMPTEXT();
			++lineno;
			if (x > max_w)
				max_w = x;
			src++;
			textstart = src;
			x = sx;
			sy += win->y_raster;
			sy = skip_graphics(win, node->gfx, lineno, sx, sy);
			
			new_windowline = sy / win->y_raster;
			node->line_ptr = g_renew(const unsigned char *, node->line_ptr, new_windowline + 1);
			while (old_windowline < new_windowline)
				node->line_ptr[old_windowline++] = linestart;
			linestart = src;
		} else
		{
			src++;
		}
	}
	DUMPTEXT();
	old_windowline = sy / win->y_raster;
	if (x != sx)
	{
		if (x > max_w)
			max_w = x;
		++lineno;
		sy += win->y_raster;
		sy = skip_graphics(win, node->gfx, lineno, sx, sy);
	}
	new_windowline = sy / win->y_raster;
	node->line_ptr = g_renew(const unsigned char *, node->line_ptr, new_windowline + 1);
	while (old_windowline < new_windowline)
		node->line_ptr[old_windowline++] = linestart;
	node->line_ptr[old_windowline] = src;
	
	vst_effects(hdc, 0);
	UNUSED(textattr);
	
	node->width = max_w;
	node->height = sy;
	win->docsize.w = node->width;
	win->docsize.h = node->height;
	
	SelectObject(hdc, (HGDIOBJ)oldfont);
	ReleaseDC(win->textwin, hdc);
	
#undef TEXTOUT
#undef DUMPTEXT
}
