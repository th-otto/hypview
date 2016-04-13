#include "hv_defs.h"
#include "hypdebug.h"
#include "resource.rh"

struct color_params {
	WINDOW_DATA *win;
	HBRUSH bg_brush;
	HFONT font;
	struct _viewer_colors colors;
};
static COLORREF custcolors[16];

struct pref_params {
	WINDOW_DATA *win;
	char *hypfold;
	char *default_file;
	char *catalog_file;
	int startup;
	gboolean rightback;
	gboolean transparent_pics;
	gboolean check_time;
	gboolean alink_newwin;
	gboolean marken_save_ask;
};

struct output_params {
	WINDOW_DATA *win;
	HYP_CHARSET charset;
	gboolean bracket_links;
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static UINT_PTR CALLBACK select_color_hook(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
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
			SetWindowTextW(hwnd, wtitle);
			g_free(wtitle);
		}
		break;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static void choose_color(HWND parent, int textid, COLORREF *color)
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

/*** ---------------------------------------------------------------------- ***/

static INT_PTR CALLBACK colors_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD notifyCode;
	WINDOW_DATA *win;
	struct color_params *params;
	
	params = (struct color_params *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_INITDIALOG:
		params = (struct color_params *)lParam;
		win = params->win;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (DWORD_PTR)params);
		CenterWindow(hwnd);
		params->font = W_FontCreate1(gl_profile.viewer.font_name);
		params->bg_brush = CreateSolidBrush(params->colors.background);
		DlgSetText(hwnd, IDC_COLOR_BG_TEXT, _("Window background"));
		SendDlgItemMessage(hwnd, IDC_COLOR_BG_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(hwnd, IDC_COLOR_TEXT_TEXT, _("Normal text and line graphics"));
		SendDlgItemMessage(hwnd, IDC_COLOR_TEXT_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(hwnd, IDC_COLOR_GHOSTED_TEXT, _("Ghosted text (attribute @{G})"));
		SendDlgItemMessage(hwnd, IDC_COLOR_GHOSTED_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(hwnd, IDC_COLOR_LINK_TEXT, _("Internal nodes (@node)"));
		SendDlgItemMessage(hwnd, IDC_COLOR_LINK_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(hwnd, IDC_COLOR_POPUP_TEXT, _("Popup nodes (@pnode)"));
		SendDlgItemMessage(hwnd, IDC_COLOR_POPUP_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(hwnd, IDC_COLOR_XREF_TEXT, _("External references (@{.. link FILE [LINE]})"));
		SendDlgItemMessage(hwnd, IDC_COLOR_XREF_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(hwnd, IDC_COLOR_SYSTEM_TEXT, _("SYSTEM-argument (@{... system ARG})"));
		SendDlgItemMessage(hwnd, IDC_COLOR_SYSTEM_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(hwnd, IDC_COLOR_RXS_TEXT, _("REXX script (@{... rxs FILE})"));
		SendDlgItemMessage(hwnd, IDC_COLOR_RXS_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(hwnd, IDC_COLOR_RX_TEXT, _("REXX command (@{... rx ARG})"));
		SendDlgItemMessage(hwnd, IDC_COLOR_RX_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(hwnd, IDC_COLOR_QUIT_TEXT, _("QUIT entry (@{... quit})"));
		SendDlgItemMessage(hwnd, IDC_COLOR_QUIT_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetText(hwnd, IDC_COLOR_CLOSE_TEXT, _("CLOSE entry (@{... close})"));
		SendDlgItemMessage(hwnd, IDC_COLOR_CLOSE_TEXT, WM_SETFONT, (WPARAM)params->font, FALSE);
		DlgSetButton(hwnd, IDC_LINK_BOLD, (gl_profile.colors.link_effect & HYP_TXT_BOLD) != 0);
		DlgSetButton(hwnd, IDC_LINK_LIGHT, (gl_profile.colors.link_effect & HYP_TXT_LIGHT) != 0);
		DlgSetButton(hwnd, IDC_LINK_ITALIC, (gl_profile.colors.link_effect & HYP_TXT_ITALIC) != 0);
		DlgSetButton(hwnd, IDC_LINK_UNDERLINED, (gl_profile.colors.link_effect & HYP_TXT_UNDERLINED) != 0);
		DlgSetButton(hwnd, IDC_LINK_OUTLINED, (gl_profile.colors.link_effect & HYP_TXT_OUTLINED) != 0);
		DlgSetButton(hwnd, IDC_LINK_SHADOWED, (gl_profile.colors.link_effect & HYP_TXT_SHADOWED) != 0);
		return TRUE;
	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		DestroyWindow(hwnd);
		return TRUE;
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
		win = params->win;
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			DestroyWindow(hwnd);
			return TRUE;
		case IDOK:
			gl_profile.colors.link_effect = 0;
			gl_profile.colors.link_effect |= DlgGetButton(hwnd, IDC_LINK_BOLD) ? HYP_TXT_BOLD : 0;
			gl_profile.colors.link_effect |= DlgGetButton(hwnd, IDC_LINK_LIGHT) ? HYP_TXT_LIGHT : 0;
			gl_profile.colors.link_effect |= DlgGetButton(hwnd, IDC_LINK_ITALIC) ? HYP_TXT_ITALIC : 0;
			gl_profile.colors.link_effect |= DlgGetButton(hwnd, IDC_LINK_UNDERLINED) ? HYP_TXT_UNDERLINED : 0;
			gl_profile.colors.link_effect |= DlgGetButton(hwnd, IDC_LINK_OUTLINED) ? HYP_TXT_OUTLINED : 0;
			gl_profile.colors.link_effect |= DlgGetButton(hwnd, IDC_LINK_SHADOWED) ? HYP_TXT_SHADOWED : 0;
			viewer_colors = params->colors;
			EndDialog(hwnd, IDOK);
			DestroyWindow(hwnd);
			HypProfile_SetChanged();
			SwitchFont(win, FALSE);
			break;
		case IDC_COLOR_BG_BUTTON:
			choose_color(hwnd, IDC_COLOR_BG_TEXT, &params->colors.background);
			break;
		case IDC_COLOR_TEXT_BUTTON:
			choose_color(hwnd, IDC_COLOR_TEXT_TEXT, &params->colors.text);
			break;
		case IDC_COLOR_GHOSTED_BUTTON:
			choose_color(hwnd, IDC_COLOR_GHOSTED_TEXT, &params->colors.ghosted);
			break;
		case IDC_COLOR_LINK_BUTTON:
			choose_color(hwnd, IDC_COLOR_LINK_TEXT, &params->colors.link);
			break;
		case IDC_COLOR_POPUP_BUTTON:
			choose_color(hwnd, IDC_COLOR_POPUP_TEXT, &params->colors.popup);
			break;
		case IDC_COLOR_XREF_BUTTON:
			choose_color(hwnd, IDC_COLOR_XREF_TEXT, &params->colors.xref);
			break;
		case IDC_COLOR_SYSTEM_BUTTON:
			choose_color(hwnd, IDC_COLOR_SYSTEM_TEXT, &params->colors.system);
			break;
		case IDC_COLOR_RX_BUTTON:
			choose_color(hwnd, IDC_COLOR_RX_TEXT, &params->colors.rx);
			break;
		case IDC_COLOR_RXS_BUTTON:
			choose_color(hwnd, IDC_COLOR_RXS_TEXT, &params->colors.rxs);
			break;
		case IDC_COLOR_QUIT_BUTTON:
			choose_color(hwnd, IDC_COLOR_QUIT_TEXT, &params->colors.quit);
			break;
		case IDC_COLOR_CLOSE_BUTTON:
			choose_color(hwnd, IDC_COLOR_RX_TEXT, &params->colors.close);
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
			
			switch (GetDlgCtrlID((HWND)lParam))
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
		hv_commdlg_help(hwnd, message, wParam, lParam);
		break;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

void hv_config_colors(WINDOW_DATA *win)
{
	HWND parent = win ? win->hwnd : 0;
	struct color_params params;
	
	params.win = win;
	params.colors = viewer_colors;
	DialogBoxExW(NULL, MAKEINTRESOURCEW(IDD_COLORS), parent, colors_dialog, (LPARAM)&params);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static INT_PTR CALLBACK preference_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD notifyCode;
	WINDOW_DATA *win;
	struct pref_params *params;
	
	params = (struct pref_params *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_INITDIALOG:
		params = (struct pref_params *)lParam;
		win = params->win;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (DWORD_PTR)params);
		CenterWindow(hwnd);
		DlgSetText(hwnd, IDC_HYPFOLD, hyp_basename(params->hypfold));
		DlgSetText(hwnd, IDC_DEFAULT_FILE, hyp_basename(params->default_file));
		DlgSetText(hwnd, IDC_CATALOG_FILE, hyp_basename(params->catalog_file));
		DlgSetButton(hwnd, IDC_PREF_FILE_SELECTOR, params->startup == 0);
		DlgSetButton(hwnd, IDC_PREF_DEFAULT_TEXT, params->startup == 1);
		DlgSetButton(hwnd, IDC_PREF_LAST_FILE, params->startup == 2);
		DlgSetButton(hwnd, IDC_PREF_RIGHTBACK, params->rightback);
		DlgSetButton(hwnd, IDC_PREF_TRANSPARENT, params->transparent_pics);
		DlgSetButton(hwnd, IDC_PREF_CHECK_TIME, params->check_time);
		DlgSetButton(hwnd, IDC_PREF_ALINK_NEWWIN, params->alink_newwin);
		DlgSetButton(hwnd, IDC_PREF_MARKEN_SAVE_ASK, params->marken_save_ask);
		return TRUE;
	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		DestroyWindow(hwnd);
		return TRUE;
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
		win = params->win;
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			DestroyWindow(hwnd);
			return TRUE;
		case IDOK:
			if (DlgGetButton(hwnd, IDC_PREF_FILE_SELECTOR)) gl_profile.viewer.startup = 0;
			if (DlgGetButton(hwnd, IDC_PREF_DEFAULT_TEXT)) gl_profile.viewer.startup = 1;
			if (DlgGetButton(hwnd, IDC_PREF_LAST_FILE)) gl_profile.viewer.startup = 2;
			gl_profile.viewer.rightback = DlgGetButton(hwnd, IDC_PREF_RIGHTBACK);
			gl_profile.viewer.transparent_pics = DlgGetButton(hwnd, IDC_PREF_TRANSPARENT);
			gl_profile.viewer.check_time = DlgGetButton(hwnd, IDC_PREF_CHECK_TIME);
			gl_profile.viewer.alink_newwin = DlgGetButton(hwnd, IDC_PREF_ALINK_NEWWIN);
			gl_profile.viewer.marken_save_ask = DlgGetButton(hwnd, IDC_PREF_MARKEN_SAVE_ASK);
			g_free(gl_profile.general.hypfold);
			gl_profile.general.hypfold = path_unsubst(params->hypfold, FALSE);
			g_free(gl_profile.viewer.default_file);
			gl_profile.viewer.default_file = path_unsubst(params->default_file, TRUE);
			g_free(gl_profile.viewer.catalog_file);
			gl_profile.viewer.catalog_file = path_unsubst(params->catalog_file, TRUE);
			EndDialog(hwnd, IDOK);
			DestroyWindow(hwnd);
			HypProfile_SetChanged();
			SwitchFont(win, FALSE);
			break;
		case IDC_HYPFOLD:
			if (choose_file(hwnd, &params->hypfold, file_dirsel, _("Path for Hypertexts"), NULL))
			{
				DlgSetText(hwnd, IDC_HYPFOLD, hyp_basename(params->hypfold));
			}
			break;
		case IDC_DEFAULT_FILE:
			if (choose_file(hwnd, &params->default_file, file_open, _("Default-Hypertext"), _(hypertext_file_filter)))
			{
				DlgSetText(hwnd, IDC_DEFAULT_FILE, hyp_basename(params->default_file));
			}
			break;
		case IDC_CATALOG_FILE:
			if (choose_file(hwnd, &params->catalog_file, file_open, _("Catalog file"), _(hypertext_file_filter)))
			{
				DlgSetText(hwnd, IDC_CATALOG_FILE, hyp_basename(params->catalog_file));
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
	case WM_DESTROY:
		g_free(params->hypfold);
		g_free(params->default_file);
		g_free(params->catalog_file);
		break;
	default:
		hv_commdlg_help(hwnd, message, wParam, lParam);
		break;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

void hv_preferences(WINDOW_DATA *win)
{
	struct pref_params params;
	HWND parent = win ? win->hwnd : 0;
	
	params.win = win;
	params.hypfold = path_subst(gl_profile.general.hypfold);
	params.default_file = path_subst(gl_profile.viewer.default_file);
	params.catalog_file = path_subst(gl_profile.viewer.catalog_file);
	params.startup = gl_profile.viewer.startup;
	params.rightback = gl_profile.viewer.rightback;
	params.transparent_pics = gl_profile.viewer.transparent_pics;
	params.check_time = gl_profile.viewer.check_time;
	params.alink_newwin = gl_profile.viewer.alink_newwin;
	params.marken_save_ask = gl_profile.viewer.marken_save_ask;
	DialogBoxExW(NULL, MAKEINTRESOURCEW(IDD_PREFERENCES), parent, preference_dialog, (LPARAM)&params);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

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

static INT_PTR CALLBACK output_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD notifyCode;
	WINDOW_DATA *win;
	struct output_params *params;
	int i;
	int val;
	
	params = (struct output_params *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_INITDIALOG:
		params = (struct output_params *)lParam;
		win = params->win;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (DWORD_PTR)params);
		CenterWindow(hwnd);
		SendDlgItemMessage(hwnd, IDC_OUTPUT_CHARSET, CB_RESETCONTENT, 0, 0);
		val = 0;
		for (i = 0; i < (int)(sizeof(charset_choices) / sizeof(charset_choices[0])); i++)
		{
			SendDlgItemMessage(hwnd, IDC_OUTPUT_CHARSET, CB_ADDSTRING, 0, (LPARAM)_(charset_choices[i].name));
			if (params->charset == charset_choices[i].charset)
				val = i;
		}
		SendDlgItemMessage(hwnd, IDC_OUTPUT_CHARSET, CB_SETCURSEL, val, 0);
		DlgSetButton(hwnd, IDC_OUTPUT_BRACKET_LINKS, params->bracket_links);
		return TRUE;
	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		DestroyWindow(hwnd);
		return TRUE;
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
		win = params->win;
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			DestroyWindow(hwnd);
			return TRUE;
		case IDOK:
			val = (int) SendDlgItemMessage(hwnd, IDC_OUTPUT_CHARSET, CB_GETCURSEL, 0, 0);
			if (val >= 0 && val < (int)(sizeof(charset_choices) / sizeof(charset_choices[0])))
				params->charset = charset_choices[val].charset;
			params->bracket_links = DlgGetButton(hwnd, IDC_OUTPUT_BRACKET_LINKS);
			EndDialog(hwnd, IDOK);
			DestroyWindow(hwnd);
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
		hv_commdlg_help(hwnd, message, wParam, lParam);
		break;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

void hv_config_output(WINDOW_DATA *win)
{
	struct output_params params;
	HWND parent = win ? win->hwnd : 0;
	
	params.win = win;
	params.charset = gl_profile.output.output_charset;
	params.bracket_links =  gl_profile.output.bracket_links;
	DialogBoxExW(NULL, MAKEINTRESOURCEW(IDD_OUTPUT), parent, output_dialog, (LPARAM)&params);
}
