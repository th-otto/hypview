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

void HypClick(WINDOW_DATA *win, LINK_INFO *info)
{
	GdkModifierType mask;
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
	gint x, y;
	
	gdk_display_get_pointer(gtk_widget_get_display(GTK_WIDGET(win)), NULL, &x, &y, &mask);

	if (hypnode_valid(hyp, info->dest_page))
	{
		switch (info->dst_type)
		{
		case HYP_NODE_INTERNAL:
			if ((mask & GDK_CONTROL_MASK) || (info->link_type >= HYP_ESC_ALINK && gl_profile.viewer.alink_newwin))
			{
				char *name = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[info->dest_page]->name, STR0TERM);
				OpenFileInWindow(win, doc->path, name, HYP_NOINDEX, FALSE, FORCE_NEW_WINDOW, FALSE);
				g_free(name);
			} else
			{
				AddHistoryEntry(win, doc);
				GotoPage(win, info->dest_page, info->line_nr, TRUE);
			}
			break;
		case HYP_NODE_POPUP:
			if (!win->is_popup)
				OpenPopup(win, info->dest_page, 0, x, y);
			break;
		case HYP_NODE_EXTERNAL_REF:
			{
				char *name = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[info->dest_page]->name, STR0TERM);
				HypOpenExtRef(win, name, info->line_nr, (mask & GDK_CONTROL_MASK) || (info->link_type >= HYP_ESC_ALINK && gl_profile.viewer.alink_newwin));
				g_free(name);
			}
			break;
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_REXX_SCRIPT:
			{
				char *tool = NULL;
				HYP_HOSTNAME *h;
				const char *argv[3];
				char *prog = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), hyp->indextable[info->dest_page]->name, STR0TERM, NULL);
				
				if (is_weblink(prog))
				{
					argv[0] = "xdg-open";
					argv[1] = prog;
					argv[2] = 0;
					hyp_utf8_spawnvp(P_WAIT, 2, argv);
				} else
				{
					/* search for host application */
					if (hyp->hostname == NULL)
					{
						show_message(GTK_WIDGET(win), _("Error"), _("No host application defined."), FALSE);
					} else
					{
						for (h = hyp->hostname; h != NULL && tool == NULL; h = h->next)
							if (Profile_ReadString(gl_profile.profile, "TOOLS", h->name, &tool))
								break;
						if (empty(tool))	/* host application found? */
						{
							char *str = g_strdup_printf(_("No application defined to handle %s."), hyp->hostname->name);
							show_message(GTK_WIDGET(win), _("Error"), str, FALSE);
							g_free(str);
						} else
						{
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
							g_free(tool);
						}
					}
				}
				g_free(prog);
			}
			break;
		case HYP_NODE_SYSTEM_ARGUMENT:
			{
				char *prog = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), hyp->indextable[info->dest_page]->name, STR0TERM, NULL);
				const char *argv[4];

				if (is_weblink(prog))
				{
					argv[0] = "xdg-open";
					argv[1] = prog;
					argv[2] = 0;
					hyp_utf8_spawnvp(P_WAIT, 2, argv);
				} else
				{
					struct stat s;
					/* search for file in directory of hypertext */
					char *dir = hyp_path_get_dirname(hyp->file);
					char *dfn = g_build_filename(dir, prog, NULL);
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
				}
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
				char *str = g_strdup_printf(_("Link to node of type %u not implemented."), info->dst_type);
				show_message(GTK_WIDGET(win), _("Error"), str, FALSE);
				g_free(str);
			}
			break;
		}
	} else
	{
		HYP_DBG(("Link to invalid node %u", info->dest_page));
	}
}
