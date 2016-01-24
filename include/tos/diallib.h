/*
 * HypView - (c) 2001 - 2006 Philipp Donze
 *               2006 -      Philipp Donze & Odd Skancke
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _diallib_h
#define _diallib_h

#define	NLS_LANG_GERMAN         0
#define	NLS_LANG_ENGLISH        1
#define	NLS_LANG_FRENCH         0

#include "tos/dl_user.h"            /* Application specific definitions */


/* Window types */
#define	WIN_WINDOW  1
#define	WIN_DIALOG  2
#define	WIN_FILESEL 3
#define	WIN_FONTSEL 4
#define	WIN_PRINTER 5

/* USER_DATA status flags */
#define	WIS_OPEN        0x01
#define	WIS_ICONIFY     0x02
#define	WIS_ALLICONIFY  0x04
#define	WIS_FULL        0x10
#define WIS_MFCLOSE     0x20	/* Marked for CLOSE */


/* Window messages */
#define	WIND_INIT                -1
#define	WIND_OPEN                -2
#define	WIND_OPENSIZE            -3
#define	WIND_EXIT                -4
#define	WIND_CLOSE               -5

#define	WIND_KEYPRESS           -10
#define	WIND_CLICK              -11

#define	WIND_REDRAW             -20
#define	WIND_SIZED              -21
#define	WIND_MOVED              -22
#define	WIND_TOPPED             -23
#define	WIND_NEWTOP             -24
#define	WIND_UNTOPPED           -25
#define	WIND_ONTOP              -26
#define	WIND_BOTTOM             -27
#define	WIND_FULLED             -28

#define	WIND_DRAGDROPFORMAT		-30
#define	WIND_DRAGDROP           -31

#define	WIND_ICONIFY            -40
#define	WIND_UNICONIFY          -41
#define	WIND_ALLICONIFY         -42

#define	WIND_BUBBLE             -50

/* Toolbar messages */
#define	WIND_TBCLICK            -60
#define	WIND_TBUPDATE           -61

/* File maximum */
#define	DL_PATHMAX              256

#define COMMON_CHAIN_DATA(linktype) \
	linktype *next; \
	linktype *previous; \
	_WORD type; \
	_WORD whandle; \
	_WORD status; \
	GRECT last; \
	_WORD owner

typedef struct _chain_data_ CHAIN_DATA;
struct _chain_data_
{
	COMMON_CHAIN_DATA(CHAIN_DATA);
};

typedef struct _dialog_data_ DIALOG_DATA;
struct _dialog_data_
{
	COMMON_CHAIN_DATA(DIALOG_DATA);

	DIALOG *dial;
	OBJECT *obj;
	const char *title;
	void *data;
};


typedef long WP_UNIT;

typedef	gboolean (*HNDL_WIN)(WINDOW_DATA *wptr, _WORD obj, void *data);

#define __WINDOW_DATA_IMPLEMENTATION__

struct _window_data_
{
	COMMON_CHAIN_DATA(WINDOW_DATA);

	GRECT full;
	GRECT work;
	GRECT scroll;
	HNDL_WIN proc;
	const char *title;
	char titlebuf[80];
	short	kind;
#if OPEN_VDI_WORKSTATION
	short	vdi_handle;
	_WORD workout[57];
	_WORD ext_workout[57];
#endif
	struct {
		WP_UNIT x;
		WP_UNIT y;
		WP_UNIT w;
		WP_UNIT h;
	} docsize;
	short	x_speed;
	short	y_speed;
	short	x_margin_left, x_margin_right;
	short	y_margin_top, y_margin_bottom;
#if USE_LOGICALRASTER
	short	x_raster;
	short	y_raster;
#endif
#if USE_TOOLBAR
	OBJECT *toolbar;
	short	x_offset;
	short	y_offset;
#endif
	char *autolocator;          /* Autolocator search string */
	BLOCK selection;            /* Contents of selection */
	DOCUMENT *data;
	WINDOW_DATA *popup;
	HISTORY *history;
};

typedef struct _filesel_data_ FILESEL_DATA;
typedef	void (*HNDL_FSL)(FILESEL_DATA *fslx,short nfiles);

struct _filesel_data_
{
	COMMON_CHAIN_DATA(FILESEL_DATA);

	HNDL_FSL proc;
	void *dialog;
	char path[DL_PATHMAX];
	char name[DL_PATHMAX];
	_WORD	button;
	_WORD	sort_mode;
	_WORD	nfiles;
	void *data;
};

typedef struct _fontsel_data_ FONTSEL_DATA;
typedef	short (*HNDL_FONT)(FONTSEL_DATA *fnts_data);

struct _fontsel_data_
{
	COMMON_CHAIN_DATA(FONTSEL_DATA);

	HNDL_FONT proc;
	FNT_DIALOG *dialog;
	_WORD	font_flag;
	const char *opt_button;
	_WORD	button;
	_WORD	check_boxes;
	long	id,pt,ratio;
	_WORD	vdi_handle;
	void *data;
};


/*
 * dl_init.c
 */
extern	_WORD aes_handle, aes_fontid, aes_fontsize, pwchar, phchar, pwbox, phbox;
extern	_WORD has_wlffp, has_iconify, has_form_popup;
#define has_fonts_dialog() (has_wlffp & 4)
#define has_listbox_dialog() (has_wlffp & 3)
#define has_window_dialogs() (has_wlffp & 1)
#define has_filesel_dialog() (has_wlffp & 8)
extern _WORD __magix;
#if USE_GLOBAL_VDI
extern	_WORD vdi_handle;
extern	_WORD workin[16];
extern	_WORD workout[57];
extern	_WORD ext_workout[57];
#endif
extern OBJECT *dial_library_tree;
extern OBJECT *toolbar_tree;
#if USE_MENU
extern	OBJECT	*menu_tree;
#endif
extern	KEYTAB *key_table;

OBJECT *rs_tree(_WORD nr);
char *rs_string(_WORD nr);

void singletos_fail_loop(void);
int DoAesInit(void);
int DoInitSystem(void);
void DoExitSystem(void);
void GetScreenSize(_WORD *width, _WORD *height);
_WORD GetNumColors(_VOID);

/*
 * dl_event.c
 */
extern	short	doneFlag;
extern	short	quitApp;
void DoEventDispatch(EVNT *event);
void DoEvent(void);

/*
 * dl_popup.c
 */
_WORD popup_select(OBJECT *tree, _WORD mx, _WORD my);

/*
 * dl_items.c
 */
extern	char	const iconified_name[];
extern	char	const prghelp_name[];
extern	OBJECT	*iconified_tree;
extern	CHAIN_DATA	*iconified_list[MAX_ICONIFY_PLACE];
extern	short	iconified_count;
extern	CHAIN_DATA	*all_list;
extern	short	modal_items;
extern	_WORD	modal_stack[MAX_MODALRECURSION];

void dialog_set_iconify(DIALOG *dialog, GRECT *g);

void add_item(CHAIN_DATA *item);
void remove_item(CHAIN_DATA *item);
void FlipIconify(void);
void AllIconify(short handle, GRECT *r);
void CycleItems(void);
void RemoveItems(void);
void ModalItem(void);
void ItemEvent(EVNT *event);
CHAIN_DATA *find_ptr_by_whandle(short handle);
CHAIN_DATA *find_ptr_by_status(short mask, short status);
CHAIN_DATA *find_ptr_by_type(_WORD type);


/*
 * dl_menu.c
 */
void ChooseMenu(short title, short entry);

/*
 * dl_dial.c
 */
DIALOG *OpenDialog(HNDL_OBJ proc, OBJECT *tree, const char *title, short x, short y, void *data);
void SendCloseDialog(DIALOG *dial);
void CloseDialog(DIALOG_DATA *ptr);
void CloseAllDialogs(void);
void RemoveDialog(DIALOG_DATA *ptr);

void DialogEvents(DIALOG_DATA *ptr,EVNT *event);
void SpecialMessageEvents(DIALOG *dialog,EVNT *event);

/*
 * dl_win.c
 */
WINDOW_DATA *OpenWindow(HNDL_WIN proc, short kind, const char *title, 
					WP_UNIT max_w, WP_UNIT max_h,void *user_data);
void CloseWindow(WINDOW_DATA *ptr);
void CloseAllWindows(void);
void RemoveWindow(WINDOW_DATA *ptr);
gboolean ScrollWindow(WINDOW_DATA *ptr, _WORD *rel_x, _WORD *rel_y);
void WindowEvents(WINDOW_DATA *ptr, EVNT *event);
void SetWindowSlider(WINDOW_DATA *ptr);
void ResizeWindow(WINDOW_DATA *ptr, WP_UNIT max_w, WP_UNIT max_h);
void IconifyWindow(WINDOW_DATA *ptr,GRECT *r);
void UniconifyWindow(WINDOW_DATA *ptr);
void DrawToolbar(WINDOW_DATA *win);
WINDOW_DATA *find_openwindow_by_whandle(short handle);
WINDOW_DATA *find_window_by_whandle(short handle);
WINDOW_DATA *find_window_by_proc(HNDL_WIN proc);
WINDOW_DATA *find_window_by_data(void *data);
int count_window(void);

/*
 * dl_filsl.c
 */
void *OpenFileselector(HNDL_FSL proc,char *comment,char *filepath,char *path, const char *pattern, short mode, void *data);
void FileselectorEvents(FILESEL_DATA *ptr,EVNT *event);
void RemoveFileselector(FILESEL_DATA *ptr);

/*
 * dl_fonsl.c
 */
extern char fnts_std_text[80];
FONTSEL_DATA *CreateFontselector(HNDL_FONT proc, short font_flag, const char *sample_text, const char *opt_button);
short OpenFontselector(FONTSEL_DATA *ptr,short button_flag,long id,long pt,long ratio);
void CloseFontselector(FONTSEL_DATA *ptr);
void RemoveFontselector(FONTSEL_DATA *ptr);
void FontselectorEvents(FONTSEL_DATA *ptr,EVNT *event);

/*
 * dl_av.c
 */
extern long av_server_cfg;

void DoVA_PROTOSTATUS(_WORD message[8]);
void DoAV_PROTOKOLL(short flags);
void DoAV_EXIT(void);

#define PROT_NAMEPTR(name) \
	(_UWORD)((_ULONG)(name) >> 16), \
	(_UWORD)(_ULONG)(name)

_BOOL Protokoll_Send(_WORD apid, _UWORD prot, _UWORD a1, _UWORD a2, _UWORD a3, _UWORD a4, _UWORD a5);
_BOOL Protokoll_Broadcast(_WORD *message, _BOOL send_to_self);
_WORD shel_xwrite(_WORD sh_wdoex, _WORD sh_wisgr, _WORD sh_wiscr, const void *sh_wpcmd, const char *sh_wptail);
_BOOL appl_xsearch(_WORD stype, char *name, _WORD *type, _WORD *id);
_WORD appl_locate(const char *pathlist, _BOOL startit);
void va_proto_init(void);
void va_proto_exit(void);

/*
 * dl_avcmd.c
 */
void SendAV_GETSTATUS(void);
void SendAV_STATUS(const char *string);
void SendAV_SENDKEY(short kbd_state, short code);
void SendAV_ASKFILEFONT(void);
void SendAV_ASKCONFONT(void);
void SendAV_ASKOBJECT(void);
void SendAV_OPENCONSOLE(void);
void SendAV_OPENWIND(const char *path, const char *wildcard);
void SendAV_STARTPROG(const char *path, const char *commandline);
void SendAV_ACCWINDOPEN(short handle);
void SendAV_ACCWINDCLOSED(short handle);
void SendAV_COPY_DRAGGED(short kbd_state, const char *path);
void SendAV_PATH_UPDATE(const char *path);
void SendAV_WHAT_IZIT(short x, short y);
void SendAV_DRAG_ON_WINDOW(short x,short y, short kbd_state, const char *path);
void SendAV_STARTED(const _WORD *msg);
void SendVA_START(_WORD dst_app, const char *path);
void SendAV_XWIND(const char *path, const char *wild_card, short bits);
void SendAV_VIEW(const char *path);
void SendAV_FILEINFO(const char *path);
void SendAV_COPYFILE(const char *file_list, const char *dest_path, short bits);
void SendAV_DELFILE(const char *file_list);
void SendAV_SETWINDPOS(short x, short y, short w, short h);
void SendAV_SENDCLICK(EVNTDATA *mouse, short ev_return);

/*
 * dl_drag.c
 */
void DragDrop(_WORD message[]);

/*
 * dl_bubbl.c
 */
void DoInitBubble(void);
void DoExitBubble(void);
void Bubble(short mx,short my);

/*
 * dl_help.c
 */
_WORD help_viewer_id(void);
void STGuideHelp(void);

/*
 * dl_ledit.c
 */
void DoInitLongEdit(void);
void DoExitLongEdit(void);

/*
 * dl_dhst.c
 */
void DhstAddFile(const char *path);
void DhstFree(_WORD message[]);

/*
 * dl_routs.c
 */
void ConvertKeypress(_WORD *key, _WORD *kstate);
void CopyMaximumChars(OBJECT *obj, char *str);
char *ParseData(char *start);
short rc_intersect_my(GRECT *p1, GRECT *p2);

/*
 * dl_user.c
 */
typedef struct
{
	short	tree;
	short	obj;
	short	len;
	XTED	xted;
} LONG_EDIT;

extern LONG_EDIT long_edit[];
extern short long_edit_count;

void DoButton(EVNT *event);
void DoUserEvents(EVNT *event);
void SelectMenu(short title, short entry);
void DD_Object(DIALOG *dial,GRECT *rect,OBJECT *tree,short obj, char *data, unsigned long format);
void DD_DialogGetFormat(OBJECT *tree,short obj, unsigned long format[]);
char *GetTopic(void);
void DoVA_START(_WORD msg[8]);
void DoVA_DRAGACCWIND(_WORD msg[8]);
void DoVA_Message(_WORD msg[8]);


#endif       /* _diallib_h */
