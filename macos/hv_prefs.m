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

#include "hv_defs.h"
#include "hypdebug.h"

#undef dprintf
#if 1
#define dprintf(x) hyp_debug x
#else
#define dprintf(x)
#endif

#define autorelease self

struct color_params {
	WINDOW_DATA *win;
	Pixel bg_brush;
	NSFont *font;
	struct _viewer_colors colors;
};

struct output_params {
	WINDOW_DATA *win;
	HYP_CHARSET charset;
	gboolean bracket_links;
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#if 0
static UINT_PTR CALLBACK select_color_hook(WINDOW_DATA *win, UINT message, WPARAM wparam, LPARAM lparam)
{
	UNUSED(wparam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			LPCHOOSECOLOR cc = (LPCHOOSECOLOR) lparam;
			const char *title = (const char *) cc->lCustData;
			wchar_t *wtitle;
			
			wtitle = hyp_utf8_to_wchar(_(title), STR0TERM, NULL);
			SetWindowTextW(win, wtitle);
			g_free(wtitle);
		}
		break;
	}
	return FALSE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0
static void choose_color(WINDOW_DATA *parent, int textid, COLORREF *color)
{
	CHOOSECOLOR cl;
	
	commdlg_help = RegisterWindowMessageW(HELPMSGSTRINGW);
	
	memset(&cl, 0, sizeof(cl));
	cl.lStructSize = sizeof(cl);
	cl.hwndOwner = parent;
	cl.hInstance = 0;
	cl.rgbResult = *color;
	custcolors[0] = *color;
	cl.Flags = CC_ANYCOLOR | CC_RGBINIT | CC_SHOWHELP | CC_ENABLEHOOK | CC_FULLOPEN;
	cl.lpfnHook = select_color_hook;
	cl.lpCustColors = custcolors;
	cl.lCustData = (LPARAM) _("Pick a color");
	if (ChooseColor(&cl))
	{
		*color = cl.rgbResult;
		if (textid == IDC_COLOR_BG_TEXT)
		{
			struct color_params *params = (struct color_params *)(DWORD_PTR)GetWindowLongPtr(parent, GWLP_USERDATA);
			DeleteObject(params->bg_brush);
			params->bg_brush = CreateSolidBrush(*color);
			InvalidateRect(parent, NULL, TRUE);
		} else
		{
			InvalidateRect(GetDlgItem(parent, textid), NULL, TRUE);
		}
	}
}
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0 /* TODO */
static INT_PTR CALLBACK colors_dialog(WINDOW_DATA *win, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD notifyCode;
	WINDOW_DATA *win;
	struct color_params *params;
	
	params = (struct color_params *)(DWORD_PTR)GetWindowLongPtr(win, GWLP_USERDATA);
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_INITDIALOG:
		params = (struct color_params *)lParam;
		win = params->win;
		SetWindowLongPtr(win, GWLP_USERDATA, (DWORD_PTR)params);
		CenterWindow(win);
		params->font = W_FontCreate1(gl_profile.viewer.font_name);
		params->bg_brush = CreateSolidBrush(params->colors.background);
		DlgSetText(win, IDC_COLOR_BG_TEXT, _("Window background"));
		SendDlgItemMessage(win, IDC_COLOR_BG_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(win, IDC_COLOR_TEXT_TEXT, _("Normal text and line graphics"));
		SendDlgItemMessage(win, IDC_COLOR_TEXT_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(win, IDC_COLOR_GHOSTED_TEXT, _("Ghosted text (attribute @{G})"));
		SendDlgItemMessage(win, IDC_COLOR_GHOSTED_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(win, IDC_COLOR_LINK_TEXT, _("Internal nodes (@node)"));
		SendDlgItemMessage(win, IDC_COLOR_LINK_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(win, IDC_COLOR_POPUP_TEXT, _("Popup nodes (@pnode)"));
		SendDlgItemMessage(win, IDC_COLOR_POPUP_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(win, IDC_COLOR_XREF_TEXT, _("External references (@{.. link FILE [LINE]})"));
		SendDlgItemMessage(win, IDC_COLOR_XREF_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(win, IDC_COLOR_SYSTEM_TEXT, _("SYSTEM-argument (@{... system ARG})"));
		SendDlgItemMessage(win, IDC_COLOR_SYSTEM_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(win, IDC_COLOR_RXS_TEXT, _("REXX script (@{... rxs FILE})"));
		SendDlgItemMessage(win, IDC_COLOR_RXS_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(win, IDC_COLOR_RX_TEXT, _("REXX command (@{... rx ARG})"));
		SendDlgItemMessage(win, IDC_COLOR_RX_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(win, IDC_COLOR_QUIT_TEXT, _("QUIT entry (@{... quit})"));
		SendDlgItemMessage(win, IDC_COLOR_QUIT_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(win, IDC_COLOR_CLOSE_TEXT, _("CLOSE entry (@{... close})"));
		SendDlgItemMessage(win, IDC_COLOR_CLOSE_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetButton(win, IDC_LINK_BOLD, (gl_profile.colors.link_effect & HYP_TXT_BOLD) != 0);
		DlgSetButton(win, IDC_LINK_LIGHT, (gl_profile.colors.link_effect & HYP_TXT_LIGHT) != 0);
		DlgSetButton(win, IDC_LINK_ITALIC, (gl_profile.colors.link_effect & HYP_TXT_ITALIC) != 0);
		DlgSetButton(win, IDC_LINK_UNDERLINED, (gl_profile.colors.link_effect & HYP_TXT_UNDERLINED) != 0);
		DlgSetButton(win, IDC_LINK_OUTLINED, (gl_profile.colors.link_effect & HYP_TXT_OUTLINED) != 0);
		DlgSetButton(win, IDC_LINK_SHADOWED, (gl_profile.colors.link_effect & HYP_TXT_SHADOWED) != 0);
		return TRUE;
	case WM_CLOSE:
		EndDialog(win, IDCANCEL);
		DestroyWindow(win);
		return TRUE;
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
		win = params->win;
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(win, IDCANCEL);
			DestroyWindow(win);
			return TRUE;
		case IDOK:
			gl_profile.colors.link_effect = 0;
			gl_profile.colors.link_effect |= DlgGetButton(win, IDC_LINK_BOLD) ? HYP_TXT_BOLD : 0;
			gl_profile.colors.link_effect |= DlgGetButton(win, IDC_LINK_LIGHT) ? HYP_TXT_LIGHT : 0;
			gl_profile.colors.link_effect |= DlgGetButton(win, IDC_LINK_ITALIC) ? HYP_TXT_ITALIC : 0;
			gl_profile.colors.link_effect |= DlgGetButton(win, IDC_LINK_UNDERLINED) ? HYP_TXT_UNDERLINED : 0;
			gl_profile.colors.link_effect |= DlgGetButton(win, IDC_LINK_OUTLINED) ? HYP_TXT_OUTLINED : 0;
			gl_profile.colors.link_effect |= DlgGetButton(win, IDC_LINK_SHADOWED) ? HYP_TXT_SHADOWED : 0;
			viewer_colors = params->colors;
			EndDialog(win, IDOK);
			DestroyWindow(win);
			HypProfile_SetChanged();
			SwitchFont(win, FALSE);
			break;
		case IDC_COLOR_BG_BUTTON:
			choose_color(win, IDC_COLOR_BG_TEXT, &params->colors.background);
			break;
		case IDC_COLOR_TEXT_BUTTON:
			choose_color(win, IDC_COLOR_TEXT_TEXT, &params->colors.text);
			break;
		case IDC_COLOR_GHOSTED_BUTTON:
			choose_color(win, IDC_COLOR_GHOSTED_TEXT, &params->colors.ghosted);
			break;
		case IDC_COLOR_LINK_BUTTON:
			choose_color(win, IDC_COLOR_LINK_TEXT, &params->colors.link);
			break;
		case IDC_COLOR_POPUP_BUTTON:
			choose_color(win, IDC_COLOR_POPUP_TEXT, &params->colors.popup);
			break;
		case IDC_COLOR_XREF_BUTTON:
			choose_color(win, IDC_COLOR_XREF_TEXT, &params->colors.xref);
			break;
		case IDC_COLOR_SYSTEM_BUTTON:
			choose_color(win, IDC_COLOR_SYSTEM_TEXT, &params->colors.system);
			break;
		case IDC_COLOR_RX_BUTTON:
			choose_color(win, IDC_COLOR_RX_TEXT, &params->colors.rx);
			break;
		case IDC_COLOR_RXS_BUTTON:
			choose_color(win, IDC_COLOR_RXS_TEXT, &params->colors.rxs);
			break;
		case IDC_COLOR_QUIT_BUTTON:
			choose_color(win, IDC_COLOR_QUIT_TEXT, &params->colors.quit);
			break;
		case IDC_COLOR_CLOSE_BUTTON:
			choose_color(win, IDC_COLOR_RX_TEXT, &params->colors.close);
			break;
		case IDHELP:
			if (notifyCode == BN_CLICKED)
			{
				Help_Show(win, _("Select Colors"));
			}
			break;
		}
		break;
	case WM_DESTROY:
		DeleteObject(params->bg_brush);
		DeleteObject(params->font);
		break;
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOREDIT:
		{
			HDC hdcStatic = (HDC) wParam;
			
			switch (GetDlgCtrlID(lParam))
			{
			case IDC_COLOR_BG_TEXT:
				SetTextColor(hdcStatic, params->colors.text);
				SetBkColor(hdcStatic, params->colors.background);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (INT_PTR)params->bg_brush;
			case IDC_COLOR_TEXT_TEXT:
				SetTextColor(hdcStatic, params->colors.text);
				SetBkColor(hdcStatic, params->colors.background);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (INT_PTR)params->bg_brush;
			case IDC_COLOR_GHOSTED_TEXT:
				SetTextColor(hdcStatic, params->colors.ghosted);
				SetBkColor(hdcStatic, params->colors.background);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (INT_PTR)params->bg_brush;
			case IDC_COLOR_LINK_TEXT:
				SetTextColor(hdcStatic, params->colors.link);
				SetBkColor(hdcStatic, params->colors.background);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (INT_PTR)params->bg_brush;
			case IDC_COLOR_POPUP_TEXT:
				SetTextColor(hdcStatic, params->colors.popup);
				SetBkColor(hdcStatic, params->colors.background);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (INT_PTR)params->bg_brush;
			case IDC_COLOR_XREF_TEXT:
				SetTextColor(hdcStatic, params->colors.xref);
				SetBkColor(hdcStatic, params->colors.background);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (INT_PTR)params->bg_brush;
			case IDC_COLOR_SYSTEM_TEXT:
				SetTextColor(hdcStatic, params->colors.system);
				SetBkColor(hdcStatic, params->colors.background);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (INT_PTR)params->bg_brush;
			case IDC_COLOR_RX_TEXT:
				SetTextColor(hdcStatic, params->colors.rx);
				SetBkColor(hdcStatic, params->colors.background);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (INT_PTR)params->bg_brush;
			case IDC_COLOR_RXS_TEXT:
				SetTextColor(hdcStatic, params->colors.rxs);
				SetBkColor(hdcStatic, params->colors.background);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (INT_PTR)params->bg_brush;
			case IDC_COLOR_QUIT_TEXT:
				SetTextColor(hdcStatic, params->colors.quit);
				SetBkColor(hdcStatic, params->colors.background);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (INT_PTR)params->bg_brush;
			case IDC_COLOR_CLOSE_TEXT:
				SetTextColor(hdcStatic, params->colors.close);
				SetBkColor(hdcStatic, params->colors.background);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (INT_PTR)params->bg_brush;
			}
		}
		break;
		
	default:
		hv_commdlg_help(win, message, wParam, lParam);
		break;
	}
	return FALSE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

void hv_config_colors(WINDOW_DATA *win)
{
	struct color_params params;
	
	params.win = win;
	params.colors = viewer_colors;
#if 0
	DialogBoxExW(NULL, MAKEINTRESOURCEW(IDD_COLORS), parent, colors_dialog, (LPARAM)&params);
#endif
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

@interface PrefsPanel : NSWindow <NSWindowDelegate>
{
@public
	WINDOW_DATA *parent;
@private
	char *hypfold;
	char *default_file;
	char *catalog_file;
	int startup;
	gboolean rightback;
	gboolean transparent_pics;
	gboolean check_time;
	gboolean alink_newwin;
	gboolean marken_save_ask;
	
	NSTabViewItem *general;
	NSButton *rightback_button;
	NSButton *transparent_pics_button;
	NSButton *check_time_button;
	NSButton *alink_newwin_button;
	NSButton *marken_save_ask_button;
	NSPopUpButton *startup_popup;
	NSPathControl *hypfold_selector;
	NSPathControl *default_file_selector;
	NSPathControl *catalog_file_selector;
}
- (void)close;
@end

@implementation PrefsPanel
- (void)close
{
	dprintf(("close: %s", [[self description] UTF8String]));
#if 0 
	[HypViewApp stopModal];
#endif
	[super close];
}

- (void)on_ok:(id)sender
{
	dprintf(("on_ok: %s", [[self description] UTF8String]));
	[self performClose:sender];
}

- (void)on_hypfold_select:(id)sender
{
	dprintf(("on_hypfold_select: %s", [[self description] UTF8String]));
}

static void _print_hierarchy(NSView *view, int indent)
{
	int i;
	FILE *fp = stdout;
	NSArray *subviews;
	const char *classname;
	
	for (i = 0; i < (indent * 4); i++)
		fputc(' ', fp);
	classname = [[view className] UTF8String];
	fprintf(fp, "%s", classname);
	if (strcmp(classname, "NSStackView") == 0)
	{
		NSStackView *stack = (NSStackView *)view;
		fprintf(fp, " %s", [stack orientation] ==  NSUserInterfaceLayoutOrientationVertical ? "vertical" : "horizontal");
		switch ([stack alignment])
		{
			case NSLayoutAttributeLeft: fprintf(fp, " left"); break;
			case NSLayoutAttributeRight: fprintf(fp, " right"); break;
			case NSLayoutAttributeTop: fprintf(fp, " top"); break;
			case NSLayoutAttributeBottom: fprintf(fp, " bottom"); break;
			case NSLayoutAttributeLeading: fprintf(fp, " leading"); break;
			case NSLayoutAttributeTrailing: fprintf(fp, " trailing"); break;
			case NSLayoutAttributeWidth: fprintf(fp, " width"); break;
			case NSLayoutAttributeHeight: fprintf(fp, " height"); break;
			case NSLayoutAttributeCenterX: fprintf(fp, " centerx"); break;
			case NSLayoutAttributeCenterY: fprintf(fp, " centery"); break;
			case NSLayoutAttributeNotAnAttribute: fprintf(fp, " none"); break;
			default: fprintf(fp, " <???>"); break;
		}
	}
	fputc('\n', fp);
	subviews = [view subviews];
	if (subviews)
	{
		for (i = 0; i < [subviews count]; i++)
			_print_hierarchy([subviews objectAtIndex:i], indent + 1);
	}
}

void print_hierarchy(NSView *view)
{
	_print_hierarchy(view, 0);
}

- (id)init
{
	NSUInteger windowStyle = NSTitledWindowMask | NSClosableWindowMask;
	NSBackingStoreType bufferingType = NSBackingStoreBuffered;
	NSTabView *tabview;
	NSStackView *vbox, *hbox, *vbox2;
	NSStackView *general_vbox;
	NSTextField *label;
	NSTabViewController *controller;
	NSButton *button;
	
	if ((self = [super initWithContentRect:NSMakeRect(0, 0, 200, 300) styleMask: windowStyle backing: bufferingType defer: NO]) == nil)
	{
		return nil;
	}
	dprintf(("init: %s", [[self description] UTF8String]));
	[self setTitle: W_("Preferences")];
	[self cascadeTopLeftFromPoint:NSMakePoint(20, 20)];

	self->hypfold = path_subst(gl_profile.general.hypfold);
	self->default_file = path_subst(gl_profile.viewer.default_file);
	self->catalog_file = path_subst(gl_profile.viewer.catalog_file);
	self->startup = gl_profile.viewer.startup;
	self->rightback = gl_profile.viewer.rightback;
	self->transparent_pics = gl_profile.viewer.transparent_pics;
	self->check_time = gl_profile.viewer.check_time;
	self->alink_newwin = gl_profile.viewer.alink_newwin;
	self->marken_save_ask = gl_profile.viewer.marken_save_ask;

	vbox = [[[NSStackView alloc] init] autorelease];
	[vbox setSpacing: 10];
	[vbox setOrientation: NSUserInterfaceLayoutOrientationVertical];
	[vbox setDistribution: NSStackViewDistributionGravityAreas];
	[vbox setAlignment: NSLayoutAttributeLeft];
	[vbox setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
	[vbox setEdgeInsets: NSEdgeInsetsMake(10, 10, 10, 10)];
	[self setContentView: vbox];

	hbox = [[[NSStackView alloc] init] autorelease];
	[hbox setAlignment: NSLayoutAttributeTop];
	tabview = [[[NSTabView alloc] init] autorelease];
	[tabview setDrawsBackground:YES];
	[tabview setTabViewType:NSTopTabsBezelBorder];
	[hbox addView:tabview inGravity: NSStackViewGravityLeading];
	[vbox addView:hbox inGravity: NSStackViewGravityLeading];

	controller = [[NSTabViewController alloc] init];
	[controller setTabStyle: NSTabViewControllerTabStyleSegmentedControlOnTop];
	
	self->general = [NSTabViewItem tabViewItemWithViewController:controller];
	[self->general setLabel:W_("General")];

	general_vbox = [[[NSStackView alloc] init] autorelease];
	[general_vbox setOrientation: NSUserInterfaceLayoutOrientationVertical];
	[general_vbox setAlignment: NSLayoutAttributeLeft];
	[general_vbox setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
	[general_vbox setDistribution: NSStackViewDistributionGravityAreas];
	[general_vbox setEdgeInsets: NSEdgeInsetsMake(10, 10, 10, 10)];
	[self->general setView: general_vbox];
	
	label = [NSTextField labelWithString: [[[NSString alloc] initWithUTF8String: _("Paths")] autorelease]];
	[label setAlignment: NSLeftTextAlignment];
	[label setFont:[NSFont boldSystemFontOfSize:[NSFont systemFontSize]]];
	[general_vbox addView:label inGravity: NSStackViewGravityLeading];

	hbox = [[[NSStackView alloc] init] autorelease];
	[hbox setSpacing: 10];
	[hbox setEdgeInsets: NSEdgeInsetsMake(10, 10, 10, 10)];
	vbox2 = [[[NSStackView alloc] init] autorelease];
	[vbox2 setOrientation: NSUserInterfaceLayoutOrientationVertical];
	[vbox2 setAlignment: NSLayoutAttributeLeft];
	[hbox addView:vbox2 inGravity: NSStackViewGravityLeading];
	[general_vbox addView:hbox inGravity: NSStackViewGravityLeading];

	self->hypfold_selector = [[[NSPathControl alloc] initWithFrame:NSMakeRect(0, 0, 300, 26)] autorelease];
	[self->hypfold_selector setEditable:NO];
	[self->hypfold_selector setPathStyle:NSPathStyleStandard];
	[self->hypfold_selector setURL:[NSURL fileURLWithPath:[[[NSString alloc] initWithUTF8String:self->hypfold] autorelease] isDirectory:YES]];
	[self->hypfold_selector setDoubleAction:@selector(on_hypfold_select:)];
	[vbox2 addView:self->hypfold_selector inGravity: NSStackViewGravityLeading];
	
	label = [NSTextField labelWithString: [[[NSString alloc] initWithUTF8String: _("At Programstart")] autorelease]];
	[label setAlignment: NSLeftTextAlignment];
	[label setFont:[NSFont boldSystemFontOfSize:[NSFont systemFontSize]]];
	[general_vbox addView:label inGravity: NSStackViewGravityLeading];

	hbox = [[[NSStackView alloc] init] autorelease];
	[hbox setSpacing: 10];
	[hbox setEdgeInsets: NSEdgeInsetsMake(10, 10, 10, 10)];
	vbox2 = [[[NSStackView alloc] init] autorelease];
	[vbox2 setOrientation: NSUserInterfaceLayoutOrientationVertical];
	[vbox2 setAlignment: NSLayoutAttributeLeft];
	[hbox addView:vbox2 inGravity: NSStackViewGravityLeading];
	[general_vbox addView:hbox inGravity: NSStackViewGravityLeading];

	self->startup_popup = [[[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 300, 26) pullsDown:NO] autorelease];
	[self->startup_popup addItemWithTitle:W_("Show File Selector")];
	[self->startup_popup addItemWithTitle:W_("Load default hypertext")];
	[self->startup_popup addItemWithTitle:W_("Load last file")];
	[self->startup_popup setObjectValue: [NSNumber numberWithInt:gl_profile.viewer.startup]];
	
	[vbox2 addView:self->startup_popup inGravity: NSStackViewGravityLeading];
	
	label = [NSTextField labelWithString: [[[NSString alloc] initWithUTF8String: _("Miscellaneous")] autorelease]];
	[label setAlignment: NSLeftTextAlignment];
	[label setFont:[NSFont boldSystemFontOfSize:[NSFont systemFontSize]]];
	[general_vbox addView:label inGravity: NSStackViewGravityLeading];

	hbox = [[[NSStackView alloc] init] autorelease];
	[hbox setSpacing: 10];
	[hbox setEdgeInsets: NSEdgeInsetsMake(10, 10, 10, 10)];
	vbox2 = [[[NSStackView alloc] init] autorelease];
	[vbox2 setOrientation: NSUserInterfaceLayoutOrientationVertical];
	[vbox2 setAlignment: NSLayoutAttributeLeft];
	[hbox addView:vbox2 inGravity: NSStackViewGravityLeading];
	[general_vbox addView:hbox inGravity: NSStackViewGravityLeading];

	self->rightback_button = [NSButton checkboxWithTitle:W_("Right mouse button is 'back'") target:self action:nil];
	[self->rightback_button setState:self->rightback ? NSOnState : NSOffState];
	[self->rightback_button setToolTip:W_("If set, a right mouse click is interpreted as a click on the Back icon")];
	[vbox2 addView:self->rightback_button inGravity: NSStackViewGravityLeading];
	self->transparent_pics_button = [NSButton checkboxWithTitle:W_("Display pictures transparent") target:self action:nil];
	[self->transparent_pics_button setState:self->transparent_pics ? NSOnState : NSOffState];
	[self->transparent_pics_button setToolTip:W_("Draw pictures transparently")];
	[vbox2 addView:self->transparent_pics_button inGravity: NSStackViewGravityLeading];
	self->check_time_button = [NSButton checkboxWithTitle:W_("Watch modification times of files") target:self action:nil];
	[self->check_time_button setState:self->check_time ? NSOnState : NSOffState];
	[self->check_time_button setToolTip:W_("Check file modification time and date before access")];
	[vbox2 addView:self->check_time_button inGravity: NSStackViewGravityLeading];
	self->alink_newwin_button = [NSButton checkboxWithTitle:W_("Open ALINKs in a new window") target:self action:nil];
	[self->alink_newwin_button setState:self->alink_newwin ? NSOnState : NSOffState];
	[self->alink_newwin_button setToolTip:W_("If set, ALINKS are opened in a new window\n"
		"If not set, ALINKS are opened in the current window (as ST-Guide)")];
	[vbox2 addView:self->alink_newwin_button inGravity: NSStackViewGravityLeading];
	self->marken_save_ask_button = [NSButton checkboxWithTitle:W_("Ask before saving bookmarks") target:self action:nil];
	[self->marken_save_ask_button setState:self->marken_save_ask ? NSOnState : NSOffState];
	[self->marken_save_ask_button setToolTip:W_("Ask before saving bookmarks")];
	[vbox2 addView:self->marken_save_ask_button inGravity: NSStackViewGravityLeading];

	[general_vbox setNeedsLayout:YES];
	[general_vbox layout];
	[tabview addTabViewItem:self->general];
	
	/* [controller addTabViewItem:self->general]; */
	
	hbox = [[[NSStackView alloc] init] autorelease];
	[hbox setSpacing: 10];
	[hbox setAlignment: NSLayoutAttributeCenterY];
	[hbox setDistribution: NSStackViewDistributionGravityAreas];
	[hbox setEdgeInsets: NSEdgeInsetsMake(10, 0, 0, 10)];
	[vbox addView:hbox inGravity: NSStackViewGravityTrailing];
	
	button = [NSButton buttonWithTitle:W_("OK") target:self action:@selector(on_ok:)];
	[button setAutoresizingMask:NSViewMinXMargin|NSViewMaxYMargin];
	[button resignFirstResponder];
	[button setKeyEquivalent:@"\015"];
	[button setHighlighted:YES];
	[self makeFirstResponder:button];
	[hbox addView:button inGravity: NSStackViewGravityTrailing];

	button = [NSButton buttonWithTitle:W_("Close") target:self action:@selector(performClose:)];
	[button setAutoresizingMask:NSViewMinXMargin|NSViewMaxYMargin];
	[button setKeyEquivalent:@"\033"];
	[hbox addView:button inGravity: NSStackViewGravityTrailing];
	[button setHighlighted:NO];

	print_hierarchy([self contentView]);

	return self;
}

- (void)dealloc
{
	dprintf(("dealloc: %s", [[self description] UTF8String]));
	g_free(self->hypfold);
	g_free(self->default_file);
	g_free(self->catalog_file);
	[super dealloc];
}

@end

#if 0 /* TODO */
static INT_PTR CALLBACK preference_dialog(WINDOW_DATA *win, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD notifyCode;
	WINDOW_DATA *win;
	struct pref_params *params;
	
	params = (struct pref_params *)(DWORD_PTR)GetWindowLongPtr(win, GWLP_USERDATA);
	switch (message)
	{
	case WM_INITDIALOG:
		DlgSetText(win, IDC_HYPFOLD, hyp_basename(params->hypfold));
		DlgSetText(win, IDC_DEFAULT_FILE, hyp_basename(params->default_file));
		DlgSetText(win, IDC_CATALOG_FILE, hyp_basename(params->catalog_file));
		DlgSetButton(win, IDC_PREF_FILE_SELECTOR, params->startup == 0);
		DlgSetButton(win, IDC_PREF_DEFAULT_TEXT, params->startup == 1);
		DlgSetButton(win, IDC_PREF_LAST_FILE, params->startup == 2);
		return TRUE;
	case WM_CLOSE:
		EndDialog(win, IDCANCEL);
		DestroyWindow(win);
		return TRUE;
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
		win = params->win;
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(win, IDCANCEL);
			DestroyWindow(win);
			return TRUE;
		case IDOK:
			if (DlgGetButton(win, IDC_PREF_FILE_SELECTOR)) gl_profile.viewer.startup = 0;
			if (DlgGetButton(win, IDC_PREF_DEFAULT_TEXT)) gl_profile.viewer.startup = 1;
			if (DlgGetButton(win, IDC_PREF_LAST_FILE)) gl_profile.viewer.startup = 2;
			gl_profile.viewer.rightback = DlgGetButton(win, IDC_PREF_RIGHTBACK);
			gl_profile.viewer.transparent_pics = DlgGetButton(win, IDC_PREF_TRANSPARENT);
			gl_profile.viewer.check_time = DlgGetButton(win, IDC_PREF_CHECK_TIME);
			gl_profile.viewer.alink_newwin = DlgGetButton(win, IDC_PREF_ALINK_NEWWIN);
			gl_profile.viewer.marken_save_ask = DlgGetButton(win, IDC_PREF_MARKEN_SAVE_ASK);
			g_free(gl_profile.general.hypfold);
			gl_profile.general.hypfold = path_unsubst(params->hypfold, FALSE);
			g_free(gl_profile.viewer.default_file);
			gl_profile.viewer.default_file = path_unsubst(params->default_file, TRUE);
			g_free(gl_profile.viewer.catalog_file);
			gl_profile.viewer.catalog_file = path_unsubst(params->catalog_file, TRUE);
			EndDialog(win, IDOK);
			DestroyWindow(win);
			HypProfile_SetChanged();
			SwitchFont(win, FALSE);
			break;
		case IDC_HYPFOLD:
			if (choose_file(win, &params->hypfold, file_dirsel, _("Path for Hypertexts"), NULL))
			{
				DlgSetText(win, IDC_HYPFOLD, hyp_basename(params->hypfold));
			}
			break;
		case IDC_DEFAULT_FILE:
			if (choose_file(win, &params->default_file, file_open, _("Default-Hypertext"), _(hypertext_file_filter)))
			{
				DlgSetText(win, IDC_DEFAULT_FILE, hyp_basename(params->default_file));
			}
			break;
		case IDC_CATALOG_FILE:
			if (choose_file(win, &params->catalog_file, file_open, _("Catalog file"), _(hypertext_file_filter)))
			{
				DlgSetText(win, IDC_CATALOG_FILE, hyp_basename(params->catalog_file));
			}
			break;
		case IDHELP:
			if (notifyCode == BN_CLICKED)
			{
				Help_Show(win, _("Preferences"));
			}
			break;
		}
		break;
	}
	return FALSE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

void hv_preferences(WINDOW_DATA *sender)
{
	PrefsPanel *panel;
	
	panel = [[[PrefsPanel alloc] init] autorelease];
	panel->parent = sender;
	[panel makeKeyAndOrderFront: sender];
	
#if 0
	NSModalSession session = [HypViewApp beginModalSessionForWindow: panel];
	for (;;)
	{
		if ([HypViewApp runModalSession:session] != NSModalResponseContinue)
			break;
	}
	[HypViewApp endModalSession:session];
#endif
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#if 0 /* TODO */
static struct {
	const char *name;
	HYP_CHARSET charset;
} const charset_choices[] = {
	{ N_("System Default"), HYP_CHARSET_NONE },
	{ N_("Windows CP1252"), HYP_CHARSET_CP1252 },
	{ N_("Atari ST"), HYP_CHARSET_ATARI },
	{ N_("Mac-Roman"), HYP_CHARSET_MACROMAN },
	{ N_("OS/2 CP850"), HYP_CHARSET_CP850 },
	{ N_("UTF-8"), HYP_CHARSET_UTF8 }
};

static INT_PTR CALLBACK output_dialog(WINDOW_DATA *win, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD notifyCode;
	WINDOW_DATA *win;
	struct output_params *params;
	int i;
	int val;
	
	params = (struct output_params *)(DWORD_PTR)GetWindowLongPtr(win, GWLP_USERDATA);
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_INITDIALOG:
		params = (struct output_params *)lParam;
		win = params->win;
		SetWindowLongPtr(win, GWLP_USERDATA, (DWORD_PTR)params);
		CenterWindow(win);
		SendDlgItemMessage(win, IDC_OUTPUT_CHARSET, CB_RESETCONTENT, 0, 0);
		val = 0;
		for (i = 0; i < (int)(sizeof(charset_choices) / sizeof(charset_choices[0])); i++)
		{
			SendDlgItemMessage(win, IDC_OUTPUT_CHARSET, CB_ADDSTRING, 0, (LPARAM)_(charset_choices[i].name));
			if (params->charset == charset_choices[i].charset)
				val = i;
		}
		SendDlgItemMessage(win, IDC_OUTPUT_CHARSET, CB_SETCURSEL, val, 0);
		DlgSetButton(win, IDC_OUTPUT_BRACKET_LINKS, params->bracket_links);
		return TRUE;
	case WM_CLOSE:
		EndDialog(win, IDCANCEL);
		DestroyWindow(win);
		return TRUE;
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
		win = params->win;
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(win, IDCANCEL);
			DestroyWindow(win);
			return TRUE;
		case IDOK:
			val = (int) SendDlgItemMessage(win, IDC_OUTPUT_CHARSET, CB_GETCURSEL, 0, 0);
			if (val >= 0 && val < (int)(sizeof(charset_choices) / sizeof(charset_choices[0])))
				params->charset = charset_choices[val].charset;
			params->bracket_links = DlgGetButton(win, IDC_OUTPUT_BRACKET_LINKS);
			EndDialog(win, IDOK);
			DestroyWindow(win);
			gl_profile.output.output_charset = params->charset;
			gl_profile.output.bracket_links = params->bracket_links;
			HypProfile_SetChanged();
			break;
		case IDHELP:
			if (notifyCode == BN_CLICKED)
			{
				Help_Show(win, _("Output Settings"));
			}
			break;
		}
		break;
	case WM_DESTROY:
		break;
	default:
		hv_commdlg_help(win, message, wParam, lParam);
		break;
	}
	return FALSE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

void hv_config_output(WINDOW_DATA *win)
{
	struct output_params params;
	
	params.win = win;
	params.charset = gl_profile.output.output_charset;
	params.bracket_links =  gl_profile.output.bracket_links;
#if 0 /* TODO */
	DialogBoxExW(NULL, MAKEINTRESOURCEW(IDD_OUTPUT), parent, output_dialog, (LPARAM)&params);
#endif
}
