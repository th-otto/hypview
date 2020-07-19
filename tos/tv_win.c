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
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hv_defs.h"
#include "hyptree.h"
#include "hypdebug.h"

#define setmsg(a,d,e,f,g,h) \
	msg[0] = a; \
	msg[1] = gl_apid; \
	msg[2] = 0; \
	msg[3] = d; \
	msg[4] = e; \
	msg[5] = f; \
	msg[6] = g; \
	msg[7] = h

struct prep_info {
	unsigned int window_id;
	WINDOW_DATA *win;
};

#define UTF8_BOM "\357\273\277"

#define line_down              UTF8_BOM "\342\224\202"
#define line_hor               UTF8_BOM "\342\224\200"
#define line_intersec          UTF8_BOM "\342\224\234"
#define line_collapsed         UTF8_BOM "\342\224\234"
#define line_collapsed_end     UTF8_BOM "\342\224\224"
#define line_expanded          UTF8_BOM "\342\224\234"
#define line_expanded_end      UTF8_BOM "\342\224\224"
#define line_end               UTF8_BOM "\342\224\224"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

void SendRedrawArea(WINDOW_DATA *win, const GRECT *area)
{
	_WORD msg[8];
	
	if (!(win->status & WIS_OPEN))
		return;
	msg[0] = WM_REDRAW;
	msg[1] = gl_apid;
	msg[2] = 0;
	msg[3] = win->whandle;
	msg[4] = area->g_x;
	msg[5] = area->g_y;
	msg[6] = area->g_w;
	msg[7] = area->g_h;
	appl_write(gl_apid, 16, msg);
}

/*** ---------------------------------------------------------------------- ***/

void SendCloseWindow(WINDOW_DATA *win)
{
	if (win)
		SendClose(win->whandle);
}

/*** ---------------------------------------------------------------------- ***/

void SendTopped(_WORD whandle)
{
	_WORD msg[8];
	setmsg(WM_TOPPED, whandle, 0, 0, 0, 0);
	appl_write(gl_apid, 16, msg);
}

/*** ---------------------------------------------------------------------- ***/

void SendRedraw(WINDOW_DATA *win)
{
	GRECT work;
	
	if (!(win->status & WIS_OPEN))
		return;
	wind_get_grect(win->whandle, WF_WORKXYWH, &work);
	SendRedrawArea(win, &work);
}

/*** ---------------------------------------------------------------------- ***/

void WindowCalcScroll(WINDOW_DATA *win)
{
	wind_get_grect(win->whandle, WF_WORKXYWH, &win->work);
	win->scroll.g_x = win->work.g_x + win->x_margin_left;
	win->scroll.g_y = win->work.g_y + win->y_margin_top;
	win->scroll.g_w = win->work.g_w - win->x_margin_left - win->x_margin_right;
	win->scroll.g_h = win->work.g_h - win->y_margin_top - win->y_margin_bottom;
}

/*** ---------------------------------------------------------------------- ***/

DOCUMENT *hypwin_doc(WINDOW_DATA *win)
{
	return win->data;
}

/*** ---------------------------------------------------------------------- ***/

HYP_NODE *hypwin_node(WINDOW_DATA *win)
{
	return win->displayed_node;
}

/*** ---------------------------------------------------------------------- ***/

void hv_set_title(WINDOW_DATA *win, const char *wintitle)
{
	if (!(win->kind & NAME))
		return;
	strncpy(win->titlebuf, wintitle ? wintitle : "", sizeof(win->titlebuf));
	win->titlebuf[sizeof(win->titlebuf) - 1] = '\0';
	wind_set_str(win->whandle, WF_NAME, win->titlebuf);
}

/*** ---------------------------------------------------------------------- ***/

static void draw_expander(_WORD x, _WORD y, _WORD size, unsigned short expanded)
{
	_WORD pxy[8];
	_WORD h;
	
	h = size >= 16 ? 6 : 4;
	if (expanded)
	{
		pxy[0] = x + 2;
		pxy[1] = y + size - 2 * h;
		pxy[2] = x + size - 2;
		pxy[3] = pxy[1];
		pxy[4] = x + (size >> 1);
		pxy[5] = pxy[1] + h;
		pxy[6] = pxy[0];
		pxy[7] = pxy[1];
		v_pline(vdi_handle, 4, pxy);
	} else
	{
		pxy[0] = x + 1;
		pxy[1] = y + 2;
		pxy[2] = pxy[0];
		pxy[3] = y + size - 2;
		pxy[4] = pxy[0] + h;
		pxy[5] = y + (size >> 1);
		pxy[6] = pxy[0];
		pxy[7] = pxy[1];
		v_pline(vdi_handle, 4, pxy);
	}
}

/*** ---------------------------------------------------------------------- ***/

static void draw_lineend(_WORD x, _WORD y, _WORD w, _WORD h)
{
	_WORD pxy[8];
	
	pxy[0] = x;
	pxy[1] = y;
	pxy[2] = x;
	pxy[3] = y + (h >> 1);
	v_pline(vdi_handle, 2, pxy);
	pxy[0] = x;
	pxy[1] = y + (h >> 1);
	pxy[2] = x + w - 1;
	pxy[3] = pxy[1];
	v_pline(vdi_handle, 2, pxy);
}

/*** ---------------------------------------------------------------------- ***/

static void draw_intersec(_WORD x, _WORD y, _WORD w, _WORD h)
{
	_WORD pxy[8];
	
	pxy[0] = x;
	pxy[1] = y;
	pxy[2] = x;
	pxy[3] = y + h - 1;
	v_pline(vdi_handle, 2, pxy);
	pxy[0] = x;
	pxy[1] = y + (h >> 1);
	pxy[2] = x + w - 1;
	pxy[3] = pxy[1];
	v_pline(vdi_handle, 2, pxy);
}

/*** ---------------------------------------------------------------------- ***/

static void draw_lines(HYPTREE *tree, hyp_nodenr node, _WORD x, _WORD y, _WORD h, _WORD indent, int depth)
{
	hyp_nodenr parent;
	hyp_nodenr pparent;
	gboolean is_last;
	_WORD pxy[4];
	
	if (node == 0 || node == HYP_NOINDEX)
		return;
	parent = tree[node].parent;
	x -= indent;
	draw_lines(tree, parent, x, y, h, indent, depth - 1);
	is_last = FALSE;
	if (parent == HYP_NOINDEX)
	{
		is_last = TRUE;
	} else
	{
		pparent = tree[parent].parent;
		if (pparent == HYP_NOINDEX || tree[pparent].tail == parent)
			is_last = TRUE;
	}
	if (is_last)
	{
		nf_debugprintf("  ");
	} else
	{
		pxy[0] = x;
		pxy[1] = y;
		pxy[2] = x;
		pxy[3] = y + h - 1;
		v_pline(vdi_handle, 2, pxy);
		nf_debugprintf(line_down " ");
	}
}

/*** ---------------------------------------------------------------------- ***/

static WP_UNIT calc_docsize(HYPTREE *tree, hyp_nodenr node, WP_UNIT y, _WORD linesize)
{
	hyp_nodenr child;
	
	y += linesize;
	if (tree[node].num_childs != 0 && (tree[node].flags & HYPTREE_IS_EXPANDED))
	{
		child = tree[node].head;
		while (child != HYP_NOINDEX)
		{
			y = calc_docsize(tree, child, y, linesize);
			child = tree[child].next;
		}
	}
	return y;
}

/*** ---------------------------------------------------------------------- ***/

static HYPTREE *tv_hyptree(WINDOW_DATA *win, hyp_nodenr *num_index)
{
	DOCUMENT *doc;
	HYP_DOCUMENT *hyp;

	doc = win->data;
	if (doc == NULL)
		return NULL;
	hyp = (HYP_DOCUMENT *)doc->data;
	if (hyp == NULL)
		return NULL;
	if (num_index)
		*num_index = hyp->num_index;
	return (HYPTREE *)hyp->ref;
}

/*** ---------------------------------------------------------------------- ***/

static void tv_display_page(WINDOW_DATA *win)
{
	_WORD x, y;
	WP_UNIT y_offset, end_y;
	hyp_nodenr node;
	hyp_nodenr num_index;
	HYPTREE *tree;
	int depth;
	
	WindowCalcScroll(win);
	tree = tv_hyptree(win, &num_index);
	if (tree == NULL)
		return;

	x = (_WORD)(win->scroll.g_x - win->docsize.x);
	y = win->scroll.g_y;
	nf_debugprintf("size %ld %ld\n", win->docsize.h, calc_docsize(tree, 0, 0, win->y_raster));
	end_y = win->docsize.y + win->scroll.g_h;
	vswr_mode(vdi_handle, MD_TRANS);
	vst_color(vdi_handle, viewer_colors.text);
	vsl_color(vdi_handle, viewer_colors.text);
	vst_effects(vdi_handle, 0);

	node = 0;
	depth = 0;
	y_offset = 0;
	for (;;)
	{
		if (node >= num_index)
			break;
		if (tree[node].flags & HYPTREE_IS_NODE)
		{
			if (y_offset >= win->docsize.y && y_offset < end_y)
			{
				_WORD indent = max(2 * win->x_raster, win->y_raster);
				_WORD x1 = x + depth * indent;
				_WORD w = (indent >> 1);
				_WORD x2 = x1 + w;
				y = (_WORD)(y_offset - win->docsize.y) + win->scroll.g_y;
				draw_lines(tree, node, x2, y, win->y_raster, indent, depth);
				if (tree[node].num_childs != 0)
				{
					draw_expander(x1, y, win->y_raster, tree[node].flags & HYPTREE_IS_EXPANDED);
					if (tree[node].flags & HYPTREE_IS_EXPANDED)
					{
						if (tree[node].next == HYP_NOINDEX)
							nf_debugprintf(line_expanded_end);
						else
							nf_debugprintf(line_expanded);
					} else
					{
						if (tree[node].next == HYP_NOINDEX)
							nf_debugprintf(line_collapsed_end);
						else
							nf_debugprintf(line_collapsed);
					}
				} else
				{
					if (tree[node].next == HYP_NOINDEX)
					{
						draw_lineend(x2, y, indent - w, win->y_raster);
						nf_debugprintf(line_end);
					} else
					{
						draw_intersec(x2, y, indent - w, win->y_raster);
						nf_debugprintf(line_intersec);
					}
				}
				nf_debugprintf("%u: %s\n", node, tree[node].name);
				v_gtext(vdi_handle, x1 + indent + win->x_raster, y, tree[node].name);
			}
			y_offset += win->y_raster;
			if (tree[node].num_childs != 0 && (tree[node].flags & HYPTREE_IS_EXPANDED))
			{
				node = tree[node].head;
				depth++;
			} else
			{
				for (;;)
				{
					if (tree[node].next != HYP_NOINDEX)
					{
						node = tree[node].next;
						break;
					} else
					{
						node = tree[node].parent;
						if (node == HYP_NOINDEX)
							return;
						depth--;
					}
				}
			}
		}
		if (y_offset >= end_y)
			break;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void tv_close(DOCUMENT *doc)
{
	HYP_DOCUMENT *hyp;
	HYPTREE *tree;

	if (doc == NULL)
		return;
	hyp = (HYP_DOCUMENT *)doc->data;
	if (hyp == NULL)
		return;
	doc->data = NULL;
	tree = (HYPTREE *)hyp->ref;
	hyp->ref = NULL;
	hyp_tree_free(hyp, tree);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean tv_goto_node(WINDOW_DATA *win, const char *chapter, hyp_nodenr node)
{
	DOCUMENT *doc = hypwin_doc(win);
	UNUSED(chapter);
	UNUSED(node);
	HYP_DBG(("TreeviewGotoNode(Chapter: <%s> / <%u>)", printnull(chapter), node));
	doc->prepNode(win, NULL);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static hyp_nodenr tv_get_node(WINDOW_DATA *win)
{
	UNUSED(win);
	return HYP_NOINDEX;
}

/*** ---------------------------------------------------------------------- ***/

static void tv_get_cursor_pos(WINDOW_DATA *win, int x, int y, TEXT_POS *pos)
{
	UNUSED(win);
	UNUSED(x);
	UNUSED(y);
	UNUSED(pos);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean tv_block_op(WINDOW_DATA *win, hyp_blockop op, BLOCK *block, void *param)
{
	UNUSED(win);
	UNUSED(op);
	UNUSED(block);
	UNUSED(param);
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static void add_items(struct prep_info *info, HYPTREE *tree, hyp_nodenr node, hyp_nodenr parent, unsigned short depth)
{
	hyp_nodenr child;
	
	UNUSED(parent);
	info->win->docsize.h += info->win->y_raster;
	tree[node].level = depth;
	if (tree[node].num_childs != 0)
	{
		child = tree[node].head;
		while (child != HYP_NOINDEX)
		{
			add_items(info, tree, child, node, depth + 1);
			child = tree[child].next;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

static void tv_prep_node(WINDOW_DATA *win, HYP_NODE *node)
{
	HYPTREE *tree;
	DOCUMENT *doc;
	HYP_DOCUMENT *hyp;
	struct prep_info info;

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

	win->docsize.x = 0;
	win->docsize.y = 0;
	win->docsize.w = 0;
	win->docsize.h = 0;
	
	tree = hyp_tree_build(hyp);
	if (tree == NULL)
	{
		char *msg = g_strdup_printf(_("[1][Error reading hyp tree|%s][Cancel]"), hyp_utf8_strerror(errno));
		form_alert(1, msg);
		g_free(msg);
		return;
	}

	info.window_id = win->treeview_parent;
	info.win = win;
	add_items(&info, tree, 0, HYP_NOINDEX, 0);
	/* ugly, but will have to do for now: stash the tree info into ref */
	hyp->ref = (REF_FILE *)tree;
	
	win->treeview_prepped = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *tv_open_window(const char *path)
{
	WINDOW_DATA *win = NULL;
	DOCUMENT *doc;
	char *real_path;

	/* done if we don't have a name */
	if (empty(path))
		return NULL;

	/* if path starts with "*:\", remove it */
	if (path[0] == '*' && path[1] == ':' && G_IS_DIR_SEPARATOR(path[2]))
		path += 3;

	if ((real_path = hyp_find_file(path)) == NULL)
	{
		FileError(path, _("not found"));
		return NULL;
	}

	/* load and initialize hypertext file if neccessary */
	doc = HypOpenFile(real_path, FALSE);
	g_free(real_path);
	
	if (doc)
	{
		doc->type = HYP_FT_TREEVIEW;
		doc->displayProc = tv_display_page;
		doc->closeProc = tv_close;
		doc->gotoNodeProc = tv_goto_node;
		doc->getNodeProc = tv_get_node;
		doc->autolocProc = 0;
		doc->getCursorProc = tv_get_cursor_pos;
		doc->blockProc = tv_block_op;
		doc->prepNode = tv_prep_node;
		win = hv_win_new(doc, FALSE);
		if (win)
		{
			g_free(win->title);
			win->title = g_strdup_printf(_("Treeview of %s"), hyp_basename(doc->path));
			ReInitWindow(win, TRUE);
			hv_win_open(win);
		} else
		{
			doc->data = NULL;
			HypCloseFile(doc);
		}
	}

	return win;
}

/*** ---------------------------------------------------------------------- ***/

static void calc_opensize(WINDOW_DATA *win, GRECT *curr)
{
	GRECT screen;
	_WORD minw, minh;

	minw = font_cw + win->x_margin_left + win->x_margin_right;
	minh = font_ch + win->y_margin_top + win->y_margin_bottom;

	wind_get_grect(DESK, WF_WORKXYWH, &screen);

	if (gl_profile.viewer.adjust_winsize)
	{
		wind_calc_grect(WC_WORK, win->kind, curr, curr);

		if (curr->g_h < minh)
			curr->g_h = minh;
		if (curr->g_w < minw)
			curr->g_w = minw;
		
		curr->g_w = ((curr->g_w - minw + win->x_raster - 1) / win->x_raster) * win->x_raster + minw;
		curr->g_h = ((curr->g_h - minh + win->y_raster - 1) / win->y_raster) * win->y_raster + minh;
		
		wind_calc_grect(WC_BORDER, win->kind, curr, curr);

		if (curr->g_x + curr->g_w > screen.g_x + screen.g_w)
		{
			_WORD val = curr->g_x - screen.g_x + curr->g_w - screen.g_w;

			curr->g_w -= (val / win->x_raster) * win->x_raster;
		}

		if (curr->g_y + curr->g_h > screen.g_y + screen.g_h)
		{
			_WORD val = curr->g_y - screen.g_y + curr->g_h - screen.g_h;

			curr->g_h -= (val / win->y_raster) * win->y_raster;
		}
	} else
	{
		wind_calc_grect(WC_WORK, win->kind, curr, curr);

		curr->g_w = ((curr->g_w - minw + win->x_raster - 1) / win->x_raster) * win->x_raster + minw;
		curr->g_h = ((curr->g_h - minh + win->y_raster - 1) / win->y_raster) * win->y_raster + minh;
		wind_calc_grect(WC_BORDER, win->kind, curr, curr);
	}
}

/*** ---------------------------------------------------------------------- ***/

void ReInitWindow(WINDOW_DATA *win, gboolean prep)
{
	DOCUMENT *doc = win->data;
	_WORD visible_lines;
	GRECT curr;
	
	win->x_raster = font_cw;
	win->y_raster = font_ch;
	if (prep)
		doc->prepNode(win, win->displayed_node);
	hv_set_title(win, win->title);
	win->selection.valid = FALSE;

	if (!(win->status & WIS_OPEN))
		return;

	/* window size: at least 5 columns and 1 line */
	ResizeWindow(win, max(win->docsize.w, 5 * win->x_raster), max(win->docsize.h, 1 * win->y_raster));

	wind_get_grect(win->whandle, WF_CURRXYWH, &curr);

	/* adjust window size to new dimensions */
	if (gl_profile.viewer.adjust_winsize)
	{
		GRECT screen;

		wind_get_grect(DESK, WF_WORKXYWH, &screen);

		curr.g_h = screen.g_h;
	}
	calc_opensize(win, &curr);
	wind_set_grect(win->whandle, WF_CURRXYWH, &curr);

	WindowCalcScroll(win);
	visible_lines = (win->scroll.g_h + win->y_raster - 1) / win->y_raster;

	win->docsize.y = min(win->docsize.h - visible_lines * win->y_raster, doc->start_line * win->y_raster);
	win->docsize.y = max(0, win->docsize.y);
	win->docsize.x = 0;

	if (!gl_profile.viewer.intelligent_fuller)
		wind_get_grect(0, WF_WORKXYWH, &win->full);

	SetWindowSlider(win);
	SendRedraw(win);
}

/*** ---------------------------------------------------------------------- ***/

gboolean HelpWindow(WINDOW_DATA *win, _WORD obj, void *data)
{
	DOCUMENT *doc = (DOCUMENT *) win->data;
	
	switch (obj)
	{
	case WIND_INIT:
		/* use font size for raster */
		win->x_raster = font_cw;
		win->y_raster = font_ch;

		win->y_margin_top = gl_profile.viewer.text_yoffset;
		win->x_margin_left = gl_profile.viewer.text_xoffset;
		win->x_margin_right = gl_profile.viewer.text_xoffset;
		
		/* window size: at least 5 columns and 1 line */
		win->docsize.w = max(win->docsize.w, 5 * win->x_raster);
		win->docsize.h = max(win->docsize.h, 1 * win->y_raster);

		win->docsize.x = 0;
		win->docsize.y = doc->start_line * win->y_raster;

#if USE_DOCUMENTHISTORY
		DhstAddFile(doc->path);
#endif
		break;
		
	case WIND_EXIT:
		hypdoc_unref(doc);
		g_free(win->autolocator);
		win->autolocator = NULL;
		break;
		
	case WIND_OPEN:
		graf_mouse(ARROW, NULL);

		SendAV_ACCWINDOPEN(win->whandle);

		if (!gl_profile.viewer.intelligent_fuller)
			wind_get_grect(0, WF_WORKXYWH, &win->full);
		break;
		
	case WIND_OPENSIZE:	/* initial window size */
		{
			GRECT *open_size = (GRECT *) data;
			GRECT screen;
	
			wind_get_grect(DESK, WF_WORKXYWH, &screen);
	
			if (!gl_profile.viewer.adjust_winsize)
				*open_size = screen;
	
			/* default X-coordinate specified? */
			if (gl_profile.viewer.win_x && (gl_profile.viewer.win_x <= screen.g_x + screen.g_w - 50))
				open_size->g_x = gl_profile.viewer.win_x;
	
			/* default Y-Koordinate specified? */
			if (gl_profile.viewer.win_y && (gl_profile.viewer.win_y <= screen.g_y + screen.g_h - 70))
				open_size->g_y = gl_profile.viewer.win_y;

			if (gl_profile.viewer.win_w != 0 && (gl_profile.viewer.win_w < open_size->g_w))
				open_size->g_w = gl_profile.viewer.win_w;
			
			if (gl_profile.viewer.win_h != 0 && (gl_profile.viewer.win_h < open_size->g_h))
				open_size->g_h = gl_profile.viewer.win_h;
			
			/* window width or height specified? */
			if (!gl_profile.viewer.adjust_winsize)
			{
				if (gl_profile.viewer.win_w == 0)
				{
					_WORD maxw = HYP_STGUIDE_DEFAULT_LINEWIDTH * font_cw + win->x_margin_left + win->x_margin_right;
					wind_calc_grect(WC_WORK, win->kind, open_size, open_size);
					if (open_size->g_w > maxw)
						open_size->g_w = maxw;
					wind_calc_grect(WC_BORDER, win->kind, open_size, open_size);
				}
			}
			calc_opensize(win, open_size);
		}
		return FALSE;
		
	case WIND_CLOSE:
		if (win->whandle > 0)
		{
			GRECT gr;
			wind_get_grect(win->whandle, WF_CURRXYWH, &gr);
			gl_profile.viewer.win_x = gr.g_x;
			gl_profile.viewer.win_y = gr.g_y;
			gl_profile.viewer.win_w = gr.g_w;
			gl_profile.viewer.win_h = gr.g_h;
			HypProfile_SetChanged();
			SendAV_ACCWINDCLOSED(win->whandle);
		}
		/*
		 * save the path of the last window closed,
		 * so it can be reopenend again on receive of AC_OPEN.
		 * Only need to do this when running as accessory,
		 * since a regular application will exit when all
		 * windows are closed.
		 */
		if (!_app)
		{
			/*
			 * is this the last window?
			 * == 1 because ptr has not yet been removed from list
			 */
			if (count_window() == 1)
			{
				DOCUMENT *doc;

				doc = (DOCUMENT *) win->data;
				gl_profile.viewer.last_node = doc->getNodeProc(win);
				g_free(gl_profile.viewer.last_file);
				gl_profile.viewer.last_file = g_strdup(doc->path);
			}
		}
		break;
		
	case WIND_REDRAW:
		{
			_WORD pxy[4];
			GRECT *box = (GRECT *) data;
			GRECT scroll;
			
			pxy[0] = box->g_x;
			pxy[1] = box->g_y;
			pxy[2] = box->g_x + box->g_w - 1;
			pxy[3] = box->g_y + box->g_h - 1;
			vsf_color(vdi_handle, viewer_colors.background);
			vsf_interior(vdi_handle, FIS_SOLID);
			vswr_mode(vdi_handle, MD_REPLACE);
	
			vs_clip(vdi_handle, TRUE, pxy);	/* clipping ON */
			vr_recfl(vdi_handle, pxy);		/* clear beackground */
	
			scroll = win->scroll;
			if (rc_intersect(box, &scroll))
			{
				pxy[0] = scroll.g_x;
				pxy[1] = scroll.g_y;
				pxy[2] = scroll.g_x + scroll.g_w - 1;
				pxy[3] = scroll.g_y + scroll.g_h - 1;
				vs_clip(vdi_handle, TRUE, pxy);	/* clipping ON */
				doc->displayProc(win);
			}
			vs_clip(vdi_handle, FALSE, pxy);	/* clipping OFF */
		}
		break;
		
	case WIND_SIZED:		/* window size has changed */
		{
			GRECT *out = (GRECT *) data;
			GRECT in;
			
			wind_calc_grect(WC_WORK, win->kind, out, &in);	/* calculate working area */
			
			in.g_w -= (in.g_w - win->x_margin_left - win->x_margin_right) % win->x_raster;	/* align width to raster */
			in.g_h -= (in.g_h - win->y_margin_top - win->y_margin_bottom) % win->y_raster;	/* align height to raster */
			
			wind_calc_grect(WC_BORDER, win->kind, &in, out);	/* calculate window frame */
		}
		break;
		
	case WIND_FULLED:		/* fuller activated */
		{
			GRECT *out = (GRECT *) data;
			GRECT in;
			GRECT screen;
			_WORD minw, minh;
			
			wind_get_grect(DESK, WF_WORKXYWH, &screen);
	
			wind_calc_grect(WC_WORK, win->kind, out, &in);	/* calculate working area */
	
			minw = win->x_margin_left + win->x_margin_right;
			minh = win->y_margin_top + win->y_margin_bottom;
			in.g_w -= (in.g_w - minw) % win->x_raster;	/* align window width */
			in.g_h -= (in.g_h - minh) % win->y_raster;	/* align window height */
	
			wind_calc_grect(WC_BORDER, win->kind, &in, out);	/* calculate window frame */
	
			if (gl_profile.viewer.intelligent_fuller && ((win->status & WIS_FULL) == 0))
			{
				out->g_x = win->last.g_x;
				out->g_y = win->last.g_y;
				if (out->g_x + out->g_w > screen.g_x + screen.g_w)
					out->g_x -= out->g_x + out->g_w - (screen.g_x + screen.g_w);
				else if (out->g_x < screen.g_x)
					out->g_x = screen.g_x;
				if (out->g_y + out->g_h > screen.g_y + screen.g_h)
					out->g_y -= out->g_y + out->g_h - (screen.g_y + screen.g_h);
				else if (out->g_y < screen.g_y)
					out->g_y = screen.g_y;
			}
		}
		break;
		
	case WIND_KEYPRESS:
#if 0
		{
			EVNT *event = (EVNT *) data;
			_WORD scan = (event->key >> 8) & 0xff;
			_WORD ascii = event->key & 0xff;
	
			WindowCalcScroll(win);
	
			event->mwhich &= ~MU_KEYBD;
	
			if ((event->kstate & KbSHIFT) && (event->kstate & KbCTRL))
			{
				if (scan == KbUP || scan == KbLEFT)
					ToolbarClick(win, TO_PREV);
				else if (scan == KbDOWN || scan == KbRIGHT)
					ToolbarClick(win, TO_NEXT);
				else if (ascii == 'V')
					BlockPaste(win, !gl_profile.viewer.clipbrd_new_window);
				else
					event->mwhich |= MU_KEYBD;
			} else if (event->kstate & KbSHIFT)
			{
				WindowCalcScroll(win);
				switch (scan)
				{
				case KbLEFT:
					ScrollWindow(win, -win->scroll.g_w / win->x_raster, 0);
					break;
				case KbRIGHT:
					ScrollWindow(win, win->scroll.g_w / win->x_raster, 0);
					break;
				case KbUP:
					ScrollWindow(win, 0, -win->scroll.g_h / win->y_raster);
					break;
				case KbDOWN:
					ScrollWindow(win, 0, win->scroll.g_h / win->y_raster);
					break;
				case KbHOME:
				case KbEND:
					win->docsize.x = 0;
					win->docsize.y = win->docsize.h - win->scroll.g_h;
					SetWindowSlider(win);
					SendRedraw(win);
					break;
				case KbPAGEUP:
					ScrollWindow(win, 0, -win->scroll.g_h / win->y_raster);
					break;
				case KbPAGEDOWN:
					ScrollWindow(win, 0, win->scroll.g_h / win->y_raster);
					break;
				case KbF11:
				case KbF12:
				case KbF13:
				case KbF14:
				case KbF15:
				case KbF16:
				case KbF17:
				case KbF18:
				case KbF19:
				case KbF20:
					MarkerSave(win, scan - KbF11);
					break;
				default:
					event->mwhich |= MU_KEYBD;
					break;
				}
			} else if (event->kstate & KbALT)
			{
				switch (ascii)
				{
				case 'D':
					GotoDefaultFile(win);
					break;
				case 'E':
					RemoveAllHistoryEntries(win);
					ToolbarUpdate(win, TRUE);
					break;
				case 'K':
					GotoCatalog(win);
					break;
				case 'R':
					BlockOperation(win, CO_REMARKER);
					break;
				case 'T':
					GoThisButton(win, TO_HOME);
					break;
				case 'X':
					GotoIndex(win);
					break;
				case 'Z':
					BlockOperation(win, CO_SELECT_FONT);
					break;
				}	
			} else if (event->kstate & KbCTRL)
			{
				if (ascii == 'A')
				{
					BlockOperation(win, CO_SELECT_ALL);
				} else if (ascii == 'C')
				{
					BlockOperation(win, CO_COPY);
				} else if (ascii == 'I')
				{
					DocumentInfos(win);
				} else if (ascii == 'F')
				{
					BlockOperation(win, CO_SEARCH);
				} else if (ascii == 'G')
				{
					BlockOperation(win, CO_SEARCH_AGAIN);
				} else if (ascii == 'O')
				{
					ToolbarClick(win, TO_LOAD);
				} else if (ascii == 'V')
				{
					if (doc->buttons.searchbox)
						AutoLocatorPaste(win);
					else
						BlockOperation(win, CO_PASTE);
				} else if (ascii == 'Z')
				{
					BlockOperation(win, CO_SWITCH_FONT);
				} else if (scan == KbUP)
				{
					ScrollWindow(win, 0, -win->scroll.g_h / win->y_raster);
				} else if (scan == KbDOWN)
				{
					ScrollWindow(win, 0, win->scroll.g_h / win->y_raster);
				} else if (scan == KbLEFT)
				{
					ToolbarClick(win, TO_PREV);
				} else if (scan == KbRIGHT)
				{
					ToolbarClick(win, TO_NEXT);
				} else if (scan >= KbF1 && scan <= KbF10)
				{
					MarkerShow(win, scan - KbF1, TRUE);
				} else
				{
					event->mwhich |= MU_KEYBD;
				}
			} else if (event->kstate & KbNUM)
			{
				if (ascii == '-')
					ToolbarClick(win, TO_PREV);
				else if (ascii == '+')
					ToolbarClick(win, TO_NEXT);
				else
					event->mwhich |= MU_KEYBD;
			} else if (event->kstate == 0)
			{
				WindowCalcScroll(win);
				switch (scan)
				{
				case KbLEFT:
					ScrollWindow(win, -win->x_speed, 0);
					break;
				case KbRIGHT:
					ScrollWindow(win, win->x_speed, 0);
					break;
				case KbUP:
					ScrollWindow(win, 0, -win->y_speed);
					break;
				case KbDOWN:
					ScrollWindow(win, 0, win->y_speed);
					break;
				case KbPAGEUP:
					ScrollWindow(win, 0, -win->scroll.g_h / win->y_raster);
					break;
				case KbPAGEDOWN:
					ScrollWindow(win, 0, win->scroll.g_h / win->y_raster);
					break;
				case KbHOME:
					if (win->docsize.y)
					{
						win->docsize.x = 0;
						win->docsize.y = 0;
						SetWindowSlider(win);
						SendRedraw(win);
					}
					break;
				case KbEND:
					win->docsize.x = 0;
					win->docsize.y = win->docsize.h - win->scroll.g_h;
					SetWindowSlider(win);
					SendRedraw(win);
					break;
				case KbHELP:
					ToolbarClick(win, TO_HELP);
					break;
				case KbUNDO:
					ToolbarClick(win, TO_BACK);
					break;
				case KbF1:
				case KbF2:
				case KbF3:
				case KbF4:
				case KbF5:
				case KbF6:
				case KbF7:
				case KbF8:
				case KbF9:
				case KbF10:
					MarkerShow(win, scan - KbF1, FALSE);
					break;
				default:
					if ((ascii == 27 || ascii == 8) && !(doc->buttons.searchbox))
						ToolbarClick(win, TO_BACK);
					else
						event->mwhich |= MU_KEYBD;
					break;
				}
			} else
			{
				event->mwhich |= MU_KEYBD;
			}
		}
#endif
		break;
		
	case WIND_DRAGDROPFORMAT:
#if USE_DRAGDROP
		DD_DialogGetFormat(NULL, 0, (unsigned long *)data);
		break;
#else
		return FALSE;
#endif
		
#if USE_DRAGDROP
	case WIND_DRAGDROP:
		{
			char *ptr = g_strdup((char *)data);
			char **argv = split_av_parameter(ptr);
			
			if (argv && !empty(argv[0]))
			{
				char *filename = argv[0];
				tv_open_window(filename);
			}
			g_strfreev(argv);
			g_free(ptr);
		}
		break;
#endif
		
	case WIND_CLICK:
#if 0
		{
			EVNT *event = (EVNT *) data;
	
			if (event->mbutton & 1)			/* left button */
			{
				EVNTDATA d;

				graf_mkstate(&event->mx, &event->my, &event->mbutton, &event->kstate);
	
				d.x = event->mx;
				d.y = event->my;
				d.bstate = event->mbutton;
				d.kstate = event->kstate;
				if ((event->mbutton & 1) ||		/* button still pressed? */
					(event->kstate & KbSHIFT))	/* or shift pressed? */
				{
					MouseSelection(win, &d);
				} else
				{
					RemoveSelection(win);
					if (doc->type == HYP_FT_HYP)
						HypClick(win, &d);
				}
			}
		}
#endif
		break;
		
	case WIND_TBUPDATE:
		break;
		
	case WIND_TBCLICK:
		break;
		
	case WIND_ICONIFY:
		hv_set_title(win, hyp_basename(doc->path));
		break;
		
	case WIND_UNICONIFY:
		hv_set_title(win, win->title);
		break;
	
	case WIND_NEWTOP:
	case WIND_ONTOP:
		break;
		
	case WIND_TOPPED:
		wind_set_top(win->whandle);
		return FALSE;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *hv_win_new(DOCUMENT *doc, gboolean popup)
{
	WINDOW_DATA *win;
	
	UNUSED(popup);
	win = CreateWindow(HelpWindow, NAME | CLOSER | FULLER | MOVER | SIZER | UPARROW | DNARROW |
					   VSLIDE | LFARROW | RTARROW | HSLIDE | SMALLER, doc->path, -1, -1, doc);
	return win;
}

/*** ---------------------------------------------------------------------- ***/

void hv_win_open(WINDOW_DATA *win)
{
	if (win == NULL)
		return;
	if (win->status & WIS_ICONIFY)
		UniconifyWindow(win);
	if (!(win->status & WIS_OPEN))
		OpenWindow(win);
	wind_set_top(win->whandle);
}
