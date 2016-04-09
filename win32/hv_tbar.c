#include "hv_defs.h"
#include "w_draw.h"
#include "windebug.h"
#include "hypdebug.h"
#include "resource.rh"

#define WCN_Toolbar L"HV_Toolbar"
#define WCN_ToolHelp L"HV_Toolhelp"

#define DEFAULT_TOOLBAR_FONT_DESC "MS Sans Serif,,120"

static HWND captured;
#define TIMER_ID 5

static int timer_id;
#define TIME_VAL 400
static int time_val = TIME_VAL;
static char *toolbar_font_desc;
static FONT_ATTR toolbar_font;

static int bitmap_width = 32;
static int bitmap_height = 21;

#define FRAMESIZE 2	/* width&height of frame around toolbar window */

#define BUTTONWIDTH  (bitmap_width + 2 * FRAMESIZE)
#define BUTTONHEIGHT (bitmap_height + 2 * FRAMESIZE)

#define TB_X_OFF 2	/* x-offset of icon in toolbar */
#define TB_Y_OFF 2	/* y-offset of icon in toolbar */

#define TOOLHEIGHT  (BUTTONHEIGHT + 2 * TB_Y_OFF)

#define TB_SEP_WIDTH 6	/* width of a separator in toolbar */

typedef struct
{
	int xPos;
	int yPos;
	int domove;
#define TB_MOVE_CHECK     0
#define TB_MOVE_ACTIVATE  1
#define TB_MOVE_MOUSEMOVE 2
	int x_off, y_off;
} CLICKINFO;

typedef struct
{
	HDC hdc;
	WINDOW_DATA *win;
	int x_off, y_off;
} DRAWINFO;

typedef struct
{
	int x_off, y_off;
	int obj;
	GRECT button;
} POSINFO;


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void toolbar_redraw(WINDOW_DATA *win)
{
	if (win->td && win->td->hwnd)
		InvalidateRect(win->td->hwnd, NULL, TRUE);
}

/*** ---------------------------------------------------------------------- ***/

void EnableMenuObj(HMENU menu, int obj, gboolean enable)
{
	EnableMenuItem(menu, obj, MF_BYCOMMAND | (enable ? MF_ENABLED : (MF_DISABLED|MF_GRAYED)));
}

/*** ---------------------------------------------------------------------- ***/

void CheckMenuObj(HMENU menu, int obj, gboolean check)
{
	CheckMenuItem(menu, obj, MF_BYCOMMAND | (check ? MF_CHECKED : MF_UNCHECKED));
}

/*** ---------------------------------------------------------------------- ***/

static char *get_search_str(WINDOW_DATA *win)
{
	wchar_t str[256];
	int len;
	
	if ((len = GetWindowTextW(win->searchentry, str, 256)) <= 0)
		return NULL;
	return hyp_wchar_to_utf8(str, len);
}

/*** ---------------------------------------------------------------------- ***/

void ToolbarUpdate(WINDOW_DATA *win, gboolean redraw)
{
	DOCUMENT *doc = win->data;
	
	/* autolocator active? */
	if (doc->buttons.searchbox && win->searchbox)
	{
		char *search = get_search_str(win);
		if (!empty(search))
		{
			g_free(search);
			ShowWindow(win->searchbox, SW_SHOW);
			return;
		}
	}
	if (win->searchbox)
		ShowWindow(win->searchbox, SW_HIDE);
	
	doc->buttons.back = TRUE;
	doc->buttons.history = TRUE;
	doc->buttons.bookmarks = TRUE;
	doc->buttons.menu = TRUE;
	doc->buttons.info = TRUE;
	doc->buttons.save = TRUE;
	doc->buttons.remarker_running = StartRemarker(win, remarker_check, TRUE) >= 0;
	doc->buttons.remarker = TRUE;
	
	if (win->history == NULL)
	{
		doc->buttons.back = FALSE;
		doc->buttons.history = FALSE;
	}
	
	if (!win->is_popup)
	{
		hv_update_winmenu(win);
#define EnableButton(obj, enable) \
		if (enable) \
			win->m_buttons[obj] &= ~WS_DISABLED; \
		else \
			win->m_buttons[obj] |= WS_DISABLED

		EnableButton(TO_REMARKER, doc->buttons.remarker);
		EnableButton(TO_BOOKMARKS, doc->buttons.bookmarks);
		EnableButton(TO_INFO, doc->buttons.info);
		EnableButton(TO_BACK, doc->buttons.back);
		EnableButton(TO_HISTORY, doc->buttons.history);

		EnableButton(TO_CATALOG, !empty(gl_profile.viewer.catalog_file));

		/* next buttons are type specific */
		EnableButton(TO_PREV, doc->buttons.previous);
		EnableButton(TO_PREV_PHYS, doc->buttons.prevphys);
		EnableButton(TO_HOME, doc->buttons.home);
		EnableButton(TO_NEXT, doc->buttons.next);
		EnableButton(TO_NEXT_PHYS, doc->buttons.nextphys);
		EnableButton(TO_FIRST, doc->buttons.first);
		EnableButton(TO_LAST, doc->buttons.last);
		EnableButton(TO_INDEX, doc->buttons.index);
		EnableButton(TO_HELP, doc->buttons.help);
		EnableButton(TO_REFERENCES, doc->buttons.references);
#undef EnableButton
	}
	{
		if (doc->buttons.remarker_running)
			win->m_buttons[TO_REMARKER] |= WS_VISIBLE;
		else
			win->m_buttons[TO_REMARKER] &= ~WS_VISIBLE;
	}
	
	if (redraw)
	{
		toolbar_redraw(win);
	}	
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_enum(TOOL_DATA *td, const int *defs, int num_defs, int maxx, void (*enumproc)(TOOL_DATA *td, int entry_idx, const TOOLBAR_ENTRY *te, int xs, int ys, int def_idx, void *specialdata), void *specialdata)
{
	int def_idx;
	int xs = TB_X_OFF;
	int ys = TB_Y_OFF;
	
	def_idx = 0;
	while (def_idx < num_defs && *defs != TB_ENDMARK)
	{
		switch (*defs)
		{
		case TB_SEPARATOR:
			if (xs + TB_SEP_WIDTH > maxx)
			{
				xs = TB_X_OFF;
				ys += TOOLHEIGHT - TB_Y_OFF;
			}
			enumproc(td, *defs, td->entries, xs, ys, def_idx, specialdata);
			xs += TB_SEP_WIDTH;
			break;
		default:
			if (xs + BUTTONWIDTH > maxx)
			{
				xs = TB_X_OFF;
				ys += TOOLHEIGHT - TB_Y_OFF;
			}
			enumproc(td, *defs, td->entries, xs, ys, def_idx, specialdata);
			xs += BUTTONWIDTH;
			break;
		}
		defs++;
		def_idx++;
	}
	enumproc(td, TB_ENDMARK, NULL, xs, ys, def_idx, specialdata);
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_pos(TOOL_DATA *td, int entry_idx, const TOOLBAR_ENTRY *te, int xs, int ys, int def_idx, void *specialdata)
{
	POSINFO *dw;

	UNUSED(te);
	UNUSED(td);
	UNUSED(def_idx);
	
	dw = (POSINFO *)specialdata;
	if (entry_idx == dw->obj)
	{
		dw->button.g_x = xs + dw->x_off;
		dw->button.g_y = ys + dw->y_off;
		dw->button.g_w = BUTTONWIDTH;
		dw->button.g_h = BUTTONHEIGHT;
	}
}

/*** ---------------------------------------------------------------------- ***/

gboolean position_popup(HMENU menu, struct popup_pos *pos, int *xret, int *yret)
{
	WINDOW_DATA *win = pos->window;
	RECT a, r;
	POSINFO dw;
	TOOL_DATA *td;
	GRECT r1;
	POINT p;
	
	td = win->td;
	if (td == NULL)
		return FALSE;
	UNUSED(menu);

	GetClientRect(td->hwnd, &r);
	RectToGrect(&r1, &r);
	dw.x_off = r1.g_x;
	dw.y_off = r1.g_y;
	dw.obj = pos->obj;
	dw.button.g_x = 0;
	dw.button.g_y = 0;
	dw.button.g_w = 0;
	dw.button.g_h = 0;
	toolbar_enum(td, td->definitions, td->num_definitions, r1.g_w, toolbar_pos, &dw);
	GrectToRect(&r, &dw.button);
	a = r;
	
	r.top += a.bottom - a.top;
#if _WIN32_WINNT >= 0x601
	{
		SIZE s;
		POINT p;
		s.cx = a.right - a.left;
		s.cy = a.bottom - a.top;
		p.x = r.left;
		p.y = r.top;
		CalculatePopupWindowPosition(&p, &s, TPM_LEFTALIGN, NULL, &r);
	}
#endif
	p.x = r.left;
	p.y = r.top;
	ClientToScreen(td->hwnd, &p);
	*xret = p.x;
	*yret = p.y;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/* Handle mouse click on toolbar */
void ToolbarClick(WINDOW_DATA *win, enum toolbutton obj, int button)
{
	if ((int)obj < 0)
		RemoveSearchBox(win);
	else if (win->m_buttons[obj] & WS_DISABLED)
		return;

	CheckFiledate(win);		/* Check if file has changed */
	
	switch (obj)
	{
	case TO_LOAD:
		SelectFileLoad(win);
		break;
	case TO_SAVE:
		SelectFileSave(win);
		break;
	case TO_INDEX:
		GotoIndex(win);
		break;
	case TO_CATALOG:
		GotoCatalog(win);
		break;
	case TO_REFERENCES:
		HypExtRefPopup(win, button);
		break;
	case TO_HELP:
		GotoHelp(win);
		break;
	case TO_HISTORY:
		HistoryPopup(win, obj, button);
		break;
	case TO_BACK:
		if (button == 2)
			HistoryPopup(win, obj, button);
		else
			GoBack(win);
		break;
	case TO_NEXT:
	case TO_PREV:
	case TO_NEXT_PHYS:
	case TO_PREV_PHYS:
	case TO_FIRST:
	case TO_LAST:
	case TO_HOME:
		GoThisButton(win, obj);
		break;
	case TO_BOOKMARKS:
		MarkerPopup(win, button);
		break;
	case TO_INFO:
		DocumentInfos(win);
		break;
	case TO_REMARKER:
		BlockOperation(win, CO_REMARKER);
		break;
	default:
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void RemoveSearchBox(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;

	/* Is the autolocator/search box displayed? */
	if (doc->buttons.searchbox && win->searchentry)
	{
		doc->buttons.searchbox = FALSE;	/* disable it */
		SetWindowTextW(win->searchentry, L"");			/* clear autolocator string */

		ToolbarUpdate(win, TRUE);	/* update toolbar */
	}
}

/*** ---------------------------------------------------------------------- ***/

static void tool_help_destroy(TOOL_DATA *td)
{
	if (td != NULL && td->toolbar_help_hwnd != NO_WINDOW)
	{
		if (timer_id != 0)
		{
			KillTimer(td->toolbar_help_hwnd, TIMER_ID);
			timer_id = 0;
		}
		DestroyWindow(td->toolbar_help_hwnd);
		td->toolbar_help_hwnd = NO_WINDOW;
		g_free(td->help_text);
		td->help_text = NULL;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void tool_help_create(TOOL_DATA *td, const char *text, int x, int y)
{
	HWND parent;
	int w, h;
	wchar_t *wtext;
	HDC hdc;
	HFONT oldfont;
	
	parent = GetParent(td->hwnd);
	wtext = hyp_utf8_to_wchar(text, STR0TERM, NULL);
	hdc = GetDC(td->hwnd);
	oldfont = (HFONT)SelectObject(hdc, toolbar_font.hfont);
	W_TextExtent(hdc, wtext, &w, &h);
	SelectObject(hdc, (HGDIOBJ)oldfont);
	ReleaseDC(td->hwnd, hdc);
	w += 20;
	h += 6;
	td->toolbar_help_hwnd = CreateWindowExW(
		WS_EX_TOPMOST,
		WCN_ToolHelp,
		wtext,
		WS_CHILD,
		x, y, w, h,
		parent,
		0,
		GetInstance(),
		td);
	if (td->toolbar_help_hwnd == NO_WINDOW)
	{
		fprintf(stderr, "can't create %s window: %s\n", "tooltip", win32_errstring(GetLastError()));
		return;
	}
	if (timer_id == 0)
	{
		timer_id = SetTimer(td->toolbar_help_hwnd, TIMER_ID, time_val, NULL);
	}
	/*
	 * if timer creation failed, show window immediately
	 */
	if (timer_id == 0)
		ShowWindow(td->toolbar_help_hwnd, SW_SHOW);
	g_free(wtext);
	td->help_text = g_strdup(text);
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_help_settext(TOOL_DATA *td, const char *text, int x, int y)
{
	RECT r;
	GRECT gr;
	int th;
	HWND parent;
	
	UNUSED(y);
	if (td != NULL && td->hwnd != NO_WINDOW)
	{
		if (td->toolbar_help_hwnd == NO_WINDOW ||
			td->help_text == NULL ||
			strcmp(td->help_text, text) != 0)
		{
			parent = GetParent(td->hwnd);
			GetClientRect(parent, &r);
			RectToGrect(&gr, &r);
			th = td->toolbar_size(td, &gr);
			tool_help_destroy(td);
			tool_help_create(td, text, x + 8, th);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_help_paint(HDC hdc, GRECT *gr, const char *text)
{
	int oldmode;
	HFONT oldfont;
	
	oldmode = SetBkMode(hdc, OPAQUE);
	W_Fill_Rect(hdc, gr, IP_SOLID, W_PAL_YELLOW);
#if 0
	W_TDFrame(hdc, gr, 1, 0);
#else
	{
		W_Rectangle(hdc, gr, W_PEN_SOLID, W_PAL_BLACK);
	}
#endif
	SetTextColor(hdc, W_PAL_BLACK);
	oldfont = (HFONT)SelectObject(hdc, toolbar_font.hfont);
	SetBkMode(hdc, TRANSPARENT);
	W_ClipText(hdc, gr, text, 0, 0);
	(void) SelectObject(hdc, oldfont);
	SetBkMode(hdc, oldmode);
}

/*** ---------------------------------------------------------------------- ***/

static LRESULT CALLBACK toolhelpWndProc(HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
	TOOL_DATA *td;
	
	win32debug_msg_print(stderr, "toolhelpWndProc", hwnd, message, wParam, lParam);
	switch (message)
	{
	case WM_CREATE:
		td = (TOOL_DATA *) (((LPCREATESTRUCT)lParam)->lpCreateParams);
		if (td != NULL)
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)td);
			td->toolbar_help_hwnd = hwnd;
		}
		break;
		
	case WM_DESTROY:
		td = (TOOL_DATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		SetWindowLongPtr(hwnd, 0, 0l);
		if (td != NULL)
			td->toolbar_help_hwnd = NO_WINDOW;
		break;
	
	case WM_TIMER:
		if (wParam == TIMER_ID)
		{
			td = (TOOL_DATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (td != NULL)
			{
				if (td->toolbar_help_hwnd != NO_WINDOW && !IsWindowVisible(hwnd))
				{
					KillTimer(hwnd, TIMER_ID);
					timer_id = 0;
					ShowWindow(hwnd, SW_SHOW);
					time_val = 1;
				}
			}
		}
		win32debug_msg_end("toolhelpWndProc", message, "0");
		return 0;
		
	case WM_PAINT:
		td = (TOOL_DATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (td  != NULL)
		{
			GRECT gr;
			PAINTSTRUCT ps;
			HDC hdc;
			
			hdc = W_BeginPaint(hwnd, &ps, &gr);
			toolbar_help_paint(hdc, &gr, td->help_text);
			W_EndPaint(hwnd, &ps);
		}
		win32debug_msg_end("toolhelpWndProc", message, "0");
		return 0;
	}
	win32debug_msg_end("toolhelpWndProc", message, "DefWindowProc");
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static LRESULT CALLBACK toolbarWndProc(HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
	WINDOW_DATA *win;
	TOOL_DATA *td;
	
	win32debug_msg_print(stderr, "toolbarWndProc", hwnd, message, wParam, lParam);
	switch (message)
	{
	case WM_CREATE:
		win = (WINDOW_DATA *) (((LPCREATESTRUCT)lParam)->lpCreateParams);
		if (win != NULL && (td = win->td) != NULL)
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)win);
			td->hwnd = hwnd;
		}
		break;
		
	case WM_DESTROY:
		win = (WINDOW_DATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0l);
		if (win != NULL && (td = win->td) != NULL)
			td->hwnd = 0;
		break;
		
	case WM_MBUTTONUP:
		win = (WINDOW_DATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (win != NULL && (td = win->td) != NULL)
		{
			if (captured != 0)
			{
				tool_help_destroy(td);
				ReleaseCapture();
				captured = 0;
				time_val = TIME_VAL;
			}
			td->buttonpress = FALSE;
			/* if (message == WM_RBUTTONUP)
				td->toolbar_config(win, tb_reconfig); */
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		win = (WINDOW_DATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (win != NULL && (td = win->td) != NULL)
		{
			int buttonnum = message == WM_LBUTTONUP ? 1 : message == WM_RBUTTONUP ? 2 : 3;
			if (captured != 0)
			{
				tool_help_destroy(td);
				ReleaseCapture();
				captured = 0;
				time_val = TIME_VAL;
			}
			td->toolbar_button_up(win, buttonnum);
		}
		break;

	case WM_MOUSEMOVE:
		win = (WINDOW_DATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (win != NULL && (td = win->td) != NULL)
		{
			int x, y;
			RECT r;
			GRECT gr;
			
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			if (captured != 0)
			{
				RECT client;

				GetClientRect(hwnd, &client);
				if (x > client.left && x < client.right && y > client.top && y < client.bottom)
				{

				} else if (td->buttonpress == FALSE)
				{
					tool_help_destroy(td);
					ReleaseCapture();
					captured = 0;
					time_val = TIME_VAL;
				}
			} else
			{
				captured = hwnd;
				SetCapture(hwnd);
				time_val = TIME_VAL;
				/* tool_help_create(td, "", x + 8, y - 8); */
			}

			GetClientRect(hwnd, &r);
			RectToGrect(&gr, &r);
			if (td->toolbar_mouse_move(td, &gr, x, y))
				td->toolbar_mouse_down(td, FALSE, &gr, x, y);
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		win = (WINDOW_DATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (win != NULL && (td = win->td) != NULL)
		{
			RECT r;
			GRECT gr;

			GetClientRect(hwnd, &r);
			RectToGrect(&gr, &r);
			if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN)
				td->toolbar_mouse_down(td, TRUE, &gr, LOWORD(lParam), HIWORD(lParam));
			if (captured == 0)
			{
				SetCapture(hwnd);
				captured = hwnd;
				time_val = TIME_VAL;
			}
		}
		break;

	case WM_PAINT:
		win = (WINDOW_DATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (win != NULL && (td = win->td) != NULL)
		{
			PAINTSTRUCT ps;
			GRECT gr;
			HDC hdc;
			
			hdc = W_BeginPaint(hwnd, &ps, &gr);
			td->toolbar_paint(hdc, win, &gr);
			W_EndPaint(hwnd, &ps);
		}
		win32debug_msg_end("toolbarWndProc", message, "0");
		return 0;
	}
	win32debug_msg_end("toolbarWndProc", message, "DefWindowProc");
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_refresh(TOOL_DATA *td, const GRECT *gr)
{
	RECT r;
	
	GrectToRect(&r, gr);
	InvalidateRect(td->hwnd, &r, TRUE);
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_drawicon(HDC hdc, WINDOW_DATA *win, const GRECT *button, int entry_idx, gboolean selected)
{
	TOOL_DATA *td = win->td;
	int icon_id = td->entries[entry_idx].icon_id;
	HICON icon;
	int x = button->g_x + FRAMESIZE + (selected ? 1 : 0);
	int y = button->g_y + FRAMESIZE + (selected ? 1 : 0);

	if (!(win->m_buttons[td->entries[entry_idx].config_id] & WS_VISIBLE))
		return;
	icon = (HICON)LoadImageW(GetInstance(), MAKEINTRESOURCEW(icon_id), IMAGE_ICON, 0, 0, LR_SHARED);
	DrawIconEx(hdc, x, y, icon, bitmap_width, bitmap_height, 0, 0, DI_NORMAL);
	if (selected)
		W_TDFrame(hdc, button, FRAMESIZE, INVERT | OUTLINE);
	else
		W_TDFrame(hdc, button, FRAMESIZE, OUTLINE);
	if (win->m_buttons[td->entries[entry_idx].config_id] & WS_DISABLED)
	{
		HANDLE oldObj = SelectObject(hdc, GetStockObject(GRAY_BRUSH));

		PatBlt(hdc, x, y, bitmap_width, bitmap_height, /* (DWORD) */ 0xFA0089L);
		SelectObject(hdc, oldObj);
	}
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_draw(TOOL_DATA *td, int entry_idx, const TOOLBAR_ENTRY *te, int xs, int ys, int def_idx, void *specialdata)
{
	GRECT button;
	DRAWINFO *dw;

	UNUSED(te);

	dw = (DRAWINFO *)specialdata;
	if (entry_idx >= 0)
	{
		button.g_x = xs + dw->x_off;
		button.g_y = ys + dw->y_off;
		button.g_w = BUTTONWIDTH;
		button.g_h = BUTTONHEIGHT;
		td->toolbar_drawicon(dw->hdc, dw->win, &button, entry_idx, def_idx == td->buttondown);
	}
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_paint(HDC hdc, WINDOW_DATA *win, const GRECT *gr)
{
	DRAWINFO dw;
	int oldmode;
	GRECT r1;
	RECT r;
	TOOL_DATA *td = win->td;
	
	oldmode = SetBkMode(hdc, OPAQUE);
	W_TDFrame(hdc, gr, 1, FILL | OUTLINE);

	dw.hdc = hdc;
	dw.win = win;
	r1 = *gr;
	GetClientRect(td->hwnd, &r);
	RectToGrect(&r1, &r);
	dw.x_off = r1.g_x;
	dw.y_off = r1.g_y;
	toolbar_enum(td, td->definitions, td->num_definitions, r1.g_w, toolbar_draw, &dw);
	SetBkMode(hdc, oldmode);
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_click(TOOL_DATA *td, int entry_idx, const TOOLBAR_ENTRY *te, int xs, int ys, int def_idx, void *specialdata)
{
	GRECT button;
	CLICKINFO *di;

	di = (CLICKINFO *)specialdata;
	if (entry_idx >= 0)
	{
		button.g_x = xs + di->x_off;
		button.g_y = ys + di->y_off;
		button.g_w = BUTTONWIDTH;
		button.g_h = BUTTONHEIGHT;
		if (rc_inside(di->xPos, di->yPos, &button))
		{
			if (di->domove == TB_MOVE_MOUSEMOVE)
			{
				if (td->toolbar_help_settext != FUNK_NULL)
				{
					const char *comment;
					
					comment = te[entry_idx].comment;
					if (comment != NULL)
					{
						td->toolbar_help_settext(td, comment, xs, ys);
					}
				}
			} else
			{
				td->buttondown = def_idx;

				td->buttonxs = button.g_x;
				td->buttonys = button.g_y;
				if (di->domove == TB_MOVE_ACTIVATE)
				{
					if (td->buttonsave != def_idx)
						if (td->toolbar_refresh != FUNK_NULL)
							td->toolbar_refresh(td, &button);
				} else
				{
					/* TB_MOVE_CHECK */
					if (td->toolbar_refresh != FUNK_NULL)
						td->toolbar_refresh(td, &button);
				}
			}
		} else
		{
			if (di->domove == TB_MOVE_ACTIVATE)
			{
				if (td->buttonsave == def_idx)
				{
					if (td->buttondown == td->buttonsave)
						td->buttondown = -1;
					if (td->toolbar_refresh != FUNK_NULL)
						td->toolbar_refresh(td, &button);
					td->buttonsave = -1;
				}
			}
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

static gboolean toolbar_mouse_move(TOOL_DATA *td, const GRECT *gr, int mousex, int mousey)
{
	if (td->buttonpress == FALSE)
	{
		CLICKINFO ci;

		ci.xPos = mousex;
		ci.yPos = mousey;
		ci.domove = TB_MOVE_MOUSEMOVE;
		ci.x_off = gr->g_x;
		ci.y_off = gr->g_y;
		toolbar_enum(td, td->definitions, td->num_definitions, gr->g_w, toolbar_click, &ci);
		return FALSE;
	}
	td->buttonsave = td->buttondown;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_mouse_down(TOOL_DATA *td, gboolean buttondown, const GRECT *gr, int mousex, int mousey)
{
	CLICKINFO ci;

	ci.xPos = mousex;
	ci.yPos = mousey;
	if (buttondown == FALSE)
		ci.domove = TB_MOVE_ACTIVATE;
	else
		ci.domove = TB_MOVE_CHECK;
	ci.x_off = gr->g_x;
	ci.y_off = gr->g_y;
	td->buttondown = -1;
	td->buttonpress = TRUE;
	toolbar_enum(td, td->definitions, td->num_definitions, gr->g_w, toolbar_click, &ci);
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_button_up(WINDOW_DATA *win, int buttonnum)
{
	TOOL_DATA *td = win->td;
	
	if (td->buttondown >= 0 &&
		td->definitions != NULL &&
		td->buttondown < td->num_definitions &&
		td->entries != NULL &&
		td->definitions[td->buttondown] >= 0 &&
		td->definitions[td->buttondown] < td->num_entries)
	{
		GRECT button;

		button.g_x = td->buttonxs;
		button.g_y = td->buttonys;
		button.g_w = BUTTONWIDTH;
		button.g_h = BUTTONHEIGHT;

		/* PostMessage(GetParent(td->hwnd), WM_COMMAND, MAKEWPARAM(td->entries[td->definitions[td->buttondown]].menu_id, 0), 0); */
		ToolbarClick(win, (enum toolbutton)td->entries[td->definitions[td->buttondown]].config_id, buttonnum);
		td->buttondown = -1;
		if (td->toolbar_refresh != FUNK_NULL)
			td->toolbar_refresh(td, &button);
	}
	td->buttonpress = FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static void tb_size_proc(TOOL_DATA *td, int entry_idx, const TOOLBAR_ENTRY *te, int xs, int ys, int def_idx, void *specialdata)
{
	_WORD *h = (_WORD *)specialdata;
	
	UNUSED(td);
	UNUSED(te);
	UNUSED(def_idx);
	UNUSED(xs);
	switch (entry_idx)
	{
	case TB_SEPARATOR:
	default:
		*h = ys + TOOLHEIGHT - TB_Y_OFF;
		break;
	case TB_ENDMARK:
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

static int toolbar_size(TOOL_DATA *td, GRECT *r)
{
	_WORD th = TOOLHEIGHT;
	
	toolbar_enum(td, td->definitions, td->num_definitions, r->g_w, tb_size_proc, &th);
	return th;
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_open(WINDOW_DATA *win)
{
	TOOL_DATA *td;
	
	if (win == NULL || (td = win->td) == NULL)
		return;

	if (toolbar_font_desc == NULL)
		toolbar_font_desc = g_strdup(DEFAULT_TOOLBAR_FONT_DESC);
	toolbar_setfont(toolbar_font_desc);
	
	if (td->hwnd == NO_WINDOW)
	{
		HWND parent;
		
		parent = win->hwnd;
		
		td->hwnd = CreateWindowW(
			WCN_Toolbar,
			NULL,
			WS_CHILD,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			parent,
			NULL,
			GetInstance(),
			win);

		if (td->hwnd == NO_WINDOW)
		{
			fprintf(stderr, "can't create %s window: %s\n", "toolbar", win32_errstring(GetLastError()));
			return;
		}
	}
	td->toolbar_help_settext = toolbar_help_settext;
	td->toolbar_refresh = toolbar_refresh;
	td->toolbar_drawicon = toolbar_drawicon;
	
	WindowCalcScroll(win);
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_close(WINDOW_DATA *win)
{
	TOOL_DATA *td;
	
	if (win == NULL || (td = win->td) == NULL)
		return;
	
	if (td->hwnd != NO_WINDOW)
	{
		tool_help_destroy(td);
		DestroyWindow(td->hwnd);
		td->hwnd = NO_WINDOW;
	}
	td->toolbar_help_settext = FUNK_NULL;
	td->toolbar_refresh = FUNK_NULL;
	td->toolbar_drawicon = FUNK_NULL;
	
	WindowCalcScroll(win);
}

/*** ---------------------------------------------------------------------- ***/

static void toolbar_exit(WINDOW_DATA *win)
{
	TOOL_DATA *td;

	if ((td = win->td) != NULL)
	{
		g_free(td);
		win->td = NULL;
	}
	g_freep(&toolbar_font_desc);
}

/*** ---------------------------------------------------------------------- ***/

TOOL_DATA *toolbar_init(WINDOW_DATA *win, const int *definitions, int num_definitions, const TOOLBAR_ENTRY *entries, int num_entries)
{
	TOOL_DATA *td;
	int i;
	
	if (win == NULL)
		return NULL;
	
	td = win->td;
	if (td != NULL)
	{
		td->toolbar_close(win);
		td->toolbar_exit(win);
	}
	
	if ((td = g_new0(TOOL_DATA, 1)) != NULL)
	{
		td->buttonsave = 0;
		td->buttondown = -1;
		td->buttonpress = FALSE;
		td->buttonxs = 0;
		td->buttonys = 0;
		td->hwnd = NO_WINDOW;
		td->toolbar_help_hwnd = NO_WINDOW;
		td->help_text = NULL;

		td->toolbar_paint = toolbar_paint;
		td->toolbar_size = toolbar_size;
		td->toolbar_mouse_move = toolbar_mouse_move;
		td->toolbar_mouse_down = toolbar_mouse_down;
		td->toolbar_button_up = toolbar_button_up;
		td->toolbar_exit = toolbar_exit;
	
		td->toolbar_open = toolbar_open;
		td->toolbar_close = toolbar_close;
	
		td->toolbar_help_settext = FUNK_NULL;
		td->toolbar_refresh = FUNK_NULL;
	
		td->definitions = definitions;
		td->num_definitions = num_definitions;
		td->entries = entries;
		td->num_entries = num_entries;
		td->ontop = TRUE;
		td->visible = TRUE;
		for (i = 0; i < num_entries; i++)
			win->m_buttons[entries[i].config_id] = 0;
		for (i = 0; i < num_definitions; i++)
		{
			int id = definitions[i];
			if (id >= 0 && id < num_entries)
				win->m_buttons[entries[id].config_id] = WS_VISIBLE;
		}
	}
	win->td = td;
	return td;
}

/*** ---------------------------------------------------------------------- ***/

void toolbar_setfont(const char *desc)
{
	FONT_ATTR attr;
	
	if (desc != toolbar_font_desc)
	{
		if (toolbar_font_desc != NULL)
			g_free(toolbar_font_desc);
		toolbar_font_desc = g_strdup(desc);
	}
	if (W_Fontname(toolbar_font_desc, &attr))
	{
		toolbar_font = attr;
	} else
	{
		W_Font_Default(&toolbar_font);
	}
}

/*** ---------------------------------------------------------------------- ***/

void toolbar_register_classes(HINSTANCE hinst)
{
	WNDCLASSW wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hinst;
	wndclass.lpszMenuName = NULL;
	wndclass.hIcon = 0;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = 0;	/* GetStockObject(WHITE_BRUSH); */
	wndclass.lpszClassName = WCN_Toolbar;
	wndclass.lpfnWndProc = toolbarWndProc;
	if (RegisterClassW(&wndclass) == 0)
	{
		hyp_debug("can't register %s window class: %s", "toolbar", win32_errstring(GetLastError()));
	}
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_SAVEBITS;
	wndclass.lpszClassName = WCN_ToolHelp;
	wndclass.lpfnWndProc = toolhelpWndProc;
	if (RegisterClassW(&wndclass) == 0)
	{
		hyp_debug("can't register %s window class: %s", "toolhelp", win32_errstring(GetLastError()));
	}
	{
		HICON icon;
		BITMAP bitmap;
		ICONINFO info;
		
		icon = (HICON)LoadImageW(GetInstance(), MAKEINTRESOURCEW(IDI_NEXT), IMAGE_ICON, 0, 0, LR_SHARED);
		GetIconInfo(icon, &info);
		if (info.hbmColor)
		{
			GetObject((HGDIOBJ)info.hbmColor, sizeof(bitmap), &bitmap);
			bitmap_width = bitmap.bmWidth;
			bitmap_height = bitmap.bmHeight;
			DeleteObject(info.hbmColor);
		} else
		{
			GetObject((HGDIOBJ)info.hbmMask, sizeof(bitmap), &bitmap);
			bitmap_width = bitmap.bmWidth;
			bitmap_height = bitmap.bmHeight / 2;
		}
		DeleteObject(info.hbmMask);
	}
}
