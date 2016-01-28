#include "hv_gtk.h"

static int gtk_inited;
static gboolean have_console;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

int toplevels_open_except(GtkWidget *top)
{
	GList *tops, *l;
	int num_open;
	
	tops = gtk_window_list_toplevels();
	for (l = tops, num_open = 0; l != NULL; l = l->next)
		if (l->data != top)
		{
			if (gtk_widget_get_visible(GTK_WIDGET(l->data)) &&
				g_object_get_data(G_OBJECT(l->data), "hypview_window_type") != NULL)
			{
				num_open++;
			}
		}
	g_list_free(tops);
	return num_open;
}

/*** ---------------------------------------------------------------------- ***/

void check_toplevels(GtkWidget *toplevel)
{
	int num_open;
	
	if ((num_open = toplevels_open_except(toplevel)) == 0)
	{
		if (gtk_main_level() > 0)
			gtk_main_quit();
	}
}

/*** ---------------------------------------------------------------------- ***/

GtkWindow *top_window(void)
{
	GList *tops, *l;
	GtkWindow *top = NULL;
	
	tops = gtk_window_list_toplevels();
	for (l = tops; l != NULL; l = l->next)
	{
		if (gtk_widget_get_visible(GTK_WIDGET(l->data)) &&
			g_object_get_data(G_OBJECT(l->data), "hypview_window_type") != NULL)
		{
			top = GTK_WINDOW(l->data);
			break;
		}
	}
	g_list_free(tops);
	return top;
}

/*** ---------------------------------------------------------------------- ***/

static void message_destroyed(GtkWidget *w, gpointer user_data)
{
	UNUSED(user_data);
	check_toplevels(w);
}

/*** ---------------------------------------------------------------------- ***/

void check_console(void)
{
#ifdef G_PLATFORM_WIN32
	CONSOLE_SCREEN_BUFFER_INFO ci;
	
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &ci))
	{
#ifdef G_OS_UNIX
		/* G_PLATFORM_WIN32 + G_OS_UNIX = CYGWIN */
		have_console = TRUE;
#endif
	} else
	{
		have_console = TRUE;
	}
#endif /* G_PLATFORM_WIN32 */

#ifdef G_OS_UNIX
	have_console = isatty(0);
#endif
}

/*** ---------------------------------------------------------------------- ***/

gboolean IsResponseOk(int resp)
{
	return resp == GTK_RESPONSE_ACCEPT ||
		   resp == GTK_RESPONSE_OK ||
		   resp == GTK_RESPONSE_YES ||
		   resp == GTK_RESPONSE_APPLY;
}

/*** ---------------------------------------------------------------------- ***/

static void dialog_response(GtkWidget *w, gint response_id, gpointer user_data)
{
	UNUSED(w);
	UNUSED(user_data);

	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		break;
	case GTK_RESPONSE_APPLY:
	case GTK_RESPONSE_ACCEPT:
	case GTK_RESPONSE_OK:
	case GTK_RESPONSE_YES:
		break;
	case GTK_RESPONSE_DELETE_EVENT:
	case GTK_RESPONSE_CANCEL:
	case GTK_RESPONSE_CLOSE:
	case GTK_RESPONSE_NO:
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void show_message(const char *title, const char *text, gboolean big)
{
	GtkWidget *dialog, *vbox, *label;
	GtkWidget *button;
	GtkWidget *scroll;
	
	dialog = gtk_dialog_new();
	g_object_set_data(G_OBJECT(dialog), "hypview_window_type", NO_CONST("message"));
	g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(message_destroyed), NULL);
	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox, TRUE, TRUE, 0);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), text);
	gtk_label_set_selectable(GTK_LABEL(label), TRUE);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
#if GTK_CHECK_VERSION(3, 0, 0)
	/*
	 * GTK2 would truncate at the maxwidth, GTK3 does not wrap without this...
	 */
	gtk_label_set_max_width_chars(GTK_LABEL(label), 66);
#endif
	if (big)
	{
		scroll = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll), label);
		gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	} else
	{
		gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);
	}
	
	button = gtk_button_new_ok();
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_CANCEL);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy), dialog);
	
	gtk_button_box_set_layout(GTK_BUTTON_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))), GTK_BUTTONBOX_SPREAD);

	gtk_widget_set_can_default(button, TRUE);
	gtk_widget_grab_default(button);

	if (big)
		gtk_window_set_default_size(GTK_WINDOW(dialog), 640, 400);
	gtk_window_set_transient_for(GTK_WINDOW(dialog), top_window());
	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
}

/*** ---------------------------------------------------------------------- ***/

gboolean ask_yesno(GtkWindow *parent, const char *text)
{
	GtkWidget *dialog, *vbox, *label;
	GtkWidget *button;
	int resp;
	
	dialog = gtk_dialog_new();
	g_object_set_data(G_OBJECT(dialog), "hypview_window_type", NO_CONST("message"));
	g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(message_destroyed), NULL);
	gtk_window_set_title(GTK_WINDOW(dialog), gl_program_name);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox, TRUE, TRUE, 0);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), text);
	gtk_label_set_selectable(GTK_LABEL(label), TRUE);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
#if GTK_CHECK_VERSION(3, 0, 0)
	/*
	 * GTK2 would truncate at the maxwidth, GTK3 does not wrap without this...
	 */
	gtk_label_set_max_width_chars(GTK_LABEL(label), 66);
#endif
	gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);
	
	button = gtk_button_new_no();
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_NO);
	gtk_widget_set_can_default(button, TRUE);
	button = gtk_button_new_yes();
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_YES);
	gtk_widget_set_can_default(button, TRUE);
	
	gtk_button_box_set_layout(GTK_BUTTON_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))), GTK_BUTTONBOX_SPREAD);

	gtk_widget_grab_default(button);

	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(dialog_response), dialog);
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	gtk_widget_show_all(dialog);
	resp = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	
	return IsResponseOk(resp);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * write a message to stdout/stderr if we are attached to a tty,
 * otherwise pop up a dialog
 */
void write_console(const char *s, gboolean use_gui, gboolean to_stderr, gboolean big)
{
	if (have_console)
	{
		char *desc;
		
		fflush(stdout);
		fflush(stderr);
		desc = g_locale_from_utf8(s, -1, NULL, NULL, NULL);
		fprintf(to_stderr ? stderr : stdout, "%s\n", desc ? desc : s);
		g_free(desc);
	}
	if (use_gui && init_gtk())
	{
		show_message(to_stderr ? _("Error") : _("Warning"), s, big);
	}
}

/*** ---------------------------------------------------------------------- ***/

gboolean init_gtk(void)
{
	int argc;
	char *argv[6];
	char **argvp;
	
	if (gtk_inited == 0)
	{
#if !GTK_CHECK_VERSION(3, 0, 0)
		gtk_set_locale();
#endif
		gdk_threads_init();
		argc = 0;
		argv[argc++] = NO_CONST(gl_program_name);
		argv[argc] = NULL;
		argvp = argv;
		if (!gtk_init_check(&argc, &argvp))
		{
			char *msg;
			
			gtk_inited = 2;
			msg = g_strdup_printf(_("%s: error initializing GTK (wrong display?)\n"), gl_program_name);
			write_console(msg, FALSE, TRUE, FALSE);
			g_free(msg);
		} else
		{
			gtk_inited = 1;
		}
	}
	return gtk_inited == 1;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void FileError(const char *path, const char *msg)
{
	char *filename;
	char *str;

	filename = hyp_path_get_basename(path);
	str = g_strdup_printf(_("File '%s'\n%s"), path, msg);
	write_console(str, TRUE, TRUE, FALSE);
	g_free(str);
	g_free(filename);
}
