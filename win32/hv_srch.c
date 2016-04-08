#include "hv_defs.h"
#include "hypdebug.h"

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
		/* NYI */
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
			/* SearchResult(win, Result_List, string); NYI */
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
