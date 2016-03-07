#include "hv_defs.h"
#include "w_draw.h"
#include "resource.rh"

#define NO_BRUSH ((HBRUSH)0)
#define NO_FONT ((HFONT)0)

#define NUM_PATTERNS 37
static HBRUSH pattern_brush[NUM_PATTERNS];

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
	case  8: case  9: case 10: case 11: case 12: case 13: case 14: case 15:
	case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
	case 24: case 25: case 26: case 27: case 28: case 29: case 30: case 31:
	case 32: case 33: case 34: case 35: case 36: case 37:
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

HDC W_BeginPaint(HWND hwnd, PAINTSTRUCT *ps, GRECT *gr)
{
	HDC hdc = BeginPaint(hwnd, ps);
	RectToGrect(gr, &ps->rcPaint);
	SetBkMode(hdc, OPAQUE);
	SetBkColor(hdc, W_PAL_WHITE);
	SetTextColor(hdc, W_PAL_BLACK);
	SetMapMode(hdc, MM_TEXT);
	SetTextAlign(hdc, TA_TOP | TA_LEFT);
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

	case W_PEN_DOTDOT:
		*line_mask = 0xaaaa;
		style = PS_DOT;
		break;

	case W_PEN_NULL:
		*line_mask = 0;
		style = PS_NULL;
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

void W_TextExtent(HFONT hfont, const wchar_t *text, int *w, int *h)
{
	size_t len;
	
	len = wcslen(text);
	W_NTextExtent(hfont, text, len, w, h);
}

/*** ---------------------------------------------------------------------- ***/

void W_NTextExtent(HFONT hfont, const wchar_t *text, size_t len, int *w, int *h)
{
	HDC hDC;
	HFONT oldHfont;
	
	hDC = GetDC(HWND_DESKTOP);
	oldHfont = (HFONT)SelectObject(hDC, hfont);

	{
		SIZE lpSi;
		
		if (GetTextExtentPointW(hDC, text, len, &lpSi))
		{
			*w = lpSi.cx;
			*h = lpSi.cy;
		} else
		{
			*w = 0;
			*h = 0;
		}
	}

 	if (oldHfont != NO_FONT)
	{
		SelectObject(hDC, oldHfont);
	}
	ReleaseDC(HWND_DESKTOP, hDC);
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
	DrawText(hdc, str, len, &r, flag);
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

gboolean W_Add_Font(FONT_ATTR *attr)
{
	(void) attr;
	return FALSE;
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
