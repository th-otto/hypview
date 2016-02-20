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
	_WORD fid, pt;

	fid = gl_profile.viewer.use_xfont ? gl_profile.viewer.xfont_id : gl_profile.viewer.font_id;
	pt = gl_profile.viewer.use_xfont ? gl_profile.viewer.xfont_pt : gl_profile.viewer.font_pt;
	if (fid == 0)
	{
		fid = aes_fontid;
		pt = aes_fontsize;
	}
	
	vst_font(vdi_handle, fid);
	vst_point(vdi_handle, pt, &font_w, &font_h, &font_cw, &font_ch);

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
				hyp_nodenr node = doc->getNodeProc(win);

				/* remove all nodes from cache */
				RemoveNodes((HYP_DOCUMENT *)doc->data);

				/* reload page */
				ret = doc->gotoNodeProc(win, NULL, node);
			} else
			{
				/* reload file */
				ret = doc->gotoNodeProc(win, NULL, doc->getNodeProc(win));
			}
			
			if (ret)
			{
				doc->start_line = win->docsize.y / win->y_raster;

				/* forget about "fulled" state */
				win->status &= ~WIS_FULL;

				/* re-init window */
				ReInitWindow(win, FALSE);
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
		if (gl_profile.viewer.use_xfont)
		{
			gl_profile.viewer.xfont_id = (int)ptr->id;
			gl_profile.viewer.xfont_pt = fix31_to_point(ptr->pt);
		} else
		{
			gl_profile.viewer.font_id = (int)ptr->id;
			gl_profile.viewer.font_pt = fix31_to_point(ptr->pt);
		}
		ApplyFont();
		HypProfile_SetChanged();
	}
	if (ptr->button == FNTS_CANCEL || ptr->button == FNTS_OK)
	{
		/* close the selector */
		return FALSE;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void SelectFont(WINDOW_DATA *win)
{
	FONTSEL_DATA *ptr;
	
	UNUSED(win);
	ptr = (FONTSEL_DATA *)find_ptr_by_type(WIN_FONTSEL);
	if (ptr == NULL)
	{
		ptr = CreateFontselector(FontSelected, FNTS_ALL, "The quick brown...", NULL);
		if (ptr)
		{
			_WORD fid = gl_profile.viewer.use_xfont ? gl_profile.viewer.xfont_id : gl_profile.viewer.font_id;
			_WORD pt = gl_profile.viewer.use_xfont ? gl_profile.viewer.xfont_pt : gl_profile.viewer.font_pt;
			OpenFontselector(ptr, FNTS_SNAME|FNTS_SSIZE|FNTS_CHNAME|FNTS_CHSIZE|FNTS_BSET, fid, point_to_fix31(pt), 0);
		}
	} else
	{
		wind_set_top(ptr->whandle);
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

void SwitchFont(WINDOW_DATA *win)
{
	UNUSED(win);
	gl_profile.viewer.use_xfont = !gl_profile.viewer.use_xfont && gl_profile.viewer.xfont_id != 0 && gl_profile.viewer.xfont_id != gl_profile.viewer.font_id;
	HypProfile_SetChanged();
	ApplyFont();
}
