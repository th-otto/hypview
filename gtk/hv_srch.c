#include "hv_gtk.h"
#include "hypdebug.h"

static REF_FILE *allref;
static RESULT_ENTRY *Result_List;

static gboolean SearchResult(WINDOW_DATA *win, RESULT_ENTRY **result_list)
{
	/* YYY */
	UNUSED(win);
	UNUSED(result_list);
	return FALSE;
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
		HYP_DBG(("Line:%d", ptr->line));
		HYP_DBG(("Descr:%s", printnull(ptr->dbase_description)));
		ptr = (RESULT_ENTRY *)ptr->item.next;
	}
}

/*----------------------------------------------------------------------------------------*/

void *search_allref(WINDOW_DATA *win, const char *string, gboolean no_message)
{
	int ret;
	long results = 0;
	
	/* abort if no all.ref is defined */
	if (empty(gl_profile.general.all_ref))
	{
		HYP_DBG(("No ref file defined"));
		return win;
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

	Result_List = ref_findall(allref, string, &results);

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
			ref_freeresults(&Result_List);
			ref_close(allref);
			allref = NULL;
		} else
		{
			if (!SearchResult(win, &Result_List))
			{
				ref_freeresults(&Result_List);
				ref_close(allref);
				allref = NULL;
			}
		}
	} else
	{
		if (!no_message)
		{
			char *str;
			char *name;
			gboolean converror = FALSE;
			
			name = hyp_utf8_to_charset(hyp_get_current_charset(), string, STR0TERM, &converror);
			str = g_strdup_printf(_("%s: could not find\n'%s'"), gl_program_name, name);
			show_message(_("Error"), str, FALSE);
			g_free(name);
			g_free(str);
		}
	
		ref_close(allref);
		allref = NULL;
	}
	return win;
}
