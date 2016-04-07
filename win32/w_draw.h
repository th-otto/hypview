#ifndef __W_DRAW_H__
#define __W_DRAW_H__

#define INVERT      1
#define FILL        2
#define OUTLINE     4

#define inside(xx, yy, x, y, w, h) \
	((xx) >= (x) && (xx) < ((x) + (w)) && \
	 (yy) >= (y) && (yy) < ((y) + (h)))
#define rc_inside(xx, yy, gr) \
	inside(xx, yy, (gr)->g_x, (gr)->g_y, (gr)->g_w, (gr)->g_h)

typedef struct _font_attr {
	int size;						/* size of font, in 1/10 points */
	unsigned int textstyle;			/* bitmask of text effects */
	char name[FONT_NAME_LEN];		/* font Family */
	HFONT hfont;
} FONT_ATTR;

void W_TDFrame(HDC hdc, const GRECT *r, int height, int flags);
void W_Lines(HDC hdc, const POINT points[], int npoints, int style, COLORREF color);
void W_Rectangle(HDC hdc, const GRECT *gr, int linestyle, COLORREF color);
void W_Fill_Rect(HDC hdc, const GRECT *gr, int style, COLORREF color);
void W_rounded_box(HDC hdc, const GRECT *gr, int fillstyle, COLORREF color);
void W_Draw_Arrows(HDC hdc, POINT *xy, int npoints, COLORREF color, unsigned char line_ends);

void W_NTextExtent(HDC hdc, const wchar_t *text, size_t len, int *w, int *h);
void W_TextExtent(HDC hdc, const wchar_t *text, int *w, int *h);
void W_ClipText(HDC hdc, GRECT *gr, const char *str, int hdir, int vdir);

void w_inithdc(HDC hdc);
HDC W_BeginPaint(HWND hwnd, PAINTSTRUCT *ps, GRECT *gr);
void W_EndPaint(HWND hwnd, PAINTSTRUCT *ps);

char *W_Fontdesc(const FONT_ATTR *attr);
gboolean W_Fontname(const char *name, FONT_ATTR *attr);
void W_Font_Default(FONT_ATTR *attr);

void W_Draw_Image(HDC hdc, _WORD x, _WORD y, _WORD w, _WORD h, _VOID *data, COLORREF fg, COLORREF bg, DWORD mode);
void W_Draw_Picture(HDC hdc, _WORD x, _WORD y, GRECT *area, MFDB *pic, DWORD mode);

gboolean w_init_brush(void);
void w_exit_brush(void);


#define W_PAL_WHITE    PALETTERGB(255, 255, 255)
#define W_PAL_BLACK    PALETTERGB(0, 0, 0)
#define W_PAL_RED      PALETTERGB(255, 0, 0)
#define W_PAL_GREEN    PALETTERGB(0, 255, 0)
#define W_PAL_BLUE     PALETTERGB(0, 0, 255)
#define W_PAL_CYAN     PALETTERGB(0, 255, 255)
#define W_PAL_YELLOW   PALETTERGB(255, 255, 0)
#define W_PAL_MAGENTA  PALETTERGB(255, 0, 255)
#define W_PAL_LGRAY    PALETTERGB(204, 204, 204)
#define W_PAL_DGRAY    PALETTERGB(136, 136, 136)
#define W_PAL_DRED     PALETTERGB(136, 0, 0)
#define W_PAL_DGREEN   PALETTERGB(0, 136, 0)
#define W_PAL_DBLUE    PALETTERGB(0, 0, 136)
#define W_PAL_DCYAN    PALETTERGB(0, 136, 136)
#define W_PAL_DYELLOW  PALETTERGB(136, 136, 0)
#define W_PAL_DMAGENTA PALETTERGB(136, 0, 136)

#define IP_HOLLOW		0
#define IP_1PATT		1
#define IP_2PATT		2
#define IP_3PATT		3
#define IP_4PATT		4
#define IP_5PATT		5
#define IP_6PATT		6
#define IP_7PATT		7
#define IP_SOLID		8

#define W_PEN_NULL             0
#define W_PEN_SOLID            1
#define W_PEN_LONGDASH         2
#define W_PEN_DOT              3
#define W_PEN_DASHDOT          4
#define W_PEN_DASH             5
#define W_PEN_DASHDOTDOT       6
#define W_PEN_USER             7

typedef struct {
	_ULONG bm_magic;
#define BITMAP_MAGIC 0x4afcdeadl
	_UBYTE *bm_orig_data;
	_UBYTE *bm_data;
	_VOID  *bm_info;
} IBITMAP;

#endif /* __W_DRAW_H__ */
