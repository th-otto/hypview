#include "hv_defs.h"
#include "hypdebug.h"
#include "resource.rh"
#include <commctrl.h>

#define ListView_InsertColumnW(hwnd,iCol,pcol) (int)SNDMSG((hwnd),LVM_INSERTCOLUMNW,(WPARAM)(int)(iCol),(LPARAM)(pcol))
#define ListView_InsertItemW(hwnd,pitem) (int)SNDMSG((hwnd),LVM_INSERTITEMW,0,(LPARAM)(pitem))
#define ListView_SetItemTextW(hwndLV,i,pitem) SNDMSG((hwndLV),LVM_SETITEMTEXTW,(WPARAM)(i),(LPARAM)(pitem))

#ifndef ListView_SetExtendedListViewStyle
#define ListView_SetExtendedListViewStyle(hwndLV,dw) (DWORD)SNDMSG((hwndLV),LVM_SETEXTENDEDLISTVIEWSTYLE,0,dw)
#endif
#ifndef ListView_SetExtendedListViewStyleEx
#define ListView_SetExtendedListViewStyleEx(hwndLV,dwMask,dw) (DWORD)SNDMSG((hwndLV),LVM_SETEXTENDEDLISTVIEWSTYLE,dwMask,dw)
#endif

/*----------------------------------------------------------------------------------------*
 * search a string using <all.ref>
 *----------------------------------------------------------------------------------------*/
 
static REF_FILE *allref;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

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

struct sr_args {
	WINDOW_DATA *win;
	RESULT_ENTRY *result_list;
	const char *searchstring;
};

/*** ---------------------------------------------------------------------- ***/

static INT_PTR CALLBACK searchresult_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WINDOW_DATA *win;
	WORD notifyCode;

	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_INITDIALOG:
		{
			struct sr_args *args = (struct sr_args *)lParam;
			char *title;
			wchar_t *wtitle;
			LVCOLUMNW col;
			HWND lv = GetDlgItem(hwnd, IDC_SR_BOX);
			RESULT_ENTRY *ptr;
			char *displayname;
			int idx;
			
			win = args->win;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (DWORD_PTR)win);
			CenterWindow(hwnd);
			title = g_strdup_printf(_("Search results: %s"), args->searchstring);
			wtitle = hyp_utf8_to_wchar(title, STR0TERM, NULL);
			SetWindowTextW(hwnd, wtitle);
			g_free(wtitle);
			g_free(title);
			
			/* ListView_SetExtendedListViewStyleEx(lv, LVS_EX_FULLROWSELECT|LVS_EX_TWOCLICKACTIVATE|LVS_EX_TRACKSELECT, LVS_EX_FULLROWSELECT|LVS_EX_TWOCLICKACTIVATE); */
			ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT);
			
			memset(&col, 0, sizeof(col));
			col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			col.fmt = LVCFMT_LEFT;
			col.cx = 30 * 8;
			col.pszText = hyp_utf8_to_wchar(_("Name"), STR0TERM, NULL);
			col.iSubItem = 0;
			ListView_InsertColumnW(lv, 0, &col);
			g_free(col.pszText);
			col.pszText = hyp_utf8_to_wchar(_("Description"), STR0TERM, NULL);
			col.iSubItem = 1;
			col.cx = 255 * 8;
			ListView_InsertColumnW(lv, 1, &col);
			g_free(col.pszText);
			
			for (ptr = args->result_list, idx = 0; ptr; ptr = (RESULT_ENTRY *)ptr->item.next, idx++)
			{
				LVITEMW item;
				int id;
				
				if (ptr->label_name)
					displayname = g_strdup_printf("%s \xe2\x88\x99 %s", ptr->node_name, ptr->label_name);
				else if (ptr->alias_name)
					displayname = g_strdup_printf("%s \xe2\x88\x99 %s", ptr->node_name, ptr->alias_name);
				else
					displayname = g_strdup(ptr->node_name);
				memset(&item, 0, sizeof(item));
				item.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
				item.iItem = idx;
				item.iSubItem = 0;
				item.pszText = hyp_utf8_to_wchar(displayname, STR0TERM, NULL);
				item.lParam = (LPARAM) ptr;
				item.state = 0;
				item.stateMask = LVIS_SELECTED;
				id = ListView_InsertItemW(lv, &item);
				g_free(item.pszText);
				item.iSubItem = 1;
				item.pszText = hyp_utf8_to_wchar(ptr->dbase_description, STR0TERM, NULL);
				ListView_SetItemTextW(lv, id, &item);
				g_free(item.pszText);
				g_free(displayname);
			}
			
		}
		return TRUE;
	
	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		DestroyWindow(hwnd);
		return TRUE;
	
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			DestroyWindow(hwnd);
			return TRUE;
		case IDHELP:
			if (notifyCode == BN_CLICKED)
			{
				Help_Contents(win);
			}
			break;
		}
		break;
	
	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (hdr->idFrom == IDC_SR_BOX && hdr->code == LVN_ITEMACTIVATE)
			{
				NMITEMACTIVATE *item = (NMITEMACTIVATE *)lParam;
				RESULT_ENTRY *ptr = (RESULT_ENTRY *)item->lParam;
				if (ptr)
				{
					if ((win = OpenFileInWindow(win, ptr->path, ptr->node_name, HYP_NOINDEX, TRUE, FALSE, FALSE)) != NULL)
					{
						if (ptr->lineno > 0)
							hv_win_scroll_to_line(win, ptr->lineno);
						EndDialog(hwnd, IDOK);
						DestroyWindow(hwnd);
					}
				}
			}
		}
		break;
		
	case WM_DESTROY:
		break;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static void SearchResult(WINDOW_DATA *win, RESULT_ENTRY *result_list, const char *searchstring)
{
	struct sr_args args;
	args.win = win;
	args.result_list = result_list;
	args.searchstring = searchstring;
	DialogBoxExW(NULL, MAKEINTRESOURCEW(IDD_SEARCHRESULT), win->hwnd, searchresult_dialog, (LPARAM)&args);
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *search_allref(WINDOW_DATA *win, const char *string, gboolean no_message)
{
	int ret;
	long results = 0;
	RESULT_ENTRY *Result_List;
	gboolean aborted;
	HWND splash = NULL;
	
	/* abort if no all.ref is defined */
	if (empty(gl_profile.general.all_ref))
	{
		if (!no_message)
			show_message(win ? win->hwnd : NULL, _("Error"), _("No ALL.REF file defined"), FALSE);
		return win;
	}

	if (!gl_profile.viewer.norefbox)
	{
		/* NYI: refbox */
	}
	
	if (allref == NULL)
	{
		char *filename;
		
		/* open and load REF file */
		filename = path_subst(gl_profile.general.all_ref);
		ret = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (ret < 0)
		{
			if (!no_message)
				FileErrorErrno(filename);
		} else
		{
			allref = ref_load(filename, ret, FALSE);
			hyp_utf8_close(ret);
		}
		g_free(filename);
	}

	Result_List = ref_findall(allref, string, &results, &aborted);

	if (splash)
		DestroyWindow(splash);
	
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
			if ((win = OpenFileInWindow(win, Result_List->path, Result_List->node_name, HYP_NOINDEX, TRUE, FALSE, FALSE)) != NULL)
			{
				if (Result_List->lineno > 0)
					hv_win_scroll_to_line(win, Result_List->lineno);
			}
			ref_freeresults(Result_List);
			Result_List = NULL;
			ref_close(allref);
			allref = NULL;
		} else
		{
			SearchResult(win, Result_List, string);
			ref_freeresults(Result_List);
			Result_List = NULL;
			ref_close(allref);
			allref = NULL;
		}
	} else
	{
		if (!no_message && !aborted)
		{
			char *str;
			
			str = g_strdup_printf(_("%s: could not find\n'%s'"), gl_program_name, string);
			show_message(win ? win->hwnd : NULL, _("Error"), str, FALSE);
			g_free(str);
		}
	
		ref_close(allref);
		allref = NULL;
	}
	return win;
}
