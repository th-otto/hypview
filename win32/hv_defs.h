#ifndef __HV_DEFS_H__
#define __HV_DEFS_H__

#include "hypdoc.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
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
	TO_REMARKER,
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
	HWND hwnd;
	GRECT last;
	char *title;						/* Window title, in utf8 encoding */
	int x_raster;
	int y_raster;
	DOCUMENT *data;
	gboolean is_popup;
	
	WINDOW_DATA *popup;
	HISTORY *history;
	HYP_NODE *displayed_node;           /* Currently displayed node */
	
	HMENU bookmarks_menu;
	HWND m_buttons[TO_MAX];
	HWND toolbar;
	HWND searchbox;
	HWND searchentry;
	GSList *image_childs;
	struct {
		WP_UNIT h;
	} docsize;
	gboolean hovering_over_link;
	WINDOW_DATA *parentwin;
};

typedef struct _link_info {
	unsigned char link_type;
	hyp_indextype dst_type;
	char *tip;
	hyp_nodenr dest_page;
	hyp_lineno line_nr;
} LINK_INFO;


/* about.c */

void About(HWND parent);


/*
 * help.c
 */
gboolean Help_Show(WINDOW_DATA *parent, const char *entry);
void Help_Contents(WINDOW_DATA *parent);
void Help_Index(WINDOW_DATA *parent);
void Help_Using_Help(WINDOW_DATA *parent);
void Help_Exit(void);
void Help_Init(void);


/*
 * hv_error.c
 */
extern gboolean defer_messages;
extern GSList *all_list;

void check_console(void);
void write_console(const char *s, gboolean use_gui, gboolean to_stderr, gboolean big);
int toplevels_open_except(HWND top);
void check_toplevels(HWND toplevel);
void show_message(HWND parent, const char *title, const char *text, gboolean big);
gboolean ask_yesno(HWND parent, const char *text);
HWND top_window(void);


/*
 * hv_gfx.c
 */
_WORD GetNumColors(void);
_WORD GetNumPlanes(void);


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
void hv_update_menu(WINDOW_DATA *win);
void hv_update_menus(void);


/*
 * hv_win.c
 */
void hv_win_set_geometry(const char *geometry);
void hv_win_open(WINDOW_DATA *win);
WINDOW_DATA *gtk_hypview_window_new(DOCUMENT *doc, gboolean popup);
void ReInitWindow(WINDOW_DATA *win, gboolean prep);
void hv_set_title(WINDOW_DATA *win, const char *wintitle);
void SendRedraw(WINDOW_DATA *win);
void SendCloseWindow(WINDOW_DATA *win);
void SendClose(HWND w);
long hv_win_topline(WINDOW_DATA *win);
void hv_win_scroll_to_line(WINDOW_DATA *win, long line);
void hv_win_destroy_images(WINDOW_DATA *win);
void hv_win_update_attributes(WINDOW_DATA *win);
void hv_win_reset_text(WINDOW_DATA *win);


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
void ToolbarClick(WINDOW_DATA *win, enum toolbutton obj, int button);
void RemoveSearchBox(WINDOW_DATA *win);
void position_popup(HMENU menu, struct popup_pos *pos, int *xret, int *yret);
void EnableMenuObj(HMENU menu, int obj, gboolean enable);
void CheckMenuObj(WINDOW_DATA *win, int obj, gboolean check);


/*
 * hv_hist.c
 */
void AddHistoryEntry(WINDOW_DATA *win, DOCUMENT *doc);
DOCUMENT *RemoveHistoryEntry(WINDOW_DATA *win, hyp_nodenr *node, long *line);
void RemoveAllHistoryEntries(WINDOW_DATA *win);


/*
 * hv_autol.c
 */
gboolean AutolocatorKey(WINDOW_DATA *win, WPARAM wparam, LPARAM lparam);
void AutoLocatorPaste(WINDOW_DATA *win);


/*
 * hv_fsel.c
 */
WINDOW_DATA *SelectFileLoad(WINDOW_DATA *win);
void SelectFileSave(WINDOW_DATA *win);


/*
 * hv_file.c
 */
#define FORCE_NEW_WINDOW 2

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
void MarkerPopup(WINDOW_DATA *win, int button);
void MarkerUpdate(WINDOW_DATA *win);
void MarkerSaveToDisk(void);
void MarkerInit(void);
void on_bookmark_selected(WINDOW_DATA *win, int num);


/*
 * hv_nav.c
 */
void GotoPage(WINDOW_DATA *win, hyp_nodenr num, long line, gboolean calc);
void GoBack(WINDOW_DATA *win);
void HistoryPopup(WINDOW_DATA *win, int button);
void GotoHelp(WINDOW_DATA *win);
void GotoIndex(WINDOW_DATA *win);
void GoThisButton(WINDOW_DATA *win, enum toolbutton obj);
void GotoCatalog(WINDOW_DATA *win);
void GotoDefaultFile(WINDOW_DATA *win);


/*
 * hv_misc.c
 */
void CenterWindow(HWND hwnd);
void hv_recent_add(const char *path);
void RecentUpdate(WINDOW_DATA *win);
void on_recent_selected(WINDOW_DATA *win, int num);
void RecentInit(void);
void RecentSaveToDisk(void);
int gtk_XParseGeometry(const char *string, int *x, int *y, int *width, int *height);
void g_slist_free_full(GSList *list, void (*freefunc)(void *));
void g_slist_free(GSList *list);
void DlgSetText(HWND hwnd, int id, const char *str);
char *DlgGetText(HWND hwnd, int id);
wchar_t *DlgGetTextW(HWND hwnd, int id);
BOOL DlgGetButton(HWND hwnd, int id);
void DlgSetButton(HWND hwnd, int id, BOOL check);
void DlgEnable(HWND hwnd, int id, BOOL enable);

HMENU WINAPI LoadMenuExW(HINSTANCE instance, LPCWSTR name);
INT_PTR WINAPI DialogBoxExW(HINSTANCE hInstance, LPCWSTR name, HWND owner, DLGPROC dlgProc, LPARAM param);


/*
 * hv_eref.c
 */
void HypExtRefPopup(WINDOW_DATA *win, int button);
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
void BlockAsciiSave(WINDOW_DATA *win, const char *path);
typedef enum { remarker_top, remarker_startup, remarker_check } remarker_mode;
int StartRemarker(WINDOW_DATA *win, remarker_mode mode, gboolean quiet);


/*
 * hv_info.c
 */
void DocumentInfos(WINDOW_DATA *win);


/*
 * hv_prefs.c
 */
void hv_config_colors(WINDOW_DATA *win);
void hv_preferences(WINDOW_DATA *win);


#endif /* __HV_DEFS_H__ */
