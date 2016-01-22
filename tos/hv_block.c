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
#include "hypdebug.h"
#include "hypview.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void BlockOperation(DOCUMENT *doc, short num)
{
	WINDOW_DATA *win = doc->window;

	switch (num)
	{
	case CO_SAVE:
		SelectFileSave(doc);
		break;
	case CO_BACK:
		GoBack(doc);
		break;
	case CO_COPY:
		BlockCopy(doc);
		break;
	case CO_PASTE:
		BlockPaste(win, gl_profile.viewer.clipbrd_new_window);
		break;
	case CO_SELECT_ALL:
		SelectAll(doc);
		break;
	case CO_SEARCH:
		Hypfind(doc);
		break;
	case CO_SEARCH_AGAIN:
		/* NYI */
		break;
	case CO_DELETE_STACK:
		RemoveAllHistoryEntries(win);
		ToolbarUpdate(doc, TRUE);
		break;
	case CO_SWITCH_FONT:
		SwitchFont(doc);
		break;
	case CO_SELECT_FONT:
		SelectFont(doc);
		break;
	case CO_REMARKER:
		form_alert(1, rs_string(HV_ERR_NOT_IMPLEMENTED));
		break;
	case CO_PRINT:
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void BlockSelectAll(DOCUMENT *doc, BLOCK *b)
{
	b->start.line = 0;
	b->start.y = 0;
	b->start.offset = 0;
	b->start.x = 0;
	b->end.line = doc->lines;
	b->end.y = doc->height;
	b->end.offset = 0;
	b->end.x = 0;
	b->valid = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void BlockCopy(DOCUMENT *doc)
{
	char *scrap_file;
	BLOCK b = doc->selection;

	if (!b.valid)
		BlockSelectAll(doc, &b);

	/* copy to clipboard */
	graf_mouse(BUSY_BEE, NULL);
	if ((scrap_file = GetScrapPath(TRUE)) == NULL)
	{
		HYP_DBG(("No clipboard defined"));
	} else
	{
		_WORD msg[8] = { SC_CHANGED, 0, 0, 2, 0x2e54 /*'.T' */ , 0x5854 /*'XT' */ , 0, 0 };
		BlockAsciiSave(doc, scrap_file);
		msg[1] = gl_apid;
		Protokoll_Broadcast(msg, FALSE);
		g_free(scrap_file);
	}
	graf_mouse(ARROW, NULL);
}

/*** ---------------------------------------------------------------------- ***/

void BlockPaste(WINDOW_DATA *win, gboolean new_window)
{
	char *scrap_file;

	/* "Paste"-action loads SCRAP.TXT from clipboard */
	if ((scrap_file = GetScrapPath(FALSE)) == NULL)
	{
		HYP_DBG(("No clipboard defined"));
	} else
	{
		int ret;

		ret = open(scrap_file, O_RDONLY);
		if (ret >= 0)
		{
			close((short) ret);
			if (new_window)
				win = NULL;
			OpenFileInWindow(win, scrap_file, NULL, HYP_NOINDEX, FALSE, new_window, FALSE);
		}
		g_free(scrap_file);
	}
}

/*** ---------------------------------------------------------------------- ***/

void BlockAsciiSave(DOCUMENT *doc, const char *path)
{
	int handle;

	if (doc->blockProc == NULL)
	{
		Cconout(7);						/* Bing!!!! */
		return;
	}

	/* path is from fileselector here and already in local encoding */
	handle = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0644);
	if (handle < 0)
		FileErrorErrno(path);
	else
	{
		BLOCK b = doc->selection;

		if (!b.valid)					/* no block selected? */
			BlockSelectAll(doc, &b);

		doc->blockProc(doc, BLK_ASCIISAVE, &b, &handle);
		close(handle);
	}
}

/*** ---------------------------------------------------------------------- ***/

char *GetScrapPath(gboolean clear)
{
	char *ptr;
	char scrap_path[DL_PATHMAX];

	if (!scrp_read(scrap_path))
		return NULL;

	if (clear)							/* empty clipboard? */
	{
		if (!scrp_clear())				/* scrp_clear() available? */
		{
			DIR *dir;
			struct dirent *entry;
			
			/* open directory and remove all "SCRAP.*" files */
			dir = opendir(scrap_path);
			if (dir != NULL)
			{
				while ((entry = readdir(dir)) != NULL)
				{
					if (strncasecmp(entry->d_name, "SCRAP.", 6) == 0)
					{
						ptr = g_build_filename(scrap_path, entry->d_name, NULL);
						Fdelete(ptr);
						g_free(ptr);
					}
				}
				closedir(dir);
			}
		}
	}

	return g_build_filename(scrap_path, "SCRAP.TXT", NULL);
}
