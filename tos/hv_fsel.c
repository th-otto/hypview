/*
 * HypView - (c)      - 2006 Philipp Donze
 *               2006 -      Philipp Donze & Odd Skancke
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
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hv_defs.h"
#include "hypview.h"
#include <mint/arch/nf_ops.h>


/* files and paths */
static char file_extensions[DL_PATHMAX] = "*.HYP\0*.*\0";

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * Create a list of absolute paths of <src> in <dst>.
 * Ignores relative paths.
 */
static void CreatePathList(char *dst, const char *src)
{
	while (*src)
	{
		const char *start = src;
		char *dir;
		
		while (*src && *src != G_SEARCHPATH_SEPARATOR)
		{
			src++;
		}
		dir = g_strndup(start, src - start);
		if (!empty(dir) && g_path_is_absolute(dir))
		{
			strcpy(dst, dir);
			dst += strlen(dst);
			if (dst[-1] != '\\' && dst[-1] != '/')
			{
				*dst++ = '\\';
			}
			*dst++ = 0;
		}
		g_free(dir);
		if (*src)
			src++;
	}

	*dst++ = 0;
	*dst = 0;
}

/*** ---------------------------------------------------------------------- ***/

static void OpenFile_FSLX(FILESEL_DATA *fslx, short nfiles)
{
	UNUSED(nfiles);
	if (fslx->button && *fslx->name)
	{
		WINDOW_DATA *win = fslx->data;
		char *path = g_build_filename(fslx->path, fslx->name, NULL);
		OpenFileInWindow(win, path, hyp_default_main_node_name, HYP_NOINDEX, TRUE, FALSE, FALSE);
		g_free(path);
	}
}

/*** ---------------------------------------------------------------------- ***/

void SelectFileLoad(WINDOW_DATA *win)
{
	char paths[550];
	char *subst;
	
	subst = path_subst(gl_profile.general.path_list);
	CreatePathList(paths, subst);
	g_free(subst);
	
	OpenFileselector(OpenFile_FSLX, rs_string(FSLX_LOAD), NULL, paths, file_extensions, 0, win);
}

/*** ---------------------------------------------------------------------- ***/

static void SaveFile_FSLX(FILESEL_DATA *fslx, short nfiles)
{
	WINDOW_DATA *win = fslx->data;

	UNUSED(nfiles);
	if (fslx->button)
	{
		char *path;
		int ret;

		path = g_build_filename(fslx->path, fslx->name, NULL);
		ret = open(path, O_RDONLY | O_BINARY);
		if (ret >= 0)
		{
			close(ret);
			if (form_alert(2, rs_string(WARN_FEXIST)) != 1)
			{
				g_free(path);
				return;
			}
		}
		BlockAsciiSave(win, path);
		g_free(path);
	}
}

/*** ---------------------------------------------------------------------- ***/

void SelectFileSave(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	char paths[550];
	char *filepath;
	char *subst;

	subst = path_subst(gl_profile.general.path_list);
	CreatePathList(paths, subst);
	filepath = replace_ext(doc->path, NULL, ".txt");

	OpenFileselector(SaveFile_FSLX, rs_string(FSLX_SAVE), filepath, paths, "*.txt\0", 0, win);
	g_free(filepath);
	g_free(subst);
}
