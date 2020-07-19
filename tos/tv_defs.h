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

void hv_init_colors(void);

/*
 * hv_fsel.c
 */
void SelectFileLoad(WINDOW_DATA *win);
void SelectFileSave(WINDOW_DATA *win);


/*
 * hv_file.c
 */
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
 * dl_tools.c
 */
WINDOW_DATA *top_window(void);


/*
 * userdef objects
 */
void *hfix_objs(RSHDR *hdr, OBJECT *_ob, _WORD _num_objs);
void hrelease_objs(OBJECT *_ob, _WORD _num_objs);
void hv_userdef_exit(void);
void hfix_palette(_WORD vdi_h);


/*
 * hv_popup.c
 */
gboolean OpenPopup(WINDOW_DATA *win, hyp_nodenr num, hyp_lineno line, short x, short y);


#endif /* __HV_DEFS_H__ */
