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
	HYP_DOCUMENT *hyp = doc->data;
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
			/* maybe FIXME: ST-Guide seems to leave an empty line below the image */
			y += gfx->pixheight;
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

	vsl_ends(vdi_handle, gfx->attr & ARROWED, (gfx->attr >> 1) & ARROWED);
	vsl_type(vdi_handle, gfx->style);
	v_pline(vdi_handle, 2, xy);
	vsl_ends(vdi_handle, 0, 0);
	vsl_type(vdi_handle, SOLID);

#if 1
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
		vsf_interior(vdi_handle, FIS_PATTERN);
		vsf_style(vdi_handle, gfx->style);
	} else
	{
		vsf_interior(vdi_handle, FIS_HOLLOW);
	}

	vsf_perimeter(vdi_handle, TRUE);
	if (gfx->type == HYP_ESC_BOX)
	{
		v_bar(vdi_handle, xy);
	} else
	{
		v_rfbox(vdi_handle, xy);
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

static char *invalid_page(hyp_nodenr page)
{
	return g_strdup_printf(_("<invalid destination page %u>"), page);
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

void HypDisplayPage(DOCUMENT *doc)
{
	WINDOW_DATA *win = doc->window;
	HYP_DOCUMENT *hyp = doc->data;
	HYP_NODE *node = doc->displayed_node;
	_UWORD len;
	long lineno;
	WP_UNIT x, sx, sy;
	const unsigned char *src, *end, *textstart;
	unsigned char textattr;
	char *str;
	
	if (node == NULL)					/* stop if no page loaded */
		return;

	end = node->end;
	
	WindowCalcScroll(win);

	sx = -win->docsize.x * win->x_raster;
	sy = -win->docsize.y * win->y_raster;

	/* standard text color */
	vst_color(vdi_handle, gl_profile.viewer.text_color);
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
		len = (_UWORD)(src - textstart); \
		s = hyp_conv_charset(hyp->comp_os, hyp_get_current_charset(), textstart, len, NULL); \
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
					
					if (*src == HYP_ESC_LINK_LINE || *src == HYP_ESC_ALINK_LINE)	/* skip destination line number */
						src += 2;

					dest_page = DEC_255(&src[1]);
					src += 3;

					/* set text effects for link text */
					vst_color(vdi_handle, gl_profile.viewer.link_color);
					vst_effects(vdi_handle, gl_profile.viewer.link_effect | textattr);

					/* get link text for output */
					if (*src <= HYP_STRLEN_OFFSET)	/* no text in link: use nodename */
					{
						if (hypnode_valid(hyp, dest_page))
						{
							str = pagename(hyp, dest_page);
						} else
						{
							str = invalid_page(dest_page);
						}
						len = (_UWORD)strlen(str);
						src++;
					} else
					{
						len = *src - HYP_STRLEN_OFFSET;
						src++;
						str = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), src, len, NULL);
						src += len;
					}
					TEXTOUT(str);
					g_free(str);

					vst_color(vdi_handle, gl_profile.viewer.text_color);
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
	
	/*
	 * clear out the margins again
	 * (at least at the top and left);
	 * they may have been overwritten by starting a
	 * drawing operation outside of a scrolled window
	 */
	if (win->x_margin_left != 0)
	{
		_WORD pxy[4];
		
		pxy[0] = win->scroll.g_x - win->x_margin_left;
		pxy[1] = win->scroll.g_y - win->y_margin_top;
		pxy[2] = win->scroll.g_x - 1;
		pxy[3] = win->scroll.g_y + win->scroll.g_h + win->y_margin_bottom - 1;
		vsf_color(vdi_handle, gl_profile.viewer.background_color);
		vsf_interior(vdi_handle, FIS_SOLID);
		vr_recfl(vdi_handle, pxy);
	}
	if (win->y_margin_top != 0)
	{
		_WORD pxy[4];
		
		pxy[0] = win->scroll.g_x - win->x_margin_left;
		pxy[1] = win->scroll.g_y - win->y_margin_top;
		pxy[2] = win->scroll.g_x + win->scroll.g_w + win->x_margin_right - 1;
		pxy[3] = win->scroll.g_y - 1;
		vsf_color(vdi_handle, gl_profile.viewer.background_color);
		vsf_interior(vdi_handle, FIS_SOLID);
		vr_recfl(vdi_handle, pxy);
	}
}

/*** ---------------------------------------------------------------------- ***/

void HypPrepNode(DOCUMENT *doc)
{
	/* nothing to do */
	UNUSED(doc);
}
