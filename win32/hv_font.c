#include "hv_defs.h"
#include "hypdebug.h"
#include "w_draw.h"
#include "resource.rh"

UINT commdlg_help;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void hv_update_menu(HMENU menu, DOCUMENT *doc)
{
	CheckMenuObj(menu, IDM_OPT_ALTFONT, gl_profile.viewer.use_xfont);
	CheckMenuObj(menu, IDM_OPT_EXPANDSPACES, gl_profile.viewer.expand_spaces);
	
	EnableMenuObj(menu, IDM_NAV_BOOKMARKSMENU, doc->buttons.bookmarks);
	EnableMenuObj(menu, IDM_FILE_OPEN, doc->buttons.load);
	EnableMenuObj(menu, IDM_FILE_SAVE, doc->buttons.save);
	EnableMenuObj(menu, IDM_FILE_RECOMPILE, doc->buttons.save && doc->type == HYP_FT_HYP);
	EnableMenuObj(menu, IDM_FILE_REMARKER, doc->buttons.remarker);
	EnableMenuObj(menu, IDM_FILE_INFO, doc->buttons.info);
	EnableMenuObj(menu, IDM_NAV_BACK, doc->buttons.back);
	EnableMenuObj(menu, IDM_NAV_HISTORYMENU, doc->buttons.history);
	EnableMenuObj(menu, IDM_NAV_CLEARSTACK, doc->buttons.history);
	EnableMenuObj(menu, IDM_FILE_CATALOG, !empty(gl_profile.viewer.catalog_file));
	EnableMenuObj(menu, IDM_NAV_PREV, doc->buttons.previous);
	EnableMenuObj(menu, IDM_NAV_PREVPHYS, doc->buttons.prevphys);
	EnableMenuObj(menu, IDM_NAV_TOC, doc->buttons.home);
	EnableMenuObj(menu, IDM_NAV_NEXT, doc->buttons.next);
	EnableMenuObj(menu, IDM_NAV_NEXTPHYS, doc->buttons.nextphys);
	EnableMenuObj(menu, IDM_NAV_FIRST, doc->buttons.first);
	EnableMenuObj(menu, IDM_NAV_LAST, doc->buttons.last);
	EnableMenuObj(menu, IDM_NAV_INDEX, doc->buttons.index);
	EnableMenuObj(menu, IDM_NAV_HELP, doc->buttons.help);
}

/*** ---------------------------------------------------------------------- ***/

void hv_update_winmenu(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	HMENU menu = GetMenu(win->hwnd);
	
	if (!win->is_popup)
	{
		hv_update_menu(menu, doc);
	}
}

/*** ---------------------------------------------------------------------- ***/

void hv_update_menus(void)
{
	WINDOW_DATA *win;
	GSList *l;
	
	for (l = all_list; l; l = l->next)
	{
		win = (WINDOW_DATA *)l->data;
		hv_update_winmenu(win);
	}
}

/*** ---------------------------------------------------------------------- ***/

static void ApplyFont(gboolean clearcache)
{
	WINDOW_DATA *win;
	DOCUMENT *doc;
	GSList *l;
	
	hv_update_menus();
	/* adjust all open documents and windows */
	for (l = all_list; l; l = l->next)
	{
		win = (WINDOW_DATA *)l->data;
		hv_set_font(win);
		/* if (win->type == WIN_WINDOW) */
		{
			gboolean ret;
			long topline;
			hyp_nodenr node_num;
			
			doc = win->data;
			/* reload page or file */

			topline = hv_win_topline(win);
			node_num = doc->getNodeProc(win);
			if (clearcache && doc->type == HYP_FT_HYP)
			{
				HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
				RemovePictures(hyp, TRUE);
			}
			ret = doc->gotoNodeProc(win, NULL, node_num);
			
			if (ret)
			{
				doc->start_line = topline;

				ReInitWindow(win, TRUE);
			}
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void SwitchFont(WINDOW_DATA *win, gboolean clearcache)
{
	UNUSED(win);
	gl_profile.viewer.use_xfont = gl_profile.viewer.use_xfont && gl_profile.viewer.xfont_name != NULL;
	ApplyFont(clearcache);
	HypProfile_SetChanged();
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

char *W_Fontdesc(const FONT_ATTR *attr)
{
	char namebuf[FONT_NAME_LEN];
	char sizebuf[30];
	char attrbuf[FONT_NAME_LEN];
	
	strcpy(namebuf, attr->name);
	strncat(namebuf, ",", sizeof(namebuf));
	*attrbuf = '\0';
	if (attr->textstyle & HYP_TXT_BOLD)
		strcat(attrbuf, " Bold");
	if (attr->textstyle & HYP_TXT_ITALIC)
		strcat(attrbuf, " Italic");
	if (attr->textstyle & HYP_TXT_UNDERLINED)
		strcat(attrbuf, " Underline");
	if (attrbuf[0] != '\0')
		strncat(namebuf, attrbuf + 1, sizeof(namebuf));
	sprintf(sizebuf, " , %d", attr->size);
	strncat(namebuf, sizebuf, sizeof(namebuf));
	return g_strdup(namebuf);
}

/*** ---------------------------------------------------------------------- ***/

gboolean W_Fontname(const char *name, FONT_ATTR *attr)
{
	char namebuf[FONT_NAME_LEN];
	char *p;
	char *stylename;
	
#define font_attr(name, len, mask) \
	while (*stylename == ' ') \
		stylename++; \
	if (strncasecmp(stylename, name, len) == 0) \
	{ \
		attr->textstyle |= mask; \
		stylename += len; \
		while (*stylename == ' ') \
			stylename++; \
	}

	attr->size = 0;
	attr->textstyle = HYP_TXT_NORMAL;
	attr->name[0] = '\0';
	if (name == NULL)
		return FALSE;
	strncpy(namebuf, name, sizeof(namebuf));
	p = strchr(namebuf, ',');
	if (p != NULL)
	{
		*p++ = '\0';
		stylename = p;
		p = strchr(p, ',');
		if (p != NULL)
		{
			*p++ = '\0';
			attr->size = (int)strtol(p, NULL, 10);
			p = strchr(p, ',');
			if (p != NULL)
			{
				*p++ = '\0';
			}
		}
		font_attr("Bold", 4, HYP_TXT_BOLD);
		font_attr("Italic", 6, HYP_TXT_ITALIC);
		font_attr("Underline", 9, HYP_TXT_UNDERLINED);
	}
	strcpy(attr->name, namebuf);
	if (*attr->name == '\0')
		return FALSE;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void W_FontCreate(const char *name, HFONT *fonts)
{
	FONT_ATTR attr;
	LOGFONTA lf;
	int h = GetDeviceCaps(GetDC(HWND_DESKTOP), LOGPIXELSY);
	int i;
	
	memset(&lf, 0, sizeof(lf));
	lf.lfWeight = FW_NORMAL;
	lf.lfQuality = DRAFT_QUALITY;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfCharSet = DEFAULT_CHARSET;
	if (W_Fontname(name, &attr))
	{
		strncpy(lf.lfFaceName, attr.name, sizeof(lf.lfFaceName));
		lf.lfHeight = -((MulDiv(attr.size, h, 72) + 5) / 10);
		for (i = 0; i <= HYP_TXT_MASK; i++)
		{
			lf.lfWeight = i & HYP_TXT_BOLD ? FW_BOLD : FW_NORMAL;
			lf.lfItalic = i & HYP_TXT_ITALIC ? TRUE : FALSE;
			lf.lfUnderline = i & HYP_TXT_UNDERLINED ? TRUE : FALSE;
			fonts[i] = CreateFontIndirect(&lf);
		}
	} else
	{
		for (i = 0; i <= HYP_TXT_MASK; i++)
			fonts[i] = (HFONT)GetStockObject(DEVICE_DEFAULT_FONT);
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

HFONT W_FontCreate1(const char *name)
{
	FONT_ATTR attr;
	LOGFONTA lf;
	int h = GetDeviceCaps(GetDC(HWND_DESKTOP), LOGPIXELSY);
	HFONT font;
	
	memset(&lf, 0, sizeof(lf));
	lf.lfWeight = FW_NORMAL;
	lf.lfQuality = DRAFT_QUALITY;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfCharSet = DEFAULT_CHARSET;
	if (W_Fontname(name, &attr))
	{
		strncpy(lf.lfFaceName, attr.name, sizeof(lf.lfFaceName));
		lf.lfHeight = -((MulDiv(attr.size, h, 72) + 5) / 10);
		font = CreateFontIndirect(&lf);
	} else
	{
		font = (HFONT)GetStockObject(DEVICE_DEFAULT_FONT);
	}
	return font;
}

static UINT_PTR CALLBACK select_font_hook(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	UNUSED(wparam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			LPCHOOSEFONT cf = (LPCHOOSEFONT) lparam;
			const char *title = (const char *) cf->lCustData;
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

static gboolean Choose1Font(HWND parent, char **desc, const char *title)
{
	CHOOSEFONTA cf;
	HDC hdc;
	FONT_ATTR attr;
	LOGFONTA lf;
	int h;
	char *fontname;
	gboolean res = FALSE;
	
	memset(&cf, 0, sizeof(cf));
	memset(&lf, 0, sizeof(lf));
	hdc = GetDC(HWND_DESKTOP);
	h = GetDeviceCaps(hdc, LOGPIXELSY);
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = parent;
	cf.hDC = hdc;
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS | CF_SHOWHELP | CF_INITTOLOGFONTSTRUCT | CF_ENABLEHOOK | CF_EFFECTS;
	cf.nFontType = REGULAR_FONTTYPE;
	cf.lCustData = (LPARAM) title;
	cf.lpfnHook = select_font_hook;
	cf.rgbColors = viewer_colors.text;
	lf.lfWeight = FW_NORMAL;
	if (W_Fontname(*desc, &attr))
	{
		strncpy(lf.lfFaceName, attr.name, sizeof(lf.lfFaceName));
		lf.lfHeight = -((MulDiv(attr.size, h, 72) + 5) / 10);
		if (attr.textstyle & HYP_TXT_BOLD)
		{
			cf.nFontType |= BOLD_FONTTYPE;
			lf.lfWeight = FW_BOLD;
		}
		if (attr.textstyle & HYP_TXT_ITALIC)
		{
			cf.nFontType |= ITALIC_FONTTYPE;
			lf.lfItalic = TRUE;
		}
		if (attr.textstyle & HYP_TXT_UNDERLINED)
		{
			lf.lfUnderline = TRUE;
		}
		lf.lfCharSet = DEFAULT_CHARSET;
	}
	
	commdlg_help = RegisterWindowMessageW(HELPMSGSTRINGW);
	
	if (ChooseFont(&cf))
	{
		if (lf.lfHeight < 0)
			lf.lfHeight = -lf.lfHeight;
		attr.size = (int)MulDiv(lf.lfHeight, 72, h) * 10;
		attr.textstyle = HYP_TXT_NORMAL;
		if (lf.lfWeight >= FW_BOLD)
			attr.textstyle |= HYP_TXT_BOLD;
		if (lf.lfItalic)
			attr.textstyle |= HYP_TXT_ITALIC;
		if (lf.lfUnderline)
			attr.textstyle |= HYP_TXT_UNDERLINED;
		strncpy(attr.name, lf.lfFaceName, sizeof(attr.name));
		fontname = W_Fontdesc(&attr);
		if (fontname)
		{
			g_free(*desc);
			*desc = fontname;
			res = TRUE;
		}
	}
	ReleaseDC(HWND_DESKTOP, hdc);
	return res;
}

/*** ---------------------------------------------------------------------- ***/

static INT_PTR CALLBACK font_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD notifyCode;
	WINDOW_DATA *win;
	static HBRUSH bg_brush;
	static HFONT font;
	static HFONT xfont;
	
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_INITDIALOG:
		win = (WINDOW_DATA *)lParam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (DWORD_PTR)win);
		CenterWindow(hwnd);
		DlgSetText(hwnd, IDC_FONT_BUTTON, gl_profile.viewer.font_name);
		DlgSetText(hwnd, IDC_FONT_TEXT, _("Standard font"));
		font = W_FontCreate1(gl_profile.viewer.font_name);
		SendDlgItemMessage(hwnd, IDC_FONT_TEXT, WM_SETFONT, (WPARAM)font, FALSE);
		DlgSetText(hwnd, IDC_XFONT_BUTTON, gl_profile.viewer.xfont_name);
		DlgSetText(hwnd, IDC_XFONT_TEXT, _("Alternative font"));
		xfont = W_FontCreate1(gl_profile.viewer.xfont_name);
		SendDlgItemMessage(hwnd, IDC_XFONT_TEXT, WM_SETFONT, (WPARAM)xfont, FALSE);
		bg_brush = CreateSolidBrush(viewer_colors.background);
		return TRUE;
	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		DestroyWindow(hwnd);
		return TRUE;
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			DestroyWindow(hwnd);
			return TRUE;
		case IDOK:
			EndDialog(hwnd, IDOK);
			DestroyWindow(hwnd);
			SwitchFont(win, FALSE);
			break;
		case IDC_FONT_BUTTON:
			if (Choose1Font(hwnd, &gl_profile.viewer.font_name, _("Standard font")))
			{
				DeleteObject(font);
				font = W_FontCreate1(gl_profile.viewer.font_name);
				SendDlgItemMessage(hwnd, IDC_FONT_TEXT, WM_SETFONT, (WPARAM)font, TRUE);
				DlgSetText(hwnd, IDC_FONT_BUTTON, gl_profile.viewer.font_name);
			}
			break;
		case IDC_XFONT_BUTTON:
			if (Choose1Font(hwnd, &gl_profile.viewer.xfont_name, _("Alternative font")))
			{
				DeleteObject(xfont);
				xfont = W_FontCreate1(gl_profile.viewer.xfont_name);
				SendDlgItemMessage(hwnd, IDC_XFONT_TEXT, WM_SETFONT, (WPARAM)xfont, TRUE);
				DlgSetText(hwnd, IDC_XFONT_BUTTON, gl_profile.viewer.xfont_name);
			}
			break;
		case IDHELP:
			if (notifyCode == BN_CLICKED)
			{
				Help_Show(win, _("Select Fonts"));
			}
			break;
		}
		break;
	case WM_DESTROY:
		DeleteObject(bg_brush);
		DeleteObject(font);
		DeleteObject(xfont);
		break;
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOREDIT:
		if (GetDlgCtrlID((HWND)lParam) == IDC_FONT_TEXT ||
			GetDlgCtrlID((HWND)lParam) == IDC_XFONT_TEXT)
		{
			HDC hdcStatic = (HDC) wParam;
			SetTextColor(hdcStatic, viewer_colors.text);
			SetBkColor(hdcStatic, viewer_colors.background);
			SetBkMode(hdcStatic, TRANSPARENT);
			return (INT_PTR)bg_brush;
		}
		break;
		
	default:
		hv_commdlg_help(hwnd, message, wParam, lParam);
		break;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

void SelectFont(WINDOW_DATA *win)
{
	HWND parent = win ? win->hwnd : 0;
	DialogBoxExW(NULL, MAKEINTRESOURCEW(IDD_FONTSEL), parent, font_dialog, (LPARAM)win);
}
