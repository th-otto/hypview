#include "hv_gtk.h"
#include "hv_vers.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void message_destroyed(GtkWidget *w, GtkWidget *dialog)
{
	UNUSED(w);
	check_toplevels(dialog);
}

/*** ---------------------------------------------------------------------- ***/

void About(GtkWidget *parent)
{
	GtkWidget *dialog, *vbox, *label;
	GtkWidget *button;
	char *str;
	char *hyp_version = hyp_lib_version();
	char *compiler_version = hyp_compiler_version();
	char *url = g_strdup_printf(_("%s is Open Source (see <a href=\"%s\">%s</a> for further information).\n"), gl_program_name, HYP_URL, HYP_URL);
	const char *homepage = "http://www.tho-otto.de/";
	const char *email = "halgara@yahoo.de";
	
	dialog = gtk_dialog_new();
	g_object_set_data(G_OBJECT(dialog), "hypview_window_type", NO_CONST("about-dialog"));
	g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(message_destroyed), dialog);
	gtk_window_set_title(GTK_WINDOW(dialog), _("HypView Versionsinfo"));
	gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox, TRUE, TRUE, 0);

	str = g_strdup_printf(
		_("HypView GTK Version %s\n"
		  "HCP %s\n"
		  "(Compiled %s)\n"
		  "%s\n"
		  "\n"
		  "%s\n"
		  "\n"
		  "%s\n"
		  "\n"
		  "Author: %s\n"
		  "\n"
		  "Email: <a href=\"mailto:%s\">%s</a>\n"
		  "\n"
		  "Homepage: <a href=\"%s\">%s</a>\n"),
		  HYPVIEW_VERSION,
		  hyp_version,
		  gl_compile_date,
		  compiler_version,
		  HYP_COPYRIGHT,
		  url,
		  "Thorsten Otto",
		  email, email,
		  homepage, homepage);
	label = gtk_label_new(NULL);
#if GTK_CHECK_VERSION(2, 18, 0)
	gtk_label_set_markup(GTK_LABEL(label), str);
#else
	gtk_label_set_label(GTK_LABEL(label), str);
#endif

	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	g_free(str);

	button = gtk_button_new_close();
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_OK);
	gtk_widget_set_can_default(button, TRUE);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy), dialog);
	gtk_widget_grab_default(button);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
	gtk_widget_show_all(dialog);

	g_free(url);
	g_free(compiler_version);
	g_free(hyp_version);
}
