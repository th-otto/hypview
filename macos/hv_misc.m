#include "hv_defs.h"
#include "hypdebug.h"
#include "../rcintl/localename.h"

#define MAX_RECENT 10

static GSList *recent_list;

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

void RecentUpdate(WINDOW_DATA *win)
{
#if 0 /* TODO */
	HMENU menu;
	int i;
	GSList *l;
	
	if (win == NULL)
		return;
	menu = win->recent_menu;
	for (i = 0; i < MAX_RECENT; i++)
		RemoveMenu(menu, i + IDM_FILE_RECENT_1, MF_BYCOMMAND);
	for (l = recent_list, i = 0; l && i < MAX_RECENT; l = l->next, i++)
	{
		const char *path = (const char *)l->data;
		char *str = g_strdup_printf("%s\tCtrl+%c", path, i == 9 ? '0' : i + '1');
		wchar_t *wstr = hyp_utf8_to_wchar(str, STR0TERM, NULL);
		AppendMenuW(menu, MF_STRING, i + IDM_FILE_RECENT_1, wstr);
		g_free(wstr);
		g_free(str);
	}
#endif
}

/*** ---------------------------------------------------------------------- ***/

#if 0 /* TODO */
void on_recent_selected(WINDOW_DATA *win, int sel)
{
	GSList *l;
	
	for (l = recent_list; l; l = l->next)
	{
		if (sel == 0)
		{
			const char *path = (const char *)l->data;
			hv_recent_add(path); /* move it to top of list */
			if (OpenFileInWindow(win, path, NULL, 0, TRUE, FALSE, FALSE) == NULL)
			{
				ASSERT(recent_list);
				g_free(recent_list->data);
				recent_list = g_slist_delete_link(recent_list, recent_list);
				RecentUpdate(win);
			}
			return;
		}
		sel--;
	}
}
#endif

/*** ---------------------------------------------------------------------- ***/

void hv_recent_add(const char *path)
{
	GSList *l, **last;
	int count;
	char *newpath = g_strdup(path);
	
	convexternalslash(newpath);
	for (last = &recent_list, count = 0; (l = *last) != NULL; last = &(*last)->next)
	{
		const char *oldpath = (const char *)l->data;
		if (filename_cmp(newpath, oldpath) == 0)
		{
			*last = l->next;
			l->next = recent_list;
			recent_list = l;
			g_free(newpath);
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
	recent_list = g_slist_prepend(recent_list, newpath);
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

/*** ---------------------------------------------------------------------- ***/

char *hyp_wchar_to_utf8(const wchar_t *str, size_t wlen)
{
	size_t len;
	char *dst, *p;
	h_unichar_t wc;
	
	if (str == NULL)
		return NULL;
	if (wlen == STR0TERM)
		wlen = wcslen(str);
	len = wlen * HYP_UTF8_CHARMAX + 1;
	dst = p = g_new(char, len);
	if (dst == NULL)
		return NULL;
	while (wlen)
	{
		wc = *str++;
		hyp_put_unichar(p, wc);
		wlen--;
	}
	*p++ = '\0';
	return g_renew(char, dst, p - dst);
}
