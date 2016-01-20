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

static DIALOG *Hypfind_Dialog;
static short HypfindID;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void hypfind_search_allref(WINDOW_DATA *win, OBJECT *tree)
{
	char *name;

	name = hyp_conv_to_utf8(hyp_get_current_charset(), tree[HYPFIND_STRING].ob_spec.tedinfo->te_ptext, STR0TERM);
	search_allref(win, name, FALSE);
	g_free(name);
}

/*** ---------------------------------------------------------------------- ***/

static void hypfind_run_hypfind(OBJECT *tree, DOCUMENT *doc)
{
	char ZStr[1024];
	char *name;
	
	strcpy(ZStr, " -p '");
	name = hyp_conv_to_utf8(hyp_get_current_charset(), tree[HYPFIND_STRING].ob_spec.tedinfo->te_ptext, STR0TERM);
	strcat(ZStr, name);
	strcat(ZStr, "'");
	g_free(name);
	if ((tree[HYPFIND_ALL_HYP].ob_state & OS_SELECTED) == 0)
	{
		strcat(ZStr, " ");
		strcat(ZStr, doc->path);
	}
	ZStr[0] = (char)strlen(ZStr + 1);
	if (!empty(gl_profile.general.hypfind_path))
	{
		char *filename = path_subst(gl_profile.general.hypfind_path);
		HypfindID = shel_write(SHW_EXEC, 0, SHW_PARALLEL, filename, ZStr);
		if (HypfindID == 0)
			HypfindID = -1;
		g_free(filename);
	}
}

/*** ---------------------------------------------------------------------- ***/

static _WORD __CDECL HypfindHandle(struct HNDL_OBJ_args args)
{
	OBJECT *tree;
	DIALOG_DATA *dial;
	WINDOW_DATA *win;
	DOCUMENT *doc;
	GRECT r;
	char *name;
	
	dial = wdlg_get_udata(args.dialog);
	doc = dial->data;
	win = doc->window;
	wdlg_get_tree(args.dialog, &tree, &r);

	if (args.obj > 0)
		tree[args.obj].ob_state &= ~OS_SELECTED;

	switch (args.obj)
	{
	case HNDL_CLSD:
		return 0;
	case HNDL_MESG:
		SpecialMessageEvents(args.dialog, args.events);
		break;
	case HYPFIND_TEXT:
		return 0;
	case HYPFIND_PAGES:
		name = hyp_conv_to_utf8(hyp_get_current_charset(), tree[HYPFIND_STRING].ob_spec.tedinfo->te_ptext, STR0TERM);
		OpenFileInWindow(win, doc->path, name, HYP_NOINDEX, FALSE, FALSE, FALSE);
		g_free(name);
		return 0;
	case HYPFIND_ABORT:
		return 0;
	case HYPFIND_REF:
		hypfind_search_allref(win, tree);
		return 0;
	case HYPFIND_ALL_PAGE:
		hypfind_run_hypfind(tree, doc);
		return 0;
	}

	return 1;
}

/*** ---------------------------------------------------------------------- ***/

void Hypfind(DOCUMENT *doc)
{
	static short first = 0;
	OBJECT *tree = rs_tree(HYPFIND);
	WINDOW_DATA *win = doc->window;
	char *name;
	
	if (!first)
	{
		strcpy(tree[HYPFIND_STRING].ob_spec.tedinfo->te_ptext, "");
		first = 1;
		HypfindID = -1;
	}

	if (HypfindID != -1)
		return;

	if (has_window_dialogs())
	{
		Hypfind_Dialog = OpenDialog(HypfindHandle, tree, rs_string(WDLG_SEARCH_PATTERN), -1, -1, doc);
	} else
	{
		short obj;
		GRECT big, little;

		little.g_x = little.g_y = little.g_w = little.g_h = 0;
		form_center_grect(tree, &big);
		form_dial_grect(FMD_START, &little, &big);
		objc_draw_grect(tree, ROOT, MAX_DEPTH, &big);
		obj = form_do(tree, HYPFIND_STRING);
		if (obj > 0)
			tree[obj].ob_state &= ~OS_SELECTED;
		form_dial_grect(FMD_FINISH, &little, &big);

		switch (obj)
		{
		case HYPFIND_ABORT:
			break;
		case HYPFIND_REF:
			hypfind_search_allref(win, tree);
			break;
		case HYPFIND_PAGES:
			name = hyp_conv_to_utf8(hyp_get_current_charset(), tree[HYPFIND_STRING].ob_spec.tedinfo->te_ptext, STR0TERM);
			OpenFileInWindow(win, doc->path, name, HYP_NOINDEX, FALSE, FALSE, FALSE);
			g_free(name);
			break;
		case HYPFIND_ALL_PAGE:
			hypfind_run_hypfind(tree, doc);
			break;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void HypfindFinish(short AppID, short ret)
{
	UNUSED(ret);
	if (AppID == HypfindID)
	{
		HypfindID = -1;
		OpenFileInWindow(NULL, HYP_FILENAME_HYPFIND, NULL, HYP_NOINDEX, FALSE, TRUE, FALSE);
	}
}
