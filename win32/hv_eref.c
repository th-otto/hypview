#include "hv_defs.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void xref_selected(WINDOW_DATA *win, int sel)
{
	DOCUMENT *doc = win->data;
	int i;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	const unsigned char *pos;
	const unsigned char *end;

	if (sel >= 0)
	{
		pos = win->displayed_node->start;
		end = win->displayed_node->end;
		i = 0;
		while (pos < end && *pos == HYP_ESC)
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
							OpenPopup(win, dest_page, -1, -1);
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
				i++;
			}
			pos = hyp_skip_esc(pos);
		}
		HYP_DBG(("openext: selection %d not found", sel));
	}
}

/*** ---------------------------------------------------------------------- ***/

void HypExtRefPopup(WINDOW_DATA *win, int button)
{
	HMENU menu;
	const unsigned char *pos;
	int i;
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	const unsigned char *end;
	struct popup_pos popup_pos;
	int x, y;
	
	UNUSED(button);
	if (!win->m_buttons[TO_REFERENCES])
		return;
	
	menu = CreateMenu();
	
	i = 0;
	pos = win->displayed_node->start;
	end = win->displayed_node->end;
	while (pos < end && *pos == HYP_ESC)
	{
		if (pos[1] == HYP_ESC_EXTERNAL_REFS)
		{
			hyp_nodenr dest_page;
			char *text;
			char *dest;
			gboolean converror = FALSE;
			size_t namelen;
			
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
			g_free(text);
			i++;
		}
		pos = hyp_skip_esc(pos);
	}
	
	if (i == 0)
	{
		/*
		 * no extrefs found? we should not get here,
		 * the toolbar entry should not have been selectable
		 */
		DestroyMenu(menu);
		return;
	}
	
	popup_pos.window = win;
	popup_pos.obj = TO_REFERENCES;
	position_popup(menu, &popup_pos, &x, &y);
	DestroyMenu(menu);
	xref_selected(win, -1);
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
	   colon as 2nd char => absolute path
	   else => no Pfad, don't have chapter name either
	 */
	cptr = strchr(temp, ':');
	if (cptr == NULL || cptr == &temp[1])
	{
		cptr = strslash(temp);
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
				ReInitWindow(win, FALSE);
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
		win = OpenFileInWindow(win, path, chapter, HYP_NOINDEX, TRUE, new_window ? FORCE_NEW_WINDOW : 0, FALSE);
	}
	g_free(temp);
}
