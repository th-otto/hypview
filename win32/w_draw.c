#include "hv_defs.h"
#include "w_draw.h"
#include "hypdebug.h"
#include "resource.rh"
#include "pattern.h"
#include <math.h>

#define NO_BRUSH ((HBRUSH)0)
#define NO_FONT ((HFONT)0)
#define NO_BITMAP ((HBITMAP)0)

static HBRUSH pattern_brush[NUM_PATTERNS];
static HBITMAP pattern_bitmap[NUM_PATTERNS];

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void w_exit_brush(void)
{
	int i;

	for (i = 0; i < NUM_PATTERNS; i++)
	{
		if (pattern_brush[i] != NO_BRUSH)
		{
			DeleteObject(pattern_brush[i]);
			pattern_brush[i] = NO_BRUSH;
		}
		if (pattern_bitmap[i] != NO_BITMAP)
		{
			DeleteObject(pattern_bitmap[i]);
			pattern_bitmap[i] = NO_BITMAP;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

gboolean w_init_brush(void)
{
	int i, j;
	HBITMAP hbmp;
	char mask[PATTERN_SIZE];

	for (i = 0; i < NUM_PATTERNS; i++)
	{
		BITMAP bitmap;
		const unsigned char *data = pattern_bits + i * PATTERN_SIZE;
		
		for (j = 0; j < PATTERN_SIZE; j++)
			mask[j] = ~bitrevtab[data[j]];
		bitmap.bmType = 0;
		bitmap.bmWidth = PATTERN_WIDTH;
		bitmap.bmHeight = PATTERN_HEIGHT;
		bitmap.bmWidthBytes = ((((PATTERN_WIDTH + 7) / 8) + 1) / 2) * 2;
		bitmap.bmPlanes = 1;
		bitmap.bmBitsPixel = 1;
		bitmap.bmBits = mask;
		hbmp = CreateBitmapIndirect(&bitmap);
		pattern_bitmap[i] = hbmp;
		if (hbmp == NO_BITMAP)
		{
			pattern_brush[i] = NO_BRUSH;
		} else
		{
			pattern_brush[i] = CreatePatternBrush(hbmp);
		}
	}
	return TRUE;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static HBRUSH w_fill_brush(HDC hdc, int fillstyle, COLORREF color, gboolean *del)
{
	HBRUSH hBrush;

	switch (fillstyle)
	{
	case IP_HOLLOW:
		hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
		*del = FALSE;
		break;

	case IP_SOLID:
		if (color == W_PAL_WHITE)
		{
			hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
			*del = FALSE;
		} else if (color == W_PAL_BLACK)
		{
			hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
			*del = FALSE;
		} else
		{
			hBrush = CreateSolidBrush(color);
			*del = TRUE;
		}
		break;

	case IP_1PATT:
	case IP_2PATT:
	case IP_3PATT:
	case IP_4PATT:
	case IP_5PATT:
	case IP_6PATT:
	case IP_7PATT:
	         case  9: case 10: case 11: case 12: case 13: case 14: case 15:
	case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
	case 24: case 25: case 26: case 27: case 28: case 29: case 30: case 31:
	case 32: case 33: case 34: case 35: case 36:
		hBrush = pattern_brush[fillstyle];
		if (hBrush == NO_BRUSH)
			hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
		SetTextColor(hdc, color);
		*del = FALSE;
		break;
	default:
		hBrush = NO_BRUSH;
		*del = FALSE;
		break;
	}
	return hBrush;
}

/*** ---------------------------------------------------------------------- ***/

void w_inithdc(HDC hdc)
{
	SetBkMode(hdc, OPAQUE);
	SetBkColor(hdc, W_PAL_WHITE);
	SetTextColor(hdc, W_PAL_BLACK);
	SetMapMode(hdc, MM_TEXT);
	SetTextAlign(hdc, TA_TOP | TA_LEFT);
}

/*** ---------------------------------------------------------------------- ***/

HDC W_BeginPaint(HWND hwnd, PAINTSTRUCT *ps, GRECT *gr)
{
	HDC hdc = BeginPaint(hwnd, ps);
	RectToGrect(gr, &ps->rcPaint);
	w_inithdc(hdc);
	return hdc;
}

/*** ---------------------------------------------------------------------- ***/

void W_EndPaint(HWND hwnd, PAINTSTRUCT *ps)
{
	EndPaint(hwnd, ps);
}

/*** ---------------------------------------------------------------------- ***/

void W_Fill_Rect(HDC hdc, const GRECT *gr, int fillstyle, COLORREF color)
{
	HBRUSH hBrush;
	gboolean del;
	RECT re;

	hBrush = w_fill_brush(hdc, fillstyle, color, &del);
	GrectToRect(&re, gr);
	FillRect(hdc, &re, hBrush);
	if (del)
	{
		DeleteObject(hBrush);
	}
}

/*** ---------------------------------------------------------------------- ***/

void W_Invert_Rect(HDC hdc, const GRECT *gr)
{
	RECT re;

	GrectToRect(&re, gr);
	if (re.left < re.right)
	{
		int x = re.left;

		re.left = re.right;
		re.right = x;
	}
	if (re.top < re.bottom)
	{
		int x = re.top;

		re.top = re.bottom;
		re.bottom = x;
	}
	InvertRect(hdc, &re);
	/* PatBlt(hdc, gr->g_x, gr->g_y, gr->g_w, gr->g_h, DSTINVERT); */
}

/*** ---------------------------------------------------------------------- ***/

void W_Rectangle(HDC hdc, const GRECT *gr, int linestyle, COLORREF color)
{
	POINT pp[5];

	pp[0].x = gr->g_x;
	pp[0].y = gr->g_y;
	pp[1].x = pp[0].x + gr->g_w - 1;
	pp[1].y = pp[0].y;
	pp[2].x = pp[1].x;
	pp[2].y = pp[0].y + gr->g_h - 1;
	pp[3].x = pp[0].x;
	pp[3].y = pp[2].y;
	pp[4].x = pp[0].x;
	pp[4].y = pp[0].y;

	W_Lines(hdc, pp, 5, linestyle, color);
}

/*** ---------------------------------------------------------------------- ***/

static HPEN W_PenCreate(HDC hdc, int width, int penStyle, COLORREF color, _UWORD *line_mask)
{
	HPEN pen;
	int style;
	
	switch (penStyle)
	{
	case W_PEN_SOLID:
		*line_mask = 0xffff;
		style = PS_SOLID;
		break;

	case W_PEN_LONGDASH:
		*line_mask = 0xFFF0;
		style = PS_DASH;
		break;

	case W_PEN_DOT:
		*line_mask = 0xE0E0;
		style = PS_DOT;
		break;

	case W_PEN_DASHDOT:
		*line_mask = 0xFF18;
		style = PS_DASHDOT;
		break;

	case W_PEN_DASH:
		*line_mask = 0xff00;
		style = PS_DASH;
		break;

	case W_PEN_DASHDOTDOT:
		*line_mask = 0xf198;
		style = PS_DASHDOTDOT;
		break;

	case W_PEN_NULL:
		*line_mask = 0;
		style = PS_NULL;
		break;

	case W_PEN_USER:
		*line_mask = 0xaaaa;
		style = PS_DOT;
		break;

	default:
		*line_mask = 0xffff;
		style = PS_SOLID;
		break;
	}
	pen = CreatePen(style, width, color);
	return (HPEN) SelectObject(hdc, pen);
}

/*** ---------------------------------------------------------------------- ***/

static void W_PenDelete(HDC hdc, HPEN obj)
{
	HGDIOBJ pen;
	
	if (obj == NULL)
		return;
	pen = SelectObject(hdc, (HGDIOBJ)obj);
	DeleteObject(pen);
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

void W_rounded_box(HDC hdc, const GRECT *gr, int fillstyle, COLORREF color)
{
	int rdeltax, rdeltay;
	int xc, yc, xrad, yrad;
	int x1, y1, x2, y2;
	int i, j;
	POINT points[21];
	_UWORD line_mask;
	
	x1 = gr->g_x;
	y1 = gr->g_y;
	x2 = x1 + gr->g_w - 1;
	y2 = y1 + gr->g_h - 1;

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
	
	if (fillstyle != IP_HOLLOW)
	{
		gboolean del;
		HBRUSH hBrush = w_fill_brush(hdc, fillstyle, color, &del);
		HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, (HGDIOBJ)hBrush);
		HPEN pen;
		pen = W_PenCreate(hdc, 1, fillstyle == IP_SOLID ? W_PEN_NULL : W_PEN_SOLID, color, &line_mask);
		Polygon(hdc, points, 20);
		SelectObject(hdc, (HGDIOBJ)oldbrush);
		if (del)
		{
			DeleteObject(hBrush);
		}
		W_PenDelete(hdc, pen);
	} else if (fillstyle != IP_SOLID)
	{
		W_Lines(hdc, points, 21, W_PEN_SOLID, color);
	}
}

/*** ---------------------------------------------------------------------- ***/

#define vec_len(x, y) (int)sqrt((x) * (x) + (y) * (y))

static void W_Draw_Arrow(HDC hdc, POINT *xy, int npoints, int inc)
{
	int arrow_len, arrow_wid, line_len;
	int dx, dy;
	int base_x, base_y, ht_x, ht_y;
	int xybeg;
	int i;
	POINT triangle[4];
	
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

	triangle[0].x = xy[0].x + base_x - ht_x;
	triangle[0].y = xy[0].y + base_y - ht_y;
	triangle[1].x = xy[0].x - base_x - ht_x;
	triangle[1].y = xy[0].y - base_y - ht_y;
	triangle[2].x = xy[0].x;
	triangle[2].y = xy[0].y;
	Polygon(hdc, triangle, 3);

	/* Adjust the end point and all points skipped. */

	xy[0].x -= ht_x;
	xy[0].y -= ht_y;

	while ((xybeg -= inc) != 0)
	{
		xy[xybeg].x = xy[0].x;
		xy[xybeg].y = xy[0].y;
	}
}

void W_Draw_Arrows(HDC hdc, POINT *xy, int npoints, COLORREF color, unsigned char line_ends)
{
	int x_start, y_start, new_x_start, new_y_start;
	HPEN pen;
	_UWORD line_mask;
	HBRUSH hbrush, oldbrush;
	gboolean del;
	
	if (line_ends == 0)
		return;
	
	/* Function "arrow" will alter the end of the line segment.  Save the */
	/* starting point of the polyline in case two calls to "arrow" are    */
	/* necessary.                                                         */
	new_x_start = x_start = xy[0].x;
	new_y_start = y_start = xy[0].y;
	
	pen = W_PenCreate(hdc, 1, W_PEN_NULL, color, &line_mask);
	hbrush = w_fill_brush(hdc, IP_SOLID, color, &del);
	oldbrush = (HBRUSH)SelectObject(hdc, (HGDIOBJ)hbrush);
	
	if (line_ends & (1 << 0))
	{
		/* draw arrow at start of line */
		W_Draw_Arrow(hdc, xy, npoints, 1);
		new_x_start = xy[0].x;
		new_y_start = xy[0].y;
	}
	
	if (line_ends & (1 << 1))
	{
		/* draw arrow at end of line */
		xy[0].x = x_start;
		xy[0].y = y_start;
		W_Draw_Arrow(hdc, xy + npoints - 1, npoints, -1);
		xy[0].x = new_x_start;
		xy[0].y = new_y_start;
	}
	
	SelectObject(hdc, (HGDIOBJ)oldbrush);
	if (del)
	{
		DeleteObject(hbrush);
	}
	W_PenDelete(hdc, pen);
}

/*** ---------------------------------------------------------------------- ***/

typedef struct {
	HDC hdc;
	_UWORD mask;
	_UWORD linestyle;
	COLORREF color;
} LDDA;


static void CALLBACK linedda_proc(int x, int y, LPARAM data)
{
	LDDA *dda;
	
	dda = (LDDA *)data;
	if (dda->mask & dda->linestyle)
		SetPixel(dda->hdc, x, y, dda->color);
	dda->mask >>= 1;
	if (dda->mask == 0)
		dda->mask = 0x8000;
}


void W_Lines(HDC hdc, const POINT points[], int npoints, int linestyle, COLORREF color)
{
	HPEN pen;
	_UWORD line_mask;
	
	pen = W_PenCreate(hdc, 1, linestyle, color, &line_mask);
	if (linestyle == W_PEN_SOLID)
	{
		Polyline(hdc, points, npoints);
		if (npoints > 0 &&
			(points[0].x != points[npoints-1].x ||
			 points[0].y != points[npoints-1].y))
		{
			SetPixel(hdc, points[npoints-1].x, points[npoints-1].y, color);
		}
	} else
	{
		LDDA dda;
		POINT pAr[2];
		int i;
		
		dda.hdc = hdc;
		dda.mask = 0x8000;
		dda.linestyle = line_mask;
		dda.color = color;
		pAr[1].x = points[0].x;
		pAr[1].y = points[0].y;
		for (i = 1; i < npoints; i++)
		{
			pAr[0].x = pAr[1].x;
			pAr[0].y = pAr[1].y;
			pAr[1].x = points[i].x;
			pAr[1].y = points[i].y;
			LineDDA(pAr[0].x, pAr[0].y, pAr[1].x, pAr[1].y, linedda_proc, (LPARAM)&dda);
		}
	}
	W_PenDelete(hdc, pen);
}

/*** ---------------------------------------------------------------------- ***/

void W_TextExtent(HDC hdc, const wchar_t *text, int *w, int *h)
{
	size_t len;
	
	len = wcslen(text);
	W_NTextExtent(hdc, text, len, w, h);
}

/*** ---------------------------------------------------------------------- ***/

void W_NTextExtent(HDC hdc, const wchar_t *text, size_t len, int *w, int *h)
{
	SIZE lpSi;
	
	if (GetTextExtentPoint32W(hdc, text, len, &lpSi))
	{
		*w = lpSi.cx;
		*h = lpSi.cy;
	} else
	{
		*w = 0;
		*h = 0;
	}
}

/*** ---------------------------------------------------------------------- ***/

void W_ClipText(HDC hdc, GRECT *gr, const char *str, int hdir, int vdir)
{
	RECT r;
	_WORD flag;
	size_t len;
	wchar_t *wstr;
	
	flag = 0;
	switch (vdir)
	{
	case -1:
		flag |= DT_TOP;
		break;
	case 0:
		flag |= DT_VCENTER | DT_SINGLELINE;
		break;
	case 1:
		flag |= DT_BOTTOM;
		break;
	}
	switch (hdir)
	{
	case -1:
		flag |= DT_LEFT;
		break;
	case 0:
		flag |= DT_CENTER;
		break;
	case 1:
		flag |= DT_RIGHT;
		break;
	}

	GrectToRect(&r, gr);
	wstr = hyp_utf8_to_wchar(str, STR0TERM, NULL);
	len = wcslen(wstr);
	DrawTextW(hdc, wstr, len, &r, flag);
	g_free(wstr);
}

/*** ---------------------------------------------------------------------- ***/

void W_Font_Default(FONT_ATTR *attr)
{
	attr->size = 120;
	attr->textstyle = HYP_TXT_NORMAL;
	strcpy(attr->name, "System Font");
	attr->hfont = (HFONT)GetStockObject(DEVICE_DEFAULT_FONT);
}

/*** ---------------------------------------------------------------------- ***/

void W_TDFrame(HDC hdc, const GRECT *r, int height, int flags)
{
	POINT p1[3];
	GRECT r1;
	int t;
	COLORREF color;
	
	if (flags & FILL)
	{
		W_Fill_Rect(hdc, r, IP_SOLID, W_PAL_LGRAY);
	}

	r1 = *r;

	if (flags & OUTLINE)
	{
		W_Rectangle(hdc, &r1, W_PEN_SOLID, W_PAL_BLACK);

		r1.g_x = r->g_x + 1;
		r1.g_y = r->g_y + 1;
		r1.g_w = r->g_w - 2;
		r1.g_h = r->g_h - 2;
	}
	if (flags & INVERT)
		color = W_PAL_DGRAY;
	else
		color = W_PAL_WHITE;

	for (t = 0; t < height; t++)
	{
		p1[0].x = r1.g_x + r1.g_w - 1 - t;
		p1[0].y = r1.g_y + t;
		p1[1].x = r1.g_x + t;
		p1[1].y = r1.g_y + t;
		p1[2].x = r1.g_x + t;
		p1[2].y = r1.g_y + r1.g_h - 1 - t;
		W_Lines(hdc, p1, 3, W_PEN_SOLID, color);
	}

	if (flags & INVERT)
	{
		if (height < 2)
			color = W_PAL_WHITE;
		else
			color = W_PAL_LGRAY;
	} else
	{
		color = W_PAL_DGRAY;
	}

	if ((height < 2 && (flags & INVERT)) || (!(flags & INVERT)))
	{
		for (t = 0; t < height; t++)
		{
			p1[0].x = r1.g_x + r1.g_w - 1 - t;
			p1[0].y = r1.g_y + t;
			p1[1].x = r1.g_x + r1.g_w - 1 - t;
			p1[1].y = r1.g_y + r1.g_h - 1 - t;
			p1[2].x = r1.g_x + t;
			p1[2].y = r1.g_y + r1.g_h - 1 - t;
			W_Lines(hdc, p1, 3, W_PEN_SOLID, color);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void W_Draw_Image(HDC hdc, _WORD x, _WORD y, _WORD w, _WORD h, _VOID *data, COLORREF fg, COLORREF bg, DWORD mode)
{
	HDC hdcMem;
	COLORREF oldColor, oldBk;
	IBITMAP *map;
	BITMAP *ic;
	HBITMAP srcBitmap;

	map = (IBITMAP *)data;
	if (map == NULL || map->bm_magic != BITMAP_MAGIC)
	{
		hyp_debug("W_Draw_Image: data not fixed");
		return;
	}
	ic = (BITMAP *)map->bm_info;
	oldColor = SetTextColor(hdc, fg);
	oldBk = SetBkColor(hdc, bg);
	srcBitmap = CreateBitmapIndirect(ic);
	hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem, srcBitmap);
	SetMapMode(hdcMem, GetMapMode(hdc));
	BitBlt(hdc, x, y, w, h, hdcMem, 0, 0, mode);
	DeleteDC(hdcMem);
	DeleteObject(srcBitmap);
	SetTextColor(hdc, oldColor);
	SetBkColor(hdc, oldBk);
}

/*** ---------------------------------------------------------------------- ***/

void W_Draw_Picture(HDC hdc, _WORD x, _WORD y, GRECT *area, MFDB *pic, DWORD mode)
{
	HDC hdcMem;
	COLORREF oldColor, oldBk;
	IBITMAP *map;
	HBITMAP srcBitmap;
	
	map = (IBITMAP *)pic->fd_addr;
	if (map == NULL || map->bm_magic != BITMAP_MAGIC)
	{
		hyp_debug("W_Draw_Picture: data not fixed");
		return;
	}
	oldColor = SetTextColor(hdc, W_PAL_BLACK);
	oldBk = SetBkColor(hdc, W_PAL_WHITE);

	if (pic->fd_nplanes == 1)
	{
		BITMAP *ic;

		ic = (BITMAP *)map->bm_info;
		srcBitmap = CreateBitmapIndirect(ic);
	} else
	{
		BITMAPINFO *info;

		info = (BITMAPINFO *) map->bm_info;
		/* srcBitmap = CreateBitmap(pic->fd_w, pic->fd_h, GetDeviceCaps(hdc, PLANES), GetDeviceCaps(hdc, BITSPIXEL), NULL);
		   SetDIBits(srcBitmap, 0, pic->fd_h, map->bm_data, info, DIB_RGB_COLORS); */
		srcBitmap = CreateDIBitmap(hdc, &info->bmiHeader, CBM_INIT, map->bm_data, info, DIB_RGB_COLORS);
	}
	hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem, srcBitmap);
	SetMapMode(hdcMem, GetMapMode(hdc));
	BitBlt(hdc, x, y, area->g_w, area->g_h, hdcMem, area->g_x, area->g_y, mode);
	DeleteDC(hdcMem);
	DeleteObject(srcBitmap);
	SetTextColor(hdc, oldColor);
	SetBkColor(hdc, oldBk);
}
