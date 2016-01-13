#include "hv_gtk.h"
#include "hypdebug.h"

/*----------------------------------------------------------------------------------*/

static void position_popup(GtkMenu *menu, gint *xret, gint *yret, gboolean *push_in, void *data)
{
	int x, y;
	int wx, wy;
	DOCUMENT *doc = (DOCUMENT *)data;
	WINDOW_DATA *win = doc->window;
	
	UNUSED(menu);
	gtk_widget_translate_coordinates(win->m_buttons[TO_REFERENCES], win->hwnd, 0, 0, &x, &y);
	gdk_window_get_origin(gtk_widget_get_window(win->hwnd), &wx, &wy);
	*xret = x + wx;
	*yret = y + wy;
	*push_in = TRUE;
}

/*----------------------------------------------------------------------------------*/

void HypExtRefPopup(DOCUMENT *doc, int button)
{
	GtkWidget *menu;
	const unsigned char *pos;
	int i, sel;
	HYP_DOCUMENT *hyp;
	WINDOW_DATA *win = doc->window;
	const unsigned char *end;
	GdkModifierType mask;
	
	hyp = doc->data;
	menu = gtk_menu_new();
	
	i = 0;
	pos = doc->displayed_node->start;
	end = doc->displayed_node->end;
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
			gtk_menu_append(menu, gtk_label_new(text));
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
		return;
	}
	
	sel = -1;
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, position_popup, doc, button, gtk_get_current_event_time());
	gdk_display_get_pointer(gtk_widget_get_display(win->hwnd), NULL, NULL, NULL, &mask);
	gtk_widget_unref(menu);
	
	if (sel > 0)
	{
		pos = doc->displayed_node->start;
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
							HypOpenExtRef(doc->window, name, FALSE);
							g_free(name);
							break;
						case HYP_NODE_INTERNAL:
							AddHistoryEntry(doc->window);
							GotoPage(doc, dest_page, 0, TRUE);
							break;
						case HYP_NODE_POPUP:
							OpenPopup(doc, dest_page, -1, -1);
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
		nf_debugprintf("openext: selection %d not found\n", sel);
	}
}


/*
 * Open an external reference <name>.
 * External references are stored as <file.hyp>/<nodename>.
 * If it is an absolute filename, nodename will be considered
 * as part of the filename.
 */
void HypOpenExtRef(void *ptr, const char *name, gboolean new_window)
{
	char *cptr;
	char *temp;
	const char *path, *chapter;
	WINDOW_DATA *win = (WINDOW_DATA *)ptr;
	
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
			if (doc->gotoNodeProc(doc, NULL, ret))
				ReInitWindow(doc);
		} else
		{
			search_allref(win, chapter, FALSE);
		}
	} else
	{
		chapter = cptr;
		if (new_window)
			win = OpenFileNewWindow(path, chapter, HYP_NOINDEX, FALSE);
		else
			win = OpenFileSameWindow(win, path, chapter, FALSE, FALSE);
	}
	g_free(temp);
}
