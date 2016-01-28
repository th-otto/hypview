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


static RSHDR *skin_rsh;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void hv_init(void)
{
	_WORD dummy;
	_WORD font_w, font_h;
	char *skin;
	_WORD fid, pt;
	
	/*
	 * initialize VDI
	 */

	/* set default value for text putput */
	vswr_mode(vdi_handle, MD_TRANS);

	if (vq_gdos())						/* GDOS avaiablable? */
		vst_load_fonts(vdi_handle, 0);	/* load fonts */

	fid = gl_profile.viewer.use_xfont ? gl_profile.viewer.xfont_id : gl_profile.viewer.font_id;
	pt = gl_profile.viewer.use_xfont ? gl_profile.viewer.xfont_pt : gl_profile.viewer.font_pt;
	if (fid == 0)
	{
		fid = aes_fontid;
		pt = aes_fontsize;
	}
	
	vst_font(vdi_handle, fid);		/* select font */
	vst_point(vdi_handle, pt, &font_w, &font_h, &font_cw, &font_ch);

	if (ProportionalFont(&font_w))
		font_cw = font_w;

	/* set default alignment */
	vst_alignment(vdi_handle, TA_LEFT, TA_TOP, &dummy, &dummy);

	/* set default fill attributes */
	vsf_color(vdi_handle, G_WHITE);
	vsf_interior(vdi_handle, FIS_SOLID);
	vsf_perimeter(vdi_handle, 0);

	/* set default line attribtes */
	vsl_udsty(vdi_handle, 0xAAAA);		/* dotted line */
	vsl_width(vdi_handle, 1);
	vsl_ends(vdi_handle, 0, 0);
	vsl_type(vdi_handle, SOLID);
	vsl_color(vdi_handle, G_BLACK);

	/*
	 * initialize AES
	 */

	/* load toolbar/skin */
	skin = gl_profile.viewer.skin_path;
	if (!empty(skin))
	{
		char *path;
		
		if (*skin != '$' && !g_path_is_absolute(skin))
			skin = g_build_filename("$BINDIR", "Skins", skin, NULL);
		else
			skin = g_strdup(skin);
		path = path_subst(skin);
		g_free(skin);
		if (rsrc_load(path))
		{
			OBJECT **skin_rs_trindex;
			OBJECT *skin_rs_object;
			RSHDR *rsh = skin_rsh;
			OBJECT *rsc_toolbar = toolbar_tree;
			
			skin_rsh = rsh = (RSHDR *) _AESrscmem;
			skin_rs_trindex = (OBJECT **) (((char *) rsh) + rsh->rsh_trindex);
			skin_rs_object = (OBJECT *) (((char *) rsh) + rsh->rsh_object);
			hfix_objs(skin_rsh, skin_rs_object, rsh->rsh_nobs);
			toolbar_tree = skin_rs_trindex[0];
			/*
			 * copy the strings from the original resource,
			 * which might have been localized
			 */
#define copystr(o) \
			toolbar_tree[o].ob_spec.free_string = rsc_toolbar[o].ob_spec.free_string; \
			toolbar_tree[o].ob_width = rsc_toolbar[o].ob_width; \
			toolbar_tree[o].ob_height = rsc_toolbar[o].ob_height
			copystr(TO_SEARCH);
			copystr(TO_STRNOTFOUND);
#undef copystr
		} else
		{
			HYP_DBG(("Could not load skin '%s'", printnull(path)));
		}
		g_free(path);
	}

#if 0
	{
		_WORD i;
		
		/* remove icon text */
		for (i = ROOT; ; i++)
		{
			switch (toolbar_tree[i].ob_type & 0xff)
			{
			case G_ICON:
			case G_CICON:
				toolbar_tree[i].ob_spec.iconblk->ib_wtext = 0;
				toolbar_tree[i].ob_spec.iconblk->ib_htext = 0;
				break;
			}
			if (toolbar_tree[i].ob_flags & OF_LASTOB)
				break;
		}
	}
#endif

	/* adjust size of background object */
	toolbar_tree[TO_BACKGRND].ob_height =
		max(toolbar_tree[TO_SEARCHBOX].ob_height, toolbar_tree[TO_BUTTONBOX].ob_height);
	/* move boxes to final location */
	toolbar_tree[TO_SEARCHBOX].ob_y = (toolbar_tree[TO_BACKGRND].ob_height - toolbar_tree[TO_SEARCHBOX].ob_height) >> 1;
	toolbar_tree[TO_BUTTONBOX].ob_y = (toolbar_tree[TO_BACKGRND].ob_height - toolbar_tree[TO_BUTTONBOX].ob_height) >> 1;



	/* load markers */
	MarkerInit();
}

/*** ---------------------------------------------------------------------- ***/

void hv_exit(void)
{
	MarkerSaveToDisk();

	hv_userdef_exit();
	
	if (vq_gdos() && vdi_handle)
		vst_unload_fonts(vdi_handle, 0);
}
