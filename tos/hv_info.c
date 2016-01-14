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
#include "hypview.h"
#include "hypdebug.h"


static DIALOG *Prog_Dialog;


static void adjust_tree(OBJECT *tree, _WORD box, _WORD diff)
{
	_WORD i;
	_WORD y = tree[box].ob_y;
	
	i = tree[ROOT].ob_head;
	do {
		if (i != box && tree[i].ob_y >= y)
		{
			tree[i].ob_y += diff;
		}
		i = tree[i].ob_next;
	} while (i != ROOT);
	tree[ROOT].ob_height += diff;
}


static void show_morebox(OBJECT *tree, _WORD box)
{
	if ((tree[box].ob_flags & OF_HIDETREE) == 0)
		return;
	adjust_tree(tree, box, tree[box].ob_height + phchar);
	tree[box].ob_flags &= ~OF_HIDETREE;
	tree[INFO_LESS].ob_flags &= ~OF_HIDETREE;
	tree[INFO_MORE].ob_flags |= OF_HIDETREE;
}


static void hide_morebox(OBJECT *tree, _WORD box)
{
	if (tree[box].ob_flags & OF_HIDETREE)
		return;
	adjust_tree(tree, box, -(tree[box].ob_height + phchar));
	tree[box].ob_flags |= OF_HIDETREE;
	tree[INFO_MORE].ob_flags &= ~OF_HIDETREE;
	tree[INFO_LESS].ob_flags |= OF_HIDETREE;
}


static void toggle_morebox(OBJECT *tree, _WORD box)
{
	if (tree[box].ob_flags & OF_HIDETREE)
		show_morebox(tree, box);
	else
		hide_morebox(tree, box);
}


static _WORD __CDECL ProgHandle(struct HNDL_OBJ_args args)
{
	OBJECT *tree;
	GRECT r;
	_WORD dummy;
	
	wdlg_get_tree(args.dialog, &tree, &r);

	switch (args.obj)
	{
	case HNDL_CLSD:
		return 0;
	case HNDL_MESG:
		SpecialMessageEvents(args.dialog, args.events);
		break;
	case PROG_OK:
		tree[PROG_OK].ob_state &= ~OS_SELECTED;
		return 0;
	case INFO_LESS:
	case INFO_MORE:
		tree[args.obj].ob_state &= ~OS_SELECTED;
		toggle_morebox(tree, INFO_MORE_BOX);
		objc_offset(tree, ROOT, &r.g_x, &r.g_y);
		r.g_w = tree[ROOT].ob_width;
		r.g_h = tree[ROOT].ob_height;
		if (wind_get(0, WF_XAAES, &dummy, &dummy, &dummy, &dummy) == WF_XAAES)
		{
			/* work around a bug in XaAES which modifies ob_height of the root */
			wdlg_set_tree(args.dialog, rs_tree(DIAL_LIBRARY));
			wdlg_set_tree(args.dialog, tree);
		}
		wdlg_set_size(args.dialog, &r);
		wdlg_redraw(args.dialog, &r, ROOT, MAX_DEPTH);
		break;
	case PROG_HELP:
		tree[PROG_HELP].ob_state &= ~OS_SELECTED;
		OpenFileNewWindow(prghelp_name, NULL, HYP_NOINDEX, FALSE);
		return 0;
	}
	return 1;
}


static void set_text(OBJECT *tree, _WORD idx, const char *txt)
{
	TEDINFO *ted = tree[idx].ob_spec.tedinfo;
	char *str;
	_WORD len;
	
	memset(ted->te_ptext, ' ', ted->te_txtlen - 1);
	if (txt != NULL)
	{
		str = hyp_utf8_to_charset(hyp_get_current_charset(), txt, STR0TERM, NULL);
		len = (_WORD)strlen(str);
		if (len > ted->te_txtlen - 1)
			len = ted->te_txtlen - 1;
		memcpy(ted->te_ptext, str, len);
		g_free(str);
	}
}


void ProgrammInfos(DOCUMENT *doc)
{
	GRECT big, little;
	HYP_DOCUMENT *hyp = doc->data;
	OBJECT *tree = rs_tree(PROGINFO);
	char *version = gl_program_version();
	char buf[20];
	
	static gboolean first = TRUE;
	if (first)
	{
		hide_morebox(tree, INFO_MORE_BOX);
		first = FALSE;
	}
	
	sprintf(tree[PROG_NAME].ob_spec.free_string, "%s %s", gl_program_name, version);
	g_free(version);
	sprintf(tree[PROG_DATE].ob_spec.free_string, rs_string(PROGINFO_FROM), gl_compile_date);

	set_text(tree, PROG_FILE, hyp_basename(doc->path));
	if (doc->type == HYP_FT_HYP)
	{
		set_text(tree, PROG_DATABASE, hyp->database);
		tree[PROG_DATABASE].ob_flags &= ~OF_HIDETREE;
		set_text(tree, PROG_AUTHOR, hyp->author);
		tree[PROG_AUTHOR].ob_flags &= ~OF_HIDETREE;
		set_text(tree, PROG_VERSION, hyp->version);
		tree[PROG_VERSION].ob_flags &= ~OF_HIDETREE;
		set_text(tree, PROG_SUBJECT, hyp->subject);
		tree[PROG_SUBJECT].ob_flags &= ~OF_HIDETREE;
		sprintf(buf, "%5d", hyp->num_index);
		set_text(tree, INFO_NODES, buf);
		sprintf(buf, "%7ld", hyp->itable_size);
		set_text(tree, INFO_INDEXSIZE, buf);
		sprintf(buf, "%3u", hyp->comp_vers);
		set_text(tree, INFO_HCPVERSION, buf);
		set_text(tree, INFO_OS, hyp_osname(hyp->comp_os));
		set_text(tree, INFO_DEFAULT, hyp->default_name);
		set_text(tree, INFO_OPTIONS, hyp->hcp_options);
		set_text(tree, INFO_HELP, hyp->help_name);
		sprintf(buf, "%3u", hyp->line_width);
		set_text(tree, INFO_WIDTH, buf);
		tree[INFO_LESS].ob_flags = (tree[INFO_LESS].ob_flags & ~OF_HIDETREE) | (tree[INFO_MORE_BOX].ob_flags & OF_HIDETREE);
		tree[INFO_MORE].ob_flags = (tree[INFO_MORE].ob_flags | OF_HIDETREE) & ~(tree[INFO_MORE_BOX].ob_flags & OF_HIDETREE);
	} else
	{
		hide_morebox(tree, INFO_MORE_BOX);
		tree[PROG_DATABASE].ob_flags |= OF_HIDETREE;
		tree[PROG_AUTHOR].ob_flags |= OF_HIDETREE;
		tree[PROG_VERSION].ob_flags |= OF_HIDETREE;
		tree[PROG_SUBJECT].ob_flags |= OF_HIDETREE;
		tree[INFO_LESS].ob_flags |= OF_HIDETREE;
		tree[INFO_MORE].ob_flags |= OF_HIDETREE;
	}
	
	if (has_window_dialogs())
	{
		Prog_Dialog = OpenDialog(ProgHandle, tree, rs_string(PROGINFO_TITLE), -1, -1, NULL);
	} else
	{
		_WORD obj;
		
		little.g_x = little.g_y = little.g_w = little.g_h = 0;
		do {
			form_center_grect(tree, &big);
			form_dial_grect(FMD_START, &little, &big);
			objc_draw_grect(tree, ROOT, MAX_DEPTH, &big);
			obj = form_do(tree, 0);
			if (obj > 0)
				tree[obj].ob_state &= ~OS_SELECTED;
			form_dial_grect(FMD_FINISH, &little, &big);
		} while (obj > 0 && obj != PROG_HELP && obj != PROG_OK);
		switch (obj)
		{
		case PROG_HELP:
			OpenFileNewWindow(prghelp_name, NULL, HYP_NOINDEX, FALSE);
			break;
		}
	}
}
