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
	g_free(win->m_geometry);
	g_free(win);
}

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
			hv_win_set_geometry(win, geom);
			g_free(geom);
#if 1
			{
				WP_UNIT w, h;
				
				w = win->x_raster * (win->displayed_node->columns + (sel_font_name == gl_profile.viewer.xfont_name ? 3 : 7));
				h = win->y_raster * (win->displayed_node->lines + 1);
				gtk_widget_set_size_request(win->text_view, w, h);
			}
#endif
			hv_win_open(win);
			gtk_grab_add(win->hwnd);
		} else
		{
			gtk_widget_destroy(win->hwnd);
		}
	}
}
