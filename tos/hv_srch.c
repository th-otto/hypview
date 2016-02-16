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

static LIST_BOX *entrie_box;
static short nclicks;
static DIALOG *SearchResult_Dialog;
static RESULT_ENTRY *Result_List;
static REF_FILE *allref;


/* Objects of the list boxes */
#define	NO_ENTRIE	10
static _WORD const entrie_ctrl[5] = { SR_BOX, SR_FSTL_UP, SR_FSTL_DOWN, SR_FSTL_BACK, SR_FSTL_WHITE };
static _WORD const entrie_objs[10] = { SR_FSTL_0, SR_FSTL_1, SR_FSTL_2, SR_FSTL_3, SR_FSTL_4, SR_FSTL_5, SR_FSTL_6, SR_FSTL_7, SR_FSTL_8, SR_FSTL_9 };


/*----------------------------------------------------------------------------------------*/
/* n entry in the list box has been selected...				                              */
/* Function result:	-								                                      */
/*  box:				Pointer to the list box structure			                      */
/*  tree:				Pointer to the object tree		                                  */
/*  item:				Pointer to the entry		                                      */
/*  user_data:			...						                                          */
/*  obj_index:			Index of the object, poss. | 0x8000, poss. 0 (not visible)        */
/*  last_state:			The previous state				                                  */
/*----------------------------------------------------------------------------------------*/

static void __CDECL select_item(struct SLCT_ITEM_args args)
{
	WINDOW_DATA *win = (WINDOW_DATA *)args.user_data;
	RESULT_ENTRY *my_item = (RESULT_ENTRY *)args.item;

	if (nclicks > 1)
	{
		if ((win = OpenFileInWindow(win, my_item->path, my_item->node_name, HYP_NOINDEX, FALSE, FALSE, FALSE)) != NULL)
		{
			if (my_item->lineno > 0)
			{
				DOCUMENT *doc;

				doc = (DOCUMENT *)win->data;

				graf_mouse(BUSY_BEE, NULL);
				if (doc->gotoNodeProc(win, my_item->node_name, 0))
					doc->start_line = my_item->lineno;
				graf_mouse(ARROW, NULL);
				ReInitWindow(win, FALSE);
			}
		}
		SendCloseDialog(SearchResult_Dialog);
	}
}

/*----------------------------------------------------------------------------------------*/
/* Set string and object status of a GTEXT object in the list box					      */
/*  Function result:	Number of the start object to be drawn							  */
/*  box:				Pointer to the list box structure 								  */
/*  tree:				Pointer to the object tree										  */
/*  item:				Pointer to the entry											  */
/*  index:				Object number													  */
/*  user_data:			...																  */
/*  rect:				GRECT for selection/deselection or 0L (not alterable)		      */
/*  offset:																				  */
/*----------------------------------------------------------------------------------------*/

static _WORD __CDECL set_str_item(struct SET_ITEM_args args)
{
	char *ptext;
	char *str;

	ptext = args.tree[args.obj_index].ob_spec.tedinfo->te_ptext;	/* Pointer to string of the GTEXT object */

	if (args.item)
	{
		if (args.item->selected)
			args.tree[args.obj_index].ob_state |= OS_SELECTED;
		else
			args.tree[args.obj_index].ob_state &= ~OS_SELECTED;

		str = ((RESULT_ENTRY *)args.item)->str;

		if (args.first == 0)
		{
			if (*ptext)
				*ptext++ = ' ';
		} else
		{
			args.first -= 1;
		}

		if (args.first <= (short)strlen(str))
		{
			str += args.first;

			while (*ptext && *str)
				*ptext++ = *str++;
		}
	} else
	{
		args.tree[args.obj_index].ob_state &= ~OS_SELECTED;
	}

	while (*ptext)
		*ptext++ = ' ';					/* Pad string end with space characters */

	return args.obj_index;				/* Object number of the start object */
}

/*----------------------------------------------------------------------------------------
 * Create entry for the listbox
 *----------------------------------------------------------------------------------------*/

static void make_results(RESULT_ENTRY *ptr)
{
	int i;

	while (ptr)
	{
		char *name;
		char *str;
		
		if (ptr->label_name)
			str = g_strdup_printf("%s \xe2\x88\x99 %s", ptr->node_name, ptr->label_name);
		else if (ptr->alias_name)
			str = g_strdup_printf("%s \xe2\x88\x99 %s", ptr->node_name, ptr->alias_name);
		else
			str = g_strdup(ptr->node_name);
		name = hyp_utf8_to_charset(hyp_get_current_charset(), str, STR0TERM, NULL);
		strncpy(ptr->str, name, 30);
		ptr->str[30] = 0;
		g_free(name);
		g_free(str);
		i = (int)strlen(ptr->str);
		if (i < 30)
			memset(ptr->str + i, ' ', 30 - i);
		name = hyp_utf8_to_charset(hyp_get_current_charset(), ptr->dbase_description, STR0TERM, NULL);
		strncat(ptr->str, name, 255 - 30);
		g_free(name);
		ptr->str[255] = 0;
		ptr->item.selected = 0;
		ptr = (RESULT_ENTRY *)ptr->item.next;
	}
}

/*----------------------------------------------------------------------------------------*/
/* Service routine for window dialog 													  */
/* Function result:	0: Close dialog     1: Continue								          */
/*  dialog:				Pointer to the dialog structure									  */
/*  events:				Pointer to EVNT structure or 0L								      */
/*  obj:				Number of the object or event number							  */
/*  clicks:				Number of mouse clicks											  */
/*  data:				Pointer to additional data									      */
/*----------------------------------------------------------------------------------------*/

static _WORD __CDECL SearchResultHandle(struct HNDL_OBJ_args args)
{
	OBJECT *tree;
	GRECT rect;
	DIALOG_DATA *dial;
	WINDOW_DATA *win;
	
	dial = (DIALOG_DATA *)wdlg_get_udata(args.dialog);
	win = (WINDOW_DATA *)dial->data;
	wdlg_get_tree(args.dialog, &tree, &rect);

	switch (args.obj)
	{
	case HNDL_INIT:
		/* create a vertical listbox with auto-scrolling and real-time-slider */
		make_results(Result_List);
		entrie_box = lbox_create(tree,
			select_item, set_str_item, (LBOX_ITEM *) Result_List,
			NO_ENTRIE, 0, entrie_ctrl, entrie_objs,
			LBOX_VERT + LBOX_AUTO + LBOX_AUTOSLCT + LBOX_REAL + LBOX_SNGL,
			40, win, SearchResult_Dialog, 0, 0, 0, 0);
		break;
	case HNDL_CLSD:
		lbox_free_items(entrie_box);
		lbox_delete(entrie_box);
		ref_freeresults(Result_List);
		Result_List = NULL;
		ref_close(allref);
		allref = NULL;
		return 0;
	case HNDL_MESG:
		SpecialMessageEvents(args.dialog, args.events);
		break;
	case SR_ABORT:
		tree[args.obj].ob_state &= (~OS_SELECTED);
		SendCloseDialog(args.dialog);
		break;
	case SR_FSTL_UP:
	case SR_FSTL_BACK:
	case SR_FSTL_WHITE:
	case SR_FSTL_DOWN:
	case SR_BOX:
		lbox_do(entrie_box, args.obj);
		break;
	case SR_FSTL_0:
	case SR_FSTL_1:
	case SR_FSTL_2:
	case SR_FSTL_3:
	case SR_FSTL_4:
	case SR_FSTL_5:
	case SR_FSTL_6:
	case SR_FSTL_7:
	case SR_FSTL_8:
	case SR_FSTL_9:
		nclicks = args.clicks;
		lbox_do(entrie_box, args.obj);
		break;
	}
	return 1;
}

/*----------------------------------------------------------------------------------------
 * Prepare dialog for search results
 *----------------------------------------------------------------------------------------*/

static gboolean SearchResult(WINDOW_DATA *win, RESULT_ENTRY **result_list)
{
	nclicks = 0;
	SearchResult_Dialog = OpenDialog(SearchResultHandle, rs_tree(SEARCH_RESULT), _("Search Result..."), -1, -1, win);
	if (SearchResult_Dialog == NULL)
	{
		ref_freeresults(*result_list);
		*result_list = NULL;
		return FALSE;
	}
	return TRUE;
}


/*----------------------------------------------------------------------------------------*
 * search a string using <all.ref>
 *----------------------------------------------------------------------------------------*/
 
static void print_results(RESULT_ENTRY *ptr)
{
	while (ptr)
	{
		HYP_DBG(("Path=%s", printnull(ptr->path)));
		HYP_DBG(("Node:%s", printnull(ptr->node_name)));
		HYP_DBG(("Label:%d", ptr->is_label));
		HYP_DBG(("Line:%d", ptr->lineno));
		HYP_DBG(("Descr:%s", printnull(ptr->dbase_description)));
		ptr = (RESULT_ENTRY *)ptr->item.next;
	}
}

/*----------------------------------------------------------------------------------------*/

WINDOW_DATA *search_allref(WINDOW_DATA *win, const char *string, gboolean no_message)
{
	int ret;
	long results = 0;
	OBJECT *tree = NULL;
	GRECT big;
	gboolean aborted;
	
	/* abort if no all.ref is defined */
	if (empty(gl_profile.general.all_ref))
	{
		HYP_DBG(("No ref file defined"));
		return win;
	}

	graf_mouse(BUSY_BEE, NULL);
	
	if (!gl_profile.viewer.norefbox)
	{
		tree = rs_tree(REFBOX);
		memset(tree[REFBOX_STRING].ob_spec.tedinfo->te_ptext, 0, tree[REFBOX_STRING].ob_spec.tedinfo->te_txtlen);
		strncpy(tree[REFBOX_STRING].ob_spec.tedinfo->te_ptext, string, tree[REFBOX_STRING].ob_spec.tedinfo->te_txtlen - 1);
		form_center_grect(tree, &big);
		form_dial_grect(FMD_START, &big, &big);
		objc_draw_grect(tree, ROOT, MAX_DEPTH, &big);
	}
	
	if (allref == NULL)
	{
		char *filename;
		
		/* open and load REF file */
		filename = path_subst(gl_profile.general.all_ref);
		ret = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (ret < 0)
		{
			HYP_DBG(("Error %s in %s", strerror(errno), filename));
		} else
		{
			allref = ref_load(filename, ret, FALSE);
			hyp_utf8_close(ret);
		}
		g_free(filename);
	}

	Result_List = ref_findall(allref, string, &results, &aborted);

	if (tree)
		form_dial_grect(FMD_FINISH, &big, &big);
	
	graf_mouse(ARROW, NULL);

	/* error loading file? */
	if (allref == NULL)
	{
		return win;
	}
	
	print_results(Result_List);

	/* open results */
	if (results > 0)
	{
		/* only one result */
		if (results == 1)
		{
			if ((win = OpenFileInWindow(win, Result_List->path, Result_List->node_name, HYP_NOINDEX, FALSE, FALSE, FALSE)) != NULL)
			{
				if (Result_List->lineno > 0)
					win->data->start_line = Result_List->lineno;
			}
			ref_freeresults(Result_List);
			Result_List = NULL;
		} else
		{
			if (has_listbox_dialog())
			{
				if (!SearchResult(win, &Result_List))
				{
					ref_freeresults(Result_List);
					Result_List = NULL;
					ref_close(allref);
					allref = NULL;
				}
			} else
			{
				/* FIXME: just open first result for now */
				if ((win = OpenFileInWindow(win, Result_List->path, Result_List->node_name, HYP_NOINDEX, FALSE, FALSE, FALSE)) != NULL)
				{
					if (Result_List->lineno > 0)
						win->data->start_line = Result_List->lineno;
				}
				ref_freeresults(Result_List);
				Result_List = NULL;
				ref_close(allref);
				allref = NULL;
			}
		}
	} else
	{
		if (!no_message && !aborted)
		{
			char *str;
			char *name;
			gboolean converror = FALSE;
			
			name = hyp_utf8_to_charset(hyp_get_current_charset(), string, STR0TERM, &converror);
			str = g_strdup_printf(rs_string(WARN_NORESULT), name);
			form_alert(1, str);
			g_free(name);
			g_free(str);
		}
	
		ref_close(allref);
		allref = NULL;
	}
	return win;
}
