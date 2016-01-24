#ifndef __HV_DEFS_H__
#define __HV_DEFS_H__

#include "hypdoc.h"
#include "tos/diallib.h"

#define HYPVIEW_VERSION "1.0"


#if defined(__WIN32__) || defined(__TOS__)
#define filename_cmp strcasecmp
#else
#define filename_cmp strcmp
#endif

/*
 * Global.c
 */
extern short sel_font_id, sel_font_pt;


/*
 * hv_init.c
 */
void hv_init(void);
void hv_exit(void);

/*
 * hv_fsel.c
 */
void SelectFileLoad(WINDOW_DATA *win);
void SelectFileSave(WINDOW_DATA *win);


/*
 * hv_file.c
 */
WINDOW_DATA *OpenFileInWindow(WINDOW_DATA *win, const char *path, const char *chapter, hyp_nodenr node, gboolean find_default, int new_window, gboolean no_message);


/*
 * hv_win.c
 */
void SendCloseWindow(WINDOW_DATA *win);
void SendClose(_WORD whandle);
void SendTopped(_WORD whandle);
void SendRedraw(WINDOW_DATA *wind);
void ReInitWindow(WINDOW_DATA *win);
gboolean HelpWindow(WINDOW_DATA *ptr, _WORD obj, void *data);
void WindowCalcScroll(WINDOW_DATA *win);
void hv_set_title(WINDOW_DATA *win, const char *wintitle);


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
void StartRemarker(gboolean quiet);


/*
 * hv_mark.c
 */
void MarkerSave(WINDOW_DATA *win, short num);
void MarkerShow(WINDOW_DATA *win, short num, gboolean new_window);
void MarkerPopup(WINDOW_DATA *win, short x, short y);
void MarkerUpdate(WINDOW_DATA *win);
void MarkerSaveToDisk(void);
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
WINDOW_DATA *get_first_window(void);


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
_VOID *hfix_objs(RSHDR *hdr, OBJECT *_ob, _WORD _num_objs);
_VOID hrelease_objs(OBJECT *_ob, _WORD _num_objs);
void hv_userdef_exit(void);


/*
 * hv_hfind.c
 */
void HypfindFinish(short AppID, short ret);


/*
 * hv_popup.c
 */
void OpenPopup(WINDOW_DATA *win, hyp_nodenr num, short x, short y);


/*
 * hv_eref.c
 */
void HypExtRefPopup(WINDOW_DATA *win, short x, short y);
void HypOpenExtRef(WINDOW_DATA *win, const char *name, gboolean new_window);

#endif /* __HV_DEFS_H__ */
