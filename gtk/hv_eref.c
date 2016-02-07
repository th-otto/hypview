#include "hv_gtk.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void xref_selected(GtkWidget *w, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	void *psel = g_object_get_data(G_OBJECT(w), "item-num");
	int sel = (int)(intptr_t)psel;
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

void HypExtRefPopup(WINDOW_DATA *win, int button, guint32 event_time)
{
	GtkWidget *menu;
	const unsigned char *pos;
	int i;
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	const unsigned char *end;
	struct popup_pos popup_pos;
	
	if (!win->m_buttons[TO_REFERENCES])
		return;
	
	menu = gtk_menu_new();
	if (g_object_is_floating(menu))
		g_object_ref_sink(menu);
	
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
			GtkWidget *item;
			
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
			item = gtk_menu_item_new_with_label(text);
			g_object_set_data(G_OBJECT(item), "item-num", (void *)(intptr_t)i);
			g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(xref_selected), win);
			gtk_widget_show(item);
			gtk_menu_append(menu, item);
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
		gtk_widget_unref(menu);
		return;
	}
	
	popup_pos.window = win;
	popup_pos.obj = TO_REFERENCES;
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, position_popup, &popup_pos, button, event_time);
	gtk_widget_unref(menu);
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
		win = OpenFileInWindow(win, path, chapter, HYP_NOINDEX, TRUE, new_window ? 2 : 0, FALSE);
	}
	g_free(temp);
}
