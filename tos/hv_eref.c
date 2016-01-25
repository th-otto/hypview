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

void HypExtRefPopup(WINDOW_DATA *win, short x, short y)
{
	DOCUMENT *doc = win->data;
	OBJECT *tree = rs_tree(EMPTYPOPUP);
	const unsigned char *pos;
	short i, sel, h;
	size_t len = 0;
	HYP_DOCUMENT *hyp;
	const unsigned char *end;
	static char eos = 0;
	
	hyp = doc->data;

	i = tree[ROOT].ob_head;
	pos = win->displayed_node->start;
	end = win->displayed_node->end;
	h = 0;
	while (pos < end && *pos == HYP_ESC)
	{
		if (pos[1] == HYP_ESC_EXTERNAL_REFS)
		{
			hyp_nodenr dest_page;
			char *text;
			char *dest;
			gboolean converror = FALSE;
			size_t namelen;
			
			if (i == ROOT)
			{
				/* FIXME: warn that there are more extrefs than fit in popup */
				break;
			}
			dest_page = DEC_255(&pos[3]);
			if (hypnode_valid(hyp, dest_page))
			{
				INDEX_ENTRY *entry = hyp->indextable[dest_page];
				namelen = entry->length - SIZEOF_INDEX_ENTRY;
				dest = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), entry->name, namelen, &converror);
			} else
			{
				dest = g_strdup_printf(_("<invalid destination page %u>"), dest_page);
			}
			text = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), pos + 5, max(pos[2], 5u) - 5u, &converror);
			if (empty(text) || strcmp(dest, text) == 0)
			{
				g_free(text);
				text = dest;
			} else
			{
				g_free(dest);
			}
			tree[i].ob_flags = OF_SELECTABLE;
			tree[i].ob_spec.free_string = text;
			len = max(strlen(text), len);
			h = max(h, tree[i].ob_y + tree[i].ob_height);
			i = tree[i].ob_next;
		}
		pos = hyp_skip_esc(pos);
	}
	
	if (h == 0)
	{
		/*
		 * no extrefs found? we should not get here,
		 * the toolbar entry should not have been selectable
		 */
		return;
	}
	
	/* Hide unused entries */
	while (i != ROOT)
	{
		tree[i].ob_flags = OF_HIDETREE;
		tree[i].ob_spec.free_string = &eos;
		i = tree[i].ob_next;
	}

	len = len * pwchar;
	tree[EM_BACK].ob_x = x;
	tree[EM_BACK].ob_y = y;
	tree[EM_BACK].ob_width = (short) len;
	tree[EM_BACK].ob_height = h;

	/* set same width for all entries */
	for (i = tree[ROOT].ob_head; i != ROOT; i = tree[i].ob_next)
	{
		tree[i].ob_width = (short) len;
		tree[i].ob_height = phchar;
	}

	sel = popup_select(tree, 0, 0);

	/*
	 * Free any previously alloced strings, taking care
	 * not to try to free() strings coming from the RSC file.
	 * In the RSC, they are empty, and we dont allocate empty strings
	 */
	for (i = tree[ROOT].ob_head; i != ROOT; i = tree[i].ob_next)
	{
		if (!empty(tree[i].ob_spec.free_string))
		{
			g_free(tree[i].ob_spec.free_string);
		}
		tree[i].ob_flags = OF_SELECTABLE;
		tree[i].ob_spec.free_string = NULL;
	}
	
	if (sel > 0)
	{
		pos = win->displayed_node->start;
		i = tree[ROOT].ob_head;
		while (pos < end && *pos == HYP_ESC && i != ROOT)
		{
			if (pos[1] == HYP_ESC_EXTERNAL_REFS)
			{
				if (sel == i)
				{
					INDEX_ENTRY *entry;
					hyp_nodenr dest_page;
					char *name;
					
					dest_page = DEC_255(&pos[3]);
					if (hypnode_valid(hyp, dest_page))
					{
						entry = hyp->indextable[dest_page];
						
						switch (entry->type)
						{
						case HYP_NODE_EXTERNAL_REF:
							name = hyp_conv_to_utf8(hyp->comp_charset, entry->name, STR0TERM);
							HypOpenExtRef(win, name, FALSE);
							g_free(name);
							break;
						case HYP_NODE_INTERNAL:
							AddHistoryEntry(win, doc);
							GotoPage(win, dest_page, 0, TRUE);
							break;
						case HYP_NODE_POPUP:
							OpenPopup(win, dest_page, 0, 0);
							break;
						default:
							HYP_DBG(("Illegal External reference!"));
							break;
						}
					} else
					{
						HYP_DBG(("reference to invalid destination page %u", dest_page));
					}
					return;
				}
				i = tree[i].ob_next;
			}
			pos = hyp_skip_esc(pos);
		}
		nf_debugprintf("openext: selection %d not found\n", sel);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Open an external reference <name>.
 * External references are stored as <file.hyp>/<nodename>.
 * If it is an absolute filename, nodename will be considered
 * as part of the filename.
 */
void HypOpenExtRef(WINDOW_DATA *win, const char *name, gboolean new_window)
{
	char *cptr;
	char *temp;
	const char *path, *chapter;
	
	if (empty(name))
		return;
	
	temp = g_strdup(name);

	path = temp;
	/*
	   No colon in name? => relative path
	   colon as 2nd char => absolute Pfad
	   else => kein Pfad, also auch kein Kapitelname suchen
	 */
	cptr = strchr(temp, ':');
	if (cptr == NULL || cptr == &temp[1])
	{
		cptr = strchr(temp, '/');
		if (cptr)
			*cptr++ = 0;
	}
	/*
	 * if we only have the first part,
	 * then that was actually a chapter name
	 */
	if (cptr == NULL)
	{
		hyp_nodenr ret;
		DOCUMENT *doc = (DOCUMENT *)win->data;
		
		chapter = path;
		ret = HypFindNode(doc, name);
		if (ret != HYP_NOINDEX)
		{
			if (doc->gotoNodeProc(win, NULL, ret))
				ReInitWindow(win);
		} else
		{
			win = search_allref(win, chapter, FALSE);
		}
	} else
	{
		chapter = cptr;
		/*
		 * "Main" from external references always means default page,
		 * regardless of the actual name
		 */
		if (strcmp(chapter, hyp_default_main_node_name) == 0)
			chapter = NULL;
		win = OpenFileInWindow(win, path, chapter, HYP_NOINDEX, FALSE, new_window ? 2 : 0, FALSE);
	}
	g_free(temp);
}
