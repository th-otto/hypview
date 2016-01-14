#include "hv_gtk.h"
#include "hypdebug.h"

static char const prghelp_name[] = "hypview.hyp";

static gboolean expanded = FALSE;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void message_destroyed(GtkWidget *w, gpointer user_data)
{
	UNUSED(user_data);
	check_toplevels(w);
}

/*** ---------------------------------------------------------------------- ***/

static void expander_toggled(GtkWidget *w, gpointer user_data)
{
	UNUSED(w);
	UNUSED(user_data);
	expanded = !expanded;
}

/*** ---------------------------------------------------------------------- ***/

void ProgrammInfos(DOCUMENT *doc)
{
	GtkWidget *dialog, *vbox, *label;
	GtkWidget *button;
	GtkWidget *expander, *frame;
	HYP_DOCUMENT *hyp = doc->data;
	char *str;
	
	dialog = gtk_dialog_new();
	g_object_set_data(G_OBJECT(dialog), "hypview_window_type", NO_CONST("message"));
	g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(message_destroyed), NULL);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Programinfo..."));
	gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox, TRUE, TRUE, 0);

#if 0
	{
	char *version = gl_program_version();
	str = g_strdup_printf("%s %s", gl_program_name, version);
	label = gtk_label_new(str);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	g_free(str);
	g_free(version);
	str = g_strdup_printf(_("from: %s"), gl_compile_date);
	label = gtk_label_new(str);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	g_free(str);
	}
#endif

	str = g_strdup_printf(_("File: %s"), hyp_basename(doc->path));
	label = gtk_label_new(str);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	g_free(str);

	if (doc->type == HYP_FT_HYP)
	{
		str = g_strdup_printf(_("Subject: %s"), fixnull(hyp->database));
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
		str = g_strdup_printf(_("Author: %s"), fixnull(hyp->author));
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
		str = g_strdup_printf(_("Version: %s"), fixnull(hyp->version));
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
		
		expander = gtk_expander_new(_("Details"));
		gtk_box_pack_start(GTK_BOX(vbox), expander, FALSE, FALSE, 0);
		frame = gtk_frame_new(NULL);
		gtk_container_add(GTK_CONTAINER(expander), frame);
		vbox = gtk_vbox_new(FALSE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
		gtk_container_add(GTK_CONTAINER(frame), vbox);
		g_signal_connect(G_OBJECT(expander), "activate", G_CALLBACK(expander_toggled), NULL);
		gtk_expander_set_expanded(GTK_EXPANDER(expander), expanded);
		
		str = g_strdup_printf(_("Nodes       : %7d"), hyp->num_index);
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
		str = g_strdup_printf(_("Index Size  : %7ld"), hyp->itable_size);
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
		str = g_strdup_printf(_("HCP-Version : %3u"), hyp->comp_vers);
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
		str = g_strdup_printf(_("Compiled on : %s"), hyp_osname(hyp->comp_os));
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
		str = g_strdup_printf(_("@charset    : %s"), hyp_charset_name(hyp->comp_charset));
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
		str = g_strdup_printf(_("@default    : %s"), fixnull(hyp->default_name));
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
		str = g_strdup_printf(_("@help       : %s"), fixnull(hyp->help_name));
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
		str = g_strdup_printf(_("@options    : %s"), fixnull(hyp->hcp_options));
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
		str = g_strdup_printf(_("@width      : %3u"), hyp->line_width);
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		g_free(str);
	}
	
	button = gtk_button_new_ok();
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_OK);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy), dialog);
	
	gtk_button_box_set_layout(GTK_BUTTON_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))), GTK_BUTTONBOX_SPREAD);

	gtk_widget_set_can_default(button, TRUE);
	gtk_widget_grab_default(button);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), top_window());
	gtk_widget_show_all(dialog);
}
