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

#include "hv_defs.h"
#include "hypdebug.h"
#include "hypview.h"
#include <xrsrc.h>


static void show_rsc(WINDOW_DATA *win, RSHDR *rsh, _UWORD treenr)
{
	_UWORD ntrees;
	OBJECT **trees;
	char *str;
	OBJECT *tree;
	GRECT big, little;
	_WORD obj;
	_WORD mx, my, button, kstate;
	HYP_NODE *node = win->displayed_node;
	hyp_nodenr dest;
	hyp_lineno line;
	
	if ((rsh->rsh_vrsn & 3) == 3)
	{
		XRS_HEADER *xrsh = (XRS_HEADER *)rsh;
		ntrees = (_UWORD)xrsh->rsh_ntree;
		trees = (OBJECT **)((char *)xrsh + xrsh->rsh_trindex);
	} else
	{
		ntrees = rsh->rsh_ntree;
		trees = (OBJECT **)((char *)rsh + rsh->rsh_trindex);
	}
	if (treenr >= ntrees)
	{
		str = g_strdup_printf(rs_string(HV_ERR_NOTREE), treenr);
		form_alert(1, str);
		g_free(str);
		return;
	}
	tree = trees[treenr];
	little.g_x = little.g_y = little.g_w = little.g_h = 0;
	form_center_grect(tree, &big);
	wind_update(BEG_UPDATE);
	form_dial_grect(FMD_START, &little, &big);
	for (;;)
	{
		objc_draw_grect(tree, ROOT, MAX_DEPTH, &big);
		evnt_button(1, 1, 1, &mx, &my, &button, &kstate);
		evnt_button(1, 1, 0, &mx, &my, &button, &kstate);
		obj = objc_find(tree, ROOT, MAX_DEPTH, mx, my);
		if (obj < 0)
			break;
		dest = hyp_node_find_objref(node, treenr, obj, &line);
		if (dest == HYP_NOINDEX)
		{
			if (form_alert(1, rs_string(HV_ERR_NO_OBJECT)) != 1)
				break;
		} else
		{
			if (OpenPopup(win, dest, line, mx - win->scroll.g_x, my - win->scroll.g_y))
			{
				wind_update(END_UPDATE);
				while (win->popup != NULL)
				{
					DoEvent();
				}
				wind_update(BEG_UPDATE);
			}
		}
	}
	form_dial_grect(FMD_FINISH, &little, &big);
	wind_update(END_UPDATE);
}


void ShowResource(WINDOW_DATA *win, const char *path, _UWORD treenr)
{
	RSHDR *old_rsh;
	OBJECT **old_tree;
	_UWORD old_rsclen;
	char *dir;
	char *filename;
	DOCUMENT *doc = win->data;
	RSHDR *rsh;
	
	old_rsh = (RSHDR *) _AESrscmem;
	old_tree = _AESrscfile;
	old_rsclen = _AESrsclen;
	dir = hyp_path_get_dirname(doc->path);
	filename = g_build_filename(dir, path, NULL);
	g_free(dir);
	
	if (rsrc_load(filename) == 0)
	{
		FileErrorNr(filename, ENOENT);
	} else
	{
		rsh = (RSHDR *) _AESrscmem;
		show_rsc(win, rsh, treenr);
		rsrc_free();
	}

	_AESrscmem = old_rsh;
	_AESrscfile = old_tree;
	_AESrsclen = old_rsclen;
}
