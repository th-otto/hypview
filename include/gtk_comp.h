#ifndef glib_get_major_version
#define glib_get_major_version() glib_major_version
#define glib_get_minor_version() glib_minor_version
#define glib_get_micro_version() glib_micro_version
#endif

/*
 * Gtk 2.18 already has GSEAL on some classes,
 * but not the accessor functions like gtk_widget_get_realized()
 */
#if GTK_CHECK_VERSION(2, 14, 0)
#undef GTK_OBJECT_FLAGS
#define GTK_OBJECT_FLAGS(obj)		  (GTK_OBJECT (obj)->GSEAL(flags))
#endif

#if !GTK_CHECK_VERSION(2, 14, 0)
#define gtk_dialog_get_content_area(dialog) ((dialog)->GSEAL(vbox))
#define gtk_dialog_get_action_area(dialog) ((dialog)->GSEAL(action_area))
#define gtk_widget_get_window(w) ((w)->GSEAL(window))
#endif

#if !GTK_CHECK_VERSION(2, 14, 0)
#define gtk_font_selection_dialog_get_ok_button(sel) ((sel)->GSEAL(ok_button))
#define gtk_font_selection_dialog_get_cancel_button(sel) ((sel)->GSEAL(cancel_button))
#define gtk_adjustment_get_value(adj) ((adj)->GSEAL(value))
#define gtk_adjustment_get_lower(adj) ((adj)->GSEAL(lower))
#define gtk_adjustment_get_upper(adj) ((adj)->GSEAL(upper))
#endif

#if !GTK_CHECK_VERSION(2, 18, 0)
#define gtk_widget_set_can_default(w, f) G_STMT_START{ if (f) GTK_WIDGET_SET_FLAGS(w, GTK_CAN_DEFAULT); else GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_DEFAULT); }G_STMT_END
#define gtk_widget_set_can_focus(w, f) G_STMT_START{ if (f) GTK_WIDGET_SET_FLAGS(w, GTK_CAN_FOCUS); else GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_FOCUS); }G_STMT_END
#define gtk_widget_is_sensitive(w) GTK_WIDGET_IS_SENSITIVE(w)
#endif

#if !GTK_CHECK_VERSION(2, 20, 0)
#define gtk_widget_get_realized(w) (GTK_WIDGET_REALIZED(w))
#endif

#if !GTK_CHECK_VERSION(2, 18, 0)
#define gtk_widget_set_window(widget, gdkwindow) ((widget)->GSEAL(window) = gdkwindow)
#define gtk_widget_set_has_window(widget, f)
#define gtk_file_chooser_set_create_folders(selector, enable)
#endif

#if GTK_CHECK_VERSION(2, 90, 6)
#include "gtktooltips.h"
#include "gtkoptionmenu.h"

#define gtk_menu_shell_get_children(shell) ((shell)->GSEAL(children))
#define gtk_menu_shell_get_active(shell) ((shell)->GSEAL(active))
#define gtk_menu_get_toplevel(menu) ((menu)->GSEAL(toplevel))
#define gtk_entry_set_position(entry, pos) gtk_editable_set_position(GTK_EDITABLE(entry), pos)

#define gtk_object_set_data(obj, key, d) g_object_set_data(G_OBJECT(obj), key, d)
#define gtk_object_get_data(obj, key) g_object_get_data(G_OBJECT(obj), key)

#define gtk_object_set_user_data(obj, data) g_object_set_data(G_OBJECT(obj), "user_data", data)
#define gtk_object_get_user_data(obj) g_object_get_data(G_OBJECT(obj), "user_data")

#define gtk_menu_append(menu,child)  gtk_menu_shell_append((GtkMenuShell *)(menu), (child))

#define gtk_idle_add(function, data) g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, function, data, NULL)
#define gtk_idle_remove(tag) g_source_remove(tag)
#define gtk_timeout_add(interval, function, data) g_timeout_add_full(0, interval, function, data, NULL)
#define gtk_timeout_remove(tag) g_source_remove(tag)

#define gtk_object_unref(obj) g_object_unref(G_OBJECT(obj))
#define gtk_object_ref(obj) (g_object_ref(G_OBJECT(obj)))
void gtk_object_sink(GtkObject *object);

typedef struct _GtkNotebookPage   GtkNotebookPage;

void gtk_drawing_area_size(GtkDrawingArea *darea, gint width, gint height);
void gtk_widget_set_usize(GtkWidget *widget, gint width, gint height);

void gtk_exit(gint errorcode);

#endif

#if !GTK_CHECK_VERSION(2, 90, 7)
# define gtk_get_major_version() gtk_major_version
# define gtk_get_minor_version() gtk_minor_version
# define gtk_get_micro_version() gtk_micro_version
#endif


#if GTK_CHECK_VERSION(2, 90, 6)

gulong	gtk_signal_connect_full			(GtkObject	    *object,
						 const char	    *name,
						 GCallback	     func,
						 GtkCallbackMarshal  unsupported,
						 gpointer	     data,
						 GDestroyNotify      destroy_func,
						 gint		     object_signal,
						 gint		     after);

typedef void (*GtkSignalFunc)       (void);

#endif

#ifndef GTK_SIGNAL_FUNC
#define GTK_SIGNAL_FUNC(f)     G_CALLBACK(f)
#endif
#ifndef gtk_signal_connect
#define	gtk_signal_connect(object,name,func,func_data)                                 \
   gtk_signal_connect_full ((object), (name), (func), NULL, (func_data), NULL, 0, 0)
#endif
#ifndef gtk_signal_connect_object
#define	gtk_signal_connect_object(object,name,func,slot_object)                        \
   gtk_signal_connect_full ((object), (name), (func), NULL, (slot_object), NULL, 1, 0)
#endif

#ifndef gtk_notebook_set_page
#define gtk_notebook_set_page(notebook, page) gtk_notebook_set_current_page(notebook, page)
#endif


#define GDKPIXBUF_VERSION_ENCODE(major, minor, micro) \
	((major) * 10000 + (minor) + 100 + (micro))
#define GDKPIXBUF_CHECK_VERSION(major, minor, micro) \
	(GDKPIXBUF_VERSION_ENCODE(GDK_PIXBUF_MAJOR, GDK_PIXBUF_MINOR, GDK_PIXBUF_MICRO) >= GDKPIXBUF_VERSION_ENCODE(major, minor, micro))

#if GDKPIXBUF_CHECK_VERSION(2, 24, 0)
#define gdk_pixbuf_unref(p) g_object_unref(p)
#endif
