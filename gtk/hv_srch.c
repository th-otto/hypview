#include "hv_gtk.h"
#include "hypdebug.h"

/*----------------------------------------------------------------------------------------*
 * search a string using <all.ref>
 *----------------------------------------------------------------------------------------*/
 
static REF_FILE *allref;

enum
{
	COL_DISPLAYNAME,
	COL_NAME,
	COL_DESC,
	COL_PATH,
	COL_LINENO,
	COL_IS_LABEL,
	COL_IS_ALIAS,
	COL_VISIBLE,
	COL_LAST
};
 
/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void message_destroyed(GtkWidget *w, gpointer user_data)
{
	UNUSED(user_data);
	check_toplevels(w);
}

static void list_activated_cb(GtkTreeView *list, GtkTreePath *path, GtkTreeViewColumn *column, WINDOW_DATA *win)
{
	char *str;
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *filename = NULL;
	char *name = NULL;
	int lineno = 0;
	
	UNUSED(column);
	
	model = gtk_tree_view_get_model(list);
	str = gtk_tree_path_to_string(path);
	gtk_tree_model_get_iter_from_string(model, &iter, str);
	gtk_tree_model_get(model, &iter, COL_PATH, &filename, COL_NAME, &name, COL_LINENO, &lineno, -1);
	if (filename && name)
	{
		GtkWidget *dialog;
		
		if ((win = OpenFileInWindow(win, filename, name, HYP_NOINDEX, TRUE, FALSE, FALSE)) != NULL)
		{
			if (lineno > 0)
				hv_win_scroll_to_line(win, lineno);
			dialog = gtk_widget_get_toplevel(GTK_WIDGET(list));
			gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
		}
	}
	g_free(filename);
	g_free(name);
	g_free(str);
}

/*** ---------------------------------------------------------------------- ***/

static void SearchResult(WINDOW_DATA *win, RESULT_ENTRY *result_list, const char *searchstring)
{
	GtkWidget *dialog;
	GtkWidget *vbox, *scroll, *list, *button;
	char *str;
	RESULT_ENTRY *ptr;
	GtkListStore *model;
	
	dialog = gtk_dialog_new();
	g_object_set_data(G_OBJECT(dialog), "hypview_window_type", NO_CONST("message"));
	g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(message_destroyed), NULL);
	str = g_strdup_printf(_("Search results: %s"), searchstring);
	gtk_window_set_title(GTK_WINDOW(dialog), str);
	g_free(str);
	gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox, TRUE, TRUE, 0);

	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll), GTK_SHADOW_IN);

	list = gtk_tree_view_new();
	gtk_container_add(GTK_CONTAINER(scroll), list);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), TRUE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(list), TRUE);

	{
		GtkCellRenderer *ren;
		GtkTreeViewColumn *column;
		int height;
		
		model = gtk_list_store_new(COL_LAST, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);
		gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(model));
		g_object_unref(model);

		ren = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Name"), ren, "text", COL_DISPLAYNAME, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
		gtk_cell_renderer_get_size(ren, list, NULL, NULL, NULL, NULL,  &height);
		
		ren = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Node/Alias name"), ren, "text", COL_NAME, NULL);
		gtk_tree_view_column_set_visible(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
		
		ren = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Description"), ren, "text", COL_DESC, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

		ren = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Path"), ren, "text", COL_PATH, NULL);
		gtk_tree_view_column_set_visible(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

		ren = gtk_cell_renderer_toggle_new();
		column = gtk_tree_view_column_new_with_attributes(_("Visible"), ren, "active", COL_VISIBLE, "visible", COL_VISIBLE, NULL);
		gtk_tree_view_column_set_visible(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

		g_signal_connect_after(G_OBJECT(list), "row_activated", G_CALLBACK(list_activated_cb), win);
		
		gtk_widget_set_size_request(list, 400, height * 12);
	}

	{
		GtkTreeIter iter;
		char *displayname;
		
		for (ptr = result_list; ptr; ptr = (RESULT_ENTRY *)ptr->item.next)
		{
			if (ptr->label_name)
				displayname = g_strdup_printf("%s \xe2\x88\x99 %s", ptr->node_name, ptr->label_name);
			else if (ptr->alias_name)
				displayname = g_strdup_printf("%s \xe2\x88\x99 %s", ptr->node_name, ptr->alias_name);
			else
				displayname = g_strdup(ptr->node_name);
			gtk_list_store_append(model, &iter);
			gtk_list_store_set(model, &iter,
				COL_DISPLAYNAME, displayname,
				COL_NAME, ptr->node_name,
				COL_DESC, ptr->dbase_description,
				COL_PATH, ptr->path,
				COL_LINENO, ptr->lineno,
				COL_IS_LABEL, ptr->label_name != NULL,
				COL_IS_ALIAS, ptr->alias_name != NULL,
				COL_VISIBLE, TRUE,
				-1);
			g_free(displayname);
		}
	}
	
	button = gtk_button_new_cancel();
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_CANCEL);
	g_signal_connect_swapped(G_OBJECT(dialog), "response", G_CALLBACK(gtk_widget_destroy), dialog);
	
	if (win)
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
}

/*** ---------------------------------------------------------------------- ***/

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
	GtkWidget *splash = NULL;
	
	/* abort if no all.ref is defined */
	if (empty(gl_profile.general.all_ref))
	{
		if (!no_message)
			show_message(GTK_WIDGET(win), _("Error"), _("No ALL.REF file defined"), FALSE);
		return win;
	}

	if (!gl_profile.viewer.norefbox)
	{
		GtkWidget *hbox;
		GtkWidget *label;
		char *str;
		
		splash = gtk_window_new(GTK_WINDOW_POPUP);
		hbox = gtk_hbox_new(FALSE, 10);
		gtk_container_add(GTK_CONTAINER(splash), hbox);
		gtk_widget_show(hbox);
		str = g_strdup_printf("Search: %s", string);
		label = gtk_label_new(str);
		g_free(str);
		gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 10);
		gtk_widget_show(label);
		gtk_window_set_type_hint(GTK_WINDOW(splash), GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
		if (win)
			gtk_window_set_transient_for(GTK_WINDOW(splash), GTK_WINDOW(win));
		gtk_window_set_position(GTK_WINDOW(splash), GTK_WIN_POS_CENTER_ON_PARENT);
		gtk_widget_show(splash);
		gtk_window_present(GTK_WINDOW(splash));
		gdk_display_flush(gtk_widget_get_display(splash));
		gdk_window_process_all_updates();
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
		gtk_widget_destroy(splash);
	
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
			show_message(GTK_WIDGET(win), _("Error"), str, FALSE);
			g_free(str);
		}
	
		ref_close(allref);
		allref = NULL;
	}
	return win;
}
