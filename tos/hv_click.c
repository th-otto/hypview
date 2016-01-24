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
#include "av.h"
#include "tos/mem.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static inline int isword(unsigned char ch)
{
	return isalnum(ch) || ch >= 0x80 || ch == '-';
}

/*** ---------------------------------------------------------------------- ***/

static void get_word(const unsigned char *min, const unsigned char *src, unsigned char *dst)
{
	const unsigned char *ptr = src;

	while (ptr >= min && isword(*ptr))
	{
		ptr--;
	}

	if (ptr != src)
	{
		ptr++;

		do
		{
			*dst++ = *ptr++;
		} while (isword(*ptr));
	} else
		*dst++ = *ptr;
	*dst = 0;
}

/*** ---------------------------------------------------------------------- ***/

void HypClick(WINDOW_DATA *win, EVNTDATA *m)
{
	DOCUMENT *doc = win->data;
	HYP_DOCUMENT *hyp;
	HYP_NODE *node;
	_WORD xy[8];
	short curr_txt_effect = 0;
	char test_mem[2];
	const unsigned char *src;
	const unsigned char *last_esc;
	LINEPTR *line_ptr;
	WP_UNIT x, y;
	WP_UNIT x_pos = font_cw;
	
	hyp = doc->data;

	wind_get_grect(win->whandle, WF_WORKXYWH, &win->work);
	x = m->x + win->docsize.x * win->x_raster - win->work.g_x;
	y = m->y - win->work.g_y + win->y_offset;

	node = doc->displayed_node;

	line_ptr = HypGetYLine(node, y + win->docsize.y * font_ch);
	src = line_ptr ? line_ptr->txt : NULL;

	if (src == NULL)
		return;

	last_esc = src;
	test_mem[1] = 0;
	vst_effects(vdi_handle, curr_txt_effect);

	while (*src)
	{
		if (*src == HYP_ESC)
		{
			src++;
			if (*src == HYP_ESC_ESC)
			{
				*test_mem = HYP_ESC_ESC;
				goto check_char;
			} else if (*src >= HYP_ESC_LINK && *src <= HYP_ESC_ALINK_LINE)
			{
				unsigned char link_type = *src;	/* remember link type */
				hyp_lineno line_nr = 0;	/* line number to go to */
				hyp_nodenr dst_page;	/* index of target node */
				unsigned short link_len;
				char *str;
				
				src++;

				if (link_type == HYP_ESC_LINK_LINE || link_type == HYP_ESC_ALINK_LINE)		/* get line number */
				{
					line_nr = DEC_255(src);
					src += 2;
				}

				dst_page = DEC_255(src);
				src += 2;

				vst_effects(vdi_handle, gl_profile.viewer.link_effect);

				link_len = *src;
				src++;
				
				if (link_len > HYP_STRLEN_OFFSET)
				{
					size_t len = link_len - HYP_STRLEN_OFFSET;
					str = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), src, len, NULL);
					vqt_extent(vdi_handle, str, xy);
					g_free(str);
					src += len;
				} else if (hypnode_valid(hyp, dst_page))
				{
					str = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), hyp->indextable[dst_page]->name, STR0TERM, NULL);
					vqt_extent(vdi_handle, str, xy);
					g_free(str);
				}
				
				vst_effects(vdi_handle, curr_txt_effect);

				x_pos += xy[0] + xy[2];

				if (x_pos > x)
				{
					if (hypnode_valid(hyp, dst_page))
					{
						hyp_indextype dst_type = hyp->indextable[dst_page]->type;
	
						switch (dst_type)
						{
						case HYP_NODE_INTERNAL:
							if ((m->kstate & K_CTRL) || (link_type >= HYP_ESC_ALINK && gl_profile.viewer.alink_newwin))
							{
								char *name = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[dst_page]->name, STR0TERM);
								OpenFileInWindow(win, doc->path, name, HYP_NOINDEX, FALSE, 2, FALSE);
								g_free(name);
							} else
							{
								AddHistoryEntry(win, doc);
								GotoPage(win, dst_page, line_nr, TRUE);
							}
							break;
						case HYP_NODE_POPUP:
							OpenPopup(win, dst_page, x_pos - xy[2], y - y % font_ch);
							break;
						case HYP_NODE_EXTERNAL_REF:
							{
								char *name = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[dst_page]->name, STR0TERM);
								HypOpenExtRef(win, name, (m->kstate & K_CTRL) || (link_type >= HYP_ESC_ALINK && gl_profile.viewer.alink_newwin));
								g_free(name);
							}
							break;
						case HYP_NODE_REXX_COMMAND:
							{
								short dst_id = -1;
								HYP_HOSTNAME *h;
								
								/* search for host application */
								if (hyp->hostname == NULL)
								{
									form_alert(1, rs_string(HV_ERR_NO_HOSTNAME));
									break;
								}
								for (h = hyp->hostname; h != NULL && dst_id < 0; h = h->next)
									dst_id = appl_locate(h->name, FALSE);
								if (dst_id < 0)	/* host application found? */
								{
									form_alert(1, rs_string(HV_ERR_HOST_NOT_FOUND));
									break;		/* ... cancel */
								}
		
								/* use VA_START to send parameter to host application */
								{
									char *prog = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), hyp->indextable[dst_page]->name, STR0TERM, NULL);
									char *dir;
									char *dfn;
		
									/* search for file in directory of hypertext */
									dir = g_path_get_dirname(hyp->file);
									dfn = g_build_filename(dir, prog, NULL);
									g_free(dir);
		
									/* can file be located with shel_find()? */
									if (!shel_find(dfn))
									{
										g_free(dfn);
										/* use filename as specified */
										dfn = prog;
										prog = NULL;
									}
		
									SendVA_START(dst_id, dfn);
									g_free(dfn);
									g_free(prog);
								}
							}
							break;
						case HYP_NODE_SYSTEM_ARGUMENT:
						case HYP_NODE_REXX_SCRIPT:
							/* ask server to start parameter using AV_STARTPROG */
							{
								char *prog = hyp_conv_charset(hyp->comp_charset, hyp_get_current_charset(), hyp->indextable[dst_page]->name, STR0TERM, NULL);
								char *dir;
								char *dfn;
	
								/* search for file in directory of hypertext */
								dir = g_path_get_dirname(hyp->file);
								dfn = g_build_filename(dir, prog, NULL);
								g_free(dir);
								
								/* can file be located with shel_find()? */
								if (!shel_find(dfn))
								{
									g_free(dfn);
									/* use filename as specified */
									dfn = prog;
									prog = NULL;
								}
	
								SendAV_STARTPROG(dfn, NULL);
								g_free(dfn);
								g_free(prog);
							}
							break;
						case HYP_NODE_QUIT:	/* quit application */
							if (_app)
								doneFlag = TRUE;
							else
								RemoveItems();
							break;
						case HYP_NODE_CLOSE:	/* close window */
							SendCloseWindow(win);
							break;
						case HYP_NODE_IMAGE:
						case HYP_NODE_EOF:
						default:
							form_alert(1, rs_string(HV_ERR_NOT_IMPLEMENTED));
							break;
						}
					} else
					{
						HYP_DBG(("Link to invalid node %u", dst_page));
					}
					break;
				}
			} else
			{
				if (HYP_ESC_IS_TEXATTR(*src))	/* text attributes */
				{
					curr_txt_effect = *src - HYP_ESC_TEXTATTR_FIRST;
					vst_effects(vdi_handle, curr_txt_effect);
					src++;
				} else
				{
					src = hyp_skip_esc(src - 1);
				}
			}
			last_esc = src;
		} else
		{
			*test_mem = *src;
		  check_char:
			vqt_extentn(vdi_handle, test_mem, 1, xy);
			x_pos += xy[0] + xy[2];
			src++;
		}

		if (x_pos > x)
		{
			if (!gl_profile.viewer.refonly)
			{
				hyp_nodenr ret;
				unsigned char buffer[256];
				char *name;
				
				get_word(last_esc, src - 1, buffer);
				name = hyp_conv_to_utf8(hyp_get_current_charset(), buffer, STR0TERM);
				ret = HypFindNode(doc, name);
				if (ret != HYP_NOINDEX)
				{
					if (doc->gotoNodeProc(win, NULL, ret))
						ReInitWindow(win);
				} else
				{
					search_allref(win, name, TRUE);
				}
				g_free(name);
			}
			break;
		}
	}
}
