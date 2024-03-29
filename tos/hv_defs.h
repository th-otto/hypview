#ifndef __HV_DEFS_H__
#define __HV_DEFS_H__

#include "hypdoc.h"
#include "tos/diallib.h"
#include "dbgwin.h"


struct _viewer_colors {
	_WORD background;       /* window background color */
	_WORD text;             /* Displays text in the specified color */
	_WORD link;             /* Displays references in the specified color */
	_WORD popup;            /* Displays references to popups in the specified color */
	_WORD xref;             /* Displays external references in the specified color */
	_WORD system;           /* Displays references to {@ system } in the specified color */
	_WORD rx;               /* Displays references to {@ rx } in the specified color */
	_WORD rxs;              /* Displays references to {@ rxs } in the specified color */
	_WORD quit;             /* Displays references to {@ quit } in the specified color */
	_WORD close;            /* Displays references to {@ close } in the specified color */
	_WORD error;            /* used to display invalid links in hypertext files */
};

/*
 * hv_init.c
 */
extern struct _viewer_colors viewer_colors;

void hv_init(void);
void hv_exit(void);
void hv_init_colors(void);

/*
 * hv_fsel.c
 */
void SelectFileLoad(WINDOW_DATA *win);
void SelectFileSave(WINDOW_DATA *win);


/*
 * hv_file.c
 */
WINDOW_DATA *OpenFileInWindow(WINDOW_DATA *win, const char *path, const char *chapter, hyp_nodenr node, gboolean find_default, int new_window, gboolean no_message);
WINDOW_DATA *tv_open_window(const char *path);


/*
 * hv_win.c
 */
#define FORCE_NEW_WINDOW 2

void SendCloseWindow(WINDOW_DATA *win);
void SendTopped(_WORD whandle);
void SendRedraw(WINDOW_DATA *wind);
void SendRedrawArea(WINDOW_DATA *win, const GRECT *area);
void ReInitWindow(WINDOW_DATA *win, gboolean prep);
gboolean HelpWindow(WINDOW_DATA *ptr, _WORD obj, void *data);
gboolean PopupWindow(WINDOW_DATA *win, _WORD obj, void *data);
void WindowCalcScroll(WINDOW_DATA *win);
void hv_set_title(WINDOW_DATA *win, const char *wintitle);
WINDOW_DATA *hv_win_new(DOCUMENT *doc, gboolean popup);
void hv_win_open(WINDOW_DATA *win);
long hv_win_topline(WINDOW_DATA *win);


/*
 * hv_tbar.c
 */
void ToolbarUpdate(WINDOW_DATA *win, gboolean redraw);
void ToolbarClick(WINDOW_DATA *win, short obj);
void RemoveSearchBox(WINDOW_DATA *win);


/*
 *	hv_click.c
 */
void HypClick(WINDOW_DATA *win, EVNTDATA *event);



/*
 * hv_hist.c
 */
void AddHistoryEntry(WINDOW_DATA *win, DOCUMENT *doc);
DOCUMENT *RemoveHistoryEntry(WINDOW_DATA *win, hyp_nodenr *node, long *line);
void RemoveAllHistoryEntries(WINDOW_DATA *win);


/*
 * hv_autol.c
 */
gboolean AutolocatorKey(WINDOW_DATA *win, short kbstate, short ascii);
void AutoLocatorPaste(WINDOW_DATA *win);


/*
 * hv_selec.c
 */
void SelectAll(WINDOW_DATA *win);
void MouseSelection(WINDOW_DATA *win, EVNTDATA *m_data);
void RemoveSelection(WINDOW_DATA *win);
void DrawSelection(WINDOW_DATA *win);


/*
 * hv_font.c
 */
gboolean ProportionalFont(_WORD *width);
void SwitchFont(WINDOW_DATA *win);
void SelectFont(WINDOW_DATA *win);


/*
 * hv_block.c
 */
char *GetScrapPath(gboolean clear);
void BlockOperation(WINDOW_DATA *win, short num);
void BlockSelectAll(WINDOW_DATA *win, BLOCK *b);
void BlockCopy(WINDOW_DATA *win);
void BlockPaste(WINDOW_DATA *win, gboolean new_window);
void BlockAsciiSave(WINDOW_DATA *win, const char *path);
typedef enum { remarker_top, remarker_startup, remarker_check, remarker_update } remarker_mode;
_WORD StartRemarker(WINDOW_DATA *win, remarker_mode mode, gboolean quiet);


/*
 * hv_mark.c
 */
void MarkerSave(WINDOW_DATA *win, short num);
void MarkerShow(WINDOW_DATA *win, short num, gboolean new_window);
void MarkerPopup(WINDOW_DATA *win, short x, short y);
void MarkerUpdate(WINDOW_DATA *win);
void MarkerSaveToDisk(gboolean ask);
void MarkerInit(void);


/*
 * hv_info.c
 */
void DocumentInfos(WINDOW_DATA *win);


/*
 * hv_hfind.c
 */
void Hypfind(WINDOW_DATA *win, gboolean again);


/*
 * dl_tools.c
 */
WINDOW_DATA *top_window(void);


/*
 * hv_nav.c
 */
void GotoPage(WINDOW_DATA *win, hyp_nodenr num, long line, gboolean calc);
void GoBack(WINDOW_DATA *win);
void HistoryPopup(WINDOW_DATA *win, short x, short y);
void GotoHelp(WINDOW_DATA *win);
void GotoIndex(WINDOW_DATA *win);
void GoThisButton(WINDOW_DATA *win, short obj);
void GotoCatalog(WINDOW_DATA *win);
void GotoDefaultFile(WINDOW_DATA *win);


/*
 * userdef objects
 */
void *hfix_objs(RSHDR *hdr, OBJECT *_ob, _WORD _num_objs);
void hrelease_objs(OBJECT *_ob, _WORD _num_objs);
void hv_userdef_exit(void);
void hfix_palette(_WORD vdi_h);


/*
 * hv_hfind.c
 */
void HypfindFinish(short AppID, short ret);


/*
 * hv_popup.c
 */
gboolean OpenPopup(WINDOW_DATA *win, hyp_nodenr num, hyp_lineno line, short x, short y);


/*
 * hv_eref.c
 */
void HypExtRefPopup(WINDOW_DATA *win, short x, short y);
void HypOpenExtRef(WINDOW_DATA *win, const char *name, hyp_lineno line_no, gboolean new_window);

/*
 * hv_rsc.c
 */
void ShowResource(WINDOW_DATA *win, const char *path, _UWORD treenr);

#endif /* __HV_DEFS_H__ */
