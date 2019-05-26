#include "vdi.h"
#include <stdio.h>
#include "debug.h"
#include "vdimaps.h"
#include "gemdos.h"
#include "ro_mem.h"
#include <math.h>
#include "maptab.h"
#include "pattern.h"
#include "writepng.h"
#include "s_endian.h"
#include <errno.h>

/*
 * our "screen" format
 */
#define vdi_w 1280
#define vdi_h 1024
#define vdi_form_id 3 /* pixel-packed */
#define vdi_bit_order 1
#define vdi_planes 8

#if vdi_planes > 32
you loose
#elif vdi_planes > 16
typedef uint32_t pel;
#elif vdi_planes > 8
typedef uint16_t pel;
#else
typedef uint8_t pel;
#endif

struct fillparams {
	const _UWORD *fill_pattern;
	pel fg;
	pel bg;
};




static VEX_BUTV user_but;
static VEX_TIMV user_tim;
static VEX_MOTV user_mot;
static VEX_CURV user_cur;
static VEX_WHEELV user_wheel;

#ifndef R_OK
#define R_OK 4
#endif
#ifndef MIN
#define MIN(_a,_b) ((_a)<(_b)?(_a):(_b))
#endif
#ifndef MAX
#define MAX(_a,_b) ((_a)>(_b)?(_a):(_b))
#endif

#define SUPPORT_GDOS 1
#define GDOS_VERSION 0x53544e58L


#define CTAB_ID 0xbadc0de1


#if defined(OS_ATARI) && 0
#define SCREEN_BASE (*((void **)0x44e))
#else
#define SCREEN_BASE 0
#endif

static VWK *vwk[MAX_VWK];

static pel framebuffer[vdi_w * vdi_h];
#define pixel(x, y) framebuffer[(y) * v->width + (x)]

#include "../data/fnt_st_6x8.c"
#include "../data/fnt_st_8x8.c"
#include "../data/fnt_st_8x16.c"
#include "../data/fnt_l9_6x8.c"
#include "../data/fnt_l9_8x8.c"
#include "../data/fnt_l9_8x16.c"
#include "../data/fnt_l2_6x8.c"
#include "../data/fnt_l2_8x8.c"
#include "../data/fnt_l2_8x16.c"
#include "../data/fnt_gr_6x8.c"
#include "../data/fnt_gr_8x8.c"
#include "../data/fnt_gr_8x16.c"
#include "../data/fnt_ru_6x8.c"
#include "../data/fnt_ru_8x8.c"
#include "../data/fnt_ru_8x16.c"

static FONT_DESC sysfont[SYSFONTS * NLSFONTSETS];
static const FONT_HDR *const sysfonthdrs[SYSFONTS * NLSFONTSETS] = {
	&fnt_st_6x8, &fnt_st_8x8, &fnt_st_8x16,
	&fnt_l9_6x8, &fnt_l9_8x8, &fnt_l9_8x16,
	&fnt_l2_6x8, &fnt_l2_8x8, &fnt_l2_8x16,
	&fnt_gr_6x8, &fnt_gr_8x8, &fnt_gr_8x16,
	&fnt_ru_6x8, &fnt_ru_8x8, &fnt_ru_8x16,
};

static int xvdi_debug;

static int phys_handle;
static vdi_palette screen_pal;
static vdi_palette dummy_pal;

static struct alpha_info global_vt52;

static const _UWORD **fillpat[5];
static const _UWORD *allpatterns[NUM_PATTERN_MASKS];

#if TRACE_VDI
static const char *const function_name[16] = {
	"GXclear",			/* D := 0 */             
	"GXand",			/* D := S AND D */
	"GXandReverse",		/* D := S AND (NOT D) */
	"GXcopy",			/* D := S */
	"GXandInverted",	/* D := (NOT S) AND D */
	"GXnoop",			/* D := D */
	"GXxor",			/* D := S XOR D */
	"GXor",				/* D := S OR D */
	"GXnor",			/* D := NOT (S OR D) */
	"GXequiv",			/* D := NOT (S XOR D) */
	"GXinvert",			/* D := NOT D */
	"GXorReverse",		/* D := S OR (NOT D) */
	"GXcopyInverted",	/* D := NOT S */
	"GXorInverted",		/* D := (NOT S) OR D */
	"GXnand",			/* D := NOT (S AND D) */
	"GXset"				/* D := 1 */
};
#endif


struct driverinfo {
	_WORD device_id;
	const char *name;
	const char *desc;
};

static struct driverinfo const devices[] = {
	{ VDI_SCREEN_DEVICE, "screen.sys", "STonX display driver" },
	{ VDI_PRINTER_DEVICE, "printer.sys", "STonX printer driver" },
	{ VDI_META_DEVICE, "meta.sys", "STonX metafile output" },
	{ VDI_MEMORY_DEVICE, "memory.sys", "STonX bitmap output" },
	{ VDI_OFFSCREEN_DEVICE, "bitmap.sys", "STonX OffScreen Bitmap driver" },
	{ VDI_FAX_DEVICE, "fax.sys", "STonX fax driver" },
	{ VDI_BITMAP_DEVICE, "img.sys", "STonX IMG output" }
};

static volatile char MOUSE_FLAG;
static volatile char CUR_FLAG;
static volatile _WORD GCURX;
static volatile _WORD GCURY;
static _WORD M_HID_CNT;
static volatile unsigned char CUR_MS_STAT;
static unsigned char V_STAT_0;
static unsigned char V_PERIOD;
static unsigned char V_DELAY;
static _WORD REQ_COL[16][3];
static _WORD MOUSE_BT;
static _WORD SAVE_STAT;

#define sys_Bconin(dev) 0
#define sys_Bconstat(dev) 1
#define sys_Kbshift(mode) 0
#define sys_Scrdmp()
#define sys_Tickcal() 20
#define bell_enabled() 0

static struct {
	int xhot;
	int yhot;
	pel fg, bg;
	_UWORD mask[MOUSE_CURSOR_MASKSIZE];
	_UWORD data[MOUSE_CURSOR_MASKSIZE];
} x_cursor;

#define NUM_COLORS(planes) ((planes) > 8 ? 256 : (1 << (planes)))
#define AVAILABLE_COLORS(planes) ((planes) == 1 ? 2 : (planes) > 8 ? 0 : 4096)

#define FIX_COLOR(_c) vdi_maptab[_c]
#define XBUF(func, ...)

#define vdi_draw_polygons(gc, points, n) \
	(void)(points); XBUF(XFillPolygon, gc, points, n, Complex, CoordModeOrigin)

#define VDI_SET_POINT(p, _x, _y) p.x = _x, p.y = _y

#define TODO 0

/******************************************************************************/
/* -------------------------------------------------------------------------- */
/******************************************************************************/

static const struct driverinfo *get_devinfo(_WORD id)
{
	int i;
	
	for (i = 0; i < (int)(sizeof(devices) / sizeof(devices[0])); i++)
	{
		if (devices[i].device_id == id)
			return &devices[i];
	}
	return NULL;
}

/******************************************************************************/
/* -------------------------------------------------------------------------- */
/******************************************************************************/

#define rolw(pattern) ((pattern) & 0x8000) ? (((pattern) << 1) | 0x0001) : ((pattern) << 1)
#define rorw(pattern) ((pattern) & 0x0001) ? (((pattern) >> 1) | 0x8000) : ((pattern) >> 1)

static void vdi_put_pixel(VWK *v, int x, int y, pel color)
{
	if (x < 0 || x >= v->width || y < 0 || y >= v->height)
		return;
	if (v->clipping)
	{
		if (x < v->clipr.x || x >= v->clipr.x + v->clipr.width)
			return;
		if (y < v->clipr.y || y >= v->clipr.y + v->clipr.height)
			return;
	}
	pixel(x, y) = color;
}


static pel vdi_get_pixel(VWK *v, int x, int y)
{
	if (x < 0 || x >= v->width || y < 0 || y >= v->height)
		return 0;
	if (v->clipping)
	{
		if (x < v->clipr.x || x >= v->clipr.x + v->clipr.width)
			return 0;
		if (y < v->clipr.y || y >= v->clipr.y + v->clipr.height)
			return 0;
	}
	return pixel(x, y);
}


static void gfx_put_pixel(VWK *v, int x, int y, _UWORD pattern, int op, pel fg, pel bg)
{
	switch (op)
	{
	case MD_REPLACE:
		vdi_put_pixel(v, x, y, pattern & 0x8000 ? fg : bg);
		break;
	case MD_TRANS:
		if (pattern & 0x8000)
			vdi_put_pixel(v, x, y, fg);
		break;
	case MD_XOR:
		if (pattern & 0x8000)
			vdi_put_pixel(v, x, y, ~vdi_get_pixel(v, x, y));
		break;
	case MD_ERASE:
		if (!(pattern & 0x8000))
			vdi_put_pixel(v, x, y, fg);
		break;
	}
}


static void vdi_draw_hline(VWK *v, int x1, int x2, int y, _UWORD pattern, int op, pel fg, pel bg)
{
	int x;
	
	for (x = x1; x <= x2; x++)
	{
		gfx_put_pixel(v, x, y, pattern, op, fg, bg);
		pattern = rolw(pattern);
	}
}


static void vdi_draw_vline(VWK *v, int x, int y1, int y2, _UWORD pattern, int op, pel fg, pel bg)
{
	int y;
	
	for (y = y1; y <= y2; y++)
	{
		gfx_put_pixel(v, x, y, pattern, op, fg, bg);
		pattern = rolw(pattern);
	}
}


static void vdi_draw_line(VWK *v, int x1, int y1, int x2, int y2, _UWORD pattern, int op, pel fg, pel bg)
{
	/* Test for special cases of straight lines or single point */
	if (x1 == x2)
	{
		if (y1 < y2)
		{
			vdi_draw_vline(v, x1, y1, y2, pattern, op, fg, bg);
		} else if (y1 > y2)
		{
			vdi_draw_vline(v, x1, y2, y1, pattern, op, fg, bg);
		} else
		{
			gfx_put_pixel(v, x1, y1, pattern, op, fg, bg);
		}
		return;
	}
	if (y1 == y2)
	{
		if (x1 < x2)
		{
			vdi_draw_hline(v, x1, x2, y1, pattern, op, fg, bg);
		} else if (x1 > x2)
		{
			vdi_draw_hline(v, x2, x1, y1, pattern, op, fg, bg);
		} else
		{
			/* x1 == x2; already catched above */
		}
		return;
	}
	/* else diagonal line; TODO */
}


static void vdi_draw_lines(VWK *v, VDI_POINT *points, int n, _UWORD pattern, int op, pel fg, pel bg)
{
	int x1, y1, x2, y2;
	
	x1 = points->x;
	y1 = points->y;
	points++;
	while (--n > 0)
	{
		x2 = points->x;
		y2 = points->y;
		points++;
		vdi_draw_line(v, x1, y1, x2, y2, pattern, op, fg, bg);
		x1 = x2;
		y1 = y2;
	}
}


static void destroy_image(void)
{
}

/******************************************************************************/

static void init_filled(VWK *v, struct fillparams *params, int color)
{
	int style;
	
	V("  init_filled: interior: %s, style: %d, color: %d, mode: %s", interior_name[v->fill_interior], v->fill_style, v->fill_color, wrmode_name[v->wrmode]);

	if (!IS_SCREEN_V(v))
		return;
	
	style = v->fill_style;
	if (style < 1 || style > patnum[v->fill_interior])
		style = 1;
	
	params->fill_pattern = v->fill_interior == FIS_USER ? v->ud_fill_pattern : fillpat[v->fill_interior][style - 1];
	switch (v->wrmode)
	{
	default:
	case MD_REPLACE:
		params->fg = FIX_COLOR(v->fill_interior == FIS_HOLLOW ? v->bg_color : color);
		params->bg = FIX_COLOR(v->bg_color);
		if (v->fill_interior == FIS_SOLID || v->fill_interior == FIS_HOLLOW || (v->fill_interior == FIS_PATTERN && style == 8))
		{
			params->fill_pattern = fillpat[FIS_SOLID][0];
		}
		break;
	case MD_TRANS:
		params->fg = FIX_COLOR(v->fill_interior == FIS_HOLLOW ? v->bg_color : color);
		params->bg = FIX_COLOR(v->bg_color);
		/* maybe fixme: MD_TRANS + FIS_HOLLOW is actually a no-op */
		break;
	case MD_XOR:
		params->fg = FIX_COLOR(WHITE) ^ FIX_COLOR(BLACK);
		params->bg = FIX_COLOR(BLACK);
		break;
	case MD_ERASE:
		params->fg = FIX_COLOR(color);
		params->bg = FIX_COLOR(v->bg_color);
		break;
	}
}


static void fill_rectangle_params(VWK *v, int x, int y, unsigned int width, unsigned int height, struct fillparams *params)
{
	unsigned int x0, y0, p;
	_UWORD pattern;
	
	if (width > 0 && height > 0)
	{
		for (y0 = 0, p = 0; y0 < height; y0++)
		{
			pattern = params->fill_pattern[p];
			for (x0 = 0; x0 < width; x0++)
			{
				gfx_put_pixel(v, x + x0, y + y0, pattern, v->wrmode, params->fg, params->bg);
				pattern = rolw(pattern);
			}
			if (++p == PATTERN_HEIGHT)
				p = 0;
		}
	}
}


static void fill_rectangle_brush(VWK *v, int x, int y, unsigned int width, unsigned int height)
{
	struct fillparams params;
	
	init_filled(v, &params, v->fill_color);
	fill_rectangle_params(v, x, y, width, height, &params);
}


static void fill_rectangle_color(VWK *v, int x, int y, unsigned int width, unsigned int height, int color)
{
	struct fillparams params;
	
	init_filled(v, &params, color);
	fill_rectangle_params(v, x, y, width, height, &params);
}


static void create_pattern_pixmap(const _UWORD *data, _UWORD *mask)
{
	int i;
	
	for (i = 0; i < PATTERN_SIZE; i++)
		mask[i] = data[i];
}

/******************************************************************************/

static void change_mode(void)
{
}


static void clear_window(VWK *v)
{
	fill_rectangle_color(v, 0, 0, v->width, v->height, WHITE);
}


static void init_window(void)
{
	change_mode();
}


static void v_reset_alpha_cursor(VWK *v)
{
	struct alpha_info *vt52;

	vt52 = v->vt52;
	vt52->cursor_inverse = FALSE;
	vt52->blink_count = vt52->blink_rate;
}


static void v_show_alpha_cursor(VWK *v)
{
	struct alpha_info *vt52;
	int x, y;
	
	vt52 = v->vt52;
	if (vt52->curs_hid_cnt != 0 || vt52->cursor_inverse)
		return;
	x = vt52->ax * vt52->acw;
	y = vt52->ay * vt52->ach;
	(void) x;
	(void) y;
	vt52->cursor_inverse = TRUE;
	vt52->blink_count = vt52->blink_rate;
}


static void v_hide_alpha_cursor(VWK *v)
{
	struct alpha_info *vt52;
	int x, y;
	
	vt52 = v->vt52;
	if (vt52->curs_hid_cnt != 0 || !vt52->cursor_inverse)
		return;
	x = vt52->ax * vt52->acw;
	y = vt52->ay * vt52->ach;
	(void) x;
	(void) y;
	vt52->cursor_inverse = FALSE;
	vt52->blink_count = vt52->blink_rate;
}


static void init_wk(VWK *v, int width, int height, int planes)
{
	int i;
	struct alpha_info *vt52;
	
	v->width = width;
	v->height = height;
	v->planes = planes;
	v->form_id = vdi_form_id;
	v->bit_order = vdi_bit_order;
	v->bitmap_addr = framebuffer;
	v->to_free = 0;
	
	v->dev_tab.max_x = v->width - 1;
	v->dev_tab.max_y = v->height - 1;
	v->dev_tab.scale_flag = 0;
	v->dev_tab.pix_width = 372;
	v->dev_tab.pix_height = 372;
	v->dev_tab.font_sizes = 3;			
	v->dev_tab.line_types = LT_MAX;
	v->dev_tab.line_widths = 0;		
	v->dev_tab.marker_types = PM_MAX;
	v->dev_tab.marker_sizes = (MAX_MKHT - MIN_MKHT) / 2 + 1;
	v->dev_tab.num_fonts = 1;
	v->dev_tab.num_patterns = patnum[FIS_PATTERN];
	v->dev_tab.num_shapes = patnum[FIS_HATCH];
	v->dev_tab.num_colors = NUM_COLORS(v->planes);
	
	/* GDPs */
	v->dev_tab.num_gdps = 10;
	for (i = 1; i <= 10; i++)
		v->dev_tab.gdp_funcs[i - 1] = i;
	/* GDP attributes: */
	v->dev_tab.gdp_attribs[0] = 3;				/* v_bar: filled */
	v->dev_tab.gdp_attribs[1] = 0;				/* v_arc: line */
	v->dev_tab.gdp_attribs[2] = 3;				/* v_pieslice: filled */
	v->dev_tab.gdp_attribs[3] = 3;				/* v_circle: filled */
	v->dev_tab.gdp_attribs[4] = 3;				/* v_ellipse: filled */
	v->dev_tab.gdp_attribs[5] = 0;				/* v_ellarc: line */
	v->dev_tab.gdp_attribs[6] = 3;				/* v_ellpie: filled */
	v->dev_tab.gdp_attribs[7] = 0;				/* v_rbox: line */
	v->dev_tab.gdp_attribs[8] = 3;				/* v_rfbox: filled */
	v->dev_tab.gdp_attribs[9] = 2;				/* v_justified: text */
	
	v->dev_tab.color_flag = v->planes > 1;
	v->dev_tab.rotation_flag = 1;
	v->dev_tab.fillarea_flag = 1;
	v->dev_tab.cellarray_flag = 0;
	v->dev_tab.available_colors = AVAILABLE_COLORS(v->planes);
	v->dev_tab.cursor_control = 2;
	v->dev_tab.valuator_control = 1;
	v->dev_tab.choice_control = 1;
	v->dev_tab.string_control = 1;
	v->dev_tab.device_type = 2;
	
	v->siz_tab.min_char_width = 5;
	v->siz_tab.min_char_height = 4;
	v->siz_tab.max_char_width = 7;
	v->siz_tab.max_char_height = 13;
	v->siz_tab.min_line_width = 1;
	v->siz_tab.reserved1 = 0;
	v->siz_tab.max_line_width = 40;
	v->siz_tab.reserved2 = 0;
	v->siz_tab.min_marker_width = MIN_MKWD;
	v->siz_tab.min_marker_height = MIN_MKHT;
	v->siz_tab.max_marker_width = MAX_MKWD;
	v->siz_tab.max_marker_height = MAX_MKHT;
	
	v->inq_tab.screen_type = 4;					/* shared graphic/text */
	v->inq_tab.background_colors = v->dev_tab.available_colors;
	v->inq_tab.supported_effects = SUPPORTED_EFFECTS;
	v->inq_tab.scaling_flag = 0;				/* scaling */
	v->inq_tab.planes = v->planes;				/* planes */
	v->inq_tab.clut_flag = v->planes > 1 && v->planes <= 8; /* lookup table */
	v->inq_tab.blits_per_sec = 5000;			/* blit operations/s, probably more ;) */
	v->inq_tab.contourfill_flag = 1; 			/* contourfill available */
	v->inq_tab.rotation_flag = 1;				/* text rotation available in 90 degree steps */
	v->inq_tab.num_wrmodes = 4;					/* # writing modes */
	v->inq_tab.max_input_mode = MODE_SAMPLE;	/* highest input mode */
	v->inq_tab.justification_flag = 1;			/* text justification available */
	v->inq_tab.pen_change = 0;					/* pen change */
	v->inq_tab.ribbon_change = 0;				/* ribbon change */
	v->inq_tab.max_points = MAX_POINTS;			/* max # of points for pline/pmarker/fillarea */
	v->inq_tab.max_intin_size = -1;				/* max size of intin array (unlimited) */
	v->inq_tab.mouse_buttons = 2;				/* mouse buttons */
	v->inq_tab.line_types_flag = 0;				/* line types for linewidth > 1 available */
	v->inq_tab.wrmode_flag = 1;					/* writing modes for linewidth > 1 available */
	v->inq_tab.clipping = v->clipping;			/* clipping flag */
	v->inq_tab.pixsize_flag = 0;				/* extended precision pixel size information */
	v->inq_tab.pix_width = 0;					/* pixel width in 1/10, 1/100 or 1/1000 microns */
	v->inq_tab.pix_height = 0;					/* pixel height in 1/10, 1/100 or 1/1000 microns */
	v->inq_tab.hdpi = 0;						/* horizontal resolution in dpi */
	v->inq_tab.vdpi = 0;						/* vertical resolution in dpi */
	v->inq_tab.image_rotation = 0;				/* image rotation available */
	v->inq_tab.quarter_screen_high = 0;			/* address of quarter screen buffer (PC/GEM) */
	v->inq_tab.quarter_screen_low = 0;
	v->inq_tab.bezier_flag = 2;					/* bezier flag */
	v->inq_tab.reserved1 = 0;					/* reserved */
	v->inq_tab.raster_flags = 0;				/* raster flags */
	v->inq_tab.reserved2 = 0;					/* reserved */
	v->inq_tab.color_flags = 0;					/* color management etc */
	v->inq_tab.reserved3 = 0;					/* reserved */
	v->inq_tab.reserved4 = 0;					/* reserved */
	v->inq_tab.reserved5 = 0;					/* reserved */
	v->inq_tab.reserved6 = 0;					/* reserved */
	v->inq_tab.reserved7 = 0;					/* reserved */
	v->inq_tab.reserved8 = 0;					/* reserved */
	v->inq_tab.reserved9 = 0;					/* reserved */
	v->inq_tab.left_border = 0;					/* not imprintable left border in pixels (printers/plotters) */
	v->inq_tab.top_border = 0;					/* not imprintable upper border in pixels (printers/plotters) */
	v->inq_tab.right_border = 0;				/* not imprintable right border in pixels (printers/plotters) */
	v->inq_tab.bottom_border = 0;				/* not imprintable lower border in pixels (printers/plotters) */
	v->inq_tab.page_size = 0;					/* page size (printers etc.) */

	if (v->handle == phys_handle)
	{
		vt52 = &global_vt52;
		
		v->vt52 = vt52;
		vt52->ach = 16;
		vt52->acw = 8;
		vt52->font_index = vt52->ach >= 16 ? 2 : 1;
		vt52->ah = v->height / vt52->ach;
		vt52->aw = v->width / vt52->acw;
		vt52->ax = vt52->ay = 0;
		vt52->start_esc = FALSE;
		vt52->curs_hid_cnt = 1;
		vt52->wrap_on = FALSE;
		vt52->rev_on = FALSE;
		vt52->asavex = vt52->asavey = 0;
		vt52->abg = WHITE;
		vt52->afg = BLACK;
		vt52->blinking = (V_STAT_0 & 1) != 0;
		vt52->blink_rate = V_PERIOD;
		vt52->blink_delay = V_DELAY;
		vt52->v_cur_of = 0;
		
		v_reset_alpha_cursor(v);
	}
}


static void set_clipping(VWK *v)
{
	if (v->can_clip)
	{
	}
}


static int make_rectangle(VWK *v, int x1, int y1, int x2, int y2, vdi_rectangle *r, int max_w, int max_h)
{
	if (x1 < x2)
	{
		r->x = x1;
		r->width = x2 - x1 + 1;
	} else
	{
		r->x = x2;
		r->width = x1 - x2 + 1;
	}
	if (r->x < 0)
	{
		if (-(r->x) >= r->width)
			r->width = 0;
		else
			r->width += r->x;
		r->x = 0;
	}
	if (r->x >= v->width || r->x >= max_w)
	{
		r->x = 0;
		r->width = 0;
	}
	if ((r->x + (int)r->width) > v->width)
	{
		r->width = v->width - r->x;
	}
	if ((r->x + (int)r->width) > max_w)
	{
		r->width = max_w - r->x;
	}
	if (y1 < y2)
	{
		r->y = y1;
		r->height = y2 - y1 + 1;
	} else
	{
		r->y = y2;
		r->height = y1 - y2 + 1;
	}
	if (r->y < 0)
	{
		if (-(r->y) >= r->height)
			r->height = 0;
		else
			r->height += r->y;
		r->y = 0;
	}
	if (r->y >= v->height || r->y >= max_h)
	{
		r->y = 0;
		r->height = 0;
	}
	if ((r->y + (int)r->height) > v->height)
	{
		r->height = v->height - r->y;
	}
	if ((r->y + (int)r->height) > max_h)
	{
		r->height = max_h - r->y;
	}
	return r->width > 0 && r->height > 0;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

static int vdi_vswr_mode(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int m = V_INTIN(pb, 0);

	V("vswr_mode[%d]: %d", v->handle, m);

	if (m < MD_REPLACE || m > MD_ERASE)
		m = MD_REPLACE;
	v->wrmode = m;
	V_INTOUT(pb, 0) = m;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vs_clip(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	vdi_rectangle *r;

	/*
	 * Do not print clip coordinates with clip_flag == FALSE,
	 * they are sometimes not initialized
	 */
	if (V_INTIN(pb, 0) == 0)
	{
		V("vs_clip[%d]: %d, %d,%d, %d,%d", v->handle, V_INTIN(pb, 0), 0, 0, v->width, v->height);
	} else
	{
		V("vs_clip[%d]: %d, %d,%d, %d,%d", v->handle, V_INTIN(pb, 0), V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 2), V_PTSIN(pb, 3));
	}
	if (v->can_clip)
	{
		r = &(v->clipr);
		if (V_INTIN(pb, 0) == 0)
		{
			v->clipping = v->inq_tab.clipping = FALSE;
			make_rectangle(v, 0, 0, v->width, v->height, r, v->width, v->height);
		} else
		{
			if (make_rectangle(v, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 2), V_PTSIN(pb, 3), r, v->width, v->height))
			{
				v->clipping = v->inq_tab.clipping = TRUE;
			}
		}
		set_clipping(v);
	}
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_color(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD c;
	
	c = V_INTIN(pb, 0);
	V("vq_color[%d]: (%d,%d)", v->handle, c, V_INTIN(pb, 1));
	
	if (c < 0 || c >= v->dev_tab.num_colors)
	{
		V_INTOUT(pb, 0) = -1;
		V_INTOUT(pb, 1) = -1;
		V_INTOUT(pb, 2) = -1;
		V_INTOUT(pb, 3) = -1;
	} else
	{
		V_INTOUT(pb, 0) = 0;
		if (V_INTIN(pb, 1) == 0)
		{
			V_INTOUT(pb, 1) = (*v->req_col)[c][0];
			V_INTOUT(pb, 2) = (*v->req_col)[c][1];
			V_INTOUT(pb, 3) = (*v->req_col)[c][2];
		} else
		{
			V_INTOUT(pb, 1) = (*v->req_col)[c][0];
			V_INTOUT(pb, 2) = (*v->req_col)[c][1];
			V_INTOUT(pb, 3) = (*v->req_col)[c][2];
		}
	}
	V_NINTOUT(pb, 4);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_ctab(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	COLOR_TAB *ctab = (COLOR_TAB *)PV_INTOUT(pb);
	uint32_t ctab_length = vdi_intin_long(0);
	uint32_t length;
	int i;
	
	V("vq_ctab[%d]: %u", v->handle, ctab_length);
	
	V_NPTSOUT(pb, 0);
	length = sizeof(COLOR_TAB) + (v->dev_tab.num_colors - 1) * sizeof(COLOR_ENTRY);
	if (length > ctab_length)
	{
		V("Too little space available for ctab (%d when ctab needs %d)!", ctab_length, length);
		V_NINTOUT(pb, 0);
	} else
	{
		ctab->magic = COLOR_TAB_MAGIC;
		ctab->length = length;
		ctab->format = 0;
		ctab->reserved = 0;
		ctab->map_id = 0xbadc0de1;
		ctab->color_space = CSPACE_RGB;
		ctab->flags = 0;
		ctab->no_colors = v->dev_tab.num_colors;
		ctab->reserved1 = 0;
		ctab->reserved2 = 0;
		ctab->reserved3 = 0;
		ctab->reserved4 = 0;
		for (i = 0; i < v->dev_tab.num_colors; i++)
		{
			int c;
			ctab->colors[i].rgb.reserved = 0;
			c = (*v->req_col)[i][0]; c = c * 255 / 1000; c |= c << 8;
			ctab->colors[i].rgb.red = c;
			c = (*v->req_col)[i][1]; c = c * 255 / 1000; c |= c << 8;
			ctab->colors[i].rgb.green = c;
			c = (*v->req_col)[i][2]; c = c * 255 / 1000; c |= c << 8;
			ctab->colors[i].rgb.blue = c;
		}
		V_NINTOUT(pb, length / 2);
	}
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_ctab_entry(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD color = V_INTIN(pb, 0);
	COLOR_ENTRY *entry;
	
	V("vq_ctab_entry[%d]: %u", v->handle, color);
	
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 6);
	if (color < 0 || color >= v->dev_tab.num_colors)
		color = BLACK;
	vdi_intout_long(0) = CSPACE_RGB;
	{
		int c;
		entry = (COLOR_ENTRY *)(intout + 2);
		entry->rgb.reserved = 0;
		c = (*v->req_col)[color][0]; c = c * 255 / 1000; c |= c << 8;
		entry->rgb.red = c;
		c = (*v->req_col)[color][1]; c = c * 255 / 1000; c |= c << 8;
		entry->rgb.green = c;
		c = (*v->req_col)[color][2]; c = c * 255 / 1000; c |= c << 8;
		entry->rgb.blue = c;
	}
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_ctab_id(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	
	UNUSED(v);
	V("vq_ctab_id[%d]", v->handle);
	
	vdi_intout_long(0) = CTAB_ID;	/* Not really correct */
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 2);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_ctab_idx2vdi(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD color = V_INTIN(pb, 0);
	
	V("v_ctab_idx2vdi[%d]: %u", v->handle, color);
	if (color < 0 || color >= v->dev_tab.num_colors)
		color = BLACK;
	
	V_INTOUT(pb, 0) = vdi_revtab256[vdi_maptab256[vdi_revtab256[color] & (v->dev_tab.num_colors - 1)]];
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 1);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_ctab_vdi2idx(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD color = V_INTIN(pb, 0);
	
	V("v_ctab_vdi2idx[%d]: %u", v->handle, color);
	if (color < 0 || color >= v->dev_tab.num_colors)
		color = BLACK;
	
	V_INTOUT(pb, 0) = vdi_maptab256[color] & (v->dev_tab.num_colors - 1);
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 1);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_ctab_idx2value(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD color = V_INTIN(pb, 0);
	
	V("v_ctab_idx2value[%d]: %u", v->handle, color);
	
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 1);
	if (color < 0 || color >= v->dev_tab.num_colors)
		color = BLACK;
	V_INTOUT(pb, 0) = vdi_revtab256[vdi_maptab256[vdi_revtab256[color] & (v->dev_tab.num_colors - 1)]];
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_dflt_ctab(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	COLOR_TAB *ctab = (COLOR_TAB *)PV_INTOUT(pb);
	uint32_t ctab_length = vdi_intin_long(0);
	uint32_t length;
	int i;
	
	V("vq_dflt_ctab[%d]: %u", v->handle, ctab_length);
	
	V_NPTSOUT(pb, 0);
	length = sizeof(COLOR_TAB) + (v->dev_tab.num_colors - 1) * sizeof(COLOR_ENTRY);
	if (length > ctab_length)
	{
		V("Too little space available for ctab (%d when ctab needs %d)!", ctab_length, length);
		V_NINTOUT(pb, 0);
	} else
	{
		ctab->magic = COLOR_TAB_MAGIC;
		ctab->length = length;
		ctab->format = 0;
		ctab->reserved = 0;
		ctab->map_id = 0xbadc0de1;
		ctab->color_space = CSPACE_RGB;
		ctab->flags = 0;
		ctab->no_colors = v->dev_tab.num_colors;
		ctab->reserved1 = 0;
		ctab->reserved2 = 0;
		ctab->reserved3 = 0;
		ctab->reserved4 = 0;
		for (i = 0; i < v->dev_tab.num_colors; i++)
		{
			int c;
			ctab->colors[i].rgb.reserved = 0;
			c = initial_palette[i][0]; c = c * 255 / 1000; c |= c << 8;
			ctab->colors[i].rgb.red = c;
			c = initial_palette[i][1]; c = c * 255 / 1000; c |= c << 8;
			ctab->colors[i].rgb.green = c;
			c = initial_palette[i][2]; c = c * 255 / 1000; c |= c << 8;
			ctab->colors[i].rgb.blue = c;
		}
		V_NINTOUT(pb, length / 2);
	}
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_get_ctab_id(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	
	V("v_get_ctab_id[%d]", v->handle);
	UNUSED(v);
	
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 2);
	vdi_intout_long(0) = CTAB_ID;	/* Not really correct */
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vs_color(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	int c;
	int i;
	int val;
	
	c = V_INTIN(pb, 0);
	V("vs_color[%d]: %d", v->handle, c);
	
	if (c >= 0 && c < v->dev_tab.num_colors)
	{
		for (i = 1; i <= 3; i++)
		{
			val = V_INTIN(pb, i);
			val = MIN(MAX(val, 0), 1000);
			(*v->req_col)[c][i - 1] = val;
			if (IS_SCREEN_V(v))
			{
				if (c < 16)
					REQ_COL[c][i - 1] = val;
			}
		}
	}
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static gboolean vdi_set_mouse(_WORD *intin)
{
	int fg, bg;
	int ncolors;
	
	MOUSE_FLAG = MOUSE_FLAG + 1;		/* disable mouse redrawing (mouse_flag) */

	create_pattern_pixmap((const _UWORD *)&V_INTIN(pb, 5), x_cursor.mask);
	create_pattern_pixmap((const _UWORD *)&V_INTIN(pb, 21), x_cursor.data);
	ncolors = NUM_COLORS(vdi_planes);
	fg = V_INTIN(pb, 4);
	if (fg < 0 || fg >= ncolors)
		fg = BLACK;
	x_cursor.fg = FIX_COLOR(fg);
	bg = V_INTIN(pb, 3);
	if (bg < 0 || bg >= ncolors)
		bg = BLACK;
	x_cursor.bg = FIX_COLOR(bg);
	x_cursor.xhot = V_INTIN(pb, 0) & (MOUSE_CURSOR_WIDTH - 1);
	x_cursor.yhot = V_INTIN(pb, 1) & (MOUSE_CURSOR_HEIGHT - 1);

	MOUSE_FLAG = MOUSE_FLAG - 1;		/* re-enable mouse drawing */

	return VDI_DONE;
}


static int vdi_vsc_form(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	
	V("vsc_form[%d]: (%d,%d,%d,%d,%d)", v->handle, V_INTIN(pb, 0), V_INTIN(pb, 1), V_INTIN(pb, 2), V_INTIN(pb, 3), V_INTIN(pb, 4));

	if (IS_SCREEN_V(v))
	{
		vdi_set_mouse(intin);
	}
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/
/* Input functions                                                            */
/******************************************************************************/

static int gchr_key(_UWORD *key)
{
	uint32_t ch;
	int retval = 0;
	
	if (sys_Bconstat(2))
	{
		ch = sys_Bconin(2);
		*key = ((ch >> 8) & 0xff00) | (ch & 0xff);
		retval = 1;
	}
	return retval;
}


/*
 * vrq_string,vsm_string
 */
static int vdi_vrq_string(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int maxl = V_INTIN(pb, 0);
#if 0
	int echo_mode = V_INTIN(pb, 1);
	int echo_x = V_PTSINX(pb, 0);
	int echo_y = V_V_PTSINY(pb, pb, 0);
#endif
	_WORD i, mask;
	_UWORD term_ch;
	
#if 0 /* called a lot from AES, so dont trace it */
	V("v%s_string[%d]: (%d,%d,%d,%d)", v->input_mode[DEV_STRING] == MODE_REQUEST ? "rq" : "sm", v->handle, maxl, echo_mode, echo_x, echo_y);
#endif
	
	mask = 0xff;
	if (maxl < 0)
	{
		maxl = -maxl;
		mask = ~0;
	}
	if (!IS_SCREEN_V(v))
	{
		i = 0;
		V_INTOUT(pb, 0) = 0;
	} else if (v->input_mode[DEV_STRING] == MODE_REQUEST)
	{
		term_ch = 0;
		for (i = 0; i < maxl && (term_ch & 0x00ff) != 0x000d; i++)
		{
			while (gchr_key(&term_ch) == 0)
				;
			term_ch &= mask;
			V_INTOUT(pb, i) = term_ch;
		}
		if ((term_ch & 0x00ff) == 0x000d)
			i--;
	} else
	{
		i = 0;
		term_ch = 0;
		while (gchr_key(&term_ch) && i < maxl)
		{
			term_ch &= mask;
			V_INTOUT(pb, i) = term_ch;
			i++;
		}
	}
	V_NINTOUT(pb, i);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_mouse(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);

	UNUSED(v);
	V("vq_mouse[%d]", v->handle);

	if (IS_SCREEN_V(v))
	{
		V_INTOUT(pb, 0) = MOUSE_BT;
		PTSOUTX(0) = GCURX;
		PTSOUTY(0) = GCURY;
	} else
	{
		V_INTOUT(pb, 0) = 0;
		PTSOUTX(0) = 0;
		PTSOUTY(0) = 0;
	}
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 1);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_key_s(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD shift;
	
	UNUSED(v);
#if 0 /* called a lot from AES, so dont trace it */
	V("vq_key_s[%d]", v->handle);
#endif
	if (IS_SCREEN_V(v))
	{
		shift = sys_Kbshift(-1);
	} else
	{
		shift = 0;
	}
	V_INTOUT(pb, 0) = shift;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsin_mode(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int d = V_INTIN(pb, 0);
	int m = V_INTIN(pb, 1);

#if 0 /* called a lot from AES, so dont trace it [e1bac6] */
	V("vsin_mode[%d]: %d(%s) %d(%s)",
	   v->handle,
	   d, d == DEV_LOCATOR ? "locator" : d == DEV_VALUATOR ? "valuator" : d == DEV_CHOICE ? "choice" : "string", m, m == MODE_REQUEST ? "request" : "sample");
#endif
	if (d < 1 || d > 4)
		d = DEV_LOCATOR;
	if (m <= 0 || m > v->inq_tab.max_input_mode)
		m = v->inq_tab.max_input_mode;
	v->input_mode[d] = m;
	V_INTOUT(pb, 0) = m;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vqin_mode(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int d = V_INTIN(pb, 0);

	V("vqin_mode[%d]: %d", v->handle, d);

	if (d < 1 || d > 4)
		d = DEV_LOCATOR;
	V_INTOUT(pb, 0) = v->input_mode[d];
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

/*
 * gloc_key - get locator key
 *
 * returns:  0    - nothing
 *           1    - button pressed
 *                  TERM_CH = 16 bit char info
 *
 *           2    - coordinate info
 *                     X1 = new x
 *                     Y1 = new y
 *           4    - NOT IMPLIMENTED IN THIS VERSION
 *
 * The variable cur_ms_stat holds the bitmap of mouse status since the last
 * interrupt. The bits are
 *
 * 0 - 0x01 Left mouse button status  (0=up)
 * 1 - 0x02 Right mouse button status (0=up)
 * 2 - 0x04 Reserved
 * 3 - 0x08 Reserved
 * 4 - 0x10 Reserved
 * 5 - 0x20 Mouse move flag (1=moved)
 * 6 - 0x40 Right mouse button status flag (0=hasn't changed)
 * 7 - 0x80 Left mouse button status flag  (0=hasn't changed)
 */

static int gloc_key(_UWORD *term_ch, _WORD *ptsin)
{
	int retval;
	uint32_t ch;
	_UBYTE cur_ms_stat;
	
	cur_ms_stat = CUR_MS_STAT;
	if (cur_ms_stat & 0xc0)			/* some button status bits set? */
	{
		if (cur_ms_stat & 0x40)		/* if bit 6 set */
			*term_ch = 0x21;		/* send terminator code for left key */
		else
			*term_ch = 0x20;		/* send terminator code for right key */
		CUR_MS_STAT = cur_ms_stat & 0x23;		/* clear mouse button status (bit 6/7) */
		retval = 1;					/* set button pressed flag */
	} else							/* check key stat */
	{
		if (sys_Bconstat(2))
		{							/* see if a character present at con */
			ch = sys_Bconin(2);
			*term_ch = (_UWORD)
				((ch >> 8) & 0xff00)|			/* scancode down to bit 8-15 */
				(ch & 0xff);		/* asciicode to bit 0-7 */
			retval = 1;				/* set button pressed flag */
		} else
		{
			if (cur_ms_stat & 0x20)   /* if bit #5 set ... */
			{
				CUR_MS_STAT = cur_ms_stat | ~0x20;	/* clear bit 5 */ /* clear? why | then??? */
				V_PTSINX(pb, 0) = GCURX;	/* set X = GCURX */
				V_PTSINY(pb, 0) = GCURY;	/* set Y = GCURY */
				retval = 2;
			} else
			{
				retval = 0;
			}
		}
	}
	return retval;
}

/******************************************************************************/

/*
 * cur_display - blits a "cursor" to the destination
 *
 * nothing really to do in the emulation,
 * except updating some Line-A vars
 */
static void cur_display(_WORD x, _WORD y)
{
	UNUSED(x);
	UNUSED(y);
	SAVE_STAT = 0; /* save_stat */
}

/******************************************************************************/

/*
 * cur_replace - replace cursor with data in save area.
 *
 * in:
 *     save_area       memory where saved data resides
 *     save_addr       points to destination
 *     save_len        number of lines to be returned
 *     save_stat       status and format of save buffer
 *     _v_planes       number of planes in destination
 *     _v_line_wr      line wrap (byte width of form)
 */

static void cur_replace(void)
{
}

/******************************************************************************/

/*
 * hide_cursor
 *
 * This routine hides the mouse cursor if it has not already
 * been hidden.
 *
 * Inputs:         None
 *
 * Outputs:
 *    m_hid_cnt = m_hid_cnt + 1
 *    cur_flag = 0
 */
static void hide_cursor(void)
{
	MOUSE_FLAG = MOUSE_FLAG + 1;		/* disable mouse redrawing (mouse_flag) */

	/*
	 * Increment the counter for the number of hide operations performed.
	 * If this is the first one then remove the cursor from the screen.
	 * If not then do nothing, because the cursor wasn't on the screen.
	 */
	M_HID_CNT = M_HID_CNT + 1;		/* increment M_HID_CNT */
	if (M_HID_CNT == 1)
	{											/* if cursor was not hidden... */
		cur_replace();							/* remove the cursor from screen */
		CUR_FLAG = 0;						/* disable vbl drawing routine (cur_flag) */
	}

	MOUSE_FLAG = MOUSE_FLAG - 1;		/* re-enable mouse drawing */
}

/******************************************************************************/

/*
 * display_cursor - Displays the mouse cursor if the number of hide 
 *		   operations has gone back to 0.
 *
 *  Decrement the counter for the number of hide operations performed.
 *  If this is not the last one then do nothing because the cursor
 *  should remain hidden.
 *
 *   Outputs:
 *	  m_hid_cnt = m_hid_cnt - 1
 *	  cur_flag = 0
 */
static void display_cursor(void)
{
	MOUSE_FLAG = MOUSE_FLAG + 1;		/* disable mouse redrawing (* mouse_flag) */
	M_HID_CNT = M_HID_CNT - 1;			/* decrement hide operations counter (M_HID_CNT) */
	if (M_HID_CNT == 0)
	{
		cur_display(GCURX, GCURY);		/* display the cursor at GCURX/GCURY */
		CUR_FLAG = 0;					/* disable vbl drawing routine (cur_flag) */
	} else if (M_HID_CNT < 0)
	{
		M_HID_CNT = 0;					/* hide counter should not become negative */
	}
	MOUSE_FLAG = MOUSE_FLAG - 1;		/* re-enable mouse drawing */
}

/******************************************************************************/

/*
 * vrq_locator, vsm_locator
 */
static int vdi_vrq_locator(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	_WORD i;
	_UWORD term_ch;
	
	V("v%s_locator[%d]: (%d, %d)", v->input_mode[DEV_LOCATOR] == MODE_REQUEST ? "rq" : "sm", v->handle, V_PTSINX(pb, 0), V_PTSINY(pb, 0));

	V_INTIN(pb, 0) = 1; /* ??? */
	V_NINTOUT(pb, 1);

	if (!IS_SCREEN_V(v))
	{
		V_NPTSOUT(pb, 0);
		return VDI_DONE;
	}
	
	/* Set the initial locator position. */

	GCURX = V_PTSINX(pb, 0);
	GCURY = V_PTSINY(pb, 0);
	if (v->input_mode[DEV_LOCATOR] == MODE_REQUEST)
	{
		display_cursor();
		/* loop till some event */
		while ((i = gloc_key(&term_ch, ptsin)) != 1)
		{
			if (i == 4)	   /* keyboard cursor? */
			{
				hide_cursor();	 /* turn cursor off */
				GCURX = V_PTSINX(pb, 0); /* GCURX */
				GCURY = V_PTSINY(pb, 0); /* GCURY */
				display_cursor();	  /* turn cursor on */
			}
		}
		V_INTOUT(pb, 0) = term_ch & 0x00ff;

		V_NPTSOUT(pb, 1);

		PTSOUTX(0) = V_PTSINX(pb, 0);
		PTSOUTY(0) = V_PTSINY(pb, 0);
		hide_cursor();
	} else
	{
		V_NPTSOUT(pb, 0);

		i = gloc_key(&term_ch, ptsin);
		switch (i)
		{
		case 0:
			V_NPTSOUT(pb, 0);
			break;

		case 1:
			V_NPTSOUT(pb, 0);
			V_NINTOUT(pb, 1);
			V_INTOUT(pb, 0) = term_ch & 0x00ff;
			break;

		case 2:
			PTSOUTX(0) = V_PTSINX(pb, 0);
			PTSOUTY(0) = V_PTSINY(pb, 0);
			break;

		case 3:
			V_NINTOUT(pb, 1);
			PTSOUTX(0) = V_PTSINX(pb, 0);
			PTSOUTY(0) = V_PTSINY(pb, 0);
			break;

		case 4:
			if (M_HID_CNT == 0)
			{
				hide_cursor();
				PTSOUTX(0) = V_PTSINX(pb, 0);
				PTSOUTY(0) = V_PTSINY(pb, 0);
				GCURX = V_PTSINX(pb, 0);
				GCURY = V_PTSINY(pb, 0);
				display_cursor();
			} else
			{
				PTSOUTX(0) = V_PTSINX(pb, 0);
				PTSOUTY(0) = V_PTSINY(pb, 0);
				GCURX = V_PTSINX(pb, 0);
				GCURY = V_PTSINY(pb, 0);
			}
			break;
		}
	}
	return VDI_DONE;
}

/******************************************************************************/

static gboolean linea_showmouse(_WORD reset)
{
    if (reset && M_HID_CNT != 0)
		M_HID_CNT = 1;           /* reset cursor to on */

    display_cursor();
    return VDI_DONE;
}

static gboolean vdi_v_show_c(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);

	V("v_show_c[%d]: %d", v->handle, V_INTIN(pb, 0));

	if (IS_SCREEN_V(v))
	{
		_WORD reset = V_INTIN(pb, 0);
		linea_showmouse(reset);
	}
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
    return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_hide_c(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("v_hide_c[%d]", v->handle);

	if (IS_SCREEN_V(v))
	{
	    hide_cursor();
	}
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
    return VDI_DONE;
}

/******************************************************************************/

static int gchc_key(_UWORD *key)
{
	*key = 1;
	return 1;
}


/*
 * vrq_choice, vsm_choice
 */
static int vdi_vrq_choice(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	_UWORD term_ch;
	_WORD i;
	
	V("v%s_choice[%d]: (%d)", v->input_mode[DEV_CHOICE] == MODE_REQUEST ? "rq" : "sm", v->handle, *PV_INTIN(pb));

	if (!IS_SCREEN_V(v))
	{
		V_INTOUT(pb, 0) = 0;
		V_NINTOUT(pb, 1);
	} else if (v->input_mode[DEV_CHOICE] == MODE_REQUEST)
	{
		while (gchc_key(&term_ch) != 1)
			;
		V_INTOUT(pb, 0) = term_ch & 0xff;
		V_NINTOUT(pb, 1);
	} else
	{
		i = gchc_key(&term_ch);
		V_NINTOUT(pb, i);
		if (i == 1)
			V_INTOUT(pb, 0) = term_ch & 0xff;
		else if (i == 2)
			V_INTOUT(pb, 0) = term_ch & 0xff;
	}
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

/*
 * vrq_valuator/vsm_valuator
 */
static int vdi_vrq_valuator(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	
	UNUSED(v);
	V("v%s_valuator[%d]: (%d)", v->input_mode[DEV_LOCATOR] == MODE_REQUEST ? "rq" : "sm", v->handle, V_INTIN(pb, 0));

	/* not implemented in ROM VDI, so probably not needed */
	V_INTOUT(pb, 0) = V_INTIN(pb, 0);
	V_INTOUT(pb, 1) = 0;
	V_NINTOUT(pb, 2);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/
/* Text functions                                                             */
/******************************************************************************/

#if TRACE_VDI

static char *ascii_text(int nchars, _WORD *string)
{
	int i;
	static char buf[256];
	unsigned short c;
	
	for (i = 0; i < nchars && i < 255; i++, string++)
	{
		c = *string & 0xff;
		if (atarifont_to_utf16[c] < 0x100)
			buf[i] = atarifont_to_utf16[c];
		else
			buf[i] = atarifont_to_utf16[0xff];
	}
	buf[i] = 0;
	return buf;
}
#endif

/******************************************************************************/

static int text_width(FONT_DESC *xf, _WORD *items, int n)
{
	int width, i, c;
	
	width = 0;
	for (i = 0; i < n; i++)
	{
		c = items[i];
		if (c >= xf->first_char && c <= xf->last_char)
			width += xf->per_char[c - xf->first_char].width;
		else
			width += xf->per_char[xf->default_char - xf->first_char].width;
	}
	return width;
}

/******************************************************************************/

static void set_font_align(VWK *v, _WORD *items, int n, gboolean justified)
{
	int width;
	int d1, d2;
	
	if (v->text_style & TXT_OUTLINED)
		v->outline_size = 1;
	else
		v->outline_size = 0;
	if (sysfont[v->font_index].bottom <= sysfont[v->font_index].underline_size)
	{
		if (sysfont[v->font_index].scaled && v->dda_inc == 0xffff)
			v->underline_size = -1;
		else
			v->underline_size = 0;
	} else
	{
		v->underline_size = 1;
	}
	
	switch (v->h_align)
	{
	case ALI_LEFT:
		v->font_xoff = 0;
		break;
	case ALI_CENTER:
		if (!justified)
			width = text_width(&sysfont[v->font_index], items, n);
		else
			width = 0;
		v->font_xoff = width / 2 - v->outline_size;
		break;
	case ALI_RIGHT:
		if (!justified)
			width = text_width(&sysfont[v->font_index], items, n);
		else
			width = 0;
		v->font_xoff = width - v->outline_size * 2;
		break;
	}
	
	if (v->text_style & TXT_SKEWED)
	{
		d1 = sysfont[v->font_index].left_offset;
		d1 = sysfont[v->font_index].right_offset;
	} else
	{
		d1 = 0;
		d2 = 0;
	}
	
	switch (v->v_align)
	{
	case ALI_BASE:
		v->font_yoff = sysfont[v->font_index].top;
		v->font_xoff += d1;
		break;
	case ALI_HALF:
		v->font_yoff = sysfont[v->font_index].top - sysfont[v->font_index].half;
		v->font_xoff += (sysfont[v->font_index].half * d2) / sysfont[v->font_index].top;
		break;
	case ALI_ASCENT:
		v->font_yoff = sysfont[v->font_index].top - sysfont[v->font_index].ascent;
		v->font_xoff += (sysfont[v->font_index].ascent * d2) / sysfont[v->font_index].top;
		break;
	case ALI_BOTTOM:
		v->font_yoff = sysfont[v->font_index].top + sysfont[v->font_index].bottom;
		break;
	case ALI_DESCENT:
		v->font_yoff = sysfont[v->font_index].top + sysfont[v->font_index].descent;
		v->font_xoff += (sysfont[v->font_index].descent * d1) / (sysfont[v->font_index].bottom ? sysfont[v->font_index].bottom : 1);
		break;
	case ALI_TOP:
		v->font_yoff = 0;
		v->font_xoff += d1 + d2;
		break;
	}
}

/******************************************************************************/

static void init_font(VWK *v, _WORD *items, int n, gboolean justified)
{
	set_font_align(v, items, n, justified);
}

/******************************************************************************/

static int vdi_draw_char(VWK *v, int x0, int y0, unsigned char c, pel fg, pel bg)
{
	const FONT_DESC *sf;
	const vdi_charinfo *info;
	int j;
	int w;
	int x, y;
	const uint8_t *line;
	int off;
	const FONT_HDR *hdr;
	int p;
	int b;
	uint8_t inmask;
	
	sf = &sysfont[v->font_index];
	hdr = sf->hdr;
	if (c < sf->first_char || c > sf->last_char)
		c = sf->default_char;
	info = &sf->per_char[c - sf->first_char];
	off = hdr->off_table[c - sf->first_char];
	y = y0;
	for (j = 0; j < hdr->form_height; j++)
	{
		line = hdr->dat_table + hdr->form_width * j;
		b = off & 7;
		inmask = 0x80 >> b;
		p = off >> 3;
		w = info->width;
		x = x0;
		while (w > 0)
		{
			switch (v->wrmode)
			{
			case MD_REPLACE:
				vdi_put_pixel(v, x, y, line[p] & inmask ? fg : bg);
				break;
			case MD_TRANS:
				if (line[p] & inmask)
					vdi_put_pixel(v, x, y, fg);
				break;
			case MD_XOR:
				if (line[p] & inmask)
					vdi_put_pixel(v, x, y, ~vdi_get_pixel(v, x, y));
				break;
			case MD_ERASE:
				if (!(line[p] & inmask))
					vdi_put_pixel(v, x, y, fg);
				break;
			}
			inmask >>= 1;
			b++;
			if (b == 8)
			{
				p++;
				inmask = 0x80;
				b = 0;
			}
			w--;
			x++;
		}
		y++;
	}
	(void) v;
	(void) x;
	(void) y;
	(void) c;
	(void) fg;
	(void) bg;
	return x0 + info->width;
}


static void draw_string(VWK *v, int x, int y, _WORD *items, int n)
{
	int i;
	pel fg, bg;
	
	fg = FIX_COLOR(v->text_color);
	bg = FIX_COLOR(v->bg_color);
	for (i = 0; i < n; i++)
		x = vdi_draw_char(v, x, y, items[i], fg, bg);
}

/******************************************************************************/

static void underline_text(VWK *v, int x, int y, int len)
{
	int ly;
	
	ly = y - sysfont[v->font_index].top + v->underline_size + sysfont[v->font_index].underline_size;
	vdi_draw_hline(v, x, x + len - 1, ly, 0xffff, v->wrmode, FIX_COLOR(v->text_color), FIX_COLOR(v->bg_color));
}

/******************************************************************************/

static void draw_text(VWK *v, int x, int y, _WORD *items, int n)
{
	init_font(v, items, n, FALSE);
	/* printf("%d.%d %s -> %d.%d yoff = %d v_align = %d", x, y,
		ascii_text(n, VADDR(items)),
		x - v->font_xoff - v->outline_size, y - v->font_yoff - v->outline_size,
		v->font_yoff, v->v_align); */
	x = x - v->font_xoff - v->outline_size;
	y = y - v->font_yoff - v->outline_size;
	draw_string(v, x, y, items, n);
	if (v->text_style & TXT_UNDERLINED)
		underline_text(v, x, y, text_width(&sysfont[v->font_index], items, n));
}

/******************************************************************************/

static void change_font(VWK *v, int i)
{
	if (IS_SCREEN_V(v))
	{
	}
	v->font_index = (v->font_id - 1) * SYSFONTS + i;
}

/******************************************************************************/

static int vdi_v_gtext(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	int x = V_PTSINX(pb, 0);
	int y = V_PTSINY(pb, 0);
	int n = V_NINTIN(pb);
	
	V("v_gtext[%d]: (%d,%d) '%s'", v->handle, x, y, ascii_text(n, &V_INTIN(pb, 0)));

	draw_text(v, x, y, &V_INTIN(pb, 0), n);
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_justified(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	int len, word_space, char_space, x, y, n, i;
	_WORD *t;
	double s, off = 0.0;
	FONT_DESC *sf;
	int cw;
	
	V("v_justified[%d]: (%d,%d), '%s', %d,%d,%d", v->handle, V_PTSINX(pb, 0), V_PTSINY(pb, 0), ascii_text(V_NINTIN(pb) - 2, &V_INTIN(pb, 2)), V_PTSINX(pb, 1), V_INTIN(pb, 0), V_INTIN(pb, 1));

	len = V_PTSIN(pb, 2);
	word_space = V_INTIN(pb, 0);
	char_space = V_INTIN(pb, 1);
	UNUSED(word_space);
	UNUSED(char_space);
	n = V_NINTIN(pb) - 2;
	t = &V_INTIN(pb, 2);
	init_font(v, t, n, TRUE);
	sf = &sysfont[v->font_index];
	x = V_PTSIN(pb, 0) - v->font_xoff - v->outline_size;
	y = V_PTSIN(pb, 1) - v->font_yoff - v->outline_size;

	/* x -= len / 2; */
	if (n > 1 && len > 0)
		s = (double) (len - text_width(sf, (_WORD *)t, n)) / (n - 1);
	else
		s = 0;
	for (i = 0; i < n; i++)
	{
		draw_string(v, (int)(x + off), y, t + i, 1);
		cw = text_width(sf, t + i, 1);
		off += s + cw;
		if (word_space & 0x8000)
			V_INTOUT(pb, i) = cw;
	}
	if (v->text_style & TXT_UNDERLINED)
		underline_text(v, x, y, (int)off);
	
	if (!(word_space & 0x8000))
		n = 0;
	V_NINTOUT(pb, n);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vst_font(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD id = *PV_INTIN(pb);
	_WORD index;
	
	UNUSED(v);
	V("vst_font[%d]: %d", v->handle, id);

	if (id < 1 || id > NLSFONTSETS)
		id = SYSTEM_FONT_ID;
	V_INTOUT(pb, 0) = id;
	index = v->font_index % SYSFONTS;
	v->font_id = id;
	change_font(v, index);
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vst_height(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	int i;
	int height = V_PTSIN(pb, 1);

	V("vst_height[%d]: %d", v->handle, height);

	i = 0;
	for (;;)
	{
		++i;
		if (i >= SYSFONTS)
			break;
		if (sysfont[i].top > height)
			break;
	}
	--i;
	change_font(v, i);
	PTSOUTX(0) = sysfont[i].width;
	PTSOUTY(0) = sysfont[i].height;
	PTSOUTX(1) = sysfont[i].cellwidth;
	PTSOUTY(1) = sysfont[i].cellheight;
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 2);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vst_point(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	int p = V_INTIN(pb, 0);
	int i;

	V("vst_point[%d]: %d", v->handle, p);

	i = 0;
	for (;;)
	{
		++i;
		if (i >= SYSFONTS)
			break;
		if (sysfont[i].pointsize > p)
			break;
	}
	--i;
	PTSOUTX(0) = sysfont[i].width;
	PTSOUTY(0) = sysfont[i].height;
	PTSOUTX(1) = sysfont[i].cellwidth;
	PTSOUTY(1) = sysfont[i].cellheight;
	V_INTOUT(pb, 0) = sysfont[i].pointsize;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 2);
	change_font(v, i);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vst_alignment(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int hor = V_INTIN(pb, 0);
	int ver = V_INTIN(pb, 1);

	V("vst_alignment[%d]: %d,%d", v->handle, hor, ver);
	
	if (ver < ALI_BASE || ver > ALI_TOP)
		ver = ALI_TOP;
	if (hor < ALI_LEFT || hor > ALI_RIGHT)
		hor = ALI_LEFT;
	V_INTOUT(pb, 0) = hor;
	V_INTOUT(pb, 1) = ver;
	v->h_align = hor;
	v->v_align = ver;
	V_NINTOUT(pb, 2);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vst_color(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int c = V_INTIN(pb, 0);

	V("vst_color[%d]: %d", v->handle, c);

	if (c < 0 || c >= v->dev_tab.num_colors)
		c = BLACK;
	v->text_color = c;
	V_INTOUT(pb, 0) = c;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vst_rotation(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int angle = V_INTIN(pb, 0);

	V("vst_rotation[%d]: %d", v->handle, angle);

	if (angle == 9001)
	{
		xvdi_debug = 1;
	} else if (angle == 9002)
	{
		xvdi_debug = 0;
	} else
	{
		angle = ((angle + 450) / 900) * 900;
		if (angle >= 3600)
			angle = 0;
		v->text_rotation = angle;
	}
	V_INTOUT(pb, 0) = angle;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE; /* not really */
}

/******************************************************************************/

static int vdi_vst_effects(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int e = V_INTIN(pb, 0);

	V("vst_effects[%d]: %d", v->handle, e);
	
#if SUPPORT_GDOS && 0
	if (external_effects)
	{
		e = external_effects(vwk, current_font, e);
	} else
#endif
	{
		e &= v->inq_tab.supported_effects; /* mask by supported effects */
	}
	v->text_style = e;
	V_INTOUT(pb, 0) = e;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vqt_fontinfo(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	const FONT_DESC *sf;

	V("vqt_fontinfo[%d]", v->handle);

	sf = &sysfont[v->font_index];
	V_INTOUT(pb, 0) = sf->first_char;	/* minade */
	V_INTOUT(pb, 1) = sf->last_char;		/* maxade */
	PTSOUT(0) = sf->cellwidth;		/* max width */
	PTSOUT(1) = sf->bottom;
	PTSOUT(2) = sf->left_offset + sf->right_offset;
	PTSOUT(3) = sf->descent;
	PTSOUT(4) = sf->left_offset;
	PTSOUT(5) = sf->half;
	PTSOUT(6) = sf->right_offset;
	PTSOUT(7) = sf->ascent;
	PTSOUT(8) = 0;
	PTSOUT(9) = sf->top;
	V_NINTOUT(pb, 2);
	V_NPTSOUT(pb, 5);

	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vqt_attributes(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	const FONT_DESC *sf;

	sf = &sysfont[v->font_index];
	V("vqt_attributes[%d] -> %dx%d", v->handle, sf->cellwidth, sf->cellheight);

	V_INTOUT(pb, 0) = v->font_id;
	V_INTOUT(pb, 1) = v->text_color;
	V_INTOUT(pb, 2) = v->text_rotation;
	V_INTOUT(pb, 3) = v->h_align;
	V_INTOUT(pb, 4) = v->v_align;
	V_INTOUT(pb, 5) = v->wrmode - 1; /* strange, but vqt_attributes reports it off-by-1 */
	PTSOUT(0) = sf->width;
	PTSOUT(1) = sf->height;
	PTSOUT(2) = sf->cellwidth;
	PTSOUT(3) = sf->cellheight;
	V_NINTOUT(pb, 6);
	V_NPTSOUT(pb, 2);

	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vqt_width(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	_UWORD c = V_INTIN(pb, 0);
	FONT_DESC *sf;
	
	V("vqt_width[%d]: %d", v->handle, c);

	sf = &sysfont[v->font_index];
	if (c < sf->first_char || c > sf->last_char)
	{	
		PTSOUT(0) = sf->cellwidth;
		PTSOUT(2) = 0;
		PTSOUT(4) = 0;
		V_INTOUT(pb, 0) = -1;
	} else
	{
		if (sf->per_char != NULL)
		{
			c -= sf->first_char;
			PTSOUT(0) = sf->per_char[c].width;
			PTSOUT(2) = sf->per_char[c].lbearing;
			PTSOUT(4) = sf->per_char[c].width - sf->per_char[c].rbearing;
		} else
		{
			PTSOUT(0) = sf->cellwidth;
			PTSOUT(2) = sf->left_offset;
			PTSOUT(4) = sf->right_offset;
		}
		V_INTOUT(pb, 0) = 0;
	}
	PTSOUT(1) = 0;
	PTSOUT(3) = 0;
	PTSOUT(5) = 0;
	V_NPTSOUT(pb, 3);
	V_NINTOUT(pb, 1);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vqt_name(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	/* _WORD *intin = PV_INTIN(pb); unused; we have only 1 font */
	_WORD *intout = PV_INTOUT(pb);
	const char *s;
	int i;
	FONT_DESC *sf;
	
	UNUSED(v);
	V("vqt_name[%d]: %d", v->handle, *PV_INTIN(pb));

	V_INTOUT(pb, 0) = v->font_id;
	sf = &sysfont[(v->font_id - 1) * SYSFONTS];
	s = sf->name;
	for (i = 1; *s && i < (VDI_FONTNAMESIZE + 1); i++, s++)
		V_INTOUT(pb, i) = *s;
	for (; i < (VDI_FONTNAMESIZE + 1); i++)
		V_INTOUT(pb, i) = 0;
	V_INTOUT(pb, VDI_FONTNAMESIZE + 1) = 0; /* type is bitmap font */
	V_NINTOUT(pb, VDI_FONTNAMESIZE + 2);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vqt_extent(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	FONT_DESC *sf;
	int n, width;
	
	V("vqt_extent[%d]", v->handle);

	sf = &sysfont[v->font_index];
	n = V_NINTIN(pb);
	width = text_width(sf, &V_INTIN(pb, 0), n);
	
	PTSOUTX(0) = 0;
	PTSOUTY(0) = 0;
	PTSOUTX(1) = width;
	PTSOUTY(1) = 0;
	PTSOUTX(2) = width;
	PTSOUTY(2) = sf->cellheight;
	PTSOUTX(3) = 0;
	PTSOUTY(3) = sf->cellheight;
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 4);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vst_skew(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD skew = V_INTIN(pb, 0);
	
	V("vst_skew[%d]: %d", v->handle, skew);
	
	skew = 0; /* NYI */
	v->skew = skew;
	V_INTOUT(pb, 0) = skew;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/
/* Raster functions                                                           */
/******************************************************************************/

/* findpixelindex: Searches for a CLUT entry for the pixel at address pptr */
static int findpixelindex(pel pixelcol)
{
	return st_revtab[pixelcol];
}


static void init_oraster(VWK *v, int mode)
{
	(void) v;
	(void) mode;
}


/*
 * copy raster from screen to MFDB d in device dependent format
 */
static void vro_cpy_from_screen(VWK *v, int x, int y, int w, int h, MFDB *d, int dx, int dy)
{
	if (d->fd_nplanes != v->planes)
	{
		V("  vro_cpy_from_screen[%d]: error: planes %d != screen planes %d", v->handle, d->fd_nplanes, v->planes);
		return;
	}
	
	if (x < 0)
	{
		w += x;
		dx -= x;
		x = 0;
	}
	if (y < 0)
	{
		h += y;
		dy -= y;
		y = 0;
	}
	if (w <= 0 || h <= 0)
		return;

	if (v->planes == 1)
	{
		V("  vro_cpy_from_screen(%d->%d, [%d,%d,%d,%d]->[%d,%d] MFDB[0x%p,%d,%d,wd=%d,std=%d]", v->planes, d->fd_nplanes, x, y, w, h, dx, dy, d->fd_addr, d->fd_w, d->fd_h, d->fd_wdwidth, d->fd_stand);
	} else								/* Not a monochrome raster: */
	{
		V("  vro_cpy_from_screen(%d->%d, [%d,%d,%d,%d]->[%d,%d] MFDB[0x%p,%d,%d,wd=%d,std=%d]", v->planes, d->fd_nplanes, x, y, w, h, dx, dy, d->fd_addr, d->fd_w, d->fd_h, d->fd_wdwidth, d->fd_stand);
	}
}


/*
 * copy raster from MFDB s in device dependent format to the screen
 */
static void vro_cpy_to_screen(VWK *v, int mode, int x, int y, int w, int h, MFDB *s, int dx, int dy)
{
	int planes;
	int rowsize, planesize;
	uint16_t srcstartmask, srcmask;
	uint16_t *sp, *srclp;
	int i, j, l;
	
	if (dx < 0)
	{
		w += dx;
		x -= dx;
		dx = 0;
	}
	if (dy < 0)
	{
		h += dy;
		y -= dy;
		dy = 0;
	}
	if (w <= 0 || h <= 0)
		return;
	
	V("  vro_cpy_to_screen(%d->%d, [%d,%d,%d,%d]->[%d,%d] MFDB[0x%p,%d,%d,wd=%d,std=%d]", s->fd_nplanes, v->planes, x, y, w, h, dx, dy, s->fd_addr, s->fd_w, s->fd_h, s->fd_wdwidth, s->fd_stand);
	/*
	 * This implementation is not VDI compliant.
	 * it depends on the data NOT being converted to screen format.
	 */
	planes = s->fd_nplanes;
	rowsize = s->fd_wdwidth;
	planesize = rowsize * s->fd_h;
	sp = (uint16_t *)s->fd_addr + rowsize * y + (x >> 4);
	srcstartmask = 0x8000 >> (x & 0x0f);
	for (i = 0; i < h; i++)
	{
		srclp = sp;
		srcmask = srcstartmask;
		for (j = 0; j < w; j++)
		{
			pel val;
			
			for (val = 0, l = 0; l < planes; l++)
			{
				/* n2hs depends on FLIP_DATA in the resource loader */
				if (n2hs(srclp[l * planesize]) & srcmask)
					val |= 1 << l;
			}
			if (planes == 2)
				val = vdi_maptab256[vdi_revtab4[val]];
			else if (planes == 4)
				val = vdi_maptab256[vdi_revtab16[val]];
			switch (mode)
			{
			case ALL_WHITE:
				val = vdi_maptab256[WHITE];
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case S_AND_D:
				val &= vdi_get_pixel(v, dx + j, dy + i);
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case S_AND_NOTD:
				val &= ~vdi_get_pixel(v, dx + j, dy + i);
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case S_ONLY:
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case NOTS_AND_D:
				val = (~val) & vdi_get_pixel(v, dx + j, dy + i);
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case D_ONLY:
				/* expensive no-op */
				break;
			case S_XOR_D:
				val ^= vdi_get_pixel(v, dx + j, dy + i);
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case S_OR_D:
				val |= vdi_get_pixel(v, dx + j, dy + i);
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case NOT_SORD:
				val = ~(val | vdi_get_pixel(v, dx + j, dy + i));
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case NOT_SXORD:
				val = ~(val ^ vdi_get_pixel(v, dx + j, dy + i));
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case NOT_D:
				val = ~vdi_get_pixel(v, dx + j, dy + i);
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case S_OR_NOTD:
				val |= ~vdi_get_pixel(v, dx + j, dy + i);
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case NOT_S:
				val = ~val;
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case NOTS_OR_D:
				val = ~val;
				val |= vdi_get_pixel(v, dx + j, dy + i);
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case NOT_SANDD:
				val &= vdi_get_pixel(v, dx + j, dy + i);
				val = ~val;
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			case ALL_BLACK:
				val = vdi_maptab256[BLACK];
				vdi_put_pixel(v, dx + j, dy + i, val);
				break;
			}
			srcmask >>= 1;
			if (srcmask == 0)
			{
				srcmask = 0x8000;
				srclp++;
			}
		}
		sp += rowsize;
	}
}


static void vro_cpy_in_memory(int x, int y, int w, int h, MFDB *s, MFDB *d, int dx, int dy)
{
	/* TODO */
	(void) x;
	(void) y;
	(void) w;
	(void) h;
	(void) s;
	(void) d;
	(void) dx;
	(void) dy;
}


static gboolean do_copy_raster(VWK *v, _WORD *ptsin, int mode, MFDB *s, MFDB *d)
{
	vdi_rectangle sr, dr;
	uint8_t *sa = NULL;
	uint8_t *da = NULL;
	int sf;
	int df;
	int sx1, sy1, sx2, sy2;
	int dx1, dy1, dx2, dy2;
	int to_s, fr_s;

	sx1 = V_PTSIN(pb, 0);
	sy1 = V_PTSIN(pb, 1);
	sx2 = V_PTSIN(pb, 2);
	sy2 = V_PTSIN(pb, 3);
	dx1 = V_PTSIN(pb, 4);
	dy1 = V_PTSIN(pb, 5);
	dx2 = V_PTSIN(pb, 6);
	dy2 = V_PTSIN(pb, 7);
	fr_s = (s == 0 || (sa = (uint8_t *)s->fd_addr) == 0 || sa == SCREEN_BASE);
	to_s = (d == 0 || (da = (uint8_t *)d->fd_addr) == 0 || da == SCREEN_BASE);

	if (fr_s)
		sf = 0;
	else
		sf = s->fd_stand & 1;
	if (to_s)
		df = 0;
	else
		df = d->fd_stand & 1;
	if ((sf != 0 || df != 0) && s && s->fd_nplanes != 1)
	{
		V("vro_cpyfm[%d]: error: illegal args (rasters in standard format)", v->handle);
	}
	
	init_oraster(v, mode);
	if (to_s)
	{
		if (make_rectangle(v, dx1, dy1, dx2, dy2, &dr, v->width, v->height))
		{
			if (fr_s)
			{
				if (make_rectangle(v, sx1, sy1, sx2, sy2, &sr, v->width, v->height))
				{
				}
			} else
			{
				if (make_rectangle(v, sx1, sy1, sx2, sy2, &sr, s->fd_w, s->fd_h))
				{
					if (dx1 + (int)sr.width > v->width)
					{
						sr.width = v->width - dx1;
					}
					if (dy1 + (int)sr.height > v->height)
					{
						sr.height = v->height - dy1;
					}
					vro_cpy_to_screen(v, mode, sr.x, sr.y, sr.width, sr.height, s, dx1, dy1);
				}
			}
		}
	} else
	{
		if (make_rectangle(v, dx1, dy1, dx2, dy2, &dr, d->fd_w, d->fd_h))
		{
			/*
			 * clipping does not apply when not copying to screen
			 */
			if (v->clipping)
			{
			}
			if (fr_s)
			{
				if (make_rectangle(v, sx1, sy1, sx2, sy2, &sr, v->width, v->height))
				{
					if ((dx1 + (int)sr.width) > d->fd_w)
						sr.width = d->fd_w - dx1;
					if ((dy1 + (int)sr.height) > d->fd_h)
						sr.height = d->fd_h - dy1;
					vro_cpy_from_screen(v, sr.x, sr.y, sr.width, sr.height, d, dx1, dy1);
				}
			} else
			{
				if (make_rectangle(v, sx1, sy1, sx2, sy2, &sr, s->fd_w, s->fd_h))
				{
					if ((dx1 + (int)sr.width) > d->fd_w)
						sr.width = d->fd_w - dx1;
					if ((dy1 + (int)sr.height) > d->fd_h)
						sr.height = d->fd_h - dy1;
					vro_cpy_in_memory(sr.x, sr.y, sr.width, sr.height, s, d, dx1, dy1);
				}
			}
			set_clipping(v);
		}
	}
	
	return VDI_DONE;
}


static int vdi_vro_cpyfm(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	MFDB *s = vdi_control_ptr(0, MFDB *);
	MFDB *d = vdi_control_ptr(1, MFDB *);
	int mode;
	int ret;
	
	/* we dont support raster scaling, disable scaling bit of mode */
	mode = V_INTIN(pb, 0) & 0x0f;
	
	ret = do_copy_raster(v, ptsin, mode, s, d);
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return ret;
}

/******************************************************************************/

static int init_traster(VWK *v, int mode, int fg, int bg)
{
	(void) v;
	(void) mode;
	(void) fg;
	(void) bg;
	return TRUE;
}


static void vdi_put_image(VWK *v, vdi_rectangle *sr, MFDB *s, int dx, int dy, int op, pel fg, pel bg)
{
	const uint16_t *src;
	int x, y;
	uint16_t bit;
	
	for (y = 0; y < sr->height; y++)
	{
		src = (const uint16_t *)s->fd_addr;
		src += (y + sr->y) * s->fd_wdwidth;
		
		for (x = 0; x < sr->width; x++)
		{
			int sx = x + sr->x;
			bit = src[sx >> 4];
			/* n2hs depends on FLIP_DATA in the resource loader */
			bit = n2hs(bit);
			bit &= (0x8000 >> (sx & 0x0f));
			switch (op)
			{
			case MD_REPLACE:
				vdi_put_pixel(v, dx + x, dy + y, bit ? fg : bg);
				break;
			case MD_TRANS:
				if (bit)
					vdi_put_pixel(v, dx + x, dy + y, fg);
				break;
			case MD_XOR:
				if (bit)
					vdi_put_pixel(v, dx + x, dy + y, ~vdi_get_pixel(v, x, y));
				break;
			case MD_ERASE:
				if (!bit)
					vdi_put_pixel(v, dx + x, dy + y, fg);
				break;
			}
		}
	}
}


static int do_copy_transparent(VWK *v, _WORD *intin, _WORD *ptsin, int mode, MFDB *s, MFDB *d)
{
	vdi_rectangle sr, dr;
	uint8_t *sa = NULL;
	uint8_t *da = NULL;
	int sx1, sy1, sx2, sy2;
	int dx1, dy1, dx2, dy2;
	int to_s, fr_s, fg, bg;
	gboolean ret = VDI_DONE;
	
	sx1 = V_PTSIN(pb, 0);
	sy1 = V_PTSIN(pb, 1);
	sx2 = V_PTSIN(pb, 2);
	sy2 = V_PTSIN(pb, 3);
	dx1 = V_PTSIN(pb, 4);
	dy1 = V_PTSIN(pb, 5);
	dx2 = V_PTSIN(pb, 6);
	dy2 = V_PTSIN(pb, 7);
	fg = V_INTIN(pb, 1);
	if (fg < 0 || fg >= v->dev_tab.num_colors)
		fg = BLACK;
	bg = V_INTIN(pb, 2);
	if (bg < 0 || bg >= v->dev_tab.num_colors)
		bg = WHITE;
	to_s = (da = (uint8_t *)d->fd_addr) == 0 || da == SCREEN_BASE;
	fr_s = (sa = (uint8_t *)s->fd_addr) == 0 || sa == SCREEN_BASE;

	if (init_traster(v, mode, fg, bg))
	{
		if (to_s)
		{
			if (s->fd_nplanes == 1)
			{
				if (make_rectangle(v, dx1, dy1, dx2, dy2, &dr, v->width, v->height))
				{
					if (fr_s)
					{
						if (v->planes == 1)
						{
							if (make_rectangle(v, sx1, sy1, sx2, sy2, &sr, v->width, v->height))
							{
								V("vrt_cpyfm: NYI: on screen blit with planes == %d", 1);
							}
						} else
						{
							V("vrt_cpyfm: NYI: on screen blit with planes != %d", 1);
						}
					} else
					{
						if (make_rectangle(v, sx1, sy1, sx2, sy2, &sr, s->fd_w, s->fd_h))
						{
							vdi_put_image(v, &sr, s, dx1, dy1, mode, FIX_COLOR(fg), FIX_COLOR(bg));
						}
					}
				}
			} else
			{
				V("vrt_cpyfm[%d]: error: source planes != 1", v->handle);
			}
		} else
		{
			if (v->planes == 1)
			{
				if (make_rectangle(v, dx1, dy1, dx2, dy2, &dr, d->fd_w, d->fd_h))
				{
					/*
					 * clipping does not apply when not copying to screen
					 */
					if (fr_s)
					{
						if (make_rectangle(v, sx1, sy1, sx2, sy2, &sr, v->width, v->height))
						{
							if ((int)(dx1 + sr.width) > d->fd_w)
								sr.width = d->fd_w - dx1;
							if ((int)(dy1 + sr.height) > d->fd_h)
								sr.height = d->fd_h - dy1;
							vro_cpy_from_screen(v, sr.x, sr.y, sr.width, sr.height, d, dx1, dy1);
						}
					} else
					{
						if (make_rectangle(v, sx1, sy1, sx2, sy2, &sr, s->fd_w, s->fd_h))
						{
							if ((int)(dx1 + sr.width) > d->fd_w)
								sr.width = d->fd_w - dx1;
							if ((int)(dy1 + sr.height) > d->fd_h)
								sr.height = d->fd_h - dy1;
							vro_cpy_in_memory(sr.x, sr.y, sr.width, sr.height, s, d, dx1, dy1);
						}
					}
					set_clipping(v);
				}
			} else
			{
				V("vrt_cpyfm[%d]: error: copy from screen with planes != 1", v->handle);
			}
		}
	}
		
	return ret;
}


static int vdi_vrt_cpyfm(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	MFDB *s = vdi_control_ptr(0, MFDB *);
	MFDB *d = vdi_control_ptr(1, MFDB *);
	int mode;
	gboolean ret;
	
	/* we dont support raster scaling, disable scaling bit of mode */
	mode = V_INTIN(pb, 0) & 0x0f;
	
	ret = do_copy_transparent(v, intin, ptsin, mode, s, d);
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return ret;
}

/******************************************************************************/

static void do_trnfm(VWK *v, MFDB *s, MFDB *d)
{
	uint8_t *sa;
	uint8_t *da;
	int height;
	long words;
	int standard;
	int planes;
	int w, h, p;
	_WORD *src, *dst;
	_WORD *conv;

	UNUSED(v);

	height = s->fd_h;
	words = s->fd_wdwidth;
	standard = s->fd_stand & 1;
	planes = s->fd_nplanes;
	sa = (uint8_t *)s->fd_addr;
	da = (uint8_t *)d->fd_addr;
	
	src = (_WORD *)sa;
	dst = (_WORD *)da;
	
	if (planes == 1)
	{
		if (sa != da)
			memcpy(dst, src, words * 2 * height);
	} else
	{
		conv = NULL;
		if (sa == da)
		{
			conv = g_new(_WORD, words * height * planes);
			dst = conv;
		}
		/* transformation is completely non-overlapped (out of place) */
		if (standard)
		{
			/* Source is in standard format and device independent (raster area) */
			long plane_total;

			plane_total = words * height;

			for (h = height - 1; h >= 0; h--)
			{
				for (w = words - 1; w >= 0; w--)
				{
					_WORD *tmp;

					tmp = src;
					for (p = planes - 1; p >= 0; p--)
					{
						*dst = *src;
						dst++;
						src += plane_total;
					}
					src = tmp + 1;
				}
			}
		} else
		{
			/* Source is device dependent (physical device) */
			for (p = planes - 1; p >= 0; p--)
			{
				_WORD *tmp;

				tmp = src;
				for (h = height - 1; h >= 0; h--)
				{
					for (w = words - 1; w >= 0; w--)
					{
						*dst = *src;
						dst++;
						src += planes;
					}
				}
				src = tmp + 1;
			}
		}
		if (conv)
		{
			dst = (_WORD *)da;
			memcpy(dst, conv, words * 2 * height * planes);
			g_free(conv);
		}
	}
}

static int vdi_vr_trnfm(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	MFDB *s = vdi_control_ptr(0, MFDB *);
	MFDB *d = vdi_control_ptr(1, MFDB *);
	
	V("vr_trnfm[%d]: [%d] S: 0x%lx (0x%lx[%d]), D: 0x%lx (0x%lx[%d]) {%d,%d,%d}",
	   v->handle, 
	   s->fd_nplanes,
	   (long) s, (long)(s ? s->fd_addr : 0), s->fd_stand,
	   (long) d, (long)(d ? d->fd_addr : 0), d->fd_stand,
	   s->fd_w, s->fd_h, s->fd_wdwidth);

	/*
	 * transforming from or to screen memory isn't allowed.
	 */
	if (s == 0 || s->fd_addr == 0 || s->fd_addr == SCREEN_BASE)
	{
		V("vr_trnfm[%d]: error: source is screen", v->handle);
		return VDI_DONE;
	}
	if (d == 0 || d->fd_addr == 0 || d->fd_addr == SCREEN_BASE)
	{
		V("vr_trnfm[%d]: error: destination is screen", v->handle);
		return VDI_DONE;
	}
	
	do_trnfm(v, s, d);
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_get_pixel(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	int x = V_PTSINX(pb, 0);
	int y = V_PTSINY(pb, 0);
	pel pixelcol = 0;
	int i;
	
	V("v_get_pixel[%d]: (%d,%d)", v->handle, x, y);

	if (x >= 0 && y >= 0 && x < v->width && y < v->height)
	{
		pixelcol = pixel(x, y);
	}
	if (v->planes >= 15)
	{
		V_INTOUT(pb, 0) = (_WORD)pixelcol;
		if (v->planes > 16)
			V_INTOUT(pb, 1) = (_WORD)(pixelcol >> 16);
		else
			V_INTOUT(pb, 1) = -1;
	} else
	{
		i = findpixelindex(pixelcol);
		V_INTOUT(pb, 1) = i;
		V_INTOUT(pb, 0) = st_maptab[i];
	}
	V_NINTOUT(pb, 2);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}


/******************************************************************************/
/* VT52 Emulator                                                              */
/******************************************************************************/

static void vt52_scroll(int sx, int sy, int w, int h, int dx, int dy)
{
	/* TODO */
	(void) sx;
	(void) sy;
	(void) w;
	(void) h;
	(void) dx;
	(void) dy;
}


static void output_raw_char_dc(VWK *v, struct alpha_info *vt52, unsigned char c)
{
	int x, y;
	
	v_reset_alpha_cursor(v);
	x = vt52->ax * vt52->acw;
	y = vt52->ay * vt52->ach;
	{
		pel fg, bg;
		if (vt52->rev_on)
		{
			fg = FIX_COLOR(vt52->abg);
			bg = FIX_COLOR(vt52->afg);
		} else
		{
			fg = FIX_COLOR(vt52->afg);
			bg = FIX_COLOR(vt52->abg);
		}
		
		vdi_draw_char(v, x, y, c, fg, bg);
	}
	if (vt52->ax < (vt52->aw - 1))
	{
		vt52->ax++;
	} else
	{
		if (vt52->wrap_on)
		{
			vt52->ax = 0;
			if (vt52->ay == (vt52->ah - 1))
			{
				vt52_scroll(0, vt52->ach, v->width, v->height - vt52->ach, 0, 0);
				fill_rectangle_color(v, 0, v->height - vt52->ach, v->width, vt52->ach, vt52->abg);
			} else
			{
				vt52->ay++;
			}
		}
	}
	v_show_alpha_cursor(v);
}


static void output_raw_char(VWK *v, unsigned char c)
{
	struct alpha_info *vt52;

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 == NULL)
		return;
	{
	output_raw_char_dc(v, vt52, c);
	}
}


/*
 * Entry point for BIOS calls
 */
int vdi_output_c(_WORD dev, unsigned char c)
{
	int h = phys_handle;
	VWK *v;
	struct alpha_info *vt52;
	
	if (!VALID_S_HANDLE(h) || framebuffer == NULL)
	{
		return VDI_PASS;
	}
	v = vwk[h];
	vt52 = v->vt52;
	if (vt52 == NULL)
	{
		return VDI_PASS;
	}
	if (dev == 5)
	{
		output_raw_char(v, c);
		return VDI_DONE;
	}
	{
		if (vt52->start_esc)
		{
			if (vt52->start_esc == 2)
			{
				/* first parameter from cursor position */
				c -= 32;
				vt52->ay = MAX(0, MIN(vt52->ah - 1, c));
				vt52->start_esc = 3;
			} else if (vt52->start_esc == 3)
			{
				/* second parameter from cursor position */
				c -= 32;
				vt52->ax = MAX(0, MIN(vt52->aw - 1, c));
				vt52->start_esc = 0;
			} else if (vt52->start_esc == 4)
			{
				/* parameter from set foreground color */
				int n = v->planes <= 8 ? (1 << v->planes) : MAX_VDI_COLS;
				vt52->afg = st_revtab[c & (n - 1)];
				vt52->start_esc = 0;
			} else if (vt52->start_esc == 5)
			{
				/* parameter from set background color */
				int n = v->planes <= 8 ? (1 << v->planes) : MAX_VDI_COLS;
				vt52->abg = st_revtab[c & (n - 1)];
				vt52->start_esc = 0;
			} else if (vt52->start_esc == 6)
			{
				/* parameter from set blink rate */
				if (c >= 32)
				{
					v_hide_alpha_cursor(v);
					c -= 32;
					if (c == 0)
					{
						vt52->blinking = FALSE;
					} else
					{
						vt52->blinking = TRUE;
						vt52->blink_rate = c;
					}
					v_show_alpha_cursor(v);
				}
			} else
			{
				/* only ESC seen so far */
				vt52->start_esc = 0;
				switch (c)
				{
				case 'A': /* cursor up */
					if (vt52->ay > 0)
					{
						v_hide_alpha_cursor(v);
						--vt52->ay;
						v_show_alpha_cursor(v);
					}
					break;
				case 'B': /* cursor down */
					if (vt52->ay < (vt52->ah - 1))
					{
						v_hide_alpha_cursor(v);
						++vt52->ay;
						v_show_alpha_cursor(v);
					}
					break;
				case 'C': /* cursor right */
					if (vt52->ax < (vt52->aw - 1))
					{
						v_hide_alpha_cursor(v);
						++vt52->ax;
						v_show_alpha_cursor(v);
					}
					break;
				case 'D': /* cursor left */
					if (vt52->ax > 0)
					{
						v_hide_alpha_cursor(v);
						--vt52->ax;
						v_show_alpha_cursor(v);
					}
					break;
				case 'E': /* clear screen */
					v_reset_alpha_cursor(v);
					vt52->ax = vt52->ay = 0;
					fill_rectangle_color(v, 0, 0, v->width, v->height, vt52->abg);
					v_show_alpha_cursor(v);
					break;
				case 'H': /* cursor home */
					v_hide_alpha_cursor(v);
					vt52->ax = vt52->ay = 0;
					v_show_alpha_cursor(v);
					break;
				case 'I': /* cursor up & scroll */
					if (vt52->ay == 0)
					{
						v_reset_alpha_cursor(v);
						vt52_scroll(0, 0, v->width, v->height - vt52->ach, 0, vt52->ach);
						fill_rectangle_color(v, 0, 0, v->width, vt52->ach, vt52->abg);
					} else
					{
						v_hide_alpha_cursor(v);
						vt52->ay--;
					}
					v_show_alpha_cursor(v);
					break;
				case 'J': /* erase to end of screen */
					v_reset_alpha_cursor(v);
					if (vt52->ax == 0)
					{
						fill_rectangle_color(v, 0, vt52->ay * vt52->ach, v->width, v->height - vt52->ay * vt52->ach, vt52->abg);
					} else
					{
						fill_rectangle_color(v, vt52->ax * vt52->acw, vt52->ay * vt52->ach, v->width - vt52->ax * vt52->acw, vt52->ach, vt52->abg);
						if (vt52->ay < (vt52->ah - 1))
						{
							fill_rectangle_color(v, 0, (vt52->ay + 1) * vt52->ach, v->width, v->height - (vt52->ay + 1) * vt52->ach, vt52->abg);
						}
					}
					v_show_alpha_cursor(v);
					break;
				case 'K': /* erase to end of line */
					v_reset_alpha_cursor(v);
					fill_rectangle_color(v, vt52->ax * vt52->acw, vt52->ay * vt52->ach, v->width - vt52->ax * vt52->acw, vt52->ach, vt52->abg);
					v_show_alpha_cursor(v);
					break;
				case 'L': /* insert line */
					v_hide_alpha_cursor(v);
					if (vt52->ay < (vt52->ah - 1))
					{
						vt52_scroll(0, vt52->ay * vt52->ach, v->width, v->height - (vt52->ay + 1) * vt52->ach, 0, (vt52->ay + 1) * vt52->ach);
					}
					vt52->ax = 0;
					fill_rectangle_color(v, 0, vt52->ay * vt52->ach, v->width, vt52->ach, vt52->abg);
					v_show_alpha_cursor(v);
					break;
				case 'M': /* delete line */
					v_reset_alpha_cursor(v);
					if (vt52->ay < (vt52->ah - 1))
					{
						vt52_scroll(0, (vt52->ay + 1) * vt52->ach, v->width, v->height - (vt52->ay + 1) * vt52->ach, 0, vt52->ay * vt52->ach);
					}
					vt52->ax = 0;
					fill_rectangle_color(v, 0, (vt52->ah - 1) * vt52->ach, v->width, v->height - (vt52->ah - 1) * vt52->ach, vt52->abg);
					v_show_alpha_cursor(v);
					break;
				case 'Y': /* set cursor position */
					vt52->start_esc = 2;
					break;
				case 'b': /* set foreground color */
					vt52->start_esc = 4;
					break;
				case 'c': /* set background color */
					vt52->start_esc = 5;
					break;
				case 'd': /* erase to beginning of screen */
					if (vt52->ax == 0)
					{
						fill_rectangle_color(v, 0, 0, v->width, vt52->ay * vt52->ach, vt52->abg);
					} else
					{
						fill_rectangle_color(v, 0, vt52->ay * vt52->ach, vt52->ax * vt52->acw, vt52->ach, vt52->abg);
						if (vt52->ay > 0)
						{
							fill_rectangle_color(v, 0, 0, v->width, (vt52->ay - 1) * vt52->ach, vt52->abg);
						}
					}
					break;
				case 'e': /* enable cursor */
					/* no decrement; unconditionally reset here */
					vt52->curs_hid_cnt = 0;
					v_reset_alpha_cursor(v);
					v_show_alpha_cursor(v);
					break;
				case 'f': /* disable cursor */
					v_hide_alpha_cursor(v);
					vt52->curs_hid_cnt++;
					break;
				case 'j': /* save cursor position */
					vt52->asavex = vt52->ax;
					vt52->asavey = vt52->ay;
					break;
				case 'k': /* restore cursor position */
					v_hide_alpha_cursor(v);
					vt52->ax = vt52->asavex;
					vt52->ay = vt52->asavey;
					v_show_alpha_cursor(v);
					break;
				case 'l': /* erase whole line */
					v_reset_alpha_cursor(v);
					fill_rectangle_color(v, 0, vt52->ay * vt52->ach, v->width, vt52->ach, vt52->abg);
					vt52->ax = 0;
					v_show_alpha_cursor(v);
					break;
				case 'o': /* erase to beginning of line */
					fill_rectangle_color(v, 0, vt52->ay * vt52->ach, vt52->ax * vt52->acw, vt52->ach, vt52->abg);
					break;
				case 'p': /* enter reverse video */
					vt52->rev_on = TRUE;
					break;
				case 'q': /* exit reverse video */
					vt52->rev_on = FALSE;
					break;
				case 't': /* set blink rate */
					vt52->start_esc = 6;
					break;
				case 'v': /* wrap at end of line */
					vt52->wrap_on = TRUE;
					break;
				case 'w': /* discard at end of line */
					vt52->wrap_on = FALSE;
					break;
				}
			}
		} else
		{
			switch (c)
			{
			case 0x0d:
				v_hide_alpha_cursor(v);
				vt52->ax = 0;
				v_show_alpha_cursor(v);
				break;
			case 0x0a:
			case 0x0b:
			case 0x0c:
				if (vt52->ay == vt52->ah - 1)
				{
					v_hide_alpha_cursor(v);
					vt52_scroll(0, vt52->ach, v->width, v->height - vt52->ach, 0, 0);
					fill_rectangle_color(v, 0, v->height - vt52->ach, v->width, vt52->ach, vt52->abg);
				} else
				{
					v_hide_alpha_cursor(v);
					vt52->ay++;
				}
				v_show_alpha_cursor(v);
				break;
			case 0x1b:
				vt52->start_esc = 1;
				break;
			case 0x07:
				if (bell_enabled())
				{
				}
				break;
			case 0x09:
				v_hide_alpha_cursor(v);
				vt52->ax = (vt52->ax & ~7) + 8;
				if (vt52->ax >= vt52->aw)
					vt52->ax = vt52->aw - 1;
				v_show_alpha_cursor(v);
				break;
			case 0x08:
				if (vt52->ax > 0)
				{
					v_hide_alpha_cursor(v);
					--vt52->ax;
					v_show_alpha_cursor(v);
				}
				break;
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06:
			case 0x0e: case 0x0f: case 0x10: case 0x11: case 0x12: case 0x13: case 0x14:
			case 0x15: case 0x16: case 0x17: case 0x18: case 0x19: case 0x1a: case 0x1c:
			case 0x1e: case 0x1f:
				break;
			default:
				output_raw_char_dc(v, vt52, c);
				break;
			}
		}
	}
	return VDI_DONE;
}

/******************************************************************************/
/* Escape functions                                                           */
/******************************************************************************/

/*
 * They are mostly identical to what is done in the VT 52 emulator.
 * In fact, it might be that we have to use the physical screen workstation here,
 * to share cursor location etc (seems that is how the original ROM-TOS works).
 * Theoretically, we also have to update the negative Line-A variables with
 * this information.
 */

static int vdi_vq_chcells(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	struct alpha_info *vt52;

	V("vq_chcells[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 2);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		V_INTOUT(pb, 0) = vt52->ah;
		V_INTOUT(pb, 1) = vt52->aw;
	} else
	{
		V_INTOUT(pb, 0) = v->height / sysfont[v->font_index].cellheight;
		V_INTOUT(pb, 1) = v->width / sysfont[v->font_index].cellwidth;
	}
	return VDI_DONE;
}

/******************************************************************************/

static void v_disable_alpha_cursor(VWK *v)
{
	struct alpha_info *vt52;

	vt52 = v->vt52;
	vt52->curs_hid_cnt++;
}

static int vdi_v_exit_cur(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	struct alpha_info *vt52;
	
	V("v_exit_cur[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		v_disable_alpha_cursor(v);
		v_reset_alpha_cursor(v);
		fill_rectangle_color(v, 0, 0, v->width, v->height, vt52->abg);
	}
	return VDI_DONE;
}

/******************************************************************************/

static void v_enable_alpha_cursor(VWK *v)
{
	struct alpha_info *vt52;

	vt52 = v->vt52;
	vt52->curs_hid_cnt = 0;
}

static int vdi_v_enter_cur(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	struct alpha_info *vt52;
	
	V("v_enter_cur[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		v->clipping = v->inq_tab.clipping = FALSE;
		set_clipping(v);
		v->wrmode = MD_REPLACE;
		vt52->rev_on = FALSE;
		vt52->ax = vt52->ay = 0;
		vt52->asavex = vt52->asavey = 0;
		{
			v_reset_alpha_cursor(v);
			v_enable_alpha_cursor(v);
			fill_rectangle_color(v, 0, 0, v->width, v->height, vt52->abg);
			v_show_alpha_cursor(v);
		}
	}
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_curup(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	struct alpha_info *vt52;

	V("v_curup[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		/* unlike VT52 cursor up, this does not scroll screen down when at top */
		if (vt52->ay > 0)
		{
			v_hide_alpha_cursor(v);
			vt52->ay--;
			v_show_alpha_cursor(v);
		}
	}
		
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_curdown(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	struct alpha_info *vt52;
	
	V("v_curdown[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		/* unlike VT52 cursor down, this does not scroll screen up when at bottom */
		if (vt52->ay < vt52->ah - 1)
		{
			v_hide_alpha_cursor(v);
			vt52->ay++;
			v_show_alpha_cursor(v);
		}
	}
	
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_curright(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	struct alpha_info *vt52;

	V("v_curright[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		if (vt52->ax < vt52->aw - 1)
		{
			v_hide_alpha_cursor(v);
			vt52->ax++;
			v_show_alpha_cursor(v);
		}
	}
	
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_curleft(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	struct alpha_info *vt52;

	V("v_curleft[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		if (vt52->ax > 0)
		{
			v_hide_alpha_cursor(v);
			vt52->ax--;
			v_show_alpha_cursor(v);
		}
	}
		
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_curhome(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	struct alpha_info *vt52;
	
	V("v_curhome[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		v_hide_alpha_cursor(v);
		vt52->ax = vt52->ay = 0;
		v_show_alpha_cursor(v);
	}
	
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_eeos(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	struct alpha_info *vt52;
	
	V("v_eeos[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		v_reset_alpha_cursor(v);
		if (vt52->ax == 0)
		{
			fill_rectangle_color(v, 0, vt52->ay * vt52->ach, v->width, v->height - vt52->ay * vt52->ach, vt52->abg);
		} else
		{
			fill_rectangle_color(v, vt52->ax * vt52->acw, vt52->ay * vt52->ach, v->width - vt52->ax * vt52->acw, vt52->ach, vt52->abg);
			if (vt52->ay < (vt52->ah - 1))
			{
				fill_rectangle_color(v, 0, (vt52->ay + 1) * vt52->ach, v->width, v->height - (vt52->ay + 1) * vt52->ach, vt52->abg);
			}
		}
		v_show_alpha_cursor(v);
	}
	
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_eeol(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	struct alpha_info *vt52;
	
	V("v_eeol[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		v_reset_alpha_cursor(v);
		fill_rectangle_color(v, vt52->ax * vt52->acw, vt52->ay * vt52->ach, v->width - vt52->ax * vt52->acw, vt52->ach, vt52->abg);
		v_show_alpha_cursor(v);
	}
	
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vs_curaddress(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	struct alpha_info *vt52;
	
	V("vs_curaddress[%d]: %d, %d", v->handle, V_INTIN(pb, 0), V_INTIN(pb, 1));

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);
	
	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		v_hide_alpha_cursor(v);
		/* strange, but coordinates are base-1 here */
		vt52->ay = MAX(0, MIN(vt52->ah - 1, V_INTIN(pb, 0) - 1));
		vt52->ax = MAX(0, MIN(vt52->aw - 1, V_INTIN(pb, 1) - 1));
		v_show_alpha_cursor(v);
	}
	
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_curtext(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	struct alpha_info *vt52;
	int x, y;
	int n;
	_WORD *items;
	
	V("v_curtext[%d]: '%s'", v->handle, ascii_text(V_NINTIN(pb), &V_INTIN(pb, 0)));

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		x = vt52->ax * vt52->acw;
		y = vt52->ay * vt52->ach;
		n = V_NINTIN(pb);
		items = (_WORD *)&V_INTIN(pb, 0);
		{
			pel fg, bg;
			
			v_reset_alpha_cursor(v);
			if (vt52->rev_on)
			{
				fg = FIX_COLOR(vt52->abg);
				bg = FIX_COLOR(vt52->afg);
			} else
			{
				fg = FIX_COLOR(vt52->afg);
				bg = FIX_COLOR(vt52->abg);
			}
			(void) x;
			(void) y;
			(void) items;
			(void) n;
			(void) fg;
			(void) bg;
			XBUF(XDrawImageString16, x, y, (XChar2b *)items, n);
			v_show_alpha_cursor(v);
		}
	}
	
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_rvon(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	struct alpha_info *vt52;

	V("v_rvon[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		vt52->rev_on = TRUE;
	}
	
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_rvoff(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	struct alpha_info *vt52;

	V("v_rvoff[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		vt52->rev_on = FALSE;
	}
	
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_curaddress(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	struct alpha_info *vt52;

	V("vq_curaddress[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 2);

	v = vwk[v->phys_wk];
	vt52 = v->vt52;
	if (vt52 != NULL)
	{
		V_INTOUT(pb, 0) = vt52->ay + 1;
		V_INTOUT(pb, 1) = vt52->ax + 1;
	} else
	{
		V_INTOUT(pb, 0) = 0;
		V_INTOUT(pb, 1) = 0;
	}
	
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_tabstatus(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);

	V("vq_tabstatus[%d]", v->handle);

	V_INTOUT(pb, 0) = v->inq_tab.mouse_buttons > 0;
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 1);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_hardcopy(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("v_hardcopy[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);
	if (IS_SCREEN_V(v))
	{
		sys_Scrdmp();
	}
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_dspcur(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	
	V("v_dspcur[%d]", v->handle);

	if (IS_SCREEN_V(v))
	{
		GCURX = V_PTSINX(pb, 0);
		GCURY = V_PTSINY(pb, 0);
		linea_showmouse(0);
	}
	
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_rmcur(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("v_rmcur[%d]", v->handle);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);
	if (IS_SCREEN_V(v))
	{
		hide_cursor();
	}
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_form_adv(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("v_form_adv[%d]", v->handle);
	UNUSED(v);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);
	/* nothing to do for screen driver */
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_output_window(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("v_output_window[%d]", v->handle);
	UNUSED(v);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);
	/* nothing to do for screen driver */
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_clear_disp_list(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("v_clear_disp_list[%d]", v->handle);
	UNUSED(v);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);
	/* nothing to do for screen driver */
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_bit_image(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("v_bit_image[%d]", v->handle);
	UNUSED(v);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);
	/* nothing to do for screen driver */
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vs_calibrate(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
#if TRACE_VDI
	_WORD *intin = PV_INTIN(pb);
#endif

	V("vs_calibrate[%d]: %d,$%x", v->handle, V_INTIN(pb, 2), vdi_intin_long(0));
	UNUSED(v);

	V_INTOUT(pb, 0) = 0; /* calibration not available */
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 1);
	/* nothing to do for screen driver */
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_calibrate(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);

	V("vq_calibrate[%d]", v->handle);
	UNUSED(v);

	V_INTOUT(pb, 0) = 0; /* calibration not available */
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);
	/* nothing to do for screen driver */
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_offset(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	struct alpha_info *vt52;
	
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);

	if (V_NINTIN(pb) == 1)
	{
		V("v_offset[%d]: %d", v->handle, V_INTIN(pb, 0));
	
		v = vwk[v->phys_wk];
		vt52 = v->vt52;
		if (vt52 != NULL)
		{
			v_hide_alpha_cursor(v);
			v_disable_alpha_cursor(v);
			vt52->v_cur_of = V_INTIN(pb, 0);
		}
	} else
	{
		V("v_xbit_image[%d]: NOT YET IMPLEMENENTED", v->handle);
	}	
	return VDI_DONE;
}

/******************************************************************************/
/* Line Functions                                                             */
/******************************************************************************/

static int vdi_vsl_udsty(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_UWORD m = V_INTIN(pb, 0);

	V("vsl_udsty[%d]: $%04x", v->handle, m);

	v->ud_linepat = m;
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsl_type(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int m = V_INTIN(pb, 0);

	V("vsl_type[%d]: %d", v->handle, m);

	if (m < 1 || m > LT_MAX)
		m = SOLID;
	v->line_type = m;
	V_INTOUT(pb, 0) = m;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vql_attributes(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);

	V("vql_attributes[%d]", v->handle);

	V_INTOUT(pb, 0) = v->line_type;
	V_INTOUT(pb, 1) = v->line_color;
	V_INTOUT(pb, 2) = v->wrmode;
	V_INTOUT(pb, 3) = v->line_ends & 3;
	V_INTOUT(pb, 4) = (v->line_ends >> 2) & 3;
	PTSOUT(0) = v->line_width;
	PTSOUT(1) = 0;
	V_NINTOUT(pb, 5);
	V_NPTSOUT(pb, 1);
	return VDI_DONE;
}

/******************************************************************************/

/* #define SMUL_DIV(x,y,z)	((short)(((short)(x)*(long)((short)(y)))/(short)(z))) */

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


#define vec_len(x, y) (int)sqrt((x) * (x) + (y) * (y))

static void arrow(VWK *v, VDI_POINT *xy, int inc, int npoints)
{
	int arrow_len, arrow_wid, line_len;
	VDI_POINT triangle[4];						/* triangle 2 high to close polygon */
	_WORD dx, dy;
	_WORD base_x, base_y, ht_x, ht_y;
	int xybeg;
	int i;

	if (npoints <= 1)
		return;
	
	/* Set up the arrow-head length and width as a function of line width. */

	arrow_len = (v->line_width <= 1) ? 8 : 3 * v->line_width - 1;
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
		dy = SMUL_DIV(xy[0].y - xy[xybeg].y, v->dev_tab.pix_height, v->dev_tab.pix_width);

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

	/* Transform the y offsets back to the correct aspect ratio space. */

	ht_y = SMUL_DIV(ht_y, v->dev_tab.pix_width, v->dev_tab.pix_height);
	base_y = SMUL_DIV(base_y, v->dev_tab.pix_width, v->dev_tab.pix_height);

	/* Build a polygon to send to plygn.  Build into a local array first since */
	/* xy will probably be pointing to the PTSIN array.                        */

	triangle[0].x = xy[0].x + base_x - ht_x;
	triangle[0].y = xy[0].y + base_y - ht_y;
	triangle[1].x = xy[0].x - base_x - ht_x;
	triangle[1].y = xy[0].y - base_y - ht_y;
	triangle[2].x = xy[0].x;
	triangle[2].y = xy[0].y;
	vdi_draw_polygons(v->gc, triangle, 3);

	/* Adjust the end point and all points skipped. */

	xy[0].x -= ht_x;
	xy[0].y -= ht_y;

	while ((xybeg -= inc) != 0)
	{
		xy[xybeg].x = xy[0].x;
		xy[xybeg].y = xy[0].y;
	}
}

static void do_arrow(VWK *v, VDI_POINT *xy, int npoints)
{
	int fill_color, line_ends;
	int x_start, y_start, new_x_start, new_y_start;
	
	/* Set up the attribute environment. */

	fill_color = v->fill_color;
	v->fill_color = v->line_color;
	line_ends = v->line_ends;
	v->line_ends = 0;
	{
		struct fillparams params;
	
		init_filled(v, &params, v->fill_color);
		
		/* Function "arrow" will alter the end of the line segment.  Save the */
		/* starting point of the polyline in case two calls to "arrow" are    */
		/* necessary.                                                         */
		new_x_start = x_start = xy[0].x;
		new_y_start = y_start = xy[0].y;
		
		if (line_ends & ARROWED)
		{
			arrow(v, xy, 1, npoints);
			new_x_start = xy[0].x;
			new_y_start = xy[0].y;
		}
		
		if (line_ends & (ARROWED << 2))
		{
			xy[0].x = x_start;
			xy[0].y = y_start;
			arrow(v, xy + npoints - 1, -1, npoints);
			xy[0].x = new_x_start;
			xy[0].y = new_y_start;
		}
		
		/* Restore the attribute environment. */
		v->fill_color = fill_color;
		v->line_ends = line_ends;
	}
}


static _UWORD get_line_pattern(VWK *v)
{
	_UWORD line_pattern;
	
	switch (v->line_type)
	{
	case SOLID:
		line_pattern = 0xffff;
		break;
	case LONGDASH:
		line_pattern = 0xfff0;
		break;
	case DOT:
		line_pattern = 0xc0c0;
		break;
	case DASHDOT:
		line_pattern = 0xff18;
		break;
	case DASH:
		line_pattern = 0xff00;
		break;
	case DASH2DOT:
		line_pattern = 0xf191;
		break;
	case USERLINE:
		line_pattern = v->ud_linepat;
		break;
	default:
		line_pattern = 0xffff;
		break;
	}
	return line_pattern;
}

static void draw_pline(VWK *v, VDI_POINT *points, int n)
{
	vdi_draw_lines(v, points, n, get_line_pattern(v), v->wrmode, FIX_COLOR(v->line_color), FIX_COLOR(v->bg_color));
}

#define IS_BEZ(f) (((f) & BEZIER_START) != 0)
#define IS_JUMP(f) (((f) & POINT_MOVE) != 0)

static void do_pline(VWK *v, VDI_POINT *points, int n)
{
	if (v->line_width > 1 && (v->line_ends & (ARROWED | (ARROWED << 2))))
		do_arrow(v, points, n);
	
	{
		draw_pline(v, points, n);
	}
	
	if (v->line_width <= 1 && (v->line_ends & (ARROWED | (ARROWED << 2))))
		do_arrow(v, points, n);
}


static void do_fillarea(VWK *v, VDI_POINT *points, int n)
{
	struct fillparams params;
	
	init_filled(v, &params, v->fill_color);
	/* TODO */
	(void) points;
	(void) n;
	vdi_draw_polygons(v->gc, points, n);
}


static void draw_segs(VWK *v, int nr_vertices, VDI_POINT *points, gboolean fill)
{
    if (nr_vertices >= 2)
    {
        /* output to driver, converting ndc if necessary */
        if (fill)
        {
            do_fillarea(v, points, nr_vertices);
        } else
        {
        	do_pline(v, points, nr_vertices);
        }
    }
}


static int32_t _labs(int32_t x)
{
	if (x < 0)
		return -x;
	return x;
}


/*
 * gen_segs -  compute bezier function by difference method
 *
 * last point is included
 * one dimension only, so use alternate elements of array & px
 * array[0] : anchor 1
 * array[2] : control 1
 * array[4] : control 2
 * array[6] : anchor 2
 */
static void gen_segs(VDI_POINT *array, VDI_POINT *px, const gboolean for_x, const int bez_qual, short *pmin, short *pmax, short xfm_mode)
{
	int32_t d3x, d2x, d1x;
	int q = 3 * bez_qual;
	int qd = 0;
	int i;
	short x;
	int32_t x0 = for_x ? array[0].x : array[0].y;
	int32_t x1 = for_x ? array[1].x : array[1].y;
	int32_t x2 = for_x ? array[2].x : array[2].y;
	int32_t x3 = for_x ? array[3].x : array[3].y;
	
	/*** calculate 1st, 2nd & 3rd order differences ***/
	d1x = x1 - x0;
	d2x = x2 - x1;
	d3x = -x0 - 3 * d2x + x3;
	d2x -= d1x;

	if (xfm_mode == 0 && q >= 3)
	{
		d1x >>= 1;
		d2x >>= 1;
		d3x >>= 1;
		q--;
	}

	d1x = ((3L * d1x) << (2 * bez_qual)) + ((3L * d2x) << bez_qual) + d3x;

	d3x = 6L * d3x;

	d2x = ((6L * d2x) << bez_qual) + d3x;

	x0 = _labs(x0);
	while (x0 >= (0x7fffffffL >> q))
		q--, qd++;

	x0 = (x0 << q) + (1L << (q - 1));

	for (i = 1 << bez_qual; i > 0; i--)
	{
		x = (short)(x0 >> q);
		if (for_x)
			px->x = x;
		else
			px->y = x;
		if (x < *pmin)
			*pmin = x;
		if (x > *pmax)
			*pmax = x;
		px += 1;

		if (_labs((x0 >> 1) + (d1x >> (qd + 1)) ) >= 0x3ffffffeL)
		{
			/** halve scale to avoid overflow **/
			x0 = x0 >> 1;
			q--;
			qd++;
		}

		x0 += d1x >> qd;
		if ( qd > 0 && _labs(x0) < 0x40000000L )
		{
			/** double scale to maximise accuracy **/
			x0 = (x0 << 1)|1;
			q++, qd--;
		}

		d1x += d2x;
		d2x += d3x;
	}

	/** add the last point .. */
	x = x3;
	if (for_x)
		px->x = x;
	else
		px->y = x;
	if (x < *pmin)
		*pmin = x;
	if (x > *pmax)
		*pmax = x;
}


static void do_bez_line(VWK *v, struct v_bez_pars *par)
{
	VDI_POINT *ptsget = par->points;
	const char *bezarr = par->bezarr;
	int nr_ptsin = par->num_pts;
	int total_vertices = nr_ptsin;
	short total_jumps = 0;
	short xmin, xmax, ymin, ymax;
	VDI_POINT ptsbuf[MAX_POINTS];
	VDI_POINT *ptsput = ptsbuf;
	int bez_qual;
	unsigned short vertices_per_bez;
	int i;
	
	bez_qual = v->bezier.qual;
	vertices_per_bez = 1 << bez_qual;
	xmin = ymin = 32767;
	xmax = ymax = 0;
	
	i = 0;
	while (i < nr_ptsin)
	{
		int flag = bezarr[i];

		if (IS_BEZ(flag))
		{
			/* bezier start point found */
			if ((i + 3) >= nr_ptsin)
				break;					/* incomplete curve, omit it */

			if (IS_JUMP(flag))
				total_jumps++;			/* count jump point */

			/* generate line segments from bez points */
			gen_segs(ptsget, ptsput, TRUE, bez_qual, &xmin, &xmax, v->xfm_mode);	/* x coords */
			gen_segs(ptsget, ptsput, FALSE, bez_qual, &ymin, &ymax, v->xfm_mode);	/* y coords */

			/* skip to coord pairs at end of bez curve */
			i += 3;
			ptsget += 3;
			total_vertices += vertices_per_bez - 3;
			draw_segs(v, vertices_per_bez + 1, ptsbuf, FALSE);
		} else
		{
			/* polyline */
			short output_vertices = 0;
			VDI_POINT *point = ptsget;
			
			do
			{
				int t;

				t = point->x;
				if (t < xmin)
					xmin = t;
				if (t > xmax)
					xmax = t;

				t = point->y;
				if (t < ymin)
					ymin = t;
				if (t > ymax)
					ymax = t;

				output_vertices++;
				if (IS_BEZ(flag))
					break;				/* stop if a jump point is next */

				/* continue polyline */
				i++;
				if (i >= nr_ptsin)
					break;

				ptsget += 1;
				/* continue polyline, stop if a jump point is next */
				{
					int old_flag = flag;
					flag = bezarr[i];
					if (!IS_JUMP(old_flag) && IS_JUMP(flag))
						total_jumps++;		/* count jump point */
				}
			} while (!IS_JUMP(flag));
			draw_segs(v, output_vertices, point, FALSE);
		}
	}

	par->totpoints = total_vertices;
	par->totmoves = total_jumps;
	par->extent[0] = xmin;
	par->extent[1] = ymin;
	par->extent[2] = xmax;
	par->extent[3] = ymax;
}


static int maybe_bez_line(VWK *v, VDIPB *pb, gboolean bezon)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	struct v_bez_pars _pars;
	struct v_bez_pars *par = &_pars;
	int i;
	int n;
	VDI_POINT points[MAX_POINTS];
	char bezarr[MAX_POINTS];
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);

	n = V_NPTSIN(pb);
	for (i = 0; i < n && i < MAX_POINTS; i++)
	{
		VDI_SET_POINT(points[i], V_PTSINX(pb, i), V_PTSINY(pb, i));
	}
	par->points = points;
	par->num_pts = i;
	n = V_NINTIN(pb) * 2;
	if (bezon && n != 0)
	{
		par->bezarr = bezarr;
		par->clipr = v->clipr;
		for (i = 0; i < n && i < MAX_POINTS; i++)
		{
			bezarr[i] = V_INTIN(pb, i ^ 1);		/* Byte swapped! */
		}
		
		do_bez_line(v, par);
		
		V_INTOUT(pb, 0) = par->totpoints;
		V_INTOUT(pb, 1) = par->totmoves;
		V_INTOUT(pb, 2) = 0;
		V_INTOUT(pb, 3) = 0;
		V_INTOUT(pb, 4) = 0;
		V_INTOUT(pb, 5) = 0;
		PTSOUTX(0) = par->extent[0];
		PTSOUTY(0) = par->extent[1];
		PTSOUTX(1) = par->extent[2];
		PTSOUTY(1) = par->extent[3];
		V_NINTOUT(pb, 6);
		V_NPTSOUT(pb, 2);
	} else
	{
		do_pline(v, par->points, par->num_pts);
	}
	
	return VDI_DONE;
}


static int vdi_v_pline(VWK *v, VDIPB *pb)
{
#if TRACE_VDI
	_WORD *control = PV_CONTROL(pb);
#endif
	V("v_pline[%d]: %d", v->handle, V_NPTSIN(pb));
	return maybe_bez_line(v, pb, v->bezier.on);
}

static int vdi_v_bez(VWK *v, VDIPB *pb)
{
#if TRACE_VDI
	_WORD *control = PV_CONTROL(pb);
#endif
	V("v_bez[%d]: %d", v->handle, V_NPTSIN(pb));
	return maybe_bez_line(v, pb, TRUE);
}

/******************************************************************************/

static int vdi_v_set_app_buff(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("v_set_app_buff[%d]", v->handle);
	UNUSED(v);

	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);
	/* nothing to do for screen driver */
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_bez_con(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	gboolean on = V_NPTSIN(pb) != 0; /* strange binding... */
	
	V("v_bez_con[%d]: %d", v->handle, on);
	
	v->bezier.on = on;
	if (on)
	{
		V_INTOUT(pb, 0) = v->bezier.qual;
		V_NINTOUT(pb, 1);
	} else
	{
		V_NINTOUT(pb, 0);
	}
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_meta_esc(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	
	if (V_NINTIN(pb) >= 3 && V_INTIN(pb, 0) == 32)
	{
		_WORD q = V_INTIN(pb, 2);
		V("v_bez_qual[%d]: %d", v->handle, q);
		if (q >= 95)
			q = MAX_BEZIER_QUAL;
		else if (q < 5)
			q = MIN_BEZIER_QUAL;
		else
			q = (q >> 4) + 1;
		v->bezier.qual = q;
		V_INTOUT(pb, 0) = bez_quals[q - MIN_BEZIER_QUAL];
		V_NINTOUT(pb, 1);
	} else if (V_NINTIN(pb) >= 3 && V_INTIN(pb, 0) == 0)
	{
		V("vm_pagesize[%d]: %d,%d", v->handle, V_INTIN(pb, 1), V_INTIN(pb, 2));
		/* nothing to do for screen driver */
		V_NINTOUT(pb, 0);
	} else if (V_NINTIN(pb) >= 5 && V_INTIN(pb, 0) == 1)
	{
		V("vm_coords[%d]: %d,%d,%d,%d", v->handle, V_INTIN(pb, 1), V_INTIN(pb, 2), V_INTIN(pb, 3), V_INTIN(pb, 4));
		/* nothing to do for screen driver */
		V_NINTOUT(pb, 0);
	} else if (V_NINTIN(pb) >= 1)
	{
		V("v_write_meta[%d]: %d,[%d],[%d]", v->handle, V_INTIN(pb, 0), V_NINTIN(pb), V_NPTSIN(pb));
		/* nothing to do for screen driver */
		V_NINTOUT(pb, 0);
	} else
	{
		V("v_write_meta[%d]: error: no sub-opcode", v->handle);
		V_NINTOUT(pb, 0);
	}
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsl_width(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	int w = V_PTSIN(pb, 0);

	V("vsl_width[%d]: %d", v->handle, w);
	
	if (w < v->siz_tab.min_line_width)
		w = v->siz_tab.min_line_width;
	else if (w > v->siz_tab.max_line_width)
		w = v->siz_tab.max_line_width;
	v->line_width = w;
	PTSOUTX(0) = w;
	PTSOUTY(0) = 0;
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 1);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsl_color(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int c = V_INTIN(pb, 0);

	V("vsl_color[%d]: %d", v->handle, c);

	if (c < 0 || c >= v->dev_tab.num_colors)
		c = BLACK;
	v->line_color = c;
	V_INTOUT(pb, 0) = c;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsl_ends(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int beg = V_INTIN(pb, 0);
	int end = V_INTIN(pb, 1);

	V("vsl_ends[%d]: %d %d", v->handle, beg, end);

	if (beg < 0 || beg > 2)
		beg = 0;
	if (end < 0 || end > 2)
		end = 0;
	v->line_ends = ((unsigned int) end << 2) + ((unsigned int) beg);
	V_INTOUT(pb, 0) = beg;
	V_INTOUT(pb, 1) = end;
	V_NINTOUT(pb, 2);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/
/* Marker Functions                                                           */
/******************************************************************************/

static void calc_marker_scale(VWK *v)
{
	v->marker_scale = (v->marker_height + MIN_MKHT / 2) / MIN_MKHT;
}

/******************************************************************************/

static int vdi_vqm_attributes(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);

	V("vqm_attributes[%d]", v->handle);

	V_INTOUT(pb, 0) = v->marker_type;
	V_INTOUT(pb, 1) = v->marker_color;
	V_INTOUT(pb, 2) = v->wrmode;
	PTSOUT(0) = v->marker_height;
	PTSOUT(1) = v->marker_height;
	V_NINTOUT(pb, 3);
	V_NPTSOUT(pb, 1);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsm_type(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int m = V_INTIN(pb, 0);

	V("vsm_type[%d]: %d", v->handle, m);

	if (m < 1 || m > PM_MAX)
		m = PM_ASTERISK;
	v->marker_type = m;
	V_INTOUT(pb, 0) = m;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsm_color(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int c = V_INTIN(pb, 0);

	V("vsm_color[%d]: %d", v->handle, c);

	if (c < 0 || c >= v->dev_tab.num_colors)
		c = BLACK;
	v->marker_color = c;
	V_INTOUT(pb, 0) = c;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsm_height(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	int height = V_PTSINY(pb, 0);

	V("vsm_height[%d]: %d", v->handle, height);

	if (v->marker_type != PM_DOT)
	{
		if (height < MIN_MKHT)
		{
			height = MIN_MKHT;
		} else if (height > MAX_MKHT)
		{
			height = MAX_MKHT;
		}
	}
	v->marker_height = height;
	calc_marker_scale(v);
	PTSOUT(0) = v->marker_scale * MIN_MKWD;
	PTSOUT(1) = v->marker_scale * MIN_MKHT;
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 1);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_pmarker(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	int i;
	int n = V_NPTSIN(pb);
	VDI_POINT points[MAX_POINTS];
	int x, y;
	
	V("v_pmarker[%d]: %d", v->handle, n);

	{
		switch (v->marker_type)
		{
		case PM_DOT:
			for (i = 0; i < n; i++)
			{
				x = V_PTSINX(pb, i);
				y = V_PTSINY(pb, i);
				gfx_put_pixel(v, x, y, 0xffff, v->wrmode, FIX_COLOR(v->marker_color), FIX_COLOR(v->bg_color));
			}
			break;
		case PM_PLUS:
			for (i = 0; i < n; i++)
			{
				x = V_PTSINX(pb, i);
				y = V_PTSINY(pb, i);
				points[0].x = x;
				points[0].y = y - 3 * v->marker_scale;
				points[1].x = x;
				points[1].y = y + 3 * v->marker_scale;
				vdi_draw_lines(v, points, 2, 0xffff, v->wrmode, FIX_COLOR(v->marker_color), FIX_COLOR(v->bg_color));
				points[0].x = x - 4 * v->marker_scale;
				points[0].y = y;
				points[1].x = x + 4 * v->marker_scale;
				points[1].y = y;
				vdi_draw_lines(v, points, 2, 0xffff, v->wrmode, FIX_COLOR(v->marker_color), FIX_COLOR(v->bg_color));
			}
			break;
		case PM_ASTERISK:
			for (i = 0; i < n; i++)
			{
				x = V_PTSINX(pb, i);
				y = V_PTSINY(pb, i);
				points[0].x = x;
				points[0].y = y - 3 * v->marker_scale;
				points[1].x = x;
				points[1].y = y + 3 * v->marker_scale;
				vdi_draw_lines(v, points, 2, 0xffff, v->wrmode, FIX_COLOR(v->marker_color), FIX_COLOR(v->bg_color));
				points[0].x = x + 3 * v->marker_scale;
				points[0].y = y + 2 * v->marker_scale;
				points[1].x = x - 3 * v->marker_scale;
				points[1].y = y - 2 * v->marker_scale;
				vdi_draw_lines(v, points, 2, 0xffff, v->wrmode, FIX_COLOR(v->marker_color), FIX_COLOR(v->bg_color));
				points[0].x = x + 3 * v->marker_scale;
				points[0].y = y - 2 * v->marker_scale;
				points[1].x = x - 3 * v->marker_scale;
				points[1].y = y + 2 * v->marker_scale;
				vdi_draw_lines(v, points, 2, 0xffff, v->wrmode, FIX_COLOR(v->marker_color), FIX_COLOR(v->bg_color));
			}
			break;
		case PM_SQUARE:
			for (i = 0; i < n; i++)
			{
				x = V_PTSINX(pb, i);
				y = V_PTSINY(pb, i);
				points[0].x = x - 4 * v->marker_scale;
				points[0].y = y - 3 * v->marker_scale;
				points[1].x = x + 4 * v->marker_scale;
				points[1].y = y - 3 * v->marker_scale;
				points[2].x = x + 4 * v->marker_scale;
				points[2].y = y + 3 * v->marker_scale;
				points[3].x = x - 4 * v->marker_scale;
				points[3].y = y + 3 * v->marker_scale;
				points[4].x = points[0].x;
				points[4].y = points[0].y;
				vdi_draw_lines(v, points, 5, 0xffff, v->wrmode, FIX_COLOR(v->marker_color), FIX_COLOR(v->bg_color));
			}
			break;
		case PM_DIAGCROSS:
			for (i = 0; i < n; i++)
			{
				x = V_PTSINX(pb, i);
				y = V_PTSINY(pb, i);
				points[0].x = x - 4 * v->marker_scale;
				points[0].y = y - 3 * v->marker_scale;
				points[1].x = x + 4 * v->marker_scale;
				points[1].y = y + 3 * v->marker_scale;
				vdi_draw_lines(v, points, 2, 0xffff, v->wrmode, FIX_COLOR(v->marker_color), FIX_COLOR(v->bg_color));
				points[0].x = x - 4 * v->marker_scale;
				points[0].y = y + 3 * v->marker_scale;
				points[1].x = x + 4 * v->marker_scale;
				points[1].y = y - 3 * v->marker_scale;
				vdi_draw_lines(v, points, 2, 0xffff, v->wrmode, FIX_COLOR(v->marker_color), FIX_COLOR(v->bg_color));
			}
			break;
		case PM_DIAMOND:
			for (i = 0; i < n; i++)
			{
				x = V_PTSINX(pb, i);
				y = V_PTSINY(pb, i);
				points[0].x = x - 4 * v->marker_scale;
				points[0].y = y;
				points[1].x = x;
				points[1].y = y - 3 * v->marker_scale;
				points[2].x = x + 4 * v->marker_scale;
				points[2].y = y;
				points[3].x = x;
				points[3].y = y + 3 * v->marker_scale;
				points[4].x = points[0].x;
				points[4].y = points[0].y;
				vdi_draw_lines(v, points, 5, 0xffff, v->wrmode, FIX_COLOR(v->marker_color), FIX_COLOR(v->bg_color));
			}
			break;
		}
	}
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/
/* Fill functions                                                             */
/******************************************************************************/

static int vdi_vsf_color(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int c = V_INTIN(pb, 0);

	V("vsf_color[%d]: %d", v->handle, c);

	if (c < 0 || c >= v->dev_tab.num_colors)
		c = BLACK;
	v->fill_color = c;
	V_INTOUT(pb, 0) = c;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsf_interior(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int i = V_INTIN(pb, 0);

	V("vsf_interior[%d]: %d", v->handle, i);

	if (i < 0 || i >= 5)
		i = FIS_HOLLOW;
	v->fill_interior = i;
	/* v->fill_style = 1; */
	V_INTOUT(pb, 0) = i;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsf_style(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int s = V_INTIN(pb, 0);

	V("vsf_style[%d]: %d", v->handle, s);

	/*
	 * can't check validity here because we don't know
	 * which one of vsf_interior and vsf_style is called first;
	 * make sure to check for valid values before using them
	 */
#if 0
	if (s < 1 || s > patnum[v->fill_interior])
		s = 1;
#endif
	v->fill_style = s;
	V_INTOUT(pb, 0) = s;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsf_perimeter(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int p = V_INTIN(pb, 0);

	V("vsf_perimeter[%d]: %d", v->handle, p);

	if (V_NINTIN(pb) == 1 || p != -1)
		v->fill_perimeter = p;
	if (V_NINTIN(pb) >= 2)
		v->fill_perimeter_whichcolor = V_INTIN(pb, 1);
	
	V_INTOUT(pb, 0) = v->fill_perimeter;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vsf_udpat(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	const _UWORD *pattern = (const _UWORD *)(&V_INTIN(pb, 0));
	
	V("vsf_udpat[%d]", v->handle);

	/*
	 * NYI: patterns with more than 1 plane
	 */
	create_pattern_pixmap(pattern, v->ud_fill_pattern);
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vqf_attributes(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);

	V("vqf_attributes[%d]", v->handle);

	V_INTOUT(pb, 0) = v->fill_interior;
	V_INTOUT(pb, 1) = v->fill_color;
	V_INTOUT(pb, 2) = v->fill_style;
	V_INTOUT(pb, 3) = v->wrmode;
	V_INTOUT(pb, 4) = v->fill_perimeter;
	V_NINTOUT(pb, 5);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static void do_bez_fill(VWK *v, struct v_bez_pars *par)
{
	int i;
	VDI_POINT *ptsget = par->points;
	const char *bezarr = par->bezarr;
	int nr_ptsin = par->num_pts;
	short bez_qual;
	short xmin, xmax, ymin, ymax;
	short total_vertices = nr_ptsin;
	short total_jumps = 0;
	short vertices_per_bez;
	short output_vertices = 0;
	VDI_POINT ptsbuf[MAX_POINTS];
	VDI_POINT *ptsput = ptsbuf;

	bez_qual = v->bezier.qual;
	vertices_per_bez = 1 << bez_qual;
	xmin = ymin = 32767;
	xmax = ymax = 0;
	
	i = 0;
	while (i < nr_ptsin)
	{
		int flag = bezarr[i];

		if (IS_BEZ(flag))
		{
			/* bezier start point found */
			if ((i + 3) >= nr_ptsin)
				break;					/* incomplete curve, omit it */

			if (IS_JUMP(flag))
				total_jumps++;			/* count jump point */

			/* generate line segments from bez points */
			gen_segs(ptsget, ptsput, TRUE, bez_qual, &xmin, &xmax, v->xfm_mode);	/* x coords */
			gen_segs(ptsget, ptsput, FALSE, bez_qual, &ymin, &ymax, v->xfm_mode);	/* y coords */

			/* skip to coord pairs at end of bez curve */
			i += 3;
			ptsget += 3;
			total_vertices += vertices_per_bez - 3;
			output_vertices += vertices_per_bez + 1;
			ptsput = ptsbuf + output_vertices;
		} else
		{
			/* polyline */
			VDI_POINT *point = ptsget;

			do
			{
				int t;

				t = point->x;
				if (t < xmin)
					xmin = t;
				if (t > xmax)
					xmax = t;

				t = point->y;
				if (t < ymin)
					ymin = t;
				if (t > ymax)
					ymax = t;

				output_vertices++;
				if (IS_BEZ(flag))
					break;				/* stop if a jump point is next */

				/* continue polyline */
				i++;
				if (i >= nr_ptsin)
					break;

				ptsget += 1;
				/* continue polyline, stop if a jump point is next */
				{
					int old_flag = flag;
					flag = bezarr[i];
					if (!IS_JUMP(old_flag) && IS_JUMP(flag))
						total_jumps++;		/* count jump point */
				}
			} while (!IS_JUMP(flag));
		}

		/* draw segments and reset all vertex information */
		draw_segs(v, output_vertices, ptsbuf, TRUE);
		ptsput = ptsbuf;
		output_vertices = 0;
	}

	par->totpoints = total_vertices;
	par->totmoves = total_jumps;
	par->extent[0] = xmin;
	par->extent[1] = ymin;
	par->extent[2] = xmax;
	par->extent[3] = ymax;
}


static int maybe_bez_fill(VWK *v, VDIPB *pb, gboolean bezon)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	struct v_bez_pars _pars;
	struct v_bez_pars *par = &_pars;
	int i;
	int n;
	VDI_POINT points[MAX_POINTS];
	char bezarr[MAX_POINTS];
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);

	n = V_NPTSIN(pb);
	for (i = 0; i < n && i < MAX_POINTS; i++)
	{
		VDI_SET_POINT(points[i], V_PTSINX(pb, i), V_PTSINY(pb, i));
	}
	par->points = points;
	par->num_pts = i;
	n = V_NINTIN(pb) * 2;
	if (bezon && n != 0)
	{
		par->bezarr = bezarr;
		par->clipr = v->clipr;
		for (i = 0; i < n && i < MAX_POINTS; i++)
		{
			bezarr[i] = V_INTIN(pb, i ^ 1);		/* Byte swapped! */
		}
		
		do_bez_fill(v, par);
		
		V_INTOUT(pb, 0) = par->totpoints;
		V_INTOUT(pb, 1) = par->totmoves;
		V_INTOUT(pb, 2) = 0;
		V_INTOUT(pb, 3) = 0;
		V_INTOUT(pb, 4) = 0;
		V_INTOUT(pb, 5) = 0;
		PTSOUTX(0) = par->extent[0];
		PTSOUTY(0) = par->extent[1];
		PTSOUTX(1) = par->extent[2];
		PTSOUTY(1) = par->extent[3];
		V_NINTOUT(pb, 6);
		V_NPTSOUT(pb, 2);
	} else
	{
		do_fillarea(v, par->points, par->num_pts);
	}
	
	return VDI_DONE;
}


static int vdi_v_fillarea(VWK *v, VDIPB *pb)
{
#if TRACE_VDI
	_WORD *control = PV_CONTROL(pb);
#endif
	V("v_fillarea[%d]: %d", v->handle, V_NPTSIN(pb));
	return maybe_bez_fill(v, pb, v->bezier.on);
}


static int vdi_v_bez_fill(VWK *v, VDIPB *pb)
{
#if TRACE_VDI
	_WORD *control = PV_CONTROL(pb);
#endif
	V("v_bez_fill[%d]: %d", v->handle, V_NPTSIN(pb));
	return maybe_bez_fill(v, pb, TRUE);
}

/******************************************************************************/

static int vdi_vr_recfl(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	vdi_rectangle r;
	
	V("vr_recfl[%d]: %d,%d %d,%d", v->handle, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 2), V_PTSIN(pb, 3));

	if (make_rectangle(v, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 2), V_PTSIN(pb, 3), &r, v->width, v->height))
	{
		fill_rectangle_brush(v, r.x, r.y, r.width, r.height);
	}
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/
/* GDP functions                                                              */
/******************************************************************************/

static int vdi_v_bar(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	vdi_rectangle r;
	VDI_POINT p[5];
	int x1, y1, x2, y2;
	
	V("v_bar[%d]: %d,%d, %d,%d", v->handle, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 2), V_PTSIN(pb, 3));

	x1 = V_PTSINX(pb, 0);
	y1 = V_PTSINY(pb, 0);
	x2 = V_PTSINX(pb, 1);
	y2 = V_PTSINY(pb, 1);
	if (x1 > x2)
	{
		x1 = V_PTSINX(pb, 1);
		x2 = V_PTSINX(pb, 0);
	}
	if (y1 > y2)
	{
		y1 = V_PTSINY(pb, 1);
		y2 = V_PTSINY(pb, 0);
	}
	if (make_rectangle(v, x1, y1, x2, y2, &r, v->width, v->height))
	{
		fill_rectangle_brush(v, r.x, r.y, r.width, r.height);
		if (v->fill_perimeter)
		{
			int color = v->fill_perimeter_whichcolor == 0 ? v->fill_color : v->line_color;
			if (x1 == r.x)
			{
				p[0].x = r.x;
				p[0].y = r.y;
				p[1].x = p[0].x;
				p[1].y = r.y + r.height - 1;
				vdi_draw_lines(v, p, 2, 0xffff, v->wrmode, FIX_COLOR(color), FIX_COLOR(v->bg_color));
			}
			if (y2 == (int)(r.y + r.height - 1))
			{
				p[0].x = r.x + 1;
				p[0].y = y2;
				p[1].x = r.x + r.width - 1;
				p[1].y = p[0].y;
				vdi_draw_lines(v, p, 2, 0xffff, v->wrmode, FIX_COLOR(color), FIX_COLOR(v->bg_color));
			}
			if (x2 == (int)(r.x + r.width - 1))
			{
				p[0].x = x2;
				p[0].y = r.y;
				p[1].x = p[0].x;
				p[1].y = r.y + r.height - 2;
				vdi_draw_lines(v, p, 2, 0xffff, v->wrmode, FIX_COLOR(color), FIX_COLOR(v->bg_color));
			}
			if (y1 == r.y)
			{
				p[0].x = r.x + 1;
				p[0].y = r.y;
				p[1].x = r.x + r.width - 2;
				p[1].y = p[0].y;
				vdi_draw_lines(v, p, 2, 0xffff, v->wrmode, FIX_COLOR(color), FIX_COLOR(v->bg_color));
			}
		}
	}
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static void draw_arc(VWK *v, int x, int y, int xradius, int yradius, int beg_angle, int end_angle, int color)
{
	_UWORD line_pattern = get_line_pattern(v);

	beg_angle = beg_angle * 64 / 10;
	end_angle = end_angle * 64 / 10 - beg_angle;
	/* TODO */
	(void) x;
	(void) y;
	(void) xradius;
	(void) yradius;
	(void) color;
	(void) line_pattern;
	XBUF(XDrawArc, v->gc, x - xradius, y - yradius, xradius * 2, yradius * 2, beg_angle, end_angle);
}

static int vdi_v_arc(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	int x, y;
	int radius;
	int beg_angle, end_angle;
	
	V("v_arc[%d]: %d, %d, %d, %d, %d)", v->handle, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 6), V_INTIN(pb, 0), V_INTIN(pb, 1));

	x = V_PTSIN(pb, 0);
	y = V_PTSIN(pb, 1);
	radius = V_PTSIN(pb, 6);
	beg_angle = V_INTIN(pb, 0);
	end_angle = V_INTIN(pb, 1);
	draw_arc(v, x, y, radius, radius, beg_angle, end_angle, v->line_color);
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);

	return VDI_DONE;
}

/******************************************************************************/

static void filled_ellpie(VWK *v, int x, int y, int xradius, int yradius, int beg_angle, int end_angle)
{
	struct fillparams params;
	
	init_filled(v, &params, v->fill_color);
	/* TODO */
	XBUF(XFillArc, v->gc, x - xradius, y - yradius, 2 * xradius, 2 * yradius, beg_angle, end_angle);
	if (v->fill_perimeter)
	{
		int color = v->fill_perimeter_whichcolor == 0 ? v->fill_color : v->line_color;
		draw_arc(v, x, y, xradius, yradius, beg_angle, end_angle, color);
	}
}

static int vdi_v_pieslice(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	int radius;
	int x, y;
	int beg_angle, end_angle;
	
	V("v_piesclice[%d]: %d, %d, %d, %d, %d)", v->handle, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 6), V_INTIN(pb, 0), V_INTIN(pb, 1));

	{
		x = V_PTSIN(pb, 0);
		y = V_PTSIN(pb, 1);
		radius = V_PTSIN(pb, 6);
		beg_angle = V_INTIN(pb, 0);
		end_angle = V_INTIN(pb, 1);
		filled_ellpie(v, x, y, radius, radius, beg_angle, end_angle);
	}
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	
	return VDI_DONE;
}

/******************************************************************************/

static void filled_ellipse(VWK *v, int x, int y, int xradius, int yradius)
{
	struct fillparams params;
	
	init_filled(v, &params, v->fill_color);
	XBUF(XFillArc, v->gc, x - xradius, y - yradius, xradius * 2, yradius * 2, 0, 360 * 64);
	/* TODO */
	(void) x;
	(void) y;
	(void) xradius;
	(void) yradius;
	if (v->fill_perimeter)
	{
		int color = v->fill_perimeter_whichcolor == 0 ? v->fill_color : v->line_color;
		draw_arc(v, x, y, xradius, yradius, 0, 360 * 64, color);
	}
}

static int vdi_v_circle(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	int radius;
	int x, y;
	
	V("v_circle[%d]: %d, %d, %d", v->handle, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 4));

	{
		x = V_PTSIN(pb, 0);
		y = V_PTSIN(pb, 1);
		radius = V_PTSIN(pb, 4);
		filled_ellipse(v, x, y, radius, radius);
	}
		
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);

	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_ellipse(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	int xradius, yradius;
	int x, y;
	
	V("v_ellipse[%d]: %d, %d, %d, %d", v->handle, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 2), V_PTSIN(pb, 3));

	{
		x = V_PTSIN(pb, 0);
		y = V_PTSIN(pb, 1);
		xradius = V_PTSIN(pb, 2);
		yradius = V_PTSIN(pb, 3);
		filled_ellipse(v, x, y, xradius, yradius);
	}
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);

	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_ellarc(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	int x, y, xradius, yradius;
	int beg_angle, end_angle;
	
	V("v_ellarc[%d]: %d, %d, %d, %d, %d, %d", v->handle, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 2), V_PTSIN(pb, 3), V_INTIN(pb, 0), V_INTIN(pb, 1));

	{
		x = V_PTSIN(pb, 0);
		y = V_PTSIN(pb, 1);
		xradius = V_PTSIN(pb, 2);
		yradius = V_PTSIN(pb, 3);
		beg_angle = V_INTIN(pb, 0);
		end_angle = V_INTIN(pb, 1);
		draw_arc(v, x, y, xradius, yradius, beg_angle, end_angle, v->line_color);
	}
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);

	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_ellpie(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *ptsin = PV_PTSIN(pb);
	int x, y;
	int xradius, yradius;
	int beg_angle, end_angle;
	
	V("v_ellpie[%d]: %d, %d, %d, %d, %d, %d)", v->handle, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 2), V_PTSIN(pb, 3), V_INTIN(pb, 0), V_INTIN(pb, 1));

	{
		x = V_PTSIN(pb, 0);
		y = V_PTSIN(pb, 1);
		xradius = V_PTSIN(pb, 2);
		yradius = V_PTSIN(pb, 3);
		beg_angle = V_INTIN(pb, 0);
		end_angle = V_INTIN(pb, 1);
		filled_ellpie(v, x, y, xradius, yradius, beg_angle, end_angle);
	}
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

/*
 * Code for calculation of rounded boxes for v_rbox/v_rfbox;
 * borrowed from fVDI and adopted to X
 */

#if 0
static short arc_split = 16384;  /* 1/4 as many lines as largest ellipse axel radius in pixels */
static short arc_min = 16;       /* Minimum number of lines in an ellipse */
static short arc_max = 256;      /* Maximum */
#endif

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
	while (angle >= I_PI) {
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


/*
 * Return integer cos between -32767 and 32767.
 */
static short Icos(short angle) 
{
	return Isin(angle + I_HALFPI);
}


#if 0
static long clc_nsteps(long xrad, long yrad)
{
	long n_steps;

	if (xrad > yrad)
		n_steps = xrad;
	else
		n_steps = yrad;

#if 0
	n_steps = n_steps >> 2;

	if (n_steps < 16)
		n_steps = 16;
	else if (n_steps > MAX_ARC_CT)
		n_steps = MAX_ARC_CT;
#else
	n_steps = (n_steps * arc_split) >> 16;

	if (n_steps < arc_min)
		n_steps = arc_min;
	else if (n_steps > arc_max)
		n_steps = arc_max;
#endif
	
	return n_steps;
}
#endif


static void rounded_box(VWK *v, VDIPB *pb, int filled)
{
	_WORD *ptsin = PV_PTSIN(pb);
	vdi_rectangle r;
	short rdeltax, rdeltay;
	short xc, yc, xrad, yrad;
	short x1, y1, x2, y2;
	/* long n_steps; */
	int i, j;
	VDI_POINT points[21];
	
	if (make_rectangle(v, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 2), V_PTSIN(pb, 3), &r, v->width, v->height))
	{
		x1 = V_PTSIN(pb, 0);
		y1 = V_PTSIN(pb, 1);
		if (x1 <= V_PTSIN(pb, 2))
		{
			x2 = V_PTSIN(pb, 2);
		} else
		{
			x2 = x1;
			x1 = V_PTSIN(pb, 2);
		}
		if (y1 <= V_PTSIN(pb, 3))
		{
			y2 = V_PTSIN(pb, 3);
		} else 
		{
			y2 = y1;
			y1 = V_PTSIN(pb, 3);
		}

		rdeltax = (x2 - x1) / 2;
		rdeltay = (y2 - y1) / 2;
	
		xrad = v->width >> 6;
		if (xrad > rdeltax)
		    xrad = rdeltax;
	
		yrad = SMUL_DIV(xrad, v->dev_tab.pix_width, v->dev_tab.pix_height);
		if (yrad > rdeltay)
		{
		    yrad = rdeltay;
			xrad = SMUL_DIV(yrad, v->dev_tab.pix_height, v->dev_tab.pix_width);
		}
		yrad = -yrad;

#if 0
		n_steps = clc_nsteps(xrad, yrad);
#endif
		for (i = 0; i < 5; i++)
		{
			points[i].x = SMUL_DIV(Icos(900 - 225 * i), xrad, 32767);
			points[i].y = SMUL_DIV(Isin(900 - 225 * i), yrad, 32767);
		}

		xc = x2 - xrad;
		yc = y1 - yrad;
		for (i = 4, j = 5; i >= 0; i--, j++)
		{
		    points[j].y = yc + points[i].y;
		    points[j].x = xc + points[i].x;
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
		    points[j].y = yc - points[i].y;
		    points[j].x = xc - points[i].x;
		}
		xc = x2 - xrad;
		for (i = 0, j = 0; i < 5; i++, j++)
		{ 
			points[j].x = xc + points[i].x;
			points[j].y = yc - points[i].y;
		}
		points[20].x = points[0].x;
		points[20].y = points[0].y; 

		if (filled)
		{
			struct fillparams params;
			
			init_filled(v, &params, v->fill_color);
			vdi_draw_polygons(v->gc, points, 21);
			if (v->fill_perimeter)
			{
				int color = v->fill_perimeter_whichcolor == 0 ? v->fill_color : v->line_color;
				vdi_draw_lines(v, points, 21, 0xffff, v->wrmode, FIX_COLOR(color), FIX_COLOR(v->bg_color));
			}
		} else
		{
			draw_pline(v, points, 21);
		}
	}
}

/******************************************************************************/

static int vdi_v_rbox(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
#if TRACE_VDI
	_WORD *ptsin = PV_PTSIN(pb);
#endif

	V("v_rbox[%d]: %d,%d, %d,%d", v->handle, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 2), V_PTSIN(pb, 3));

	rounded_box(v, pb, FALSE);
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_rfbox(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
#if TRACE_VDI
	_WORD *ptsin = PV_PTSIN(pb);
#endif

	V("v_rfbox[%d]: %d,%d, %d,%d", v->handle, V_PTSIN(pb, 0), V_PTSIN(pb, 1), V_PTSIN(pb, 2), V_PTSIN(pb, 3));

	rounded_box(v, pb, TRUE);
	
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/
/* Input functions                                                            */
/******************************************************************************/

#ifdef IN_ATARI
static _UWORD set_sr(_UWORD new_sr)
{
	_UWORD old_sr;
	
	old_sr = sr;
	sr = new_sr;
	return old_sr;
}
#endif


static int vdi_vex_timv(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD ms;
	
	V("vex_timv[%d] %p", v->handle, vdi_control_ptr(0, void *));

	if (IS_SCREEN_V(v))
	{
		vdi_control_ptr(1, VEX_TIMV) = user_tim;
		user_tim = vdi_control_ptr(0, VEX_TIMV);
	} else
	{
		vdi_control_ptr(1, void *) = NULL;
	}
	ms = sys_Tickcal();
	V_INTOUT(pb, 0) = ms;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vex_butv(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("vex_butv[%d] %p", v->handle, vdi_control_ptr(0, void *));

	if (IS_SCREEN_V(v))
	{
		vdi_control_ptr(1, VEX_BUTV) = user_but;
		user_but = vdi_control_ptr(0, VEX_BUTV);
	} else
	{
		vdi_control_ptr(1, void *) = NULL;
	}
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vex_motv(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("vex_motv[%d] %p", v->handle, vdi_control_ptr(0, void *));

	if (IS_SCREEN_V(v))
	{
		vdi_control_ptr(1, VEX_MOTV) = user_mot;
		user_mot = vdi_control_ptr(0, VEX_MOTV);
	} else
	{
		vdi_control_ptr(1, void *) = NULL;
	}
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vex_curv(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("vex_curv[%d] %p", v->handle, vdi_control_ptr(0, void *));

	if (IS_SCREEN_V(v))
	{
		vdi_control_ptr(1, VEX_CURV) = user_cur;
		user_cur = vdi_control_ptr(0, VEX_CURV);
	} else
	{
		vdi_control_ptr(1, void *) = NULL;
	}
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vex_wheelv(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
#if TRACE_VDI
	_WORD *intin = PV_INTIN(pb);
#endif

	if (V_NINTIN(pb) == 1)
	{
		V("v_pat_rotate[%d] %d", v->handle, V_INTIN(pb, 0));
		/* NYI */
		V_NINTOUT(pb, 0);
		V_NPTSOUT(pb, 0);
		return VDI_DONE;
	}
	
	V("vex_wheelv[%d] %p", v->handle, vdi_control_ptr(0, void *));

	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	if (IS_SCREEN_V(v))
	{
		vdi_control_ptr(1, VEX_WHEELV) = user_wheel;
		user_wheel = vdi_control_ptr(0, VEX_WHEELV);
		/*
		 * we don't handle wheel packets yet,
		 * and the vector is not in any official variables,
		 * so update ROM-TOS, too.
		 * Should be safe since there is no error generated like in
		 * AES when calling unknown functions.
		 */
		return VDI_PASS;
	} else
	{
		vdi_control_ptr(1, void *) = NULL;
	}
	return VDI_DONE;
}

/******************************************************************************/
/* Workstation management                                                     */
/******************************************************************************/

void vdi_clswk(VWK *v)
{
	int h;
	
	if (v != NULL)
	{
		if (v->to_free)
			dos_free(v->to_free);
		if (v->req_col != &screen_pal && v->req_col != &dummy_pal)
			g_free(v->req_col);
		
		h = v->handle;
		g_free(v);
		vwk[h] = NULL;
	}
}

/* -------------------------------------------------------------------------- */

static void chomp(char *dst, const char *src, size_t maxlen)
{
	size_t len;
	
	strncpy(dst, src, maxlen);
	dst[maxlen - 1] = '\0';
	len = strlen(dst);
	while (len > 0 && dst[len - 1] == ' ')
		dst[--len] = '\0';
}

static void x_get_font_props(FONT_DESC *sf, const FONT_HDR *h)
{
	int c, count;
	
	sf->hdr = h;
	sf->top = h->top;
	sf->height = h->top;
	sf->cellheight = h->form_height;
	sf->descent = sf->bottom = sf->cellheight - sf->top - 1;
	sf->scaled = 0;
	sf->first_char = h->first_ade;
	sf->last_char = h->last_ade;
	sf->cellwidth = h->max_cell_width;
	sf->width = h->max_cell_width - 1;
	sf->left_offset = 0;
	sf->right_offset = sf->width - h->right_offset;
	sf->all_chars_exist = sf->first_char == 0 && sf->last_char == 255;
	chomp(sf->name, h->name, VDI_FONTNAMESIZE);
	sf->charset = 255; /* OEM */
	sf->pointsize = h->point;
	sf->half = h->half;
	sf->ascent = h->ascent;
	sf->monospaced = h->hor_table == NULL;
	sf->default_char = '?';
	
#if 0
	{
	int number = sf - sysfont;

	static sysfontinfo const sfinfo[4] = {
		{  4,  6,  4,  4,  3, 1, 1,  5,  6, 0, 0,  8 },
		{  6,  8,  6,  6,  4, 1, 1,  7,  8, 0, 0,  9 },
		{ 13, 16, 13, 11,  8, 2, 2,  7,  8, 0, 0, 10 },
		{ 26, 32, 26, 21, 15, 5, 5, 16, 16, 0, 0, 20 }
	};

	printf("font %d %s:\n", number, sf->name);
	printf(" height      : %d %d\n", sf->height, sfinfo[number].height);
	printf(" cellheight  : %d %d\n", sf->cellheight, sfinfo[number].cellheight);
	printf(" top         : %d %d\n", sf->top, sfinfo[number].top);
	printf(" ascent      : %d %d\n", sf->ascent, sfinfo[number].ascent);
	printf(" half        : %d %d\n", sf->half, sfinfo[number].half);
	printf(" descent     : %d %d\n", sf->descent, sfinfo[number].descent);
	printf(" bottom      : %d %d\n", sf->bottom, sfinfo[number].bottom);
	printf(" width       : %d %d\n", sf->width, sfinfo[number].width);
	printf(" cellwidth   : %d %d\n", sf->cellwidth, sfinfo[number].cellwidth);
	printf(" left_offset : %d %d\n", sf->left_offset, sfinfo[number].left_offset);
	printf(" right_offset: %d %d\n", sf->left_offset, sfinfo[number].right_offset);
	printf(" point       : %d %d\n", sf->pointsize, sfinfo[number].point);
	}
#endif
	count = sf->last_char - sf->first_char + 1;
	for (c = 0; c < count; c++)
	{
		vdi_charinfo *info = &sf->per_char[c];
		
		info->lbearing = sf->left_offset;
		info->width = h->off_table[c + 1] - h->off_table[c];
		info->rbearing = info->width - info->lbearing;
	}
}

static void vdi_reset_vars(void)
{
	int i, j, k;

	phys_handle = -1;
	user_but = 0;
	user_tim = 0;
	user_mot = 0;
	user_cur = 0;
	user_wheel = 0;

	for (i = 0, k = 0; i < 5; i++)
	{
		fillpat[i] = &allpatterns[k];
		for (j = 0; j < patnum[i]; j++, k++)
			allpatterns[k] = &pattern_bits[PATTERN_SIZE * k];
	}

	/* Now load the system font: */
	for (i = 0; i < SYSFONTS * NLSFONTSETS; i++)
	{
		x_get_font_props(&sysfont[i], sysfonthdrs[i]);
	}
}

/* -------------------------------------------------------------------------- */

static void close_all_wk(void)
{
	int i;

	for (i = 0; i < MAX_VWK; i++)
	{
		vdi_clswk(vwk[i]);
	}
	vdi_reset_vars();
}

/* -------------------------------------------------------------------------- */

static void store_vwk_params(VWK *v, _WORD *intout, _WORD *ptsout)
{
	V_INTOUT(pb, 0) = v->dev_tab.max_x;
	V_INTOUT(pb, 1) = v->dev_tab.max_y;
	V_INTOUT(pb, 2) = v->dev_tab.scale_flag;
	V_INTOUT(pb, 3) = v->dev_tab.pix_width;
	V_INTOUT(pb, 4) = v->dev_tab.pix_height;
	V_INTOUT(pb, 5) = v->dev_tab.font_sizes;
	V_INTOUT(pb, 6) = v->dev_tab.line_types;
	V_INTOUT(pb, 7) = v->dev_tab.line_widths;
	V_INTOUT(pb, 8) = v->dev_tab.marker_types;
	V_INTOUT(pb, 9) = v->dev_tab.marker_sizes;
	V_INTOUT(pb, 10) = v->dev_tab.num_fonts;
	V_INTOUT(pb, 11) = v->dev_tab.num_patterns;
	V_INTOUT(pb, 12) = v->dev_tab.num_shapes;
	V_INTOUT(pb, 13) = v->dev_tab.num_colors;
	V_INTOUT(pb, 14) = v->dev_tab.num_gdps;
	V_INTOUT(pb, 15) = v->dev_tab.gdp_funcs[0];
	V_INTOUT(pb, 16) = v->dev_tab.gdp_funcs[1];
	V_INTOUT(pb, 17) = v->dev_tab.gdp_funcs[2];
	V_INTOUT(pb, 18) = v->dev_tab.gdp_funcs[3];
	V_INTOUT(pb, 19) = v->dev_tab.gdp_funcs[4];
	V_INTOUT(pb, 20) = v->dev_tab.gdp_funcs[5];
	V_INTOUT(pb, 21) = v->dev_tab.gdp_funcs[6];
	V_INTOUT(pb, 22) = v->dev_tab.gdp_funcs[7];
	V_INTOUT(pb, 23) = v->dev_tab.gdp_funcs[8];
	V_INTOUT(pb, 24) = v->dev_tab.gdp_funcs[9];
	V_INTOUT(pb, 25) = v->dev_tab.gdp_attribs[0];
	V_INTOUT(pb, 26) = v->dev_tab.gdp_attribs[1];
	V_INTOUT(pb, 27) = v->dev_tab.gdp_attribs[2];
	V_INTOUT(pb, 28) = v->dev_tab.gdp_attribs[3];
	V_INTOUT(pb, 29) = v->dev_tab.gdp_attribs[4];
	V_INTOUT(pb, 30) = v->dev_tab.gdp_attribs[5];
	V_INTOUT(pb, 31) = v->dev_tab.gdp_attribs[6];
	V_INTOUT(pb, 32) = v->dev_tab.gdp_attribs[7];
	V_INTOUT(pb, 33) = v->dev_tab.gdp_attribs[8];
	V_INTOUT(pb, 34) = v->dev_tab.gdp_attribs[9];
	V_INTOUT(pb, 35) = v->dev_tab.color_flag;
	V_INTOUT(pb, 36) = v->dev_tab.rotation_flag;
	V_INTOUT(pb, 37) = v->dev_tab.fillarea_flag;
	V_INTOUT(pb, 38) = v->dev_tab.cellarray_flag;
	V_INTOUT(pb, 39) = v->dev_tab.available_colors;
	V_INTOUT(pb, 40) = v->dev_tab.cursor_control;
	V_INTOUT(pb, 41) = v->dev_tab.valuator_control;
	V_INTOUT(pb, 42) = v->dev_tab.choice_control;
	V_INTOUT(pb, 43) = v->dev_tab.string_control;
	V_INTOUT(pb, 44) = v->dev_tab.device_type;
	
	PTSOUT(0) = v->siz_tab.min_char_width;
	PTSOUT(1) = v->siz_tab.min_char_height;
	PTSOUT(2) = v->siz_tab.max_char_width;
	PTSOUT(3) = v->siz_tab.max_char_height;
	PTSOUT(4) = v->siz_tab.min_line_width;
	PTSOUT(5) = v->siz_tab.reserved1;
	PTSOUT(6) = v->siz_tab.max_line_width;
	PTSOUT(7) = v->siz_tab.reserved2;
	PTSOUT(8) = v->siz_tab.min_marker_width;
	PTSOUT(9) = v->siz_tab.min_marker_height;
	PTSOUT(10) = v->siz_tab.max_marker_width;
	PTSOUT(11) = v->siz_tab.max_marker_height;
}

/* -------------------------------------------------------------------------- */

static VWK *init_vwk(VDIPB *pb, int h, int phys_wk, int width, int height, int planes)
{
	VWK *v;
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	int m;
	
	if (h >= MAX_VWK)
		return NULL;
	
	if ((v = vwk[h]) == NULL)
	{
		v = vwk[h] = g_new0(VWK, 1);
		if (v == NULL)
			return NULL;
	}
	
	/*
	 * important note:
	 * This function must make sure that each and every member
	 * of the workstation struct is explicitly assigned.
	 * If a GEM program crashes, or after a warmboot,
	 * we may get a handle back from the ROM VDI that was
	 * already in our table, but has to be reset to a sane state now.
	 */
	
	v->handle = h;
	v->phys_wk = phys_wk;
	v->can_clip = phys_wk == phys_handle;
	
	v->wrmode = MD_REPLACE;
	v->clipping = FALSE;
	v->clipr.x = 0;
	v->clipr.y = 0;
	v->clipr.width = width;
	v->clipr.height = height;
	v->input_mode[0] = MODE_UNDEFINED;
	v->input_mode[DEV_LOCATOR] = MODE_REQUEST;
	v->input_mode[DEV_VALUATOR] = MODE_REQUEST;
	v->input_mode[DEV_CHOICE] = MODE_REQUEST;
	v->input_mode[DEV_STRING] = MODE_REQUEST;

	v->bg_color = WHITE;
	
	init_wk(v, width, height, planes);

	set_clipping(v);

	m = V_INTIN(pb, 1);
	if (m < 1 || m > LT_MAX)
		m = SOLID;
	v->line_type = m;

	m = V_INTIN(pb, 2);
	if (m < 0 || m >= v->dev_tab.num_colors)
		m = BLACK;
	v->line_color = m;
	v->ud_linepat = 0xffff;
	v->line_width = 1;
	v->line_ends = SQUARED | (SQUARED << 2);
	
	m = V_INTIN(pb, 3);
	if (m < 1 || m > PM_MAX)
		m = PM_ASTERISK;
	v->marker_type = m;

	m = V_INTIN(pb, 4);
	if (m < 0 || m >= v->dev_tab.num_colors)
		m = BLACK;
	v->marker_color = m;
	v->marker_height = MIN_MKHT;
	calc_marker_scale(v);
	
	/* V_INTIN(pb, 5) ignored (font_id) */
	
	m = V_INTIN(pb, 6);
	if (m < 0 || m >= v->dev_tab.num_colors)
		m = BLACK;
	v->text_color = m;
	v->text_rotation = 0;
	v->v_align = ALI_BASE;
	v->h_align = ALI_LEFT;
	v->text_style = 0;
	v->skew = 0;
	v->mapmode = MAP_ATARI;
	v->pairmode = 0;
	v->kern_offset = 0;
	v->font_id = SYSTEM_FONT_ID;
	change_font(v, vdi_h >= 400 ? 2 : 1);

	m = V_INTIN(pb, 7);
	if (m < 0 || m >= 5)
		m = FIS_HOLLOW;
	v->fill_interior = m;

	m = V_INTIN(pb, 8);
	/*
	 * can't check validity here because we don't know
	 * which one of vsf_interior and vsf_style is called first;
	 * make sure to check for valid values before using them
	 */
#if 0
	if (m < 1 || m > patnum[v->fill_interior])
		m = 1;
#endif
	v->fill_style = m;

	m = V_INTIN(pb, 9);
	if (m < 0 || m >= v->dev_tab.num_colors)
		m = BLACK;
	v->fill_color = m;
	v->fill_perimeter = TRUE;
	v->fill_perimeter_whichcolor = 0;
	
	m = V_INTIN(pb, 10);
	if (m != 0 && m != 2)
		m = 2;
	v->xfm_mode = m;
	
	v->bezier.on = FALSE;
	v->bezier.qual = MAX_BEZIER_QUAL;
	
	store_vwk_params(v, intout, ptsout);
	V_NINTOUT(pb, 45);
	V_NPTSOUT(pb, 6);
	
	return v;
}

/******************************************************************************/

/*
 * allocate a VDI handle for v_opnwk().
 */
static int vdi_alloc_handle(void)
{
	_WORD handle;
	
	for (handle = 0; handle < MAX_VWK; handle++)
		if (vwk[handle] == NULL)
			return handle;
	KINFO(("VDI: out of handles\n"));
	return -1;
}

/******************************************************************************/

void vdi_release_handle(VDIPB *pb, _WORD h)
{
	UNUSED(pb);
	if (h >= 0 && h < MAX_VWK)
		vwk[h] = NULL;
}

/******************************************************************************/

static int vdi_v_updwk(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("v_updwk[%d]", v->handle);
	UNUSED(v);

	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	/* nothing to do for screen driver */
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_clrwk(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	
	V("v_clrwk[%d]", v->handle);

	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	if (v->handle == phys_handle)
	{
		v_reset_alpha_cursor(v);
		clear_window(v);
		v_show_alpha_cursor(v);
	}
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_clsvwk(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("v_clsvwk[%d]", v->handle);

	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	if (v->handle != v->phys_wk)
	{
		vdi_clswk(v);
		/*
		 * we got the handle from ROM; pass it through, too
		 */
		return VDI_PASS;
	}
	
	/* it wasn't really a virtual WK */
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_v_clswk(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	V("v_clswk[%d]", v->handle);

	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	if (v->handle == phys_handle)
	{
		close_all_wk();
		destroy_image();
		/*
		 * we got the handle from ROM; pass it through, too
		 */
		return VDI_PASS;
	} else
	{
		if (v->handle == v->phys_wk)
		{
			vdi_clswk(v);
		}
		/* otherwise was actually a virtual WK */
		return VDI_DONE;
	}
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_extnd(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	
	V("vq_extnd[%d]", v->handle);

	if (V_INTIN(pb, 0))
	{
		V_INTOUT(pb, 0) = v->inq_tab.screen_type;
		V_INTOUT(pb, 1) = v->inq_tab.background_colors;
		V_INTOUT(pb, 2) = v->inq_tab.supported_effects;
		V_INTOUT(pb, 3) = v->inq_tab.scaling_flag;
		V_INTOUT(pb, 4) = v->inq_tab.planes;
		V_INTOUT(pb, 5) = v->inq_tab.clut_flag;
		V_INTOUT(pb, 6) = v->inq_tab.blits_per_sec;
		V_INTOUT(pb, 7) = v->inq_tab.contourfill_flag;
		V_INTOUT(pb, 8) = v->inq_tab.rotation_flag;
		V_INTOUT(pb, 9) = v->inq_tab.num_wrmodes;
		V_INTOUT(pb, 10) = v->inq_tab.max_input_mode;
		V_INTOUT(pb, 11) = v->inq_tab.justification_flag;
		V_INTOUT(pb, 12) = v->inq_tab.pen_change;
		V_INTOUT(pb, 13) = v->inq_tab.ribbon_change;
		V_INTOUT(pb, 14) = v->inq_tab.max_points;
		V_INTOUT(pb, 15) = v->inq_tab.max_intin_size;
		V_INTOUT(pb, 16) = v->inq_tab.mouse_buttons;
		V_INTOUT(pb, 17) = v->inq_tab.line_types_flag;
		V_INTOUT(pb, 18) = v->inq_tab.wrmode_flag;
		V_INTOUT(pb, 19) = v->inq_tab.clipping;
		V_INTOUT(pb, 20) = v->inq_tab.pixsize_flag;
		V_INTOUT(pb, 21) = v->inq_tab.pix_width;
		V_INTOUT(pb, 22) = v->inq_tab.pix_height;
		V_INTOUT(pb, 23) = v->inq_tab.hdpi;
		V_INTOUT(pb, 24) = v->inq_tab.vdpi;
		V_INTOUT(pb, 25) = v->inq_tab.image_rotation;
		V_INTOUT(pb, 26) = v->inq_tab.quarter_screen_high;
		V_INTOUT(pb, 27) = v->inq_tab.quarter_screen_low;
		V_INTOUT(pb, 28) = v->inq_tab.bezier_flag;
		V_INTOUT(pb, 29) = v->inq_tab.reserved1;
		V_INTOUT(pb, 30) = v->inq_tab.raster_flags;
		V_INTOUT(pb, 31) = v->inq_tab.reserved2;
		V_INTOUT(pb, 32) = v->inq_tab.color_flags;
		V_INTOUT(pb, 33) = v->inq_tab.reserved3;
		V_INTOUT(pb, 34) = v->inq_tab.reserved4;
		V_INTOUT(pb, 35) = v->inq_tab.reserved5;
		V_INTOUT(pb, 36) = v->inq_tab.reserved6;
		V_INTOUT(pb, 37) = v->inq_tab.reserved7;
		V_INTOUT(pb, 38) = v->inq_tab.reserved8;
		V_INTOUT(pb, 39) = v->inq_tab.reserved9;
		V_INTOUT(pb, 40) = v->inq_tab.left_border;
		V_INTOUT(pb, 41) = v->inq_tab.top_border;
		V_INTOUT(pb, 42) = v->inq_tab.right_border;
		V_INTOUT(pb, 43) = v->inq_tab.bottom_border;
		V_INTOUT(pb, 44) = v->inq_tab.page_size;

		PTSOUT(0) = v->clipr.x;
		PTSOUT(1) = v->clipr.y;
		PTSOUT(2) = v->clipr.x + v->clipr.width - 1;
		PTSOUT(3) = v->clipr.y + v->clipr.height - 1;
		PTSOUT(4) = 0;
		PTSOUT(5) = 0;
		PTSOUT(6) = 0;
		PTSOUT(7) = 0;
		PTSOUT(8) = 0;
		PTSOUT(9) = 0;
		PTSOUT(10) = 0;
		PTSOUT(11) = 0;
	} else
	{
		store_vwk_params(v, intout, ptsout);
	}
	V_NINTOUT(pb, 45);
	V_NPTSOUT(pb, 6);
	return VDI_DONE;
}

/******************************************************************************/

static int vdi_vq_scrninfo(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int i;
	
	V("vq_scrninfo[%d]", v->handle);

	if (V_INTIN(pb, 0) == 2)
	{
		V_INTOUT(pb, 0) = v->planes <= 8 ? 0 : 2; /* format */
		V_INTOUT(pb, 1) = v->planes == 1 ? 0 : (v->planes > 1 && v->planes <= 8) ? 1 : 2; /* clut */
		V_INTOUT(pb, 2) = (v->planes >= 1 && v->planes <= 8) ? v->planes : 16; /* bit_depth */
		V_INTOUT(pb, 3) = v->planes == 16 ? 1 : 0;
		V_INTOUT(pb, 4) = (v->planes >= 1 && v->planes <= 16) ? (1 << v->planes) : 0; /* colours */
		V_INTOUT(pb, 5) = (v->width >> 4) * 2 * v->planes;
		V_INTOUT(pb, 6) = 0; /* v->bitmap_addr; */
		V_INTOUT(pb, 7) = 0; /* v->bitmap_addr; */
		V_INTOUT(pb, 8) = v->planes == 15 || v->planes == 16 ? 5 : 8; /* red bits */
		V_INTOUT(pb, 9) = v->planes == 15 ? 5 : v->planes == 16 ? 6 : 8; /* green bits */
		V_INTOUT(pb, 10) = v->planes == 15 || v->planes == 16 ? 5 : 8; /* blue bits */
		V_INTOUT(pb, 11) = v->planes > 24 ? 8 : 0; /* alpha bits */
		V_INTOUT(pb, 12) = 0; /* genlock */
		V_INTOUT(pb, 13) = v->planes == 15 ? 1 : 0; /* unused bits */
		V_INTOUT(pb, 14) = v->bit_order; /* usual bit order */
		V_INTOUT(pb, 15) = 0; /* dummy2 */
		if (v->planes <= 8)
		{
			for (i = 0; i < MAX_VDI_COLS; i++)
			{
				V_INTOUT(pb, i + 16) = vdi_maptab256[i];
			}
			V_INTOUT(pb, 16 + BLACK) = st_maptab[BLACK];
		} else
		{
			for (i = 16; i < 128; i++)
				V_INTOUT(pb, i) = -1;
			for (i = 128; i < 272; i++)
				V_INTOUT(pb, i) = 0;
			if (v->planes <= 15)
			{
				for (i = 0; i < 5; i++)
					V_INTOUT(pb, 16 + i) = 10 + i;
				for (i = 0; i < 5; i++)
					V_INTOUT(pb, 32 + i) = 5 + i;
				for (i = 0; i < 5; i++)
					V_INTOUT(pb, 48 + i) = i;
			} else if (v->planes <= 16)
			{
				for (i = 0; i < 5; i++)
					V_INTOUT(pb, 16 + i) = 11 + i;
				for (i = 0; i < 6; i++)
					V_INTOUT(pb, 32 + i) = 5 + i;
				for (i = 0; i < 5; i++)
					V_INTOUT(pb, 48 + i) = i;
			} else
			{
				for (i = 0; i < 8; i++)
					V_INTOUT(pb, 16 + i) = 16 + i;
				for (i = 0; i < 8; i++)
					V_INTOUT(pb, 32 + i) = 8 + i;
				for (i = 0; i < 8; i++)
					V_INTOUT(pb, 48 + i) = i;
			}
		}
		V_NINTOUT(pb, 272);
		V_NPTSOUT(pb, 0);
		return VDI_DONE;
	}
	V_INTOUT(pb, 0) = 0;
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/

static int store_string(_WORD *dst, const char *src)
{
	int l = 0;
	
	while (*src)
	{
		*dst = *src;
		dst++;
		src++;
		l++;
	}
	*dst = 0;
	return l;
}


static int store_cstring(char *dst, const char *src)
{
	int l = 0;
	
	while (*src)
	{
		*dst = *src;
		dst++;
		src++;
		l++;
	}
	*dst = 0;
	return (l + 1) / 2;
}


static void vdi_devinfo(VDIPB *pb, const struct driverinfo *info)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD *ptsout = PV_PTSOUT(pb);
	int l;
	
	/*
	 * who the hell invented this strange binding?
	 */
	/* filename is in intout, with 1 char per entry */
	l = store_string(&V_INTOUT(pb, 0), info ? info->name : "");
	V_NINTOUT(pb, l);
	PTSOUT(0) = 1;
	/* device description is in ptsout, with 2 chars per entry */
	l = store_cstring((char *)&PTSOUT(1), info ? info->desc : "");
	V_NPTSOUT(pb, l + 1);
}


static int vdi_vq_devinfo(VDIPB *pb)
{
	_WORD *intin = PV_INTIN(pb);
#if TRACE_VDI
	_WORD *control = PV_CONTROL(pb);
#endif
	_WORD device = V_INTIN(pb, 0);
	
	V("vq_devinfo[%d] %d", V_HANDLE(pb), device);
	
	vdi_devinfo(pb, get_devinfo(device));
	return VDI_DONE;
}

static int vdi_vq_ext_devinfo(VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	
	V_NINTOUT(pb, 2);
	V_NPTSOUT(pb, 0);
	V_INTOUT(pb, 0) = 0;
	V_INTOUT(pb, 1) = 0;
	
	if (V_NINTIN(pb) >= 1 + 3 * (int)VDI_NPTRINTS)
	{
		_WORD device = V_INTIN(pb, 0);
		char *file_path = vdi_intin_ptr(1, char *);
		char *file_name = vdi_intin_ptr(1 + VDI_NPTRINTS, char *);
		char *name = vdi_intin_ptr(1 + 2 * VDI_NPTRINTS, char *);
		const struct driverinfo *info = get_devinfo(device);
		int l;
		
		V("vq_ext_devinfo[%d] %d,$%p,$%p,$%p", V_HANDLE(pb), device, file_path, file_name, name);
		if (file_path)
		{
			const char *path = "C:\\GEMSYS\\"; /* FIXME */
			l = strlen(path) + 1;
			memcpy(file_path, path, l);
		}
		if (file_name)
		{
			const char *name = info ? info->name : "";
			l = strlen(name) + 1;
			memcpy(file_name, name, l);
		}
		if (name)
		{
			const char *desc = info ? info->desc : "";
			l = strlen(desc) + 1;
			memcpy(name, desc, l);
		}
		V_INTOUT(pb, 0) = info != NULL;
		V_INTOUT(pb, 1) = info != NULL && info->device_id == VDI_SCREEN_DEVICE;
	} else
	{
		V("vq_ext_devinfo[%d] %d", V_HANDLE(pb), V_INTIN(pb, 0));
		V_NINTOUT(pb, 0);
	}
	
	return VDI_DONE;
}
	
/******************************************************************************/
/* -------------------------------------------------------------------------- */
/******************************************************************************/

gboolean vdi_vq_vgdos(void)
{
	V("vq_vgdos: %lx", GDOS_VERSION);
#if SUPPORT_GDOS
	return GDOS_VERSION;
#else
	return 0;
#endif
}

/* -------------------------------------------------------------------------- */

static int vdi_vst_load_fonts(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);

	UNUSED(v);
	V("vst_%sload_fonts[%d]", V_NINTIN(pb) >= 3 ? "ex_" : "", v->handle);
	V_INTOUT(pb, 0) = 0;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/* -------------------------------------------------------------------------- */

static int vdi_vst_unload_fonts(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);

	UNUSED(v);
	V("vst_unload_fonts[%d]", v->handle);
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/* -------------------------------------------------------------------------- */

static int vdi_vst_charmap(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD mode = V_INTIN(pb, 0);
	
	if (mode < 0 || mode > 2)
		mode = MAP_ATARI;
	v->mapmode = mode;
	if (V_NINTIN(pb) == 2 && V_INTIN(pb, 1) == 1)
	{
		V("vst_map_mode[%d]: %d", v->handle, V_INTIN(pb, 0));
		V_NINTOUT(pb, 1);
		V_NPTSOUT(pb, 0);
		V_INTOUT(pb, 0) = mode;
	} else
	{
		V("vst_charmap[%d]: %d", v->handle, V_INTIN(pb, 0));
		V_NINTOUT(pb, 0);
		V_NPTSOUT(pb, 0);
	}
	return VDI_DONE;
}

/* -------------------------------------------------------------------------- */

static int vdi_v_fontinit(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
#if TRACE_VDI
	_WORD *intin = PV_INTIN(pb);
#endif

	UNUSED(v);
	if (V_NINTIN(pb) == 1 * VDI_NPTRINTS)
	{
		V("v_fontinit[%d]: $%p", v->handle, vdi_intin_ptr(0, void *));
		V_INTOUT(pb, 0) = 0;
		V_NINTOUT(pb, 1);
		V_NPTSOUT(pb, 0);
	} else if (V_NINTIN(pb) == 1)
	{
		V("vs_bkcolor[%d]: %d", v->handle, V_INTIN(pb, 0));
		V_NINTOUT(pb, 0);
		V_NPTSOUT(pb, 0);
	} else
	{
		V("v_fontinit[%d]: incorrect number of arguments", v->handle);
		V_NINTOUT(pb, 0);
		V_NPTSOUT(pb, 0);
	}
	return VDI_DONE;
}

/* -------------------------------------------------------------------------- */

static int vdi_v_tray(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);
#if TRACE_VDI
	_WORD *intin = PV_INTIN(pb);
#endif
	
	UNUSED(v);
	if (V_NINTIN(pb) == 2)
	{
		V("v_trays[%d]: %d,%d", v->handle, V_INTIN(pb, 0), V_INTIN(pb, 1));
	} else
	{
		V("v_tray[%d]: %d", v->handle, V_INTIN(pb, 0));
	}
	/* NYI */
	V_NINTOUT(pb, 2);
	V_NPTSOUT(pb, 0);
	V_INTOUT(pb, 0) = 0;
	V_INTOUT(pb, 1) = 0;
	return VDI_DONE;
}

/* -------------------------------------------------------------------------- */

static int vdi_vsp_state(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
#if TRACE_VDI
	_WORD *intin = PV_INTIN(pb);
#endif
	
	UNUSED(v);
	if (V_NINTIN(pb) == 1)
	{
		V("vsc_expose[%d]: %d", v->handle, V_INTIN(pb, 0));
	} else if (V_NINTIN(pb) >= 13)
	{
		V("vsp_state[%d]: %d,%d,%d,%d,%d", v->handle, V_INTIN(pb, 0), V_INTIN(pb, 1), V_INTIN(pb, 2), V_INTIN(pb, 3), V_INTIN(pb, 4));
	} else
	{
		V("vsp_???[%d]", v->handle);
	}
	/* NYI */
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/* -------------------------------------------------------------------------- */

static int vdi_vst_kern(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	_WORD trackmode = V_INTIN(pb, 0);
	_WORD pairmode = V_INTIN(pb, 1);
	
	v->pairmode = pairmode;
	if (V_NINTIN(pb) == 4 && trackmode == 255)
	{
		fix31 offset = vdi_intin_long(2);
		V("vst_track_offset[%d]: 0x%x,%d", v->handle, offset, pairmode);
		v->kern_offset = offset;
		V_NINTOUT(pb, 2);
		V_NPTSOUT(pb, 0);
		V_INTOUT(pb, 0) = 0; /* NYI */
		V_INTOUT(pb, 1) = 0;
	} else
	{
		V("vst_kern[%d]: %d,%d", v->handle, trackmode, pairmode);
		V_NINTOUT(pb, 2);
		V_NPTSOUT(pb, 0);
		V_INTOUT(pb, 0) = 0; /* NYI */
		V_INTOUT(pb, 1) = 0;
	}
	return VDI_DONE;
}

/******************************************************************************/
/******************************************************************************/
/* Extensions                                                                 */
/******************************************************************************/
/******************************************************************************/

static int vdi_v_write_png(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	_WORD *intout = PV_INTOUT(pb);
	int n = V_NINTIN(pb);
	char *filename = (char *)malloc(n + 1);
	int i;
	writepng_info *info;
	int rc;
	
	V("v_write_png[%d]: '%s'", v->handle, ascii_text(n, &V_INTIN(pb, 0)));

	for (i = 0; i < n; i++)
	{
		filename[i] = intin[i];
	}
	filename[i] = 0;
	
	info = writepng_new();
	info->rowbytes = v->width * sizeof(pel);
	info->bpp = v->planes;
	if (v->clipping)
	{
		info->image_data = (unsigned char *)(&framebuffer[v->clipr.y * v->width + v->clipr.x]);
		info->width = v->clipr.width;
		info->height = v->clipr.height;
	} else
	{
		info->image_data = (unsigned char *)framebuffer;
		info->width = v->width;
		info->height = v->height;
	}
	info->outfile = fopen(filename, "wb");
	if (info->outfile == NULL)
	{
		rc = errno;
	} else
	{
		info->num_palette = v->dev_tab.num_colors;
		for (i = 0; i < v->dev_tab.num_colors; i++)
		{
			int c;
			pel pix;
			pix = FIX_COLOR(i);
			c = (*v->req_col)[i][0]; c = c * 255 / 1000;
			info->palette[pix].red = c;
			c = (*v->req_col)[i][1]; c = c * 255 / 1000;
			info->palette[pix].green = c;
			c = (*v->req_col)[i][2]; c = c * 255 / 1000;
			info->palette[pix].blue = c;
		}
		rc = writepng_output(info);
		fclose(info->outfile);
	}
	writepng_exit(info);
	free(filename);
	V_INTOUT(pb, 0) = rc;
	V_NINTOUT(pb, 1);
	V_NPTSOUT(pb, 0);
	return VDI_DONE;
}

/******************************************************************************/
/* -------------------------------------------------------------------------- */
/******************************************************************************/

#undef IGNORE /* defined somewhere in windows headers */
#define IGNORE(_x) \
static int ign_ ## _x(VWK *v, VDIPB *pb) \
{ \
	_WORD *control = PV_CONTROL(pb); \
	UNUSED(v); \
	V("%s[%d]: ignored", #_x, V_HANDLE(pb)); \
	V_NPTSOUT(pb, 0); \
	V_NINTOUT(pb, 0); \
	return VDI_DONE; \
}

#define PASS(_x) \
static int pass_ ## _x(VWK *v, VDIPB *pb) \
{ \
	UNUSED(v); \
	UNUSED(pb); \
	V("%s[%d]: passthrough", #_x, PV_CONTROL(pb) + 6 * 2); \
	return VDI_PASS; \
}

#define NYI(_x, nintout, nptsout) \
static int vdi_##_x(VWK *v, VDIPB *pb) \
{ \
	_WORD *control = PV_CONTROL(pb); \
	_WORD *intout = PV_INTOUT(pb); \
	_WORD *ptsout = PV_PTSOUT(pb); \
	int i; \
	UNUSED(v); \
	V("%s[%d]: NOT YET IMPLEMENTED", #_x, V_HANDLE(pb)); \
	V_NPTSOUT(pb, nptsout); \
	V_NINTOUT(pb, nintout); \
	if (intout != 0) \
	{ \
		for (i = 0; i < nintout; i++) \
			V_INTOUT(pb, i) = 0; \
	} \
	if (ptsout != 0) \
	{ \
		for (i = 0; i < nptsout; i++) \
		{ \
			PTSOUTX(i) = 0; \
			PTSOUTY(i) = 0; \
		} \
	} \
	return VDI_DONE; \
}

/*
 * functions not yet implemented:
 */

IGNORE(v_cellarray)
IGNORE(vq_cellarray)
IGNORE(v_escape2000)
NYI(v_contourfill, 0, 0)

NYI(v_get_driver_info, 1, 0) /* GEM/3; not important since no VDI supports it */
NYI(v_resize_bm, 0, 0) /* EdDI >= 1.20 */
NYI(vqt_ext_name, 35, 0)
NYI(vqt_justified, 0, 0) /* PC/GEM; not important since no VDI supports it */
NYI(vs_grayoverride, 0, 0)
NYI(v_setrgb, 0, 0)
NYI(vr_transfer_bits, 0, 0)
NYI(vr_clip_rects_by_dst, 1, 4)
NYI(vr_clip_rects_by_src, 1, 4)
NYI(vr_clip_rects32_by_dst, 1, 8)
NYI(vr_clip_rects32_by_src, 1, 8)
NYI(v_create_driver_info, 2, 0)
NYI(v_delete_driver_info, 1, 0)
NYI(v_read_default_settings, 1, 0)
NYI(v_write_default_settings, 0, 0)
NYI(vqt_char_index, 1, 0)
NYI(vst_fg_color, 1, 0)
NYI(vsf_fg_color, 1, 0)
NYI(vsl_fg_color, 1, 0)
NYI(vsm_fg_color, 1, 0)
NYI(vsr_fg_color, 1, 0)
NYI(vst_bg_color, 1, 0)
NYI(vsf_bg_color, 1, 0)
NYI(vsl_bg_color, 1, 0)
NYI(vsm_bg_color, 1, 0)
NYI(vsr_bg_color, 1, 0)
NYI(vqt_fg_color, 6, 0)
NYI(vqf_fg_color, 6, 0)
NYI(vql_fg_color, 6, 0)
NYI(vqm_fg_color, 6, 0)
NYI(vqr_fg_color, 6, 0)
NYI(vqt_bg_color, 6, 0)
NYI(vqf_bg_color, 6, 0)
NYI(vql_bg_color, 6, 0)
NYI(vqm_bg_color, 6, 0)
NYI(vqr_bg_color, 6, 0)
NYI(v_color2value, 2, 0)
NYI(v_value2color, 6, 0)
NYI(v_color2nearest, 6, 0)
NYI(vq_px_format, 4, 0)
NYI(vs_ctab, 1, 0)
NYI(vs_ctab_entry, 1, 0)
NYI(vs_dflt_ctab, 1, 0)
NYI(v_create_ctab, 2, 0)
NYI(v_delete_ctab, 1, 0)
NYI(vs_hilite_color, 1, 0)
NYI(vs_min_color, 1, 0)
NYI(vs_max_color, 1, 0)
NYI(vs_weight_color, 1, 0)
NYI(v_create_itab, 2, 0)
NYI(v_delete_itab, 2, 0)
NYI(vq_hilite_color, 6, 0)
NYI(vq_min_color, 6, 0)
NYI(vq_max_color, 6, 0)
NYI(vq_weight_color, 6, 0)
NYI(vqt_xfntinfo, 3, 0)
NYI(vst_name, 0, 0)
NYI(vst_name_and_id, 0, 0)
NYI(vst_width, 0, 2)
NYI(vqt_fontheader, 0, 0)
NYI(vqt_trackkern, 4, 0)
NYI(vqt_pairkern, 4, 0)
NYI(v_getbitmap_info, 12, 0)
NYI(vqt_f_extent, 0, 4)
NYI(vqt_real_extent, 0, 4)
NYI(v_ftext, 0, 0)
NYI(v_killoutline, 0, 0)
NYI(v_getoutline, 0, 0)
NYI(v_get_outline, 0, 0)
NYI(v_fgetoutline, 0, 0)
NYI(vst_scratch, 0, 1)
NYI(vst_error, 0, 1)
NYI(vst_arbpt, 1, 2)
NYI(vqt_advance, 0, 4)
NYI(v_savecache, 1, 0)
NYI(v_loadcache, 1, 0)
NYI(v_flushcache, 1, 0)
NYI(vst_setsize, 1, 2)
NYI(vqt_get_table, 2, 0)
NYI(vqt_cachesize, 2, 0)
NYI(vqt_cacheinfo, 0, 0)
NYI(vs_backmap, 0, 0)
NYI(vs_outmode, 0, 0)
NYI(vs_use_fonts, 0, 0)
NYI(v_setrgbi, 0, 0)
NYI(v_topbot, 4, 0)
NYI(vq_margins, 7, 0)
NYI(vs_document_info, 1, 0)

NYI(vq_scan, 5, 0)
NYI(v_alpha_text, 0, 0)
NYI(v_orient, 1, 0)
NYI(v_copies, 1, 0)
NYI(v_ps_halftone, 0, 0)
NYI(vq_tray_names, 2, 0)
NYI(v_page_size, 1, 0)
NYI(vq_page_name, 5, 0)
NYI(vq_prn_scaling, 2, 0)
NYI(vs_palette, 1, 0)
NYI(v_sound, 0, 0)
NYI(vs_mute, 0, 0)
NYI(vt_resolution, 2, 0)
NYI(vt_axis, 2, 0)
NYI(vt_origin, 2, 0)
NYI(vq_tdimensions, 2, 0)
NYI(vt_alignment, 0, 0)
NYI(vqp_films, 0, 0)
NYI(vqp_state, 0, 0)
NYI(vsp_save, 0, 0)
NYI(vsp_message, 0, 0)
NYI(vqp_error, 1, 0)
NYI(v_meta_extents, 0, 0)
NYI(vm_filename, 0, 0)
NYI(v_etext, 0, 0)

IGNORE(v_clsbm) /* ignored on non-bitmap drivers */


/******************************************************************************/

/*
 * Initialize dispatch functions for a screen driver.
 * Also sets the functions that are common amongst all
 * drivers (inquire/attribute functions etc.)
 */
static void v_init_screen(VWK *v)
{
	if (v == NULL)
		return;
	
	v->req_col = &screen_pal;
	v->driver_id = VDI_SCREEN_DEVICE;
	
	v->drv.v_clswk = vdi_v_clswk;
	v->drv.v_clrwk = vdi_v_clrwk;
	v->drv.v_updwk = vdi_v_updwk;
	v->drv.v_form_adv = vdi_v_form_adv;
	v->drv.v_output_window = vdi_v_output_window;
	v->drv.v_clear_disp_list = vdi_v_clear_disp_list;
	v->drv.v_bit_image = vdi_v_bit_image;
	v->drv.vq_scan = vdi_vq_scan;
	v->drv.v_alpha_text = vdi_v_alpha_text;
	v->drv.v_orient = vdi_v_orient;
	v->drv.v_copies = vdi_v_copies;
	v->drv.v_tray = vdi_v_tray;
	v->drv.v_ps_halftone = vdi_v_ps_halftone;
	v->drv.vq_tray_names = vdi_vq_tray_names;
	v->drv.v_page_size = vdi_v_page_size;
	v->drv.vq_page_name = vdi_vq_page_name;
	v->drv.vq_prn_scaling = vdi_vq_prn_scaling;
	v->drv.vs_calibrate = vdi_vs_calibrate;
	v->drv.vq_calibrate = vdi_vq_calibrate;
	v->drv.vt_resolution = vdi_vt_resolution;
	v->drv.vt_axis = vdi_vt_axis;
	v->drv.vt_origin = vdi_vt_origin;
	v->drv.vq_tdimensions = vdi_vq_tdimensions;
	v->drv.vt_alignment = vdi_vt_alignment;
	v->drv.vqp_films = vdi_vqp_films;
	v->drv.vqp_state = vdi_vqp_state;
	v->drv.vsp_state = vdi_vsp_state;
	v->drv.vsp_save = vdi_vsp_save;
	v->drv.vsp_message = vdi_vsp_message;
	v->drv.vqp_error = vdi_vqp_error;
	v->drv.v_meta_extents = vdi_v_meta_extents;
	v->drv.vm_filename = vdi_vm_filename;
	v->drv.v_offset = vdi_v_offset;
	v->drv.v_pline = vdi_v_pline;
	v->drv.v_bez = vdi_v_bez;
	v->drv.v_pmarker = vdi_v_pmarker;
	v->drv.v_gtext = vdi_v_gtext;
	v->drv.v_fillarea = vdi_v_fillarea;
	v->drv.v_bez_fill = vdi_v_bez_fill;
	v->drv.v_bar = vdi_v_bar;
	v->drv.v_arc = vdi_v_arc;
	v->drv.v_pieslice = vdi_v_pieslice;
	v->drv.v_circle = vdi_v_circle;
	v->drv.v_ellipse = vdi_v_ellipse;
	v->drv.v_ellarc = vdi_v_ellarc;
	v->drv.v_ellpie = vdi_v_ellpie;
	v->drv.v_rbox = vdi_v_rbox;
	v->drv.v_rfbox = vdi_v_rfbox;
	v->drv.v_justified = vdi_v_justified;
	v->drv.v_clsbm = ign_v_clsbm;
	v->drv.v_contourfill = vdi_v_contourfill;
	v->drv.v_get_pixel = vdi_v_get_pixel;
	v->drv.vro_cpyfm = vdi_vro_cpyfm;
	v->drv.vr_recfl = vdi_vr_recfl;
	v->drv.vrt_cpyfm = vdi_vrt_cpyfm;
	v->drv.v_ftext = vdi_v_ftext;
}

IGNORE(v_form_adv)
IGNORE(v_output_window)
IGNORE(v_clear_disp_list)
IGNORE(v_bit_image)
IGNORE(v_offset)
IGNORE(v_updwk)
IGNORE(v_pline)
IGNORE(v_bez)
IGNORE(v_pmarker)
IGNORE(v_fillarea)
IGNORE(vr_recfl)
IGNORE(v_bar)
IGNORE(v_arc)
IGNORE(v_pieslice)
IGNORE(v_circle)
IGNORE(v_ellipse)
IGNORE(v_ellarc)
IGNORE(v_ellpie)
IGNORE(v_rbox)
IGNORE(v_rfbox)
IGNORE(v_gtext)
IGNORE(v_justified)
IGNORE(vro_cpyfm)
IGNORE(vrt_cpyfm)
IGNORE(v_contourfill)
IGNORE(v_ftext)

/*
 * Initialize dispatch functions for other drivers.
 */
void vdi_init_common(VWK *v)
{
	v_init_screen(v);

	v->req_col = &dummy_pal;
	memcpy(v->req_col, &initial_palette, sizeof(initial_palette));
	
	v->drv.v_form_adv = ign_v_form_adv;
	v->drv.v_output_window = ign_v_output_window;
	v->drv.v_clear_disp_list = ign_v_clear_disp_list;
	v->drv.v_bit_image = ign_v_bit_image;
	v->drv.v_offset = ign_v_offset;
	v->drv.v_updwk = ign_v_updwk;
	v->drv.v_pline = ign_v_pline;
	v->drv.v_bez = ign_v_bez;
	v->drv.v_pmarker = ign_v_pmarker;
	v->drv.v_fillarea = ign_v_fillarea;
	v->drv.v_arc = ign_v_arc;
	v->drv.v_pieslice = ign_v_pieslice;
	v->drv.v_circle = ign_v_circle;
	v->drv.v_ellipse = ign_v_ellipse;
	v->drv.v_ellarc = ign_v_ellarc;
	v->drv.v_ellpie = ign_v_ellpie;
	v->drv.v_rbox = ign_v_rbox;
	v->drv.v_rfbox = ign_v_rfbox;
	v->drv.vr_recfl = ign_vr_recfl;
	v->drv.v_gtext = ign_v_gtext;
	v->drv.v_justified = ign_v_justified;
	v->drv.vro_cpyfm = ign_vro_cpyfm;
	v->drv.vrt_cpyfm = ign_vrt_cpyfm;
	v->drv.v_bar = ign_v_bar;
	v->drv.v_contourfill = ign_v_contourfill;
	v->drv.v_ftext = ign_v_ftext;
	
	v->inq_tab.screen_type = 0; /* no screen */
	v->inq_tab.max_input_mode = 0;
	v->inq_tab.pen_change = 0;
	v->inq_tab.mouse_buttons = 0;
	v->dev_tab.cursor_control = 0;
	v->dev_tab.valuator_control = 0;
	v->dev_tab.choice_control = 0;
	v->dev_tab.string_control = 0;
	v->dev_tab.device_type = 0;
}

/******************************************************************************/
/* -------------------------------------------------------------------------- */
/******************************************************************************/

static int vdi_v_opnvwk(VDIPB *pb)
{
	VWK *v;
	_WORD *control = PV_CONTROL(pb);

	V("v_opnvwk[%d]", V_HANDLE(pb));

	if (phys_handle < 0)
	{
		KINFO(("v_opnvwk: no workstation yet\n"));
		return VDI_DONE;
	}
	V_HANDLE(pb) = vdi_alloc_handle();
	v = init_vwk(pb, V_HANDLE(pb), phys_handle, vdi_w, vdi_h, vdi_planes);
	if (v == NULL)
		vdi_release_handle(pb, V_HANDLE(pb));
	v_init_screen(v);
	return VDI_DONE;
}

static int vdi_v_opnbm(VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin;
	int h;
	MFDB *mfdb;
	int width, height;
	int bitplanes;
	long lwidth, size;
	VWK *v;
	uint8_t *addr;
	long colors;
	_WORD format;
	_WORD bit_order;
	
	if (V_NINTIN(pb) < 20)
	{
		/* not enough parameters for v_opnbm(), fall through to v_opnvwk() */
		V_CONTRL(pb, 5) = 0;
		return vdi_v_opnvwk(pb);
	}
	
	mfdb = vdi_control_ptr(0, MFDB *);

	V("v_opnbm[%d]: MFDB[0x%p,%dx%dx%d,wd=%d,std=%d]",
		V_HANDLE(pb),
		mfdb->fd_addr,
		mfdb->fd_w,
		mfdb->fd_h,
		mfdb->fd_nplanes,
		mfdb->fd_wdwidth,
		mfdb->fd_stand);

	V_CONTRL(pb, 6) = 0;
	intin = PV_INTIN(pb);
	
	/* Doesn't allow the EdDI v1.1 variant yet */
	colors = vdi_intin_long(15);
	if (colors != 0 && colors != 2 && colors != (1L << vdi_planes))
		return VDI_DONE;
	bitplanes = V_INTIN(pb, 17);
	if (bitplanes != 0 && bitplanes != 1 && bitplanes != vdi_planes)
		return VDI_DONE;
	format = V_INTIN(pb, 18);
	if (format != 0 && format != 1)
		return VDI_DONE;
		
	bit_order = V_INTIN(pb, 19);
	if (bit_order != 0 && bit_order != 1)
		return VDI_DONE;
	
	h = vdi_alloc_handle();
	V_HANDLE(pb) = h;
	if (h <= 0)
		return VDI_DONE;

	if (mfdb->fd_addr != 0 || V_INTIN(pb, 11) != 0 || V_INTIN(pb, 12) != 0)
	{
		if (V_INTIN(pb, 11) != 0 && V_INTIN(pb, 12) != 0)
		{
			width = V_INTIN(pb, 11);
			height = V_INTIN(pb, 12);
		} else
		{
			width = mfdb->fd_w;
			height = mfdb->fd_h;
		}
	} else
	{
		width = vdi_w;
		height = vdi_h;
	}
	width = (width + 15) & ~0x0f;
	if (bitplanes == 0)
	{
		bitplanes = mfdb->fd_nplanes;
	} else if (mfdb->fd_nplanes != 0 && mfdb->fd_nplanes != bitplanes)
	{
		goto error;
	}
	
	if (bitplanes != 1 && bitplanes != vdi_planes)
	{
		if (bitplanes != 0)
			goto error;
		bitplanes = vdi_planes;
	}
	lwidth = (width >> 3) * bitplanes;
	size = lwidth * height;
	addr = (uint8_t *)mfdb->fd_addr;

	if ((v = init_vwk(pb, h, h, width, height, bitplanes)) == NULL)
		goto error;
	v->driver_id = VDI_OFFSCREEN_DEVICE;
	vdi_init_bm(v);

	if (addr == 0)
	{
		addr = (uint8_t *)dos_alloc_anyram(size);
		if (addr == NULL)
		{
			vdi_clswk(v);
			goto error;
		}
		mfdb->fd_stand = 0;
		mfdb->fd_addr = addr;
		memset(addr, 0, size);
		v->to_free = addr;
	}
	
	v->bitmap_addr = addr;
	
	mfdb->fd_w = width;
	mfdb->fd_h = height;
	mfdb->fd_wdwidth = (width + 15) >> 4;
	mfdb->fd_nplanes = bitplanes;
	
	/* Need to convert input MFDB to device dependent format? */
	if (mfdb->fd_stand)
	{
		do_trnfm(v, mfdb, mfdb);
		mfdb->fd_stand = 0;
	}
	mfdb->fd_r1 = 0;
	mfdb->fd_r2 = 0;
	mfdb->fd_r3 = 0;
	
	v->inq_tab.pix_width = V_INTIN(pb, 13);
	v->inq_tab.pix_height = V_INTIN(pb, 14);
	
	return VDI_DONE;

error:
	vdi_release_handle(pb, h);
	return VDI_DONE;
}

static int vdi_v_open_bm(VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	
	V("v_open_bm[%d]: NOT YET IMPLEMENTED", V_HANDLE(pb));

	/* NYI; EdDI >= 1.20 */
	V_CONTRL(pb, 6) = 0;
	V_NPTSOUT(pb, 0);
	V_NINTOUT(pb, 0);
	return VDI_DONE;
}

/* -------------------------------------------------------------------------- */

static int vdi_v_opnwk(VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intin = PV_INTIN(pb);
	VWK *v;
	
	V("%s[%d]: %d", V_NINTIN(pb) >= 16 ? "v_opnprn" : "v_opnwk", V_HANDLE(pb), V_INTIN(pb, 0));
#if 0
	printf("%s[%d]: %d\n", V_NINTIN(pb) >= 16 ? "v_opnprn" : "v_opnwk", V_HANDLE(pb), V_INTIN(pb, 0));
	fflush(stdout);
#endif

	if (V_INTIN(pb, 0) <= LAST_SCREEN_DEVICE)
	{
		int i, j;
		
		close_all_wk();
		destroy_image();
		vdi_change_colors();

		init_window();
		
		phys_handle = 0;
		v = init_vwk(pb, phys_handle, phys_handle, vdi_w, vdi_h, vdi_planes);
		v_init_screen(v);
		
		/* our vars */
		v->req_col = &screen_pal;
		memcpy(v->req_col, initial_palette, sizeof(initial_palette));
		/* linea-vars */
		for (i = 0; i < 16; i++)
			for (j = 0; j < 3; j++)
				REQ_COL[i][j] = initial_palette[i][j];

		return VDI_DONE;
	}
	
	if (V_HANDLE(pb) == phys_handle)
	{
		V_HANDLE(pb) = 0; /* return failure */
		V_NINTOUT(pb, 0);
		V_NPTSOUT(pb, 0);
		{
			_WORD *intout = PV_INTOUT(pb);
			_WORD *ptsout = PV_PTSOUT(pb);
			int i;
			
			for (i = 0; i < 45; i++)
				V_INTOUT(pb, i) = 0;
			for (i = 0; i < 12; i++)
				PTSOUT(i) = 0;
		}
	} else
	{
		v = init_vwk(pb, V_HANDLE(pb), V_HANDLE(pb), vdi_w, vdi_h, vdi_planes);
		vdi_init_common(v);
	}
	return VDI_DONE;
}

/******************************************************************************/
/******************************************************************************/
/* Dispatcher                                                                 */
/******************************************************************************/
/******************************************************************************/

static int vdi_unknown(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD *intout = PV_INTOUT(pb);

	UNUSED(v);
	V("VDI%d,%d[%d]", V_OPCODE(pb), V_SUBCODE(pb), v->handle);
	V_INTOUT(pb, 0) = 0;
	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	return VDI_PASS;
}

/******************************************************************************/

static gboolean vdi_call(VDIPB *pb)
{
	_WORD *control;
	uint32_t op;
	VWK *v;
	gboolean vdi_done = VDI_PASS;
	
	control = PV_CONTROL(pb);
	op = OPCODE(V_OPCODE(pb), V_SUBCODE(pb));

	if (op == V_OPNWK)
	{
		_WORD *intin = PV_INTIN(pb);
		if (V_INTIN(pb, 0) <= LAST_SCREEN_DEVICE)
		{
			/*
			 * disable vbl drawing routine (cur_flag),
			 * it will only draw the mouse on an invisible screen
			 */
			CUR_FLAG = 0;
		}
	}

	{
		V_NINTOUT(pb, 0);
		V_NPTSOUT(pb, 0);
		switch (op)
		{
			/*
			 * functions that don't require a valid input handle:
			 */
		case V_OPNWK:
		case V_OPNVWK:
		case V_OPNBM:
		case V_OPEN_BM:
		case OPCODE(248, 0): /* vq_devinfo */
		case OPCODE(248, 4242): /* vq_ext_devinfo */
		case OPCODE(-1, 4): /* v_get_driver_info */
			v = NULL;
			break;
		default:
			{
				int h = V_HANDLE(pb);
				if (!VALID_V_HANDLE(h))
				{
					V("invalid handle: %d", h);
					return VDI_PASS;
				}
				v = vwk[h];
			}
			break;
		}
		switch (op)
		{
			case OPCODE(-1, 6): vdi_done = vdi_v_set_app_buff(v, pb); break;
			case OPCODE(-1, 4): vdi_done = vdi_v_get_driver_info(v, pb); break;
			case OPCODE(1, 0): vdi_done = vdi_v_opnwk(pb); break;
			case OPCODE(2, 0): vdi_done = v->drv.v_clswk(v, pb); break;
			case OPCODE(3, 0): vdi_done = v->drv.v_clrwk(v, pb); break;
			case OPCODE(4, 0): vdi_done = v->drv.v_updwk(v, pb); break;
			case OPCODE(5, 1): vdi_done = vdi_vq_chcells(v, pb); break;
			case OPCODE(5, 2): vdi_done = vdi_v_exit_cur(v, pb); break;
			case OPCODE(5, 3): vdi_done = vdi_v_enter_cur(v, pb); break;
			case OPCODE(5, 4): vdi_done = vdi_v_curup(v, pb); break;
			case OPCODE(5, 5): vdi_done = vdi_v_curdown(v, pb); break;
			case OPCODE(5, 6): vdi_done = vdi_v_curright(v, pb); break;
			case OPCODE(5, 7): vdi_done = vdi_v_curleft(v, pb); break;
			case OPCODE(5, 8): vdi_done = vdi_v_curhome(v, pb); break;
			case OPCODE(5, 9): vdi_done = vdi_v_eeos(v, pb); break;
			case OPCODE(5, 10): vdi_done = vdi_v_eeol(v, pb); break;
			case OPCODE(5, 11): vdi_done = vdi_vs_curaddress(v, pb); break;
			case OPCODE(5, 12): vdi_done = vdi_v_curtext(v, pb); break;
			case OPCODE(5, 13): vdi_done = vdi_v_rvon(v, pb); break;
			case OPCODE(5, 14): vdi_done = vdi_v_rvoff(v, pb); break;
			case OPCODE(5, 15): vdi_done = vdi_vq_curaddress(v, pb); break;
			case OPCODE(5, 16): vdi_done = vdi_vq_tabstatus(v, pb); break;
			case OPCODE(5, 17): vdi_done = vdi_v_hardcopy(v, pb); break;
			case OPCODE(5, 18): vdi_done = vdi_v_dspcur(v, pb); break;
			case OPCODE(5, 19): vdi_done = vdi_v_rmcur(v, pb); break;
			case OPCODE(5, 20): vdi_done = v->drv.v_form_adv(v, pb); break;
			case OPCODE(5, 21): vdi_done = v->drv.v_output_window(v, pb); break;
			case OPCODE(5, 22): vdi_done = v->drv.v_clear_disp_list(v, pb); break;
			case OPCODE(5, 23): vdi_done = v->drv.v_bit_image(v, pb); break;
			case OPCODE(5, 24): vdi_done = v->drv.vq_scan(v, pb); break;
			case OPCODE(5, 25): vdi_done = v->drv.v_alpha_text(v, pb); break;
			case OPCODE(5, 27): vdi_done = v->drv.v_orient(v, pb); break;
			case OPCODE(5, 28): vdi_done = v->drv.v_copies(v, pb); break;
			case OPCODE(5, 29): vdi_done = v->drv.v_tray(v, pb); break;
			case OPCODE(5, 32): vdi_done = v->drv.v_ps_halftone(v, pb); break;
			case OPCODE(5, 36): vdi_done = v->drv.vq_tray_names(v, pb); break;
			case OPCODE(5, 37): vdi_done = v->drv.v_page_size(v, pb); break;
			case OPCODE(5, 38): vdi_done = v->drv.vq_page_name(v, pb); break;
			case OPCODE(5, 39): vdi_done = v->drv.vq_prn_scaling(v, pb); break;
			case OPCODE(5, 60): vdi_done = vdi_vs_palette(v, pb); break;
			case OPCODE(5, 61): vdi_done = vdi_v_sound(v, pb); break;
			case OPCODE(5, 62): vdi_done = vdi_vs_mute(v, pb); break;
			case OPCODE(5, 76): vdi_done = v->drv.vs_calibrate(v, pb); break;
			case OPCODE(5, 77): vdi_done = v->drv.vq_calibrate(v, pb); break;
			case OPCODE(5, 81): vdi_done = v->drv.vt_resolution(v, pb); break;
			case OPCODE(5, 82): vdi_done = v->drv.vt_axis(v, pb); break;
			case OPCODE(5, 83): vdi_done = v->drv.vt_origin(v, pb); break;
			case OPCODE(5, 84): vdi_done = v->drv.vq_tdimensions(v, pb); break;
			case OPCODE(5, 85): vdi_done = v->drv.vt_alignment(v, pb); break;
			case OPCODE(5, 91): vdi_done = v->drv.vqp_films(v, pb); break; /* also vsp_film */
			case OPCODE(5, 92): vdi_done = v->drv.vqp_state(v, pb); break; /* also vqp_filmname */
			case OPCODE(5, 93): vdi_done = v->drv.vsp_state(v, pb); break; /* also vsc_expose */
			case OPCODE(5, 94): vdi_done = v->drv.vsp_save(v, pb); break;
			case OPCODE(5, 95): vdi_done = v->drv.vsp_message(v, pb); break;
			case OPCODE(5, 96): vdi_done = v->drv.vqp_error(v, pb); break;
			case OPCODE(5, 98): vdi_done = v->drv.v_meta_extents(v, pb); break;
			case OPCODE(5, 99): vdi_done = vdi_v_meta_esc(v, pb); break; /* v_write_meta,vm_pagesize,vm_coords,v_bez_qual */
			case OPCODE(5, 100): vdi_done = v->drv.vm_filename(v, pb); break;
			case OPCODE(5, 101): vdi_done = v->drv.v_offset(v, pb); break; /* also v_xbit_image */
			case OPCODE(5, 102): vdi_done = vdi_v_fontinit(v, pb); break; /* also vs_bkcolor */
			case OPCODE(5, 2000): vdi_done = ign_v_escape2000(v, pb); break;
			case OPCODE(5, 2100): vdi_done = vdi_vq_margins(v, pb); break;
			case OPCODE(5, 2103): vdi_done = vdi_vs_document_info(v, pb); break;
			case OPCODE(5, 18500): vdi_done = vdi_v_setrgbi(v, pb); break;
			case OPCODE(5, 18501): vdi_done = vdi_v_topbot(v, pb); break;
			case OPCODE(6, 0): vdi_done = v->drv.v_pline(v, pb); break;
			case OPCODE(6, 13): vdi_done = v->drv.v_bez(v, pb); break;
			case OPCODE(7, 0): vdi_done = v->drv.v_pmarker(v, pb); break;
			case OPCODE(8, 0): vdi_done = v->drv.v_gtext(v, pb); break;
			case OPCODE(9, 0): vdi_done = v->drv.v_fillarea(v, pb); break;
			case OPCODE(9, 13): vdi_done = v->drv.v_bez_fill(v, pb); break;
			case OPCODE(10, 0): vdi_done = ign_v_cellarray(v, pb); break;
			case OPCODE(11, 1): vdi_done = v->drv.v_bar(v, pb); break;
			case OPCODE(11, 2): vdi_done = v->drv.v_arc(v, pb); break;
			case OPCODE(11, 3): vdi_done = v->drv.v_pieslice(v, pb); break;
			case OPCODE(11, 4): vdi_done = v->drv.v_circle(v, pb); break;
			case OPCODE(11, 5): vdi_done = v->drv.v_ellipse(v, pb); break;
			case OPCODE(11, 6): vdi_done = v->drv.v_ellarc(v, pb); break;
			case OPCODE(11, 7): vdi_done = v->drv.v_ellpie(v, pb); break;
			case OPCODE(11, 8): vdi_done = v->drv.v_rbox(v, pb); break;
			case OPCODE(11, 9): vdi_done = v->drv.v_rfbox(v, pb); break;
			case OPCODE(11, 10): vdi_done = v->drv.v_justified(v, pb); break;
			case OPCODE(11, 11): vdi_done = vdi_v_etext(v, pb); break;
			case OPCODE(11, 13): vdi_done = vdi_v_bez_con(v, pb); break;
			case OPCODE(12, 0): vdi_done = vdi_vst_height(v, pb); break;
			case OPCODE(13, 0): vdi_done = vdi_vst_rotation(v, pb); break;
			case OPCODE(14, 0): vdi_done = vdi_vs_color(v, pb); break;
			case OPCODE(15, 0): vdi_done = vdi_vsl_type(v, pb); break;
			case OPCODE(16, 0): vdi_done = vdi_vsl_width(v, pb); break;
			case OPCODE(17, 0): vdi_done = vdi_vsl_color(v, pb); break;
			case OPCODE(18, 0): vdi_done = vdi_vsm_type(v, pb); break;
			case OPCODE(19, 0): vdi_done = vdi_vsm_height(v, pb); break;
			case OPCODE(20, 0): vdi_done = vdi_vsm_color(v, pb); break;
			case OPCODE(21, 0): vdi_done = vdi_vst_font(v, pb); break;
			case OPCODE(22, 0): vdi_done = vdi_vst_color(v, pb); break;
			case OPCODE(23, 0): vdi_done = vdi_vsf_interior(v, pb); break;
			case OPCODE(24, 0): vdi_done = vdi_vsf_style(v, pb); break;
			case OPCODE(25, 0): vdi_done = vdi_vsf_color(v, pb); break;
			case OPCODE(26, 0): vdi_done = vdi_vq_color(v, pb); break;
			case OPCODE(27, 0): vdi_done = ign_vq_cellarray(v, pb); break;
			case OPCODE(28, 0): vdi_done = vdi_vrq_locator(v, pb); break;
			case OPCODE(29, 0): vdi_done = vdi_vrq_valuator(v, pb); break;
			case OPCODE(30, 0): vdi_done = vdi_vrq_choice(v, pb); break;
			case OPCODE(31, 0): vdi_done = vdi_vrq_string(v, pb); break;
			case OPCODE(32, 0): vdi_done = vdi_vswr_mode(v, pb); break;
			case OPCODE(33, 0): vdi_done = vdi_vsin_mode(v, pb); break;
			case OPCODE(35, 0): vdi_done = vdi_vql_attributes(v, pb); break;
			case OPCODE(36, 0): vdi_done = vdi_vqm_attributes(v, pb); break;
			case OPCODE(37, 0): vdi_done = vdi_vqf_attributes(v, pb); break;
			case OPCODE(38, 0): vdi_done = vdi_vqt_attributes(v, pb); break;
			case OPCODE(39, 0): vdi_done = vdi_vst_alignment(v, pb); break;
			case OPCODE(96, 1): vdi_done = vdi_v_write_png(v, pb); break;
			case OPCODE(100, 0): vdi_done = vdi_v_opnvwk(pb); break;
			case OPCODE(100, 1): vdi_done = vdi_v_opnbm(pb); break;
			case OPCODE(100, 2): vdi_done = vdi_v_resize_bm(v, pb); break;
			case OPCODE(100, 3): vdi_done = vdi_v_open_bm(pb); break;
			case OPCODE(101, 0): vdi_done = vdi_v_clsvwk(v, pb); break;
			case OPCODE(101, 1): vdi_done = v->drv.v_clsbm(v, pb); break;
			case OPCODE(102, 0): vdi_done = vdi_vq_extnd(v, pb); break;
			case OPCODE(102, 1): vdi_done = vdi_vq_scrninfo(v, pb); break;
			case OPCODE(103, 0): vdi_done = v->drv.v_contourfill(v, pb); break;
			case OPCODE(104, 0): vdi_done = vdi_vsf_perimeter(v, pb); break;
			case OPCODE(105, 0): vdi_done = v->drv.v_get_pixel(v, pb); break;
			case OPCODE(106, 0): vdi_done = vdi_vst_effects(v, pb); break;
			case OPCODE(107, 0): vdi_done = vdi_vst_point(v, pb); break;
			case OPCODE(108, 0): vdi_done = vdi_vsl_ends(v, pb); break;
			case OPCODE(109, 0): vdi_done = v->drv.vro_cpyfm(v, pb); break;
			case OPCODE(110, 0): vdi_done = vdi_vr_trnfm(v, pb); break;
			case OPCODE(111, 0): vdi_done = vdi_vsc_form(v, pb); break;
			case OPCODE(112, 0): vdi_done = vdi_vsf_udpat(v, pb); break;
			case OPCODE(113, 0): vdi_done = vdi_vsl_udsty(v, pb); break;
			case OPCODE(114, 0): vdi_done = v->drv.vr_recfl(v, pb); break;
			case OPCODE(115, 0): vdi_done = vdi_vqin_mode(v, pb); break;
			case OPCODE(116, 0): vdi_done = vdi_vqt_extent(v, pb); break;
			case OPCODE(117, 0): vdi_done = vdi_vqt_width(v, pb); break;
			case OPCODE(118, 0): vdi_done = vdi_vex_timv(v, pb); break;
			case OPCODE(119, 0): vdi_done = vdi_vst_load_fonts(v, pb); break;
			case OPCODE(120, 0): vdi_done = vdi_vst_unload_fonts(v, pb); break;
			case OPCODE(121, 0): vdi_done = v->drv.vrt_cpyfm(v, pb); break;
			case OPCODE(122, 0): vdi_done = vdi_v_show_c(v, pb); break;
			case OPCODE(123, 0): vdi_done = vdi_v_hide_c(v, pb); break;
			case OPCODE(124, 0): vdi_done = vdi_vq_mouse(v, pb); break;
			case OPCODE(125, 0): vdi_done = vdi_vex_butv(v, pb); break;
			case OPCODE(126, 0): vdi_done = vdi_vex_motv(v, pb); break;
			case OPCODE(127, 0): vdi_done = vdi_vex_curv(v, pb); break;
			case OPCODE(128, 0): vdi_done = vdi_vq_key_s(v, pb); break;
			case OPCODE(129, 0): vdi_done = vdi_vs_clip(v, pb); break;
			case OPCODE(130, 0): vdi_done = vdi_vqt_name(v, pb); break;
			case OPCODE(130, 1): vdi_done = vdi_vqt_ext_name(v, pb); break;
			case OPCODE(131, 0): vdi_done = vdi_vqt_fontinfo(v, pb); break;
			case OPCODE(132, 0): vdi_done = vdi_vqt_justified(v, pb); break;
			case OPCODE(133, 0): vdi_done = vdi_vs_grayoverride(v, pb); break;
			case OPCODE(134, 0): vdi_done = vdi_vex_wheelv(v, pb); break; /* also v_pat_rotate */
			case OPCODE(138, 0): vdi_done = vdi_v_setrgb(v, pb); break;
			case OPCODE(138, 1): vdi_done = vdi_v_setrgb(v, pb); break;
			case OPCODE(138, 2): vdi_done = vdi_v_setrgb(v, pb); break;
			case OPCODE(138, 3): vdi_done = vdi_v_setrgb(v, pb); break;
			case OPCODE(170, 0): vdi_done = vdi_vr_transfer_bits(v, pb); break;
			case OPCODE(171, 0): vdi_done = vdi_vr_clip_rects_by_dst(v, pb); break;
			case OPCODE(171, 1): vdi_done = vdi_vr_clip_rects_by_src(v, pb); break;
			case OPCODE(171, 2): vdi_done = vdi_vr_clip_rects32_by_dst(v, pb); break;
			case OPCODE(171, 3): vdi_done = vdi_vr_clip_rects32_by_src(v, pb); break;
			case OPCODE(180, 0): vdi_done = vdi_v_create_driver_info(v, pb); break;
			case OPCODE(181, 0): vdi_done = vdi_v_delete_driver_info(v, pb); break;
			case OPCODE(182, 0): vdi_done = vdi_v_read_default_settings(v, pb); break;
			case OPCODE(182, 1): vdi_done = vdi_v_write_default_settings(v, pb); break;
			case OPCODE(190, 0): vdi_done = vdi_vqt_char_index(v, pb); break;
			case OPCODE(200, 0): vdi_done = vdi_vst_fg_color(v, pb); break;
			case OPCODE(200, 1): vdi_done = vdi_vsf_fg_color(v, pb); break;
			case OPCODE(200, 2): vdi_done = vdi_vsl_fg_color(v, pb); break;
			case OPCODE(200, 3): vdi_done = vdi_vsm_fg_color(v, pb); break;
			case OPCODE(200, 4): vdi_done = vdi_vsr_fg_color(v, pb); break;
			case OPCODE(201, 0): vdi_done = vdi_vst_bg_color(v, pb); break;
			case OPCODE(201, 1): vdi_done = vdi_vsf_bg_color(v, pb); break;
			case OPCODE(201, 2): vdi_done = vdi_vsl_bg_color(v, pb); break;
			case OPCODE(201, 3): vdi_done = vdi_vsm_bg_color(v, pb); break;
			case OPCODE(201, 4): vdi_done = vdi_vsr_bg_color(v, pb); break;
			case OPCODE(202, 0): vdi_done = vdi_vqt_fg_color(v, pb); break;
			case OPCODE(202, 1): vdi_done = vdi_vqf_fg_color(v, pb); break;
			case OPCODE(202, 2): vdi_done = vdi_vql_fg_color(v, pb); break;
			case OPCODE(202, 3): vdi_done = vdi_vqm_fg_color(v, pb); break;
			case OPCODE(202, 4): vdi_done = vdi_vqr_fg_color(v, pb); break;
			case OPCODE(203, 0): vdi_done = vdi_vqt_bg_color(v, pb); break;
			case OPCODE(203, 1): vdi_done = vdi_vqf_bg_color(v, pb); break;
			case OPCODE(203, 2): vdi_done = vdi_vql_bg_color(v, pb); break;
			case OPCODE(203, 3): vdi_done = vdi_vqm_bg_color(v, pb); break;
			case OPCODE(203, 4): vdi_done = vdi_vqr_bg_color(v, pb); break;
			case OPCODE(204, 0): vdi_done = vdi_v_color2value(v, pb); break;
			case OPCODE(204, 1): vdi_done = vdi_v_value2color(v, pb); break;
			case OPCODE(204, 2): vdi_done = vdi_v_color2nearest(v, pb); break;
			case OPCODE(204, 3): vdi_done = vdi_vq_px_format(v, pb); break;
			case OPCODE(205, 0): vdi_done = vdi_vs_ctab(v, pb); break;
			case OPCODE(205, 1): vdi_done = vdi_vs_ctab_entry(v, pb); break;
			case OPCODE(205, 2): vdi_done = vdi_vs_dflt_ctab(v, pb); break;
			case OPCODE(206, 0): vdi_done = vdi_vq_ctab(v, pb); break;
			case OPCODE(206, 1): vdi_done = vdi_vq_ctab_entry(v, pb); break;
			case OPCODE(206, 2): vdi_done = vdi_vq_ctab_id(v, pb); break;
			case OPCODE(206, 3): vdi_done = vdi_v_ctab_idx2vdi(v, pb); break;
			case OPCODE(206, 4): vdi_done = vdi_v_ctab_vdi2idx(v, pb); break;
			case OPCODE(206, 5): vdi_done = vdi_v_ctab_idx2value(v, pb); break;
			case OPCODE(206, 6): vdi_done = vdi_v_get_ctab_id(v, pb); break;
			case OPCODE(206, 7): vdi_done = vdi_vq_dflt_ctab(v, pb); break;
			case OPCODE(206, 8): vdi_done = vdi_v_create_ctab(v, pb); break;
			case OPCODE(206, 9): vdi_done = vdi_v_delete_ctab(v, pb); break;
			case OPCODE(207, 0): vdi_done = vdi_vs_hilite_color(v, pb); break;
			case OPCODE(207, 1): vdi_done = vdi_vs_min_color(v, pb); break;
			case OPCODE(207, 2): vdi_done = vdi_vs_max_color(v, pb); break;
			case OPCODE(207, 3): vdi_done = vdi_vs_weight_color(v, pb); break;
			case OPCODE(208, 0): vdi_done = vdi_v_create_itab(v, pb); break;
			case OPCODE(208, 1): vdi_done = vdi_v_delete_itab(v, pb); break;
			case OPCODE(209, 0): vdi_done = vdi_vq_hilite_color(v, pb); break;
			case OPCODE(209, 1): vdi_done = vdi_vq_min_color(v, pb); break;
			case OPCODE(209, 2): vdi_done = vdi_vq_max_color(v, pb); break;
			case OPCODE(209, 3): vdi_done = vdi_vq_weight_color(v, pb); break;
			case OPCODE(224, 100): vdi_done = vdi_vs_backmap(v, pb); break;
			case OPCODE(224, 101): vdi_done = vdi_vs_outmode(v, pb); break;
			case OPCODE(224, 105): vdi_done = vdi_vs_use_fonts(v, pb); break;
			case OPCODE(229, 0): vdi_done = vdi_vqt_xfntinfo(v, pb); break;
			case OPCODE(230, 0): vdi_done = vdi_vst_name(v, pb); break;
			case OPCODE(230, 100): vdi_done = vdi_vst_name_and_id(v, pb); break;
			case OPCODE(231, 0): vdi_done = vdi_vst_width(v, pb); break;
			case OPCODE(232, 0): vdi_done = vdi_vqt_fontheader(v, pb); break;
			case OPCODE(234, 0): vdi_done = vdi_vqt_trackkern(v, pb); break;
			case OPCODE(235, 0): vdi_done = vdi_vqt_pairkern(v, pb); break;
			case OPCODE(236, 0): vdi_done = vdi_vst_charmap(v, pb); break; /* also vst_map_mode */
			case OPCODE(237, 0): vdi_done = vdi_vst_kern(v, pb); break; /* also vst_track_offset */
			case OPCODE(239, 0): vdi_done = vdi_v_getbitmap_info(v, pb); break;
			case OPCODE(240, 0): vdi_done = vdi_vqt_f_extent(v, pb); break;
			case OPCODE(240, 4200): vdi_done = vdi_vqt_real_extent(v, pb); break;
			case OPCODE(241, 0): vdi_done = v->drv.v_ftext(v, pb); break;
			case OPCODE(242, 0): vdi_done = vdi_v_killoutline(v, pb); break;
			case OPCODE(243, 0): vdi_done = vdi_v_getoutline(v, pb); break;
			case OPCODE(243, 1): vdi_done = vdi_v_get_outline(v, pb); break;
			case OPCODE(243, 31): vdi_done = vdi_v_fgetoutline(v, pb); break;
			case OPCODE(244, 0): vdi_done = vdi_vst_scratch(v, pb); break;
			case OPCODE(245, 0): vdi_done = vdi_vst_error(v, pb); break;
			case OPCODE(246, 0): vdi_done = vdi_vst_arbpt(v, pb); break; /* also vst_arbpt32 */
			case OPCODE(247, 0): vdi_done = vdi_vqt_advance(v, pb); break; /* also vqt_advance32 */
			case OPCODE(248, 0): vdi_done = vdi_vq_devinfo(pb); break;
			case OPCODE(248, 4242): vdi_done = vdi_vq_ext_devinfo(pb); break;
			case OPCODE(249, 0): vdi_done = vdi_v_savecache(v, pb); break;
			case OPCODE(250, 0): vdi_done = vdi_v_loadcache(v, pb); break;
			case OPCODE(251, 0): vdi_done = vdi_v_flushcache(v, pb); break;
			case OPCODE(252, 0): vdi_done = vdi_vst_setsize(v, pb); break; /* also vst_setsize32 */
			case OPCODE(253, 0): vdi_done = vdi_vst_skew(v, pb); break;
			case OPCODE(254, 0): vdi_done = vdi_vqt_get_table(v, pb); break;
			case OPCODE(255, 0): vdi_done = vdi_vqt_cachesize(v, pb); break;
			case OPCODE(255, 100): vdi_done = vdi_vqt_cacheinfo(v, pb); break;
		
			default:
				vdi_done = vdi_unknown(v, pb);
				break;
		}
		/*
		 * many programs use incomplete vdi bindings, that don't clear
		 * the subcode for functions they don't expect to have any
		 * other subcode than 0. There are however some functions that were
		 * added later, or are specific for some GDOS.
		 * If we don't clear it, we might interpret a wrong subcode from
		 * some previous call.
		 */
		V_CONTRL(pb, 5) = 0;
	}
	return vdi_done;
}

void vditrap(VDIPB *pb)
{
	vdi_call(pb);
}

/******************************************************************************/
/******************************************************************************/
/* Initialization                                                             */
/******************************************************************************/
/******************************************************************************/

int vdi_phys_handle(void)
{
	return phys_handle;
}

/* -------------------------------------------------------------------------- */

void vdi_change_colors(void)
{
	int i, n;

	n = vdi_planes <= 8 ? (1 << vdi_planes) : MAX_VDI_COLS;
	switch (vdi_planes)
	{
	case 1:
		st_maptab = vdi_maptab2;
		st_revtab = vdi_revtab2;
		break;
	case 2:
		st_maptab = vdi_maptab4;
		st_revtab = vdi_revtab4;
		break;
	case 4:
		st_maptab = vdi_maptab16;
		st_revtab = vdi_revtab16;
		break;
	case 8:
		/* oops */
	default:
		st_maptab = vdi_maptab256;
		st_revtab = vdi_revtab256;
		n = MAX_VDI_COLS;
		break;
	}
	for (i = 0; i < MAX_VDI_COLS; i++)
		vdi_maptab[i] = n - 1;
	{
		V("vdi_change_colors: vdi_planes=%d", vdi_planes);
		for (i = 0; i < n; i++)
		{
			vdi_maptab[i] = st_maptab[i];
			V("vdi_maptab[%d] = %08x", i, vdi_maptab[i]);
		}
	}
}

/* -------------------------------------------------------------------------- */

void vdi_init(void)
{
	vdi_reset_vars();
}

/******************************************************************************/
/******************************************************************************/
/* Bios functions                                                             */
/******************************************************************************/
/******************************************************************************/

int vdi_cursconf(_WORD func, _WORD rate)
{
	VWK *v;
	struct alpha_info *vt52;
	
	/*
	 * If VDI is enabled, we dont pass the call
	 * to the ROM bios, because we didnt update the
	 * corresponding BIOS variables.
	 * We could do that (all information is well
	 * known and contained in negative linea variables),
	 * but it would only let the ROM do some
	 * screen update and cursor blinking on a screen
	 * that actually isnt visible. It might also make
	 * problems on older TOS that were not prepared for
	 * screen sizes > 32k.
	 */
	if (!VALID_S_HANDLE(phys_handle))
		return FALSE;
	v = vwk[phys_handle];
	vt52 = v->vt52;
	if (vt52 == NULL)
		return FALSE;
	/* DREG(0) = 0; */
	switch (func)
	{
	case 0:
		v_disable_alpha_cursor(v);
		break;
	case 1:
		v_enable_alpha_cursor(v);
		break;
	case 2:
		vt52->blinking = TRUE;
		break;
	case 3:
		vt52->blinking = FALSE;
		break;
	case 4:
		vt52->blink_rate = rate;
		break;
	case 5:
		/* DREG(0) = vt52->blink_rate; */
		break;
	case 6:
		vt52->blink_delay = rate;
		break;
	case 7:
		/* DREG(0) = vt52->blink_delay; */
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

/* -------------------------------------------------------------------------- */

void vdi_cursblink(void)
{
	VWK *v;
	struct alpha_info *vt52;
	
	if (!VALID_S_HANDLE(phys_handle))
		return;
	v = vwk[phys_handle];
	vt52 = v->vt52;
	if (vt52 == NULL)
		return;
	if (vt52->curs_hid_cnt != 0 || !vt52->blinking)
		return;
	if (--vt52->blink_count != 0)
		return;
	{
		if (vt52->cursor_inverse)
			v_hide_alpha_cursor(v);
		else
			v_show_alpha_cursor(v);
	}
}
