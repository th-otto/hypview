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
			SwitchFont(win);
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

void hv_preferences(WINDOW_DATA *win)
{
	UNUSED(win);
	/* NYI: preferences */
}
