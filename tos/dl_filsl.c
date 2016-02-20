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

static char fslx_default_paths[DL_PATHMAX] = "C:\\\0";

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void *OpenFileselector(HNDL_FSL proc, char *comment, char *filepath, char *path, const char *pattern, short mode, void *user_data)
{
	FILESEL_DATA *ptr;
	const char *fslx_pattern = "*\0";
	char *fslx_paths = fslx_default_paths;
	
	ptr = g_new0(FILESEL_DATA, 1);
	if (ptr == NULL)
	{
		form_alert(1, rs_string(DI_MEMORY_ERROR));
		return NULL;
	}
	ptr->data = user_data;
	
	if (filepath)
	{
		const char *f;

		strcpy(ptr->path, filepath);
		convslash(ptr->path);
		
		f = hyp_basename(ptr->path);

		strcpy(ptr->name, f);
		ptr->path[f - ptr->path] = 0;
	}

	if (path)
		fslx_paths = path;

	if (!*ptr->path)
		strcpy(ptr->path, fslx_paths);

	if (has_filesel_dialog())
	{
		if (pattern)
			fslx_pattern = pattern;

		ptr->dialog = fslx_open(comment, -1, -1, &ptr->whandle, ptr->path, DL_PATHMAX,
								ptr->name, DL_PATHMAX, fslx_pattern, 0L, fslx_paths, SORTBYNAME, mode);

		if (ptr->dialog)
		{
			ptr->type = WIN_FILESEL;
			ptr->status = WIS_OPEN;
			ptr->proc = proc;
			ptr->owner = gl_apid;
			add_item((CHAIN_DATA *) ptr);
			ModalItem();
		} else
		{
			g_free(ptr);
			form_alert(1, rs_string(DI_MEMORY_ERROR));
			return NULL;
		}

		return ptr->dialog;
	} else
	{
		short result;
		const char *p;
		
		if (pattern && !strchr(pattern, ','))	/* only a single pattern? */
			strcat(ptr->path, pattern);
		else
			strcat(ptr->path, "*.*");
		ptr->dialog = NULL;

		result = fsel_exinput(ptr->path, ptr->name, &ptr->button, comment);
		if (!result || !ptr->button)
		{
			g_free(ptr);
			return NULL;						/* return error */
		}
		
		p = hyp_basename(ptr->path);
		/* remove file pattern */
		ptr->path[p - ptr->path] = '\0';
		proc(ptr, 1);
		g_free(ptr);
		return NULL;
	}
}

/*** ---------------------------------------------------------------------- ***/

void FileselectorEvents(FILESEL_DATA *ptr, EVNT *event)
{
	char *pattern;

	if (!fslx_evnt(ptr->dialog, event, ptr->path, ptr->name, &ptr->button, &ptr->nfiles, &ptr->sort_mode, &pattern))
	{
		ptr->proc(ptr, ptr->nfiles);
		RemoveFileselector(ptr);
	}
}

/*** ---------------------------------------------------------------------- ***/

void RemoveFileselector(FILESEL_DATA * ptr)
{
	if (ptr == NULL || ptr->dialog == NULL)
		return;
	fslx_close(ptr->dialog);
	remove_item((CHAIN_DATA *) ptr);
	g_free(ptr);
	if (modal_items >= 0)
		modal_items--;
}
