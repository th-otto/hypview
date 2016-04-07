#include "hv_defs.h"
#include "hypdebug.h"
#include <mint/cookie.h>

struct fVDI_cookie {
	short version;
	short flags;
	long (*remove)(void);
	long (*setup)(unsigned long type, long value);
	void /* struct fVDI_log */ *log;
};

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
		_WORD xy[8];
		MFDB screen, src;
		_WORD scrn_planes = GetNumPlanes();
		_WORD mode;
		
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

		/* source rectangle */
		xy[0] = 0;
		xy[1] = 0;
		xy[2] = gfx->pixwidth - 1;
		xy[3] = gfx->pixheight - 1;

		/* destination rectangle */
		xy[4] = (_WORD) (tx + win->scroll.g_x);
		xy[5] = (_WORD) (ty + win->scroll.g_y);
		xy[6] = xy[4] + xy[2];
		xy[7] = xy[5] + xy[3];

		memset(&screen, 0, sizeof(screen));
		src.fd_addr = pic->pic.fd_addr;
		src.fd_w = pic->pic.fd_w;
		src.fd_h = pic->pic.fd_h;
		src.fd_wdwidth = pic->pic.fd_wdwidth;
		src.fd_stand = FALSE;
		src.fd_nplanes = scrn_planes;
		src.fd_r1 = src.fd_r2 = src.fd_r3 = 0;
		
		if (gfx->planes == 1)
		{
			_WORD col[2];
			
			col[0] = G_BLACK;
			col[1] = G_WHITE;
			src.fd_nplanes = 1;
			if (gl_profile.viewer.transparent_pics)
				mode = MD_TRANS;
			else
				mode = MD_REPLACE;
			vrt_cpyfm(vdi_handle, mode, xy, &src, &screen, col);
		} else
		{
			if (!gl_profile.viewer.transparent_pics)
				mode = S_ONLY;
			else if (scrn_planes > 8)
				mode = S_AND_D;
			else
				mode = S_OR_D;
			vro_cpyfm(vdi_handle, mode, xy, &src, &screen);
		}
	}
	return y;
}

/*** ---------------------------------------------------------------------- ***/

static void DrawLine(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y)
{
	_WORD xy[10], w, h;
	WP_UNIT tx, ty;

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
	xy[0] = (_WORD) (tx + win->scroll.g_x);
	xy[1] = (_WORD) (ty + win->scroll.g_y);
	xy[2] = xy[0] + w;
	xy[3] = xy[1] + h;

	if (w)
	{
		if (w < 0)
			xy[2]++;
		else
			xy[2]--;
	}
	if (h)
		xy[3]--;

	vsl_ends(vdi_handle, gfx->begend & ARROWED, (gfx->begend >> 1) & ARROWED);
	vsl_type(vdi_handle, gfx->style);
	v_pline(vdi_handle, 2, xy);
	vsl_ends(vdi_handle, 0, 0);
	vsl_type(vdi_handle, SOLID);

#if 0
	{
	_WORD dummy;
	v_get_pixel(vdi_handle, 999, 999, &dummy, &dummy);
	/* some fVDI testing code */
	vsl_ends(vdi_handle, 1, 1);
	xy[0] = 150 + win->scroll.g_x;
	xy[1] = 16 + win->scroll.g_y;
	xy[2] = xy[0] + 80;
	xy[3] = xy[1] + 40;
	xy[4] = xy[2];
	xy[5] = xy[3] + 80;
	vsl_width(vdi_handle, 10);
	v_pline(vdi_handle, 2, xy);
	xy[0] = 220 + win->scroll.g_x;
	xy[1] = 16 + win->scroll.g_y;
	xy[2] = xy[0] + 80;
	xy[3] = xy[1] + 40;
	xy[4] = xy[2];
	xy[5] = xy[3] + 80;
	vsl_width(vdi_handle, 1);
	v_pline(vdi_handle, 2, xy);
	vsl_width(vdi_handle, 1);
	vsl_width(vdi_handle, 1);
	vsl_ends(vdi_handle, 0, 0);
	/* top left */
	xy[0] = 320 + win->scroll.g_x;
	xy[1] = 16 + win->scroll.g_y;
	/* bottom left */
	xy[2] = xy[0];
	xy[3] = xy[1] + 40 - 1;
	/* bottom right */
	xy[4] = xy[0] + 80 - 1;
	xy[5] = xy[3];
	/* top right */
	xy[6] = xy[4];
	xy[7] = xy[1];
	/* top left, again */
	xy[8] = xy[0];
	xy[9] = xy[1];
	v_pline(vdi_handle, 5, xy);
	v_get_pixel(vdi_handle, 999, 998, &dummy, &dummy);
	}
#endif
}

/*** ---------------------------------------------------------------------- ***/

static void DrawBox(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y)
{
	_WORD xy[4], w, h;
	WP_UNIT tx, ty;

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
	xy[0] = (_WORD) (tx + win->scroll.g_x);
	xy[1] = (_WORD) (ty + win->scroll.g_y);
	xy[2] = xy[0] + w - 1;
	xy[3] = xy[1] + h - 1;
	
	if (gfx->style != 0)
	{
		if (gfx->style > 36)
		{
			vsf_interior(vdi_handle, FIS_SOLID);
		} else if (gfx->style > 24)
		{
			vsf_interior(vdi_handle, FIS_HATCH);
			vsf_style(vdi_handle, gfx->style - 24);
		} else
		{
			vsf_interior(vdi_handle, FIS_PATTERN);
			vsf_style(vdi_handle, gfx->style);
		}
	} else
	{
		vsf_interior(vdi_handle, FIS_HOLLOW);
	}

	vsf_perimeter(vdi_handle, TRUE);
	vsf_color(vdi_handle, viewer_colors.text);
	vsl_color(vdi_handle, viewer_colors.text);
	if (gfx->type == HYP_ESC_BOX)
	{
		v_bar(vdi_handle, xy);
	} else
	{
		if (gfx->style != 0)
			v_rfbox(vdi_handle, xy);
		else
			v_rbox(vdi_handle, xy);
	}
	vsf_interior(vdi_handle, FIS_SOLID);
	vsf_style(vdi_handle, 0);
	vsf_perimeter(vdi_handle, FALSE);
}

/*** ---------------------------------------------------------------------- ***/

static char *pagename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;

	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	return hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), entry->name, namelen, NULL);
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
	
	if (node == NULL)					/* stop if no page loaded */
		return;

	end = node->end;
	
	WindowCalcScroll(win);

	sx = -win->docsize.x;
	sy = -win->docsize.y;

	/* standard text color */
	vst_color(vdi_handle, viewer_colors.text);
	vsl_color(vdi_handle, viewer_colors.text);
	vst_effects(vdi_handle, 0);

#define TEXTOUT(str) \
	{ \
	_WORD ext[8], w; \
	vqt_extent(vdi_handle, str, ext); \
	w = (ext[2] + ext[4]) >> 1; \
	if (x < win->scroll.g_w && (x + w) > 0 && sy < win->scroll.g_h && (sy + win->y_raster) > 0) \
		v_gtext(vdi_handle, (_WORD) (x + win->scroll.g_x), (_WORD)(sy + win->scroll.g_y), str); \
	x += w; \
	}

#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		/* draw remaining text */ \
		char *s; \
		_UWORD len = (_UWORD)(src - textstart); \
		s = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), textstart, len, NULL); \
		TEXTOUT(s); \
		g_free(s); \
	}

	src = node->start;
	textstart = src;
	textattr = 0;
	lineno = 0;
	x = sx;
	sy = draw_graphics(win, node->gfx, lineno, sx, sy);
	vst_effects(vdi_handle, 0);
	vswr_mode(vdi_handle, MD_TRANS);
	
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
					_WORD color;
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
						str = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), src, len, NULL);
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
					vst_color(vdi_handle, color);
					vsl_color(vdi_handle, color);
					vst_effects(vdi_handle, gl_profile.colors.link_effect | textattr);

					TEXTOUT(str);
					g_free(str);

					vst_color(vdi_handle, viewer_colors.text);
					vsl_color(vdi_handle, viewer_colors.text);
					vst_effects(vdi_handle, textattr);
					textstart = src;
				}
				break;
				
			case HYP_ESC_CASE_TEXTATTR:
				textattr = *src - HYP_ESC_TEXTATTR_FIRST;
				vst_effects(vdi_handle, textattr);
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
	
	vst_effects(vdi_handle, 0);
	vswr_mode(vdi_handle, MD_REPLACE);
#undef TEXTOUT
#undef DUMPTEXT
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
	
	sx = sy = 0;

	vst_effects(vdi_handle, 0);

#define TEXTOUT(str) \
	{ \
	_WORD ext[8], w; \
	vqt_extent(vdi_handle, str, ext); \
	w = (ext[2] + ext[4]) >> 1; \
	x += w; \
	}

#define DUMPTEXT() \
	if (src > textstart) \
	{ \
		/* draw remaining text */ \
		char *s; \
		_UWORD len = (_UWORD)(src - textstart); \
		s = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), textstart, len, NULL); \
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
	vst_effects(vdi_handle, 0);

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
						str = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), src, len, NULL);
						src += len;
					}
					
					/* set text effects for link text */
					vst_effects(vdi_handle, gl_profile.colors.link_effect | textattr);

					TEXTOUT(str);
					g_free(str);

					vst_effects(vdi_handle, textattr);
					textstart = src;
				}
				break;
				
			case HYP_ESC_CASE_TEXTATTR:
				textattr = *src - HYP_ESC_TEXTATTR_FIRST;
				vst_effects(vdi_handle, textattr);
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
	
	vst_effects(vdi_handle, 0);

	node->width = max_w;
	node->height = sy;
	win->docsize.w = node->width;
	win->docsize.h = node->height;
#undef TEXTOUT
#undef DUMPTEXT
}
