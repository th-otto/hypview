#include "hv_defs.h"
#include "hypdebug.h"
#include "localename.h"

#define MAX_RECENT 10

static GSList *recent_list;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * like LoadMenu, but uses the specified LANGID
 * (which should be determined by gl_locale_win32_messages_langid())
 * so it can be overridden by environment variables,
 * like on Unix
 */
HMENU WINAPI LoadMenuExW(HINSTANCE instance, LPCWSTR name)
{
	HRSRC hrsrc;
	LPCWSTR type = MAKEINTRESOURCEW((DWORD)(DWORD_PTR)RT_MENU);
	LANGID lang = gl_locale_win32_messages_langid();
 	
	if (!instance)
		instance = GetInstance();
	hrsrc = FindResourceExW(instance, type, name, lang);
	if (!hrsrc && SUBLANGID(lang) != SUBLANG_DEFAULT)
		hrsrc = FindResourceExW(instance, type, name, MAKELANGID(PRIMARYLANGID(lang), SUBLANG_DEFAULT));
	if (!hrsrc && SUBLANGID(lang) != SUBLANG_NEUTRAL)
		hrsrc = FindResourceExW(instance, type, name, MAKELANGID(PRIMARYLANGID(lang), SUBLANG_NEUTRAL));
	if (!hrsrc && lang != MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
		hrsrc = FindResourceExW(instance, type, name, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
	if (!hrsrc)
		return 0;
	return LoadMenuIndirectW(LoadResource(instance, hrsrc));
}

/*** ---------------------------------------------------------------------- ***/

INT_PTR WINAPI DialogBoxExW(HINSTANCE instance, LPCWSTR name, HWND owner, DLGPROC dlgProc, LPARAM param)
{
	HRSRC hrsrc;
	LPCWSTR type = MAKEINTRESOURCEW((DWORD)(DWORD_PTR)RT_DIALOG);
	LPCDLGTEMPLATE templ;
	LANGID lang = gl_locale_win32_messages_langid();
	
	if (!instance)
		instance = GetInstance();
	hrsrc = FindResourceExW(instance, type, name, lang);
	if (!hrsrc && SUBLANGID(lang) != SUBLANG_DEFAULT)
		hrsrc = FindResourceExW(instance, type, name, MAKELANGID(PRIMARYLANGID(lang), SUBLANG_DEFAULT));
	if (!hrsrc && SUBLANGID(lang) != SUBLANG_NEUTRAL)
		hrsrc = FindResourceExW(instance, type, name, MAKELANGID(PRIMARYLANGID(lang), SUBLANG_NEUTRAL));
	if (!hrsrc && lang != MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
		hrsrc = FindResourceExW(instance, type, name, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
	if (!hrsrc)
		return -1;
	templ = (LPCDLGTEMPLATE)LoadResource(instance, hrsrc);
	if (!templ)
		return -1;
	return DialogBoxIndirectParamW(instance, templ, owner, dlgProc, param);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static int read_int(const char *string, const char **next)
{
	int result = 0;
	int sign = 1;

	if (*string == '+')
		string++;
	else if (*string == '-')
	{
		string++;
		sign = -1;
	}

	for (; (*string >= '0') && (*string <= '9'); string++)
	{
		result = (result * 10) + (*string - '0');
	}

	*next = string;

	if (sign >= 0)
		return result;
	return -result;
}

/* 
 * Bitmask returned by XParseGeometry().  Each bit tells if the corresponding
 * value (x, y, width, height) was found in the parsed string.
 */
#define NoValue         0x0000
#define XValue          0x0001
#define YValue          0x0002
#define WidthValue      0x0004
#define HeightValue     0x0008
#define AllValues       0x000F
#define XNegative       0x0010
#define YNegative       0x0020

int gtk_XParseGeometry(const char *string, int *x, int *y, int *width, int *height)
{
	int mask = NoValue;
	const char *strind;
	unsigned int tempWidth, tempHeight;
	int tempX, tempY;

	const char *nextCharacter;

	/* These initializations are just to silence gcc */
	tempWidth = 0;
	tempHeight = 0;
	tempX = 0;
	tempY = 0;

	if ((string == NULL) || (*string == '\0'))
		return mask;
	if (*string == '=')
		string++;						/* ignore possible '=' at beg of geometry spec */

	strind = string;
	if (*strind != '+' && *strind != '-' && *strind != 'x')
	{
		tempWidth = read_int(strind, &nextCharacter);
		if (strind == nextCharacter)
			return NoValue;
		strind = nextCharacter;
		mask |= WidthValue;
	}

	if (*strind == 'x' || *strind == 'X')
	{
		strind++;
		tempHeight = read_int(strind, &nextCharacter);
		if (strind == nextCharacter)
			return NoValue;
		strind = nextCharacter;
		mask |= HeightValue;
	}

	if ((*strind == '+') || (*strind == '-'))
	{
		if (*strind == '-')
		{
			strind++;
			tempX = -read_int(strind, &nextCharacter);
			if (strind == nextCharacter)
				return NoValue;
			strind = nextCharacter;
			mask |= XNegative;

		} else
		{
			strind++;
			tempX = read_int(strind, &nextCharacter);
			if (strind == nextCharacter)
				return NoValue;
			strind = nextCharacter;
		}
		mask |= XValue;
		if ((*strind == '+') || (*strind == '-'))
		{
			if (*strind == '-')
			{
				strind++;
				tempY = -read_int(strind, &nextCharacter);
				if (strind == nextCharacter)
					return NoValue;
				strind = nextCharacter;
				mask |= YNegative;
			} else
			{
				strind++;
				tempY = read_int(strind, &nextCharacter);
				if (strind == nextCharacter)
					return NoValue;
				strind = nextCharacter;
			}
			mask |= YValue;
		}
	}

	/* If strind isn't at the end of the string then it's an invalid
	   geometry specification. */

	if (*strind != '\0')
		return NoValue;

	if (mask & XValue)
		*x = tempX;
	if (mask & YValue)
		*y = tempY;
	if (mask & WidthValue)
		*width = tempWidth;
	if (mask & HeightValue)
		*height = tempHeight;
	return mask;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

char *DlgGetText(HWND hwnd, int id)
{
	int len = SendDlgItemMessageW(hwnd, id, WM_GETTEXTLENGTH, 0, 0);
	wchar_t *wstr;
	char *str;
	
	wstr = g_new0(wchar_t, len + 1);
	if (wstr == NULL)
		return NULL;
	SendDlgItemMessageW(hwnd, id, WM_GETTEXT, len + 1, (LPARAM)wstr);
	str = hyp_wchar_to_utf8(wstr, len);
	g_free(wstr);
	return str;
}

/*** ---------------------------------------------------------------------- ***/

wchar_t *DlgGetTextW(HWND hwnd, int id)
{
	int len = SendDlgItemMessageW(hwnd, id, WM_GETTEXTLENGTH, 0, 0);
	wchar_t *wstr;
	
	wstr = g_new0(wchar_t, len + 1);
	if (wstr == NULL)
		return NULL;
	SendDlgItemMessageW(hwnd, id, WM_GETTEXT, len + 1, (LPARAM)wstr);
	return wstr;
}

/*** ---------------------------------------------------------------------- ***/

void DlgSetText(HWND hwnd, int id, const char *str)
{
	wchar_t *wstr = hyp_utf8_to_wchar(str, STR0TERM, NULL);
	SendDlgItemMessageW(hwnd, id, WM_SETTEXT, 0, (LPARAM)wstr);
	g_free(wstr);
}

/*** ---------------------------------------------------------------------- ***/

gboolean DlgGetButton(HWND hwnd, int id)
{
	return IsDlgButtonChecked(hwnd, id) == BST_CHECKED;
}

/*** ---------------------------------------------------------------------- ***/

void DlgSetButton(HWND hwnd, int id, gboolean check)
{
	CheckDlgButton(hwnd, id, check ? BST_CHECKED : BST_UNCHECKED);
}

/*** ---------------------------------------------------------------------- ***/

void DlgEnable(HWND hwnd, int id, gboolean enable)
{
	EnableWindow(GetDlgItem(hwnd, id), enable);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void CenterWindow(HWND hwnd)
{
	RECT r;
	RECT p;
	RECT mainrect;
	HWND parent;
	
	parent = GetParent(hwnd);
	GetWindowRect(hwnd, &r);
	if (parent == NULL)
	{
		p.left = 0;
		p.top = 0;
		p.right = GetSystemMetrics(SM_CXFULLSCREEN);
		p.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
		mainrect = p;
	} else
	{
		GetClientRect(parent, &p);
		GetWindowRect(parent, &mainrect);
	}
	if (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
		SetWindowPos(hwnd, NULL, p.left + ((p.right - p.left) - (r.right - r.left)) / 2, p.top + ((p.bottom - p.top) - (r.bottom - r.top)) / 2, r.right - r.left, r.bottom - r.top, SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER);
	else
		SetWindowPos(hwnd, NULL, mainrect.left + ((p.right - p.left) - (r.right - r.left)) / 2, mainrect.top + ((p.bottom - p.top) - (r.bottom - r.top)) / 2, r.right - r.left, r.bottom - r.top, SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER);
}

/*** ---------------------------------------------------------------------- ***/

void RecentUpdate(WINDOW_DATA *win)
{
	UNUSED(win);
	/* NYI */
}

/*** ---------------------------------------------------------------------- ***/

void on_recent_selected(WINDOW_DATA *win, int sel)
{
	GSList *l;
	
	for (l = recent_list; l; l = l->next)
	{
		if (sel == 0)
		{
			const char *path = (const char *)l->data;
			hv_recent_add(path); /* move it to top of list */
			OpenFileInWindow(win, path, NULL, HYP_NOINDEX, TRUE, FALSE, FALSE);
			return;
		}
		sel--;
	}
}

/*** ---------------------------------------------------------------------- ***/

void hv_recent_add(const char *path)
{
	GSList *l, **last;
	int count;
	
	for (last = &recent_list, count = 0; (l = *last) != NULL; last = &(*last)->next)
	{
		const char *oldpath = (const char *)l->data;
		if (filename_cmp(path, oldpath) == 0)
		{
			*last = l->next;
			l->next = recent_list;
			recent_list = l;
			return;
		}
		if (++count >= MAX_RECENT)
		{
			g_free(l->data);
			g_slist_free_1(l);
			*last = NULL;
			break;
		}
	}
	recent_list = g_slist_prepend(recent_list, g_strdup(path));
}

/*** ---------------------------------------------------------------------- ***/

void RecentInit(void)
{
	int i;
	char *name;
	gboolean found;
	Profile *profile = gl_profile.profile;
	char *path;
	
	g_slist_free_full(recent_list, g_free);
	recent_list = NULL;
	i = 0;
	for (;;)
	{
		if (i >= MAX_RECENT)
			break;
		name = g_strdup_printf("recent-%d", i);
		path = NULL;
		found = Profile_ReadString(profile, "Recent", name, &path);
		g_free(name);
		if (!found)
			break;
		hv_recent_add(path);
		g_free(path);
		i++;
	}
}

/*** ---------------------------------------------------------------------- ***/

void RecentExit(void)
{
	g_slist_free_full(recent_list, g_free);
	recent_list = NULL;
}

/*** ---------------------------------------------------------------------- ***/

void RecentSaveToDisk(void)
{
	int i;
	char *name;
	gboolean done;
	Profile *profile = gl_profile.profile;
	GSList *l;
	
	i = 0;
	do
	{
		name = g_strdup_printf("recent-%d", i);
		done = Profile_DeleteKey(profile, "Recent", name);
		g_free(name);
		i++;
	} while (done);
	i = 0;
	for (l = recent_list; l; l = l->next)
	{
		name = g_strdup_printf("recent-%d", i);
		Profile_WriteString(profile, "Recent", name, (const char *)l->data);
		g_free(name);
		i++;
	}
}
