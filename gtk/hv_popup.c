#include "hv_gtk.h"
#include "hypdebug.h"

/* YYY */

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
	WINDOW_DATA *win = hv_win_new(newdoc, TRUE);
	
	if (win != NULL)
	{
		g_signal_connect(G_OBJECT(win->hwnd), "destroy", G_CALLBACK(popup_destroyed), (gpointer) parentwin);
		g_signal_connect(G_OBJECT(win->hwnd), "destroy", G_CALLBACK(delete_me), (gpointer) win);

		if (newdoc->gotoNodeProc(win, NULL, num))
		{
			char *geom;
			
			parentwin->popup = win;
			geom = g_strdup_printf("%dx%ld+%d+%d", gl_profile.viewer.win_w, win->y_raster * newdoc->displayed_node->lines, x, y);
			hv_win_set_geometry(win, geom);
			g_free(geom);
			hv_win_open(win);
		} else
		{
			gtk_widget_destroy(win->hwnd);
		}
	}
}
