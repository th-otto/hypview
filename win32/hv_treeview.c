/*
 * HypView - (c) 2019 - 2020 Thorsten Otto
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
#include "windebug.h"
#include "resource.rh"
#include <commctrl.h>

struct prep_info {
	HWND hwnd;
	unsigned int window_id;
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static gboolean TreeviewGotoNode(WINDOW_DATA *win, const char *chapter, hyp_nodenr node)
{
	DOCUMENT *doc = hypwin_doc(win);
	UNUSED(chapter);
	UNUSED(node);
	HYP_DBG(("TreeviewGotoNode(Chapter: <%s> / <%u>)", printnull(chapter), node));
	doc->prepNode(win, NULL);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static hyp_nodenr TreeviewGetNode(WINDOW_DATA *win)
{
	UNUSED(win);
	return HYP_NOINDEX;
}

/*** ---------------------------------------------------------------------- ***/

static void TreeviewClose(DOCUMENT *doc)
{
	doc->data = NULL;
}

static HTREEITEM add_items(struct prep_info *info, HYP_DOCUMENT *hyp, HYPTREE *tree, hyp_nodenr node, TVINSERTSTRUCTW *parent)
{
	hyp_nodenr child;
	TVINSERTSTRUCTW ins;
	
	ins.item.mask = TVIF_TEXT | TVIF_PARAM;
	ins.item.hItem = 0;
	ins.item.state = TVIS_EXPANDED;
	ins.item.stateMask = TVIS_EXPANDED;
	ins.item.pszText = hyp_utf8_to_wchar(tree[node].title, STR0TERM, NULL);
	ins.item.cchTextMax = wcslen(ins.item.pszText) + 1;
	ins.item.iImage = 0;
	ins.item.iSelectedImage = 0;
	ins.item.cChildren = 0;
	ins.item.lParam = node;
	ins.hInsertAfter = parent->hInsertAfter;
	ins.hParent = parent->hParent;
	
	ins.hParent = (HTREEITEM)SendMessage(info->hwnd, TVM_INSERTITEMW, 0, (LPARAM)&ins);
	if (ins.hParent == NULL)
		return NULL;
	
	g_free(ins.item.pszText);
	if (tree[node].num_childs != 0)
	{
		ins.hInsertAfter = ins.hParent;
		child = tree[node].head;
		while (child != HYP_NOINDEX)
		{
			ins.hInsertAfter = add_items(info, hyp, tree, child, &ins);
			if (ins.hInsertAfter == NULL)
				return NULL;
			child = tree[child].next;
		}
		SendMessage(info->hwnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)ins.hParent);
	}
	return ins.hParent;
}

/*** ---------------------------------------------------------------------- ***/

static void TreeviewPrep(WINDOW_DATA *win, HYP_NODE *node)
{
	HYPTREE *tree;
	DOCUMENT *doc;
	HYP_DOCUMENT *hyp;
	struct prep_info info;
	TVINSERTSTRUCTW parent;

	UNUSED(node);
	/*
	 * we only need to do this once
	 */
	if (win->treeview_prepped)
		return;
	doc = win->data;
	if (doc == NULL)
		return;
	hyp = (HYP_DOCUMENT *)doc->data;
	if (hyp == NULL)
		return;

	tree = hyp_tree_build(hyp);
	if (tree == NULL)
	{
		char *msg = g_strdup_printf(_("Error reading hyp tree: %s"), hyp_utf8_strerror(errno));
		show_message(NULL, _("Error"), msg, FALSE);
		g_free(msg);
		return;
	}

	info.window_id = win->treeview_parent;
	info.hwnd = win->treewin;
	parent.hInsertAfter = (HTREEITEM)TVI_FIRST;
	parent.hParent = (HTREEITEM)TVI_ROOT;
	add_items(&info, hyp, tree, 0, &parent);
	hyp_tree_free(hyp, tree);
	
	win->treeview_prepped = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void TreeviewGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos)
{
	UNUSED(win);
	UNUSED(x);
	UNUSED(y);
	pos->line = 0;
	pos->y = 0;
	pos->offset = 0;
	pos->x = 0;
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *HaveTreeview(WINDOW_DATA *orig)
{
	WINDOW_DATA *win;
	GSList *l;;

	for (l = all_list; l; l = l->next)
	{
		win = (WINDOW_DATA *)l->data;
		if (win->window_id == orig->treeview_window_id)
		{
			return win;
		}
	}
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *ShowTreeview(WINDOW_DATA *orig)
{
	DOCUMENT *doc;
	HYP_DOCUMENT *hyp;
	const char *path;
	WINDOW_DATA *win;
	struct stat st;

	doc = orig->data;
	ASSERT(doc);
	hyp = (HYP_DOCUMENT *)doc->data;
	ASSERT(hyp);
	path = doc->path;
	/*
	 * see if we have already a treeview window
	 */
	win = HaveTreeview(orig);
	if (win)
	{
		hv_win_open(win);
		return win;
	}

	doc = g_new0(DOCUMENT, 1);
	doc->path = g_strdup(path);
	doc->type = HYP_FT_TREEVIEW;
	doc->ref_count = 1;
	doc->data = hyp; /* only while preparing the view */

	doc->displayProc = HypDisplayPage;
	doc->closeProc = TreeviewClose;
	doc->gotoNodeProc = TreeviewGotoNode;
	doc->getNodeProc = TreeviewGetNode;
	doc->autolocProc = 0;
	doc->getCursorProc = TreeviewGetCursorPosition;
	doc->blockProc = HypBlockOperations;
	doc->prepNode = TreeviewPrep;
	if (hyp_utf8_stat(path, &st) == 0)
		doc->mtime = st.st_mtime;

	win = win32_hypview_window_new(doc, FALSE, TRUE);
	if (win == NULL)
	{
		doc->data = NULL;
		HypCloseFile(doc);
		return NULL;
	}
	g_free(win->title);
	win->title = g_strdup_printf(_("Treeview of %s"), hyp_basename(doc->path));
	win->treeview_parent = orig->window_id;
	orig->treeview_window_id = win->window_id;
	SendMessage(win->treewin, TVM_SETUNICODEFORMAT, TRUE, 0);
	ReInitWindow(win, TRUE);
	hv_win_open(win);
	doc->data = NULL; /* no longer used */

	return win;
}
