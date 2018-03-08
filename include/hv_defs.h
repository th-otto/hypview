#ifndef __HV_DEFS_H__
#define __HV_DEFS_H__

#include "hypdoc.h"
#include "tos/diallib.h"

#define HYPVIEW_VERSION "1.0"


/*
 *		Global.c
 */
extern short sel_font_id, sel_font_pt;


/*
 *		Init.c
 */
void Init(void);
void Exit(void);

/*
 *		Fileselc.c
 */
void SelectFileLoad(WINDOW_DATA *win);
void SelectFileSave(DOCUMENT *doc);


/*
 *		hv_file.c
 */
WINDOW_DATA *OpenFileNewWindow(const char *path, const char *chapter, hyp_nodenr node, _BOOL find_default);
WINDOW_DATA *OpenFileSameWindow(WINDOW_DATA *win, const char *path, const char *chapter, gboolean new_window, gboolean no_message);
/*
 *		Window.c
 */
void SendCloseWindow(WINDOW_DATA *wind);
void SendClose(_WORD whandle);
void SendTopped(_WORD whandle);
void SendRedraw(WINDOW_DATA *wind);
void ReInitWindow(DOCUMENT *doc);
gboolean HelpWindow(WINDOW_DATA *ptr, _WORD obj, void *data);
void WindowCalcScroll(WINDOW_DATA *win);


/*
 *		Toolbar.c
 */
void ToolbarUpdate(DOCUMENT *doc, gboolean redraw);
void ToolbarClick(DOCUMENT *doc, short obj);
void RemoveSearchBox(DOCUMENT *doc);


/*
 *		History.c
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
 *		Autoloc.c
 */
short AutolocatorKey(DOCUMENT *doc, short kbstate, short ascii);
void AutoLocatorPaste(DOCUMENT *doc);


/*
 *		selectio.c
 */
void SelectAll(DOCUMENT *doc);
void MouseSelection(DOCUMENT *doc,EVNTDATA *m_data);
void RemoveSelection(DOCUMENT *doc);
void DrawSelection(DOCUMENT *doc);


/*	
 *		Font.c
 */
gboolean ProportionalFont(_WORD *width);
void SwitchFont(DOCUMENT *doc);
void SelectFont(DOCUMENT *doc);


/*
 *		Block.c
 */
void BlockOperation(DOCUMENT *doc, short num);
void BlockSelectAll(DOCUMENT *doc, BLOCK *b);
void BlockCopy(DOCUMENT *doc);
void BlockPaste(WINDOW_DATA *win, gboolean new_window);
void BlockAsciiSave(DOCUMENT *doc, const char *path);


/*
 *		Marker.c
 */
void MarkerSave(DOCUMENT *doc, short num);
void MarkerShow(DOCUMENT *doc, short num, gboolean new_window);
void MarkerPopup(DOCUMENT *doc, short x, short y);
void MarkerSaveToDisk(void);
void MarkerInit(void);

/*
 *		Info.c
 */
void ProgrammInfos(DOCUMENT *doc);

/*
 *		search.c
 */
void Search(DOCUMENT *doc);

/*
 *		hyp\search.c
 */
void Hypfind(DOCUMENT *doc);

char *GetScrapPath(gboolean clear);


/*
 *		Tools.c
 */
WINDOW_DATA *get_first_window(void);
void hv_set_title(WINDOW_DATA *win, const char *wintitle);


/*
 *		Navigate.c
 */
void GotoPage(DOCUMENT *doc, hyp_nodenr num, long line, gboolean calc);
void GoBack(DOCUMENT *doc);
void MoreBackPopup(DOCUMENT *doc, short x, short y);
void GotoHelp(DOCUMENT *doc);
void GotoIndex(DOCUMENT *doc);
void GoThisButton(DOCUMENT *doc, short obj);


/*
 * userdef objects
 */
_VOID *hfix_objs(RSHDR *hdr, OBJECT *_ob, _WORD _num_objs);
_VOID hrelease_objs(OBJECT *_ob, _WORD _num_objs);
void hv_userdef_exit(void);


/*
 *		hv_hfind.c
 */
void HypfindFinish(short AppID, short ret);

#endif /* __HV_DEFS_H__ */
