#ifndef __HV_DEFS_H__
#define __HV_DEFS_H__

#include "hypdefs.h"
#include <Cocoa/Cocoa.h>

@class HypViewWindow;

/* handle of a window */
#define WINDOW_DATA HypViewWindow
#define NO_WINDOW ((WINDOW_DATA *)0)

#include "hypdoc.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif

@interface HypViewView : NSView
{
@private
	CGDirectDisplayID w_display;
	CGColorSpaceRef colorspace;
}
@end

@interface HypViewApplication : NSApplication <NSApplicationDelegate>
{
@private
	NSDate *				_distantFuture;
	NSDate *				_distantPast;
	BOOL _isPackaged;
	NSImage *icon;
	NSString *m_applicationName;
@public
	NSMenuItem *useAltFontMenuItem;
	NSMenuItem *expandSpacesMenuItem;
}

- (void)setPackaged:(BOOL)packaged;
- (NSString *) applicationName;
- (NSString *) executablePath;
- (void) updateAppMenu;

#define HypViewApp ((HypViewApplication *)NSApp)

@end

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

@interface HypViewWindow : NSWindow <NSWindowDelegate>
{
@public
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

	gboolean m_button_disabled[TO_MAX];
	gboolean m_button_hidden[TO_MAX];
	
	HypViewView *textview;

	WP_UNIT scrollhpos, scrollhsize;
	WP_UNIT scrollvpos, scrollvsize;
	BLOCK selection;
	gboolean hovering_over_link;
	struct {
		WP_UNIT x;
		WP_UNIT y;
		WP_UNIT w;
		WP_UNIT h;
	} docsize;
	WINDOW_DATA *parentwin;
}

- (id)initWithContentRect:(NSRect)contentRect;

@end

enum blockop {
	CO_SAVE,
	CO_BACK,
	CO_CUT,
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

typedef unsigned int Pixel;

struct _viewer_colors {
	Pixel background;       /* window background color */
	Pixel text;             /* Displays text in the specified color */
	Pixel link;             /* Displays references in the specified color */
	Pixel popup;            /* Displays references to popups in the specified color */
	Pixel xref;             /* Displays external references in the specified color */
	Pixel system;           /* Displays references to {@ system } in the specified color */
	Pixel rx;               /* Displays references to {@ rx } in the specified color */
	Pixel rxs;              /* Displays references to {@ rxs } in the specified color */
	Pixel quit;             /* Displays references to {@ quit } in the specified color */
	Pixel close;            /* Displays references to {@ close } in the specified color */
	Pixel error;            /* used to display invalid links in hypertext files */
	Pixel ghosted;          /* used to display light text attribute */
};

typedef struct _link_info {
	unsigned char link_type;
	hyp_indextype dst_type;
	char *tip;
	hyp_nodenr dest_page;
	hyp_lineno line_nr;
} LINK_INFO;


/* about.c */

void About(id sender);


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
void show_message(WINDOW_DATA *parent, const char *title, const char *text, gboolean big);
gboolean ask_yesno(WINDOW_DATA *parent, const char *text);
WINDOW_DATA *top_window(void);


/*
 * hv_gfx.c
 */
_WORD GetNumColors(void);
_WORD GetNumPlanes(void);
_WORD GetScreenHeight(void);
_WORD GetScreenWidth(void);


/*
 * hv_init.c
 */
extern struct _viewer_colors viewer_colors;
extern Pixel const hyp_colors[256];

void hv_init(void);
void hv_exit(void);


/*
 * hv_font.c
 */
#define FONT_NAME_LEN 256

void SwitchFont(WINDOW_DATA *win, gboolean clearcache);
void SelectFont(WINDOW_DATA *win);


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
void hv_win_set_geometry(const char *geometry);
void hv_win_open(WINDOW_DATA *win);
WINDOW_DATA *macos_hypview_window_new(DOCUMENT *doc, gboolean popup);
void macos_destroy_window(WINDOW_DATA *win);
void ReInitWindow(WINDOW_DATA *win, gboolean prep);
void hv_set_title(WINDOW_DATA *win, const char *wintitle);
void hv_set_font(WINDOW_DATA *win);
void SendRedraw(WINDOW_DATA *win);
void SendCloseWindow(WINDOW_DATA *win);
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
gboolean position_popup(NSMenu *menu, struct popup_pos *pos, int *xret, int *yret);


/*
 * hv_hist.c
 */
void AddHistoryEntry(WINDOW_DATA *win, DOCUMENT *doc);
DOCUMENT *RemoveHistoryEntry(WINDOW_DATA *win, hyp_nodenr *node, long *line);
void RemoveAllHistoryEntries(WINDOW_DATA *win);


/*
 * hv_autol.c
 */
gboolean AutolocatorKey(WINDOW_DATA *win, unsigned int key);
void AutoLocatorPaste(WINDOW_DATA *win);
void AutoLocatorCut(WINDOW_DATA *win);
void AutoLocatorCopy(WINDOW_DATA *win);


/*
 * hv_fsel.c
 */
enum choose_file_mode {
	file_open,
	file_save,
	file_dirsel
};

extern char const hypertext_file_filter[];
extern char const text_file_filter[];
gboolean choose_file(WINDOW_DATA *parent, char **name, enum choose_file_mode mode, const char *title, const char *filter);
WINDOW_DATA *SelectFileLoad(WINDOW_DATA *win);
char *SelectFileSave(WINDOW_DATA *win, hyp_filetype type);


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
void MarkerSaveToDisk(gboolean ask);
void MarkerInit(void);
void on_bookmark_selected(WINDOW_DATA *win, int num);


/*
 * hv_nav.c
 */
void GotoPage(WINDOW_DATA *win, hyp_nodenr num, long line, gboolean calc);
void GoBack(WINDOW_DATA *win);
void HistoryPopup(WINDOW_DATA *win, enum toolbutton obj, int button);
void GotoHelp(WINDOW_DATA *win);
void GotoIndex(WINDOW_DATA *win);
void GoThisButton(WINDOW_DATA *win, enum toolbutton obj);
void GotoCatalog(WINDOW_DATA *win);
void GotoDefaultFile(WINDOW_DATA *win);


/*
 * hv_misc.c
 */
void CenterWindow(WINDOW_DATA *win);
void hv_recent_add(const char *path);
void RecentUpdate(WINDOW_DATA *win);
void on_recent_selected(WINDOW_DATA *win, int num);
void RecentInit(void);
void RecentExit(void);
void RecentSaveToDisk(void);
int gtk_XParseGeometry(const char *string, int *x, int *y, int *width, int *height);


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
gboolean BlockCopy(WINDOW_DATA *win);
gboolean BlockPaste(WINDOW_DATA *win, gboolean new_window);
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
void hv_config_output(WINDOW_DATA *win);


/*
 * hv_recomp.c
 */
gboolean hv_recompile(HYP_DOCUMENT *hyp, const char *output_filename, hyp_filetype type);



#endif /* __HV_DEFS_H__ */
