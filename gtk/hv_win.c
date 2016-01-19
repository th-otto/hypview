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
#include "../icons/nextphys.h"
#include "../icons/previous.h"
#include "../icons/prevphys.h"
#include "../icons/first.h"
#include "../icons/last.h"
#include "../icons/referenc.h"
#include "../icons/save.h"

GSList *all_list;

static const char *const tb_action_names[TO_MAX] = {
	[TO_BACK] = "back",
	[TO_HISTORY] = "history",
	[TO_MEMORY] = "bookmarks",
	[TO_FIRST] = "firstpage",
	[TO_PREV_PHYS] = "prevphyspage",
	[TO_PREV] = "prevlogpage",
	[TO_HOME] = "toc",
	[TO_NEXT] = "nextlogpage",
	[TO_NEXT_PHYS] = "nextphyspage",
	[TO_LAST] = "lastpage",
	[TO_INDEX] = "index",
	[TO_KATALOG] = "catalog",
	[TO_REFERENCES] = "xref",
	[TO_HELP] = "help",
	[TO_INFO] = "info",
	[TO_LOAD] = "open",
	[TO_SAVE] = "save",
};

/*
 * colornames used for tags; the actual color is configurable
 */
static const char *const colornames[16] = {
	"white",
	"black",
	"red",
	"green",
	"blue",
	"cyan",
	"yellow",
	"magenta",
	"light-gray",
	"dark-gray",
	"dark-red",
	"dark-green",
	"dark-blue",
	"dark-cyan",
	"dark-yellow",
	"dark-magenta"
};
static GdkColor gdk_colors[16];

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
	/* YYY */
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

static void on_select_source(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	SelectFileLoad(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_font_select(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	UNUSED(win);
	printf("NYI: on_font_select\n");
}

/*** ---------------------------------------------------------------------- ***/

static void on_color_select(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	UNUSED(win);
	printf("NYI: on_color_select\n");
}

/*** ---------------------------------------------------------------------- ***/

static void on_output_settings(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	UNUSED(win);
	printf("NYI: on_output_settings\n");
}

/*** ---------------------------------------------------------------------- ***/

static void on_switch_font(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	UNUSED(win);
	SwitchFont(win->data);
}

/*** ---------------------------------------------------------------------- ***/

static void on_expand_spaces(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	gl_profile.viewer.expand_spaces = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(gtk_action_group_get_action(win->action_group, "expandspaces")));
	if (win->text_window)
	{
		DOCUMENT *doc = win->data;
		if (doc && doc->prepNode)
		{
			doc->prepNode(doc);
			ReInitWindow(doc);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

static void on_scale_bitmaps(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	UNUSED(win);
	printf("NYI: on_scale_bitmaps\n");
}

/*** ---------------------------------------------------------------------- ***/

static void on_preferences(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	UNUSED(win);
	printf("NYI: on_preferences\n");
}

/*** ---------------------------------------------------------------------- ***/

static void on_help_contents(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	Help_Contents(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

static void on_help_index(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	Help_Index(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

#if 0
static gboolean on_history_open(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	UNUSED(win);
	return TRUE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

static void on_about(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	About(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

static void on_back(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	GoBack(doc);
}

/*** ---------------------------------------------------------------------- ***/

static void on_clearstack(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	RemoveAllHistoryEntries(win);
	ToolbarUpdate(doc, TRUE);
}

/*** ---------------------------------------------------------------------- ***/

static void on_history(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	ToolbarClick(doc, TO_HISTORY, GDK_BUTTON_PRIMARY, gtk_get_current_event_time());
}

/*** ---------------------------------------------------------------------- ***/

static void on_catalog(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	GotoCatalog(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_help(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	GotoHelp(doc);
}

/*** ---------------------------------------------------------------------- ***/

static void on_bookmarks(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	ToolbarClick(doc, TO_MEMORY, GDK_BUTTON_PRIMARY, gtk_get_current_event_time());
}

/*** ---------------------------------------------------------------------- ***/

static void on_next(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	GoThisButton(doc, TO_NEXT);
}

/*** ---------------------------------------------------------------------- ***/

static void on_nextphys(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	GoThisButton(doc, TO_NEXT_PHYS);
}

/*** ---------------------------------------------------------------------- ***/

static void on_prev(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	GoThisButton(doc, TO_PREV);
}

/*** ---------------------------------------------------------------------- ***/

static void on_prevphys(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	GoThisButton(doc, TO_PREV_PHYS);
}

/*** ---------------------------------------------------------------------- ***/

static void on_toc(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	GoThisButton(doc, TO_HOME);
}

/*** ---------------------------------------------------------------------- ***/

static void on_first(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	GoThisButton(doc, TO_FIRST);
}

/*** ---------------------------------------------------------------------- ***/

static void on_last(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	GoThisButton(doc, TO_LAST);
}

/*** ---------------------------------------------------------------------- ***/

static void on_index(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	GotoIndex(doc);
}

/*** ---------------------------------------------------------------------- ***/

static void on_xref(GtkWidget *widget, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(widget);
	HypExtRefPopup(doc, GDK_BUTTON_PRIMARY, gtk_get_current_event_time());
}

/*** ---------------------------------------------------------------------- ***/

static void on_quit(GtkWidget *widget, WINDOW_DATA *win)
{
	if (!WriteProfile(win))
		return;
	quit_force(widget, win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_info(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	ProgrammInfos(win->data);
}

/*** ---------------------------------------------------------------------- ***/

static void on_close(GtkWidget *widget, WINDOW_DATA *win)
{
	UNUSED(widget);
	SendCloseWindow(win);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean wm_toplevel_close_cb(GtkWidget *widget, GdkEvent *event, WINDOW_DATA *win)
{
	UNUSED(event);
	on_quit(widget, win);
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

#if 0
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
#endif

/*** ---------------------------------------------------------------------- ***/

#if 0

static void tb_button_activated(GtkWidget *w, gpointer user_data)
{
	WINDOW_DATA *win = (WINDOW_DATA *)user_data;
	void *pbutton = g_object_get_data(G_OBJECT(w), "buttonnumber");
	enum toolbutton button_num = (enum toolbutton)(int)(intptr_t)pbutton;
	ToolbarClick(win->data, button_num, GDK_BUTTON_PRIMARY, gtk_get_current_event_time());
}

#else

static gboolean tb_button_clicked(GtkWidget *w, GdkEventButton *event, gpointer user_data)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_PRIMARY)
	{
		WINDOW_DATA *win = (WINDOW_DATA *)user_data;
		void *pbutton = g_object_get_data(G_OBJECT(w), "buttonnumber");
		enum toolbutton button_num = (enum toolbutton)(int)(intptr_t)pbutton;
		ToolbarClick(win->data, button_num, event->button, event->time);
		return TRUE;
	}
	return FALSE;
}

#endif

/*** ---------------------------------------------------------------------- ***/

static GtkWidget *AppendButton(WINDOW_DATA *win, int button_num)
{
#if 1
	/*
	 * construct image widgets for the icons;
	 * the rescaled icons that are placed inside
	 * buttons by GTK look ugly
	 */
	GtkAction *action = gtk_action_group_get_action(win->action_group, tb_action_names[button_num]);
	GtkWidget *button = (GtkWidget *)gtk_tool_item_new();
	GtkWidget *event_box = gtk_event_box_new();
	GtkStockItem stock_item;
	GtkWidget *icon;
	GdkPixbuf *pixbuf;
	
	gtk_stock_lookup(gtk_action_get_stock_id(action), &stock_item);
	gtk_widget_set_name(button, gtk_action_get_name(action));
	pixbuf = gtk_widget_render_icon(win->toolbar, stock_item.stock_id, -1, NULL);
	icon = gtk_image_new_from_pixbuf(pixbuf);
	gdk_pixbuf_unref(pixbuf);
	gtk_container_add(GTK_CONTAINER(event_box), icon);
	gtk_container_add(GTK_CONTAINER(button), event_box);
	gtk_toolbar_insert(GTK_TOOLBAR(win->toolbar), GTK_TOOL_ITEM(button), -1);
	gtk_widget_add_events(event_box, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	g_signal_connect(G_OBJECT(event_box), "button-press-event", G_CALLBACK(tb_button_clicked), win);
	g_signal_connect(G_OBJECT(event_box), "button-release-event", G_CALLBACK(tb_button_clicked), win);
	gtk_widget_set_can_focus(button, FALSE);
	gtk_widget_set_receives_default(button, FALSE);
	g_object_set_data(G_OBJECT(event_box), "buttonnumber", (void *)(intptr_t)button_num);
	gtk_activatable_set_related_action(GTK_ACTIVATABLE(button), action);
#else
	GtkWidget *button = (GtkWidget *)gtk_tool_button_new(icon, NULL);
	if (text && 0)
	{
		GtkWidget *label = gtk_label_new(NULL);
		char *str = g_strdup_printf("<span size=\"small\">%s</span>", text);
		gtk_label_set_markup(GTK_LABEL(label), str);
		g_free(str);
		gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(button), label);
	}
	
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(button), tooltip);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(tb_button_activated), win);
	gtk_tool_item_set_visible_horizontal(GTK_TOOL_ITEM(button), TRUE);
	gtk_tool_item_set_visible_vertical(GTK_TOOL_ITEM(button), TRUE);
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(button), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(win->toolbar), GTK_TOOL_ITEM(button), -1);
	gtk_widget_set_can_focus(GTK_WIDGET(button), FALSE);
	gtk_widget_set_can_focus(gtk_bin_get_child(GTK_BIN(button)), FALSE);
	gtk_widget_set_receives_default(gtk_bin_get_child(GTK_BIN(button)), FALSE);
	g_object_set_data(G_OBJECT(button), "buttonnumber", (void *)(intptr_t)button_num);
#endif
	win->m_buttons[button_num] = button;
	return button;
}

/*** ---------------------------------------------------------------------- ***/

/* This function registers our custom toolbar icons, so they can be themed.
 */

static void register_icon(GtkIconFactory *factory, const char *stock_id, const unsigned char *data)
{
	GdkPixbuf *pixbuf;
	GtkIconSet *icon_set;
	pixbuf = gtk_load_image_from_data(data);
	icon_set = gtk_icon_set_new_from_pixbuf(pixbuf);
	gtk_icon_factory_add(factory, stock_id, icon_set);
	gtk_icon_set_unref(icon_set);
	gdk_pixbuf_unref(pixbuf);
}


static void register_stock_icons(void)
{
	struct ConstGtkStockItem {
		const char *stock_id;
		const char *label;
		GdkModifierType modifier;
		guint keyval;
		const char *translation_domain;
	};
	/* verify(sizeof(struct ConstGtkStockItem) == sizeof(GtkStockItem)); */

	static struct ConstGtkStockItem const items[] = {
		{ "hv-back", N_("Back"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-history", N_("History"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-bookmarks", N_("Bookmarks"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-prev", N_("Previous logical page"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-prevphys", N_("Previous physical page"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-toc", N_("Contents"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-next", N_("Next logical page"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-nextphys", N_("Next physical page"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-first", N_("First page"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-last", N_("Last page"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-index", N_("Index"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-catalog", N_("Catalog"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-xref", N_("References"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-help", N_("Show help page"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-info", N_("Show info page"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-load", N_("_Open Hypertext"), 0, 0, GETTEXT_PACKAGE },
		{ "hv-save", N_("_Save text"), 0, 0, GETTEXT_PACKAGE },
	};
	static gboolean registered = FALSE;
	GtkIconFactory *factory;
	
	if (registered)
		return;

	gtk_stock_add_static((const GtkStockItem *)items, G_N_ELEMENTS(items));
	factory = gtk_icon_factory_new();
	gtk_icon_factory_add_default(factory);
	
	register_icon(factory, "hv-back", back_icon_data);
	register_icon(factory, "hv-history", history_icon_data);
	register_icon(factory, "hv-bookmarks", memory_icon_data);
	register_icon(factory, "hv-prev", previous_icon_data);
	register_icon(factory, "hv-prevphys", prevphys_icon_data);
	register_icon(factory, "hv-toc", home_icon_data);
	register_icon(factory, "hv-next", next_icon_data);
	register_icon(factory, "hv-nextphys", nextphys_icon_data);
	register_icon(factory, "hv-first", first_icon_data);
	register_icon(factory, "hv-last", last_icon_data);
	register_icon(factory, "hv-index", index_icon_data);
	register_icon(factory, "hv-catalog", katalog_icon_data);
	register_icon(factory, "hv-xref", reference_icon_data);
	register_icon(factory, "hv-help", help_icon_data);
	register_icon(factory, "hv-info", info_icon_data);
	register_icon(factory, "hv-load", load_icon_data);
	register_icon(factory, "hv-save", save_icon_data);
	
	g_object_unref(factory);
	registered = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static GtkTextTag *gtk_text_table_create_tag(GtkTextTagTable *table, const gchar *tag_name, const gchar *first_property_name, ...)
{
	GtkTextTag *tag;
	va_list list;

	tag = gtk_text_tag_new(tag_name);

	gtk_text_tag_table_add(table, tag);

	if (first_property_name)
	{
		va_start(list, first_property_name);
		g_object_set_valist(G_OBJECT(tag), first_property_name, list);
		va_end(list);
	}

	g_object_unref(tag);

	return tag;
}

/*** ---------------------------------------------------------------------- ***/

static GtkTextTag *create_link_tag(GtkTextTagTable *table, const gchar *tag_name, int color)
{
	GtkTextTag *tag;

	tag = gtk_text_tag_new(tag_name);

	gtk_text_tag_table_add(table, tag);

	g_object_set(G_OBJECT(tag), "foreground", gl_profile.viewer.color[color], NULL);
	if (gl_profile.viewer.link_effect & HYP_TXT_BOLD)
		g_object_set(G_OBJECT(tag), "weight", PANGO_WEIGHT_BOLD, NULL);
	if (gl_profile.viewer.link_effect & HYP_TXT_LIGHT)
		g_object_set(G_OBJECT(tag), "foreground", "#cccccc", NULL);
	if (gl_profile.viewer.link_effect & HYP_TXT_ITALIC)
		g_object_set(G_OBJECT(tag), "style", PANGO_STYLE_ITALIC, NULL);
	if (gl_profile.viewer.link_effect & HYP_TXT_UNDERLINED)
		g_object_set(G_OBJECT(tag), "underline", PANGO_UNDERLINE_SINGLE, NULL);
	if (gl_profile.viewer.link_effect & HYP_TXT_SHADOWED)
		g_object_set(G_OBJECT(tag), "weight", PANGO_WEIGHT_HEAVY, NULL);
	if (gl_profile.viewer.link_effect & HYP_TXT_OUTLINED)
		g_object_set(G_OBJECT(tag), "strikethrough", TRUE, NULL);
	
	g_object_unref(tag);

	return tag;
}

/*** ---------------------------------------------------------------------- ***/

static GtkTextTagTable *create_tags(void)
{
	GtkTextTagTable *table;
	int i;
	
	table = gtk_text_tag_table_new();
	
	for (i = 0; i < 16; i++)
	{
		gdk_color_parse(gl_profile.viewer.color[i], &gdk_colors[i]);
		gtk_text_table_create_tag(table, colornames[i], "foreground", gl_profile.viewer.color[i], NULL);
	}
	
	gtk_text_table_create_tag(table, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_table_create_tag(table, "ghosted", "foreground", "#cccccc", NULL);
	gtk_text_table_create_tag(table, "italic", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_table_create_tag(table, "underlined", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	gtk_text_table_create_tag(table, "outlined", "strikethrough", TRUE, NULL); /* TODO */
	gtk_text_table_create_tag(table, "shadowed", "weight", PANGO_WEIGHT_HEAVY, NULL); /* TODO */

	create_link_tag(table, "link", gl_profile.viewer.link_color);
	create_link_tag(table, "popup", gl_profile.viewer.popup_color);
	create_link_tag(table, "xref", gl_profile.viewer.xref_color);
	create_link_tag(table, "system", gl_profile.viewer.system_color);
	create_link_tag(table, "rexx", gl_profile.viewer.rexx_color);
	create_link_tag(table, "quit", gl_profile.viewer.quit_color);

	return table;
}

/*** ---------------------------------------------------------------------- ***/

static void set_font_attributes(WINDOW_DATA *win)
{
	PangoFontDescription *desc = pango_font_description_from_string(sel_font_name);
	PangoFontMap *font_map;
	PangoFont *font;
	PangoContext *context;
	GdkScreen *screen;
	PangoFontMetrics *metrics = NULL;
	
	gtk_widget_modify_text(win->text_view, GTK_STATE_NORMAL, &gdk_colors[gl_profile.viewer.text_color]);
	gtk_widget_modify_base(win->text_view, GTK_STATE_NORMAL, &gdk_colors[gl_profile.viewer.background_color]);
	gtk_widget_modify_font(win->text_view, desc);
	screen = gtk_widget_get_screen(win->hwnd);
	context = gdk_pango_context_get_for_screen(screen);
	font_map = pango_context_get_font_map(context);
	font = pango_font_map_load_font(font_map, context, desc);
	if (font)
	{
		metrics = pango_font_get_metrics(font, pango_language_get_default());
		g_object_unref(font);
	}
	if (metrics)
	{
		win->x_raster = pango_font_metrics_get_approximate_char_width(metrics) / PANGO_SCALE;
		win->y_raster = (pango_font_metrics_get_ascent(metrics) + pango_font_metrics_get_descent(metrics)) / PANGO_SCALE;
		pango_font_metrics_unref(metrics);
	} else
	{
		win->x_raster = font_cw;
		win->y_raster = font_ch;
	}
	pango_font_description_free(desc);
}

/*** ---------------------------------------------------------------------- ***/

static GtkActionEntry const action_entries[] = {
	/*
	 * menu titles
	 * name,                stock id,  label
	 */
	{ "FileMenu",           NULL,      N_("_File"), 0, 0, 0 },
	{ "EditMenu",           NULL,      N_("_Edit"), 0, 0, 0 },
	{ "NavigateMenu",       NULL,      N_("_Navigate"), 0, 0, 0 },
	{ "OptionsMenu",        NULL,      N_("_Options"), 0, 0, 0 },
	{ "HelpMenu",           NULL,      N_("_Help"), 0, 0, 0 },
	{ "RecentMenu",         NULL,      N_("Open _Recent"), 0, 0, 0 },
	/*
	 * menu entries
	 * name,                stock id,                label                                  accelerator    tooltip                                              callback
	 */
	{ "open",               "hv-load",               N_("_Open Hypertext..."),              "<Ctrl>O",     N_("Load a file"),                                   G_CALLBACK(on_select_source) },
	{ "save",               "hv-save",               N_("_Save text..."),                   "<Ctrl>S",     N_("Save page to file"),                             G_CALLBACK(on_select_source) },
	{ "info",               "hv-info",               N_("_File info..."),                   "<Ctrl>I",     N_("Show info about hypertext"),                     G_CALLBACK(on_info) },
	{ "close",              "gtk-close",             N_("_Close"),                          "<Ctrl>U",     NULL,                                                G_CALLBACK(on_close) },
	{ "quit",               "gtk-quit",              N_("_Quit"),                           "<Ctrl>Q",     NULL,                                                G_CALLBACK(on_quit) },

	{ "back",               "hv-back",               N_("Back one page"),                   NULL,          N_("Back one page"),                                 G_CALLBACK(on_back) },
	{ "clearstack",         NULL,                    N_("Clear stack"),                     NULL,          N_("Clear stack"),                                   G_CALLBACK(on_clearstack) },
	{ "history",            "hv-history",            N_("History"),                         NULL,          N_("Show history of pages"),                         G_CALLBACK(on_history) },
	{ "bookmarks",          "hv-bookmarks",          N_("Bookmarks"),                       NULL,          N_("Show list of bookmarks"),                        G_CALLBACK(on_bookmarks) },
	{ "prevphyspage",       "hv-prevphys",           N_("Previous physical page"),          NULL,          N_("Goto previous physical page"),                   G_CALLBACK(on_prevphys) },
	{ "prevlogpage",        "hv-prev",               N_("Previous logical page"),           NULL,          N_("Goto previous page"),                            G_CALLBACK(on_prev) },
	{ "toc",                "hv-toc",                N_("Contents"),                        NULL,          N_("Go up one page"),                                G_CALLBACK(on_toc) },
	{ "nextlogpage",        "hv-next",               N_("Next logical page"),               NULL,          N_("Goto next page"),                                G_CALLBACK(on_next) },
	{ "nextphyspage",       "hv-nextphys",           N_("Next physical page"),              NULL,          N_("Goto next physical page"),                       G_CALLBACK(on_nextphys) },
	{ "firstpage",          "hv-first",              N_("First page"),                      NULL,          N_("Goto first page"),                               G_CALLBACK(on_first) },
	{ "lastpage",           "hv-last",               N_("Last page"),                       NULL,          N_("Goto last page"),                                G_CALLBACK(on_last) },
	{ "index",              "hv-index",              N_("Index"),                           NULL,          N_("Goto index page"),                               G_CALLBACK(on_index) },
	{ "catalog",            "hv-catalog",            N_("Catalog"),                         NULL,          N_("Show catalog of hypertexts"),                    G_CALLBACK(on_catalog) },
	{ "xref",               "hv-xref",               N_("References"),                      NULL,          N_("Show list of cross references"),                 G_CALLBACK(on_xref) },
	{ "help",               "hv-help",               N_("Show help page"),                  NULL,          N_("Show help page"),                                G_CALLBACK(on_help) },

	{ "selectfont",         "gtk-font",              N_("_Font..."),                        "<Ctrl>Z",     NULL,                                                G_CALLBACK(on_font_select) },
	{ "selectcolors",       NULL,                    N_("_Colors..."),                      NULL,          NULL,                                                G_CALLBACK(on_color_select) },
	{ "outputconfig",       NULL,                    N_("_Output..."),                      NULL,          NULL,                                                G_CALLBACK(on_output_settings) },
	{ "preferences",        "gtk-preferences",       N_("_Settings..."),                    NULL,          NULL,                                                G_CALLBACK(on_preferences) },

	{ "helpcontents",       "gtk-info",              N_("_Contents"),                       NULL,          NULL,                                                G_CALLBACK(on_help_contents) },
	{ "helpindex",          "gtk-index",             N_("_Index"),                          NULL,          NULL,                                                G_CALLBACK(on_help_index) },
	{ "about",              "gtk-about",             N_("_About"),                          NULL,          NULL,                                                G_CALLBACK(on_about) },
};

static GtkToggleActionEntry const toggle_action_entries[] = {
	{ "altfont",            NULL,                    N_("_Alternative font"),               "<Ctrl><Shift>Z", NULL,                                             G_CALLBACK(on_switch_font), FALSE },
	{ "expandspaces",       NULL,                    N_("_Expand multiple spaces"),         "<Ctrl>L",     NULL,                                                G_CALLBACK(on_expand_spaces), FALSE },
	{ "scalebitmaps",       NULL,                    N_("_Scale bitmaps"),                  "<Ctrl>B",     NULL,                                                G_CALLBACK(on_scale_bitmaps), FALSE },
};

static char const ui_info[] =
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu action='FileMenu'>"
"      <menuitem action='open'/>"
"      <separator/>"
"      <menuitem action='save'/>"
"      <separator/>"
"      <menuitem action='recent' />"
"      <separator/>"
"      <menuitem action='info'/>"
"      <separator/>"
"      <menuitem action='close'/>"
"      <menuitem action='quit'/>"
"    </menu>"
"    <menu action='NavigateMenu'>"
"      <menuitem action='prevlogpage'/>"
"      <menuitem action='nextlogpage'/>"
"      <separator/>"
"      <menuitem action='prevphyspage'/>"
"      <menuitem action='nextphyspage'/>"
"      <separator/>"
"      <menuitem action='firstpage'/>"
"      <menuitem action='lastpage'/>"
"      <separator/>"
"      <menuitem action='toc'/>"
"      <menuitem action='index'/>"
"      <menuitem action='help'/>"
"      <separator/>"
"      <menuitem action='back'/>"
"      <menuitem action='clearstack'/>"
"    </menu>"
"    <menu action='OptionsMenu'>"
"      <menuitem action='selectfont'/>"
"      <menuitem action='selectcolors'/>"
"      <separator/>"
"      <menuitem action='outputconfig'/>"
"      <menuitem action='outputconfig'/>"
"      <separator/>"
"      <menuitem action='altfont'/>"
"      <menuitem action='expandspaces'/>"
"      <menuitem action='scalebitmaps'/>"
"      <separator/>"
"      <menuitem action='preferences'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='helpcontents'/>"
"      <menuitem action='helpindex'/>"
"      <separator/>"
"      <menuitem action='about'/>"
"    </menu>"
"  </menubar>"
"  <toolbar name='ToolBar'>"
"    <toolitem action='back'/>"
"    <toolitem action='history'/>"
"    <toolitem action='bookmarks'/>"
"    <separator/>"
"    <toolitem action='firstpage'/>"
"    <toolitem action='prevphyspage'/>"
"    <toolitem action='prevlogpage'/>"
"    <toolitem action='toc'/>"
"    <toolitem action='nextlogpage'/>"
"    <toolitem action='nextphyspage'/>"
"    <toolitem action='lastpage'/>"
"    <separator/>"
"    <toolitem action='index'/>"
"    <toolitem action='catalog'/>"
"    <toolitem action='xref'/>"
"    <toolitem action='help'/>"
"    <separator/>"
"    <toolitem action='info'/>"
"    <toolitem action='open'/>"
"    <toolitem action='save'/>"
"    <separator/>"
"  </toolbar>"
"</ui>";

WINDOW_DATA *hv_win_new(DOCUMENT *doc, gboolean popup)
{
	WINDOW_DATA *win;
	GtkWidget *vbox, *hbox, *vbox2, *hbox2;
	GtkWidget *menubar;
	GtkWidget *tool_box;
	GtkUIManager *ui_manager;
	GError *error = NULL;
	GtkTextTagTable *tagtable;
	
	win = g_new0(WINDOW_DATA, 1);
	if (win == NULL)
		return NULL;
	win->data = doc;
	
	register_stock_icons();
	
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
 	
 	if (!popup)
 	{
 		GtkAction *recent_action;
 		static GtkRecentManager *recent_manager;
 		
		win->action_group = gtk_action_group_new("AppWindowActions");
		gtk_action_group_add_actions(win->action_group, action_entries, G_N_ELEMENTS(action_entries), win);
		gtk_action_group_add_toggle_actions(win->action_group, toggle_action_entries, G_N_ELEMENTS(toggle_action_entries), win);
		if (!recent_manager)
			recent_manager = gtk_recent_manager_new();
		recent_action = gtk_recent_action_new_for_manager("recent", _("Open _Recent"), NULL, NULL, recent_manager);
		/* g_signal_connect(G_OBJECT(recent_action), "activate", G_CALLBACK(on_recent), win); */
		
		gtk_action_group_add_action(win->action_group, recent_action);
		
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(gtk_action_group_get_action(win->action_group, "altfont")), sel_font_name != gl_profile.viewer.font_name);
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(gtk_action_group_get_action(win->action_group, "expandspaces")), gl_profile.viewer.expand_spaces);
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(gtk_action_group_get_action(win->action_group, "scalebitmaps")), gl_profile.viewer.scale_bitmaps);
		
		ui_manager = gtk_ui_manager_new();
		g_object_set_data_full(G_OBJECT(win->hwnd), "ui-manager", ui_manager, g_object_unref);
		
		gtk_ui_manager_insert_action_group(ui_manager, win->action_group, 0);
		gtk_window_add_accel_group(GTK_WINDOW(win->hwnd), gtk_ui_manager_get_accel_group(ui_manager));
		
		if (!gtk_ui_manager_add_ui_from_string(ui_manager, ui_info, -1, &error))
		{
			g_message("building menus failed: %s", error->message);
			g_error_free(error);
		}
	
		menubar = gtk_ui_manager_get_widget(ui_manager, "/MenuBar");
		gtk_widget_show(menubar);
		gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
	
		win->history_menu = gtk_ui_manager_get_widget(ui_manager, "/MenuBar/FileMenu/recent");
		win->history_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(win->history_menu));
		
		tool_box = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), tool_box, FALSE, FALSE, 0);
		gtk_widget_show(tool_box);
		win->toolbar = gtk_ui_manager_get_widget(ui_manager, "/ToolBar");
		win->toolbar = gtk_toolbar_new();
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
		AppendButton(win, TO_BACK);
		AppendButton(win, TO_HISTORY);
		AppendButton(win, TO_MEMORY);
		AppendButton(win, TO_PREV);
		AppendButton(win, TO_HOME);
		AppendButton(win, TO_NEXT);
		AppendButton(win, TO_INDEX);
		AppendButton(win, TO_KATALOG);
		AppendButton(win, TO_REFERENCES);
		AppendButton(win, TO_HELP);
		AppendButton(win, TO_INFO);
		AppendButton(win, TO_LOAD);
		AppendButton(win, TO_SAVE);
		}
	
		gtk_widget_show_all(win->toolbar);
		
		win->searchbox = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), win->searchbox, TRUE, TRUE, 0);
		win->searchentry = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(win->searchbox), win->searchentry, TRUE, TRUE, 0);
		win->strnotfound = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(win->strnotfound), _("<span color=\"red\">not found!</span>"));
		gtk_box_pack_end(GTK_BOX(win->searchbox), win->strnotfound, FALSE, FALSE, 0);
	}
			
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	vbox2 = gtk_vbox_new(TRUE, 0);
	gtk_widget_show(vbox2);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 0);

	hbox2 = gtk_hbox_new(TRUE, 0);
	gtk_widget_show(hbox2);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox2, TRUE, TRUE, 0);

	tagtable = create_tags();
	win->text_buffer = gtk_text_buffer_new(tagtable);
	g_object_unref(tagtable);
	
	win->text_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(win->text_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(win->text_window), GTK_SHADOW_IN);
	gtk_widget_show(win->text_window);
	win->text_view = gtk_text_view_new_with_buffer(win->text_buffer);
	g_object_unref(win->text_buffer);
	gtk_widget_set_can_default(win->text_view, TRUE);
	gtk_widget_set_receives_default(win->text_view, TRUE);
	gtk_widget_show(win->text_view);
	gtk_container_add(GTK_CONTAINER(win->text_window), win->text_view);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(win->text_view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(win->text_view), GTK_WRAP_NONE); 
	gtk_widget_set_can_focus(win->text_view, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox2), win->text_window, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT(win->hwnd), "destroy", G_CALLBACK(shell_destroyed), (gpointer) win);
	set_font_attributes(win);
	
	all_list = g_slist_prepend(all_list, win);

	if (!popup)
	{
		g_signal_connect(G_OBJECT(win->hwnd), "delete_event", G_CALLBACK(wm_toplevel_close_cb), (gpointer) win);

		doc->window = win;
	
		ToolbarUpdate(doc, FALSE);
	}
	
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

void SendCloseWindow(WINDOW_DATA *win)
{
	if (win)
		SendClose(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

void SendClose(GtkWidget *w)
{
	if (w)
		w = gtk_widget_get_toplevel(w);
	if (w)
	{
		GdkEvent *event = gdk_event_new(GDK_DELETE);
		event->any.window = gtk_widget_get_window(w);
		gdk_event_put(event);
		gdk_event_free(event);
	}
}

/*** ---------------------------------------------------------------------- ***/

void ReInitWindow(DOCUMENT *doc)
{
	WINDOW_DATA *win = doc->window;
	
	win->data = doc;
	win->title = doc->window_title;
	hv_set_title(win, win->title);
	doc->selection.valid = FALSE;
	set_font_attributes(win);
	
	/* adjust window size to new dimensions */
	if (gl_profile.viewer.adjust_winsize)
	{
	}

	/* gtk_text_view_scroll_to_iter(iter-of-line at doc->start_line) */

	ToolbarUpdate(doc, FALSE);
	SendRedraw(win);
}
