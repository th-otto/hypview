/*****************************************************************************
 * bghio.c
 *****************************************************************************/

#include "config.h"
#include "windows_.h"
#include <stdint.h>
#include <gem.h>
#include <mobject.h>
#include <xrsrc.h>
#include <rsc.h>
#include <ro_mem.h>
#include "fileio.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * like strdup, but append an additional '\0' for use as string array
 */
char *g_strdup0(const char *str)
{
	char *p;
	size_t len;
	
	if (str == NULL)
		return NULL;
	len = strlen(str);
	if ((p = g_new(char, len + 2)) != NULL)
	{
		strcpy(p, str);
		p[len + 1] = '\0';
	}
	return p;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void bgh_delete(BGHINFO *bgh)
{
	BGHENTRY *entry, *next;
	
	if (bgh == NULL)
		return;
	for (entry = bgh->head.next; entry != &bgh->head; entry = next)
	{
		next = entry->next;
		g_free(entry->cmnt);
		g_free(entry);
	}
	g_free(bgh);
}

/*** ---------------------------------------------------------------------- ***/

BGHINFO *bgh_new(_WORD type, _WORD idx)
{
	BGHINFO *bgh;
	
	bgh = g_new0(BGHINFO, 1);
	if (bgh != NULL)
	{
		bgh->type = type;
		bgh->idx = idx;
		bgh->head.next = bgh->head.prev = &bgh->head;
	}
	return bgh;
}

/*** ---------------------------------------------------------------------- ***/

BGHENTRY *bgh_new_entry(BGHINFO *bgh, _WORD idx, const char *cmnt)
{
	BGHENTRY *entry;
	
	entry = g_new0(BGHENTRY, 1);
	if (entry != NULL)
	{
		entry->next = &bgh->head;
		entry->prev = bgh->head.prev;
		entry->prev->next = entry;
		entry->next->prev = entry;
		entry->idx = idx;
		entry->cmnt = g_strdup0(cmnt);
	}
	return entry;
}

/*** ---------------------------------------------------------------------- ***/

_WORD bgh_count(BGHINFO *bgh)
{
	_WORD count = 0;
	BGHENTRY *entry;
	
	if (bgh != NULL)
	{
		FOR_ALL_BGH(bgh, entry)
			count++;
	}
	return count;
}
