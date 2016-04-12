#include "hv_defs.h"
#include "hypdebug.h"
#include "resource.rh"


#define MAX_MARKEN		12
#define UNKNOWN_LEN		10
#define PATH_LEN		128
#define NODE_LEN		40


typedef struct
{
	hyp_nodenr node_num;
	short line;
	char unknown[UNKNOWN_LEN];
	char path[PATH_LEN];				/* full path */
	char node_name[NODE_LEN];			/* display title */
} MARKEN;

static gboolean marken_change;
static MARKEN marken[MAX_MARKEN];

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void MarkerDelete(short num)
{
	char *dst;
	
	memset(&marken[num], 0, sizeof(MARKEN));
	marken[num].node_num = HYP_NOINDEX;
	dst = marken[num].node_name;
	strcpy(dst, _("free"));
}

/*** ---------------------------------------------------------------------- ***/

void MarkerSave(WINDOW_DATA *win, short num)
{
	DOCUMENT *doc = win->data;
	const char *src;
	char *dst, *end;

	/* avoid illegal parameters */
	if (num < 0 || num >= MAX_MARKEN)
		return;

	marken[num].node_num = doc->getNodeProc(win);
	marken[num].line = hv_win_topline(win);
	strncpy(marken[num].path, doc->path, PATH_LEN - 1);
	marken[num].path[PATH_LEN - 1] = 0;

	/* copy marker title */
	src = win->title;
	dst = marken[num].node_name;
	end = &marken[num].node_name[NODE_LEN - 1];
	while (dst < end)
	{
		if (*src)
			*dst++ = *src++;
		else
			break;
	}
	*dst = 0;

	{
		char ZStr[255];
		long len;

		strcpy(ZStr, " (");
		src = hyp_basename(marken[num].path);
		strcat(ZStr, src);
		strcat(ZStr, ")");
		len = strlen(ZStr);
		if (strlen(marken[num].node_name) + len < NODE_LEN)
			strcat(marken[num].node_name, ZStr);
	}

	marken_change = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void MarkerShow(WINDOW_DATA *win, short num, gboolean new_window)
{
	/* avoid illegal parameters */
	if (num < 0 || num >= MAX_MARKEN)
		return;

	if (marken[num].node_num != HYP_NOINDEX)
	{
		win = OpenFileInWindow(win, marken[num].path, NULL, marken[num].node_num, TRUE, new_window, FALSE);
		if (win != NULL)
		{
			GotoPage(win, marken[num].node_num, marken[num].line, FALSE);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void on_bookmark_selected(WINDOW_DATA *win, int sel)
{
	gboolean shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
	gboolean alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
	gboolean ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
	
	if (sel >= 0 && sel < MAX_MARKEN)
	{
		if (shift)
		{
			MarkerSave(win, sel);
		} else if (marken[sel].node_num == HYP_NOINDEX)
		{
			char *buff;

			buff = g_strdup_printf(_("Do you want to add\n%s\nto your bookmarks?"), win->title);
			if (ask_yesno(win ? win->hwnd : NULL, buff))
				MarkerSave(win, sel);
			g_free(buff);
		} else
		{
			if (alt)
			{
				char *buff;

				buff = g_strdup_printf(_("Do you want to remove\n%s\nfrom your bookmarks?"), marken[sel].node_name);
				if (ask_yesno(win ? win->hwnd : NULL, buff))
				{
					MarkerDelete(sel);
					marken_change = TRUE;
				}
				g_free(buff);
			} else
			{
				MarkerShow(win, sel, ctrl);
			}
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void MarkerUpdate(WINDOW_DATA *win)
{
	HMENU menu;
	int i;
	MENUITEMINFOW info;
	
	menu = win->bookmarks_menu;
	memset(&info, 0, sizeof(info));
	info.cbSize = sizeof(info);
	info.fMask = MIIM_STRING;
	info.fType = MFT_STRING;
	for (i = 0; i < MAX_MARKEN; i++)
	{
		char *str = g_strdup_printf("%s\tF%d", marken[i].node_name, i + 1);
		info.dwTypeData = hyp_utf8_to_wchar(str, STR0TERM, NULL);
		SetMenuItemInfoW(menu, i + IDM_NAV_BOOKMARK_1, FALSE, &info);
		g_free(info.dwTypeData);
		g_free(str);
	}
}

/*** ---------------------------------------------------------------------- ***/

void MarkerPopup(WINDOW_DATA *win, int button)
{
	HMENU menu;
	struct popup_pos popup_pos;
	int x, y;
	UINT flags = TPM_LEFTALIGN | TPM_TOPALIGN | (button == 1 ? TPM_LEFTBUTTON : TPM_RIGHTBUTTON) | TPM_NOANIMATION;
	
	if (!(win->m_buttons[TO_BOOKMARKS] & WS_VISIBLE))
		return;
	if ((win->m_buttons[TO_BOOKMARKS] & WS_DISABLED))
		return;
	
	MarkerUpdate(win);
	menu = win->bookmarks_menu;
	
	popup_pos.window = win;
	popup_pos.obj = TO_BOOKMARKS;
	if (position_popup(menu, &popup_pos, &x, &y) == FALSE)
		return;
	SetForegroundWindow(win->hwnd);
	TrackPopupMenu(menu, flags, x, y, 0, win->hwnd, NULL);
	PostMessage(win->hwnd, WM_NULL, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

void MarkerSaveToDisk(gboolean ask)
{
	char *filename;
	
	if (!marken_change)
		return;

	if (!empty(gl_profile.viewer.marker_path))
	{
		int ret;

		if (ask)
		{
			if (ask_yesno(top_window(), _("Save bookmarks?")) == FALSE)
				return;
		}
		filename = path_subst(gl_profile.viewer.marker_path);
		ret = open(filename, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0644);
		if (ret >= 0)
		{
			write(ret, marken, sizeof(MARKEN) * MAX_MARKEN);
			close(ret);
			marken_change = FALSE;
		} else
		{
			HYP_DBG(("Error %ld: saving %s", ret, printnull(filename)));
		}
		g_free(filename);
	}
}

/*** ---------------------------------------------------------------------- ***/

void MarkerInit(void)
{
	short i;
	int ret;
	char *filename;
	
	/* initialize markers */
	for (i = 0; i < MAX_MARKEN; i++)
	{
		MarkerDelete(i);
	}

	/* load file if it exists */
	if (!empty(gl_profile.viewer.marker_path))
	{
		filename = path_subst(gl_profile.viewer.marker_path);
		ret = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (ret >= 0)
		{
			read(ret, marken, sizeof(MARKEN) * MAX_MARKEN);
			for (i = 0; i < MAX_MARKEN; i++)
			{
				if (marken[i].node_name[0] == 0)
					MarkerDelete(i);
			}
			hyp_utf8_close(ret);
		}
		g_free(filename);
	}

	marken_change = FALSE;
}
