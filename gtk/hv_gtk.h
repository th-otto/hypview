#ifndef __HV_GTK_H__
#define __HV_GTK_H__

#include "hypdoc.h"
#include <gtk/gtk.h>
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

typedef long WP_UNIT;

enum toolbutton {
	TO_BACK,
	TO_HISTORY,
	TO_MEMORY,
	TO_FIRST,
	TO_PREV_PHYS,
	TO_PREV,
	TO_HOME,
	TO_NEXT,
	TO_NEXT_PHYS,
	TO_LAST,
	TO_INDEX,
	TO_KATALOG,
	TO_REFERENCES,
	TO_HELP,
	TO_INFO,
	TO_LOAD,
	TO_SAVE,
	TO_MAX
};

enum blockop {
	CO_SAVE,
	CO_BACK,
	CO_COPY,
	CO_PASTE,
	CO_SELECT_ALL,
	CO_SEARCH,
	CO_SEARCH_AGAIN,
	CO_DELETE_STACK,
	CO_SWITCH_FONT,
	CO_SELECT_FONT,
	CO_REMARKER,
	CO_PRINT
};


struct _window_data_
{
	GRECT last;
	char *title;						/* Window title, in utf8 encoding */
	int x_raster;
	int y_raster;
	DOCUMENT *data;

	GtkActionGroup *action_group;
	GtkTextMark *curlink_mark;			/* link currently selected with <tab> */

	GtkWidget *hwnd;					/* GtkWindow */
	GtkWidget *history_menu;			/* GtkMenu */
	GtkWidget *bookmarks_menu;			/* GtkMenu */
	GtkTextBuffer *text_buffer;
	GtkWidget *text_window;				/* GtkScrolledWindow */
	GtkWidget *text_view;				/* GtkTextView */
	GtkWidget *toolbar;					/* GtkToolbar */
	GtkWidget *m_buttons[TO_MAX];		/* GtkToolItem */
	GtkWidget *searchbox;				/* GtkHBox */
	GtkWidget *searchentry;				/* GtkEntry */
	GtkWidget *strnotfound;				/* GtkLabel */
	gboolean hovering_over_link;
	WINDOW_DATA *popup;
	HISTORY *history;
	HYP_NODE *displayed_node;           /* Currently displayed node */
	GSList *image_childs;               /* list of child widget to implement graphic commands */
};

typedef struct _link_info {
	unsigned char link_type;
	hyp_indextype dst_type;
	char *tip;
	hyp_nodenr dest_page;
	hyp_lineno line_nr;
} LINK_INFO;


/* hv_main.c */

void check_toplevels(GtkWidget *toplevel);
GdkPixbuf *app_icon(void);
void check_desktop_file(char **filename);


/* about.c */

void About(GtkWidget *parent);


/*
 * help.c
 */
gboolean Help_Show(GtkWidget *parent, const char *entry);
void Help_Contents(GtkWidget *parent);
void Help_Index(GtkWidget *parent);
void Help_Using_Help(GtkWidget *parent);
void Help_Exit(void);
void Help_Init(void);


/*
 * hv_error.c
 */
void check_console(void);
void write_console(const char *s, gboolean use_gui, gboolean to_stderr, gboolean big);
gboolean init_gtk(void);
int toplevels_open_except(GtkWidget *top);
void check_toplevels(GtkWidget *toplevel);
void show_message(const char *title, const char *text, gboolean big);
gboolean ask_yesno(GtkWindow *parent, const char *text);
GtkWindow *top_window(void);


/*
 * hv_gfx.c
 */
_WORD GetNumColors(void);
_WORD GetNumPlanes(void);
void gdk_pixbuf_make_transparent(GdkPixbuf *pixbuf);
void gdk_pixbuf_make_opaque(GdkPixbuf *pixbuf);
cairo_surface_t *convert_pixbuf_to_cairo(GdkPixbuf *pixbuf);


/*
 * hv_init.c
 */
void hv_init(void);
void hv_exit(void);


/*
 * hv_font.c
 */
void SwitchFont(WINDOW_DATA *win);
void SelectFont(WINDOW_DATA *win);


/*
 * hv_win.c
 */
extern GSList *all_list;
extern GdkColor gdk_colors[16];

void hv_win_set_geometry(const char *geometry);
void hv_win_open(WINDOW_DATA *win);
WINDOW_DATA *hv_win_new(DOCUMENT *doc, gboolean popup);
void ReInitWindow(WINDOW_DATA *win, gboolean prep);
void hv_set_title(WINDOW_DATA *win, const char *wintitle);
void SendRedraw(WINDOW_DATA *win);
void SendCloseWindow(WINDOW_DATA *win);
void SendClose(GtkWidget *w);
long hv_win_topline(WINDOW_DATA *win);
void hv_win_scroll_to_line(WINDOW_DATA *win, long line);
GtkTextTag *gtk_text_table_create_tag(GtkTextTagTable *table, const gchar *tag_name, const gchar *first_property_name, ...);
void hv_win_destroy_images(WINDOW_DATA *win);


/*
 *	hv_click.c
 */
void HypClick(WINDOW_DATA *win, LINK_INFO *info);



/*
 * hv_tbar.c
 */
struct popup_pos {
	WINDOW_DATA *window;
	enum toolbutton obj;
};
void ToolbarUpdate(WINDOW_DATA *win, gboolean redraw);
void ToolbarClick(WINDOW_DATA *win, enum toolbutton obj, int button, guint32 event_time);
void RemoveSearchBox(WINDOW_DATA *win);
void position_popup(GtkMenu *menu, gint *xret, gint *yret, gboolean *push_in, void *data);


/*
 * hv_hist.c
 */
void AddHistoryEntry(WINDOW_DATA *win, DOCUMENT *doc);
DOCUMENT *RemoveHistoryEntry(WINDOW_DATA *win, hyp_nodenr *node, long *line);
void RemoveAllHistoryEntries(WINDOW_DATA *win);


/*
 * hv_autol.c
 */
gboolean AutolocatorKey(WINDOW_DATA *win, GdkEventKey *event);
void AutoLocatorPaste(WINDOW_DATA *win);


/*
 * hv_fsel.c
 */
WINDOW_DATA *SelectFileLoad(WINDOW_DATA *win);
void SelectFileSave(WINDOW_DATA *win);


/*
 * hv_file.c
 */
WINDOW_DATA *OpenFileInWindow(WINDOW_DATA *win, const char *path, const char *chapter, hyp_nodenr node, gboolean find_default, int new_window, gboolean no_message);


/*
 * hv_hfind.c
 */
void Hypfind(WINDOW_DATA *win, gboolean again);


/*
 * hv_mark.c
 */
void MarkerSave(WINDOW_DATA *win, short num);
void MarkerShow(WINDOW_DATA *win, short num, gboolean new_window);
void MarkerPopup(WINDOW_DATA *win, int button, guint32 event_time);
void MarkerUpdate(WINDOW_DATA *win);
void MarkerSaveToDisk(void);
void MarkerInit(void);
void on_bookmark_selected(GtkAction *action, WINDOW_DATA *win);


/*
 * hv_nav.c
 */
void GotoPage(WINDOW_DATA *win, hyp_nodenr num, long line, gboolean calc);
void GoBack(WINDOW_DATA *win);
void HistoryPopup(WINDOW_DATA *win, int button, guint32 event_time);
void GotoHelp(WINDOW_DATA *win);
void GotoIndex(WINDOW_DATA *win);
void GoThisButton(WINDOW_DATA *win, enum toolbutton obj);
void GotoCatalog(WINDOW_DATA *win);
void GotoDefaultFile(WINDOW_DATA *win);


/*
 * hv_misc.c
 */
gboolean show_dialog(GtkWidget *parent, const char *type, const char *message, gboolean can_cancel);
void CenterWindow(GtkWidget *hwnd);
int gtk_XParseGeometry(const char *string, int *x, int *y, int *width, int *height);
void hv_recent_add(const char *path);


/*
 * hv_eref.c
 */
void HypExtRefPopup(WINDOW_DATA *win, int button, guint32 event_time);
void HypOpenExtRef(WINDOW_DATA *win, const char *name, gboolean new_window);


/*
 * hv_popup.c
 */
void OpenPopup(WINDOW_DATA *win, hyp_nodenr num, int x, int y);


/*
 * hv_block.c
 */
void BlockOperation(WINDOW_DATA *win, enum blockop num);
void BlockSelectAll(WINDOW_DATA *win, BLOCK *b);
void BlockCopy(WINDOW_DATA *win);
void BlockPaste(WINDOW_DATA *win, gboolean new_window);
void BlockAsciiSave(WINDOW_DATA *win, const char *path, GtkTextIter *start, GtkTextIter *end);
void StartRemarker(gboolean quiet);


/*
 * hv_info.c
 */
void DocumentInfos(WINDOW_DATA *win);


#endif /* __HV_GTK_H__ */
