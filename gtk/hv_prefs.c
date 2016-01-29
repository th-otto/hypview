#include "hv_gtk.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static GtkWidget *make_entry(const char *txt, const GdkColor *color)
{
	GtkWidget *entry;
	GdkColor bg;
	
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), txt);
	gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
	gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
	gtk_widget_set_can_default(entry, FALSE);
	gtk_widget_set_can_focus(entry, FALSE);
	gtk_widget_set_size_request(entry, 300,-1);
	gdk_color_parse(gl_profile.colors.background, &bg);
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &bg);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &bg);
	gtk_widget_modify_text(entry, GTK_STATE_NORMAL, color);
	return entry;
}

/*** ---------------------------------------------------------------------- ***/

static void bg_color_set(GtkColorButton *button, GtkWidget *dialog)
{
	GtkWidget *entry;
	GdkColor color;
	
	gtk_color_button_get_color(button, &color);
	
	entry = g_object_get_data(G_OBJECT(button), "entry");
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &color);

	entry  = g_object_get_data(G_OBJECT(dialog), "text-entry");
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &color);
	entry  = g_object_get_data(G_OBJECT(dialog), "ghosted-entry");
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &color);
	entry  = g_object_get_data(G_OBJECT(dialog), "link-entry");
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &color);
	entry  = g_object_get_data(G_OBJECT(dialog), "popup-entry");
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &color);
	entry  = g_object_get_data(G_OBJECT(dialog), "xref-entry");
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &color);
	entry  = g_object_get_data(G_OBJECT(dialog), "system-entry");
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &color);
	entry  = g_object_get_data(G_OBJECT(dialog), "rx-entry");
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &color);
	entry  = g_object_get_data(G_OBJECT(dialog), "rxs-entry");
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &color);
	entry  = g_object_get_data(G_OBJECT(dialog), "quit-entry");
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &color);
	entry  = g_object_get_data(G_OBJECT(dialog), "close-entry");
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &color);
}

/*** ---------------------------------------------------------------------- ***/

static void color_set(GtkColorButton *button, GtkWidget *dialog)
{
	GtkWidget *entry;
	GdkColor color;
	
	UNUSED(dialog);
	gtk_color_button_get_color(button, &color);
	
	entry = g_object_get_data(G_OBJECT(button), "entry");
	gtk_widget_modify_text(entry, GTK_STATE_NORMAL, &color);
}

/*** ---------------------------------------------------------------------- ***/

static void set_color(GtkWidget *dialog, const char *buttonname, char **valp)
{
	GtkColorButton *button = g_object_get_data(G_OBJECT(dialog), buttonname);
	GdkColor color;
	
	gtk_color_button_get_color(button, &color);
	g_free(*valp);
	*valp = g_strdup_printf("#%02x%02x%02x", color.red >> 8, color.green >> 8, color.blue >> 8);
}

/*** ---------------------------------------------------------------------- ***/

static void dialog_response(GtkWidget *w, gint response_id, GtkWidget *dialog)
{
	WINDOW_DATA *win;
	DOCUMENT *doc;
	GSList *l;

	UNUSED(w);

	switch (response_id)
	{
	case GTK_RESPONSE_HELP:
		break;
	case GTK_RESPONSE_APPLY:
	case GTK_RESPONSE_ACCEPT:
	case GTK_RESPONSE_OK:
	case GTK_RESPONSE_YES:
		set_color(dialog, "bg-button", &gl_profile.colors.background);
		set_color(dialog, "text-button", &gl_profile.colors.text);
		set_color(dialog, "link-button", &gl_profile.colors.link);
		set_color(dialog, "popup-button", &gl_profile.colors.popup);
		set_color(dialog, "xref-button", &gl_profile.colors.xref);
		set_color(dialog, "system-button", &gl_profile.colors.system);
		set_color(dialog, "rx-button", &gl_profile.colors.rx);
		set_color(dialog, "rxs-button", &gl_profile.colors.rxs);
		set_color(dialog, "quit-button", &gl_profile.colors.quit);
		set_color(dialog, "close-button", &gl_profile.colors.close);
		set_color(dialog, "ghosted-button", &gl_profile.colors.ghosted);
		HypProfile_SetChanged();
		for (l = all_list; l; l = l->next)
		{
			win = l->data;
			/* if (win->type == WIN_WINDOW) */
			{
				doc = win->data;
				/* reload page or file */

				doc->start_line = hv_win_topline(win);
				hv_win_update_attributes(win);
				ReInitWindow(win, TRUE);
			}
		}
		break;
	case GTK_RESPONSE_DELETE_EVENT:
	case GTK_RESPONSE_CANCEL:
	case GTK_RESPONSE_CLOSE:
	case GTK_RESPONSE_NO:
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void hv_config_colors(WINDOW_DATA *win)
{
	GtkWidget *dialog, *vbox, *hbox, *entry;
	GtkWidget *button;
	GtkWidget *color_button;
	gint resp;
	GdkColor color;
	
	dialog = gtk_dialog_new();
	g_object_set_data(G_OBJECT(dialog), "hypview_window_type", NO_CONST("color-dialog"));
	g_signal_connect_swapped(G_OBJECT(dialog), "response", G_CALLBACK(gtk_widget_hide), dialog);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Select Colors"));

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox, TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.text, &color);
	entry = make_entry(_("Window background"), &color);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.background, &color);
	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(hbox), color_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(color_button), "entry", entry);
	g_object_set_data(G_OBJECT(dialog), "bg-entry", entry);
	g_object_set_data(G_OBJECT(dialog), "bg-button", color_button);
	g_signal_connect(G_OBJECT(color_button), "color-set", G_CALLBACK(bg_color_set), dialog);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.text, &color);
	entry = make_entry(_("Normal text and line graphics"), &color);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);

	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(hbox), color_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(color_button), "entry", entry);
	g_object_set_data(G_OBJECT(dialog), "text-entry", entry);
	g_object_set_data(G_OBJECT(dialog), "text-button", color_button);
	g_signal_connect(G_OBJECT(color_button), "color-set", G_CALLBACK(color_set), dialog);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.ghosted, &color);
	entry = make_entry(_("Ghosted text (attribute @{G})"), &color);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);

	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(hbox), color_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(color_button), "entry", entry);
	g_object_set_data(G_OBJECT(dialog), "ghosted-entry", entry);
	g_object_set_data(G_OBJECT(dialog), "ghosted-button", color_button);
	g_signal_connect(G_OBJECT(color_button), "color-set", G_CALLBACK(color_set), dialog);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.link, &color);
	entry = make_entry(_("Internal nodes (@node)"), &color);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);

	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(hbox), color_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(color_button), "entry", entry);
	g_object_set_data(G_OBJECT(dialog), "link-entry", entry);
	g_object_set_data(G_OBJECT(dialog), "link-button", color_button);
	g_signal_connect(G_OBJECT(color_button), "color-set", G_CALLBACK(color_set), dialog);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.popup, &color);
	entry = make_entry(_("Popup nodes (@pnode)"), &color);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);

	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(hbox), color_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(color_button), "entry", entry);
	g_object_set_data(G_OBJECT(dialog), "popup-entry", entry);
	g_object_set_data(G_OBJECT(dialog), "popup-button", color_button);
	g_signal_connect(G_OBJECT(color_button), "color-set", G_CALLBACK(color_set), dialog);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.xref, &color);
	entry = make_entry(_("External references (@{.. link FILE [LINE]})"), &color);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);

	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(hbox), color_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(color_button), "entry", entry);
	g_object_set_data(G_OBJECT(dialog), "xref-entry", entry);
	g_object_set_data(G_OBJECT(dialog), "xref-button", color_button);
	g_signal_connect(G_OBJECT(color_button), "color-set", G_CALLBACK(color_set), dialog);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.system, &color);
	entry = make_entry(_("SYSTEM-argument (@{... system ARG})"), &color);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);

	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(hbox), color_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(color_button), "entry", entry);
	g_object_set_data(G_OBJECT(dialog), "system-entry", entry);
	g_object_set_data(G_OBJECT(dialog), "system-button", color_button);
	g_signal_connect(G_OBJECT(color_button), "color-set", G_CALLBACK(color_set), dialog);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.rxs, &color);
	entry = make_entry(_("REXX script (@{... rxs FILE})"), &color);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);

	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(hbox), color_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(color_button), "entry", entry);
	g_object_set_data(G_OBJECT(dialog), "rxs-entry", entry);
	g_object_set_data(G_OBJECT(dialog), "rxs-button", color_button);
	g_signal_connect(G_OBJECT(color_button), "color-set", G_CALLBACK(color_set), dialog);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.rx, &color);
	entry = make_entry(_("REXX command (@{... rx ARG})"), &color);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);

	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(hbox), color_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(color_button), "entry", entry);
	g_object_set_data(G_OBJECT(dialog), "rx-entry", entry);
	g_object_set_data(G_OBJECT(dialog), "rx-button", color_button);
	g_signal_connect(G_OBJECT(color_button), "color-set", G_CALLBACK(color_set), dialog);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.quit, &color);
	entry = make_entry(_("QUIT entry (@{... quit})"), &color);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);

	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(hbox), color_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(color_button), "entry", entry);
	g_object_set_data(G_OBJECT(dialog), "quit-entry", entry);
	g_object_set_data(G_OBJECT(dialog), "quit-button", color_button);
	g_signal_connect(G_OBJECT(color_button), "color-set", G_CALLBACK(color_set), dialog);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gdk_color_parse(gl_profile.colors.close, &color);
	entry = make_entry(_("CLOSE entry (@{... close})"), &color);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);

	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(hbox), color_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(color_button), "entry", entry);
	g_object_set_data(G_OBJECT(dialog), "close-entry", entry);
	g_object_set_data(G_OBJECT(dialog), "close-button", color_button);
	g_signal_connect(G_OBJECT(color_button), "color-set", G_CALLBACK(color_set), dialog);
	
	button = gtk_button_new_ok();
	gtk_widget_set_can_default(button, TRUE);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_OK);
	gtk_widget_grab_default(button);
	
	button = gtk_button_new_cancel();
	gtk_widget_set_can_default(button, TRUE);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_CANCEL);
	
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(dialog_response), dialog);
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win->hwnd));
	gtk_widget_show_all(dialog);
	resp = gtk_dialog_run(GTK_DIALOG(dialog));
	
	if (IsResponseOk(resp))
	{
	}
	gtk_widget_destroy(dialog);
}
