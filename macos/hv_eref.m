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
							HypOpenExtRef(win, name, 0, FALSE);
							g_free(name);
							break;
						case HYP_NODE_INTERNAL:
							AddHistoryEntry(win, doc);
							GotoPage(win, dest_page, 0, TRUE);
							break;
						case HYP_NODE_POPUP:
							OpenPopup(win, dest_page, 0, -1, -1);
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
	NSMenu *menu;
	NSMenuItem *item;
	const unsigned char *pos;
	int i;
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	const unsigned char *end;
	struct popup_pos popup_pos;
	int x, y;
	int sel;
	NSEvent *event;
	
	if (win->m_button_hidden[TO_REFERENCES])
		return;
	if (win->m_button_disabled[TO_REFERENCES])
		return;
	
	menu = [[[NSMenu alloc] init] autorelease];
	[menu setAutoenablesItems:NO];
	
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
			item = [menu addItemWithTitle:[[[NSString alloc] initWithCString:text encoding:NSUTF8StringEncoding] autorelease] action:NULL keyEquivalent:@""];
			[item setTag:i];
			
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
		[menu release];
		return;
	}
	
	popup_pos.window = win;
	popup_pos.obj = TO_REFERENCES;
	if (position_popup(menu, &popup_pos, &x, &y) == FALSE)
	{
		[menu release];
		return;
	}
	[win makeKeyAndOrderFront: win];
	event = [NSEvent mouseEventWithType: NSEventTypeLeftMouseDown
		location: NSMakePoint(x, y)
		modifierFlags: 0
		timestamp: [[NSProcessInfo processInfo] systemUptime]
		windowNumber: [win windowNumber]
		context: nil
		eventNumber: 0
		clickCount: 1
		pressure: 0];
	[NSMenu popUpContextMenu:menu withEvent:event forView: win->textview];
	/* TODO */
	sel = -1;
	[menu release];
	xref_selected(win, sel);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Open an external reference <name>.
 * External references are stored as <file.hyp>/<nodename>.
 * If it is an absolute filename, nodename will be considered
 * as part of the filename.
 */
void HypOpenExtRef(WINDOW_DATA *win, const char *name, hyp_lineno line_no, gboolean new_window)
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
	if (cptr != NULL && cptr == &temp[1])
	{
		cptr++;
		if (G_IS_DIR_SEPARATOR(*cptr))
			cptr++;
		memmove(temp, cptr, strlen(cptr) + 1);
	}
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
			/*
			 * older compiler versions apparently accepted links to external HYP files
			 * without a /nodename, handle this too
			 */
			if (hyp_guess_filetype(path) == HYP_FT_HYP)
				win = OpenFileInWindow(win, path, NULL, 0, TRUE, new_window ? FORCE_NEW_WINDOW : 0, FALSE);
			else
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
		if (hyp_guess_filetype(path) == HYP_FT_RSC)
		{
			ShowResource(win, path, line_no);
		} else
		{
			win = OpenFileInWindow(win, path, chapter, 0, TRUE, new_window ? FORCE_NEW_WINDOW : 0, FALSE);
		}
	}
	g_free(temp);
}
