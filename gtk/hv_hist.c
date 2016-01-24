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
	new_entry->node = doc->getNodeProc(doc);
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
