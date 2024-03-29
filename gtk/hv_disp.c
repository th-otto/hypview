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
#include "hypdebug.h"
#include <math.h>
#include "pattern.h"

struct prep_info {
	PangoTabArray *tab_array;
	int tab_array_size;
	GtkTextMark *linestart;
	GtkTextMark *tagstart;
	GtkTextMark *attrstart;
	GtkTextMark *picstart;
	GtkTextMark *picend;
	GtkTextBuffer *text_buffer;
	GtkTextTagTable *tag_table;
	int last_was_space;
	gboolean ignore_spaces;
	int tab_id;
	int target_link_id;
	int indent_id;
	long lineno;
	WP_UNIT x, maxx;
	WP_UNIT x_raster;
	GtkTextIter iter;
	unsigned char textattr;
	unsigned char fgcolor;
	unsigned char bgcolor;
};

static double const long_dashes[] = { 12, 4 };
static double const dot_dashes[] = { 2, 6, 2, 6 };
static double const dashdot_dashes[] = { 8, 3, 2, 3 };
static double const dash_dashes[] = { 8, 8 };
static double const dash2dot_dashes[] = { 4, 3, 2, 2, 1, 3, 1 };
static double const userline_dashes[] = { 1 };

typedef struct vdi_point {
	int x;
	int y;
} VDI_POINT;

#define LINE_ARROW_INC 10


typedef guint32 cairo_pixel_t;

#define RGBA(r,g,b,a)\
	(((cairo_pixel_t)((unsigned char)(a)) << 24) | \
	 ((cairo_pixel_t)((unsigned char)(r)) << 16) | \
	 ((cairo_pixel_t)((unsigned char)(g)) <<  8) | \
	 ((cairo_pixel_t)((unsigned char)(b))     ))
#define ALPHA_OPAQUE 0xff

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void HypDisplayPage(WINDOW_DATA *win)
{
	GSList *l;
	cairo_t *cr;
	GtkAdjustment *adj;
	gdouble xoffset, yoffset;
	
	if (win->image_childs == NULL)
		return;

	cr = gdk_cairo_create(gtk_text_view_get_window(GTK_TEXT_VIEW(win->text_view), GTK_TEXT_WINDOW_TEXT));
	adj = gtk_text_view_get_vadjustment(GTK_TEXT_VIEW(win->text_view));
	yoffset = gtk_adjustment_get_value(adj);
	adj = gtk_text_view_get_hadjustment(GTK_TEXT_VIEW(win->text_view));
	xoffset = gtk_adjustment_get_value(adj);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	for (l = win->image_childs; l; l = l->next)
	{
		struct hyp_gfx *gfx = (struct hyp_gfx *)l->data;
		int y = gfx->window_y;
		
		if (gfx->type == HYP_ESC_LINE)
		{
			GtkTextIter line;
			GdkRectangle line_pos;
			GtkTextMark *mark;
			char *markname;
			
			markname = g_strdup_printf("hv-line-%u", gfx->y_offset);
			mark = gtk_text_buffer_get_mark(win->text_buffer, markname);
			g_free(markname);
			gtk_text_buffer_get_iter_at_mark(win->text_buffer, &line, mark);
			gtk_text_view_get_iter_location(GTK_TEXT_VIEW(win->text_view), &line, &line_pos);
			y = line_pos.y;
		}
		cairo_set_source_surface(cr, (cairo_surface_t *)gfx->surf, gfx->window_x - xoffset, y - yoffset);
		cairo_paint(cr);
	}
	cairo_destroy(cr);
}

/*** ---------------------------------------------------------------------- ***/

static void destroy_surf(struct hyp_gfx *gfx)
{
	if (gfx && gfx->surf)
	{
		cairo_surface_destroy((cairo_surface_t *)gfx->surf);
		gfx->surf = NULL;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void add_child_window(WINDOW_DATA *win, struct hyp_gfx *gfx)
{
	cairo_status_t status = cairo_surface_status((cairo_surface_t *)gfx->surf);
	
	if (status != CAIRO_STATUS_SUCCESS)
	{
		HYP_DBG(("gfx: %s", cairo_status_to_string(status)));
	}
	gfx->destroy = destroy_surf;
	win->image_childs = g_slist_append(win->image_childs, gfx);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static long DrawPicture(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y, struct prep_info *info, int need_nl)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	HYP_IMAGE *pic;
	WP_UNIT tx;
	
	UNUSED(x);
	pic = (HYP_IMAGE *)AskCache(hyp, gfx->extern_node_index);
	
	if (pic && pic->decompressed)
	{
		/*
		 * pic.fd_addr contains the pixbuf the image was converted to.
		 * see W_Fix_Bitmap()
		 */
		GdkPixbuf *pixbuf = (GdkPixbuf *)pic->pic.fd_addr;

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

		if (gfx->islimage)
		{
			GtkTextIter start;
			
			start = info->iter;
			gtk_text_buffer_move_mark(info->text_buffer, info->picstart, &info->iter);
			if (!need_nl)
				gtk_text_iter_backward_chars(&info->iter, 2);
			gtk_text_buffer_insert_pixbuf(info->text_buffer, &info->iter, pixbuf);
			/*
			 * two newlines, because the St-guide leaves an empty line after each @limage
			 */
			if (need_nl)
			{
				gtk_text_buffer_insert(info->text_buffer, &info->iter, "\n\n", 2);
			} else
			{
				gtk_text_iter_forward_chars(&info->iter, 2);
			}
			y += gfx->pixheight + win->y_raster;
			if (gfx->x_offset == 0)
			{
				gtk_text_buffer_move_mark(info->text_buffer, info->picend, &info->iter);
				gtk_text_buffer_get_iter_at_mark(info->text_buffer, &start, info->picstart);
				if (0)
				{
					gtk_text_buffer_apply_tag_by_name(info->text_buffer, "center", &start, &info->iter);
				} else
				{
					char *tag_name = g_strdup_printf("hv-indent-%d", info->indent_id);
					GtkTextTag *tag = gtk_text_table_create_tag(info->tag_table, tag_name, "indent", tx, NULL);
					gtk_text_buffer_apply_tag(info->text_buffer, tag, &start, &info->iter);
					info->indent_id++;
					g_free(tag_name);
				}
			} else if (((gfx->x_offset - 1) + (gfx->pixwidth / HYP_PIC_FONTW)) == hyp->line_width)
			{
				gtk_text_buffer_move_mark(info->text_buffer, info->picend, &info->iter);
				gtk_text_buffer_get_iter_at_mark(info->text_buffer, &start, info->picstart);
				gtk_text_buffer_apply_tag_by_name(info->text_buffer, "right_justify", &start, &info->iter);
			} else
			{
				char *tag_name = g_strdup_printf("hv-indent-%d", info->indent_id);
				GtkTextTag *tag = gtk_text_table_create_tag(info->tag_table, tag_name, "indent", (gfx->x_offset - 1) * win->x_raster, NULL);
				gtk_text_buffer_move_mark(info->text_buffer, info->picend, &info->iter);
				gtk_text_buffer_get_iter_at_mark(info->text_buffer, &start, info->picstart);
				gtk_text_buffer_apply_tag(info->text_buffer, tag, &start, &info->iter);
				info->indent_id++;
				g_free(tag_name);
			}
		} else
		{
			HYP_NODE *node = win->displayed_node;
			
			gfx->window_margin = 0;
			gfx->window_x = tx;
			gfx->window_y = y;
			gfx->surf = convert_pixbuf_to_cairo(pixbuf);
			add_child_window(win, gfx);
			if (gfx->window_y + 2 * gfx->window_margin + gfx->pixheight > node->height)
				node->height = gfx->window_y + 2 * gfx->window_margin + gfx->pixheight;
		}
	}
	return y;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void set_source_color(cairo_t *cr, const char *colorname)
{
	GdkColor color;
	
	gdk_color_parse(colorname, &color);
	return cairo_set_source_rgba(cr, color.red / 65535.0, color.green / 65535.0, color.blue / 65535.0, 1.0);
}

/*** ---------------------------------------------------------------------- ***/

static cairo_t *gfx_create_surface(struct hyp_gfx *gfx)
{
	cairo_t *cr;
	
	gfx->surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, gfx->pixwidth + gfx->window_margin * 2, gfx->pixheight + gfx->window_margin * 2);
	gfx->destroy = destroy_surf;
	cr = cairo_create((cairo_surface_t *)gfx->surf);
	
	/*
	 * make the surface background transparent
	 */
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	set_source_color(cr, gl_profile.colors.text);
	cairo_set_line_width(cr, 1.0);
	
	return cr;
}

/*** ---------------------------------------------------------------------- ***/

#define vec_len(x, y) (int)sqrt((x) * (x) + (y) * (y))

static inline int SMUL_DIV(int m1, int m2, int d1)
{
	int inc;
	int x;
	int quot, rem;
	
	if (d1 == 0 || d1 == m2)
		return m1;
	inc = 1;
	x = m1 * m2;
	if (x < 0)
		inc = -inc;
	quot = x / d1;
	rem = x % d1;
	if (d1 < 0)
	{
		d1 = -d1;
		inc = -inc;
	}
	if (rem < 0)
		rem = -rem;
	rem += rem;
	if (rem > d1)
		quot += inc;
	return quot;
}

/*** ---------------------------------------------------------------------- ***/

static void arrow(cairo_t *cr, VDI_POINT *xy, int npoints, int inc)
{
	int arrow_len, arrow_wid, line_len;
	int dx, dy;
	int base_x, base_y, ht_x, ht_y;
	int xybeg;
	int i;

	if (npoints <= 1)
		return;
	
	/* Set up the arrow-head length and width as a function of line width. */

	arrow_len = 8;
	arrow_wid = arrow_len / 2;

	/* Find the first point which is not so close to the end point that it */
	/* will be obscured by the arrowhead.                                  */

	xybeg = 0;
	for (i = 1; i < npoints; i++)
	{
		/* Find the deltas between the next point and the end point.  Transform */
		/* to a space such that the aspect ratio is uniform and the x axis      */
		/* distance is preserved.                                               */

		xybeg += inc;
		dx = xy[0].x - xy[xybeg].x;
		dy = xy[0].y - xy[xybeg].y;

		/* Get the length of the vector connecting the point with the end point. */
		/* If the vector is of sufficient length, the search is over.            */

		line_len = vec_len(dx, dy);
		if (line_len >= arrow_len)
			break;
	}

	/* If the longest vector is insufficiently long, don't draw an arrow. */
	if (line_len < arrow_len)
		return;

	/* Rotate the arrow-head height and base vectors.  Perform calculations */
	/* in 1000x space.                                                      */

	ht_x = SMUL_DIV(arrow_len, SMUL_DIV(dx, 1000, line_len), 1000);
	ht_y = SMUL_DIV(arrow_len, SMUL_DIV(dy, 1000, line_len), 1000);
	base_x = SMUL_DIV(arrow_wid, SMUL_DIV(dy, -1000, line_len), 1000);
	base_y = SMUL_DIV(arrow_wid, SMUL_DIV(dx, 1000, line_len), 1000);

	/* draw the polygon */

	cairo_move_to(cr, xy[0].x + base_x - ht_x, xy[0].y + base_y - ht_y);
	cairo_line_to(cr, xy[0].x - base_x - ht_x, xy[0].y - base_y - ht_y);
	cairo_line_to(cr, xy[0].x, xy[0].y);
	cairo_fill(cr);

	/* Adjust the end point and all points skipped. */

	xy[0].x -= ht_x;
	xy[0].y -= ht_y;

	while ((xybeg -= inc) != 0)
	{
		xy[xybeg].x = xy[0].x;
		xy[xybeg].y = xy[0].y;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void do_arrow(cairo_t *cr, VDI_POINT *xy, int npoints, unsigned char line_ends)
{
	int x_start, y_start, new_x_start, new_y_start;

	/* Function "arrow" will alter the end of the line segment.  Save the */
	/* starting point of the polyline in case two calls to "arrow" are    */
	/* necessary.                                                         */
	new_x_start = x_start = xy[0].x;
	new_y_start = y_start = xy[0].y;
	
	if (line_ends & (1 << 0))
	{
		/* draw arrow at start of line */
		arrow(cr, xy, npoints, 1);
		new_x_start = xy[0].x;
		new_y_start = xy[0].y;
	}
	
	if (line_ends & (1 << 1))
	{
		/* draw arrow at end of line */
		xy[0].x = x_start;
		xy[0].y = y_start;
		arrow(cr, xy + npoints - 1, npoints, -1);
		xy[0].x = new_x_start;
		xy[0].y = new_y_start;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void image_draw_line(cairo_t *cr, struct hyp_gfx *gfx)
{
	int x0, y0, x1, y1;
	VDI_POINT xy[2];
	int w = gfx->pixwidth;
	int h = gfx->pixheight;
	
	if (gfx->width < 0)
	{
		/* draw from right to left */
		x0 = w - 1;
		x1 = 0;
	} else
	{
		/* draw from left to right */
		x0 = 0;
		x1 = w - 1;
	}
	if (gfx->height < 0)
	{
		/* draw from bottom to top */
		y0 = h - 1;
		y1 = 0;
	} else
	{
		/* draw from top to bottom */
		y0 = 0;
		y1 = h - 1;
	}
	
	switch (gfx->style)
	{
	default:
	case 1: /* SOLID */
		break;
	case 2: /* LONGDASH */
		cairo_set_dash(cr, long_dashes, 2, 0);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
		break;
	case 3: /* DOT */
		cairo_set_dash(cr, dot_dashes, 4, 0);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
		break;
	case 4: /* DASHDOT */
		cairo_set_dash(cr, dashdot_dashes, 4, 0);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
		break;
	case 5: /* DASH */
		cairo_set_dash(cr, dash_dashes, 2, 0);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
		break;
	case 6: /* DASH2DOT */
		cairo_set_dash(cr, dash2dot_dashes, 7, 0);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
		break;
	case 7: /* USERLINE */
		cairo_set_dash(cr, userline_dashes, 1, 0);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
		break;
	}
	
	xy[0].x = x0 + gfx->window_margin;
	xy[0].y = y0 + gfx->window_margin;
	xy[1].x = x1 + gfx->window_margin;
	xy[1].y = y1 + gfx->window_margin;
	
	cairo_move_to(cr, xy[0].x, xy[0].y);
	cairo_line_to(cr, xy[1].x, xy[1].y);
	cairo_stroke(cr);

	do_arrow(cr, xy, 2, gfx->begend);
}

/*** ---------------------------------------------------------------------- ***/

static int adjust_for_limage(WINDOW_DATA *win, long lineno, int h)
{
	struct hyp_gfx *gfx;
	int diff = 0;
	long end = lineno + h / win->y_raster;
	
	for (gfx = win->displayed_node->gfx; gfx != NULL; gfx = gfx->next)
	{
		if (gfx->type == HYP_ESC_PIC && gfx->islimage && gfx->y_offset >= lineno && gfx->y_offset < end)
		{
			int adj = ((gfx->pixheight + HYP_PIC_FONTH - 1) / HYP_PIC_FONTH);
			if (adj > 0)
			{
				diff += adj * (HYP_PIC_FONTH - win->y_raster);
			}
		}
	}
	return diff;
}

/*** ---------------------------------------------------------------------- ***/

static long DrawLine(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y, struct prep_info *info)
{
	int w = gfx->width * win->x_raster;
	int h = gfx->height * win->y_raster;
	cairo_t *cr;
	long ret = y;
	int diff;
	
	if (w == 0)
	{
		w = 1;
	} else if (w < 0)
	{
		w = -w;
		x -= w;
	}
	if (h == 0)
	{
		h = 1;
	} else if (h < 0)
	{
		h = -h;
		diff = adjust_for_limage(win, info->lineno - h / win->y_raster, h);
		h += diff;
		y -= h;
	} else
	{
		diff = adjust_for_limage(win, info->lineno, h);
		h += diff;
	}
	gfx->pixwidth = w;
	gfx->pixheight = h;
	gfx->window_margin = gfx->begend ? LINE_ARROW_INC : 0;
	gfx->window_x = x + (gfx->x_offset - 1) * win->x_raster - gfx->window_margin;
	gfx->window_y = y - gfx->window_margin;
	cr = gfx_create_surface(gfx);
	/*
	 * move the origin to address the "center" of pixels
	 */
	cairo_translate(cr, 0.5, 0.5);
	image_draw_line(cr, gfx);
	cairo_destroy(cr);
	add_child_window(win, gfx);
	return ret;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * Integer sine and cosine functions.
 */

#define I_HALFPI	900 
#define I_PI	1800
#define I_TWOPI	3600

/* Sines of angles 1 - 90 degrees normalized between 0 and 32767. */

static short const sin_tbl[92] = {    
		    0,   572,  1144,  1716,  2286,  2856,  3425,  3993, 
		 4560,  5126,  5690,  6252,  6813,  7371,  7927,  8481, 
		 9032,  9580, 10126, 10668, 11207, 11743, 12275, 12803,
		13328, 13848, 14364, 14876, 15383, 15886, 16383, 16876,
		17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621,
		21062, 21497, 21925, 22347, 22762, 23170, 23571, 23964,
		24351, 24730, 25101, 25465, 25821, 26169, 26509, 26841,
		27165, 27481, 27788, 28087, 28377, 28659, 28932, 29196,
		29451, 29697, 29934, 30162, 30381, 30591, 30791, 30982,
		31163, 31335, 31498, 31650, 31794, 31927, 32051, 32165,
		32269, 32364, 32448, 32523, 32587, 32642, 32687, 32722,
		32747, 32762, 32767, 32767 
		};

/*
 * Returns integer sin between -32767 and 32767.
 * Uses integer lookup table sintable^[].
 * Expects angle in tenths of degree 0 - 3600.
 * Assumes positive angles only.
 */
static short Isin(unsigned short angle) 
{
	short index;
	unsigned short remainder, tmpsin;	/* Holder for sin. */
	short  half;			/* 0-1 = 1st/2nd, 3rd/4th. */
	const short *table;

	half = 0;
	while (angle >= I_PI)
	{
		half ^= 1;
		angle -= I_PI;
	}
	if (angle >= I_HALFPI)
		angle = I_PI - angle;	

	index = angle / 10;
	remainder = angle % 10;
	table = &sin_tbl[index];
	tmpsin = *table++;
	if (remainder)		/* Add interpolation. */
		tmpsin += (unsigned short)((unsigned short)(*table - tmpsin) * remainder) / 10;

	if (half > 0)
		return -tmpsin;
	else
		return tmpsin;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Return integer cos between -32767 and 32767.
 */
static short Icos(short angle) 
{
	return Isin(angle + I_HALFPI);
}

/*** ---------------------------------------------------------------------- ***/

static void rounded_box(cairo_t *cr, struct hyp_gfx *gfx)
{
	int rdeltax, rdeltay;
	int xc, yc, xrad, yrad;
	int x1, y1, x2, y2;
	int i, j;
	VDI_POINT points[21];
	
	x1 = gfx->window_margin;
	y1 = gfx->window_margin;
	x2 = x1 + gfx->pixwidth - 1;
	y2 = y1 + gfx->pixheight - 1;

	rdeltax = (x2 - x1) / 2;
	rdeltay = (y2 - y1) / 2;

	xrad = 10;
	if (xrad > rdeltax)
	    xrad = rdeltax;

	yrad = xrad;
	if (yrad > rdeltay)
	    yrad = rdeltay;
	yrad = -yrad;

	for (i = 0; i < 5; i++)
	{
		points[i].x = SMUL_DIV(Icos(900 - 225 * i), xrad, 32767);
		points[i].y = SMUL_DIV(Isin(900 - 225 * i), yrad, 32767);
	}

	xc = x2 - xrad;
	yc = y1 - yrad;
	for (i = 4, j = 5; i >= 0; i--, j++)
	{
	    points[j].x = xc + points[i].x;
	    points[j].y = yc + points[i].y;
	}
	xc = x1 + xrad; 
	for (i = 0, j = 10; i < 5; i++, j++)
	{
	    points[j].x = xc - points[i].x;
	    points[j].y = yc + points[i].y;
	}
	yc = y2 + yrad;
	for (i = 4, j = 15; i >= 0; i--, j++)
	{ 
	    points[j].x = xc - points[i].x;
	    points[j].y = yc - points[i].y;
	}
	xc = x2 - xrad;
	for (i = 0, j = 0; i < 5; i++, j++)
	{ 
		points[j].x = xc + points[i].x;
		points[j].y = yc - points[i].y;
	}
	points[20].x = points[0].x;
	points[20].y = points[0].y;
	
	cairo_move_to(cr, points[0].x, points[0].y);
	for (i = 0; i < 21; i++)
		cairo_line_to(cr, points[i].x, points[i].y);
}

/*** ---------------------------------------------------------------------- ***/

static cairo_surface_t *create_pattern(int w, int h, const unsigned char *pattern_bits, unsigned char **pdata, const char *colorname)
{
	int x, y;
	int px, py;
	unsigned char *data;
	int dst_stride;
	cairo_format_t format = CAIRO_FORMAT_ARGB32;
	cairo_surface_t *surf;
	unsigned char r, g, b, a;
	const unsigned char *pbits;
	cairo_pixel_t *dst;
	GdkColor color;
	
	gdk_color_parse(colorname, &color);
	r = color.red >> 8;
	g = color.green >> 8;
	b = color.blue >> 8;
	a = ALPHA_OPAQUE;
	
	dst_stride = cairo_format_stride_for_width(format, w);
	data = g_new(unsigned char, h * dst_stride);

	py = 0;
	pbits = pattern_bits;
	for (y = 0; y < h; y++)
	{
		dst = (cairo_pixel_t *)(data + y * dst_stride);
		px = 0;
		for (x = 0; x < w; x++)
		{
			if (pbits[(px >> 3) & 1] & (0x80 >> (px & 7)))
				*dst = RGBA(r, g, b, a);
			else
				*dst = RGBA(0xff, 0xff, 0xff, 0x00);
			px++;
			dst++;
		}
		pbits += PATTERN_WIDTH / 8;
		py++;
		if (py == PATTERN_HEIGHT)
		{
			py = 0;
			pbits = pattern_bits;
		}
	}
	surf = cairo_image_surface_create_for_data(data, format, w, h, dst_stride);
	*pdata = data;
	return surf;
}

/*** ---------------------------------------------------------------------- ***/

static long DrawBox(WINDOW_DATA *win, struct hyp_gfx *gfx, long x, long y, struct prep_info *info)
{
	int w = gfx->width * win->x_raster;
	int h = gfx->height * win->y_raster;
	cairo_t *cr;
	long ret = y;

	h += adjust_for_limage(win, info->lineno, h);
	gfx->pixwidth = w;
	gfx->pixheight = h;
	gfx->window_margin = 0;
	gfx->window_x = x + (gfx->x_offset - 1) * win->x_raster - gfx->window_margin;
	gfx->window_y = y - gfx->window_margin;

	cr = gfx_create_surface(gfx);

	/*
	 * move the origin to address the "center" of pixels
	 */
	cairo_translate(cr, 0.5, 0.5);
	if (gfx->type == HYP_ESC_BOX)
	{
		cairo_rectangle(cr, gfx->window_margin, gfx->window_margin, w - 1, h - 1);
	} else
	{
		rounded_box(cr, gfx);
	}
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	if (gfx->style != 0)
	{
		if ((gfx->style >= 1 && gfx->style <= 7) || /* IP_1PATT..IP_7PATT */
			/* 8 is solid */
			(gfx->style >= 9 && gfx->style <= 36))
		{
			unsigned char *data = NULL;
			cairo_surface_t *pattern;
			
			cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
			pattern = create_pattern(w, h, &pattern_bits[gfx->style * PATTERN_SIZE], &data, gl_profile.colors.text);
			cairo_set_source_surface(cr, pattern, gfx->window_margin + 0.5, gfx->window_margin + 0.5);
			cairo_fill_preserve(cr);
			cairo_surface_destroy(pattern);
			g_free(data);
		} else
		{
			set_source_color(cr, gl_profile.colors.text);
			cairo_fill_preserve(cr);
		}
	}
	if (gfx->style != 8)
	{
		set_source_color(cr, gl_profile.colors.text);
		cairo_stroke(cr);
	}
	
	cairo_destroy(cr);
	add_child_window(win, gfx);
	return ret;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static char *pagename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;

	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	return hyp_conv_charset(hyp->comp_charset, HYP_CHARSET_UTF8, entry->name, namelen, NULL);
}

/*** ---------------------------------------------------------------------- ***/

static long draw_graphics(WINDOW_DATA *win, WP_UNIT sx, WP_UNIT sy, struct prep_info *info)
{
	struct hyp_gfx *gfx;
	HYP_NODE *node = win->displayed_node;
	WP_UNIT y, max_y;
	int need_nl = TRUE;

	max_y = sy;
	for (gfx = node->gfx; gfx != NULL; gfx = gfx->next)
	{
		if (gfx->y_offset == info->lineno)
		{
			switch (gfx->type)
			{
			case HYP_ESC_PIC:
				y = DrawPicture(win, gfx, sx, sy, info, need_nl);
				need_nl = !gfx->islimage;
				break;
			case HYP_ESC_LINE:
				y = DrawLine(win, gfx, sx, sy, info);
				if (gfx->window_y + 2 * gfx->window_margin + gfx->pixheight > node->height)
				{
					node->height = gfx->window_y + 2 * gfx->window_margin + gfx->pixheight;
				}
				need_nl = TRUE;
				break;
			case HYP_ESC_BOX:
			case HYP_ESC_RBOX:
				y = DrawBox(win, gfx, sx, sy, info);
				if (gfx->window_y + 2 * gfx->window_margin + gfx->pixheight > node->height)
					node->height = gfx->window_y + 2 * gfx->window_margin + gfx->pixheight;
				need_nl = TRUE;
				break;
			default:
				y = sy;
				break;
			}
			if (y > max_y)
				max_y = y;
		}
	}
	return max_y;
}

/*** ---------------------------------------------------------------------- ***/

#if 0 /* for debugging */
static void print_text_and_tabs(WINDOW_DATA *win, GtkTextIter *start, GtkTextIter *end, PangoTabArray *tab_array)
{
	char *line;
	int i, j, size;
	int x;
	int tabnum;
	
	line = gtk_text_buffer_get_text(win->text_buffer, start, end, FALSE);
	size = pango_tab_array_get_size(tab_array);
	x = 0;
	i = 0;
	tabnum = 0;
	for (i = 0; line[i]; i++)
	{
		if (line[i] == '\t')
		{
			int pos;
			pango_tab_array_get_tab(tab_array, tabnum, NULL, &pos);
			tabnum++;
			pos /= win->x_raster;
			for (j = x; j < pos; j++)
				putchar(' ');
			x = pos;
		} else
		{
			putchar(line[i]);
		}
		x++;
	}
	printf("\n");
	x = 0;
	for (i = 0; i < size; i++)
	{
		int pos;
		pango_tab_array_get_tab(tab_array, i, NULL, &pos);
		pos /= win->x_raster;
		for (j = x; j < pos; j++)
			putchar(' ');
		putchar('^');
		x = pos + 1;
	}
	printf("\n");
	g_free(line);
}
#endif

/*** ---------------------------------------------------------------------- ***/

static void info_flush_spaces(struct prep_info *info)
{
	int i;
	
	if (info->last_was_space > 1)
	{
		gtk_text_buffer_insert(info->text_buffer, &info->iter, "\t", 1);
		pango_tab_array_set_tab(info->tab_array, info->tab_array_size, PANGO_TAB_LEFT, info->x * info->x_raster);
		info->tab_array_size++;
		for (i = 0; i < info->last_was_space; i++)
			gtk_text_buffer_insert(info->text_buffer, &info->iter, "\342\200\213", 3); /* zero width space */
	} else if (info->last_was_space == 1)
	{
		gtk_text_buffer_insert(info->text_buffer, &info->iter, " ", 1);
	}
	info->last_was_space = 0;
	info->ignore_spaces = FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static GtkTextTag *insert_str(struct prep_info *info, const char *str, const char *tag, gboolean flush_spaces)
{
	GtkTextTag *target_tag = NULL;
	
	if (tag)
	{
		info_flush_spaces(info);
		gtk_text_buffer_move_mark(info->text_buffer, info->tagstart, &info->iter);
	}
		
	if (gl_profile.viewer.expand_spaces)
	{
		const char *scan = str;
		const char *next;
		while (*scan)
		{
			next = g_utf8_skipchar(scan);
			if (info->last_was_space)
			{
				if (*scan != ' ' && *scan != '\t')
				{
					info_flush_spaces(info);
					gtk_text_buffer_insert(info->text_buffer, &info->iter, scan, next - scan);
				} else
				{
					info->last_was_space++;
				}
			} else if (*scan == ' ' || *scan == '\t')
			{
				if (info->ignore_spaces)
					gtk_text_buffer_insert(info->text_buffer, &info->iter, scan, next - scan);
				else
					info->last_was_space++;
			} else
			{
				gtk_text_buffer_insert(info->text_buffer, &info->iter, scan, next - scan);
				info->last_was_space = 0;
				info->ignore_spaces = FALSE;
				if (*scan == '.')
					info->ignore_spaces = TRUE;
			}
			scan = next;
			info->x++;
		}
		if (flush_spaces)
			info_flush_spaces(info);
	} else
	{ 
		gtk_text_buffer_insert(info->text_buffer, &info->iter, str, -1);
	}
	
	if (tag)
	{
		GtkTextIter tagstart_iter, tagend_iter;
		char *target_name;
		
		gtk_text_buffer_get_iter_at_mark(info->text_buffer, &tagstart_iter, info->tagstart);
		tagend_iter = info->iter;
		gtk_text_buffer_apply_tag_by_name(info->text_buffer, tag, &tagstart_iter, &tagend_iter);
		target_name = g_strdup_printf("hv-link-%d", info->target_link_id);
		target_tag = gtk_text_buffer_create_tag(info->text_buffer, target_name, NULL);
		gtk_text_buffer_apply_tag(info->text_buffer, target_tag, &tagstart_iter, &tagend_iter);
		info->target_link_id++;
		g_free(target_name);
	}

	if (info->textattr)
	{
		GtkTextIter tagstart_iter, tagend_iter;
		
		gtk_text_buffer_get_iter_at_mark(info->text_buffer, &tagstart_iter, info->attrstart);
		tagend_iter = info->iter;
		if (info->textattr & HYP_TXT_BOLD)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bold", &tagstart_iter, &tagend_iter);
		if (info->textattr & HYP_TXT_LIGHT)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "ghosted", &tagstart_iter, &tagend_iter);
		if (info->textattr & HYP_TXT_ITALIC)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "italic", &tagstart_iter, &tagend_iter);
		if (info->textattr & HYP_TXT_UNDERLINED)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "underlined", &tagstart_iter, &tagend_iter);
		if (info->textattr & HYP_TXT_SHADOWED)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "shadowed", &tagstart_iter, &tagend_iter);
		if (info->textattr & HYP_TXT_OUTLINED)
			gtk_text_buffer_apply_tag_by_name(info->text_buffer, "outlined", &tagstart_iter, &tagend_iter);
	}

	if (info->fgcolor != HYP_DEFAULT_FG)
	{
		GtkTextIter tagstart_iter, tagend_iter;
		
		gtk_text_buffer_get_iter_at_mark(info->text_buffer, &tagstart_iter, info->attrstart);
		tagend_iter = info->iter;
		switch (info->fgcolor)
		{
			case HYP_COLOR_WHITE: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_white", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_BLACK: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_black", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_RED: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_red", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_GREEN: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_green", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_BLUE: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_blue", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_CYAN: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_cyan", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_YELLOW: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_yellow", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_MAGENTA: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_magenta", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_LGRAY: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_lgray", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DGRAY: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_dgray", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DRED: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_dred", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DGREEN: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_dgreen", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DBLUE: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_dblue", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DCYAN: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_dcyan", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DYELLOW: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_dyellow", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DMAGENTA: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "fgcolor_dmagenta", &tagstart_iter, &tagend_iter); break;
		}
	}

	if (info->bgcolor != HYP_DEFAULT_BG)
	{
		GtkTextIter tagstart_iter, tagend_iter;
		
		gtk_text_buffer_get_iter_at_mark(info->text_buffer, &tagstart_iter, info->attrstart);
		tagend_iter = info->iter;
		switch (info->bgcolor)
		{
			case HYP_COLOR_WHITE: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_white", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_BLACK: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_black", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_RED: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_red", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_GREEN: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_green", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_BLUE: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_blue", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_CYAN: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_cyan", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_YELLOW: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_yellow", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_MAGENTA: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_magenta", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_LGRAY: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_lgray", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DGRAY: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_dgray", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DRED: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_dred", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DGREEN: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_dgreen", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DBLUE: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_dblue", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DCYAN: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_dcyan", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DYELLOW: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_dyellow", &tagstart_iter, &tagend_iter); break;
			case HYP_COLOR_DMAGENTA: gtk_text_buffer_apply_tag_by_name(info->text_buffer, "bgcolor_dmagenta", &tagstart_iter, &tagend_iter); break;
		}
	}

	return target_tag;
}

/*** ---------------------------------------------------------------------- ***/

HYP_NODE *hypwin_node(WINDOW_DATA *win)
{
	return win->displayed_node;
}

/*** ---------------------------------------------------------------------- ***/

void hv_win_reset_text(WINDOW_DATA *win)
{
	GtkTextBuffer *text_buffer = win->text_buffer;
	GtkTextIter iter;
	GtkTextMark *mark;
	int id;
	GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(text_buffer);
	
	gtk_text_buffer_set_text(text_buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset(text_buffer, &iter, 0);

	mark = gtk_text_buffer_get_mark(text_buffer, "hv-linestart");
	if (mark == NULL)
		mark = gtk_text_buffer_create_mark(text_buffer, "hv-linestart", &iter, TRUE);
	gtk_text_buffer_move_mark(text_buffer, mark, &iter);
	mark = gtk_text_buffer_get_mark(text_buffer, "hv-tagstart");
	if (mark == NULL)
		mark = gtk_text_buffer_create_mark(text_buffer, "hv-tagstart", &iter, TRUE);
	gtk_text_buffer_move_mark(text_buffer, mark, &iter);
	mark = gtk_text_buffer_get_mark(text_buffer, "hv-attrstart");
	if (mark == NULL)
		mark = gtk_text_buffer_create_mark(text_buffer, "hv-attrstart", &iter, TRUE);
	gtk_text_buffer_move_mark(text_buffer, mark, &iter);
	mark = gtk_text_buffer_get_mark(text_buffer, "hv-curlink");
	if (mark == NULL)
		mark = gtk_text_buffer_create_mark(text_buffer, "hv-curlink", &iter, TRUE);
	else
		gtk_text_buffer_move_mark(text_buffer, mark, &iter);
	win->curlink_mark = mark;
	mark = gtk_text_buffer_get_mark(text_buffer, "hv-picstart");
	if (mark == NULL)
		mark = gtk_text_buffer_create_mark(text_buffer, "hv-picstart", &iter, TRUE);
	mark = gtk_text_buffer_get_mark(text_buffer, "hv-picend");
	if (mark == NULL)
		mark = gtk_text_buffer_create_mark(text_buffer, "hv-picend", &iter, FALSE);

	/*
	 * remove old tabulator tags
	 */
	id = 0;
	for (;;)
	{
		char *tag_name = g_strdup_printf("hv-tabtag-%d", id);
		GtkTextTag *tag = gtk_text_tag_table_lookup(tag_table, tag_name);
		g_free(tag_name);
		if (tag == 0)
			break;
		gtk_text_tag_table_remove(tag_table, tag);
		id++;
	}

	/*
	 * remove old target links
	 */
	id = 0;
	for (;;)
	{
		char *tag_name = g_strdup_printf("hv-link-%d", id);
		GtkTextTag *tag = gtk_text_tag_table_lookup(tag_table, tag_name);
		g_free(tag_name);
		if (tag == 0)
			break;
		gtk_text_tag_table_remove(tag_table, tag);
		id++;
	}
	
	/*
	 * remove old indent tags
	 */
	id = 0;
	for (;;)
	{
		char *tag_name = g_strdup_printf("hv-indent-%d", id);
		GtkTextTag *tag = gtk_text_tag_table_lookup(tag_table, tag_name);
		g_free(tag_name);
		if (tag == 0)
			break;
		gtk_text_tag_table_remove(tag_table, tag);
		id++;
	}
	
	/*
	 * remove old images
	 */
	hv_win_destroy_images(win);
}

/*** ---------------------------------------------------------------------- ***/

void HypPrepNode(WINDOW_DATA *win, HYP_NODE *node)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	const unsigned char *src, *end, *textstart;
	WP_UNIT sx, sy;
	gboolean at_bol;
	struct prep_info info;
	char *markname;
	long lastlineno;

#define DUMPTEXT(flush_spaces) \
	if (src > textstart) \
	{ \
		char *s; \
		size_t len = src - textstart; \
		/* draw remaining text */ \
		s = hyp_conv_charset(hyp->comp_charset, HYP_CHARSET_UTF8, textstart, len, NULL); \
		insert_str(&info, s, NULL, flush_spaces); \
		g_free(s); \
		at_bol = FALSE; \
	}

	win->displayed_node = node;
	
	RemoveSearchBox(win);
	ToolbarUpdate(win, FALSE);
	
	end = node->end;
	src = node->start;

	info.text_buffer = win->text_buffer;
	info.x_raster = win->x_raster;
	info.tab_id = 0;
	info.tag_table = gtk_text_buffer_get_tag_table(info.text_buffer);
	info.target_link_id = 0;
	info.indent_id = 0;
	info.textattr = 0;
	info.fgcolor = HYP_DEFAULT_FG;
	info.bgcolor = HYP_DEFAULT_BG;
	info.lineno = 0;
	info.x = info.maxx = sx = sy = 0;
	
	info.last_was_space = 0;
	info.ignore_spaces = FALSE;
	
	info.tab_array_size = 0;
	info.tab_array = pango_tab_array_new(info.tab_array_size, TRUE);
	
	/*
	 * clear buffer
	 */
	hv_win_reset_text(win);
	gtk_text_buffer_get_iter_at_offset(info.text_buffer, &info.iter, 0);

	info.linestart = gtk_text_buffer_get_mark(info.text_buffer, "hv-linestart");
	info.tagstart = gtk_text_buffer_get_mark(info.text_buffer, "hv-tagstart");
	info.attrstart = gtk_text_buffer_get_mark(info.text_buffer, "hv-attrstart");
	info.picstart = gtk_text_buffer_get_mark(info.text_buffer, "hv-picstart");
	info.picend = gtk_text_buffer_get_mark(info.text_buffer, "hv-picend");

	textstart = src;
	at_bol = TRUE;
	node->height = 0;
	markname = g_strdup_printf("hv-line-%ld", info.lineno);
	gtk_text_buffer_create_mark(info.text_buffer, markname, &info.iter, TRUE);
	g_free(markname);

	sy = draw_graphics(win, sx, sy, &info);
	
	while (src < end)
	{
		if (*src == HYP_ESC)		/* ESC-sequence */
		{
			/* unwritten data? */
			DUMPTEXT(TRUE);
			src++;
			
			gtk_text_buffer_move_mark(info.text_buffer, info.attrstart, &info.iter);

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
					unsigned char link_type = *src;	/* remember link type */
					hyp_lineno line_nr = 0;	/* line number to go to */
					hyp_nodenr dest_page;	/* Index of destination page */
					char *str;
					size_t len;
					unsigned short link_len;
					const char *tagtype = "link";
					hyp_indextype dst_type = HYP_NODE_EOF;
					char *tip = NULL;
					LINK_INFO *linkinfo;
					GtkTextTag *target_tag;
					
					src++;
					
					if (link_type == HYP_ESC_LINK_LINE || link_type == HYP_ESC_ALINK_LINE)	/* skip destination line number */
					{
						line_nr = DEC_255(src);
						src += 2;
					}
					
					dest_page = DEC_255(src);
					src += 2;

					link_len = *src;
					src++;
					
					/* get link text for output */
					if (link_len <= HYP_STRLEN_OFFSET)	/* no text in link: use nodename */
					{
						if (hypnode_valid(hyp, dest_page))
						{
							str = pagename(hyp, dest_page);
						} else
						{
							str = hyp_invalid_page(dest_page);
						}
						len = strlen(str);
					} else
					{
						len = link_len - HYP_STRLEN_OFFSET;
						str = hyp_conv_charset(hyp->comp_charset, HYP_CHARSET_UTF8, src, len, NULL);
						src += len;
					}

					if (hypnode_valid(hyp, dest_page))
					{
						dst_type = (hyp_indextype)hyp->indextable[dest_page]->type;
						tip = pagename(hyp, dest_page);
						switch (dst_type)
						{
						case HYP_NODE_INTERNAL:
							tagtype = "link";
							break;
						case HYP_NODE_POPUP:
							tagtype = "popup";
							break;
						case HYP_NODE_EXTERNAL_REF:
							tagtype = "xref";
							break;
						case HYP_NODE_REXX_COMMAND:
							tagtype = "rx";
							break;
						case HYP_NODE_REXX_SCRIPT:
							tagtype = "rxs";
							break;
						case HYP_NODE_QUIT:
							tagtype = "quit";
							break;
						case HYP_NODE_CLOSE:
							tagtype = "close";
							break;
						case HYP_NODE_SYSTEM_ARGUMENT:
							tagtype = "system";
							break;
						case HYP_NODE_IMAGE:
						case HYP_NODE_EOF:
						default:
							tagtype = "error";
							g_free(tip);
							tip = g_strdup_printf(_("Link to node of type %u not implemented."), dst_type);
							break;
						}
					} else
					{
						tip = hyp_invalid_page(dest_page);
					}
					
					target_tag = insert_str(&info, str, tagtype, FALSE);
					g_free(str);
					
					linkinfo = g_new(LINK_INFO, 1);
					linkinfo->link_type = link_type;
					linkinfo->dst_type = dst_type;
					linkinfo->tip = tip;
					linkinfo->dest_page = dest_page;
					linkinfo->line_nr = line_nr;
					linkinfo->window_id = 0;
					g_object_set_data(G_OBJECT(target_tag), "hv-linkinfo", linkinfo);
					
					textstart = src;
				}
				break;
				
			case HYP_ESC_CASE_TEXTATTR:
				info.textattr = *src - HYP_ESC_TEXTATTR_FIRST;
				src++;
				textstart = src;
				break;
			
			case HYP_ESC_FG_COLOR:
				src++;
				info.fgcolor = *src;
				src++;
				textstart = src;
				break;
			
			case HYP_ESC_BG_COLOR:
				src++;
				info.bgcolor = *src;
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
			case HYP_ESC_ATTR_TYPEWRITER:
			default:
				src = hyp_skip_esc(--src);
				textstart = src;
				break;
			}
		} else if (*src == HYP_EOL)
		{
			DUMPTEXT(TRUE);
			if (info.tab_array_size > 0)
			{
				char *tag_name = g_strdup_printf("hv-tabtag-%d", info.tab_id);
				GtkTextIter start;
				GtkTextTag *tag = gtk_text_tag_new(tag_name);
				
				g_free(tag_name);
				info.tab_id++;
				g_object_set(G_OBJECT(tag), "tabs", info.tab_array, NULL);
				gtk_text_tag_table_add(info.tag_table, tag);
				gtk_text_buffer_get_iter_at_mark(info.text_buffer, &start, info.linestart);
				
				/* print_text_and_tabs(win, &start, &info.iter, info.tab_array); */
				
				gtk_text_buffer_apply_tag(info.text_buffer, tag, &start, &info.iter);
				info.tab_array_size = 0;
				info.tab_array = pango_tab_array_new(info.tab_array_size, TRUE);
			}
			++info.lineno;
			src++;
			textstart = src;
			info.last_was_space = 0;
			info.ignore_spaces = FALSE;
			if (info.x > info.maxx)
				info.maxx = info.x;
			info.x = sx;
			at_bol = TRUE;
			gtk_text_buffer_insert(info.text_buffer, &info.iter, "\n", 1);
			gtk_text_buffer_move_mark(info.text_buffer, info.linestart, &info.iter);
			markname = g_strdup_printf("hv-line-%ld", info.lineno);
			gtk_text_buffer_create_mark(info.text_buffer, markname, &info.iter, TRUE);
			g_free(markname);
			sy += win->y_raster;
			sy = draw_graphics(win, sx, sy, &info);
		} else
		{
			src++;
		}
	}
	DUMPTEXT(FALSE);
	if (!at_bol)
	{
		if (info.tab_array_size > 0)
		{
			char *tag_name = g_strdup_printf("hv-tabtag-%d", info.tab_id);
			GtkTextIter start;
			GtkTextTag *tag = gtk_text_tag_new(tag_name);
			
			g_free(tag_name);
			info.tab_id++;
			g_object_set(G_OBJECT(tag), "tabs", info.tab_array, NULL);
			gtk_text_tag_table_add(info.tag_table, tag);
			gtk_text_buffer_get_iter_at_mark(info.text_buffer, &start, info.linestart);
			gtk_text_buffer_apply_tag(info.text_buffer, tag, &start, &info.iter);
			info.tab_array_size = 0;
			info.tab_array = NULL;
		}
		if (info.x > info.maxx)
			info.maxx = info.x;
		++info.lineno;
		gtk_text_buffer_move_mark(info.text_buffer, info.linestart, &info.iter);
		markname = g_strdup_printf("hv-line-%ld", info.lineno);
		gtk_text_buffer_create_mark(info.text_buffer, markname, &info.iter, TRUE);
		g_free(markname);
		sy += win->y_raster;
		sy = draw_graphics(win, sx, sy, &info);
	}
	if (info.tab_array)
		pango_tab_array_free(info.tab_array);

	if (sy > node->height)
		node->height = sy;
	node->width = info.maxx * win->x_raster;
	
	/*
	 * remove trailing empty lines from textbuffer,
	 * unless there are graphics attached to it
	 */
	lastlineno = 0;
	{
		struct hyp_gfx *gfx;
		for (gfx = node->gfx; gfx != NULL; gfx = gfx->next)
			if (gfx->y_offset > lastlineno)
				lastlineno = gfx->y_offset;
	}
	while (info.lineno > lastlineno)
	{
		GtkTextIter start, end;
		char *txt;
		
		gtk_text_buffer_get_end_iter(info.text_buffer, &end);
		start = end;
		if (!gtk_text_iter_backward_char(&start))
			break;
		txt = gtk_text_buffer_get_text(info.text_buffer, &start, &end, FALSE);
		if (txt == NULL)
			break;
		if (*txt == '\0')
		{
			g_free(txt);
			break;
		}
		{
			size_t len = strlen(txt) - 1;
			char *lineend = txt + len;
			gboolean eol = *lineend == '\n';
			g_free(txt);
			if (!eol)
				break;
			gtk_text_buffer_delete(info.text_buffer, &start, &end);
			info.lineno--;
			if (len != 0)
				break;
		}
	}

	/*
	 * if there are any floating images who extend beyond the end
	 * of the last text line, add enough newlines to the textbuffer so
	 * we can scroll to them
	 */
	if (node->height > sy)
	{
		GtkTextIter end;
		int diff = node->height - sy;
		int i;
		
		diff = node->height - sy;
		diff = (diff + win->y_raster - 1) / win->y_raster;
		gtk_text_buffer_get_end_iter(win->text_buffer, &end);
		for (i = 0; i < diff; i++)
			gtk_text_buffer_insert(win->text_buffer, &end, "\n", 1);
	}
	
	g_free(win->title);
	if (node->window_title)
		win->title = hyp_conv_to_utf8(hyp->comp_charset, node->window_title, STR0TERM);
	else
		win->title = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[node->number]->name, STR0TERM);
}
