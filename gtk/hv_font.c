#include "hv_gtk.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void ApplyFont(void)
{
	WINDOW_DATA *win;
	DOCUMENT *doc;
	GSList *l;
	
	/* adjust all open documents and windows */
	for (l = all_list; l; l = l->next)
	{
		win = (WINDOW_DATA *)l->data;
		/* if (win->type == WIN_WINDOW) */
		{
			gboolean ret;
			long topline;
			
			doc = win->data;
			/* reload page or file */

			topline = hv_win_topline(win);
			ret = doc->gotoNodeProc(win, NULL, doc->getNodeProc(win));
			
			if (ret)
			{
				doc->start_line = topline;

				ReInitWindow(win, TRUE);
			}
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void SwitchFont(WINDOW_DATA *win)
{
	UNUSED(win);
	gl_profile.viewer.use_xfont = gl_profile.viewer.use_xfont && gl_profile.viewer.xfont_name != NULL;
	ApplyFont();
	HypProfile_SetChanged();
}

/*** ---------------------------------------------------------------------- ***/

static void font_set(GtkFontButton *w, WINDOW_DATA *win)
{
	const char *fontname = gtk_font_button_get_font_name(w);
	GtkWidget *entry = (GtkWidget *)g_object_get_data(G_OBJECT(w), "entry");
	PangoFontDescription *desc;
	
	UNUSED(win);
	desc = pango_font_description_from_string(fontname);
	gtk_widget_modify_font(entry, desc);
	pango_font_description_free(desc);
}

/*** ---------------------------------------------------------------------- ***/

void SelectFont(WINDOW_DATA *win)
{
	GtkWidget *dialog, *vbox, *hbox, *frame, *entry;
	GtkWidget *button;
	GtkWidget *font_button, *xfont_button;
	int resp;
	PangoFontDescription *desc;
	const char *fontname;
	GdkColor color, bg;
	
	dialog = gtk_dialog_new();
	g_object_set_data(G_OBJECT(dialog), "hypview_window_type", NO_CONST("font-dialog"));
	g_signal_connect_swapped(G_OBJECT(dialog), "response", G_CALLBACK(gtk_widget_hide), dialog);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Select Fonts"));

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox, TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 15);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	frame = gtk_frame_new(_("Standard font: "));
	gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 15);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), _("Standard font"));
	gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);
	gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
	gtk_widget_set_can_default(entry, FALSE);
	gtk_widget_set_can_focus(entry, FALSE);
	gtk_widget_set_size_request(entry, 300, 40);
	desc = pango_font_description_from_string(gl_profile.viewer.font_name);
	gtk_widget_modify_font(entry, desc);
	pango_font_description_free(desc);
	gdk_color_parse(gl_profile.colors.background, &bg);
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &bg);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &bg);
	gdk_color_parse(gl_profile.colors.text, &color);
	gtk_widget_modify_text(entry, GTK_STATE_NORMAL, &color);
	
	font_button = gtk_font_button_new_with_font(gl_profile.viewer.font_name);
	gtk_box_pack_start(GTK_BOX(hbox), font_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(font_button), "xfont", GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(font_button), "entry", entry);
	g_signal_connect(G_OBJECT(font_button), "font-set", G_CALLBACK(font_set), win);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 15);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	frame = gtk_frame_new(_("Alternative font: "));
	gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 15);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), _("Alternative font"));
	gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);
	gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
	gtk_widget_set_can_default(entry, FALSE);
	gtk_widget_set_can_focus(entry, FALSE);
	gtk_widget_set_size_request(entry, 300, 40);
	desc = pango_font_description_from_string(gl_profile.viewer.xfont_name);
	gtk_widget_modify_font(entry, desc);
	pango_font_description_free(desc);
	gtk_widget_modify_bg(entry, GTK_STATE_NORMAL, &bg);
	gtk_widget_modify_base(entry, GTK_STATE_NORMAL, &bg);
	gtk_widget_modify_text(entry, GTK_STATE_NORMAL, &color);

	xfont_button = gtk_font_button_new_with_font(gl_profile.viewer.xfont_name);
	gtk_box_pack_start(GTK_BOX(hbox), xfont_button, FALSE, TRUE, 0);
	g_object_set_data(G_OBJECT(xfont_button), "xfont", GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(xfont_button), "entry", entry);
	g_signal_connect(G_OBJECT(xfont_button), "font-set", G_CALLBACK(font_set), win);
	
	button = gtk_button_new_ok();
	gtk_widget_set_can_default(button, TRUE);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_OK);
	gtk_widget_grab_default(button);
	
	button = gtk_button_new_cancel();
	gtk_widget_set_can_default(button, TRUE);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_CANCEL);
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	gtk_widget_show_all(dialog);
	resp = gtk_dialog_run(GTK_DIALOG(dialog));
	
	if (IsResponseOk(resp))
	{
		fontname = gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_button));
		g_free(gl_profile.viewer.font_name);
		gl_profile.viewer.font_name = g_strdup(fontname);
		fontname = gtk_font_button_get_font_name(GTK_FONT_BUTTON(xfont_button));
		g_free(gl_profile.viewer.xfont_name);
		gl_profile.viewer.xfont_name = g_strdup(fontname);
		SwitchFont(win);
	}
	gtk_widget_destroy(dialog);
}
