/*****************************************************************************
 * bghio.c
 *****************************************************************************/

#include "config.h"
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

BGHINFO *bgh_dup(BGHINFO *old)
{
	BGHINFO *bgh;
	BGHENTRY *entry;
	
	if (old == NULL)
		return NULL;
	bgh = bgh_new(old->type, old->idx);
	if (bgh != NULL)
	{
		FOR_ALL_BGH(old, entry)
		{
			bgh_new_entry(bgh, entry->idx, entry->cmnt);
		}
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

/*** ---------------------------------------------------------------------- ***/

BGHENTRY *bgh_idx(BGHINFO *bgh, _WORD idx)
{
	BGHENTRY *entry;
	
	if (bgh != NULL)
	{
		FOR_ALL_BGH(bgh, entry)
		{
			if (idx == 0)
				return entry;
			--idx;
		}
	}
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL bgh_split_cmnt(cstringarray cmnt, stringarray *pcmnt, stringarray *pbgh)
{
	char *dst;
	const char *src;
	
	*pcmnt = NULL;
	*pbgh = NULL;
	src = cmnt;
	if (src == NULL || *src != '[')
	{
		return FALSE;
	}
	*pcmnt = g_strdup(src);
	*pbgh = g_strdup(src);
	if (*pcmnt == NULL || *pbgh == NULL)
	{
		return FALSE;
	}
	src++;
	dst = *pcmnt;
	while (*src != '\0' && *src != ']')
	{
		*dst++ = *src++;
	}
	if (*src++ != ']')
	{
		g_free(*pbgh);
		g_free(*pcmnt);
		return FALSE;
	}
	*dst++ = '\0';
	*dst = '\0';
	
	dst = *pbgh;
	while (*src != '\0')
	{
		if (*src++ != '[')
		{
			g_free(*pbgh);
			g_free(*pcmnt);
			return FALSE;
		}
		if (*src == ']')
			*dst++ = ' ';
		while (*src != '\0' && *src != ']')
		{
			*dst++ = *src++;
		}
		if (*src++ != ']')
		{
			g_free(*pbgh);
			g_free(*pcmnt);
			return FALSE;
		}
		*dst++ = '\0';
	}
	*dst = '\0';
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL bgh_bgh(cstringarray bgh, _WORD idx, _BOOL all)
{
	cstringarray p;

	if ((p = bgh) == NULL)
		return TRUE;
	while (*p != '\0')
	{
		output2("# %03d %s\n", idx, p);
		if (!all)
			break;
		while (*p++ != '\0')
			;
		idx++;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL bgh_ob_object(RSCFILE *file, RSCTREE *rtree, OBJECT *tree, _WORD parent, _BOOL *first)
{
	_WORD ob = parent == NIL ? ROOT : tree[parent].ob_head;
	
	do {
		if (tree[ob].ob_head != NIL)
		{
			if (bgh_ob_object(file, rtree, tree, ob, first) == FALSE)
				return FALSE;
		}
		ob = tree[ob].ob_next;
	} while (ob != parent);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL bgh_bghinfo(BGHINFO *bgh)
{
	BGHENTRY *entry;
	
	if (bgh == NULL)
		return TRUE;
	FOR_ALL_BGH(bgh, entry)
	{
		bgh_bgh(entry->cmnt, entry->idx, FALSE);
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL bgh_trees(RSCFILE *file)
{
	RSCTREE *tree;
	OBJECT *ob;
	
	/*
	 * The sample source supplied with RSM relies on the More
	 * section being present before any references to it appear.
	 */
	FOR_ALL_RSC(file, tree)
	{
		switch (tree->rt_type)
		{
		case RT_BUBBLEMORE:
		case RT_BUBBLEUSER:
			output3("#%s %03ld %s\n", tree->rt_type == RT_BUBBLEMORE ? "More" : "User", tree->rt_index, fixnull(tree->rt_cmnt));
			bgh_bghinfo(tree->rt_objects.bgh);
			break;
		}
	}
	
	/*
	 * no do everything else
	 */
	FOR_ALL_RSC(file, tree)
	{
		switch (tree->rt_type)
		{
		case RT_DIALOG:
		case RT_FREE:
		case RT_UNKNOWN:
		case RT_MENU:
			if (tree->rt_type == RT_MENU)
				ob = tree->rt_objects.menu.mn_tree;
			else
				ob = tree->rt_objects.dial.di_tree;
			if (ob != NULL)
			{
				_BOOL first = TRUE;
				
				if (bgh_ob_object(file, tree, ob, NIL, &first) == FALSE)
					return FALSE;
			}
			break;
		case RT_FRSTR:
		case RT_ALERT:
			if (tree->rt_objects.alert.al_bgh != NULL)
			{
				output2("#Alert %03ld %s\n", tree->rt_index, fixnull(tree->rt_cmnt));
				bgh_bghinfo(tree->rt_objects.alert.al_bgh);
			}
			break;
		case RT_FRIMG:
		case RT_MOUSE:
			/* nothing to do */
			break;
		case RT_BUBBLEMORE:
		case RT_BUBBLEUSER:
			/* already done above */
			break;
		}
	}

	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL bgh_output_source_file(RSCFILE *file)
{
	rsc_tree_count(file);
	rsc_count_all(file);
	outstr("#BGH 000 000 000\n");
	outstr("\n");
	output(" BubbleGEM Help-File by ORCS %s for ResourceMaster/ORCS\n", program_version);
	outstr("\n");
	bgh_trees(file);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL rsc_bgh_source(RSCFILE *file, rsc_counter *counter, char *filename, char *buf)
{
	XRS_HEADER xrsc_header;
	_BOOL ok;

	UNUSED(counter);
	xrsc_get_header(&xrsc_header, buf);
	if (file_create(filename, "w") == FALSE)
		return FALSE;
	ok = bgh_output_source_file(file);
	return file_close(ok);
}

