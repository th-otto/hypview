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
	TO_BOOKMARKS,
	TO_FIRST,
	TO_PREV_PHYS,
	TO_PREV,
	TO_HOME,
	TO_NEXT,
	TO_NEXT_PHYS,
	TO_LAST,
	TO_INDEX,
	TO_CATALOG,
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

struct _viewer_colors {
	COLORREF background;       /* window background color */
	COLORREF text;             /* Displays text in the specified color */
	COLORREF link;             /* Displays references in the specified color */
	COLORREF popup;            /* Displays references to popups in the specified color */
	COLORREF xref;             /* Displays external references in the specified color */
	COLORREF system;           /* Displays references to {@ system } in the specified color */
	COLORREF rx;               /* Displays references to {@ rx } in the specified color */
	COLORREF rxs;              /* Displays references to {@ rxs } in the specified color */
	COLORREF quit;             /* Displays references to {@ quit } in the specified color */
	COLORREF close;            /* Displays references to {@ close } in the specified color */
	COLORREF error;            /* used to display invalid links in hypertext files */
};

typedef struct
{
	int config_id;			/* unique id */
	int menu_id;			/* corresponding menu id */
	int icon_id;			/* resource id of icon */
	const char *comment;	/* optional tooltip text */
} TOOLBAR_ENTRY;

#define TB_ENDMARK (-1)
#define TB_SEPARATOR (-2)

typedef struct _tool_data TOOL_DATA;

struct _tool_data {
	HWND hwnd;
	int		buttonsave;			/* id of last pressed button*/
	int		num_entries;		/* number of entries */
	int		num_definitions;	/* number of definititions */
	const TOOLBAR_ENTRY *entries;		/* all definitions */
	const int *definitions;		/* currently displayed entries */
	int		buttondown;			/* number of currently pressed button */
	int		buttonxs;			/* X-coordinate of last pressed buttpn */
	int     buttonys;			/* Y-coordinate of last pressed buttpn */
	gboolean buttonpress;		/* TRUE if mousebutton pressed in toolbar */
	gboolean visible;			/* TRUE if toolbar should be displayed */
	gboolean ontop;				/* TRUE if toolbar should be display at top */
	char    *help_text;			/* currently display tooltip text */
	HWND    toolbar_help_hwnd;
	
	void	(*toolbar_paint)(HDC hdc, WINDOW_DATA *win, const GRECT *gr);
	int		(*toolbar_size)(TOOL_DATA *td, GRECT *r);
	gboolean (*toolbar_mouse_move)(TOOL_DATA *td, const GRECT *gr, int mousex, int mousey);
	void	(*toolbar_mouse_down)(TOOL_DATA *td, gboolean buttondown, const GRECT *gr, int mousex, int mousey);
	void	(*toolbar_button_up)(TOOL_DATA *td);
	void	(*toolbar_exit)(WINDOW_DATA *win);
	
	void    (*toolbar_close)(WINDOW_DATA *win);
	void	(*toolbar_open)(WINDOW_DATA *win);

	void	(*toolbar_help_settext)(TOOL_DATA *td, const char *text, int x, int y);
	void	(*toolbar_refresh)(TOOL_DATA *td, const GRECT *gr);
	void	(*toolbar_drawicon)(HDC hdc, WINDOW_DATA *win, const GRECT *button, int entry_idx, gboolean selected);
};

typedef struct _status_data STATUS_DATA;

#define STATUS_MAX_LEN 160

typedef struct
{
	int x;
	int y;
	int w;
	int flags;
	char txt[STATUS_MAX_LEN];
} STATUS_PARTS;

struct _status_data {
	HWND hwnd;
	STATUS_PARTS *StatusParts;	/* Koordinaten der Felder                         */
	int     count;				/* Anzahl Felder in der Statuszeile               */
	gboolean ontop;				/* TRUE wenn Statusbar oben angezeigt werden soll */
	gboolean visible;			/* TRUE wenn Statusbar angezeigt werden soll      */
	int     text_width;
	int     text_height;
	
	int     (*statusbar_size)(STATUS_DATA *sd, GRECT *r);
	int     (*statusbar_paint)(HDC hdc, STATUS_DATA *sd, GRECT *gr);
	void    (*statusbar_textout)(HDC hdc, STATUS_DATA *sd, const char *text, int fieldno, gboolean refresh);
	void    (*statusbar_exit)(WINDOW_DATA *win);
	
	void    (*statusbar_open)(WINDOW_DATA *win);
	void	(*statusbar_close)(WINDOW_DATA *win);

	void	(*statusbar_refresh)(HDC hdc, STATUS_DATA *sd, GRECT *gr);
};

struct _window_data_
{
	HWND hwnd;
	GRECT last;
	char *title;						/* Window title, in utf8 encoding */
	int x_raster;
	int y_raster;
	int x_margin_left, x_margin_right;
	int y_margin_top, y_margin_bottom;
	GRECT scroll;
	DOCUMENT *data;
	gboolean is_popup;
	
	WINDOW_DATA *popup;
	HISTORY *history;
	HYP_NODE *displayed_node;           /* Currently displayed node */
	
	HMENU bookmarks_menu;
	DWORD m_buttons[TO_MAX];
	HWND textwin;
	HDC draw_hdc;
	HRGN cliprgn;
	HFONT fonts[HYP_TXT_MASK + 1];
	HWND searchbox;
	HWND searchentry;
	struct {
		WP_UNIT x;
		WP_UNIT y;
		WP_UNIT w;
		WP_UNIT h;
	} docsize;
	WP_UNIT scrollhpos, scrollhsize;
	WP_UNIT scrollvpos, scrollvsize;
	BLOCK selection;
	gboolean hovering_over_link;
	WINDOW_DATA *parentwin;
	TOOL_DATA *td;
	STATUS_DATA *sd;
};

typedef struct _link_info {
	unsigned char link_type;
	hyp_indextype dst_type;
	char *tip;
	hyp_nodenr dest_page;
	hyp_lineno line_nr;
} LINK_INFO;


#define RectToGrect(gr, re) \
	((gr)->g_x = (re)->left, \
	 (gr)->g_y = (re)->top, \
	 (gr)->g_w = (re)->right - (re)->left, \
	 (gr)->g_h = (re)->bottom - (re)->top)
#define GrectToRect(re, gr) \
	((re)->left = (gr)->g_x, \
	 (re)->top = (gr)->g_y, \
	 (re)->right = (gr)->g_x + (gr)->g_w, \
	 (re)->bottom = (gr)->g_y + (gr)->g_h)


#define NO_WINDOW ((HWND)0)


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
int toplevels_open_except(WINDOW_DATA *top);
void check_toplevels(WINDOW_DATA *toplevel);
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
extern struct _viewer_colors viewer_colors;

void hv_init(void);
void hv_exit(void);


/*
 * hv_font.c
 */
#define FONT_NAME_LEN 256

void SwitchFont(WINDOW_DATA *win);
void SelectFont(WINDOW_DATA *win);
void hv_update_menu(HMENU menu, DOCUMENT *doc);
void hv_update_winmenu(WINDOW_DATA *win);
void hv_update_menus(void);
void W_FontCreate(const char *name, HFONT *fonts);


/*
 * hv_selec.c
 */
void SelectAll(WINDOW_DATA *win);
void RemoveSelection(WINDOW_DATA *win);
void DrawSelection(WINDOW_DATA *win);
void MouseSelection(WINDOW_DATA *win, int x, int y, gboolean extend);


/*
 * hv_win.c
 */
extern UINT commdlg_help;

void hv_win_set_geometry(const char *geometry);
void hv_win_open(WINDOW_DATA *win);
WINDOW_DATA *win32_hypview_window_new(DOCUMENT *doc, gboolean popup);
void ReInitWindow(WINDOW_DATA *win, gboolean prep);
void hv_set_title(WINDOW_DATA *win, const char *wintitle);
void hv_set_font(WINDOW_DATA *win);
void SendRedraw(WINDOW_DATA *win);
void SendCloseWindow(WINDOW_DATA *win);
void SendClose(HWND w);
long hv_win_topline(WINDOW_DATA *win);
void hv_win_scroll_to_line(WINDOW_DATA *win, long line);
void hv_win_update_attributes(WINDOW_DATA *win);
void hv_win_reset_text(WINDOW_DATA *win);
void WindowCalcScroll(WINDOW_DATA *win);
void SetWindowSlider(WINDOW_DATA *win);
gboolean hv_scroll_window(WINDOW_DATA *win, long xamount, long yamount);


/*
 *	hv_click.c
 */
void HypClick(WINDOW_DATA *win, LINK_INFO *info);
gboolean HypFindLink(WINDOW_DATA *win, int x, int y, LINK_INFO *info, gboolean select_word);



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
void CheckMenuObj(HMENU menu, int obj, gboolean check);
void toolbar_register_classes(HINSTANCE hinst);
TOOL_DATA *toolbar_init(WINDOW_DATA *win, const int *definitions, int num_definitions, const TOOLBAR_ENTRY *entries, int num_entries);
void toolbar_setfont(const char *desc);


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
void RecentExit(void);
void RecentSaveToDisk(void);
int gtk_XParseGeometry(const char *string, int *x, int *y, int *width, int *height);
void g_slist_free_full(GSList *list, void (*freefunc)(void *));
void g_slist_free(GSList *list);
void DlgSetText(HWND hwnd, int id, const char *str);
char *DlgGetText(HWND hwnd, int id);
wchar_t *DlgGetTextW(HWND hwnd, int id);
gboolean DlgGetButton(HWND hwnd, int id);
void DlgSetButton(HWND hwnd, int id, gboolean check);
void DlgEnable(HWND hwnd, int id, gboolean enable);

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
typedef enum { remarker_top, remarker_startup, remarker_check, remarker_update } remarker_mode;
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
