#include "hv_gtk.h"
#include <sys/wait.h>

static GtkWidget *dialog;
static gboolean can_search_again;
static int HypfindID = -1;

/*** ---------------------------------------------------------------------- ***/

static void hypfind_text(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	long line = hv_win_topline(win);
	long start_line = line;
	GtkWidget *entry = g_object_get_data(G_OBJECT(dialog), "entry");
	const char *search = gtk_entry_get_text(GTK_ENTRY(entry));
	doc->autolocator_dir = 1;
	if (!empty(search))
	{
		line = doc->autolocProc(win, start_line, search);
	}
	if (line >= 0)
	{
		if (line != start_line)
		{
			can_search_again = TRUE;
			hv_win_scroll_to_line(win, line);
		}
	} else
	{
		gtk_widget_error_bell(win->hwnd);
	}
}

/*** ---------------------------------------------------------------------- ***/

static void hypfind_page(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	GtkWidget *entry = g_object_get_data(G_OBJECT(dialog), "entry");
	const char *name = gtk_entry_get_text(GTK_ENTRY(entry));
	OpenFileInWindow(win, doc->path, name, HYP_NOINDEX, TRUE, FALSE, FALSE);
}

/*** ---------------------------------------------------------------------- ***/

static void hypfind_search_allref(WINDOW_DATA *win)
{
	GtkWidget *entry = g_object_get_data(G_OBJECT(dialog), "entry");
	const char *name = gtk_entry_get_text(GTK_ENTRY(entry));
	search_allref(win, name, FALSE);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean check_hypfind(void *userdata)
{
	WINDOW_DATA *win = (WINDOW_DATA *)userdata;
	
	UNUSED(win);
	
	if (HypfindID > 0)
	{
		int retval = 0;
		
		if (waitpid(HypfindID, &retval, 0) > 0 && WIFEXITED(retval))
		{
			int retcode;
			
			HypfindID = -1;
			GDK_THREADS_ENTER();
			retcode = WEXITSTATUS(retval);
			if (retcode == 0)
			{
				OpenFileInWindow(NULL, HYP_FILENAME_HYPFIND, NULL, HYP_NOINDEX, FALSE, TRUE, FALSE);
			} else
			{
				char *str = g_strdup_printf(_("HypFind exited with code %d"), retcode);
				show_message(_("Error"), str, FALSE);
				g_free(str);
			}
			GDK_THREADS_LEAVE();
		}
	}
	if (HypfindID < 0)
		return FALSE; /* remove timeout handler */
	return TRUE; /* keep checking for process to finish */
}

/*** ---------------------------------------------------------------------- ***/

static void hypfind_run_hypfind(WINDOW_DATA *win, gboolean all_hyp)
{
	DOCUMENT *doc = win->data;
	GtkWidget *entry = g_object_get_data(G_OBJECT(dialog), "entry");
	const char *argv[5];
	int argc = 0;
	const char *name;
	char *filename;
	
	if (empty(gl_profile.general.hypfind_path))
	{
		show_message(_("Error"), _("No path to HypFind configured"), FALSE);
		return;
	}
	filename = path_subst(gl_profile.general.hypfind_path);
	argv[argc++] = filename;
	argv[argc++] = "-p";
	name = gtk_entry_get_text(GTK_ENTRY(entry));
	argv[argc++] = name;
	if (!all_hyp)
	{
		argv[argc++] = doc->path;
	}
	HypfindID = hyp_utf8_spawnvp(P_NOWAIT, argc, argv);
	if (HypfindID > 0)
	{
		g_timeout_add(20, check_hypfind, win);
	} else
	{
		char *str = g_strdup_printf(_("Can not execute\n'%s'\n%s"), filename, hyp_utf8_strerror(errno));
		show_message(_("Error"), str, FALSE);
		g_free(str);
	}
	g_free(filename);
}

/*** ---------------------------------------------------------------------- ***/

static void enter_pressed(GtkWidget *w, GtkWidget *dialog)
{
	UNUSED(w);
	gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
}

/*** ---------------------------------------------------------------------- ***/

void Hypfind(WINDOW_DATA *win, gboolean again)
{
	gint resp;
	
	if (dialog == NULL)
	{
		GtkWidget *vbox, *label, *hbox, *entry;
		GtkWidget *button;
		GtkWidget *bbox;
		GtkWidget *bbox_parent;
		
		dialog = gtk_dialog_new();
		g_object_set_data(G_OBJECT(dialog), "hypview_window_type", NO_CONST("search-dialog"));
		g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(gtk_widget_destroyed), &dialog);
		g_signal_connect_swapped(G_OBJECT(dialog), "response", G_CALLBACK(gtk_widget_hide), dialog);
		gtk_window_set_title(GTK_WINDOW(dialog), _("Search text"));

		vbox = gtk_vbox_new(FALSE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
		gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox, TRUE, TRUE, 0);

		hbox = gtk_hbox_new(FALSE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(hbox), 15);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

		label = gtk_label_new(_("Search: "));
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
		
		entry = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
		g_object_set_data(G_OBJECT(dialog), "entry", entry);
		g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(enter_pressed), dialog);
		
		button = gtk_button_new_with_label(_("in page"));
		gtk_widget_set_can_default(button, TRUE);
		gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, 1);
		
		button = gtk_button_new_with_label(_("as page"));
		gtk_widget_set_can_default(button, TRUE);
		gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, 2);
		
		button = gtk_button_new_with_label(_("as reference"));
		gtk_widget_set_can_default(button, TRUE);
		gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, 3);
		
		bbox = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
		bbox_parent = gtk_widget_get_parent(bbox);
		vbox = gtk_vbox_new(FALSE, 0);
		hbox = gtk_hbutton_box_new();
		gtk_widget_reparent(bbox, vbox);
		gtk_container_add(GTK_CONTAINER(bbox_parent), vbox);
		gtk_container_add(GTK_CONTAINER(vbox), hbox);
		
		button = gtk_button_new_with_label(_("in all pages"));
		gtk_widget_set_can_default(button, TRUE);
		gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, 4);
		gtk_widget_reparent(button, hbox);
		
		button = gtk_button_new_with_label(_("... of all Hypertexts"));
		gtk_widget_set_can_default(button, TRUE);
		gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, 5);
		gtk_widget_reparent(button, hbox);
		
		button = gtk_button_new_cancel();
		gtk_widget_set_can_default(button, TRUE);
		gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_CANCEL);
		gtk_widget_reparent(button, hbox);
		
		can_search_again = FALSE;
	}
	
	/*
	 * background process still running?
	 */
	if (HypfindID != -1)
		return;

	if (again && can_search_again)
	{
		hypfind_text(win);
		return;
	}

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win->hwnd));
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), 2);
	gtk_widget_show_all(dialog);
	resp = gtk_dialog_run(GTK_DIALOG(dialog));

	can_search_again = FALSE;
	switch (resp)
	{
	case 1:
		hypfind_text(win);
		break;
	case 2:
	case GTK_RESPONSE_OK:
		hypfind_page(win);
		break;
	case 3:
		hypfind_search_allref(win);
		break;
	case 4:
		hypfind_run_hypfind(win, FALSE);
		break;
	case 5:
		hypfind_run_hypfind(win, TRUE);
		break;
	}
}
