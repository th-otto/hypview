#include "hv_gtk.h"
#include "hypdebug.h"

/* YYY */

/*** ---------------------------------------------------------------------- ***/

static void popup_destroyed(GtkWidget *w, void *userdata)
{
	WINDOW_DATA *win = (WINDOW_DATA *)userdata;
	DOCUMENT *doc = win->data;
	UNUSED(w);
	
	doc->popup = NULL;
}

void OpenPopup(DOCUMENT *doc, hyp_nodenr num, int x, int y)
{
	WINDOW_DATA *win = doc->window;

	win = hv_win_new(doc);
	UNUSED(x);
	UNUSED(y);
	
	if (win != NULL)
	{
		HYP_NODE *old_entry = doc->displayed_node;
		long old_lines = doc->lines;
		long old_height = doc->height;
		long old_columns = doc->columns;
		hyp_nav_buttons old_buttons = doc->buttons;
		char *old_wtitle = doc->window_title;

		g_signal_connect(G_OBJECT(win->hwnd), "destroy", G_CALLBACK(popup_destroyed), (gpointer) win);

		if (doc->gotoNodeProc(doc, NULL, num))
		{
			doc->displayed_node = old_entry;
			doc->lines = old_lines;
			doc->height = old_height;
			doc->columns = old_columns;
			doc->buttons = old_buttons;
			doc->window_title = old_wtitle;
	
			doc->popup = win;
			hv_win_open(win);
		} else
		{
			gtk_widget_destroy(win->hwnd);
		}
	}
}
