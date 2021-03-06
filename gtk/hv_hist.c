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

#include "hv_gtk.h"
#include "hypdebug.h"


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void AddHistoryEntry(WINDOW_DATA *win, DOCUMENT *doc)
{
	HISTORY *new_entry;

	if (win == NULL || doc == NULL)
		return;
	new_entry = g_new0(HISTORY, 1);

	if (new_entry == NULL)
	{
		HYP_DBG(("ERROR: out of memory => can't add new history entry"));
		return;
	}
	new_entry->doc = hypdoc_ref(doc);
	new_entry->node = doc->getNodeProc(win);
	new_entry->line = hv_win_topline(win);
	new_entry->next = win->history;
	new_entry->title = g_strdup(win->title);

	/* put new entry at top */
	win->history = new_entry;
}

/*** ---------------------------------------------------------------------- ***/

DOCUMENT *RemoveHistoryEntry(WINDOW_DATA *win, hyp_nodenr *node, long *line)
{
	HISTORY *entry = win->history;
	DOCUMENT *doc;
	
	if (entry == NULL)
		return NULL;

	win->history = entry->next;

	*node = entry->node;
	*line = entry->line;
	doc = entry->doc;

	g_free(entry->title);
	g_free(entry);
	return doc;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * remove all entries connected to <win>
 */
void RemoveAllHistoryEntries(WINDOW_DATA *win)
{
	hyp_nodenr node;
	long line;
	DOCUMENT *doc;
	
	while (win->history)
	{
		doc = RemoveHistoryEntry(win, &node, &line);
		ASSERT(doc);
		hypdoc_unref(doc);
	}	
}
