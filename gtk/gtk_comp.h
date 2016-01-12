#ifndef glib_get_major_version
#define glib_get_major_version() glib_major_version
#define glib_get_minor_version() glib_minor_version
#define glib_get_micro_version() glib_micro_version
#endif

#if GLIB_CHECK_VERSION(2, 32, 0)

/*
typedef struct
{
	GMutex *mutex;
} GStaticMutex;

typedef enum
{
  G_THREAD_PRIORITY_LOW,
  G_THREAD_PRIORITY_NORMAL,
  G_THREAD_PRIORITY_HIGH,
  G_THREAD_PRIORITY_URGENT
} GThreadPriority;
*/

#ifndef G_STATIC_MUTEX_INIT
#define G_STATIC_MUTEX_INIT { NULL }
#endif

#define g_static_mutex_init(sm) g_static_mutex_get_mutex (sm)

#ifndef g_static_mutex_lock
#define g_static_mutex_lock(mutex) \
    g_mutex_lock (g_static_mutex_get_mutex (mutex))
#endif
#ifndef g_static_mutex_trylock
#define g_static_mutex_trylock(mutex) \
    g_mutex_trylock (g_static_mutex_get_mutex (mutex))
#endif
#ifndef g_static_mutex_unlock
#define g_static_mutex_unlock(mutex) \
    g_mutex_unlock (g_static_mutex_get_mutex (mutex))
#endif

GThread *g_thread_create_detached_not_implemented(void);
GThread *g_thread_create_with_stacksize_not_implemented(void);
GThread *g_thread_create_with_priority_not_implemented(void);

#define g_thread_create_full(func, data, stacksize, joinable, bound, prio, error) \
	(!(joinable) ? g_thread_create_detached_not_implemented() : \
	 stacksize != 0 ? g_thread_create_with_stacksize_not_implemented() : \
	 (prio) != G_THREAD_PRIORITY_NORMAL ? g_thread_create_with_priority_not_implemented() : \
		g_thread_try_new(NULL, func, data, error))

#define g_thread_create(func, data, joinable, error) \
  (g_thread_create_full (func, data, 0, joinable, FALSE, G_THREAD_PRIORITY_NORMAL, error))

#if GLIB_CHECK_VERSION(2, 34, 0)
static __inline__ GMutex *__g_mutex_new(void)
{
	GMutex *p = g_slice_new(GMutex);
	g_mutex_init(p);
	return p;
}
#define g_mutex_new __g_mutex_new

static __inline__ void __g_mutex_free(GMutex *p)
{
	if (p)
	{
		g_mutex_clear(p);
		g_slice_free(GMutex, p);
	}
}
#define g_mutex_free __g_mutex_free
#define g_static_mutex_free(m) g_mutex_clear(g_static_mutex_get_mutex(m))

static __inline__ GCond *__g_cond_new(void)
{
	GCond *p = g_slice_new (GCond);
	g_cond_init(p);
	return p;
}
#define g_cond_new __g_cond_new

static __inline__ void __g_cond_free(GCond *p)
{
	if (p)
	{
		g_cond_clear(p);
		g_slice_free (GCond, p);
	}
}
#define g_cond_free __g_cond_free

#endif

#else

#define g_thread_try_new(name, func, data, error) g_thread_create(func, data, TRUE, error)

#endif

#if GLIB_CHECK_VERSION(2, 29, 5)

#define g_atomic_int_exchange_and_add(p, v) g_atomic_int_add(p, v)

#endif

#if GLIB_CHECK_VERSION(2, 35, 3)
/* g_type_init() is deprecated in glib >= 2.35,3 */
#define g_type_init()
#endif

gboolean glib_ensure_version(void);

/*
 * for debugging
 */ 
void g_object_list_properties(const char *name, GObject *obj);

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
#define gtk_file_chooser_set_create_folders(selector, enable)
#endif

#if GTK_CHECK_VERSION(2, 90, 6)

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
#define gtk_object_sink(object) g_object_ref_sink(G_OBJECT(object))

typedef struct _GtkNotebookPage   GtkNotebookPage;

#endif

#if !GTK_CHECK_VERSION(2, 90, 7)
# define gtk_get_major_version() gtk_major_version
# define gtk_get_minor_version() gtk_minor_version
# define gtk_get_micro_version() gtk_micro_version
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
#define gtk_widget_ref(w) g_object_ref(G_OBJECT(w))
#define gtk_widget_unref(w) g_object_unref(G_OBJECT(w))
#endif

#if !GTK_CHECK_VERSION(3, 0, 0)
#define gdk_window_get_display(win) gdk_drawable_get_display(win)
#define gdk_window_get_screen(win) gdk_drawable_get_screen(win)
#endif


#if GTK_CHECK_VERSION(2, 90, 6)

typedef void (*GtkSignalFunc)       (void);

#endif

#ifndef GTK_SIGNAL_FUNC
#define GTK_SIGNAL_FUNC(f)     G_CALLBACK(f)
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
struct _GdkEventClient
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  GdkAtom message_type;
  gushort data_format;
  union {
    char b[20];
    short s[10];
    long l[5];
  } data;
};
typedef struct _GdkEventClient	    GdkEventClient;
#endif

#ifndef gtk_notebook_set_page
#define gtk_notebook_set_page(notebook, page) gtk_notebook_set_current_page(notebook, page)
#endif

#if GTK_CHECK_VERSION(3, 2, 0)
static inline GtkWidget *_gtk_hbox_new(gboolean homogeneous, gint spacing)
{
	GtkWidget *w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
	gtk_box_set_homogeneous(GTK_BOX(w), homogeneous);
	return w;
}
static inline GtkWidget *_gtk_vbox_new(gboolean homogeneous, gint spacing)
{
	GtkWidget *w = gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing);
	gtk_box_set_homogeneous(GTK_BOX(w), homogeneous);
	return w;
}
#define gtk_hbox_new _gtk_hbox_new
#define gtk_vbox_new _gtk_vbox_new
#endif

#define GDKPIXBUF_VERSION_ENCODE(major, minor, micro) \
	((major) * 10000 + (minor) + 100 + (micro))
#define GDKPIXBUF_CHECK_VERSION(major, minor, micro) \
	(GDKPIXBUF_VERSION_ENCODE(GDK_PIXBUF_MAJOR, GDK_PIXBUF_MINOR, GDK_PIXBUF_MICRO) >= GDKPIXBUF_VERSION_ENCODE(major, minor, micro))

#if GDKPIXBUF_CHECK_VERSION(2, 24, 0)
#define gdk_pixbuf_unref(p) g_object_unref(p)
#endif

#if GTK_CHECK_VERSION(2, 6, 0)

#define gtk_button_new_with_image_and_label(stock, text) \
	_gtk_button_new_with_image_and_label(stock, text)

GtkWidget *_gtk_button_new_with_image_and_label(const char *stock, const gchar *label);

#if GTK_CHECK_VERSION(3, 0, 0)

#define GTK3_DOMAIN "gtk30"

#define gtk_button_new_close()  gtk_button_new_with_image_and_label("gtk-close", g_dgettext(GTK3_DOMAIN, "_Close"))
#define gtk_button_new_clear()  gtk_button_new_with_image_and_label("gtk-clear", g_dgettext(GTK3_DOMAIN, "_Clear"))
#define gtk_button_new_ok()     gtk_button_new_with_image_and_label("gtk-ok", g_dgettext(GTK3_DOMAIN, "_OK"))
#define gtk_button_new_cancel() gtk_button_new_with_image_and_label("gtk-cancel", g_dgettext(GTK3_DOMAIN, "_Cancel"))
#define gtk_button_new_delete() gtk_button_new_with_image_and_label("gtk-delete", g_dgettext(GTK3_DOMAIN, "_Delete"))
#define gtk_button_new_apply()  gtk_button_new_with_image_and_label("gtk-apply", g_dgettext(GTK3_DOMAIN, "_Apply"))
#define gtk_button_new_open()   gtk_button_new_with_image_and_label("gtk-open", g_dgettext(GTK3_DOMAIN, "_Open"))
#define gtk_button_new_save()   gtk_button_new_with_image_and_label("gtk-save", g_dgettext(GTK3_DOMAIN, "_Save"))
#define gtk_button_new_jump()   gtk_button_new_with_image_and_label("gtk-jump-to", g_dgettext(GTK3_DOMAIN, "_Jump to"))
#define gtk_button_new_about()  gtk_button_new_with_image_and_label("gtk-about", g_dgettext(GTK3_DOMAIN, "_About"))
#define gtk_button_new_yes()    gtk_button_new_with_image_and_label("gtk-yes", g_dgettext(GTK3_DOMAIN, "_Yes"))
#define gtk_button_new_no()     gtk_button_new_with_image_and_label("gtk-no", g_dgettext(GTK3_DOMAIN, "_No"))
#define gtk_button_new_add()    gtk_button_new_with_image_and_label("gtk-add", g_dgettext(GTK3_DOMAIN, "_Add"))
#define gtk_button_new_help()   gtk_button_new_with_image_and_label("gtk-help", g_dgettext(GTK3_DOMAIN, "_Help"))

#else

#define gtk_button_new_close()  gtk_button_new_from_stock(GTK_STOCK_CLOSE)
#define gtk_button_new_clear()  gtk_button_new_from_stock(GTK_STOCK_CLEAR)
#define gtk_button_new_ok()     gtk_button_new_from_stock(GTK_STOCK_OK)
#define gtk_button_new_cancel() gtk_button_new_from_stock(GTK_STOCK_CANCEL)
#define gtk_button_new_delete() gtk_button_new_from_stock(GTK_STOCK_DELETE)
#define gtk_button_new_apply()  gtk_button_new_from_stock(GTK_STOCK_APPLY)
#define gtk_button_new_open()   gtk_button_new_from_stock(GTK_STOCK_OPEN)
#define gtk_button_new_save()   gtk_button_new_from_stock(GTK_STOCK_SAVE)
#define gtk_button_new_jump()   gtk_button_new_from_stock(GTK_STOCK_JUMP_TO)
#define gtk_button_new_about()  gtk_button_new_from_stock(GTK_STOCK_ABOUT)
#define gtk_button_new_yes()    gtk_button_new_from_stock(GTK_STOCK_YES)
#define gtk_button_new_no()     gtk_button_new_from_stock(GTK_STOCK_NO)
#define gtk_button_new_add()    gtk_button_new_from_stock(GTK_STOCK_ADD)
#define gtk_button_new_help()   gtk_button_new_from_stock(GTK_STOCK_HELP)

#endif

#else

#define gtk_button_new_close()  gtk_button_new_with_label(_("Close"))
#define gtk_button_new_clear()  gtk_button_new_with_label(_("Clear"))
#define gtk_button_new_ok()     gtk_button_new_with_label(_("Ok"))
#define gtk_button_new_cancel() gtk_button_new_with_label(_("Cancel"))
#define gtk_button_new_delete() gtk_button_new_with_label(_("Delete"))
#define gtk_button_new_apply()  gtk_button_new_with_label(_("Apply"))
#define gtk_button_new_open()   gtk_button_new_with_label(_("Open"))
#define gtk_button_new_save()   gtk_button_new_with_label(_("Save"))
#define gtk_button_new_jump()   gtk_button_new_with_label(_("Jump to"))
#define gtk_button_new_about()  gtk_button_new_with_label(_("About"))
#define gtk_button_new_yes()    gtk_button_new_with_label(_("Yes"))
#define gtk_button_new_no()     gtk_button_new_with_label(_("No"))
#define gtk_button_new_add()    gtk_button_new_with_label(_("Add"))
#define gtk_button_new_help()   gtk_button_new_with_label(_("Help"))

#define gtk_button_new_with_image_and_label(stock, text) \
	gtk_button_new_with_mnemonic(text)

#endif

#ifndef gdk_window_get_size
#define gdk_window_get_size          gdk_drawable_get_size
#endif

#ifndef GDK_BUTTON_PRIMARY
#  define GDK_BUTTON_PRIMARY 1
#endif
#ifndef GDK_BUTTON_MIDDLE
#  define GDK_BUTTON_MIDDLE 2
#endif
#ifndef GDK_BUTTON_SECONDARY
#  define GDK_BUTTON_SECONDARY 3
#endif
