/*
 * HypView - (c)      - 2020 Thorsten Otto
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
#include "gdkkeysyms.h"

struct prep_info {
	int target_link_id;
	int tab_id;
	unsigned int window_id;
	GtkTreeStore *model;
};

/* columns */
enum
{
	TITLE_COLUMN,
	NAME_COLUMN,
	TOOLTIP_COLUMN,
	PAGE_COLUMN,
	VISIBLE_COLUMN,
	NUM_COLUMNS
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
	HYP_DBG(("TreeviewGetNode not implemented."));
	return HYP_NOINDEX;
}

/*** ---------------------------------------------------------------------- ***/

static void TreeviewClose(DOCUMENT *doc)
{
	doc->data = NULL;
}

/*** ---------------------------------------------------------------------- ***/

static void add_items(struct prep_info *info, HYP_DOCUMENT *hyp, HYPTREE *tree, hyp_nodenr node, GtkTreeIter *parent_iter)
{
	hyp_nodenr child;
	GtkTreeIter iter;
	
	gtk_tree_store_append(info->model, &iter, parent_iter);
	gtk_tree_store_set(info->model, &iter,
		TITLE_COLUMN, tree[node].title,
		NAME_COLUMN, tree[node].name,
		TOOLTIP_COLUMN, tree[node].name, /* FIXME: must quote some chars */
		PAGE_COLUMN, node,
		VISIBLE_COLUMN, FALSE,
		-1);
	
	if (tree[node].num_childs != 0)
	{
		child = tree[node].head;
		while (child != HYP_NOINDEX)
		{
			add_items(info, hyp, tree, child, &iter);
			child = tree[child].next;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

static void add_columns(GtkTreeView *treeview)
{
	gint col_offset;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	/* GtkTreeModel *model = gtk_tree_view_get_model(treeview); */
	
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", 0.0, NULL);
	col_offset = gtk_tree_view_insert_column_with_attributes(treeview,
		-1, "Title", renderer, "text", TITLE_COLUMN, NULL);
	column = gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), col_offset - 1);
	gtk_tree_view_column_set_clickable(GTK_TREE_VIEW_COLUMN(column), TRUE);
}

/*** ---------------------------------------------------------------------- ***/

static void row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, void *user_data)
{
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GValue value = G_VALUE_INIT;
	LINK_INFO linkinfo;
	WINDOW_DATA *win = (WINDOW_DATA *)user_data;
	
	UNUSED(column);
	gtk_tree_model_get_iter(model, &iter, path);
	linkinfo.link_type = HYP_ESC_LINK_LINE;
	linkinfo.dst_type = HYP_NODE_INTERNAL;
	linkinfo.tip = NULL;
	gtk_tree_model_get_value(model, &iter, PAGE_COLUMN, &value);
	linkinfo.dest_page = g_value_get_uint(&value);
	g_value_unset(&value);
	linkinfo.line_nr = 0;
	linkinfo.window_id = win->treeview_parent;
	win = hv_link_targetwin(win, &linkinfo);
	if (win)
		HypClick(win, &linkinfo);
}

/*** ---------------------------------------------------------------------- ***/

static void TreeviewPrep(WINDOW_DATA *win, HYP_NODE *node)
{
	HYPTREE *tree;
	DOCUMENT *doc;
	HYP_DOCUMENT *hyp;
	struct prep_info info;
	GtkTreeSelection *select;

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

	tree = hyp_tree_build(hyp, -1);
	if (tree == NULL)
	{
		char *msg = g_strdup_printf(_("Error reading hyp tree: %s"), hyp_utf8_strerror(errno));
		show_message(NULL, _("Error"), msg, FALSE);
		g_free(msg);
		return;
	}
	info.target_link_id = 0;
	info.tab_id = 0;
	info.window_id = win->treeview_parent;
	info.model = gtk_tree_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_BOOLEAN);
	
	add_items(&info, hyp, tree, 0, NULL);
	hyp_tree_free(hyp, tree);
	gtk_tree_view_set_model(GTK_TREE_VIEW(win->tree_view), GTK_TREE_MODEL(info.model));

	add_columns(GTK_TREE_VIEW(win->tree_view));
	g_signal_connect(win->tree_view, "realize", G_CALLBACK(gtk_tree_view_expand_all), NULL);
	g_signal_connect(win->tree_view, "row-activated", G_CALLBACK(row_activated), win);

	gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(win->tree_view), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(win->tree_view), FALSE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(win->tree_view), FALSE);
	gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(win->tree_view), TOOLTIP_COLUMN);
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(win->tree_view));
	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
	
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

static void hyp_destroyed(GtkWidget *w, gpointer user_data)
{
	WINDOW_DATA *orig;
	WINDOW_DATA *win;
	GSList *l;
	
	orig = (WINDOW_DATA *)w;
	win = (WINDOW_DATA *)user_data;
	/*
	 * when the original reference window is destroyed,
	 * remove also the treeview
	 */
	for (l = all_list; l; l = l->next)
	{
		if (l->data == win)
		{
			g_signal_handlers_disconnect_by_func(G_OBJECT(orig), G_CALLBACK(hyp_destroyed), win);
			gtk_widget_destroy(GTK_WIDGET(win));
			return;
		}
	}
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

static gboolean key_press(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GtkTreePath *path;
	gboolean handled = FALSE;
	
	UNUSED(user_data);
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(widget), &path, NULL);
	if (path)
	{
		switch (event->key.keyval)
		{
		case GDK_KEY_Left:
		case GDK_KEY_KP_Left:
			gtk_tree_view_collapse_row(GTK_TREE_VIEW(widget), path);
			handled = TRUE;
			break;
		case GDK_KEY_Right:
		case GDK_KEY_KP_Right:
			gtk_tree_view_expand_row(GTK_TREE_VIEW(widget), path, FALSE);
			handled = TRUE;
			break;
		}
		gtk_tree_path_free(path);
	}
	return handled;
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

	win = gtk_hypview_window_new(doc, FALSE, TRUE);
	g_free(win->title);
	win->title = g_strdup_printf(_("Treeview of %s"), hyp_basename(doc->path));
	win->treeview_parent = orig->window_id;
	orig->treeview_window_id = win->window_id;
	ReInitWindow(win, TRUE);
	hv_win_open(win);
	doc->data = NULL; /* no longer used */

	g_signal_connect(G_OBJECT(orig), "destroy", G_CALLBACK(hyp_destroyed), win);

	g_signal_connect(G_OBJECT(win->tree_view), "key-press-event", G_CALLBACK(key_press), win);

	return win;
}
