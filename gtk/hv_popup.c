#include "hv_gtk.h"
#include "hypdebug.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void popup_destroyed(GtkWidget *w, WINDOW_DATA *parentwin)
{
	UNUSED(w);
	
	parentwin->popup = NULL;
}

/*** ---------------------------------------------------------------------- ***/

static void delete_me(GtkWidget *w, WINDOW_DATA *win)
{
	DOCUMENT *doc = win->data;
	UNUSED(w);
	
	hypdoc_unref(doc);
	g_free(win);
}

/*** ---------------------------------------------------------------------- ***/

#if 0
static gboolean popup_grab_on_window(GdkWindow *window, guint32 activate_time, gboolean grab_keyboard)
{
	if ((gdk_pointer_grab(window, TRUE,
						  GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
						  GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
						  GDK_POINTER_MOTION_MASK, NULL, NULL, activate_time) == 0))
	{
		if (!grab_keyboard || gdk_keyboard_grab(window, TRUE, activate_time) == 0)
			return TRUE;
		else
		{
			gdk_display_pointer_ungrab(gdk_window_get_display(window), activate_time);
			return FALSE;
		}
	}

	return FALSE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

void OpenPopup(WINDOW_DATA *parentwin, hyp_nodenr num, int x, int y)
{
	DOCUMENT *doc = parentwin->data;
	DOCUMENT *newdoc = hypdoc_ref(doc);
	WINDOW_DATA *win;
	
	if (parentwin->popup)
		gtk_widget_destroy(parentwin->popup->hwnd);
	win = hv_win_new(newdoc, TRUE);
	if (win != NULL)
	{
		g_signal_connect(G_OBJECT(win->hwnd), "destroy", G_CALLBACK(popup_destroyed), (gpointer) parentwin);
		g_signal_connect(G_OBJECT(win->hwnd), "destroy", G_CALLBACK(delete_me), (gpointer) win);

		if (newdoc->gotoNodeProc(win, NULL, num))
		{
			char *geom;
			
			parentwin->popup = win;
			geom = g_strdup_printf("+%d+%d", x, y);
			gtk_window_parse_geometry(GTK_WINDOW(win->hwnd), geom);
			g_free(geom);
			{
				WP_UNIT w, h;
				
				w = win->x_raster * (win->displayed_node->columns + (sel_font_name == gl_profile.viewer.xfont_name ? 3 : 7));
				h = win->y_raster * (win->displayed_node->lines + 1);
				gtk_widget_set_size_request(win->text_view, w, h);
			}
			gtk_window_set_transient_for(GTK_WINDOW(win->hwnd), GTK_WINDOW(parentwin->hwnd));
			hv_win_open(win);
#if 0
			popup_grab_on_window(gtk_widget_get_window(win->hwnd), gtk_get_current_event_time(), TRUE);
			gtk_grab_add(win->hwnd);
#endif
		} else
		{
			gtk_widget_destroy(win->hwnd);
		}
	}
}
