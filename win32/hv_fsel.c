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

char const hypertext_file_filter[] = N_("*.hyp|Hypertext files (*.hyp)\n*.*|All files (*.*)\n");
char const text_file_filter[] = N_("*.txt|Text files (*.txt)\n*.*|All files (*.*)\n");
char const stg_file_filter[] = N_("*.stg|ST-Guide files (*.stg)\n*.*|All files (*.*)\n");
#ifdef WITH_PDF
char const pdf_file_filter[] = N_("*.pdf|PDF files (*.pdf)\n*.*|All files (*.*)\n");
#endif

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

gboolean choose_file(HWND parent, char **name, enum choose_file_mode mode, const char *title, const char *filter)
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
	char *realname;
	char *freeme = NULL;
	
	commdlg_help = RegisterWindowMessageW(HELPMSGSTRINGW);
	
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = parent;
	ofn.hInstance = 0;
	if (mode == choose_file_dirsel)
	{
		realname = freeme = g_build_filename(*name, "*.*", NULL);
	} else
	{
		realname = *name;
	}
	base = hyp_basename(realname);
	wpath = hyp_utf8_to_wchar(realname, base - realname, NULL);
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
	if (mode == choose_file_save)
	{
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		retV = GetSaveFileNameW(&ofn);
	} else if (mode == choose_file_dirsel)
	{
		retV = GetOpenFileNameW(&ofn);
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
			if (mode == choose_file_save)
				retV = GetSaveFileNameW(&ofn);
			else
				retV = GetOpenFileNameW(&ofn);
		}
	}
	g_free(wtitle);
	g_free(wfilter);
	g_free(freeme);
	if (retV)
	{
		g_free(*name);
		*name = hyp_wchar_to_utf8(szName, STR0TERM);
		if (mode == choose_file_dirsel)
		{
			freeme = hyp_path_get_dirname(*name);
			g_free(*name);
			*name = freeme;
		}
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
	
	if (choose_file(parent, &name, choose_file_open, _("Open Hypertext..."), _(hypertext_file_filter)))
	{
		hv_recent_add(name);
		win = OpenFileInWindow(win, name, NULL, 0, TRUE, FALSE, FALSE);
	}
	g_free(name);
	return win;
}

/*** ---------------------------------------------------------------------- ***/

char *SelectFileSave(WINDOW_DATA *win, hyp_filetype type)
{
	DOCUMENT *doc = win->data;
	char *filepath;
	HWND parent = win ? win->hwnd : NULL;
	const char *defext;
	const char *filter;
	const char *title;
	
	switch (type)
	{
	case HYP_FT_ASCII:
	case HYP_FT_CHEADER:
		defext = HYP_EXT_TXT;
		filter = _(text_file_filter);
		title = _("Save ASCII text as");
		break;
	case HYP_FT_STG:
	case HYP_FT_GUIDE:
		defext = HYP_EXT_STG;
		filter = _(stg_file_filter);
		title = _("Recompile to");
		break;
	case HYP_FT_PDF:
#ifdef WITH_PDF
		defext = HYP_EXT_PDF;
		filter = _(pdf_file_filter);
		title = _("Recompile to PDF");
		break;
#else
		return NULL;
#endif
	case HYP_FT_NONE:
	case HYP_FT_UNKNOWN:
	case HYP_FT_LOADERROR:
	case HYP_FT_BINARY:
	case HYP_FT_HYP:
	case HYP_FT_REF:
	case HYP_FT_RSC:
	case HYP_FT_IMAGE:
	case HYP_FT_HTML:
	case HYP_FT_XML:
	case HYP_FT_HTML_XML:
	case HYP_FT_TREEVIEW:
	default:
		return NULL;
	}
	
	if (gl_profile.output.output_dir)
	{
		char *name = replace_ext(hyp_basename(doc->path), NULL, defext);
		filepath = g_build_filename(gl_profile.output.output_dir, name, NULL);
		g_free(name);
	} else
	{
		filepath = replace_ext(doc->path, NULL, defext);
	}
	
	if (choose_file(parent, &filepath, choose_file_save, title, filter))
	{
		g_free(gl_profile.output.output_dir);
		gl_profile.output.output_dir = hyp_path_get_dirname(filepath);
		HypProfile_SetChanged();
	} else
	{
		g_free(filepath);
		filepath = NULL;
	}
	return filepath;
}
