#define GDK_DISABLE_DEPRECATION_WARNINGS

#include "hv_gtk.h"
#include "gdkkeysyms.h"

#include "../icons/back.h"
#include "../icons/home.h"
#include "../icons/index.h"
#include "../icons/info.h"
#include "../icons/help.h"
#include "../icons/katalog.h"
#include "../icons/load.h"
#include "../icons/memory.h"
#include "../icons/menu.h"
#include "../icons/history.h"
#include "../icons/next.h"
#include "../icons/previous.h"
#include "../icons/referenc.h"
#include "../icons/save.h"

GSList *all_list;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void hv_win_set_geometry(WINDOW_DATA *win, const char *geometry)
{
	g_free(win->m_geometry);
	win->m_geometry = g_strdup(geometry);
}

/*** ---------------------------------------------------------------------- ***/

void hv_win_open(WINDOW_DATA *win)
{
	if (win->m_geometry == NULL)
	{
		win->m_geometry = g_strdup_printf("%dx%d+%d+%d",
			gl_profile.viewer.win_w,
			gl_profile.viewer.win_h,
			gl_profile.viewer.win_x,
			gl_profile.viewer.win_y);

	}
	gtk_window_parse_geometry(GTK_WINDOW(win->hwnd), win->m_geometry);
		
	gtk_window_present(GTK_WINDOW(win->hwnd));
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void quit_force(GtkWidget *w, void *userdata)
{
	WINDOW_DATA *win = (WINDOW_DATA *)userdata;

	UNUSED(w);
	gtk_widget_destroy(win->hwnd);
	check_toplevels(NULL);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean NOINLINE WriteProfile(WINDOW_DATA *win)
{
	gboolean ret;
	Profile *inifile;
	
	inifile = gl_profile.profile;
	
	ret = HypProfile_Save(TRUE);
	
	if (ret == FALSE)
	{
		char *msg = g_strdup_printf(_("Can't write Settings:\n%s\n%s\nQuit anyway?"), Profile_GetFilename(inifile), g_strerror(errno));
		show_dialog(win->hwnd, GTK_STOCK_DIALOG_ERROR, msg, quit_force, win);
		g_free(msg);
		return FALSE;
	}
	return TRUE;
}
	
/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/* NYI */
static void About(GtkWidget *parent)
{
	UNUSED(parent);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void NOINLINE hv_win_delete(WINDOW_DATA *win)
{
	DOCUMENT *doc;
	
	if (win != NULL)
	{
		doc = win->data;
		if (doc->popup)
		{
			WINDOW_DATA *pop = (WINDOW_DATA *)(doc->popup);
			doc->popup = NULL;
			gtk_widget_destroy(pop->hwnd);
		}
		HypCloseFile(doc);
		RemoveAllHistoryEntries(win);
		all_list = g_slist_remove(all_list, win);
		g_free(win->m_geometry);
		g_free(win);
	}
}

/*** ---------------------------------------------------------------------- ***/

static void shell_destroyed(GtkWidget *w, void *userdata)
{
	WINDOW_DATA *win = (WINDOW_DATA *)userdata;

	UNUSED(w);
	hv_win_delete(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_select_source(GtkWidget *widget, gpointer user_data)
{
	WINDOW_DATA *win = (WINDOW_DATA *)user_data;
	UNUSED(widget);
	SelectFileLoad(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_help_contents(GtkWidget *widget, gpointer user_data)
{
	WINDOW_DATA *win = (WINDOW_DATA *)user_data;
	UNUSED(widget);
	Help_Contents(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

static void on_help_index(GtkWidget *widget, gpointer user_data)
{
	WINDOW_DATA *win = (WINDOW_DATA *)user_data;
	UNUSED(widget);
	Help_Index(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

#if 0
static gboolean on_history_open(GtkWidget *widget, gpointer user_data)
{
	WINDOW_DATA *win = (WINDOW_DATA *)user_data;
	UNUSED(widget);
	UNUSED(win);
	return TRUE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

static void on_about(GtkWidget *widget, gpointer user_data)
{
	WINDOW_DATA *win = (WINDOW_DATA *)user_data;
	UNUSED(widget);
	About(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

static void on_quit(GtkWidget *widget, gpointer user_data)
{
	WINDOW_DATA *win = (WINDOW_DATA *)user_data;
	if (!WriteProfile(win))
		return;
	quit_force(widget, win);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean wm_toplevel_close_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	UNUSED(event);
	on_quit(widget, user_data);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static GdkPixbuf *gtk_load_image_from_data(const unsigned char *data)
{
	GdkPixbuf *pixbuf;
	
	pixbuf = gdk_pixbuf_new_from_inline(-1, data, FALSE, NULL);
	if (!gdk_pixbuf_get_has_alpha(pixbuf))
	{
		guint8 const *src_pixel = gdk_pixbuf_get_pixels(pixbuf);
		/*
		 * use color of first pixel for transparent.
		 * This is what LR_LOADTRANSPARENT from the win32 version also does
		 */
		GdkPixbuf *icon = gdk_pixbuf_add_alpha(pixbuf, TRUE, src_pixel[0], src_pixel[1], src_pixel[2]);
		gdk_pixbuf_unref(pixbuf);
		pixbuf = icon;
	}
	return pixbuf;
}

/*** ---------------------------------------------------------------------- ***/

static GtkWidget *gtk_load_icon_from_data(const unsigned char *data)
{
	GtkWidget *w;
	GdkPixbuf *pixbuf;
	
	pixbuf = gtk_load_image_from_data(data);
	w = gtk_image_new_from_pixbuf(pixbuf);
	gdk_pixbuf_unref(pixbuf);
	gtk_widget_ref(w);
	gtk_widget_show(w);
	return w;
}

/*** ---------------------------------------------------------------------- ***/

static void tb_button_activated(GtkWidget *w, gpointer user_data)
{
	WINDOW_DATA *win = (WINDOW_DATA *)user_data;
	void *pbutton = g_object_get_data(G_OBJECT(w), "buttonnumber");
	enum toolbutton button_num = (enum toolbutton)(int)(intptr_t)pbutton;
	ToolbarClick(win->data, button_num, GDK_BUTTON_PRIMARY);
}

/*** ---------------------------------------------------------------------- ***/

static GtkWidget *AppendButton(WINDOW_DATA *win, GtkWidget *icon, const char *text, int button_num)
{
	GtkWidget *button = (GtkWidget *)gtk_tool_button_new(icon, NULL);
	if (text && 0)
	{
		GtkWidget *label = gtk_label_new(NULL);
		char *str = g_strdup_printf("<span size=\"small\">%s</span>", text);
		gtk_label_set_markup(GTK_LABEL(label), str);
		g_free(str);
		gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(button), label);
	}
	
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(tb_button_activated), win);
	gtk_tool_item_set_visible_horizontal(GTK_TOOL_ITEM(button), TRUE);
	gtk_tool_item_set_visible_vertical(GTK_TOOL_ITEM(button), TRUE);
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(button), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(win->toolbar), GTK_TOOL_ITEM(button), -1);
	gtk_widget_set_can_focus(GTK_WIDGET(button), FALSE);
	gtk_widget_set_can_focus(gtk_bin_get_child(GTK_BIN(button)), FALSE);
	gtk_widget_set_receives_default(gtk_bin_get_child(GTK_BIN(button)), FALSE);
	g_object_set_data(G_OBJECT(button), "buttonnumber", (void *)(intptr_t)button_num);
	win->m_buttons[button_num] = button;
	return button;
}

/*** ---------------------------------------------------------------------- ***/

WINDOW_DATA *hv_win_new(DOCUMENT *doc, gboolean popup)
{
	WINDOW_DATA *win;
	GtkWidget *vbox, *hbox, *vbox2, *hbox2;
	GtkWidget *menubar;
	GtkWidget *submenu;
	GtkWidget *historymenu;
	GtkWidget *item;
	GtkWidget *image;
	GtkAccelGroup *accel_group;
	GtkWidget *tool_box;

	win = g_new0(WINDOW_DATA, 1);
	if (win == NULL)
		return NULL;
	win->data = doc;
	
	accel_group = gtk_accel_group_new();
	
	win->hwnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_object_set_data(G_OBJECT(win->hwnd), "shell-dialog", win);
	g_object_set_data(G_OBJECT(win->hwnd), "hypview_window_type", NO_CONST("shell-window"));
	win->title = doc->window_title;
	gtk_window_set_title(GTK_WINDOW(win->hwnd), win->title);
	{
	GdkPixbuf *icon;
	icon = app_icon();
	gtk_window_set_icon(GTK_WINDOW(win->hwnd), icon);
	gdk_pixbuf_unref(icon);
	gtk_window_set_role(GTK_WINDOW(win->hwnd), "hypview");
	}
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(win->hwnd), vbox);
 	
	menubar = gtk_menu_bar_new();
	gtk_widget_show(menubar);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

	item = gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(menubar), item);
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);

	item = gtk_image_menu_item_new_with_mnemonic(_("Open Hypertext..."));
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	gtk_widget_add_accelerator(item, "activate", accel_group, GDK_KEY_O, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	image = gtk_image_new_from_stock("gtk-open", GTK_ICON_SIZE_MENU);
	gtk_widget_show(image);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
	g_signal_connect((gpointer)item, "activate", G_CALLBACK(on_select_source), (gpointer) win);
 
	item = gtk_menu_item_new_with_mnemonic(_("Open _Recent"));
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	historymenu = win->history_menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), historymenu);

	item = gtk_separator_menu_item_new();
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	gtk_widget_set_sensitive(item, FALSE);

	item = gtk_image_menu_item_new_from_stock("gtk-quit", accel_group);
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	g_signal_connect((gpointer)item, "activate", G_CALLBACK(on_quit), (gpointer) win);

	item = gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(menubar), item);
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);

	item = gtk_image_menu_item_new_with_mnemonic(_("_Contents"));
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	image = gtk_image_new_from_stock("gtk-info", GTK_ICON_SIZE_MENU);
	gtk_widget_show(image);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
	g_signal_connect((gpointer)item, "activate", G_CALLBACK(on_help_contents), (gpointer) win);

	item = gtk_image_menu_item_new_with_mnemonic(_("_Index"));
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	image = gtk_image_new_from_stock("gtk-index", GTK_ICON_SIZE_MENU);
	gtk_widget_show(image);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
	g_signal_connect((gpointer)item, "activate", G_CALLBACK(on_help_index), (gpointer) win);

	item = gtk_separator_menu_item_new();
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	gtk_widget_set_sensitive(item, FALSE);

	item = gtk_image_menu_item_new_with_mnemonic(_("_About"));
	gtk_widget_show(item);
	gtk_container_add(GTK_CONTAINER(submenu), item);
	image = gtk_image_new_from_stock("gtk-about", GTK_ICON_SIZE_MENU);
	gtk_widget_show(image);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
	g_signal_connect((gpointer)item, "activate", G_CALLBACK(on_about), (gpointer) win);

	tool_box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), tool_box, FALSE, FALSE, 0);
	gtk_widget_show(tool_box);
	win->toolbar = gtk_toolbar_new();
	gtk_widget_set_name(win->toolbar, "toolbar");
	gtk_box_pack_start(GTK_BOX(tool_box), win->toolbar, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_orientable_set_orientation(GTK_ORIENTABLE(win->toolbar), GTK_ORIENTATION_HORIZONTAL);
#else
	gtk_toolbar_set_orientation(GTK_TOOLBAR(win->toolbar), GTK_ORIENTATION_HORIZONTAL);
#endif
	gtk_toolbar_set_style(GTK_TOOLBAR(win->toolbar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(win->toolbar), FALSE);
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(win->toolbar), GTK_ICON_SIZE_BUTTON);
	
	{
	GtkWidget *icon;
	GtkWidget *w;

	icon = gtk_load_icon_from_data(back_icon_data);
	w = AppendButton(win, icon, _("Back"), TO_BACK);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Back one page"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(history_icon_data);
	w = AppendButton(win, icon, _("History"), TO_HISTORY);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Show history of pages"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(memory_icon_data);
	w = AppendButton(win, icon, _("Bookmarks"), TO_MEMORY);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Show list of bookmarks"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(previous_icon_data);
	w = AppendButton(win, icon, _("Previous"), TO_PREVIOUS);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Goto previous page"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(home_icon_data);
	w = AppendButton(win, icon, _("Contents"), TO_HOME);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Go up one page"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(next_icon_data);
	w = AppendButton(win, icon, _("Next"), TO_NEXT);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Goto next page"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(index_icon_data);
	w = AppendButton(win, icon, _("Index"), TO_INDEX);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Goto index page"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(katalog_icon_data);
	w = AppendButton(win, icon, _("Catalog"), TO_KATALOG);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Show catalog of hypertexts"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(reference_icon_data);
	w = AppendButton(win, icon, _("References"), TO_REFERENCES);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Show list of cross references"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(help_icon_data);
	w = AppendButton(win, icon, _("Help"), TO_HELP);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Show help page of hypertext"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(info_icon_data);
	w = AppendButton(win, icon, _("Info"), TO_INFO);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Show info about hypertext"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(load_icon_data);
	w = AppendButton(win, icon, _("Load"), TO_LOAD);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Load a file"));
	gtk_widget_unref(icon);
	icon = gtk_load_icon_from_data(save_icon_data);
	w = AppendButton(win, icon, _("Save"), TO_SAVE);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(w), _("Save page to file"));
	gtk_widget_unref(icon);
	}
	gtk_widget_show_all(win->toolbar);
	
	win->searchbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), win->searchbox, TRUE, TRUE, 0);
	win->searchentry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(win->searchbox), win->searchentry, TRUE, TRUE, 0);
	win->strnotfound = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(win->strnotfound), _("<span color=\"red\">not found!</span>"));
	gtk_box_pack_end(GTK_BOX(win->searchbox), win->strnotfound, FALSE, FALSE, 0);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	vbox2 = win->main_vbox = gtk_vbox_new(TRUE, 0);
	gtk_widget_show(vbox2);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 0);

	hbox2 = win->main_hbox = gtk_hbox_new(TRUE, 0);
	gtk_widget_show(hbox2);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, TRUE, 0);

	win->text_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(win->text_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(win->text_window), GTK_SHADOW_IN);
	gtk_widget_show(win->text_window);
	win->text_view = gtk_text_view_new();
	gtk_widget_set_can_default(win->text_view, TRUE);
	gtk_widget_set_receives_default(win->text_view, TRUE);
	gtk_widget_show(win->text_view);
	gtk_container_add(GTK_CONTAINER(win->text_window), win->text_view);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(win->text_view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(win->text_view), GTK_WRAP_NONE); 
	gtk_widget_set_can_focus(win->text_view, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox2), win->text_window, TRUE, TRUE, 0);
	
	gtk_window_add_accel_group(GTK_WINDOW(win->hwnd), accel_group);

	g_signal_connect(G_OBJECT(win->hwnd), "delete_event", G_CALLBACK(wm_toplevel_close_cb), (gpointer) win);
	g_signal_connect(G_OBJECT(win->hwnd), "destroy", G_CALLBACK(shell_destroyed), (gpointer) win);
	
	all_list = g_slist_prepend(all_list, win);
	
	if (!popup)
		doc->window = win;
	
	ToolbarUpdate(doc, FALSE);

	return win;
}

/*** ---------------------------------------------------------------------- ***/

void hv_set_title(WINDOW_DATA *win, const char *title)
{
	win->title = title;
	gtk_window_set_title(GTK_WINDOW(win->hwnd), title);
}

/*** ---------------------------------------------------------------------- ***/

void SendRedraw(WINDOW_DATA *win)
{
	gtk_widget_queue_draw(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

void ReInitWindow(DOCUMENT *doc)
{
	WINDOW_DATA *win = doc->window;
	
	win->data = doc;
	win->title = doc->window_title;
	win->x_raster = font_cw;
	win->y_raster = font_ch;
	hv_set_title(win, win->title);
	doc->selection.valid = FALSE;

	/*  Fenstergroesse: mindestens 5 Kolonnen und eine Zeile    */
	/* ResizeWindow(win, max(doc->columns, 5), max(doc->lines, 1)); */

	/*  Breite und Hoehe des Fensters den neuen Ausmassen anpassen  */
	if (gl_profile.viewer.adjust_winsize)
	{
	}

	/* gtk_text_view_scroll_to_iter(iter-of-line at doc->start_line) */

	ToolbarUpdate(doc, FALSE);
	SendRedraw(win);
}
