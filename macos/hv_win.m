#include "hv_defs.h"
#include "hypdebug.h"
#include "w_draw.h"

static char *default_geometry;

#define set_tooltip_text(win, txt) /* NYI */

#if 1
#undef dprintf
#define dprintf(x) hyp_debug  x
#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void hv_win_set_geometry(const char *geometry)
{
	g_free(default_geometry);
	default_geometry = g_strdup(geometry);
}

/*** ---------------------------------------------------------------------- ***/

void hv_win_open(WINDOW_DATA *win)
{
	[win deminiaturize: win];
	[win makeKeyWindow];
}

/*** ---------------------------------------------------------------------- ***/

void WindowCalcScroll(WINDOW_DATA *win)
{
	/* TODO */
}

/*------------------------------------------------------------------*/

void macos_destroy_window(WINDOW_DATA *win)
{
	if (win!= NO_WINDOW)
	{
		if (!win->is_popup)
			[NSApp removeWindowsItem: win];
		[win close];
		[win release];
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#if 0
static void macos_hypview_window_finalize(WINDOW_DATA *win)
{
	DOCUMENT *doc;
	
	if (win != NULL)
	{
		doc = hypwin_doc(win);
		if (win->popup)
		{
			WINDOW_DATA *pop = win->popup;
			macos_destroy_window(pop);
		}
		if (win->parentwin)
			win->parentwin->popup = NULL;
		hypdoc_unref(doc);
		if (!win->is_popup)
		{
			RemoveAllHistoryEntries(win);
			all_list = g_slist_remove(all_list, win);
		}
		g_freep(&win->title);
		[win release];
		check_toplevels(NULL);
	}
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static void on_quit(WINDOW_DATA *win)
{
	GSList *l;
	
	for (l = all_list; l; l = l->next)
	{
		win = (WINDOW_DATA *)l->data;
		SendCloseWindow(win);
	}
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static gboolean NOINLINE WriteProfile(WINDOW_DATA *win)
{
	gboolean ret;
	Profile *inifile;
	
	inifile = gl_profile.profile;
	
	ret = HypProfile_Save(TRUE);
	
	if (ret == FALSE)
	{
		char *msg = g_strdup_printf(_("Can't write Settings:\n%s\n%s\nQuit anyway?"), Profile_GetFilename(inifile), hyp_utf8_strerror(errno));
		ret = ask_yesno(win, msg);
		g_free(msg);
	}
	return ret;
}
#endif

/*** ---------------------------------------------------------------------- ***/

gboolean hv_scroll_window(WINDOW_DATA *win, long xamount, long yamount)
{
	WP_UNIT old_x, old_y;
	
	old_x = win->docsize.x;
	old_y = win->docsize.y;
	
	win->docsize.y += yamount;
	if (win->docsize.y > (win->docsize.h - win->scroll.g_h))
		win->docsize.y = (win->docsize.h - win->scroll.g_h);
	if (win->docsize.y < 0)
		win->docsize.y = 0;
	win->docsize.y = (win->docsize.y / win->y_raster) * win->y_raster;
	
	win->docsize.x += xamount;
	if (win->docsize.x > (win->docsize.w - win->scroll.g_w))
		win->docsize.x = (win->docsize.w - win->scroll.g_w);
	if (win->docsize.x < 0)
		win->docsize.x = 0;
	win->docsize.x = (win->docsize.x / win->x_raster) * win->x_raster;
	
	if (win->docsize.x == old_x && win->docsize.y == old_y)
		return FALSE;
	SetWindowSlider(win);
	SendRedraw(win);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

#if 0
static gboolean key_press_event(WINDOW_DATA *win, unsigned int message, WPARAM wParam, LPARAM lParam)
{
	unsigned int keycode;
	unsigned int keystate;
	gboolean handled = FALSE;
	DOCUMENT *doc = hypwin_doc(win);
	
	keystate = getkeystate();
	switch (message)
	{
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
		keycode = MapVirtualKey((lParam >> 16) & 0xff, MAPVK_VSC_TO_VK_EX);
		if (IsModifierKey(keycode))
			return FALSE;
		
		if (win->is_popup)
		{
			macos_destroy_window(win);
			return TRUE;
		}
		if (win->popup)
		{
			macos_destroy_window(win->popup);
			return TRUE;
		}
		handled = TRUE;
		switch (keycode)
		{
		case VK_RETURN:
			/* NYI: find position of selected link */
			break;
		case VK_LEFT:
			if ((keystate & K_SHIFT) && (keystate & K_CTRL))
				GoThisButton(win, TO_PREV);
			else if (keystate & K_SHIFT)
				hv_scroll_window(win, -win->scroll.g_w, 0);
			else if (keystate & K_CTRL)
				GoThisButton(win, TO_PREV);
			else
				hv_scroll_window(win, -win->x_raster, 0);
			break;
		case VK_RIGHT:
			if ((keystate & K_SHIFT) && (keystate & K_CTRL))
				GoThisButton(win, TO_NEXT);
			else if (keystate & K_SHIFT)
				hv_scroll_window(win, win->scroll.g_w, 0);
			else if (keystate & K_CTRL)
				GoThisButton(win, TO_NEXT);
			else
				hv_scroll_window(win, win->x_raster, 0);
			break;
		case VK_UP:
			if ((keystate & K_SHIFT) && (keystate & K_CTRL))
				GoThisButton(win, TO_PREV);
			else if (keystate & K_SHIFT)
				hv_scroll_window(win, 0, -win->scroll.g_h);
			else if (keystate & K_CTRL)
				hv_scroll_window(win, 0, -win->scroll.g_h);
			else
				hv_scroll_window(win, 0, -win->y_raster);
			break;
		case VK_DOWN:
			if ((keystate & K_SHIFT) && (keystate & K_CTRL))
				GoThisButton(win, TO_NEXT);
			else if (keystate & K_SHIFT)
				hv_scroll_window(win, 0, win->scroll.g_h);
			else if (keystate & K_CTRL)
				hv_scroll_window(win, 0, win->scroll.g_h);
			else
				hv_scroll_window(win, 0, win->y_raster);
			break;
		case VK_PRIOR:
			hv_scroll_window(win, 0, -win->scroll.g_h);
			break;
		case VK_NEXT:
			hv_scroll_window(win, 0, win->scroll.g_h);
			break;
		case VK_HOME:
			if ((keystate & K_SHIFT) || (keystate & K_CTRL))
				hv_scroll_window(win, -INT_MAX, -INT_MAX);
			else
				hv_scroll_window(win, -INT_MAX, 0);
			break;
		case VK_END:
			if ((keystate & K_SHIFT) || (keystate & K_CTRL))
				hv_scroll_window(win, -INT_MAX, INT_MAX);
			else
				hv_scroll_window(win, INT_MAX, 0);
			break;
		case VK_SUBTRACT:
			GoThisButton(win, TO_PREV);
			break;
		case VK_ADD:
			GoThisButton(win, TO_NEXT);
			break;
		case VK_DIVIDE:
			GoThisButton(win, TO_PREV_PHYS);
			break;
		case VK_MULTIPLY:
			GoThisButton(win, TO_NEXT_PHYS);
			break;
		case VK_HELP:
			GotoHelp(win);
			break;
		case VK_ESCAPE:
		case VK_BACK:
			if (!win->searchentry || !(doc->buttons.searchbox))
				GoThisButton(win, TO_BACK);
			else
				handled = FALSE;
			break;
		case VK_F1:				/* already handled by accelerators */
		case VK_F2:				/* already handled by accelerators */
		case VK_F3:				/* already handled by accelerators */
		case VK_F4:				/* already handled by accelerators */
		case VK_F5:				/* already handled by accelerators */
		case VK_F6:				/* already handled by accelerators */
		case VK_F7:				/* already handled by accelerators */
		case VK_F8:				/* already handled by accelerators */
		case VK_F9:				/* already handled by accelerators */
		case VK_F10:			/* already handled by accelerators */
		case VK_F11:			/* already handled by accelerators */
		case VK_F12:			/* already handled by accelerators */
		default:
			handled = FALSE;
			break;
		}
		break;
	}
	if (!handled && win->searchentry)
		handled = AutolocatorKey(win, message, wParam, lParam);
	return handled;
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static void set_cursor_if_appropriate(WINDOW_DATA *win, int x, int y)
{
	LINK_INFO info;
	gboolean hovering = FALSE;
	
	if (HypFindLink(win, x, y, &info, FALSE))
	{
		if (info.tip)
		{
			set_tooltip_text(win->textview, info.tip);
			g_free(info.tip);
		}
		hovering = TRUE;
	}
	if (hovering != win->hovering_over_link)
	{
		win->hovering_over_link = hovering;
		if (hovering)
		{
			[[NSCursor pointingHandCursor] set];
		} else
		{
			[[NSCursor arrowCursor] set];
			set_tooltip_text(win->textview, NULL);
		}
	}
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static gboolean follow_if_link(WINDOW_DATA *win, int x, int y)
{
	LINK_INFO info;

	if (HypFindLink(win, x, y, &info, !gl_profile.viewer.refonly))
	{
		g_freep(&info.tip);
		HypClick(win, &info);
		return TRUE;
	}
	return FALSE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static gboolean on_button_release(WINDOW_DATA *win, int x, int y, int button)
{
	if (win->is_popup)
	{
		macos_destroy_window(win);
		return TRUE;
	}
	
	if (button == 1)
	{
		CheckFiledate(win);
		
		/* we shouldn't follow a link if the user has selected something */
		if (win->selection.valid)
			return FALSE;
	
		return follow_if_link(win, x, y);
	}
	return FALSE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static gboolean on_button_press(WINDOW_DATA *win, int x, int y, int button)
{
	if (win->popup)
	{
		macos_destroy_window(win->popup);
		return TRUE;
	}
	
	RemoveSearchBox(win);
	
	if (button == 1)
	{
		CheckFiledate(win);
		MouseSelection(win, x, y, /* (getkeystate() & K_SHIFT) != */ 0);
	} else if (button == 2)
	{
		if (gl_profile.viewer.rightback)
		{
			GoThisButton(win, TO_BACK);
		} else
		{
			/* TODO: right click: context popup */
		}
	}
	return FALSE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static LRESULT CALLBACK textWndProc(WINDOW_DATA *win, UINT message, WPARAM wParam, LPARAM lParam)
{
	WINDOW_DATA *win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(win, GWLP_USERDATA);
	
	switch (message)
	{
	case WM_NCCREATE:
		win = (WINDOW_DATA *)(((CREATESTRUCT *)lParam)->lpCreateParams);
		SetWindowLongPtr(win, GWLP_USERDATA, (DWORD_PTR)win);
		win->textview = win;
		break;

	case WM_DESTROY:
		win->textview = NULL;
		return FALSE;
	
	case WM_ERASEBKGND:
		{
			RECT r;
			HBRUSH brush = CreateSolidBrush(viewer_colors.background);
			GetClientRect(win, &r);
			FillRect((HDC)wParam, &r, brush);
			DeleteObject(brush);
		}
		return TRUE;
	
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			GRECT gr;
			DOCUMENT *doc;
			
			doc = hypwin_doc(win);
			win->draw_hdc = W_BeginPaint(win, &ps, &gr);
			if (win->cliprgn == NULL)
				win->cliprgn = CreateRectRgn(win->scroll.g_x, win->scroll.g_y, win->scroll.g_x + win->scroll.g_w, win->scroll.g_y + win->scroll.g_h);
			else
				SetRectRgn(win->cliprgn, win->scroll.g_x, win->scroll.g_y, win->scroll.g_x + win->scroll.g_w, win->scroll.g_y + win->scroll.g_h);
			SelectClipRgn(win->draw_hdc, win->cliprgn);
			doc->displayProc(win);
			DrawSelection(win);
			W_EndPaint(win, &ps);
			win->draw_hdc = NULL;
		}
		return TRUE;

	case WM_MOUSEWHEEL:
		{
			int amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

			UpdateWindow(win->textview);

			hv_scroll_window(win, 0, -amount * win->y_raster);
		}
		return 0;
	
	case WM_MOUSEHWHEEL:
		{
			int amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

			UpdateWindow(win->textview);

			hv_scroll_window(win, -amount * win->x_raster, 0);
		}
		return 0;
	
	case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			set_cursor_if_appropriate(win, x, y);
		}
		break;
	
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			int button = message == WM_LBUTTONDOWN ? 1 : message == WM_RBUTTONDOWN ? 2 : 3;
			on_button_press(win, x, y, button);
		}
		break;
	
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			int button = message == WM_LBUTTONUP ? 1 : message == WM_RBUTTONUP ? 2 : 3;
			on_button_release(win, x, y, button);
		}
		break;
	}
	
	return DefWindowProc(win, message, wParam, lParam);
}
#endif

/*** ---------------------------------------------------------------------- ***/

DOCUMENT *hypwin_doc(WINDOW_DATA *win)
{
	return win->data;
}

/*** ---------------------------------------------------------------------- ***/

void hv_set_font(WINDOW_DATA *win)
{
	const char *name = gl_profile.viewer.use_xfont ? gl_profile.viewer.xfont_name : gl_profile.viewer.font_name;
	(void) name; /* TODO */
		/* to avoid divisions by zero */
		win->x_raster = HYP_PIC_FONTW;
		win->y_raster = HYP_PIC_FONTH;
}

/*------------------------------------------------------------------*/

static int x_translate_coordinates(WINDOW_DATA *win, int *dest_x_ret, int *dest_y_ret)
{
	NSView *nsv = [win contentView];
	NSPoint wpos;
	NSRect r;
	
	wpos.x = 0;
	wpos.y = 0;
	r = [nsv bounds];
	wpos = r.origin;
	wpos = [win convertBaseToScreen:wpos];
	
	*dest_x_ret = wpos.x;
	*dest_y_ret = GetScreenHeight() - wpos.y - r.size.height;
	
	return TRUE;
}

/*------------------------------------------------------------------*/

static void get_window_size(WINDOW_DATA *win)
{
	NSRect frame;

	frame = [win frame];
	frame = [win contentRectForFrameRect: frame];
	gl_profile.viewer.win_x = frame.size.width;
	gl_profile.viewer.win_y = frame.size.height;
	gl_profile.viewer.win_w = frame.origin.x;
	gl_profile.viewer.win_h = frame.origin.y;
	x_translate_coordinates(win, &gl_profile.viewer.win_x, &gl_profile.viewer.win_y);
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *macos_hypview_window_new(DOCUMENT *doc, gboolean popup)
{
	WINDOW_DATA *win;
	NSRect sizehints;
	int x, y, w, h;
	
	x = gl_profile.viewer.win_x;
	y = gl_profile.viewer.win_y;
	w = gl_profile.viewer.win_w;
	h = gl_profile.viewer.win_h;
	if (default_geometry && !popup)
	{
		gtk_XParseGeometry(default_geometry, &x, &y, &w, &h);
		hv_win_set_geometry(NULL);
	}
	sizehints.origin.x = x;
	sizehints.origin.y = y;
	sizehints.size.width = w;
	sizehints.size.height = h;

	win = [[HypViewWindow alloc] initWithContentRect: sizehints];
	if (win == NULL)
		return NULL;

	win->data = doc;
	win->is_popup = popup;
	win->title = g_strdup(doc->path);
	
	if (popup)
	{
	} else
	{
	}

	win->y_margin_top = gl_profile.viewer.text_yoffset;
	win->y_margin_bottom = gl_profile.viewer.text_yoffset;
	win->x_margin_left = gl_profile.viewer.text_xoffset;
	win->x_margin_right = gl_profile.viewer.text_xoffset;
	
	hv_set_font(win);
	hv_set_title(win, win->title);

	WindowCalcScroll(win);
	SetWindowSlider(win);
	
	if (!popup)
	{
		all_list = g_slist_prepend(all_list, win);
	}
	
	[win setInitialFirstResponder: win->textview];
	
	[NSApp activateIgnoringOtherApps:YES];

	[win makeKeyAndOrderFront:nil];
	[win makeMainWindow];
	get_window_size(win);
		
	return win;
}

/*** ---------------------------------------------------------------------- ***/

long hv_win_topline(WINDOW_DATA *win)
{
	return win->docsize.y / win->y_raster;
}

/*** ---------------------------------------------------------------------- ***/

void hv_set_title(WINDOW_DATA *win, const char *title)
{
	if (!empty(title))
	{
		[win setTitle: [NSString stringWithCString:title encoding:NSUTF8StringEncoding]];
		[NSApp changeWindowsItem: win title: [win title] filename:NO];
	} else
	{
		[win setTitle: @""];
		[NSApp removeWindowsItem: win];
	}
}

/*** ---------------------------------------------------------------------- ***/

void SendRedraw(WINDOW_DATA *win)
{
	[win->textview setNeedsDisplayInRect:[win->textview bounds]];
}

/*** ---------------------------------------------------------------------- ***/

void SendCloseWindow(WINDOW_DATA *win)
{
	if (win)
		[win close];
}

/*** ---------------------------------------------------------------------- ***/

/*
 * scroll window such that <line> is displayed at the top
 */
void hv_win_scroll_to_line(WINDOW_DATA *win, long line)
{
	hv_scroll_window(win, 0, line * win->y_raster - win->docsize.y);
}

/*** ---------------------------------------------------------------------- ***/

void SetWindowSlider(WINDOW_DATA *win)
{
	WP_UNIT pos, range;
	
	pos = win->docsize.x;
	range = win->docsize.w - 1;
	if (range < 0)
		range = 0;
	if (pos != win->scrollhpos || range != win->scrollhsize)
	{
		/* TODO */
	}
	win->scrollhpos = pos;
	win->scrollhsize = range;

	pos = win->docsize.y;
	range = win->docsize.h - 1;
	if (range < 0)
		range = 0;
	if (pos != win->scrollvpos || range != win->scrollvsize)
	{
		/* TODO */
	}
	win->scrollvpos = pos;
	win->scrollvsize = range;
}

/*** ---------------------------------------------------------------------- ***/

void ReInitWindow(WINDOW_DATA *win, gboolean prep)
{
	DOCUMENT *doc = hypwin_doc(win);
	int visible_lines;
	
	win->hovering_over_link = FALSE;
	[[NSCursor arrowCursor] set];
	hv_set_font(win);
	if (prep)
		doc->prepNode(win, win->displayed_node);
	hv_set_title(win, win->title);
	win->selection.valid = FALSE;
	
	/* adjust window size to new dimensions */
	if (gl_profile.viewer.adjust_winsize)
	{
	}

	visible_lines = (win->scroll.g_h + win->y_raster - 1) / win->y_raster;

	win->docsize.y = min(win->docsize.h - visible_lines * win->y_raster, doc->start_line * win->y_raster);
	win->docsize.y = max(0, win->docsize.y);
	win->docsize.x = 0;

	SetWindowSlider(win);
	ToolbarUpdate(win, TRUE);
	SendRedraw(win);
}

/********************************************************************/
/*------------------------------------------------------------------*/
/********************************************************************/

static void cocoa_click(WINDOW_DATA *win, int button, int x, int y, int click, NSUInteger state)
{
	/* TODO */
	UNUSED(win);
	UNUSED(button);
	UNUSED(x);
	UNUSED(y);
	UNUSED(click);
	UNUSED(state);
}

/*------------------------------------------------------------------*/

static void cocoa_motion(WINDOW_DATA *win, int x, int y)
{
	UNUSED(win);
	UNUSED(x);
	UNUSED(y);
	/* nothing todo */
}

/*------------------------------------------------------------------*/

static BOOL cocoa_key(WINDOW_DATA *win, NSEvent *theEvent)
{
	NSUInteger count;
	unsigned short keysym;
	int ascii_code, ascii_code_unmodded;
	unsigned int key;
	int shift, ctrl, alt;
	NSUInteger state;
	NSString *characters;
	NSString *charactersUnmodded;
	
	UNUSED(win);
	ascii_code = 0;
	ascii_code_unmodded = 0;
	key = 0;
	state = [theEvent modifierFlags];
	shift = (state & NSShiftKeyMask) != 0;
	ctrl = (state & NSControlKeyMask) != 0;
	alt = (state & (NSCommandKeyMask | NSAlternateKeyMask)) != 0;
	characters = [theEvent characters];
	charactersUnmodded = [theEvent charactersIgnoringModifiers];
	count = [characters length];
	if (count > 0)
		ascii_code = [characters characterAtIndex:0] & 0xff;
	count = [charactersUnmodded length];
	if (count > 0)
		ascii_code_unmodded = [charactersUnmodded characterAtIndex:0] & 0xff;
	keysym = [theEvent keyCode];
	/* printf("ascii=%02x unmodded=%02x alt=%d ctrl=%d keysym=%x\n", ascii_code, ascii_code_unmodded, alt, ctrl, keysym); */
	
	/*
	 * you might think that keyCode return a virtual keyCode value;
	 * you are wrong. WTF.
	 */
	if (state & (NSFunctionKeyMask | NSHelpKeyMask))
	{
		ascii_code = ascii_code_unmodded = 0;
		switch (keysym)
		{
			case 0x7e: keysym = NSUpArrowFunctionKey; break;
			case 0x7d: keysym = NSDownArrowFunctionKey; break;
			case 0x7b: keysym = NSLeftArrowFunctionKey; break;
			case 0x7c: keysym = NSRightArrowFunctionKey; break;
			case 0x75: keysym = NSDeleteFunctionKey; break;
			case 0x72: keysym = NSInsertFunctionKey; break;
			case 0x77: keysym = NSEndFunctionKey; break;
			case 0x73: keysym = NSHomeFunctionKey; break;
			case 0x79: keysym = NSPageDownFunctionKey; break;
			case 0x74: keysym = NSPageUpFunctionKey; break;
			case 0x69: keysym = NSPrintFunctionKey; break;
			case 0x7a: keysym = NSF1FunctionKey; break;
			case 0x78: keysym = NSF2FunctionKey; break;
			case 0x63: keysym = NSF3FunctionKey; break;
			case 0x76: keysym = NSF4FunctionKey; break;
			case 0x60: keysym = NSF5FunctionKey; break;
			case 0x61: keysym = NSF6FunctionKey; break;
			case 0x62: keysym = NSF7FunctionKey; break;
			case 0x64: keysym = NSF8FunctionKey; break;
			case 0x65: keysym = NSF9FunctionKey; break;
			case 0x6d: keysym = NSF10FunctionKey; break;
			case 0x67: keysym = NSF11FunctionKey; break;
	 		case 0x6f: keysym = NSF12FunctionKey; break;
			case 0x47: keysym = NSHelpFunctionKey; break; /* NumLock */
		}
	}
	
	if (state & NSNumericPadKeyMask)
	{
		switch (keysym)
		{
			case 0x4c: ascii_code = ascii_code_unmodded = 0x0d; break;
		}
	}
	
	if (alt)
	{
		/* those should be handled by shortcuts in menus etc. */
		return FALSE;
	}

	if (ctrl)
	{
		switch (ascii_code_unmodded)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case 'A':
			case 'a':
			case 'B':
			case 'b':
			case 'C':
			case 'c':
			case 'D':
			case 'd':
			case 'E':
			case 'e':
			case 'F':
			case 'f':
			case 'G':
			case 'g':
			case 'H':
			case 'h':
			case 'I':
			case 'i':
			case 'J':
			case 'j':
			case 'K':
			case 'k':
			case 'L':
			case 'l':
			case 'M':
			case 'm':
			case 'N':
			case 'n':
			case 'O':
			case 'o':
			case 'P':
			case 'p':
			case 'Q':
			case 'q':
			case 'R':
			case 'r':
			case 'S':
			case 's':
			case 'T':
			case 't':
			case 'U':
			case 'u':
			case 'V':
			case 'v':
			case 'W':
			case 'w':
			case 'X':
			case 'x':
			case 'Y':
			case 'y':
			case 'Z':
			case 'z':
			case ' ':
				return FALSE;
		}
	}

	if (shift && ascii_code == 0)
	{
		switch (ascii_code_unmodded)
		{
			case '0': key = '0'; break;
			case '1': key = '1'; break;
			case '2': key = '2'; break;
			case '3': key = '3'; break;
			case '4': key = '4'; break;
			case '5': key = '5'; break;
			case '6': key = '6'; break;
			case '7': key = '7'; break;
			case '8': key = '8'; break;
			case '9': key = '9'; break;
			case 'A': key = 'A'; break;
			case 'B': key = 'B'; break;
			case 'C': key = 'C'; break;
			case 'D': key = 'D'; break;
			case 'E': key = 'E'; break;
			case 'F': key = 'F'; break;
			case 'G': key = 'G'; break;
			case 'H': key = 'H'; break;
			case 'I': key = 'I'; break;
			case 'J': key = 'J'; break;
			case 'K': key = 'K'; break;
			case 'L': key = 'L'; break;
			case 'M': key = 'M'; break;
			case 'N': key = 'N'; break;
			case 'O': key = 'O'; break;
			case 'P': key = 'P'; break;
			case 'Q': key = 'Q'; break;
			case 'R': key = 'R'; break;
			case 'S': key = 'S'; break;
			case 'T': key = 'T'; break;
			case 'U': key = 'U'; break;
			case 'V': key = 'V'; break;
			case 'W': key = 'W'; break;
			case 'X': key = 'X'; break;
			case 'Y': key = 'Y'; break;
			case 'Z': key = 'Z'; break;
		}
		if (key != 0)
		{
			return FALSE;
		}
	}

	switch (keysym)
	{
	case NSLeftArrowFunctionKey:
		if (shift && ctrl)
			GoThisButton(win, TO_PREV);
		else if (shift)
			hv_scroll_window(win, -win->scroll.g_w, 0);
		else if (ctrl)
			GoThisButton(win, TO_PREV);
		else
			hv_scroll_window(win, -win->x_raster, 0);
		break;
	case NSRightArrowFunctionKey:
		if (shift && ctrl)
			GoThisButton(win, TO_NEXT);
		else if (shift)
			hv_scroll_window(win, win->scroll.g_w, 0);
		else if (ctrl)
			GoThisButton(win, TO_NEXT);
		else
			hv_scroll_window(win, win->x_raster, 0);
		break;
	case NSUpArrowFunctionKey:
		if (shift && ctrl)
			GoThisButton(win, TO_PREV);
		else if (shift)
			hv_scroll_window(win, 0, -win->scroll.g_h);
		else if (ctrl)
			hv_scroll_window(win, 0, -win->scroll.g_h);
		else
			hv_scroll_window(win, 0, -win->y_raster);
		break;
	case NSDownArrowFunctionKey:
		if (shift && ctrl)
			GoThisButton(win, TO_NEXT);
		else if (shift)
			hv_scroll_window(win, 0, win->scroll.g_h);
		else if (ctrl)
			hv_scroll_window(win, 0, win->scroll.g_h);
		else
			hv_scroll_window(win, 0, win->y_raster);
		break;
	case NSPrevFunctionKey:
	case NSPageUpFunctionKey:
		hv_scroll_window(win, 0, -win->scroll.g_h);
		break;
	case NSNextFunctionKey:
	case NSPageDownFunctionKey:
		hv_scroll_window(win, 0, win->scroll.g_h);
		break;
	case NSHomeFunctionKey:
	case NSBeginFunctionKey:
		if (shift || ctrl)
			hv_scroll_window(win, -INT_MAX, -INT_MAX);
		else
			hv_scroll_window(win, -INT_MAX, 0);
		break;
	case NSEndFunctionKey:
		if (shift || ctrl)
			hv_scroll_window(win, -INT_MAX, INT_MAX);
		else
			hv_scroll_window(win, INT_MAX, 0);
		break;
	case NSInsertFunctionKey:
	case NSInsertCharFunctionKey:
	case NSDeleteFunctionKey:
	case NSDeleteCharFunctionKey:
		return FALSE;
	case NSUndoFunctionKey:
	/* mapped to insert-char, since there is no help function key on a normal keyboard,
	 * but the insert key generates 'help' there
	 */
	case NSHelpFunctionKey:
		return FALSE;
	case NSPrintScreenFunctionKey:
	case NSScrollLockFunctionKey:
	case NSPauseFunctionKey:
	case NSSysReqFunctionKey:
	case NSBreakFunctionKey:
	case NSResetFunctionKey:
	case NSStopFunctionKey:
	case NSMenuFunctionKey:
	case NSUserFunctionKey:
	case NSSystemFunctionKey:
	case NSPrintFunctionKey:
	case NSClearLineFunctionKey:
	case NSClearDisplayFunctionKey:
	case NSInsertLineFunctionKey:
	case NSSelectFunctionKey:
	case NSExecuteFunctionKey:
	case NSRedoFunctionKey:
	case NSFindFunctionKey:
	case NSModeSwitchFunctionKey:
		return FALSE;

	case 0x33:
		key = 0x08;
		return FALSE; /* backspace */
	case 0x18:
		key = shift ? 0x60 : 0xb4;
		shift = 0;
		break;
	}
	
	/* if (ascii_code == 0 && keysym < 256)
		ascii_code = keysym;
	*/
	
	if (ascii_code == 0)
	{
		switch (keysym)
		{		
			case NSF1FunctionKey:
			case NSF2FunctionKey:
			case NSF3FunctionKey:
			case NSF4FunctionKey:
			case NSF5FunctionKey:
			case NSF6FunctionKey:
			case NSF7FunctionKey:
			case NSF8FunctionKey:
			case NSF9FunctionKey:
			case NSF10FunctionKey:
			case NSF11FunctionKey:
			case NSF12FunctionKey:
			case NSF13FunctionKey:
			case NSF14FunctionKey:
			case NSF15FunctionKey:
			case NSF16FunctionKey:
			case NSF17FunctionKey:
			case NSF18FunctionKey:
			case NSF19FunctionKey:
			case NSF20FunctionKey:
			case NSPrintFunctionKey: return FALSE;
			case 0x1e: key = '~'; break;
		}
	}
	
	if (key == 0 && ascii_code != 0)
		key = ascii_code;
	if (key != 0)
	{
		return TRUE;
	}
	return TRUE;
}

/*------------------------------------------------------------------*/

@implementation HypViewView

- (id) initWithFrame:(NSRect)frame
{
	if ((self = [super initWithFrame:frame]) == NULL)
		return 0;
	[self setAutoresizingMask: (NSViewHeightSizable | NSViewWidthSizable)];
	[self allocateGState];
	
	return self;
}


- (void)dealloc
{
	[self releaseGState];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSViewGlobalFrameDidChangeNotification object:self];
	[super dealloc];
}


- (void)finalize
{
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSViewGlobalFrameDidChangeNotification object:self];
	[super finalize];
}


- (void)setFrame:(NSRect)frame
{
	[super setFrame:frame];
}


- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect: dirtyRect];
}


- (void)mouseMoved:(NSEvent *)theEvent
{
	WINDOW_DATA *win = (WINDOW_DATA *)[theEvent window];

	[super mouseMoved:theEvent];
	if (!win)
		return;
	{
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		cocoa_motion(win, (int)local_point.x, (int)local_point.y - 1);
	}
}


- (void)mouseDragged:(NSEvent *)theEvent
{
	WINDOW_DATA *win = (WINDOW_DATA *)[theEvent window];

	dprintf(("NSView::mouseDragged"));
	[super mouseDragged:theEvent];
	if (!win)
		return;
	{
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		cocoa_motion(win, (int)local_point.x, (int)local_point.y - 1);
	}
}


- (void)mouseDown:(NSEvent *)theEvent
{
	WINDOW_DATA *win = (WINDOW_DATA *)[theEvent window];

	dprintf(("NSView::mouseDown"));
	[super mouseDown:theEvent];
	if (!win)
		return;
	{
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		cocoa_click(win, 1, (int)local_point.x, (int)local_point.y - 1, TRUE, [theEvent modifierFlags]);
	}
}

- (void)mouseUp:(NSEvent *)theEvent
{
	WINDOW_DATA *win = (WINDOW_DATA *)[theEvent window];

	dprintf(("NSView::mouseUp"));
	[super mouseUp:theEvent];
	if (!win)
		return;
	{
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		cocoa_click(win, 1, (int)local_point.x, (int)local_point.y - 1, FALSE, [theEvent modifierFlags]);
	}
}


- (void)rightMouseDragged:(NSEvent *)theEvent
{
	WINDOW_DATA *win = (WINDOW_DATA *)[theEvent window];

	dprintf(("NSView::rightMouseDragged"));
	[super rightMouseDragged:theEvent];
	if (!win)
		return;
	{
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		cocoa_motion(win, (int)local_point.x, (int)local_point.y - 1);
	}
}


- (void)rightMouseDown:(NSEvent *)theEvent
{
	WINDOW_DATA *win = (WINDOW_DATA *)[theEvent window];

	dprintf(("NSView::rightMouseDown"));
	[super rightMouseDown:theEvent];
	if (!win)
		return;
	{
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		cocoa_click(win, 2, (int)local_point.x, (int)local_point.y - 1, TRUE, [theEvent modifierFlags]);
	}
}


- (void)rightMouseUp:(NSEvent *)theEvent
{
	WINDOW_DATA *win = (WINDOW_DATA *)[theEvent window];

	dprintf(("NSView::rightMouseUp"));
	[super rightMouseUp:theEvent];
	if (!win)
		return;
	{
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		cocoa_click(win, 2, (int)local_point.x, (int)local_point.y - 1, FALSE, [theEvent modifierFlags]);
	}
}


- (void)otherMouseDragged:(NSEvent *)theEvent
{
	WINDOW_DATA *win = (WINDOW_DATA *)[theEvent window];

	dprintf(("NSView::otherMouseDragged"));
	[super otherMouseDragged:theEvent];
	if (!win)
		return;
	{
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		cocoa_motion(win, (int)local_point.x, (int)local_point.y - 1);
	}
}


- (void)otherMouseDown:(NSEvent *)theEvent
{
	WINDOW_DATA *win = (WINDOW_DATA *)[theEvent window];

	dprintf(("NSView::otherMouseDown"));
	[super otherMouseDown:theEvent];
	if (!win)
		return;
	{
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		cocoa_click(win, (int)[theEvent buttonNumber], (int)local_point.x, (int)local_point.y - 1, TRUE, [theEvent modifierFlags]);
	}
}


- (void)otherMouseUp:(NSEvent *)theEvent
{
	WINDOW_DATA *win = (WINDOW_DATA *)[theEvent window];

	dprintf(("NSView::otherMouseUp"));
	[super otherMouseUp:theEvent];
	if (!win)
		return;
	{
		NSPoint event_location = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:event_location fromView:nil];
		cocoa_click(win, (int)[theEvent buttonNumber], (int)local_point.x, (int)local_point.y - 1, FALSE, [theEvent modifierFlags]);
	}
}


- (void)keyDown:(NSEvent *)theEvent
{
	WINDOW_DATA *win = (WINDOW_DATA *)[theEvent window];

	dprintf(("NSView::keyDown"));
	if (!win)
		return;
	if (!cocoa_key(win, theEvent))
		[super keyDown:theEvent];
}


- (void)keyUp:(NSEvent *)theEvent
{
	UNUSED(theEvent);
	dprintf(("NSView::keyUp"));
	/* [super keyUp:theEvent]; */
}


- (BOOL)canBecomeKeyView
{
	dprintf(("NSView::canBecomeKeyView"));
	return YES;
}


- (BOOL)canDraw
{
	dprintf(("NSView::canDraw"));
	return YES;
}


- (BOOL)acceptsFirstMouse: (NSEvent *)theEvent
{
	UNUSED(theEvent);
	dprintf(("NSView::acceptsFirstMouse"));
	return YES;
}


- (BOOL)becomeFirstResponder
{
	dprintf(("NSView<Responder>::becomeFirstResponder"));
	return YES;
}


@end

/********************************************************************/
/*------------------------------------------------------------------*/
/********************************************************************/

static int get_context(WINDOW_DATA *win)
{
	return TRUE;
}

static NSColor *nscolor_from_pixel(Pixel rgb)
{
	float a = ((rgb >> 24) & 0xFF) / 255.0;
	float r = ((rgb >> 16) & 0xFF) / 255.0;
	float g = ((rgb >>  8) & 0xFF) / 255.0;
	float b = ((rgb      ) & 0xFF) / 255.0;
	NSColor *fg = [NSColor colorWithDeviceRed:r green:g blue:b alpha:a];
	return fg;
}

@implementation HypViewWindow

- (id)initWithContentRect:(NSRect)contentRect
{
	HypViewView *_view;
	NSUInteger windowStyle = NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask;
	NSBackingStoreType bufferingType = NSBackingStoreBuffered;
	NSSize resize;
	int i;

	_view = [[HypViewView alloc] initWithFrame: contentRect];
	if (_view == nil)
		return nil;
	if ((self = [super initWithContentRect: contentRect styleMask: windowStyle backing: bufferingType defer: NO]) == nil)
	{
		[_view release];
		return nil;
	}
	[_view retain];
	self->textview = _view;
	self->x_raster = HYP_PIC_FONTW;
	self->y_raster = HYP_PIC_FONTH;
	self->data = NULL;
	self->is_popup = FALSE;
	self->title = NULL;
	self->popup = NULL;
	self->history = NULL;
	self->displayed_node = NULL;
	for (i = 0; i < TO_MAX; i++)
	{
		self->m_button_disabled[i] = FALSE;
		self->m_button_hidden[i] = FALSE;
	}
	self->parentwin = NULL;

	[self setReleasedWhenClosed: NO];
	get_context(self);
	[self setContentView: _view];
	[self retain];
	[self setDelegate: self];
	hv_set_font(self);
	resize.width = self->x_raster;
	resize.height = self->y_raster;
	[self setContentResizeIncrements: resize];
	
	[self setIgnoresMouseEvents: NO];
#if USEMOUSE
	[self setAcceptsMouseMovedEvents: YES];
#else
	[self setAcceptsMouseMovedEvents: NO];
#endif
	/* [self setAlphaValue: 0.9]; */

	[self makeFirstResponder: _view];

	[self setBackgroundColor: nscolor_from_pixel(viewer_colors.background)];
	
	return self;
}


- (BOOL)windowShouldClose:(id)sender
{
	WINDOW_DATA *win = sender;
	
	dprintf(("NSWindowDelegate::windowShouldClose: %s", [[sender description] UTF8String]));
	if (!win)
		return TRUE;
	return TRUE;
}


- (void)windowDidMove:(NSNotification *)notification
{
	WINDOW_DATA *win = [notification object];
	
	dprintf(("NSWindowDelegate::windowDidMove"));
	if (!win)
		return;
	get_window_size(win);
}


- (void)windowDidResize:(NSNotification *)notification
{
	WINDOW_DATA *win = [notification object];
	static int been_here = 0;
	
	dprintf(("NSWindowDelegate::windowDidResize"));
	if (!win)
		return;
	if (been_here++ == 0)
	{
		get_window_size(win);
		/* cocoa_resize(win); */
		--been_here;
	}
}


- (void)becomeKeyWindow
{
	dprintf(("NSWindow::becomeKeyWindow"));
	[HypViewApp updateAppMenu];
	[super becomeKeyWindow];
}


- (void)resignKeyWindow
{
	dprintf(("NSWindow::resignKeyWindow"));
	[super resignKeyWindow];
}


- (void)windowDidBecomeKey:(NSNotification *)notification
{
	dprintf(("NSWindowDelegate::windowDidBecomeKey"));
}


- (void)windowDidResignKey:(NSNotification *)notification
{
	dprintf(("NSWindowDelegate::windowDidResignKey"));
}


-(BOOL)canBecomeKeyWindow
{
	dprintf(("NSWindow::canBecomeKeyWindow"));
	return YES;
}


- (void)becomeMainWindow
{
	dprintf(("NSWindow::becomeMainWindow"));
	[super becomeMainWindow];
}


- (void)resignMainWindow
{
	dprintf(("NSWindow::resignMainWindow"));
	[super resignMainWindow];
}


-(BOOL)canBecomeMainWindow
{
	dprintf(("NSWindow::canBecomeMainWindow"));
	return YES;
}


- (void)windowDidBecomeMain:(NSNotification *)notification
{
	UNUSED(notification);
	dprintf(("NSWindowDelegate::windowDidBecomeMain"));
}


- (void)windowDidResignMain:(NSNotification *)notification
{
	UNUSED(notification);
	dprintf(("NSWindowDelegate::windowDidResignMain"));
}


- (void)windowDidChangeScreen:(NSNotification *)notification
{
	WINDOW_DATA *win = [notification object];
	
	if (!win)
		return;
	dprintf(("NSWindowDelegate::windowDidChangeScreen"));
	get_window_size(win);
}


- (void)windowDidDeminiaturize:(NSNotification *)notification
{
	dprintf(("NSWindowDelegate::windowDidDeminiaturize"));
}


- (void)windowDidMiniaturize:(NSNotification *)notification
{
	dprintf(("NSWindowDelegate::windowDidMiniaturize"));
}


- (void)windowWillClose:(NSNotification *)notification
{
	WINDOW_DATA *win = notification ? [notification object] : NULL;
	
	if (!win)
		return;
	dprintf(("NSWindowDelegate::windowWillClose: %s", [[notification description] UTF8String]));
	macos_destroy_window(win);
}

@end
