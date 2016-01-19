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


#define MAX_MARKEN		10
#define UNKNOWN_LEN		10
#define PATH_LEN		128
#define NODE_LEN		40


typedef struct
{
	hyp_nodenr node_num;
	short line;
	char unknown[UNKNOWN_LEN];
	char path[PATH_LEN];				/* full path */
	char node_name[NODE_LEN];			/* display title */
} MARKEN;

static gboolean marken_change;
static MARKEN marken[MAX_MARKEN];

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void MarkerDelete(short num)
{
	char *dst;
	
	memset(&marken[num], 0, sizeof(MARKEN));
	marken[num].node_num = HYP_NOINDEX;
	dst = marken[num].node_name;
	*dst++ = ' ';
	strcpy(dst, _("free"));
}

/*** ---------------------------------------------------------------------- ***/

void MarkerSave(DOCUMENT *doc, short num)
{
	WINDOW_DATA *win = doc->window;
	const char *src;
	char *dst, *end;

	/* avoid illegal parameters */
	if (num < 0 || num >= MAX_MARKEN)
		return;

	marken[num].node_num = doc->getNodeProc(doc);
	marken[num].line = (short) win->docsize.y;
	strncpy(marken[num].path, doc->path, PATH_LEN - 1);
	marken[num].path[PATH_LEN - 1] = 0;

	/* copy marker title */
	src = doc->window_title;
	dst = marken[num].node_name;
	end = &marken[num].node_name[NODE_LEN - 1];
	*dst++ = ' ';
	while (dst < end)
	{
		if (*src)
			*dst++ = *src++;
		else
			break;
	}
	if (dst < end)
		*dst++ = ' ';
	if (dst < end)
		*dst++ = ' ';
	*dst = 0;

	{
		char ZStr[255];
		long len;

		strcpy(ZStr, "(");
		src = hyp_basename(marken[num].path);
		strcat(ZStr, src);
		strcat(ZStr, ") ");
		len = strlen(ZStr);
		if (strlen(marken[num].node_name) + len > NODE_LEN)
			marken[num].node_name[NODE_LEN - len] = '\0';
		strcat(marken[num].node_name, ZStr);
	}

	marken_change = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void MarkerShow(DOCUMENT *doc, short num, gboolean new_window)
{
	WINDOW_DATA *win = doc->window;

	/* avoid illegal parameters */
	if (num < 0 || num >= MAX_MARKEN)
		return;

	if (marken[num].node_num != HYP_NOINDEX)
	{
		if (new_window)
			win = OpenFileNewWindow(marken[num].path, NULL, marken[num].node_num, TRUE);
		else
			win = OpenFileSameWindow(win, marken[num].path, NULL, FALSE, FALSE);
		if (win != NULL)
		{
			doc = win->data;
			GotoPage(doc, marken[num].node_num, marken[num].line, FALSE);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void MarkerPopup(DOCUMENT *doc, short x, short y)
{
	OBJECT *tree = rs_tree(EMPTYPOPUP);
	short i, sel, ob, h;
	EVNTDATA event;
	size_t len = 0;
	static char eos = 0;

	h = 0;
	for (i = 0, ob = tree[ROOT].ob_head; i < MAX_MARKEN && ob != ROOT; i++, ob = tree[ob].ob_next)
	{
		tree[ob].ob_spec.free_string = marken[i].node_name;
		tree[ob].ob_flags = OF_SELECTABLE;

		len = max(strlen(marken[i].node_name), len);
		h = max(h, tree[ob].ob_y + tree[ob].ob_height);
	}

	/* Hide unused entries */
	while (ob != ROOT)
	{
		tree[ob].ob_flags = OF_HIDETREE;
		tree[ob].ob_spec.free_string = &eos;
		ob = tree[ob].ob_next;
	}
	len = max(len, 14);

	len = len * pwchar;
	tree[EM_BACK].ob_x = x;
	tree[EM_BACK].ob_y = y;
	tree[EM_BACK].ob_width = (short) len;
	tree[EM_BACK].ob_height = h;

	/* set same width for all entries */
	for (i = tree[ROOT].ob_head; i != ROOT; i = tree[i].ob_next)
	{
		tree[i].ob_width = (short) len;
		tree[i].ob_height = phchar;
	}

	sel = popup_select(tree, 0, 0);
	graf_mkstate(&event.x, &event.y, &event.bstate, &event.kstate);

	/* reset strings; the popup is used also by other routines */
	for (i = tree[ROOT].ob_head; i != ROOT; i = tree[i].ob_next)
	{
		tree[i].ob_flags = OF_SELECTABLE;
		tree[i].ob_spec.free_string = NULL;
	}

	if (sel > 0)
	{
		for (i = 0, ob = tree[ROOT].ob_head; i < MAX_MARKEN && ob != ROOT; ob = tree[ob].ob_next, i++)
			if (ob == sel)
				break;
		if (i < MAX_MARKEN)
		{
			sel = i;
			if (event.kstate & KbSHIFT)
			{
				MarkerSave(doc, sel);
			} else if (marken[sel].node_num == HYP_NOINDEX)
			{
				char *buff;
	
				buff = g_strdup_printf(rs_string(ASK_SETMARK), doc->window_title);
				if (form_alert(1, buff) == 1)
					MarkerSave(doc, sel);
				g_free(buff);
			} else
			{
				if (event.kstate & KbALT)
				{
					char *buff;
	
					buff = g_strdup_printf(rs_string(WARN_ERASEMARK), marken[sel].node_name + 1);
					if (form_alert(1, buff) == 1)
					{
						MarkerDelete(sel);
						marken_change = TRUE;
					}
					g_free(buff);
				} else
				{
					MarkerShow(doc, sel, (event.kstate & KbCTRL) != 0);
				}
			}
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void MarkerSaveToDisk(void)
{
	char *filename;
	
	if (!marken_change)
		return;

	if (!empty(gl_profile.viewer.marker_path))
	{
		int ret;

		if (gl_profile.viewer.marken_save_ask)
		{
			if (form_alert(1, rs_string(ASK_SAVEMARKFILE)) == 2)
				return;
		}
		filename = path_subst(gl_profile.viewer.marker_path);
		ret = open(filename, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0644);
		if (ret >= 0)
		{
			write(ret, marken, sizeof(MARKEN) * MAX_MARKEN);
			close(ret);
		} else
		{
			HYP_DBG(("Error %ld: saving %s", ret, printnull(filename)));
		}
		g_free(filename);
	}
}

/*** ---------------------------------------------------------------------- ***/

void MarkerInit(void)
{
	short i;
	int ret;
	char *filename;
	
	/* initialize markers */
	for (i = 0; i < MAX_MARKEN; i++)
	{
		MarkerDelete(i);
	}

	/* load file if it exists */
	if (!empty(gl_profile.viewer.marker_path))
	{
		filename = path_subst(gl_profile.viewer.marker_path);
		ret = open(filename, O_RDONLY | O_BINARY);
		if (ret >= 0)
		{
			read(ret, marken, sizeof(MARKEN) * MAX_MARKEN);
			for (i = 0; i < MAX_MARKEN; i++)
			{
				if (marken[i].node_name[0] == 0)
					MarkerDelete(i);
			}
			close(ret);
		}
		g_free(filename);
	}

	marken_change = FALSE;
}
