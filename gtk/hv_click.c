#include "hv_gtk.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void HypClick(WINDOW_DATA *win, LINK_INFO *info)
{
	GdkModifierType mask;
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	gint x, y;
	
	gdk_display_get_pointer(gtk_widget_get_display(win->hwnd), NULL, &x, &y, &mask);

	if (hypnode_valid(hyp, info->dest_page))
	{
		switch (info->dst_type)
		{
		case HYP_NODE_INTERNAL:
			if ((mask & GDK_CONTROL_MASK) || (info->link_type >= HYP_ESC_ALINK && gl_profile.viewer.alink_newwin))
			{
				char *name = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[info->dest_page]->name, STR0TERM);
				OpenFileInWindow(win, doc->path, name, HYP_NOINDEX, FALSE, 2, FALSE);
				g_free(name);
			} else
			{
				AddHistoryEntry(win, doc);
				GotoPage(win, info->dest_page, info->line_nr, TRUE);
			}
			break;
		case HYP_NODE_POPUP:
			OpenPopup(win, info->dest_page, x, y);
			break;
		case HYP_NODE_EXTERNAL_REF:
			{
				char *name = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[info->dest_page]->name, STR0TERM);
				HypOpenExtRef(win, name, (mask & GDK_CONTROL_MASK) || (info->link_type >= HYP_ESC_ALINK && gl_profile.viewer.alink_newwin));
				g_free(name);
			}
			break;
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_REXX_SCRIPT:
			{
				char *tool = NULL;
				HYP_HOSTNAME *h;
				const char *argv[3];
				
				/* search for host application */
				if (hyp->hostname == NULL)
				{
					show_message(_("Error"), _("No host application defined."), FALSE);
					break;
				}
				for (h = hyp->hostname; h != NULL && tool == NULL; h = h->next)
					if (Profile_ReadString(gl_profile.profile, "TOOLS", h->name, &tool))
						break;
				if (empty(tool))	/* host application found? */
				{
					char *str = g_strdup_printf(_("No application defined to handle %s."), hyp->hostname->name);
					show_message(_("Error"), str, FALSE);
					g_free(str);
					break;		/* ... cancel */
				}

				{
					char *prog = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), hyp->indextable[info->dest_page]->name, STR0TERM, NULL);
					char *dir;
					char *dfn;
					struct stat s;
					
					/* search for file in directory of hypertext */
					dir = hyp_path_get_dirname(hyp->file);
					dfn = g_build_filename(dir, prog, NULL);
					g_free(dir);

					/* can file be located with shel_find()? */
					if (hyp_utf8_stat(dfn, &s) != 0)
					{
						g_free(dfn);
						/* use filename as specified */
						dfn = prog;
						prog = NULL;
					}

					argv[0] = tool;
					argv[1] = dfn;
					argv[2] = 0;
					hyp_utf8_spawnvp(P_NOWAIT, 2, argv);
					g_free(dfn);
					g_free(prog);
					g_free(tool);
				}
			}
			break;
		case HYP_NODE_SYSTEM_ARGUMENT:
			{
				char *prog = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), hyp->indextable[info->dest_page]->name, STR0TERM, NULL);
				char *dir;
				char *dfn;
				struct stat s;
				const char *argv[4];

				/* search for file in directory of hypertext */
				dir = hyp_path_get_dirname(hyp->file);
				dfn = g_build_filename(dir, prog, NULL);
				g_free(dir);
				
				if (hyp_utf8_stat(dfn, &s) != 0)
				{
					g_free(dfn);
					/* use filename as specified */
					dfn = prog;
					prog = NULL;
				}
				
				argv[0] = "/bin/sh";
				argv[1] = "-c";
				argv[2] = dfn;
				argv[3] = 0;
				hyp_utf8_spawnvp(P_WAIT, 3, argv);
				g_free(dfn);
				g_free(prog);
			}
			break;
		case HYP_NODE_QUIT:	/* quit application */
			{
				GSList *l;
				
				for (l = all_list; l; l = l->next)
					SendCloseWindow((WINDOW_DATA *)l->data);
			}
			break;
		case HYP_NODE_CLOSE:	/* close window */
			SendCloseWindow(win);
			break;
		case HYP_NODE_IMAGE:
		case HYP_NODE_EOF:
		default:
			{
				char *str = g_strdup_printf(_("Link to node of type %u not implemented."), info->dest_page);
				show_message(_("Error"), str, FALSE);
				g_free(str);
			}
			break;
		}
	} else
	{
		HYP_DBG(("Link to invalid node %u", info->dest_page));
	}
}
