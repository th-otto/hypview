#include "hv_defs.h"

#define IDS_SELECT_HYPERTEXT _("*.hyp|Hypertext files (*.hyp)\n*.*|All files (*.*)\n")
#define IDS_SELECT_TEXTFILES _("*.txt|Text files (*.txt)\n*.*|All files (*.*)\n")

#define G_SEARCHPATH_SEPARATOR_S ";"

#ifndef OFN_NONETWORKBUTTON
#define OFN_NONETWORKBUTTON			(0x00020000UL)
#endif
#ifndef OFN_NOLONGNAMES
#define OFN_NOLONGNAMES				(0x00040000UL)
#endif
#ifndef OFN_EXPLORER
#define OFN_EXPLORER				(0x00080000UL)
#endif
#ifndef OFN_NODEREFERENCELINKS
#define OFN_NODEREFERENCELINKS		(0x00100000UL)
#endif
#ifndef OFN_LONGNAMES
#define OFN_LONGNAMES				(0x00200000UL)
#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static wchar_t *hv_file_selector_add_filter(const char *filter)
{
	char **filterlist;
	char *displayname;
	int i;
	wchar_t *wfilter = NULL;
	size_t wlen = 0;
	size_t len;
	wchar_t *wdisplayname;
	wchar_t *wpattern;
	
	if (empty(filter))
		return NULL;
	filterlist = g_strsplit(filter, "\n", 0);
	for (i = 0; filterlist != NULL && filterlist[i] != NULL; i++)
	{
		if (empty(filterlist[i]))
			continue;
		displayname = strchr(filterlist[i], '|');
		if (displayname != NULL)
		{
			*displayname++ = '\0';
			wdisplayname = hyp_utf8_to_wchar(displayname, STR0TERM, NULL);
			wpattern = hyp_utf8_to_wchar(filterlist[i], STR0TERM, NULL);
		} else
		{
			wpattern = hyp_utf8_to_wchar(filterlist[i], STR0TERM, NULL);
			wdisplayname = wpattern;
		}
		len = wcslen(wdisplayname) + 1;
		wfilter = g_renew(wchar_t, wfilter, wlen + len);
		wcscpy(wfilter + wlen, wdisplayname);
		wlen += len;
		len = wcslen(wpattern) + 1;
		wfilter = g_renew(wchar_t, wfilter, wlen + len);
		wcscpy(wfilter + wlen, wpattern);
		wlen += len;
		g_free(wpattern);
		if (wdisplayname != wpattern)
			g_free(wdisplayname);
	}
	wfilter = g_renew(wchar_t, wfilter, wlen + 1);
	wfilter[wlen] = 0;
	g_strfreev(filterlist);
	return wfilter;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean choose_file(HWND parent, char **name, gboolean save, const char *title, const char *filter)
{
	OPENFILENAMEW ofn;
	gboolean retV;
	const int wmax = PATH_MAX;
	wchar_t szName[wmax];
	wchar_t *wpath;
	wchar_t *wtitle;
	wchar_t *wfilter;
	wchar_t dirname[wmax];
	const char *base;
	
	commdlg_help = RegisterWindowMessageW(HELPMSGSTRINGW);
	
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = parent;
	ofn.hInstance = 0;
	base = hyp_basename(*name);
	wpath = hyp_utf8_to_wchar(*name, base - *name, NULL);
	wcsncpy(dirname, wpath, wmax);
	g_free(wpath);
	wpath = hyp_utf8_to_wchar(base, STR0TERM, NULL);
	wcsncpy(szName, wpath, wmax);
	if (*szName == '*')
		*szName = '\0';
	g_free(wpath);
	ofn.nMaxFile = wmax;
	ofn.lpstrFile = szName;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = dirname;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	wtitle = hyp_utf8_to_wchar(title, STR0TERM, NULL);
	ofn.lpstrTitle = wtitle;
	wfilter = hv_file_selector_add_filter(filter);
	ofn.lpstrFilter = wfilter;
	if (save)
	{
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		retV = GetSaveFileNameW(&ofn);
	} else
	{
		ofn.Flags |= OFN_FILEMUSTEXIST;
		retV = GetOpenFileNameW(&ofn);
	}
	if (retV == FALSE)
	{
		DWORD err;
		
		err = CommDlgExtendedError();
		if (err == CDERR_STRUCTSIZE)
		{
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			if (save)
				retV = GetSaveFileNameW(&ofn);
			else
				retV = GetOpenFileNameW(&ofn);
		}
	}
	g_free(wtitle);
	g_free(wfilter);
	if (retV)
	{
		g_free(*name);
		*name = hyp_wchar_to_utf8(szName, STR0TERM);
	}
	return retV;
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *SelectFileLoad(WINDOW_DATA *win)
{
	char *name;
	char *subst;
	HWND parent = win ? win->hwnd : NULL;
	char **paths;
	
	if (win)
	{
		DOCUMENT *doc = (DOCUMENT *)win->data;
		name = g_strdup(doc->path);
	} else
	{
		subst = path_subst(gl_profile.general.path_list);
		paths = g_strsplit(subst, G_SEARCHPATH_SEPARATOR_S, 0);
		name = g_build_filename(paths[0], "*.hyp", NULL);
		g_strfreev(paths);
		g_free(subst);
	}
	
	if (choose_file(parent, &name, FALSE, _("Open Hypertext..."), IDS_SELECT_HYPERTEXT))
	{
		hv_recent_add(name);
		win = OpenFileInWindow(win, name, hyp_default_main_node_name, HYP_NOINDEX, TRUE, FALSE, FALSE);
	}
	g_free(name);
	return win;
}

/*** ---------------------------------------------------------------------- ***/

void SelectFileSave(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	char *filepath;
	HWND parent = win ? win->hwnd : NULL;
	filepath = replace_ext(doc->path, NULL, ".txt");
	
	if (choose_file(parent, &filepath, TRUE, _("Save ASCII text as"), IDS_SELECT_TEXTFILES))
	{
#if 0
		int ret;

		ret = hyp_utf8_open(filepath, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (ret >= 0)
		{
			hyp_utf8_close(ret);
			if (ask_yesno(parent, _("This file exists already.\nDo you want to replace it?")))
				ret = -1;
		}
		if (ret < 0)
#endif
			BlockAsciiSave(win, filepath);
	}
	g_free(filepath);
}
