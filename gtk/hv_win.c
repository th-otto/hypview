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
static char *default_geometry;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static int read_int(const char *string, const char **next)
{
	int result = 0;
	int sign = 1;

	if (*string == '+')
		string++;
	else if (*string == '-')
	{
		string++;
		sign = -1;
	}

	for (; (*string >= '0') && (*string <= '9'); string++)
	{
		result = (result * 10) + (*string - '0');
	}

	*next = string;

	if (sign >= 0)
		return result;
	return -result;
}

/* 
 * Bitmask returned by XParseGeometry().  Each bit tells if the corresponding
 * value (x, y, width, height) was found in the parsed string.
 */
#define NoValue         0x0000
#define XValue          0x0001
#define YValue          0x0002
#define WidthValue      0x0004
#define HeightValue     0x0008
#define AllValues       0x000F
#define XNegative       0x0010
#define YNegative       0x0020

int gtk_XParseGeometry(const char *string, int *x, int *y, int *width, int *height)
{
	int mask = NoValue;
	const char *strind;
	unsigned int tempWidth, tempHeight;
	int tempX, tempY;

	const char *nextCharacter;

	/* These initializations are just to silence gcc */
	tempWidth = 0;
	tempHeight = 0;
	tempX = 0;
	tempY = 0;

	if ((string == NULL) || (*string == '\0'))
		return mask;
	if (*string == '=')
		string++;						/* ignore possible '=' at beg of geometry spec */

	strind = string;
	if (*strind != '+' && *strind != '-' && *strind != 'x')
	{
		tempWidth = read_int(strind, &nextCharacter);
		if (strind == nextCharacter)
			return NoValue;
		strind = nextCharacter;
		mask |= WidthValue;
	}

	if (*strind == 'x' || *strind == 'X')
	{
		strind++;
		tempHeight = read_int(strind, &nextCharacter);
		if (strind == nextCharacter)
			return NoValue;
		strind = nextCharacter;
		mask |= HeightValue;
	}

	if ((*strind == '+') || (*strind == '-'))
	{
		if (*strind == '-')
		{
			strind++;
			tempX = -read_int(strind, &nextCharacter);
			if (strind == nextCharacter)
				return NoValue;
			strind = nextCharacter;
			mask |= XNegative;

		} else
		{
			strind++;
			tempX = read_int(strind, &nextCharacter);
			if (strind == nextCharacter)
				return NoValue;
			strind = nextCharacter;
		}
		mask |= XValue;
		if ((*strind == '+') || (*strind == '-'))
		{
			if (*strind == '-')
			{
				strind++;
				tempY = -read_int(strind, &nextCharacter);
				if (strind == nextCharacter)
					return NoValue;
				strind = nextCharacter;
				mask |= YNegative;
			} else
			{
				strind++;
				tempY = read_int(strind, &nextCharacter);
				if (strind == nextCharacter)
					return NoValue;
				strind = nextCharacter;
			}
			mask |= YValue;
		}
	}

	/* If strind isn't at the end of the string then it's an invalid
	   geometry specification. */

	if (*strind != '\0')
		return NoValue;

	if (mask & XValue)
		*x = tempX;
	if (mask & YValue)
		*y = tempY;
	if (mask & WidthValue)
		*width = tempWidth;
	if (mask & HeightValue)
		*height = tempHeight;
	return mask;
}

/*** ---------------------------------------------------------------------- ***/

void hv_win_set_geometry(const char *geometry)
{
	g_free(default_geometry);
	default_geometry = g_strdup(geometry);
}

/*** ---------------------------------------------------------------------- ***/

void hv_win_open(WINDOW_DATA *win)
{
	gtk_window_present(GTK_WINDOW(win->hwnd));
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void quit_force(GtkAction *action, void *userdata)
{
	WINDOW_DATA *win = (WINDOW_DATA *)userdata;

	UNUSED(action);
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
		if (show_dialog(win->hwnd, GTK_STOCK_DIALOG_ERROR, msg, TRUE))
			quit_force(NULL, win);
		g_free(msg);
		return FALSE;
	}
	return TRUE;
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
		if (win->popup)
		{
			WINDOW_DATA *pop = win->popup;
			win->popup = NULL;
			gtk_widget_destroy(pop->hwnd);
		}
		hypdoc_unref(doc);
		RemoveAllHistoryEntries(win);
		all_list = g_slist_remove(all_list, win);
		g_free(win->title);
		g_free(win);
	}
}

/*** ---------------------------------------------------------------------- ***/

static void shell_destroyed(GtkWidget *w, WINDOW_DATA *win)
{
	UNUSED(w);
	hv_win_delete(win);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean state_changed(GtkWidget *w, GdkEvent *event, WINDOW_DATA *win)
{
	UNUSED(w);
	if (event->type == GDK_WINDOW_STATE &&
		(event->window_state.changed_mask & GDK_WINDOW_STATE_ICONIFIED))
	{
		DOCUMENT *doc = win->data;
		
		if (event->window_state.new_window_state & GDK_WINDOW_STATE_ICONIFIED)
			hv_set_title(win, hyp_basename(doc->path));
		else
			hv_set_title(win, win->title);
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void on_select_source(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	SelectFileLoad(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_save(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	SelectFileSave(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_font_select(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	SelectFont(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_color_select(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	UNUSED(win);
	printf("NYI: on_color_select\n");
}

/*** ---------------------------------------------------------------------- ***/

static void on_output_settings(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	UNUSED(win);
	printf("NYI: on_output_settings\n");
}

/*** ---------------------------------------------------------------------- ***/

static void on_switch_font(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	SwitchFont(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_expand_spaces(GtkAction *action, WINDOW_DATA *win)
{
	gl_profile.viewer.expand_spaces = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));
	if (win->text_window)
	{
		DOCUMENT *doc = win->data;
		if (doc && doc->prepNode)
		{
			doc->start_line = hv_win_topline(win);
			ReInitWindow(win);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

static void on_scale_bitmaps(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	UNUSED(win);
	printf("NYI: on_scale_bitmaps\n");
}

/*** ---------------------------------------------------------------------- ***/

static void on_preferences(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	UNUSED(win);
	printf("NYI: on_preferences\n");
}

/*** ---------------------------------------------------------------------- ***/

static void on_help_contents(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	Help_Contents(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

static void on_help_index(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	Help_Index(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

#if 0
static gboolean on_history_open(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	UNUSED(win);
	return TRUE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

static void on_about(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	About(win->hwnd);
}

/*** ---------------------------------------------------------------------- ***/

static void on_back(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GoBack(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_clearstack(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	BlockOperation(win, CO_DELETE_STACK);
}

/*** ---------------------------------------------------------------------- ***/

static void on_history(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	ToolbarClick(win, TO_HISTORY, GDK_BUTTON_PRIMARY, gtk_get_current_event_time());
}

/*** ---------------------------------------------------------------------- ***/

static void on_catalog(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GotoCatalog(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_defaultfile(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GotoDefaultFile(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_help(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GotoHelp(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_bookmarks(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	ToolbarClick(win, TO_MEMORY, GDK_BUTTON_PRIMARY, gtk_get_current_event_time());
}

/*** ---------------------------------------------------------------------- ***/

static void on_bookmarks_menu(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	MarkerUpdate(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_next(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GoThisButton(win, TO_NEXT);
}

/*** ---------------------------------------------------------------------- ***/

static void on_nextphys(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GoThisButton(win, TO_NEXT_PHYS);
}

/*** ---------------------------------------------------------------------- ***/

static void on_prev(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GoThisButton(win, TO_PREV);
}

/*** ---------------------------------------------------------------------- ***/

static void on_prevphys(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GoThisButton(win, TO_PREV_PHYS);
}

/*** ---------------------------------------------------------------------- ***/

static void on_toc(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GoThisButton(win, TO_HOME);
}

/*** ---------------------------------------------------------------------- ***/

static void on_first(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GoThisButton(win, TO_FIRST);
}

/*** ---------------------------------------------------------------------- ***/

static void on_last(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GoThisButton(win, TO_LAST);
}

/*** ---------------------------------------------------------------------- ***/

static void on_index(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	GotoIndex(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_xref(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	HypExtRefPopup(win, GDK_BUTTON_PRIMARY, gtk_get_current_event_time());
}

/*** ---------------------------------------------------------------------- ***/

static void on_quit(GtkAction *action, WINDOW_DATA *win)
{
	if (!WriteProfile(win))
		return;
	quit_force(action, win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_info(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	DocumentInfos(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_close(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	SendCloseWindow(win);
}

/*** ---------------------------------------------------------------------- ***/

static void on_remarker(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	BlockOperation(win, CO_REMARKER);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean wm_toplevel_close_cb(GtkWidget *widget, GdkEvent *event, WINDOW_DATA *win)
{
	UNUSED(widget);
	UNUSED(event);
	on_quit(NULL, win);
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
		ToolbarClick(win, button_num, event->button, event->time);
		return TRUE;
	}
	return FALSE;
}

#endif

/*** ---------------------------------------------------------------------- ***/

/* Looks at all tags covering the position (x, y) in the text view, 
 * and if one of them is a link, change the cursor to the "hands" cursor
 * typically used by web browsers.
 */
static GdkCursor *hand_cursor = NULL;
static GdkCursor *regular_cursor = NULL;

static void set_cursor_if_appropriate(WINDOW_DATA *win, gint x, gint y)
{
	GtkTextView *text_view = (GtkTextView *)win->text_view;
	GSList *tags;
	GSList *tagp;
	GtkTextIter iter;
	gboolean hovering = FALSE;

	gtk_text_view_get_iter_at_location(text_view, &iter, x, y);

	tags = gtk_text_iter_get_tags(&iter);
	for (tagp = tags; tagp != NULL; tagp = tagp->next)
	{
		GtkTextTag *tag = tagp->data;
		LINK_INFO *info = g_object_get_data(G_OBJECT(tag), "hv-linkinfo");

		if (info)
		{
			if (info->tip)
				gtk_widget_set_tooltip_text(GTK_WIDGET(text_view), info->tip);
			hovering = TRUE;
			break;
		}
	}

	if (hovering != win->hovering_over_link)
	{
		GdkWindow *window = gtk_text_view_get_window(text_view, GTK_TEXT_WINDOW_TEXT);
		
		if (window)
		{
			win->hovering_over_link = hovering;
			if (hovering)
			{
				gdk_window_set_cursor(window, hand_cursor);
			} else
			{
				gdk_window_set_cursor(window, regular_cursor);
				gtk_widget_set_tooltip_text(GTK_WIDGET(text_view), NULL);
			}
		}
	}

	if (tags)
		g_slist_free(tags);
}

/* Update the cursor image if the pointer moved. 
 */
static gboolean motion_notify_event(GtkWidget *text_view, GdkEventMotion *event, WINDOW_DATA *win)
{
	gint x, y;

	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(text_view), GTK_TEXT_WINDOW_WIDGET, event->x, event->y, &x, &y);

	set_cursor_if_appropriate(win, x, y);

	gdk_window_get_pointer(text_view->window, NULL, NULL, NULL);
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/* Looks at all tags covering the position of iter in the text view, 
 * and if one of them is a link, follow it by showing the page identified
 * by the data attached to it.
 */
static gboolean follow_if_link(WINDOW_DATA *win, GtkTextIter *iter)
{
	GSList *tags;
	GSList *tagp;
	gboolean found = FALSE;
	
	UNUSED(win);
	tags = gtk_text_iter_get_tags(iter);
	for (tagp = tags; tagp != NULL; tagp = tagp->next)
	{
		GtkTextTag *tag = tagp->data;
		LINK_INFO *info = g_object_get_data(G_OBJECT(tag), "hv-linkinfo");
		
		if (info)
		{
			HypClick(win, info);
			found = TRUE;
			break;
		}
	}

	if (tags)
		g_slist_free(tags);
	return found;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean event_after(GtkWidget *text_view, GdkEventButton *event, WINDOW_DATA *win)
{
	GtkTextIter start, end, iter;
	GtkTextBuffer *buffer;
	gint x, y;

	if (event->type == GDK_BUTTON_PRESS || event->type == GDK_KEY_PRESS)
	{
		if (win->popup)
			gtk_widget_destroy(win->popup->hwnd);
	}
	
	if (event->type != GDK_BUTTON_RELEASE)
		return FALSE;

	if (gtk_window_get_window_type(GTK_WINDOW(win->hwnd)) == GTK_WINDOW_POPUP)
	{
		gtk_widget_destroy(win->hwnd);
		return TRUE;
	}
	
	if (event->button != GDK_BUTTON_PRIMARY)
		return FALSE;

	CheckFiledate(win);
	
	buffer = win->text_buffer;

	/* we shouldn't follow a link if the user has selected something */
	gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
	if (gtk_text_iter_get_offset(&start) != gtk_text_iter_get_offset(&end))
		return FALSE;

	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(text_view), GTK_TEXT_WINDOW_WIDGET, event->x, event->y, &x, &y);

	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(text_view), &iter, x, y);

	follow_if_link(win, &iter);

	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static void hv_scroll_window(WINDOW_DATA *win, gint xamount, gint yamount)
{
	GdkWindow *window;
	GtkAdjustment *adj;
	gdouble val;
	
	window = gtk_text_view_get_window(GTK_TEXT_VIEW(win->text_view), GTK_TEXT_WINDOW_TEXT);
	if (!window)
		return;
	if (xamount != 0)
	{
		adj = gtk_text_view_get_hadjustment(GTK_TEXT_VIEW(win->text_view));
		val = adj->value + xamount;
		if (val > (adj->upper - adj->page_size))
			val = adj->upper - adj->page_size;
		if (val < adj->lower)
			val = adj->lower;
		if (val != adj->value)
			gtk_adjustment_set_value(adj, val);
	}
	if (yamount != 0)
	{
		adj = gtk_text_view_get_vadjustment(GTK_TEXT_VIEW(win->text_view));
		val = adj->value + yamount;
		if (val > (adj->upper - adj->page_size))
			val = adj->upper - adj->page_size;
		if (val < adj->lower)
			val = adj->lower;
		if (val != adj->value)
			gtk_adjustment_set_value(adj, val);
	}
}

/*** ---------------------------------------------------------------------- ***/

static gboolean key_press_event(GtkWidget *text_view, GdkEventKey *event, WINDOW_DATA *win)
{
	gboolean handled = TRUE;
	GtkTextIter iter;
	DOCUMENT *doc = win->data;
	gint win_w, win_h;
	
	if (gtk_window_get_window_type(GTK_WINDOW(win->hwnd)) == GTK_WINDOW_POPUP)
	{
		gtk_widget_destroy(win->hwnd);
		return TRUE;
	}
	
	gdk_drawable_get_size(gtk_text_view_get_window(GTK_TEXT_VIEW(win->text_view), GTK_TEXT_WINDOW_TEXT), &win_w, &win_h);
	UNUSED(text_view);
	switch (event->keyval)
	{
	case GDK_KEY_Return:
	case GDK_KEY_KP_Enter:
		gtk_text_buffer_get_iter_at_mark(win->text_buffer, &iter, win->curlink_mark);
		handled = follow_if_link(win, &iter);
		break;
	case GDK_KEY_Left:
		if ((event->state & GDK_SHIFT_MASK) && (event->state & GDK_CONTROL_MASK))
			GoThisButton(win, TO_PREV);
		else if ((event->state & GDK_SHIFT_MASK))
			hv_scroll_window(win, -win_w, 0);
		else if ((event->state & GDK_CONTROL_MASK))
			GoThisButton(win, TO_PREV);
		else
			hv_scroll_window(win, -win->x_raster, 0);
		break;
	case GDK_KEY_Right:
		if ((event->state & GDK_SHIFT_MASK) && (event->state & GDK_CONTROL_MASK))
			GoThisButton(win, TO_NEXT);
		else if ((event->state & GDK_SHIFT_MASK))
			hv_scroll_window(win, win_w, 0);
		else if ((event->state & GDK_CONTROL_MASK))
			GoThisButton(win, TO_NEXT);
		else
			hv_scroll_window(win, win->x_raster, 0);
		break;
	case GDK_KEY_Up:
		if ((event->state & GDK_SHIFT_MASK) && (event->state & GDK_CONTROL_MASK))
			GoThisButton(win, TO_PREV);
		else if ((event->state & GDK_SHIFT_MASK))
			hv_scroll_window(win, 0, -win_h);
		else if ((event->state & GDK_CONTROL_MASK))
			hv_scroll_window(win, 0, -win_h);
		else
			hv_scroll_window(win, 0, -win->y_raster);
		break;
	case GDK_KEY_Down:
		if ((event->state & GDK_SHIFT_MASK) && (event->state & GDK_CONTROL_MASK))
			GoThisButton(win, TO_NEXT);
		else if ((event->state & GDK_SHIFT_MASK))
			hv_scroll_window(win, 0, win_h);
		else if ((event->state & GDK_CONTROL_MASK))
			hv_scroll_window(win, 0, win_h);
		else
			hv_scroll_window(win, 0, win->y_raster);
		break;
	case GDK_KEY_KP_Subtract:
		GoThisButton(win, TO_PREV);
		break;
	case GDK_KEY_KP_Add:
		GoThisButton(win, TO_NEXT);
		break;
	case GDK_KEY_KP_Divide:
		GoThisButton(win, TO_PREV_PHYS);
		break;
	case GDK_KEY_KP_Multiply:
		GoThisButton(win, TO_NEXT_PHYS);
		break;
	case GDK_KEY_Help:
		GotoHelp(win);
		break;
	case GDK_KEY_Escape:
	case GDK_KEY_BackSpace:
		if (!(doc->buttons.searchbox))
			GoThisButton(win, TO_BACK);
		else
			handled = FALSE;
		break;
	case GDK_KEY_Undo:
		GoThisButton(win, TO_BACK);
		break;
	case GDK_KEY_Page_Up:			/* already handled by GtkTextView */
	case GDK_KEY_Page_Down:			/* already handled by GtkTextView */
	case GDK_KEY_End:				/* already handled by GtkTextView */
	case GDK_KEY_Home:				/* already handled by GtkTextView */
	case GDK_KEY_F1:				/* already handled by actions */
	case GDK_KEY_F2:				/* already handled by actions */
	case GDK_KEY_F3:				/* already handled by actions */
	case GDK_KEY_F4:				/* already handled by actions */
	case GDK_KEY_F5:				/* already handled by actions */
	case GDK_KEY_F6:				/* already handled by actions */
	case GDK_KEY_F7:				/* already handled by actions */
	case GDK_KEY_F8:				/* already handled by actions */
	case GDK_KEY_F9:				/* already handled by actions */
	case GDK_KEY_F10:				/* already handled by actions */
	default:
		handled = FALSE;
		break;
	}
	if (!handled)
		handled = AutolocatorKey(win, event);
	return handled;
}

/*** ---------------------------------------------------------------------- ***/

static void drag_data_received(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, GtkSelectionData *data, guint info, guint time, WINDOW_DATA *win)
{
	UNUSED(widget);
	UNUSED(info);
	UNUSED(x);
	UNUSED(y);
	if (drag_context->action == GDK_ACTION_ASK)
	{
		drag_context->action = GDK_ACTION_COPY;
	}
	if (data->length > 0 && data->format == 8 && gtk_targets_include_uri(&data->target, 1))
	{
		char **names = g_strsplit((const char *)data->data, "\r\n", 0);
		int i;
		
		for (i = 0; names[i] != 0; i++)
		{
			char *scheme = g_uri_parse_scheme(names[i]);
			if (strcmp(scheme, "file") == 0)
			{
				char *filename = g_filename_from_uri(names[i], NULL, NULL);
				OpenFileInWindow(win, filename, hyp_default_main_node_name, HYP_NOINDEX, FALSE, gl_profile.viewer.va_start_newwin, TRUE);
				g_free(filename);
			}
			g_free(scheme);
		}
		g_strfreev(names);
	}
	gtk_drag_finish(drag_context, TRUE, FALSE, time);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean drag_motion(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint time, WINDOW_DATA *win)
{
	GdkAtom target;
	
	UNUSED(widget);
	UNUSED(x);
	UNUSED(y);
	UNUSED(win);
	target = gtk_drag_dest_find_target(widget, drag_context, NULL);
	if (drag_context->action == GDK_ACTION_ASK)
	{
		drag_context->action = GDK_ACTION_COPY;
	}
	if (target == GDK_NONE)
		gdk_drag_status(drag_context, 0, time);
	else
		gdk_drag_status(drag_context, GDK_ACTION_COPY, time);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean drag_drop(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint time, WINDOW_DATA *win)
{
	GdkAtom target;

	UNUSED(x);
	UNUSED(y);
	UNUSED(win);
	target = gtk_drag_dest_find_target(widget, drag_context, NULL);
	gtk_drag_get_data(widget, drag_context, target, time);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

DOCUMENT *hypwin_doc(WINDOW_DATA *win)
{
	return win->data;
}

/*** ---------------------------------------------------------------------- ***/

static void populate_popup(GtkWidget *text_view, GtkMenu *menu, WINDOW_DATA *win)
{
	GList *children, *child;
	
	UNUSED(text_view);
	UNUSED(win);
	children = gtk_container_get_children(GTK_CONTAINER(menu));
	for (child = children; child; child = child->next)
	{
		GtkMenuItem *item = child->data;
		const char *signal = g_object_get_data(G_OBJECT(item), "gtk-signal");
		
		if (signal && strcmp(signal, "paste-clipboard") == 0)
		{
			/*
			 * the default popup disables this entry, because the textbuffer
			 * is not editable. Fix that.
			 */
			gtk_widget_set_sensitive(GTK_WIDGET(item), TRUE);
		}
		if (signal && strcmp(signal, "copy-clipboard") == 0)
		{
			/*
			 * the default popup disables this entry if there is no selection.
			 * Fix that.
			 */
			gtk_widget_set_sensitive(GTK_WIDGET(item), TRUE);
		}
		if (signal && strcmp(signal, "cut-clipboard") == 0)
		{
			/* we don't need this entry */
			gtk_widget_hide(GTK_WIDGET(item));
		}
	}
	g_list_free(children);
}

/*** ---------------------------------------------------------------------- ***/

static void paste_clipboard(GtkWidget *text_view, WINDOW_DATA *win)
{
	UNUSED(text_view);
	BlockOperation(win, CO_PASTE);
}

/*** ---------------------------------------------------------------------- ***/

static void on_select_all(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	BlockOperation(win, CO_SELECT_ALL);
}

/*** ---------------------------------------------------------------------- ***/

static void on_paste_clipboard(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	BlockOperation(win, CO_PASTE);
}

/*** ---------------------------------------------------------------------- ***/

static void copy_clipboard(GtkWidget *text_view, WINDOW_DATA *win)
{
	UNUSED(text_view);
	BlockOperation(win, CO_COPY);
}

/*** ---------------------------------------------------------------------- ***/

static void on_copy_clipboard(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	BlockOperation(win, CO_COPY);
}

/*** ---------------------------------------------------------------------- ***/

static void on_search(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	BlockOperation(win, CO_SEARCH);
}

/*** ---------------------------------------------------------------------- ***/

static void on_search_again(GtkAction *action, WINDOW_DATA *win)
{
	UNUSED(action);
	BlockOperation(win, CO_SEARCH_AGAIN);
}

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

	hand_cursor = gdk_cursor_new(GDK_HAND2);
	regular_cursor = gdk_cursor_new(GDK_XTERM);
	
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
	{ "BookmarksMenu",      "hv-bookmarks",          N_("Bookmarks"),                       NULL,          N_("Show list of bookmarks"),                        G_CALLBACK(on_bookmarks_menu) },
	/*
	 * menu entries
	 * name,                stock id,                label                                  accelerator    tooltip                                              callback
	 */
	{ "open",               "hv-load",               N_("_Open Hypertext..."),              "<Ctrl>O",     N_("Load a file"),                                   G_CALLBACK(on_select_source) },
	{ "save",               "hv-save",               N_("_Save text..."),                   "<Ctrl>S",     N_("Save page to file"),                             G_CALLBACK(on_save) },
	{ "info",               "hv-info",               N_("_File info..."),                   "<Ctrl>I",     N_("Show info about hypertext"),                     G_CALLBACK(on_info) },
	{ "remarker",           NULL,                    N_("_Run Remarker"),                   "<Alt>R",      NULL,                                                G_CALLBACK(on_remarker) },
	{ "close",              "gtk-close",             N_("_Close"),                          "<Ctrl>U",     NULL,                                                G_CALLBACK(on_close) },
	{ "quit",               "gtk-quit",              N_("_Quit"),                           "<Ctrl>Q",     NULL,                                                G_CALLBACK(on_quit) },

	{ "edit-copy",          "gtk-copy",              N_("_Copy"),                           "<Ctrl>C",     NULL,                                                G_CALLBACK(on_copy_clipboard) },
	{ "edit-paste",         "gtk-paste",             N_("_Paste"),                          "<Ctrl>V",     NULL,                                                G_CALLBACK(on_paste_clipboard) },
	{ "edit-find",          "gtk-find",              N_("_Find"),                           "<Ctrl>F",     NULL,                                                G_CALLBACK(on_search) },
	{ "edit-find-next",     "gtk-find-next",         N_("Find _Next"),                      "<Ctrl>G",     NULL,                                                G_CALLBACK(on_search_again) },
	{ "edit-select-all",    "gtk-select-all",        N_("Select _All"),                     "<Ctrl>A",     NULL,                                                G_CALLBACK(on_select_all) },

	{ "back",               "hv-back",               N_("Back one page"),                   NULL,          N_("Back one page"),                                 G_CALLBACK(on_back) },
	{ "clearstack",         NULL,                    N_("Clear stack"),                     "<Alt>E",      N_("Clear stack"),                                   G_CALLBACK(on_clearstack) },
	{ "history",            "hv-history",            N_("History"),                         NULL,          N_("Show history of pages"),                         G_CALLBACK(on_history) },
	{ "bookmarks",          "hv-bookmarks",          N_("Bookmarks"),                       NULL,          N_("Show list of bookmarks"),                        G_CALLBACK(on_bookmarks) },
	{ "prevphyspage",       "hv-prevphys",           N_("Previous physical page"),          NULL,          N_("Goto previous physical page"),                   G_CALLBACK(on_prevphys) },
	{ "prevlogpage",        "hv-prev",               N_("Previous logical page"),           NULL,          N_("Goto previous page"),                            G_CALLBACK(on_prev) },
	{ "toc",                "hv-toc",                N_("Contents"),                        "<Alt>T",      N_("Go up one page"),                                G_CALLBACK(on_toc) },
	{ "nextlogpage",        "hv-next",               N_("Next logical page"),               NULL,          N_("Goto next page"),                                G_CALLBACK(on_next) },
	{ "nextphyspage",       "hv-nextphys",           N_("Next physical page"),              NULL,          N_("Goto next physical page"),                       G_CALLBACK(on_nextphys) },
	{ "firstpage",          "hv-first",              N_("First page"),                      NULL,          N_("Goto first page"),                               G_CALLBACK(on_first) },
	{ "lastpage",           "hv-last",               N_("Last page"),                       NULL,          N_("Goto last page"),                                G_CALLBACK(on_last) },
	{ "index",              "hv-index",              N_("Index"),                           "<Alt>X",      N_("Goto index page"),                               G_CALLBACK(on_index) },
	{ "catalog",            "hv-catalog",            N_("Catalog"),                         "<Alt>K",      N_("Show catalog of hypertexts"),                    G_CALLBACK(on_catalog) },
	{ "defaultfile",        NULL,                    N_("Default file"),                    "<Alt>D",      N_("Show default file"),                             G_CALLBACK(on_defaultfile) },
	{ "xref",               "hv-xref",               N_("References"),                      NULL,          N_("Show list of cross references"),                 G_CALLBACK(on_xref) },
	{ "help",               "hv-help",               N_("Show help page"),                  NULL,          N_("Show help page"),                                G_CALLBACK(on_help) },

	{ "bookmark-1",         NULL,                    N_("free"),                            "F1",          N_("Open bookmark"),                                 G_CALLBACK(on_bookmark_selected) },
	{ "bookmark-2",         NULL,                    N_("free"),                            "F2",          N_("Open bookmark"),                                 G_CALLBACK(on_bookmark_selected) },
	{ "bookmark-3",         NULL,                    N_("free"),                            "F3",          N_("Open bookmark"),                                 G_CALLBACK(on_bookmark_selected) },
	{ "bookmark-4",         NULL,                    N_("free"),                            "F4",          N_("Open bookmark"),                                 G_CALLBACK(on_bookmark_selected) },
	{ "bookmark-5",         NULL,                    N_("free"),                            "F5",          N_("Open bookmark"),                                 G_CALLBACK(on_bookmark_selected) },
	{ "bookmark-6",         NULL,                    N_("free"),                            "F6",          N_("Open bookmark"),                                 G_CALLBACK(on_bookmark_selected) },
	{ "bookmark-7",         NULL,                    N_("free"),                            "F7",          N_("Open bookmark"),                                 G_CALLBACK(on_bookmark_selected) },
	{ "bookmark-8",         NULL,                    N_("free"),                            "F8",          N_("Open bookmark"),                                 G_CALLBACK(on_bookmark_selected) },
	{ "bookmark-9",         NULL,                    N_("free"),                            "F9",          N_("Open bookmark"),                                 G_CALLBACK(on_bookmark_selected) },
	{ "bookmark-10",        NULL,                    N_("free"),                            "F10",         N_("Open bookmark"),                                 G_CALLBACK(on_bookmark_selected) },

	{ "selectfont",         "gtk-font",              N_("_Font..."),                        "<Alt>Z",      NULL,                                                G_CALLBACK(on_font_select) },
	{ "selectcolors",       NULL,                    N_("_Colors..."),                      NULL,          NULL,                                                G_CALLBACK(on_color_select) },
	{ "outputconfig",       NULL,                    N_("_Output..."),                      NULL,          NULL,                                                G_CALLBACK(on_output_settings) },
	{ "preferences",        "gtk-preferences",       N_("_Settings..."),                    NULL,          NULL,                                                G_CALLBACK(on_preferences) },

	{ "helpcontents",       "gtk-info",              N_("_Contents"),                       NULL,          NULL,                                                G_CALLBACK(on_help_contents) },
	{ "helpindex",          "gtk-index",             N_("_Index"),                          NULL,          NULL,                                                G_CALLBACK(on_help_index) },
	{ "about",              "gtk-about",             N_("_About"),                          NULL,          NULL,                                                G_CALLBACK(on_about) },
};

static GtkToggleActionEntry const toggle_action_entries[] = {
	{ "altfont",            NULL,                    N_("_Alternative font"),               "<Ctrl>Z",     NULL,                                             G_CALLBACK(on_switch_font), FALSE },
	{ "expandspaces",       NULL,                    N_("_Expand multiple spaces"),         "<Ctrl>L",     NULL,                                                G_CALLBACK(on_expand_spaces), FALSE },
	{ "scalebitmaps",       NULL,                    N_("_Scale bitmaps"),                  "<Ctrl>B",     NULL,                                                G_CALLBACK(on_scale_bitmaps), FALSE },
};

static char const ui_info[] =
"<ui>\n"
"  <menubar name='MenuBar'>\n"
"    <menu action='FileMenu'>\n"
"      <menuitem action='open'/>\n"
"      <separator/>\n"
"      <menuitem action='save'/>\n"
"      <separator/>\n"
"      <menuitem action='catalog' />\n"
"      <menuitem action='defaultfile' />\n"
"      <menuitem action='remarker' />\n"
"      <separator/>\n"
"      <menuitem action='recent' />\n"
"      <separator/>\n"
"      <menuitem action='info'/>\n"
"      <separator/>\n"
"      <menuitem action='close'/>\n"
"      <menuitem action='quit'/>\n"
"    </menu>\n"
"    <menu action='EditMenu'>\n"
"      <menuitem action='edit-select-all'/>\n"
"      <menuitem action='edit-copy'/>\n"
"      <menuitem action='edit-paste'/>\n"
"      <separator/>\n"
"      <menuitem action='edit-find'/>\n"
"      <menuitem action='edit-find-next'/>\n"
"    </menu>\n"
"    <menu action='NavigateMenu'>\n"
"      <menuitem action='prevlogpage'/>\n"
"      <menuitem action='nextlogpage'/>\n"
"      <separator/>\n"
"      <menuitem action='prevphyspage'/>\n"
"      <menuitem action='nextphyspage'/>\n"
"      <separator/>\n"
"      <menuitem action='firstpage'/>\n"
"      <menuitem action='lastpage'/>\n"
"      <separator/>\n"
"      <menuitem action='toc'/>\n"
"      <menuitem action='index'/>\n"
"      <menuitem action='help'/>\n"
"      <separator/>\n"
"      <menu action='BookmarksMenu'>\n"
"        <menuitem action='bookmark-1'/>\n"
"        <menuitem action='bookmark-2'/>\n"
"        <menuitem action='bookmark-3'/>\n"
"        <menuitem action='bookmark-4'/>\n"
"        <menuitem action='bookmark-5'/>\n"
"        <menuitem action='bookmark-6'/>\n"
"        <menuitem action='bookmark-7'/>\n"
"        <menuitem action='bookmark-8'/>\n"
"        <menuitem action='bookmark-9'/>\n"
"        <menuitem action='bookmark-10'/>\n"
"      </menu>\n"
"      <separator/>\n"
"      <menuitem action='back'/>\n"
"      <menuitem action='clearstack'/>\n"
"    </menu>\n"
"    <menu action='OptionsMenu'>\n"
"      <menuitem action='selectfont'/>\n"
"      <menuitem action='selectcolors'/>\n"
"      <separator/>\n"
"      <menuitem action='outputconfig'/>\n"
"      <menuitem action='outputconfig'/>\n"
"      <separator/>\n"
"      <menuitem action='altfont'/>\n"
"      <menuitem action='expandspaces'/>\n"
"      <menuitem action='scalebitmaps'/>\n"
"      <separator/>\n"
"      <menuitem action='preferences'/>\n"
"    </menu>\n"
"    <menu action='HelpMenu'>\n"
"      <menuitem action='helpcontents'/>\n"
"      <menuitem action='helpindex'/>\n"
"      <separator/>\n"
"      <menuitem action='about'/>\n"
"    </menu>\n"
"  </menubar>\n"
"  <toolbar name='ToolBar'>\n"
"    <toolitem action='back'/>\n"
"    <toolitem action='history'/>\n"
"    <toolitem action='bookmarks'/>\n"
"    <separator/>\n"
"    <toolitem action='firstpage'/>\n"
"    <toolitem action='prevphyspage'/>\n"
"    <toolitem action='prevlogpage'/>\n"
"    <toolitem action='toc'/>\n"
"    <toolitem action='nextlogpage'/>\n"
"    <toolitem action='nextphyspage'/>\n"
"    <toolitem action='lastpage'/>\n"
"    <separator/>\n"
"    <toolitem action='index'/>\n"
"    <toolitem action='catalog'/>\n"
"    <toolitem action='xref'/>\n"
"    <toolitem action='help'/>\n"
"    <separator/>\n"
"    <toolitem action='info'/>\n"
"    <toolitem action='open'/>\n"
"    <toolitem action='save'/>\n"
"    <separator/>\n"
"  </toolbar>\n"
"</ui>\n";

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
	
	win->hwnd = gtk_window_new(popup ? GTK_WINDOW_POPUP : GTK_WINDOW_TOPLEVEL);
	g_object_set_data(G_OBJECT(win->hwnd), "shell-dialog", win);
	g_object_set_data(G_OBJECT(win->hwnd), "hypview_window_type", NO_CONST("shell-window"));
	win->title = g_strdup(doc->path);
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
		
		win->bookmarks_menu = gtk_ui_manager_get_widget(ui_manager, "/MenuBar/NavigateMenu/BookmarksMenu");
		win->bookmarks_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(win->bookmarks_menu));
		
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
		gtk_box_pack_start(GTK_BOX(vbox), win->searchbox, FALSE, TRUE, 0);
		win->searchentry = gtk_entry_new();
		gtk_widget_show(win->searchentry);
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
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(win->text_view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(win->text_view), GTK_WRAP_NONE); 
	gtk_widget_set_can_focus(win->text_view, TRUE);
	gtk_box_pack_start(GTK_BOX(hbox2), win->text_window, TRUE, TRUE, 0);

	/*
	 * we don't want error beeps from unhandled key presses
	 */
	{
		GtkSettings *settings;
	
		settings = gtk_widget_get_settings(win->text_view);
		if (settings)
			g_object_set(settings, "gtk-error-bell", FALSE, NULL);
	}
	
	g_signal_connect(G_OBJECT(win->hwnd), "window-state-event", G_CALLBACK(state_changed), (gpointer) win);
	g_signal_connect(G_OBJECT(win->hwnd), "frame-event", G_CALLBACK(state_changed), (gpointer) win);
	g_signal_connect(G_OBJECT(win->text_view), "motion-notify-event",  G_CALLBACK(motion_notify_event), win);
	g_signal_connect(G_OBJECT(win->text_view), "event-after", G_CALLBACK(event_after), win);
	g_signal_connect(G_OBJECT(win->text_view), "key-press-event", G_CALLBACK(key_press_event), win);
	g_signal_connect(G_OBJECT(win->text_view), "populate-popup", G_CALLBACK(populate_popup), win);
	g_signal_connect(G_OBJECT(win->text_view), "paste-clipboard", G_CALLBACK(paste_clipboard), win);
	g_signal_connect(G_OBJECT(win->text_view), "copy-clipboard", G_CALLBACK(copy_clipboard), win);
	
    set_font_attributes(win);
	
	if (!popup)
	{
		all_list = g_slist_prepend(all_list, win);

		g_signal_connect(G_OBJECT(win->hwnd), "destroy", G_CALLBACK(shell_destroyed), (gpointer) win);
		g_signal_connect(G_OBJECT(win->hwnd), "delete_event", G_CALLBACK(wm_toplevel_close_cb), (gpointer) win);

		ToolbarUpdate(win, FALSE);

		g_signal_connect(G_OBJECT(win->text_view), "drag-data-received", G_CALLBACK(drag_data_received), (gpointer) win);
		g_signal_connect(G_OBJECT(win->text_view), "drag-motion", G_CALLBACK(drag_motion), (gpointer) win);
		g_signal_connect(G_OBJECT(win->text_view), "drag-drop", G_CALLBACK(drag_drop), (gpointer) win);

		/*
		 * remove the plain text targets from the drag destion target list;
		 * our view is not editable and we dont' want to accept them
		 */
		{
			GtkTargetList *newlist;
			
			newlist = gtk_target_list_new(NULL, 0);
			gtk_target_list_add_uri_targets(newlist, 0);
			gtk_drag_dest_set_target_list(win->text_view, newlist);
		}
		if (default_geometry != NULL)
		{
			gtk_window_parse_geometry(GTK_WINDOW(win->hwnd), default_geometry);
		}	
	}
	
	return win;
}

/*** ---------------------------------------------------------------------- ***/

void hv_set_title(WINDOW_DATA *win, const char *title)
{
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

long hv_win_topline(WINDOW_DATA *win)
{
	GtkTextIter iter;
	long lineno;
	
	int x, y;
	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(win->text_view), GTK_TEXT_WINDOW_TEXT, 0, 0, &x, &y);
	gtk_text_view_get_line_at_y(GTK_TEXT_VIEW(win->text_view), &iter, y, NULL);
	lineno = gtk_text_iter_get_line(&iter);
	return lineno;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * scroll window such that <line> is displayed at the top
 */
void hv_win_scroll_to_line(WINDOW_DATA *win, long line)
{
	GdkWindow *window;
	GtkTextIter iter;
	GtkAdjustment *adj;
	gdouble val;
	
	window = gtk_text_view_get_window(GTK_TEXT_VIEW(win->text_view), GTK_TEXT_WINDOW_TEXT);
	if (window)
		gdk_window_process_updates(window, TRUE);
	gtk_text_buffer_get_iter_at_line(win->text_buffer, &iter, line);
#if 0
	{
		GtkTextMark *topmark;
		topmark = gtk_text_buffer_get_mark(win->text_buffer, "hv-topmark");
		if (topmark == NULL)
			topmark = gtk_text_buffer_create_mark(win->text_buffer, "hv-topmark", &iter, TRUE);
		else
			gtk_text_buffer_move_mark(win->text_buffer, topmark, &iter);
	}
#endif
	adj = gtk_text_view_get_vadjustment(GTK_TEXT_VIEW(win->text_view));
	{
		GdkRectangle rect;
		gtk_text_view_get_iter_location(GTK_TEXT_VIEW(win->text_view), &iter, &rect);
		val = rect.y;
#if 0
		printf("start %ld %d rect %d %d %d %d adj %f %f %f %f\n",
			line,
			gtk_text_iter_get_line(&iter),
			rect.x, rect.y, rect.width, rect.height,
			adj->lower, adj->upper, adj->page_size, adj->upper - adj->page_size);
#endif
	}
#if 0
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(win->text_view), &iter, 0.0, TRUE, 0.0, 0.0);
#else
	if (val > adj->upper)
		val = adj->upper;
	if (val < adj->lower)
		val = adj->lower;
	if (val != adj->value)
		gtk_adjustment_set_value(adj, val);
#endif
}

/*** ---------------------------------------------------------------------- ***/

void ReInitWindow(WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	GdkWindow *window;
	
	win->data = doc;
	win->hovering_over_link = FALSE;
	window = gtk_text_view_get_window(GTK_TEXT_VIEW(win->text_view), GTK_TEXT_WINDOW_TEXT);
	if (window)
		gdk_window_set_cursor(window, regular_cursor);
	gtk_widget_set_tooltip_text(GTK_WIDGET(win->text_view), NULL);
	set_font_attributes(win);
	doc->prepNode(win, win->displayed_node);
	hv_set_title(win, win->title);
	
	/* adjust window size to new dimensions */
	if (gl_profile.viewer.adjust_winsize)
	{
	}

	if (doc->start_line)
	{
		hv_win_scroll_to_line(win, doc->start_line);
	}
	
	ToolbarUpdate(win, FALSE);
	SendRedraw(win);
}
