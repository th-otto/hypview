#ifndef __HV_GTK_H__
#define __HV_GTK_H__

#include "hypdoc.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gstdio.h>
#ifdef G_OS_UNIX
#include <gio/gdesktopappinfo.h>
#endif
#include "gtk_comp.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif

#define HYPVIEW_VERSION_MAJOR "1"
#define HYPVIEW_VERSION_MINOR "0"
#define HYPVIEW_VERSION_MICRO "0"
#define HYPVIEW_VERSION HYPVIEW_VERSION_MAJOR "." HYPVIEW_VERSION_MINOR "." HYPVIEW_VERSION_MICRO

#if defined(__WIN32__) || defined(__TOS__)
#define filename_cmp strcasecmp
#else
#define filename_cmp strcmp
#endif

/*
 * filters for file chooser.
 * Filters are separated by newlines.
 * In each filter, the display string is separated from the extension list by '|'.
 * If a display string is not defined, the extension list is used.
 * An extension list may specify several wildcard specifications separated by spaces.
 */
#define IDS_SELECT_HYPERTEXT _("*.hyp|Hypertext files (*.hyp)\n*.*|All files (*.*)\n")


/* hv_main.c */

void check_toplevels(GtkWidget *toplevel);
GdkPixbuf *app_icon(void);
void check_desktop_file(char **filename);

/*
 * help.c
 */
gboolean Help_Show(GtkWidget *parent, const char *filename, const char *entry);
void Help_Contents(GtkWidget *parent, const char *filename);
void Help_Index(GtkWidget *parent, const char *filename);
void Help_Using_Help(GtkWidget *parent, const char *filename);
void Help_Exit(void);
void Help_Init(void);

/*
 * misc.c
 */
int choose_file(GtkWidget *parent, char **name, gboolean must_exist, const char *title, const char *filter);

void show_dialog(GtkWidget *parent, const char *type, const char *message, void (*ok_fn)(GtkWidget *widget, gpointer user_data), gpointer user_data);

void CenterWindow(GtkWidget *hwnd);


#endif /* __HV_GTK_H__ */
