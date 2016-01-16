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

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void ApplyFont(void)
{
	WINDOW_DATA *win;
	DOCUMENT *doc;
	_WORD font_w, font_h;

	vst_font(vdi_handle, sel_font_id);
	vst_point(vdi_handle, sel_font_pt, &font_w, &font_h, &font_cw, &font_ch);

	if (ProportionalFont(&font_w))
	{
		font_cw = font_w;
	}

	/* adjust all open documents and windows */
	win = (WINDOW_DATA *) all_list;

	while (win)
	{
		if (win->type == WIN_WINDOW)
		{
			gboolean ret;
			
			doc = win->data;
			/* reload page or file */
			graf_mouse(BUSY_BEE, NULL);

			if (doc->type == HYP_FT_HYP)
			{
				hyp_nodenr node = doc->getNodeProc(doc);

				/* remove all nodes from cache */
				RemoveNodes(doc->data);

				/* reload page */
				ret = doc->gotoNodeProc(doc, NULL, node);
			} else
			{
				/* reload file */
				ret = doc->gotoNodeProc(doc, NULL, doc->getNodeProc(doc));
			}
			
			if (ret)
			{
				doc->start_line = win->docsize.y;

				/* forget about "fulled" state */
				win->status &= ~WIS_FULL;

				/* re-init window*/
				ReInitWindow(doc);
			}
			graf_mouse(ARROW, NULL);
		}
		win = win->next;
	}
}

/*** ---------------------------------------------------------------------- ***/

static short FontSelected(FONTSEL_DATA *ptr)
{
	if (ptr->button == FNTS_OK || ptr->button == FNTS_SET)
	{
		sel_font_id = ptr->id;
		sel_font_pt = fix31_to_point(ptr->pt);
		ApplyFont();
	}
	if (ptr->button == FNTS_CANCEL || ptr->button == FNTS_OK)
	{
		/* close the selector */
		return FALSE;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void SelectFont(DOCUMENT *doc)
{
	FONTSEL_DATA *ptr;
	
	(void) doc;
	ptr = (FONTSEL_DATA *)find_ptr_by_type(WIN_FONTSEL);
	if (ptr == NULL)
	{
		ptr = CreateFontselector(FontSelected, FNTS_ALL, "The quick brown...", NULL);
		if (ptr)
			OpenFontselector(ptr, FNTS_SNAME|FNTS_SSIZE|FNTS_CHNAME|FNTS_CHSIZE|FNTS_BSET, sel_font_id, point_to_fix31(sel_font_pt), 0);
	} else
	{
		wind_set_int(ptr->whandle, WF_TOP, 0);
	}
}

/*** ---------------------------------------------------------------------- ***/

gboolean ProportionalFont(_WORD *width)
{
#define PF_START	32
#define PF_END		155
	char str[2];
	_WORD ext[8];
	short i = PF_START;
	gboolean isProp = FALSE;
	short last_width = 0;

	long w = 0;

	str[1] = 0;
	do
	{
		*str = i++;
		vqt_extent(vdi_handle, str, ext);

		if (last_width && last_width != (ext[2] - ext[1]))
			isProp = TRUE;

		last_width = ext[2] - ext[1];
		w += last_width;

	} while (i < PF_END);
	*width = (short) (w / (PF_END - PF_START) + 1);
	return isProp;
}

/*** ---------------------------------------------------------------------- ***/

void SwitchFont(DOCUMENT *doc)
{
	_WORD normal_font_id;
	_WORD normal_font_size;
	
	UNUSED(doc);
	normal_font_id = gl_profile.viewer.font_id ? gl_profile.viewer.font_id : aes_fontid;
	normal_font_size = gl_profile.viewer.font_pt ? gl_profile.viewer.font_pt : aes_fontsize;
	if (sel_font_id == normal_font_id && gl_profile.viewer.xfont_id != 0)
	{
		sel_font_id = gl_profile.viewer.xfont_id;
		sel_font_pt = gl_profile.viewer.xfont_pt;
	} else
	{
		sel_font_id = normal_font_id;
		sel_font_pt = normal_font_size;
	}
	ApplyFont();
}
