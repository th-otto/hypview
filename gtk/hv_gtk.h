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

typedef struct _window_data_ WINDOW_DATA;

struct _window_data_
{
	GRECT last;
	const char *title;	/* copy of doc->window_title; do not free */
	short	x_speed;
	short	y_speed;
	short	x_raster;
	short	y_raster;
	DOCUMENT *data;
	struct {
		WP_UNIT y;
	} docsize;

	GtkActionGroup *action_group;

	char *m_geometry;
	GtkWidget *hwnd;					/* GtkWindow */
	GtkWidget *history_menu;			/* GtkMenu */
	GtkTextBuffer *text_buffer;
	GtkWidget *text_window;				/* GtkScrolledWindow */
	GtkWidget *text_view;				/* GtkTextView */
	GtkWidget *toolbar;					/* GtkToolbar */
	GtkWidget *m_buttons[TO_MAX];		/* GtkToolItem */
	GtkWidget *searchbox;				/* GtkHBox */
	GtkWidget *searchentry;				/* GtkEntry */
	GtkWidget *strnotfound;				/* GtkLabel */
};


/* hv_main.c */

void check_toplevels(GtkWidget *toplevel);
GdkPixbuf *app_icon(void);
void check_desktop_file(char **filename);


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


/*
 * hv_init.c
 */
extern const char *sel_font_name;

void hv_init(void);
void hv_exit(void);


/*
 * hv_font.c
 */
void SwitchFont(DOCUMENT *doc);
void SelectFont(DOCUMENT *doc);


/*
 * hv_win.c
 */
extern GSList *all_list;

void hv_win_set_geometry(WINDOW_DATA *win, const char *geometry);
void hv_win_open(WINDOW_DATA *win);
WINDOW_DATA *hv_win_new(DOCUMENT *doc, gboolean popup);
void ReInitWindow(DOCUMENT *doc);
void hv_set_title(WINDOW_DATA *win, const char *wintitle);
void SendRedraw(WINDOW_DATA *win);
void SendCloseWindow(WINDOW_DATA *win);
void SendClose(GtkWidget *w);


/*
 * hv_tbar.c
 */
struct popup_pos {
	DOCUMENT *doc;
	enum toolbutton obj;
};
void ToolbarUpdate(DOCUMENT *doc, gboolean redraw);
void ToolbarClick(DOCUMENT *doc, enum toolbutton obj, int button, guint32 event_time);
void RemoveSearchBox(DOCUMENT *doc);
void position_popup(GtkMenu *menu, gint *xret, gint *yret, gboolean *push_in, void *data);


/*
 * hv_hist.c
 */
typedef struct _history_  HISTORY;
struct _history_
{
	HISTORY *next;               /* Pointer to next history entry */
	WINDOW_DATA *win;            /* Associated window */
	DOCUMENT *doc;               /* Pointer to document */
	long line;                   /* First visible line */
	hyp_nodenr node;             /* Document node (=chapter) number */
	char *title;                 /* history title */
};

extern HISTORY *history;         /* Pointer to history data */

void AddHistoryEntry(WINDOW_DATA *wind);
gboolean RemoveHistoryEntry(DOCUMENT **doc, hyp_nodenr *node, long *line);
void RemoveAllHistoryEntries(WINDOW_DATA *wind);
short CountWindowHistoryEntries(WINDOW_DATA *wind);
short CountDocumentHistoryEntries(DOCUMENT *doc);
void DeleteLastHistory(HISTORY *entry);
HISTORY *GetLastHistory(void);
void SetLastHistory(WINDOW_DATA *the_win, HISTORY *last);
void DeleteLastHistory(HISTORY *entry);


/*
 * hv_autol.c
 */
short AutolocatorKey(DOCUMENT *doc, GdkModifierType state, int ascii);
void AutoLocatorPaste(DOCUMENT *doc);


/*
 * hv_fsel.c
 */
WINDOW_DATA *SelectFileLoad(WINDOW_DATA *win);
void SelectFileSave(DOCUMENT *doc);


/*
 * hv_file.c
 */
WINDOW_DATA *OpenFileSameWindow(WINDOW_DATA *win, const char *path, const char *chapter, gboolean new_window, gboolean no_message);
WINDOW_DATA *OpenFileNewWindow(const char *path, const char *chapter, hyp_nodenr node, _BOOL find_default);


/*
 * hv_mark.c
 */
void MarkerSave(DOCUMENT *doc, short num);
void MarkerShow(DOCUMENT *doc, short num, gboolean new_window);
void MarkerPopup(DOCUMENT *doc, int button, guint32 event_time);
void MarkerSaveToDisk(void);
void MarkerInit(void);


/*
 * hv_nav.c
 */
void GotoPage(DOCUMENT *doc, hyp_nodenr num, long line, gboolean calc);
void GoBack(DOCUMENT *doc);
void HistoryPopup(DOCUMENT *doc, int button, guint32 event_time);
void GotoHelp(DOCUMENT *doc);
void GotoIndex(DOCUMENT *doc);
void GoThisButton(DOCUMENT *doc, enum toolbutton obj);
void GotoCatalog(WINDOW_DATA *win);


/*
 * hv_misc.c
 */
void show_dialog(GtkWidget *parent, const char *type, const char *message, void (*ok_fn)(GtkWidget *widget, gpointer user_data), gpointer user_data);
void CenterWindow(GtkWidget *hwnd);


/*
 * hv_eref.c
 */
void HypExtRefPopup(DOCUMENT *doc, int button, guint32 event_time);
void HypOpenExtRef(void *win, const char *name, gboolean new_window);


/*
 * hv_popup.c
 */
void OpenPopup(DOCUMENT *doc, hyp_nodenr num, int x, int y);


/*
 * hv_block.c
 */
void BlockOperation(DOCUMENT *doc, short num);
void BlockSelectAll(DOCUMENT *doc, BLOCK *b);
void BlockCopy(DOCUMENT *doc);
void BlockPaste(WINDOW_DATA *win, gboolean new_window);
void BlockAsciiSave(DOCUMENT *doc, const char *path);


/*
 * hv_info.c
 */
void ProgrammInfos(DOCUMENT *doc);


#endif /* __HV_GTK_H__ */
