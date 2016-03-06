/*****************************************************************************
 * win32/windebug.c
 *****************************************************************************/

#include "windows_.h"
#include <winver.h>
#include <winspool.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <commdlg.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <imm.h>
#include <ddeml.h>
#include <pbt.h>
#include <cpl.h>
#include <dde.h>
#include <dbt.h>
#include <stdio.h>
#include <stdarg.h>
#include "windebug.h"


/* MFC AFX messages WM_AFXFIRST through WM_AFXLAST */
#define WM_QUERYAFXWNDPROC  0x0360
#define WM_SIZEPARENT       0x0361
#define WM_SETMESSAGESTRING 0x0362
#define WM_IDLEUPDATECMDUI  0x0363
#define WM_INITIALUPDATE    0x0364
#define WM_COMMANDHELP      0x0365
#define WM_HELPHITTEST      0x0366
#define WM_EXITHELPMODE     0x0367
#define WM_RECALCPARENT     0x0368
#define WM_SIZECHILD        0x0369
#define WM_KICKIDLE         0x036A
#define WM_QUERYCENTERWND   0x036B
#define WM_DISABLEMODAL     0x036C
#define WM_FLOATSTATUS      0x036D
#define WM_ACTIVATETOPLEVEL 0x036E
#define WM_QUERY3DCONTROLS  0x036F
#define WM_RESERVED_0370    0x0370
#define WM_RESERVED_0371    0x0371
#define WM_RESERVED_0372    0x0372
#define WM_SOCKET_NOTIFY    0x0373
#define WM_SOCKET_DEAD      0x0374
#define WM_POPMESSAGESTRING 0x0375
#define WM_OCC_LOADFROMSTREAM   0x0376
#define WM_OCC_LOADFROMSTORAGE  0x0377
#define WM_OCC_INITNEW          0x0378
#define WM_QUEUE_SENTINEL   0x0379
#define WM_RESERVED_037A    0x037A
#define WM_RESERVED_037B    0x037B
#define WM_RESERVED_037C    0x037C
#define WM_RESERVED_037D    0x037D
#define WM_RESERVED_037E    0x037E
#define WM_RESERVED_037F    0x037F

/* PENWIN message WM_PENWINFIRST through WM_PENWINLAST */
#define WM_RCRESULT             (WM_PENWINFIRST+1)  /* 0x381 */
#define WM_HOOKRCRESULT         (WM_PENWINFIRST+2)  /* 0x382 */
#define WM_PENMISCINFO          (WM_PENWINFIRST+3)  /* 0x383 */
#define WM_SKB                  (WM_PENWINFIRST+4)  /* 0x384 */
#define WM_PENCTL               (WM_PENWINFIRST+5)  /* 0x385 */
#define WM_PENMISC              (WM_PENWINFIRST+6)  /* 0x386 */
#define WM_CTLINIT              (WM_PENWINFIRST+7)  /* 0x387 */
#define WM_PENEVENT             (WM_PENWINFIRST+8)  /* 0x388 */


#define WM_RASDIALEVENT 0xCCCD


#ifndef UNUSED
# define UNUSED(x) (void) x
#endif

#ifndef WM_SYSTEMERROR
#  define WM_SYSTEMERROR 23
#endif
#ifndef WM_CTLCOLOR
#  define WM_CTLCOLOR 25
#endif



#define caseconst(name) case name: return #name
#define caseconst2(name) case name: strcat(buf, #name); break
#define bit(val, name) \
	if ((val & name) == name) \
	{ \
		strcat(buf, "|" #name); \
		val &= ~name; \
	}
#define ebit(val, name, elsename) \
	if ((val & name) == name) \
	{ \
		strcat(buf, "|" #name); \
		val &= ~name; \
	} else \
	{ \
		strcat(buf, "|" #elsename); \
	}
#define ebit2(val, name, name2, elsename) \
	if ((val & name) == name) \
	{ \
		strcat(buf, "|" #name); \
		val &= ~name; \
	} else if ((val & name2) == name2) \
	{ \
		strcat(buf, "|" #name2); \
		val &= ~name2; \
	} else \
	{ \
		strcat(buf, "|" #elsename); \
	}


#define checkptr(ptr, size) \
	if ((ptr) == NULL) \
		return "NULL"; \
	if (IsBadReadPtr(ptr, size)) \
	{ \
		sprintf(buf, "Inv:$%08lx", (DWORD)(ptr)); \
		return buf; \
	}

/*** ---------------------------------------------------------------------- ***/

static const char *win32debug_msg_name1(UINT message, char *buf)
{
#ifndef WM_GETOBJECT
#define WM_GETOBJECT                    0x003D
#endif
#ifndef WM_SYNCPAINT
#define WM_SYNCPAINT                    0x0088
#endif
#ifndef WM_NCXBUTTONDOWN
#define WM_NCXBUTTONDOWN                0x00AB
#endif
#ifndef WM_NCXBUTTONUP
#define WM_NCXBUTTONUP                  0x00AC
#endif
#ifndef WM_NCXBUTTONDBLCLK
#define WM_NCXBUTTONDBLCLK              0x00AD
#endif
#ifndef BM_SETDONTCLICK
#define BM_SETDONTCLICK    0x00F8
#endif
#ifndef WM_INPUT_DEVICE_CHANGE
#define WM_INPUT_DEVICE_CHANGE          0x00FE
#endif
#ifndef WM_INPUT
#define WM_INPUT                        0x00FF
#endif
#ifndef WM_UNICHAR
#define WM_UNICHAR                      0x0109
#endif
#ifndef WM_GESTURE
#define WM_GESTURE                      0x0119
#endif
#ifndef WM_GESTURENOTIFY
#define WM_GESTURENOTIFY                0x011A
#endif
#ifndef WM_MENURBUTTONUP
#define WM_MENURBUTTONUP                0x0122
#endif
#ifndef WM_MENUDRAG
#define WM_MENUDRAG                     0x0123
#endif
#ifndef WM_MENUGETOBJECT
#define WM_MENUGETOBJECT                0x0124
#endif
#ifndef WM_UNINITMENUPOPUP
#define WM_UNINITMENUPOPUP              0x0125
#endif
#ifndef WM_MENUCOMMAND
#define WM_MENUCOMMAND                  0x0126
#endif
#ifndef WM_CHANGEUISTATE
#define WM_CHANGEUISTATE                0x0127
#endif
#ifndef WM_UPDATEUISTATE
#define WM_UPDATEUISTATE                0x0128
#endif
#ifndef WM_QUERYUISTATE
#define WM_QUERYUISTATE                 0x0129
#endif
#ifndef MN_GETHMENU
#define MN_GETHMENU                     0x01E1
#endif
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN                  0x020B
#endif
#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP                    0x020C
#endif
#ifndef WM_XBUTTONDBLCLK
#define WM_XBUTTONDBLCLK                0x020D
#endif
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL                  0x020E
#endif
#ifndef WM_POINTERDEVICECHANGE
#define WM_POINTERDEVICECHANGE          0x238
#endif
#ifndef WM_POINTERDEVICEINRANGE
#define WM_POINTERDEVICEINRANGE         0x239
#endif
#ifndef WM_POINTERDEVICEOUTOFRANGE
#define WM_POINTERDEVICEOUTOFRANGE      0x23A
#endif
#ifndef WM_TOUCH
#define WM_TOUCH                        0x0240
#endif
#ifndef WM_NCPOINTERUPDATE
#define WM_NCPOINTERUPDATE              0x0241
#endif
#ifndef WM_NCPOINTERDOWN
#define WM_NCPOINTERDOWN                0x0242
#endif
#ifndef WM_NCPOINTERUP
#define WM_NCPOINTERUP                  0x0243
#endif
#ifndef WM_POINTERUPDATE
#define WM_POINTERUPDATE                0x0245
#endif
#ifndef WM_POINTERDOWN
#define WM_POINTERDOWN                  0x0246
#endif
#ifndef WM_POINTERUP
#define WM_POINTERUP                    0x0247
#endif
#ifndef WM_POINTERENTER
#define WM_POINTERENTER                 0x0249
#endif
#ifndef WM_POINTERLEAVE
#define WM_POINTERLEAVE                 0x024A
#endif
#ifndef WM_POINTERACTIVATE
#define WM_POINTERACTIVATE              0x024B
#endif
#ifndef WM_POINTERCAPTURECHANGED
#define WM_POINTERCAPTURECHANGED        0x024C
#endif
#ifndef WM_TOUCHHITTESTING
#define WM_TOUCHHITTESTING              0x024D
#endif
#ifndef WM_POINTERWHEEL
#define WM_POINTERWHEEL                 0x024E
#endif
#ifndef WM_POINTERHWHEEL
#define WM_POINTERHWHEEL                0x024F
#endif
#ifndef DM_POINTERHITTEST
#define DM_POINTERHITTEST               0x0250
#endif
#ifndef WM_IME_REQUEST
#define WM_IME_REQUEST                  0x0288
#endif
#ifndef WM_NCMOUSEHOVER
#define WM_NCMOUSEHOVER                 0x02A0
#endif
#ifndef WM_NCMOUSELEAVE
#define WM_NCMOUSELEAVE                 0x02A2
#endif
#ifndef WM_WTSSESSION_CHANGE
#define WM_WTSSESSION_CHANGE            0x02B1
#endif
#ifndef WM_TABLET_DEFBASE
#define WM_TABLET_DEFBASE                    0x02C0
#endif
#ifndef WM_TABLET_ADDED
#define WM_TABLET_ADDED                      (WM_TABLET_DEFBASE + 8)
#endif
#ifndef WM_TABLET_DELETED
#define WM_TABLET_DELETED                    (WM_TABLET_DEFBASE + 9)
#endif
#ifndef WM_TABLET_FLICK
#define WM_TABLET_FLICK                      (WM_TABLET_DEFBASE + 11)
#endif
#ifndef WM_TABLET_QUERYSYSTEMGESTURESTATUS
#define WM_TABLET_QUERYSYSTEMGESTURESTATUS   (WM_TABLET_DEFBASE + 12)
#endif
#ifndef WM_DPICHANGED
#define WM_DPICHANGED                   0x02E0
#endif
#ifndef WM_APPCOMMAND
#define WM_APPCOMMAND                   0x0319
#endif
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED                 0x031A
#endif
#ifndef WM_CLIPBOARDUPDATE
#define WM_CLIPBOARDUPDATE              0x031D
#endif
#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED        0x031E
#endif
#ifndef WM_DWMNCRENDERINGCHANGED
#define WM_DWMNCRENDERINGCHANGED        0x031F
#endif
#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED  0x0320
#endif
#ifndef WM_DWMWINDOWMAXIMIZEDCHANGE
#define WM_DWMWINDOWMAXIMIZEDCHANGE     0x0321
#endif
#ifndef WM_DWMSENDICONICTHUMBNAIL
#define WM_DWMSENDICONICTHUMBNAIL           0x0323
#endif
#ifndef WM_DWMSENDICONICLIVEPREVIEWBITMAP
#define WM_DWMSENDICONICLIVEPREVIEWBITMAP   0x0326
#endif
#ifndef WM_GETTITLEBARINFOEX
#define WM_GETTITLEBARINFOEX            0x033F
#endif

	switch (message)
	{
	caseconst(WM_NULL);						/* 0x0000 */
	caseconst(WM_CREATE);					/* 0x0001 */
	caseconst(WM_DESTROY);					/* 0x0002 */
	caseconst(WM_MOVE);						/* 0x0003 */
											/* 0x0004 */
	caseconst(WM_SIZE);						/* 0x0005 */
	caseconst(WM_ACTIVATE);					/* 0x0006 */
	caseconst(WM_SETFOCUS);					/* 0x0007 */
	caseconst(WM_KILLFOCUS);				/* 0x0008 */
											/* 0x0009 */
	caseconst(WM_ENABLE);					/* 0x000A */
	caseconst(WM_SETREDRAW);				/* 0x000B */
	caseconst(WM_SETTEXT);					/* 0x000C */
	caseconst(WM_GETTEXT);					/* 0x000D */
	caseconst(WM_GETTEXTLENGTH);			/* 0x000E */
	caseconst(WM_PAINT);					/* 0x000F */
	caseconst(WM_CLOSE);					/* 0x0010 */
	caseconst(WM_QUERYENDSESSION);			/* 0x0011 */
	caseconst(WM_QUIT);						/* 0x0012 */
	caseconst(WM_QUERYOPEN);				/* 0x0013 */
	caseconst(WM_ERASEBKGND);				/* 0x0014 */
	caseconst(WM_SYSCOLORCHANGE);			/* 0x0015 */
	caseconst(WM_ENDSESSION);				/* 0x0016 */
	caseconst(WM_SYSTEMERROR);				/* 0x0017 */
	caseconst(WM_SHOWWINDOW);				/* 0x0018 */
	caseconst(WM_CTLCOLOR);					/* 0x0019 */
	caseconst(WM_WININICHANGE);				/* 0x001A */
	caseconst(WM_DEVMODECHANGE);			/* 0x001B */
	caseconst(WM_ACTIVATEAPP);				/* 0x001C */
	caseconst(WM_FONTCHANGE);				/* 0x001D */
	caseconst(WM_TIMECHANGE);				/* 0x001E */
	caseconst(WM_CANCELMODE);				/* 0x001F */
	caseconst(WM_SETCURSOR);				/* 0x0020 */
	caseconst(WM_MOUSEACTIVATE);			/* 0x0021 */
	caseconst(WM_CHILDACTIVATE);			/* 0x0022 */
	caseconst(WM_QUEUESYNC);				/* 0x0023 */
	caseconst(WM_GETMINMAXINFO);			/* 0x0024 */
											/* 0x0025 */
	caseconst(WM_PAINTICON);				/* 0x0026 */
	caseconst(WM_ICONERASEBKGND);			/* 0x0027 */
	caseconst(WM_NEXTDLGCTL);				/* 0x0028 */
											/* 0x0029 */
	caseconst(WM_SPOOLERSTATUS);			/* 0x002A */
	caseconst(WM_DRAWITEM);					/* 0x002B */
	caseconst(WM_MEASUREITEM);				/* 0x002C */
	caseconst(WM_DELETEITEM);				/* 0x002D */
	caseconst(WM_VKEYTOITEM);				/* 0x002E */
	caseconst(WM_CHARTOITEM);				/* 0x002F */
	caseconst(WM_SETFONT);					/* 0x0030 */
	caseconst(WM_GETFONT);					/* 0x0031 */
	caseconst(WM_SETHOTKEY);				/* 0x0032 */
	caseconst(WM_GETHOTKEY);				/* 0x0033 */
											/* 0x0034 */
											/* 0x0035 */
											/* 0x0036 */
	caseconst(WM_QUERYDRAGICON);			/* 0x0037 */
											/* 0x0038 */
	caseconst(WM_COMPAREITEM);				/* 0x0039 */
											/* 0x003a */
											/* 0x003b */
											/* 0x003c */
	caseconst(WM_GETOBJECT);				/* 0x003d */
											/* 0x003e */
											/* 0x003f */
											/* 0x0040 */
	caseconst(WM_COMPACTING);				/* 0x0041 */
											/* 0x0042 */
											/* 0x0043 */
	caseconst(WM_COMMNOTIFY);				/* 0x0044 */
											/* 0x0045 */
	caseconst(WM_WINDOWPOSCHANGING);		/* 0x0046 */
	caseconst(WM_WINDOWPOSCHANGED);			/* 0x0047 */
	caseconst(WM_POWER);					/* 0x0048 */
											/* 0x0049 */
	caseconst(WM_COPYDATA);					/* 0x004A */
	caseconst(WM_CANCELJOURNAL);			/* 0x004B */
											/* 0x004c */
											/* 0x004d */
	caseconst(WM_NOTIFY);					/* 0x004E */
											/* 0x004f */
	caseconst(WM_INPUTLANGCHANGEREQUEST);	/* 0x0050 */
	caseconst(WM_INPUTLANGCHANGE);			/* 0x0051 */
	caseconst(WM_TCARD);					/* 0x0052 */
	caseconst(WM_HELP);						/* 0x0053 */
	caseconst(WM_USERCHANGED);				/* 0x0054 */
	caseconst(WM_NOTIFYFORMAT);				/* 0x0055 */
											/* 0x0056 */
											/* 0x0057 */
											/* 0x0058 */
											/* 0x0059 */
											/* 0x005a */
											/* 0x005b */
											/* 0x005c */
											/* 0x005d */
											/* 0x005e */
											/* 0x005f */
											/* 0x0060 */
											/* 0x0061 */
											/* 0x0062 */
											/* 0x0063 */
											/* 0x0064 */
											/* 0x0065 */
											/* 0x0066 */
											/* 0x0067 */
											/* 0x0068 */
											/* 0x0069 */
											/* 0x006a */
											/* 0x006b */
											/* 0x006c */
											/* 0x006d */
											/* 0x006e */
											/* 0x006f */
											/* 0x0070 */
											/* 0x0071 */
											/* 0x0072 */
											/* 0x0073 */
											/* 0x0074 */
											/* 0x0075 */
											/* 0x0076 */
											/* 0x0077 */
											/* 0x0078 */
											/* 0x0079 */
											/* 0x007a */
	caseconst(WM_CONTEXTMENU);				/* 0x007B */
	caseconst(WM_STYLECHANGING);			/* 0x007C */
	caseconst(WM_STYLECHANGED);				/* 0x007D */
	caseconst(WM_DISPLAYCHANGE);			/* 0x007E */
	caseconst(WM_GETICON);					/* 0x007F */
	caseconst(WM_SETICON);					/* 0x0080 */
	caseconst(WM_NCCREATE);					/* 0x0081 */
	caseconst(WM_NCDESTROY);				/* 0x0082 */
	caseconst(WM_NCCALCSIZE);				/* 0x0083 */
	caseconst(WM_NCHITTEST);				/* 0x0084 */
	caseconst(WM_NCPAINT);					/* 0x0085 */
	caseconst(WM_NCACTIVATE);				/* 0x0086 */
	caseconst(WM_GETDLGCODE);				/* 0x0087 */
	caseconst(WM_SYNCPAINT);				/* 0x0088 */
											/* 0x0089 */
											/* 0x008a */
											/* 0x008b */
											/* 0x008c */
											/* 0x008d */
											/* 0x008e */
											/* 0x008f */
											/* 0x0090 */
											/* 0x0091 */
											/* 0x0092 */
											/* 0x0093 */
											/* 0x0094 */
											/* 0x0095 */
											/* 0x0096 */
											/* 0x0097 */
											/* 0x0098 */
											/* 0x0099 */
											/* 0x009a */
											/* 0x009b */
											/* 0x009c */
											/* 0x009d */
											/* 0x009e */
											/* 0x009f */
	caseconst(WM_NCMOUSEMOVE);				/* 0x00A0 */
	caseconst(WM_NCLBUTTONDOWN);			/* 0x00A1 */
	caseconst(WM_NCLBUTTONUP);				/* 0x00A2 */
	caseconst(WM_NCLBUTTONDBLCLK);			/* 0x00A3 */
	caseconst(WM_NCRBUTTONDOWN);			/* 0x00A4 */
	caseconst(WM_NCRBUTTONUP);				/* 0x00A5 */
	caseconst(WM_NCRBUTTONDBLCLK);			/* 0x00A6 */
	caseconst(WM_NCMBUTTONDOWN);			/* 0x00A7 */
	caseconst(WM_NCMBUTTONUP);				/* 0x00A8 */
	caseconst(WM_NCMBUTTONDBLCLK);			/* 0x00A9 */
											/* 0x00aa */
	caseconst(WM_NCXBUTTONDOWN);			/* 0x00ab */
	caseconst(WM_NCXBUTTONUP);				/* 0x00ac */
	caseconst(WM_NCXBUTTONDBLCLK);			/* 0x00ad */
											/* 0x00ae */
											/* 0x00af */
											/* 0x00b0 */
											/* 0x00b1 */
											/* 0x00b2 */
											/* 0x00b3 */
											/* 0x00b4 */
											/* 0x00b5 */
											/* 0x00b6 */
											/* 0x00b7 */
											/* 0x00b8 */
											/* 0x00b9 */
											/* 0x00ba */
											/* 0x00bb */
											/* 0x00bc */
											/* 0x00bd */
											/* 0x00be */
											/* 0x00bf */
											/* 0x00c0 */
											/* 0x00c1 */
											/* 0x00c2 */
											/* 0x00c3 */
											/* 0x00c4 */
											/* 0x00c5 */
											/* 0x00c6 */
											/* 0x00c7 */
											/* 0x00c8 */
											/* 0x00c9 */
											/* 0x00ca */
											/* 0x00cb */
											/* 0x00cc */
											/* 0x00cd */
											/* 0x00ce */
											/* 0x00cf */
											/* 0x00d0 */
											/* 0x00d1 */
											/* 0x00d2 */
											/* 0x00d3 */
											/* 0x00d4 */
											/* 0x00d5 */
											/* 0x00d6 */
											/* 0x00d7 */
											/* 0x00d8 */
											/* 0x00d9 */
											/* 0x00da */
											/* 0x00db */
											/* 0x00dc */
											/* 0x00dd */
											/* 0x00de */
											/* 0x00df */
											/* 0x00e0 */
											/* 0x00e1 */
											/* 0x00e2 */
											/* 0x00e3 */
											/* 0x00e4 */
											/* 0x00e5 */
											/* 0x00e6 */
											/* 0x00e7 */
											/* 0x00e8 */
											/* 0x00e9 */
											/* 0x00ea */
											/* 0x00eb */
											/* 0x00ec */
											/* 0x00ed */
											/* 0x00ee */
											/* 0x00ef */
	caseconst(BM_GETCHECK);					/* 0x00F0 */
	caseconst(BM_SETCHECK);					/* 0x00F1 */
	caseconst(BM_GETSTATE);					/* 0x00F2 */
	caseconst(BM_SETSTATE);					/* 0x00F3 */
	caseconst(BM_SETSTYLE);					/* 0x00F4 */
	caseconst(BM_CLICK);					/* 0x00F5 */
	caseconst(BM_GETIMAGE);					/* 0x00F6 */
	caseconst(BM_SETIMAGE);					/* 0x00F7 */
	caseconst(BM_SETDONTCLICK);				/* 0x00F8 */
											/* 0x00F9 */
											/* 0x00FA */
											/* 0x00FB */
											/* 0x00FC */
											/* 0x00FD */
	caseconst(WM_INPUT_DEVICE_CHANGE);		/* 0x00FE */
	caseconst(WM_INPUT);					/* 0x00FF */
	caseconst(WM_KEYDOWN);					/* 0x0100 */
	caseconst(WM_KEYUP);					/* 0x0101 */
	caseconst(WM_CHAR);						/* 0x0102 */
	caseconst(WM_DEADCHAR);					/* 0x0103 */
	caseconst(WM_SYSKEYDOWN);				/* 0x0104 */
	caseconst(WM_SYSKEYUP);					/* 0x0105 */
	caseconst(WM_SYSCHAR);					/* 0x0106 */
	caseconst(WM_SYSDEADCHAR);				/* 0x0107 */
											/* 0x0108 */
	caseconst(WM_UNICHAR);					/* 0x0109 */
											/* 0x010a */
											/* 0x010b */
											/* 0x010c */
	caseconst(WM_IME_STARTCOMPOSITION);		/* 0x010D */
	caseconst(WM_IME_ENDCOMPOSITION);		/* 0x010E */
	caseconst(WM_IME_COMPOSITION);			/* 0x010F */
	caseconst(WM_INITDIALOG);				/* 0x0110 */
	caseconst(WM_COMMAND);					/* 0x0111 */
	caseconst(WM_SYSCOMMAND);				/* 0x0112 */
	caseconst(WM_TIMER);					/* 0x0113 */
	caseconst(WM_HSCROLL);					/* 0x0114 */
	caseconst(WM_VSCROLL);					/* 0x0115 */
	caseconst(WM_INITMENU);					/* 0x0116 */
	caseconst(WM_INITMENUPOPUP);			/* 0x0117 */
											/* 0x0118 */
	caseconst(WM_GESTURE);					/* 0x0119 */
	caseconst(WM_GESTURENOTIFY);			/* 0x011a */
											/* 0x011b */
											/* 0x011c */
											/* 0x011d */
											/* 0x011e */
	caseconst(WM_MENUSELECT);				/* 0x011F */
	caseconst(WM_MENUCHAR);					/* 0x0120 */
	caseconst(WM_ENTERIDLE);				/* 0x0121 */
	caseconst(WM_MENURBUTTONUP);			/* 0x0122 */
	caseconst(WM_MENUDRAG);					/* 0x0123 */
	caseconst(WM_MENUGETOBJECT);			/* 0x0124 */
	caseconst(WM_UNINITMENUPOPUP);			/* 0x0125 */
	caseconst(WM_MENUCOMMAND);				/* 0x0126 */
	caseconst(WM_CHANGEUISTATE);			/* 0x0127 */
	caseconst(WM_UPDATEUISTATE);			/* 0x0128 */
	caseconst(WM_QUERYUISTATE);				/* 0x0129 */
											/* 0x012a */
											/* 0x012b */
											/* 0x012c */
											/* 0x012d */
											/* 0x012e */
											/* 0x012f */
											/* 0x0130 */
											/* 0x0131 */
	caseconst(WM_CTLCOLORMSGBOX);			/* 0x0132 */
	caseconst(WM_CTLCOLOREDIT);				/* 0x0133 */
	caseconst(WM_CTLCOLORLISTBOX);			/* 0x0134 */
	caseconst(WM_CTLCOLORBTN);				/* 0x0135 */
	caseconst(WM_CTLCOLORDLG);				/* 0x0136 */
	caseconst(WM_CTLCOLORSCROLLBAR);		/* 0x0137 */
	caseconst(WM_CTLCOLORSTATIC);			/* 0x0138 */

	caseconst(MN_GETHMENU);					/* 0x01e1 */

	caseconst(WM_MOUSEMOVE);				/* 0x0200 */
	caseconst(WM_LBUTTONDOWN);				/* 0x0201 */
	caseconst(WM_LBUTTONUP);				/* 0x0202 */
	caseconst(WM_LBUTTONDBLCLK);			/* 0x0203 */
	caseconst(WM_RBUTTONDOWN);				/* 0x0204 */
	caseconst(WM_RBUTTONUP);				/* 0x0205 */
	caseconst(WM_RBUTTONDBLCLK);			/* 0x0206 */
	caseconst(WM_MBUTTONDOWN);				/* 0x0207 */
	caseconst(WM_MBUTTONUP);				/* 0x0208 */
	caseconst(WM_MBUTTONDBLCLK);			/* 0x0209 */
	caseconst(WM_MOUSEWHEEL);				/* 0x020a */
	caseconst(WM_XBUTTONDOWN);				/* 0x020b */
	caseconst(WM_XBUTTONUP);				/* 0x020c */
	caseconst(WM_XBUTTONDBLCLK);			/* 0x020d */
	caseconst(WM_MOUSEHWHEEL);				/* 0x020e */
											/* 0x020f */
	caseconst(WM_PARENTNOTIFY);				/* 0x0210 */
	caseconst(WM_ENTERMENULOOP);			/* 0x0211 */
	caseconst(WM_EXITMENULOOP);				/* 0x0212 */
	caseconst(WM_NEXTMENU);					/* 0x0213 */
	caseconst(WM_SIZING);					/* 0x0214 */
	caseconst(WM_CAPTURECHANGED);			/* 0x0215 */
	caseconst(WM_MOVING);					/* 0x0216 */
	caseconst(WM_POWERBROADCAST);			/* 0x0218 */
	caseconst(WM_DEVICECHANGE);				/* 0x0219 */
											/* 0x021a */
											/* 0x021b */
											/* 0x021c */
											/* 0x021d */
											/* 0x021e */
											/* 0x021f */
	caseconst(WM_MDICREATE);				/* 0x0220 */
	caseconst(WM_MDIDESTROY);				/* 0x0221 */
	caseconst(WM_MDIACTIVATE);				/* 0x0222 */
	caseconst(WM_MDIRESTORE);				/* 0x0223 */
	caseconst(WM_MDINEXT);					/* 0x0224 */
	caseconst(WM_MDIMAXIMIZE);				/* 0x0225 */
	caseconst(WM_MDITILE);					/* 0x0226 */
	caseconst(WM_MDICASCADE);				/* 0x0227 */
	caseconst(WM_MDIICONARRANGE);			/* 0x0228 */
	caseconst(WM_MDIGETACTIVE);				/* 0x0229 */
											/* 0x022a */
											/* 0x022b */
											/* 0x022c */
											/* 0x022d */
											/* 0x022e */
											/* 0x022f */
	caseconst(WM_MDISETMENU);				/* 0x0230 */
	caseconst(WM_ENTERSIZEMOVE);			/* 0x0231 */
	caseconst(WM_EXITSIZEMOVE);				/* 0x0232 */
	caseconst(WM_DROPFILES);				/* 0x0233 */
	caseconst(WM_MDIREFRESHMENU);			/* 0x0234 */

	caseconst(WM_POINTERDEVICECHANGE);		/* 0x0238 */
	caseconst(WM_POINTERDEVICEINRANGE);		/* 0x0239 */
	caseconst(WM_POINTERDEVICEOUTOFRANGE);	/* 0x023a */
											/* 0x023b */
											/* 0x023c */
											/* 0x023d */
											/* 0x023e */
											/* 0x023f */
	caseconst(WM_TOUCH);					/* 0x0240 */
	caseconst(WM_NCPOINTERUPDATE);			/* 0x0241 */
	caseconst(WM_NCPOINTERDOWN);			/* 0x0242 */
	caseconst(WM_NCPOINTERUP);				/* 0x0243 */
											/* 0x0244 */
	caseconst(WM_POINTERUPDATE);			/* 0x0245 */
	caseconst(WM_POINTERDOWN);				/* 0x0246 */
	caseconst(WM_POINTERUP);				/* 0x0247 */
											/* 0x0248 */
	caseconst(WM_POINTERENTER);				/* 0x0249 */
	caseconst(WM_POINTERLEAVE);				/* 0x024A */
	caseconst(WM_POINTERACTIVATE);			/* 0x024B */
	caseconst(WM_POINTERCAPTURECHANGED);	/* 0x024C */
	caseconst(WM_TOUCHHITTESTING);			/* 0x024D */
	caseconst(WM_POINTERWHEEL);				/* 0x024E */
	caseconst(WM_POINTERHWHEEL);			/* 0x024F */
	caseconst(DM_POINTERHITTEST);			/* 0x0250 */
											
	caseconst(WM_IME_SETCONTEXT);			/* 0x0281 */
	caseconst(WM_IME_NOTIFY);				/* 0x0282 */
	caseconst(WM_IME_CONTROL);				/* 0x0283 */
	caseconst(WM_IME_COMPOSITIONFULL);		/* 0x0284 */
	caseconst(WM_IME_SELECT);				/* 0x0285 */
	caseconst(WM_IME_CHAR);					/* 0x0286 */
											/* 0x0287 */
	caseconst(WM_IME_REQUEST);				/* 0x0288 */
											/* 0x0289 */
											/* 0x028A */
											/* 0x028B */
											/* 0x028C */
											/* 0x028D */
											/* 0x028E */
											/* 0x028F */
	caseconst(WM_IME_KEYDOWN);				/* 0x0290 */
	caseconst(WM_IME_KEYUP);				/* 0x0291 */

	caseconst(WM_NCMOUSEHOVER);				/* 0x02A0 */
	caseconst(WM_MOUSEHOVER);				/* 0x02A1 */
	caseconst(WM_NCMOUSELEAVE);				/* 0x02A2 */
	caseconst(WM_MOUSELEAVE);				/* 0x02A3 */
	
	caseconst(WM_WTSSESSION_CHANGE);		/* 0x02b1 */

	caseconst(WM_TABLET_ADDED);				/* 0x02c8 */
	caseconst(WM_TABLET_DELETED);			/* 0x02c9 */
	caseconst(WM_TABLET_FLICK);				/* 0x02cb */
	caseconst(WM_TABLET_QUERYSYSTEMGESTURESTATUS);	/* 0x02cc */

	caseconst(WM_DPICHANGED);				/* 0x02e0 */
	
	caseconst(WM_CUT);						/* 0x0300 */
	caseconst(WM_COPY);						/* 0x0301 */
	caseconst(WM_PASTE);					/* 0x0302 */
	caseconst(WM_CLEAR);					/* 0x0303 */
	caseconst(WM_UNDO);						/* 0x0304 */
	caseconst(WM_RENDERFORMAT);				/* 0x0305 */
	caseconst(WM_RENDERALLFORMATS);			/* 0x0306 */
	caseconst(WM_DESTROYCLIPBOARD);			/* 0x0307 */
	caseconst(WM_DRAWCLIPBOARD);			/* 0x0308 */
	caseconst(WM_PAINTCLIPBOARD);			/* 0x0309 */
	caseconst(WM_VSCROLLCLIPBOARD);			/* 0x030A */
	caseconst(WM_SIZECLIPBOARD);			/* 0x030B */
	caseconst(WM_ASKCBFORMATNAME);			/* 0x030C */
	caseconst(WM_CHANGECBCHAIN);			/* 0x030D */
	caseconst(WM_HSCROLLCLIPBOARD);			/* 0x030E */
	caseconst(WM_QUERYNEWPALETTE);			/* 0x030F */
	caseconst(WM_PALETTEISCHANGING);		/* 0x0310 */
	caseconst(WM_PALETTECHANGED);			/* 0x0311 */
	caseconst(WM_HOTKEY);					/* 0x0312 */
											/* 0x0313 */
											/* 0x0314 */
											/* 0x0315 */
											/* 0x0316 */
	caseconst(WM_PRINT);					/* 0x0317 */
	caseconst(WM_PRINTCLIENT);				/* 0x0318 */
	caseconst(WM_APPCOMMAND);				/* 0x0319 */
	caseconst(WM_THEMECHANGED);				/* 0x031A */
											/* 0x031B */
											/* 0x031C */
	caseconst(WM_CLIPBOARDUPDATE);			/* 0x031D */
	caseconst(WM_DWMCOMPOSITIONCHANGED);	/* 0x031E */
	caseconst(WM_DWMNCRENDERINGCHANGED);	/* 0x031F */
	caseconst(WM_DWMCOLORIZATIONCOLORCHANGED);	/* 0x0320 */
	caseconst(WM_DWMWINDOWMAXIMIZEDCHANGE);	/* 0x0321 */
											/* 0x0322 */
	caseconst(WM_DWMSENDICONICTHUMBNAIL);	/* 0x0323 */
											/* 0x0324 */
											/* 0x0325 */
	caseconst(WM_DWMSENDICONICLIVEPREVIEWBITMAP);	/* 0x0326 */
	
	caseconst(WM_GETTITLEBARINFOEX);		/* 0x033f */
	
	caseconst(WM_HANDHELDFIRST);			/* 0x0358 */
	caseconst(WM_HANDHELDLAST);				/* 0x035F */
	
	caseconst(WM_QUERYAFXWNDPROC);			/* 0x0360 AFX */
	caseconst(WM_SIZEPARENT);				/* 0x0361 AFX */
	caseconst(WM_SETMESSAGESTRING);			/* 0x0362 AFX */
	caseconst(WM_IDLEUPDATECMDUI);			/* 0x0363 AFX */
	caseconst(WM_INITIALUPDATE);			/* 0x0364 AFX */
	caseconst(WM_COMMANDHELP);				/* 0x0365 AFX */
	caseconst(WM_HELPHITTEST);				/* 0x0366 AFX */
	caseconst(WM_EXITHELPMODE);				/* 0x0367 AFX */
	caseconst(WM_RECALCPARENT);				/* 0x0368 AFX */
	caseconst(WM_SIZECHILD);				/* 0x0369 AFX */
	caseconst(WM_KICKIDLE);					/* 0x036a AFX */
	caseconst(WM_QUERYCENTERWND);			/* 0x036b AFX */
	caseconst(WM_DISABLEMODAL);				/* 0x036c AFX */
	caseconst(WM_FLOATSTATUS);				/* 0x036d AFX */
	caseconst(WM_ACTIVATETOPLEVEL);			/* 0x036e AFX */
	caseconst(WM_QUERY3DCONTROLS);			/* 0x036f AFX */
	caseconst(WM_RESERVED_0370);			/* 0x0370 AFX */
	caseconst(WM_RESERVED_0371);			/* 0x0371 AFX */
	caseconst(WM_RESERVED_0372);			/* 0x0372 AFX */
	caseconst(WM_SOCKET_NOTIFY);			/* 0x0373 AFX */
	caseconst(WM_SOCKET_DEAD);				/* 0x0374 AFX */
	caseconst(WM_POPMESSAGESTRING);			/* 0x0375 AFX */
	caseconst(WM_OCC_LOADFROMSTREAM);		/* 0x0376 AFX */
	caseconst(WM_OCC_LOADFROMSTORAGE);		/* 0x0377 AFX */
	caseconst(WM_OCC_INITNEW);				/* 0x0378 AFX */
	caseconst(WM_QUEUE_SENTINEL);			/* 0x0379 AFX */
	caseconst(WM_RESERVED_037A);			/* 0x037a AFX */
	caseconst(WM_RESERVED_037B);			/* 0x037b AFX */
	caseconst(WM_RESERVED_037C);			/* 0x037c AFX */
	caseconst(WM_RESERVED_037D);			/* 0x037d AFX */
	caseconst(WM_RESERVED_037E);			/* 0x037e AFX */
	caseconst(WM_RESERVED_037F);			/* 0x037f AFX */

	caseconst(WM_RCRESULT);					/* 0x0381 WM_PENWINFIRST+1 */
	caseconst(WM_HOOKRCRESULT);				/* 0x0382 WM_PENWINFIRST+2 */
	caseconst(WM_PENMISCINFO);				/* 0x0383 WM_PENWINFIRST+3 */
	caseconst(WM_SKB);						/* 0x0384 WM_PENWINFIRST+4 */
	caseconst(WM_PENCTL);					/* 0x0385 WM_PENWINFIRST+5 */
	caseconst(WM_PENMISC);					/* 0x0386 WM_PENWINFIRST+6 */
	caseconst(WM_CTLINIT);					/* 0x0387 WM_PENWINFIRST+7 */
	caseconst(WM_PENEVENT);					/* 0x0388 WM_PENWINFIRST+8 */

	caseconst(WM_DDE_INITIATE);				/* 0x03e0 WM_DDE_FIRST+0 */
	caseconst(WM_DDE_TERMINATE);			/* 0x03e1 WM_DDE_FIRST+1 */
	caseconst(WM_DDE_ADVISE);				/* 0x03e2 WM_DDE_FIRST+2 */
	caseconst(WM_DDE_UNADVISE);				/* 0x03e3 WM_DDE_FIRST+3 */
	caseconst(WM_DDE_ACK);					/* 0x03e4 WM_DDE_FIRST+4 */
	caseconst(WM_DDE_DATA);					/* 0x03e5 WM_DDE_FIRST+5 */
	caseconst(WM_DDE_REQUEST);				/* 0x03e6 WM_DDE_FIRST+6 */
	caseconst(WM_DDE_POKE);					/* 0x03e7 WM_DDE_FIRST+7 */
	caseconst(WM_DDE_EXECUTE);				/* 0x03e8 WM_DDE_FIRST+8 */

#if 0
	caseconst(WM_CHOOSEFONT_GETLOGFONT);	/* 0x0401 WM_USER+1 */
	caseconst(WM_CHOOSEFONT_SETLOGFONT);	/* 0x0465 WM_USER+101 */
	caseconst(WM_CHOOSEFONT_SETFLAGS);		/* 0x0466 WM_USER+102 */
#endif

	caseconst(WM_CPL_LAUNCH);				/* 0x07e8 WM_USER+1000 */
	caseconst(WM_CPL_LAUNCHED);				/* 0x07e9 WM_USER+1001 */
	
#if 0
	caseconst(WM_PSD_PAGESETUPDLG);			/* 0x0400 WM_USER+0 */
	caseconst(WM_PSD_FULLPAGERECT);			/* 0x0401 WM_USER+1 */
	caseconst(WM_PSD_MINMARGINRECT);		/* 0x0402 WM_USER+2 */
	caseconst(WM_PSD_MARGINRECT);			/* 0x0403 WM_USER+3 */
	caseconst(WM_PSD_GREEKTEXTRECT);		/* 0x0404 WM_USER+4 */
	caseconst(WM_PSD_ENVSTAMPRECT);			/* 0x0405 WM_USER+5 */
	caseconst(WM_PSD_YAFULLPAGERECT);		/* 0x0406 WM_USER+6 */
#endif
	}
	if (message >= WM_USER)
		sprintf(buf, "WM_USER+0x%04x", message - WM_USER);
	else
		sprintf(buf, "WM_$%04x", message);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

const char *win32debug_msg_name(UINT message)
{
	static char buf[40];
	return win32debug_msg_name1(message, buf);
}

/*** ---------------------------------------------------------------------- ***/

#if WIN32_DEBUG_MSGS

static const char *saveptr(const VOID *ptr, LPCSTR fmt, char *buf)
{
	checkptr(ptr, 0);
	sprintf(buf, fmt, ptr);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *wm_size(WPARAM wparam, char *pbuf)
{
	switch (wparam)
	{
		caseconst(SIZE_MAXHIDE);
		caseconst(SIZE_MAXIMIZED);
		caseconst(SIZE_MAXSHOW);
		caseconst(SIZE_MINIMIZED);
		caseconst(SIZE_RESTORED);
	}
	sprintf(pbuf, "%d", wparam);
	return pbuf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *hittest(WORD code, char *pbuf)
{
	switch ((int)code)
	{
		caseconst(HTERROR);
		caseconst(HTTRANSPARENT);
		caseconst(HTNOWHERE);
		caseconst(HTCLIENT);
		caseconst(HTCAPTION);
		caseconst(HTSYSMENU);
		caseconst(HTGROWBOX);
		caseconst(HTMENU);
		caseconst(HTHSCROLL);
		caseconst(HTVSCROLL);
		caseconst(HTREDUCE);
		caseconst(HTZOOM);
		caseconst(HTLEFT);
		caseconst(HTRIGHT);
		caseconst(HTTOP);
		caseconst(HTTOPLEFT);
		caseconst(HTTOPRIGHT);
		caseconst(HTBOTTOM);
		caseconst(HTBOTTOMLEFT);
		caseconst(HTBOTTOMRIGHT);
		caseconst(HTBORDER);
		caseconst(HTOBJECT);
		caseconst(HTCLOSE);
		caseconst(HTHELP);
	}
	sprintf(pbuf, "%d", code);
	return pbuf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *wm_sethotkey(BYTE key, char *buf)
{
	*buf = '\0';
	
	bit(key, HOTKEYF_ALT);
	bit(key, HOTKEYF_CONTROL);
	bit(key, HOTKEYF_EXT);
	bit(key, HOTKEYF_SHIFT);
	if (key) 
	{
		sprintf(buf + strlen(buf), "|$%02x", key);
	}
	if (*buf != '\0')
		buf++;
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *isc_show(DWORD show, char *buf)
{
#ifndef ISC_SHOWUISOFTKBD
#  define ISC_SHOWUISOFTKBD 0x20000000l
#endif

#define ISC_SHOWUICANDIDATEWINDOW0 ISC_SHOWUICANDIDATEWINDOW
#define ISC_SHOWUICANDIDATEWINDOW1 (ISC_SHOWUICANDIDATEWINDOW << 1)
#define ISC_SHOWUICANDIDATEWINDOW2 (ISC_SHOWUICANDIDATEWINDOW << 2)
#define ISC_SHOWUICANDIDATEWINDOW3 (ISC_SHOWUICANDIDATEWINDOW << 3)
	*buf = '\0';

	bit(show, ISC_SHOWUICOMPOSITIONWINDOW);
	bit(show, ISC_SHOWUIGUIDELINE);
	bit(show, ISC_SHOWUISOFTKBD);
	bit(show, ISC_SHOWUICANDIDATEWINDOW0);
	bit(show, ISC_SHOWUICANDIDATEWINDOW1);
	bit(show, ISC_SHOWUICANDIDATEWINDOW2);
	bit(show, ISC_SHOWUICANDIDATEWINDOW3);
	if (show) 
	{
		sprintf(buf + strlen(buf), "|$%08lx", show);
	}
	if (*buf != '\0')
		buf++;
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *vkey(WORD key, char *buf)
{
#ifndef VK_XBUTTON1
#  define VK_XBUTTON1 0x05
#endif
#ifndef VK_XBUTTON2
#  define VK_XBUTTON2 0x06
#endif
#ifndef VK_NAVIGATION_VIEW
#define VK_NAVIGATION_VIEW     0x88
#endif
#ifndef VK_NAVIGATION_MENU
#define VK_NAVIGATION_MENU     0x89
#endif
#ifndef VK_NAVIGATION_UP
#define VK_NAVIGATION_UP       0x8A
#endif
#ifndef VK_NAVIGATION_DOWN
#define VK_NAVIGATION_DOWN     0x8B
#endif
#ifndef VK_NAVIGATION_LEFT
#define VK_NAVIGATION_LEFT     0x8C
#endif
#ifndef VK_NAVIGATION_RIGHT
#define VK_NAVIGATION_RIGHT    0x8D
#endif
#ifndef VK_NAVIGATION_ACCEPT
#define VK_NAVIGATION_ACCEPT   0x8E
#endif
#ifndef VK_NAVIGATION_CANCEL
#define VK_NAVIGATION_CANCEL   0x8F
#endif
#ifndef VK_OEM_FJ_JISHO
#  define VK_OEM_FJ_JISHO 0x92
#endif
#ifndef VK_OEM_FJ_MASSHOU
#  define VK_OEM_FJ_MASSHOU 0x93
#endif
#ifndef VK_OEM_FJ_TOUROKU
#  define VK_OEM_FJ_TOUROKU 0x94
#endif
#ifndef VK_OEM_FJ_LOYA
#  define VK_OEM_FJ_LOYA 0x95
#endif
#ifndef VK_OEM_FJ_ROYA
#  define VK_OEM_FJ_ROYA 0x96
#endif
#ifndef VK_BROWSER_BACK
#  define VK_BROWSER_BACK 0xa6
#endif
#ifndef VK_BROWSER_FORWARD
#  define VK_BROWSER_FORWARD 0xa7
#endif
#ifndef VK_BROWSER_REFRESH
#  define VK_BROWSER_REFRESH 0xa8
#endif
#ifndef VK_BROWSER_STOP
#  define VK_BROWSER_STOP 0xa9
#endif
#ifndef VK_BROWSER_SEARCH
#  define VK_BROWSER_SEARCH 0xaa
#endif
#ifndef VK_BROWSER_FAVORITES
#  define VK_BROWSER_FAVORITES 0xab
#endif
#ifndef VK_BROWSER_HOME
#  define VK_BROWSER_HOME 0xac
#endif
#ifndef VK_VOLUME_MUTE
#  define VK_VOLUME_MUTE 0xad
#endif
#ifndef VK_VOLUME_DOWN
#  define VK_VOLUME_DOWN 0xae
#endif
#ifndef VK_VOLUME_UP
#  define VK_VOLUME_UP 0xaf
#endif
#ifndef VK_MEDIA_NEXT_TRACK
#  define VK_MEDIA_NEXT_TRACK 0xb0
#endif
#ifndef VK_MEDIA_PREV_TRACK
#  define VK_MEDIA_PREV_TRACK 0xb1
#endif
#ifndef VK_MEDIA_STOP
#  define VK_MEDIA_STOP 0xb2
#endif
#ifndef VK_MEDIA_PLAY_PAUSE
#  define VK_MEDIA_PLAY_PAUSE 0xb3
#endif
#ifndef VK_LAUNCH_MAIL
#  define VK_LAUNCH_MAIL 0xb4
#endif
#ifndef VK_LAUNCH_MEDIA_SELECT
#  define VK_LAUNCH_MEDIA_SELECT 0xb5
#endif
#ifndef VK_LAUNCH_APP1
#  define VK_LAUNCH_APP1 0xb6
#endif
#ifndef VK_LAUNCH_APP2
#  define VK_LAUNCH_APP2 0xb7
#endif
#ifndef VK_OEM_PLUS
#  define VK_OEM_PLUS 0xbb
#endif
#ifndef VK_OEM_COMMA
#  define VK_OEM_COMMA 0xbc
#endif
#ifndef VK_OEM_MINUS
#  define VK_OEM_MINUS 0xbd
#endif
#ifndef VK_OEM_PERIOD
#  define VK_OEM_PERIOD 0xbe
#endif
#ifndef VK_OEM_2
#  define VK_OEM_2 0xbf
#endif
#ifndef VK_OEM_3
#  define VK_OEM_3 0xc0
#endif
#ifndef VK_GAMEPAD_A
#define VK_GAMEPAD_A                         0xC3
#endif
#ifndef VK_GAMEPAD_B
#define VK_GAMEPAD_B                         0xC4
#endif
#ifndef VK_GAMEPAD_X
#define VK_GAMEPAD_X                         0xC5
#endif
#ifndef VK_GAMEPAD_Y
#define VK_GAMEPAD_Y                         0xC6
#endif
#ifndef VK_GAMEPAD_RIGHT_SHOULDER
#define VK_GAMEPAD_RIGHT_SHOULDER            0xC7
#endif
#ifndef VK_GAMEPAD_LEFT_SHOULDER
#define VK_GAMEPAD_LEFT_SHOULDER             0xC8
#endif
#ifndef VK_GAMEPAD_LEFT_TRIGGER
#define VK_GAMEPAD_LEFT_TRIGGER              0xC9
#endif
#ifndef VK_GAMEPAD_RIGHT_TRIGGER
#define VK_GAMEPAD_RIGHT_TRIGGER             0xCA
#endif
#ifndef VK_GAMEPAD_DPAD_UP
#define VK_GAMEPAD_DPAD_UP                   0xCB
#endif
#ifndef VK_GAMEPAD_DPAD_DOWN
#define VK_GAMEPAD_DPAD_DOWN                 0xCC
#endif
#ifndef VK_GAMEPAD_DPAD_LEFT
#define VK_GAMEPAD_DPAD_LEFT                 0xCD
#endif
#ifndef VK_GAMEPAD_DPAD_RIGHT
#define VK_GAMEPAD_DPAD_RIGHT                0xCE
#endif
#ifndef VK_GAMEPAD_MENU
#define VK_GAMEPAD_MENU                      0xCF
#endif
#ifndef VK_GAMEPAD_VIEW
#define VK_GAMEPAD_VIEW                      0xD0
#endif
#ifndef VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON
#define VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON    0xD1
#endif
#ifndef VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON
#define VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON   0xD2
#endif
#ifndef VK_GAMEPAD_LEFT_THUMBSTICK_UP
#define VK_GAMEPAD_LEFT_THUMBSTICK_UP        0xD3
#endif
#ifndef VK_GAMEPAD_LEFT_THUMBSTICK_DOWN
#define VK_GAMEPAD_LEFT_THUMBSTICK_DOWN      0xD4
#endif
#ifndef VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT
#define VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT     0xD5
#endif
#ifndef VK_GAMEPAD_LEFT_THUMBSTICK_LEFT
#define VK_GAMEPAD_LEFT_THUMBSTICK_LEFT      0xD6
#endif
#ifndef VK_GAMEPAD_RIGHT_THUMBSTICK_UP
#define VK_GAMEPAD_RIGHT_THUMBSTICK_UP       0xD7
#endif
#ifndef VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN
#define VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN     0xD8
#endif
#ifndef VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT
#define VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT    0xD9
#endif
#ifndef VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT
#define VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT     0xDA
#endif
#ifndef VK_OEM_AX
#  define VK_OEM_AX 0xe1
#endif
#ifndef VK_OEM_102
#  define VK_OEM_102 0xe2
#endif
#ifndef VK_ICO_HELP
#  define VK_ICO_HELP 0xe3
#endif
#ifndef VK_ICO_00
#  define VK_ICO_00 0xe4
#endif
#ifndef VK_ICO_CLEAR
#  define VK_ICO_CLEAR 0xe6
#endif
#ifndef VK_PACKET
#  define VK_PACKET 0xe7
#endif
#ifndef VK_OEM_RESET
#  define VK_OEM_RESET 0xe9
#endif
#ifndef VK_OEM_JUMP
#  define VK_OEM_JUMP 0xea
#endif
#ifndef VK_OEM_PA1
#  define VK_OEM_PA1 0xeb
#endif
#ifndef VK_OEM_PA2
#  define VK_OEM_PA2 0xec
#endif
#ifndef VK_OEM_PA3
#  define VK_OEM_PA3 0xed
#endif
#ifndef VK_OEM_WSCTRL
#  define VK_OEM_WSCTRL 0xee
#endif
#ifndef VK_OEM_CUSEL
#  define VK_OEM_CUSEL 0xef
#endif
#ifndef VK_OEM_ATTN
#  define VK_OEM_ATTN 0xf0
#endif
#ifndef VK_OEM_FINISH
#  define VK_OEM_FINISH 0xf1
#endif
#ifndef VK_OEM_COPY
#  define VK_OEM_COPY 0xf2
#endif
#ifndef VK_OEM_AUTO
#  define VK_OEM_AUTO 0xf3
#endif
#ifndef VK_OEM_ENLW
#  define VK_OEM_ENLW 0xf4
#endif
#ifndef VK_OEM_BACKTAB
#  define VK_OEM_BACKTAB 0xf5
#endif
	switch (key)
	{
		caseconst(VK_LBUTTON);
		caseconst(VK_RBUTTON);
		caseconst(VK_CANCEL);
		caseconst(VK_MBUTTON);
		caseconst(VK_XBUTTON1);
		caseconst(VK_XBUTTON2);
		caseconst(VK_BACK);
		caseconst(VK_TAB);
		caseconst(VK_CLEAR);
		caseconst(VK_RETURN);
		caseconst(VK_SHIFT);
		caseconst(VK_CONTROL);
		caseconst(VK_MENU);
		caseconst(VK_PAUSE);
		caseconst(VK_CAPITAL);
		caseconst(VK_KANA);
		caseconst(VK_JUNJA);
		caseconst(VK_FINAL);
		caseconst(VK_HANJA);
		caseconst(VK_ESCAPE);
		caseconst(VK_CONVERT);
		caseconst(VK_NONCONVERT);
		caseconst(VK_ACCEPT);
		caseconst(VK_MODECHANGE);
		caseconst(VK_SPACE);
		caseconst(VK_PRIOR);
		caseconst(VK_NEXT);
		caseconst(VK_END);
		caseconst(VK_HOME);
		caseconst(VK_LEFT);
		caseconst(VK_UP);
		caseconst(VK_RIGHT);
		caseconst(VK_DOWN);
		caseconst(VK_SELECT);
		caseconst(VK_PRINT);
		caseconst(VK_EXECUTE);
		caseconst(VK_SNAPSHOT);
		caseconst(VK_INSERT);
		caseconst(VK_DELETE);
		caseconst(VK_HELP);
		caseconst(VK_LWIN);
		caseconst(VK_RWIN);
		caseconst(VK_APPS);
		caseconst(VK_SLEEP);
		caseconst(VK_NUMPAD0);
		caseconst(VK_NUMPAD1);
		caseconst(VK_NUMPAD2);
		caseconst(VK_NUMPAD3);
		caseconst(VK_NUMPAD4);
		caseconst(VK_NUMPAD5);
		caseconst(VK_NUMPAD6);
		caseconst(VK_NUMPAD7);
		caseconst(VK_NUMPAD8);
		caseconst(VK_NUMPAD9);
		caseconst(VK_MULTIPLY);
		caseconst(VK_ADD);
		caseconst(VK_SEPARATOR);
		caseconst(VK_SUBTRACT);
		caseconst(VK_DECIMAL);
		caseconst(VK_DIVIDE);
		caseconst(VK_F1);
		caseconst(VK_F2);
		caseconst(VK_F3);
		caseconst(VK_F4);
		caseconst(VK_F5);
		caseconst(VK_F6);
		caseconst(VK_F7);
		caseconst(VK_F8);
		caseconst(VK_F9);
		caseconst(VK_F10);
		caseconst(VK_F11);
		caseconst(VK_F12);
		caseconst(VK_F13);
		caseconst(VK_F14);
		caseconst(VK_F15);
		caseconst(VK_F16);
		caseconst(VK_F17);
		caseconst(VK_F18);
		caseconst(VK_F19);
		caseconst(VK_F20);
		caseconst(VK_F21);
		caseconst(VK_F22);
		caseconst(VK_F23);
		caseconst(VK_F24);
		caseconst(VK_NAVIGATION_VIEW);
		caseconst(VK_NAVIGATION_MENU);
		caseconst(VK_NAVIGATION_UP);
		caseconst(VK_NAVIGATION_DOWN);
		caseconst(VK_NAVIGATION_LEFT);
		caseconst(VK_NAVIGATION_RIGHT);
		caseconst(VK_NAVIGATION_ACCEPT);
		caseconst(VK_NAVIGATION_CANCEL);
		caseconst(VK_NUMLOCK);
		caseconst(VK_SCROLL);
		caseconst(VK_OEM_FJ_JISHO);
		caseconst(VK_OEM_FJ_MASSHOU);
		caseconst(VK_OEM_FJ_TOUROKU);
		caseconst(VK_OEM_FJ_LOYA);
		caseconst(VK_OEM_FJ_ROYA);
		caseconst(VK_LSHIFT);
		caseconst(VK_RSHIFT);
		caseconst(VK_LCONTROL);
		caseconst(VK_RCONTROL);
		caseconst(VK_LMENU);
		caseconst(VK_RMENU);
		caseconst(VK_BROWSER_BACK);
		caseconst(VK_BROWSER_FORWARD);
		caseconst(VK_BROWSER_REFRESH);
		caseconst(VK_BROWSER_STOP);
		caseconst(VK_BROWSER_SEARCH);
		caseconst(VK_BROWSER_FAVORITES);
		caseconst(VK_BROWSER_HOME);
		caseconst(VK_VOLUME_MUTE);
		caseconst(VK_VOLUME_DOWN);
		caseconst(VK_VOLUME_UP);
		caseconst(VK_MEDIA_NEXT_TRACK);
		caseconst(VK_MEDIA_PREV_TRACK);
		caseconst(VK_MEDIA_STOP);
		caseconst(VK_MEDIA_PLAY_PAUSE);
		caseconst(VK_LAUNCH_MAIL);
		caseconst(VK_LAUNCH_MEDIA_SELECT);
		caseconst(VK_LAUNCH_APP1);
		caseconst(VK_LAUNCH_APP2);
		caseconst(VK_OEM_1);
		caseconst(VK_OEM_PLUS);
		caseconst(VK_OEM_COMMA);
		caseconst(VK_OEM_MINUS);
		caseconst(VK_OEM_PERIOD);
		caseconst(VK_OEM_2);
		caseconst(VK_OEM_3);
		caseconst(VK_GAMEPAD_A);
		caseconst(VK_GAMEPAD_B);
		caseconst(VK_GAMEPAD_X);
		caseconst(VK_GAMEPAD_Y);
		caseconst(VK_GAMEPAD_RIGHT_SHOULDER);
		caseconst(VK_GAMEPAD_LEFT_SHOULDER);
		caseconst(VK_GAMEPAD_LEFT_TRIGGER);
		caseconst(VK_GAMEPAD_RIGHT_TRIGGER);
		caseconst(VK_GAMEPAD_DPAD_UP);
		caseconst(VK_GAMEPAD_DPAD_DOWN);
		caseconst(VK_GAMEPAD_DPAD_LEFT);
		caseconst(VK_GAMEPAD_DPAD_RIGHT);
		caseconst(VK_GAMEPAD_MENU);
		caseconst(VK_GAMEPAD_VIEW);
		caseconst(VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON);
		caseconst(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON);
		caseconst(VK_GAMEPAD_LEFT_THUMBSTICK_UP);
		caseconst(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN);
		caseconst(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT);
		caseconst(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT);
		caseconst(VK_GAMEPAD_RIGHT_THUMBSTICK_UP);
		caseconst(VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN);
		caseconst(VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT);
		caseconst(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT);
		caseconst(VK_OEM_4);
		caseconst(VK_OEM_5);
		caseconst(VK_OEM_6);
		caseconst(VK_OEM_7);
		caseconst(VK_OEM_8);
		caseconst(VK_OEM_AX);
		caseconst(VK_OEM_102);
		caseconst(VK_ICO_HELP);
		caseconst(VK_ICO_00);
		caseconst(VK_PROCESSKEY);
		caseconst(VK_ICO_CLEAR);
		caseconst(VK_OEM_RESET);
		caseconst(VK_OEM_JUMP);
		caseconst(VK_OEM_PA1);
		caseconst(VK_OEM_PA2);
		caseconst(VK_OEM_PA3);
		caseconst(VK_OEM_WSCTRL);
		caseconst(VK_OEM_CUSEL);
		caseconst(VK_OEM_ATTN);
		caseconst(VK_OEM_FINISH);
		caseconst(VK_OEM_COPY);
		caseconst(VK_OEM_AUTO);
		caseconst(VK_OEM_ENLW);
		caseconst(VK_OEM_BACKTAB);
		caseconst(VK_ATTN);
		caseconst(VK_CRSEL);
		caseconst(VK_EXSEL);
		caseconst(VK_EREOF);
		caseconst(VK_PLAY);
		caseconst(VK_ZOOM);
		caseconst(VK_NONAME);
		caseconst(VK_PA1);
		caseconst(VK_OEM_CLEAR);
	}
	if (key >= '0' && key <= '9')
		sprintf(buf, "%c", key);
	else if (key >= 'A' && key <= 'Z')
		sprintf(buf, "%c", key);
	else
		sprintf(buf, "VK_$%04x", key);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *handle(HANDLE h, char *buf)
{
	checkptr(h, 0);
	sprintf(buf, "$%08x", (UINT)h);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *activate(WORD factive, char *buf)
{
	switch (factive)
	{
		caseconst(WA_INACTIVE);
		caseconst(WA_ACTIVE);
		caseconst(WA_CLICKACTIVE);
	}
	sprintf(buf, "%d", factive);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *imn_command(int command, char *buf)
{
	switch (command)
	{
		caseconst(IMN_CLOSESTATUSWINDOW);
		caseconst(IMN_OPENSTATUSWINDOW);
		caseconst(IMN_CHANGECANDIDATE);
		caseconst(IMN_CLOSECANDIDATE);
		caseconst(IMN_OPENCANDIDATE);
		caseconst(IMN_SETCONVERSIONMODE);
		caseconst(IMN_SETSENTENCEMODE);
		caseconst(IMN_SETOPENSTATUS);
		caseconst(IMN_SETCANDIDATEPOS);
		caseconst(IMN_SETCOMPOSITIONFONT);
		caseconst(IMN_SETCOMPOSITIONWINDOW);
		caseconst(IMN_SETSTATUSWINDOWPOS);
		caseconst(IMN_GUIDELINE);
		caseconst(IMN_PRIVATE);
	}
	sprintf(buf, "IMN_%d", command);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *imc_command(int command, char *buf)
{
	switch (command)
	{
		caseconst(IMC_GETCANDIDATEPOS);
		caseconst(IMC_SETCANDIDATEPOS);
		caseconst(IMC_GETCOMPOSITIONFONT);
		caseconst(IMC_SETCOMPOSITIONFONT);
		caseconst(IMC_GETCOMPOSITIONWINDOW);
		caseconst(IMC_SETCOMPOSITIONWINDOW);
		caseconst(IMC_GETSTATUSWINDOWPOS);
		caseconst(IMC_SETSTATUSWINDOWPOS);
		caseconst(IMC_CLOSESTATUSWINDOW);
		caseconst(IMC_OPENSTATUSWINDOW);
	}
	sprintf(buf, "IMC_%d", command);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *syscommand(int command, char *buf)
{
#ifndef SC_DEFAULT
#  define SC_DEFAULT 0xf160
#endif
#ifndef SC_MONITORPOWER
#  define SC_MONITORPOWER 0xf170
#endif
#ifndef SC_CONTEXTHELP
#  define SC_CONTEXTHELP 0xf180
#endif
#ifndef SC_SEPARATOR
#  define SC_SEPARATOR 0xf00f
#endif

	switch (command)
	{
		caseconst(SC_SIZE);
		caseconst(SC_MOVE);
		caseconst(SC_MINIMIZE);
		caseconst(SC_MAXIMIZE);
		caseconst(SC_NEXTWINDOW);
		caseconst(SC_PREVWINDOW);
		caseconst(SC_CLOSE);
		caseconst(SC_VSCROLL);
		caseconst(SC_HSCROLL);
		caseconst(SC_MOUSEMENU);
		caseconst(SC_KEYMENU);
		caseconst(SC_ARRANGE);
		caseconst(SC_RESTORE);
		caseconst(SC_TASKLIST);
		caseconst(SC_SCREENSAVE);
		caseconst(SC_HOTKEY);
		caseconst(SC_DEFAULT);
		caseconst(SC_MONITORPOWER);
		caseconst(SC_CONTEXTHELP);
		caseconst(SC_SEPARATOR);
	}
	sprintf(buf, "SC_$%04x", command);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *vscrollcode(int code, char *buf)
{
	switch (code)
	{
		caseconst(SB_BOTTOM);
		caseconst(SB_ENDSCROLL);
		caseconst(SB_LINEDOWN);
		caseconst(SB_LINEUP);
		caseconst(SB_PAGEDOWN);
		caseconst(SB_PAGEUP);
		caseconst(SB_THUMBPOSITION);
		caseconst(SB_THUMBTRACK);
		caseconst(SB_TOP);
	}
	sprintf(buf, "SB_%d", code);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *hscrollcode(int code, char *buf)
{
	switch (code)
	{
		caseconst(SB_LEFT);
		caseconst(SB_ENDSCROLL);
		caseconst(SB_LINELEFT);
		caseconst(SB_LINERIGHT);
		caseconst(SB_PAGELEFT);
		caseconst(SB_PAGERIGHT);
		caseconst(SB_THUMBPOSITION);
		caseconst(SB_THUMBTRACK);
		caseconst(SB_RIGHT);
	}
	sprintf(buf, "SB_%d", code);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *windowside(int code, char *buf)
{
	switch (code)
	{
		caseconst(WMSZ_LEFT);
		caseconst(WMSZ_RIGHT);
		caseconst(WMSZ_TOP);
		caseconst(WMSZ_TOPLEFT);
		caseconst(WMSZ_TOPRIGHT);
		caseconst(WMSZ_BOTTOM);
		caseconst(WMSZ_BOTTOMLEFT);
		caseconst(WMSZ_BOTTOMRIGHT);
	}
	sprintf(buf, "WMSZ_%d", code);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *menuflags(DWORD flags, char *buf)
{
	*buf = '\0';
	ebit(flags, MF_BITMAP, MF_STRING);
	bit(flags, MF_OWNERDRAW);
	ebit(flags, MF_CHECKED, MF_UNCHECKED);
	bit(flags, MF_USECHECKBITMAPS);
	ebit2(flags, MF_GRAYED, MF_DISABLED, MF_ENABLED);
	ebit(flags, MF_HILITE, MF_UNHILITE);
	bit(flags, MF_MOUSESELECT);
	bit(flags, MF_OWNERDRAW);
	bit(flags, MF_POPUP);
	bit(flags, MF_MENUBARBREAK);
	bit(flags, MF_MENUBREAK);
	bit(flags, MF_SYSMENU);
	bit(flags, MF_DEFAULT);
	bit(flags, MF_HELP);
	bit(flags, MF_MOUSESELECT);
	bit(flags, MF_RIGHTJUSTIFY);
	bit(flags, MF_END);
	if (flags)
	{
		sprintf(buf + strlen(buf), "|$%08lx", flags);
	}
	if (*buf != '\0')
		buf++;
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *powerbroadcast(int code, char *buf)
{
#ifndef PBT_POWERSETTINGCHANGE
#define PBT_POWERSETTINGCHANGE          0x8013
#endif
	switch (code)
	{
		caseconst(PBT_APMQUERYSUSPEND);
		caseconst(PBT_APMQUERYSTANDBY);
		caseconst(PBT_APMQUERYSUSPENDFAILED);
		caseconst(PBT_APMQUERYSTANDBYFAILED);
		caseconst(PBT_APMSUSPEND);
		caseconst(PBT_APMSTANDBY);
		caseconst(PBT_APMRESUMECRITICAL);
		caseconst(PBT_APMRESUMESUSPEND);
		caseconst(PBT_APMRESUMESTANDBY);
		caseconst(PBT_APMBATTERYLOW);
		caseconst(PBT_APMPOWERSTATUSCHANGE);
		caseconst(PBT_APMOEMEVENT);
		caseconst(PBT_APMRESUMEAUTOMATIC);
		caseconst(PBT_POWERSETTINGCHANGE);
	}
	sprintf(buf, "PBT_%d", code);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *devicechange(int code, char *buf)
{
#ifndef DBT_CUSTOMEVENT
#  define DBT_CUSTOMEVENT 0x8006
#endif
#ifndef DBT_VPOWERDAPI
#  define DBT_VPOWERDAPI 0x8100
#endif
	switch (code)
	{
		caseconst(DBT_APPYBEGIN);
		caseconst(DBT_APPYEND);
		caseconst(DBT_DEVNODES_CHANGED);
		caseconst(DBT_QUERYCHANGECONFIG);
		caseconst(DBT_CONFIGCHANGED);
		caseconst(DBT_CONFIGCHANGECANCELED);
		caseconst(DBT_MONITORCHANGE);
		caseconst(DBT_SHELLLOGGEDON);
		caseconst(DBT_CONFIGMGAPI32);
		caseconst(DBT_VXDINITCOMPLETE);
		caseconst(DBT_DEVICEARRIVAL);
		caseconst(DBT_DEVICEQUERYREMOVE);
		caseconst(DBT_DEVICEQUERYREMOVEFAILED);
		caseconst(DBT_DEVICEREMOVEPENDING);
		caseconst(DBT_DEVICEREMOVECOMPLETE);
		caseconst(DBT_DEVICETYPESPECIFIC);
		caseconst(DBT_CUSTOMEVENT);
		caseconst(DBT_VOLLOCKQUERYLOCK);
		caseconst(DBT_VOLLOCKLOCKTAKEN);
		caseconst(DBT_VOLLOCKLOCKFAILED);
		caseconst(DBT_VOLLOCKQUERYUNLOCK);
		caseconst(DBT_VOLLOCKLOCKRELEASED);
		caseconst(DBT_VOLLOCKUNLOCKFAILED);
		caseconst(DBT_VPOWERDAPI);
		caseconst(DBT_USERDEFINED);
	}
	sprintf(buf, "DBT_%d", code);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *msg_source(UINT source, char *buf)
{
	switch (source)
	{
		caseconst(MSGF_DIALOGBOX);
		caseconst(MSGF_MESSAGEBOX);
		caseconst(MSGF_MENU);
		caseconst(MSGF_MOVE);
		caseconst(MSGF_SIZE);
		caseconst(MSGF_SCROLLBAR);
		caseconst(MSGF_MAINLOOP);
		caseconst(MSGF_NEXTWINDOW);
		caseconst(MSGF_COMMCTRL_BEGINDRAG);
		caseconst(MSGF_COMMCTRL_SIZEHEADER);
		caseconst(MSGF_COMMCTRL_DRAGSELECT);
		caseconst(MSGF_COMMCTRL_TOOLBARCUST);
		caseconst(MSGF_DDEMGR);
	}
	if (source >= MSGF_USER)
		sprintf(buf, "MSGF_USER+%d", source - MSGF_USER);
	else
		sprintf(buf, "MSGF_%d", source);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *vkeyflags(UINT flags, char *buf)
{
	*buf = '\0';

#ifndef MK_XBUTTON1
#define MK_XBUTTON1         0x0020
#endif
#ifndef MK_XBUTTON2
#define MK_XBUTTON2         0x0040
#endif
	bit(flags, MK_LBUTTON);
	bit(flags, MK_RBUTTON);
	bit(flags, MK_SHIFT);
	bit(flags, MK_CONTROL);
	bit(flags, MK_MBUTTON);
	bit(flags, MK_XBUTTON1);
	bit(flags, MK_XBUTTON2);
	if (flags)
	{
		sprintf(buf + strlen(buf), "|$%04x", flags);
	}
	if (*buf != '\0')
		buf++;
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *windowstyle(DWORD style, char *buf, int type)
{
	*buf = '\0';
#define ODT_MDICLIENT 100
#define ODT_DIALOG    101

#ifndef DS_USEPIXELS
#define DS_USEPIXELS        0x8000L
#endif

	bit(style, WS_OVERLAPPEDWINDOW);
	bit(style, WS_POPUPWINDOW);
	ebit(style, WS_POPUP, WS_OVERLAPPED);
	bit(style, WS_CHILD);
	bit(style, WS_MINIMIZE);
	bit(style, WS_VISIBLE);
	bit(style, WS_DISABLED);
	bit(style, WS_CLIPSIBLINGS);
	bit(style, WS_CLIPCHILDREN);
	bit(style, WS_MAXIMIZE);
	bit(style, WS_CAPTION);
	bit(style, WS_BORDER);
	bit(style, WS_DLGFRAME);
	bit(style, WS_VSCROLL);
	bit(style, WS_HSCROLL);
	bit(style, WS_SYSMENU);
	bit(style, WS_THICKFRAME);
	bit(style, WS_MINIMIZEBOX);
	bit(style, WS_MAXIMIZEBOX);
	switch (type)
	{
	case ODT_MDICLIENT:
		bit(style, MDIS_ALLCHILDSTYLES);
		break;
	case ODT_DIALOG:
		bit(style, DS_ABSALIGN);
		bit(style, DS_SYSMODAL);
		bit(style, DS_LOCALEDIT);
		bit(style, DS_SETFONT);
		bit(style, DS_MODALFRAME);
		bit(style, DS_NOIDLEMSG);
		bit(style, DS_SETFOREGROUND);
		bit(style, DS_3DLOOK);
		bit(style, DS_FIXEDSYS);
		bit(style, DS_NOFAILCREATE);
		bit(style, DS_CONTROL);
		bit(style, DS_CENTER);
		bit(style, DS_CENTERMOUSE);
		bit(style, DS_CONTEXTHELP);
		bit(style, DS_USEPIXELS);
		bit(style, WS_GROUP);
		bit(style, WS_TABSTOP);
		break;
	default:
		bit(style, WS_MINIMIZEBOX);
		bit(style, WS_MAXIMIZEBOX);
		break;
	}
	if (style) 
	{
		sprintf(buf + strlen(buf), "|$%08lx", style);
	}
	if (*buf != '\0')
		buf++;
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *win32debug_windowexstyle(DWORD style, char *buf)
{
	*buf = '\0';


#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED           0x00080000
#endif
#ifndef WS_EX_NOINHERITLAYOUT
#define WS_EX_NOINHERITLAYOUT   0x00100000L // Disable inheritence of mirroring by children
#endif
#ifndef WS_EX_NOREDIRECTIONBITMAP
#define WS_EX_NOREDIRECTIONBITMAP 0x00200000L
#endif
#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL         0x00400000L // Right to left mirroring
#endif
#ifndef WS_EX_COMPOSITED
#define WS_EX_COMPOSITED        0x02000000L
#endif
#ifndef WS_EX_NOACTIVATE
#define WS_EX_NOACTIVATE        0x08000000L
#endif

	bit(style, WS_EX_OVERLAPPEDWINDOW);
	bit(style, WS_EX_PALETTEWINDOW);
	bit(style, WS_EX_DLGMODALFRAME);
	bit(style, WS_EX_NOPARENTNOTIFY);
	bit(style, WS_EX_TOPMOST);
	bit(style, WS_EX_ACCEPTFILES);
	bit(style, WS_EX_TRANSPARENT);
	bit(style, WS_EX_MDICHILD);
	bit(style, WS_EX_TOOLWINDOW);
	bit(style, WS_EX_WINDOWEDGE);
	bit(style, WS_EX_CLIENTEDGE);
	bit(style, WS_EX_CONTEXTHELP);
	bit(style, WS_EX_RIGHT);
	bit(style, WS_EX_LEFT);
	bit(style, WS_EX_RTLREADING);
	bit(style, WS_EX_LTRREADING);
	bit(style, WS_EX_LEFTSCROLLBAR);
	bit(style, WS_EX_RIGHTSCROLLBAR);
	bit(style, WS_EX_CONTROLPARENT);
	bit(style, WS_EX_STATICEDGE);
	bit(style, WS_EX_APPWINDOW);
	bit(style, WS_EX_LAYERED);
	bit(style, WS_EX_NOINHERITLAYOUT);
	bit(style, WS_EX_NOREDIRECTIONBITMAP);
	bit(style, WS_EX_LAYOUTRTL);
	bit(style, WS_EX_COMPOSITED);
	bit(style, WS_EX_NOACTIVATE);
	if (style) 
	{
		sprintf(buf + strlen(buf), "|$%08lx", style);
	}
	if (*buf != '\0')
		buf++;
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *mditile(UINT code, char *buf)
{
	*buf = '\0';
	
#ifndef MDITILE_ZORDER
#define MDITILE_ZORDER         0x0004
#endif

	ebit(code, MDITILE_HORIZONTAL, MDITILE_VERTICAL);
	bit(code, MDITILE_SKIPDISABLED);
	bit(code, MDITILE_ZORDER);
	if (code) 
	{
		sprintf(buf + strlen(buf), "|$%x", code);
	}
	if (*buf != '\0')
		buf++;
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *clipformat(UINT format, char *buf)
{
	switch (format)
	{
		case 0: return "NULL";
		caseconst(CF_TEXT);
		caseconst(CF_BITMAP);
		caseconst(CF_METAFILEPICT);
		caseconst(CF_SYLK);
		caseconst(CF_DIF);
		caseconst(CF_TIFF);
		caseconst(CF_OEMTEXT);
		caseconst(CF_DIB);
		caseconst(CF_PALETTE);
		caseconst(CF_PENDATA);
		caseconst(CF_RIFF);
		caseconst(CF_WAVE);
		caseconst(CF_UNICODETEXT);
		caseconst(CF_ENHMETAFILE);
		caseconst(CF_HDROP);
		caseconst(CF_LOCALE);
		caseconst(CF_OWNERDISPLAY);
		caseconst(CF_DSPTEXT);
		caseconst(CF_DSPBITMAP);
		caseconst(CF_DSPMETAFILEPICT);
		caseconst(CF_DSPENHMETAFILE);
	}
	if (format >= CF_PRIVATEFIRST && format <= CF_PRIVATELAST)
	{
		sprintf(buf, "CF_PRIVATEFIRST+%d", format - CF_PRIVATEFIRST);
	} else if (format >= CF_GDIOBJFIRST && format <= CF_GDIOBJLAST)
	{
		sprintf(buf, "CF_GDIOBJFIRST+%d", format - CF_GDIOBJFIRST);
	} else
	{
		sprintf(buf, "CF_%d", format);
	}
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *wm_hotkey(UINT code, char *buf)
{
	switch (code)
	{
		caseconst(IDHOT_SNAPWINDOW);
		caseconst(IDHOT_SNAPDESKTOP);
	}
	sprintf(buf, "0x%04x", code);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *ctltype(UINT code, char *buf)
{
	switch (code)
	{
		caseconst(CTLCOLOR_BTN);
		caseconst(CTLCOLOR_DLG);
		caseconst(CTLCOLOR_EDIT);
		caseconst(CTLCOLOR_LISTBOX);
		caseconst(CTLCOLOR_MSGBOX);
		caseconst(CTLCOLOR_SCROLLBAR);
		caseconst(CTLCOLOR_STATIC);
	}
	sprintf(buf, "0x%04x", code);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *dde_param(UINT message, LPARAM param, char *buf)
{
	UINT low, high;
	
	UNUSED(message);
	low = LOWORD(param);
	high = HIWORD(param);
	sprintf(buf, "$%04x, $%04x", low, high);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *wm_print(DWORD flags, char *buf)
{
	*buf = '\0';
	bit(flags, PRF_CHECKVISIBLE);
	bit(flags, PRF_NONCLIENT);
	bit(flags, PRF_CLIENT);
	bit(flags, PRF_ERASEBKGND);
	bit(flags, PRF_CHILDREN);
	bit(flags, PRF_OWNED);
	if (flags) 
	{
		sprintf(buf + strlen(buf), "|PRF_$%08lx", flags);
	}
	if (*buf != '\0')
		buf++;
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *buttonstyle(UINT style, char *buf)
{
#define bsstyle(name) \
	case name: strcat(buf, #name)
#ifndef BS_TYPEMASK
#define BS_TYPEMASK         0x0000000FL
#endif
#ifndef BS_SPLITBUTTON
#  define BS_SPLITBUTTON   0xC
#endif
#ifndef BS_DEFSPLITBUTTON
#  define BS_DEFSPLITBUTTON 0xD
#endif
#ifndef BS_COMMANDLINK
#  define BS_COMMANDLINK 0xE
#endif
#ifndef BS_DEFCOMMANDLINK
#  define BS_DEFCOMMANDLINK 0xF
#endif

	*buf = '\0';
	switch (style & BS_TYPEMASK)
	{
		bsstyle(BS_PUSHBUTTON);
		bsstyle(BS_DEFPUSHBUTTON);
		bsstyle(BS_CHECKBOX);
		bsstyle(BS_AUTOCHECKBOX);
		bsstyle(BS_RADIOBUTTON);
		bsstyle(BS_3STATE);
		bsstyle(BS_AUTO3STATE);
		bsstyle(BS_GROUPBOX);
		bsstyle(BS_USERBUTTON);
		bsstyle(BS_AUTORADIOBUTTON);
		bsstyle(BS_OWNERDRAW);
		bsstyle(BS_SPLITBUTTON);
		bsstyle(BS_DEFSPLITBUTTON);
		bsstyle(BS_COMMANDLINK);
		bsstyle(BS_DEFCOMMANDLINK);
	default:
		sprintf(buf + strlen(buf), "BS_$%04x", (unsigned int)(style & BS_TYPEMASK));
		break;
	}
	style &= ~BS_TYPEMASK;
	bit(style, BS_LEFTTEXT);
	bit(style, BS_ICON);
	bit(style, BS_BITMAP);
	bit(style, BS_CENTER);
	bit(style, BS_LEFT);
	bit(style, BS_RIGHT);
	bit(style, BS_VCENTER);
	bit(style, BS_TOP);
	bit(style, BS_BOTTOM);
	bit(style, BS_PUSHLIKE);
	bit(style, BS_MULTILINE);
	bit(style, BS_NOTIFY);
	bit(style, BS_FLAT);
	if (style)
	{
		sprintf(buf + strlen(buf), "BS_$%04x", style);
	}
	if (*buf != '\0')
		buf++;
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *rectangle(LPRECT rc, char *buf)
{
	checkptr(rc, sizeof(*rc));
	sprintf(buf, "{%ld, %ld, %ld, %ld}", (LONG)rc->left, (LONG)rc->top, (LONG)rc->right, (LONG)rc->bottom);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *mdicreatestruct(LPMDICREATESTRUCT s, char *buf)
{
	char buf2[80];
	char buf3[512];
	
	checkptr(s, sizeof(*s));
	sprintf(buf, "{ %s, %s, %s, %d, %d, %d, %d, %s, $%08lx }",
		s->szClass,
		s->szTitle,
		handle(s->hOwner, buf2),
		s->x, s->y, s->cx, s->cy,
		windowstyle(s->style, buf3, ODT_MDICLIENT),
		s->lParam);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

static const char *createstruct(LPCREATESTRUCT s, char *buf, int type)
{
	char buf1[20];
	char buf2[20];
	char buf3[20];
	char buf4[512];
	char buf5[80];
	char buf7[512];
	
	checkptr(s, sizeof(*s));
	sprintf(buf, "{ $%08lx, %s, %s, %s, %d, %d, %d, %d, %s, %s, $%08lx, %s }",
		(DWORD)s->lpCreateParams,
		handle(s->hInstance, buf1),
		handle(s->hMenu, buf2),
		handle(s->hwndParent, buf3),
		s->x, s->y, s->cx, s->cy,
		windowstyle(s->style, buf4, type),
		saveptr(s->lpszName, "%s", buf5),
		(DWORD)s->lpszClass,
		win32debug_windowexstyle(s->dwExStyle, buf7));
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

#define WM_ACTIVATE_PARAMS(factive, minimized, hwnd) \
	factive = LOWORD(wparam); \
	minimized = (BOOL)HIWORD(wparam); \
	hwnd = (HWND)(lparam)
#define WM_NCACTIVATE_PARAMS(factive, minimized, hwnd) \
	factive = LOWORD(wparam); \
	minimized = (BOOL)HIWORD(wparam); \
	hwnd = (HWND)(lparam)
#define WM_QUERYENDSESSION_PARAMS(hwnd) \
	hwnd = (HWND)(wparam)
#define WM_VKEYTOITEM_PARAMS(vkey, lbhwnd, caretpos) \
	vkey = LOWORD(wparam); \
	lbhwnd = (HWND)(lparam); \
	caretpos = HIWORD(wparam)
#define WM_CHARTOITEM_PARAMS(vkey, lbhwnd, caretpos) \
	vkey = LOWORD(wparam); \
	lbhwnd = (HWND)(lparam); \
	caretpos = HIWORD(wparam)
#define WM_COMMAND_PARAMS(notifycode, id, ctl) \
	id = LOWORD(wparam); \
	ctl = (HWND)(lparam); \
	notifycode = HIWORD(wparam)
#define WM_HSCROLL_PARAMS(code, pos, ctl) \
	code = LOWORD(wparam); \
	pos = HIWORD(wparam); \
	ctl = (HWND)(lparam)
#define WM_VSCROLL_PARAMS(code, pos, ctl) WM_HSCROLL_PARAMS(code, pos, ctl)
#define WM_MENUSELECT_PARAMS(id, flags, menu) \
	id = LOWORD(wparam); \
	flags = HIWORD(wparam); \
	menu = (HMENU)(lparam)
#define WM_MENUCHAR_PARAMS(ch, flags, menu) \
	ch = LOWORD(wparam); \
	flags = HIWORD(wparam); \
	menu = (HMENU)(lparam)
#define WM_ENTERIDLE_PARAMS(source, hwnd) \
	source = LOWORD(wparam); \
	hwnd = (HWND)(lparam)
#define WM_PARENTNOTIFY_PARAMS(event, id, hwnd, x, y) \
	event = LOWORD(wparam); \
	if (event == WM_CREATE || event == WM_DESTROY) \
	{ \
		id = HIWORD(wparam); \
		hwnd = (HWND)(lparam); \
		x = y = 0; \
	} else \
	{ \
		x = LOWORD(lparam); \
		y = HIWORD(lparam); \
		id = 0; \
		hwnd = 0; \
	}
#define WM_MDIACTIVATE_PARAMS(active, deacttive, actflag) \
	active = (HWND)(lparam); \
	deactive = (HWND)(wparam); \
	actflag = active == hwnd
#define WM_MDISETMENU_PARAMS(refresh, frame, window) \
	refresh = FALSE; \
	frame = (HMENU)(wparam); \
	window = (HMENU)(lparam)
#define WM_PAINTCLIPBOARD_PARAMS(viewer, ps) \
	viewer = (HWND)(wparam); \
	ps = (HGLOBAL)(lparam)
#define WM_SIZECLIPBOARD_PARAMS(viewer, rc) \
	viewer = (HWND)(wparam); \
	rc = (HGLOBAL)(lparam)
#define WM_CHANGECBCHAIN_PARAMS(hremove, hnext) \
	hremove = (HWND)(wparam); \
	hnext = (HWND)(lparam)

/*** ---------------------------------------------------------------------- ***/

static BOOL is_mdiclient(HWND hwnd)
{
	char buf[80];
	
	if (GetClassNameA(hwnd, buf, sizeof(buf)) != 0 &&
		lstrcmp(buf, "MDICLIENT") == 0)
		return TRUE;
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

void win32debug_msg_print(FILE *out, const TCHAR *prefix, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	char hwnd_buf[80];
	char pbuf[1024];
	char buf2[1024];
	
#define hwndb(w, buf) ((HWND)(w) == HWND_BROADCAST ? "HWND_BROADCAST" : handle((HANDLE)(w), buf))
#define hwnd(w) hwndb(w, hwnd_buf)
#define hwnd2(w) hwndb(w, buf2)
#define hmenu(m) handle((HANDLE)(m), buf2)
#define hdc(w) handle((HANDLE)(w), buf2)
#define hilo(l) HIWORD(l), LOWORD(l)
#define lohi(l) LOWORD(l), HIWORD(l)
#define bool(x) ((x) == FALSE ? "FALSE" : (x) == TRUE ? "TRUE" : (sprintf(hwnd_buf, "%ld", (long)(x)), hwnd_buf))
#define tstr(x) saveptr((LPCTSTR)(x), "%s", buf2) /*TODO: wchar */
#define str(x) saveptr((LPCSTR)(x), "%s", buf2)
#define points(p) lohi(p)
#define rect(p) rectangle((LPRECT)(p), buf2)

	fprintf(out, "%s(%s, %s", prefix ? prefix : "", hwnd(hwnd), win32debug_msg_name1(message, pbuf));

	switch (message)
	{
	case BM_CLICK:
		break;
	case BM_GETCHECK:
		break;
	case BM_GETIMAGE:
		break;
	case BM_GETSTATE:
		break;
	case BM_SETCHECK:
		fprintf(out, ", %d", wparam);
		break;
	case BM_SETIMAGE:
		fprintf(out, ", %s", handle((HANDLE) lparam, pbuf));
		break;
	case BM_SETSTATE:
		fprintf(out, ", %s", bool(wparam));
		break;
	case BM_SETSTYLE:
		fprintf(out, ", %s, %s", buttonstyle(wparam, pbuf), bool(lparam));
		break;

	case WM_ACTIVATE:
		{
			WORD factive;
			BOOL minimized;
			HWND deactive;
			
			WM_ACTIVATE_PARAMS(factive, minimized, deactive);
			fprintf(out, ", %s, %s, %s", activate(factive, buf2), hwnd(deactive), bool(minimized));
		}
		break;
	case WM_ACTIVATEAPP: /* !!! lparam: threadid <-> htask */
		fprintf(out, ", %s, $%08lx", bool(wparam), lparam);
		break;
	case WM_ASKCBFORMATNAME:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_CANCELJOURNAL:
		break;
	case WM_CANCELMODE:
		break;
	case WM_CAPTURECHANGED:
		fprintf(out, ", %s", hwnd(lparam));
		break;
	case WM_CHANGECBCHAIN:
		{
			HWND hremove;
			HWND hnext;
			
			WM_CHANGECBCHAIN_PARAMS(hremove, hnext);
			fprintf(out, ", %s, %s", hwnd(hremove), hwnd2(hnext));
		}
		break;
	case WM_CHAR:
		fprintf(out, ", $%02x, $%08lx", wparam, lparam);
		break;
	case WM_CHARTOITEM:
		{
			WORD key;
			HWND lbhwnd;
			WORD caretpos;
			
			WM_CHARTOITEM_PARAMS(key, lbhwnd, caretpos);
			fprintf(out, ", %s, %s, %d", hwnd(lbhwnd), vkey(key, pbuf), caretpos);
		}
		break;
	case WM_CHILDACTIVATE:
		break;
	case WM_CHOOSEFONT_GETLOGFONT:
		fprintf(out, ", $%08lx", lparam);
		break;
	case WM_CHOOSEFONT_SETFLAGS:
		fprintf(out, ", $%08lx", lparam);
		break;
	case WM_CHOOSEFONT_SETLOGFONT:
		fprintf(out, ", $%08lx", lparam);
		break;
	case WM_CLEAR:
		break;
	case WM_CLOSE:
		break;
	case WM_COMMAND:
		{
			WORD notifycode;
			WORD id;
			HWND ctl;
			
			WM_COMMAND_PARAMS(notifycode, id, ctl);
			fprintf(out, ", %s, %d, %d", hwnd(ctl), id, notifycode);
		}
		break;
	case WM_COMMANDHELP: /* WM_AFXFIRST + 5 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_COMPACTING:
		fprintf(out, ", %d", wparam);
		break;
	case WM_COMPAREITEM:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_CONTEXTMENU:
		fprintf(out, ", %s, %d, %d", hwnd(wparam), lohi(lparam));
		break;
	case WM_COPY:
		break;
	case WM_COPYDATA:
		fprintf(out, ", %s, $%08lx", hwnd(wparam), lparam);
		break;
	case WM_CPL_LAUNCH:
		fprintf(out, ", %s, %s", hwnd(wparam), str(lparam));
		break;
	case WM_CPL_LAUNCHED:
		fprintf(out, ", %s", bool(wparam));
		break;
	case WM_CREATE:
		fprintf(out, ", %s", createstruct((LPCREATESTRUCT)lparam, pbuf, 0));
		break;
	case WM_CTLCOLOR:
		fprintf(out, ", %s, %s, %s", hdc((LONG)(INT)wparam), hwnd2((LONG)(INT)LOWORD(lparam)), ctltype(HIWORD(lparam), pbuf));
		break;
	case WM_CTLCOLORBTN:
		fprintf(out, ", %s, %s", hdc(wparam), hwnd(lparam));
		break;
	case WM_CTLCOLORDLG:
		fprintf(out, ", %s, %s", hdc(wparam), hwnd(lparam));
		break;
	case WM_CTLCOLOREDIT:
		fprintf(out, ", %s, %s", hdc(wparam), hwnd(lparam));
		break;
	case WM_CTLCOLORLISTBOX:
		fprintf(out, ", %s, %s", hdc(wparam), hwnd(lparam));
		break;
	case WM_CTLCOLORMSGBOX:
		fprintf(out, ", %s, %s", hdc(wparam), hwnd(lparam));
		break;
	case WM_CTLCOLORSCROLLBAR:
		fprintf(out, ", %s, %s", hdc(wparam), hwnd(lparam));
		break;
	case WM_CTLCOLORSTATIC:
		fprintf(out, ", %s, %s", hdc(wparam), hwnd(lparam));
		break;
	case WM_CUT:
		break;
	case WM_DDE_ACK:
		fprintf(out, ", %s, $%08lx", hwnd(wparam), lparam); /*TODO*/
		break;
	case WM_DDE_ADVISE:
		fprintf(out, ", %s, %s", hwnd(wparam), dde_param(message, lparam, pbuf));
		break;
	case WM_DDE_DATA:
		fprintf(out, ", %s, %s", hwnd(wparam), dde_param(message, lparam, pbuf));
		break;
	case WM_DDE_EXECUTE:
		fprintf(out, ", %s, %s", hwnd(wparam), dde_param(message, lparam, pbuf));
		break;
	case WM_DDE_INITIATE:
		fprintf(out, ", %s, %s", hwnd(wparam), dde_param(message, lparam, pbuf));
		break;
	case WM_DDE_POKE:
		fprintf(out, ", %s, %s", hwnd(wparam), dde_param(message, lparam, pbuf));
		break;
	case WM_DDE_REQUEST:
		fprintf(out, ", %s, %s", hwnd(wparam), dde_param(message, lparam, pbuf));
		break;
	case WM_DDE_TERMINATE:
		fprintf(out, ", %s", hwnd(wparam));
		break;
	case WM_DDE_UNADVISE:
		fprintf(out, ", %s, %s, $%04x", hwnd(wparam), clipformat(LOWORD(lparam), pbuf), HIWORD(lparam));
		break;
	case WM_DEADCHAR:
		fprintf(out, ", $%02x, $%08lx", wparam, lparam);
		break;
	case WM_DELETEITEM:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_DESTROY:
		break;
	case WM_DESTROYCLIPBOARD:
		break;
	case WM_DEVICECHANGE:
		fprintf(out, ", %s, $%08lx", devicechange(wparam, pbuf), lparam);
		break;
	case WM_DEVMODECHANGE:
		fprintf(out, ", %s", tstr(lparam));
		break;
	case WM_DISPLAYCHANGE:
		fprintf(out, ", %d, %d, %d", wparam, lohi(lparam));
		break;
	case WM_DRAWCLIPBOARD:
		break;
	case WM_DRAWITEM:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_DROPFILES:
		fprintf(out, ", %s", handle((HANDLE) wparam, pbuf));
		break;
	case WM_ENABLE:
		fprintf(out, ", %d", wparam);
		break;
	case WM_ENDSESSION:
		fprintf(out, ", %s", bool(wparam));
		break;
	case WM_ENTERIDLE:
		{
			WORD source;
			HWND dlg;
			
			WM_ENTERIDLE_PARAMS(source, dlg);
			fprintf(out, ", %s, %s", msg_source(source, pbuf), hwnd(dlg));
		}
		break;
	case WM_ENTERMENULOOP:
		fprintf(out, ", %s", bool(wparam));
		break;
	case WM_ENTERSIZEMOVE:
		break;
	case WM_ERASEBKGND:
		fprintf(out, ", %s", hdc(wparam));
		break;
	case WM_EXITMENULOOP:
		fprintf(out, ", %s", bool(wparam));
		break;
	case WM_EXITSIZEMOVE:
		break;
	case WM_FONTCHANGE:
		break;
	case WM_GETDLGCODE:
		break;
	case WM_GETFONT:
		break;
	case WM_GETHOTKEY:
		break;
	case WM_GETICON:
		fprintf(out, ", %s", bool(wparam));
		break;
	case WM_GETMINMAXINFO:
		fprintf(out, ", $%08lx", lparam);
		break;
	case WM_GETTEXT:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_GETTEXTLENGTH:
		break;
	case WM_HELP:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_HOTKEY:
		fprintf(out, ", %s", wm_hotkey(wparam, pbuf));
		break;
	case WM_HSCROLL:
		{
			WORD code;
			WORD pos;
			HWND ctl;
			
			WM_HSCROLL_PARAMS(code, pos, ctl);
			fprintf(out, ", %s, %d, %s", hscrollcode(code, pbuf), pos, hwnd(ctl));
		}
		break;
	case WM_HSCROLLCLIPBOARD:
		fprintf(out, ", %s, %s, %d", hwnd(wparam), hscrollcode(LOWORD(lparam), pbuf), HIWORD(lparam));
		break;
	case WM_ICONERASEBKGND:
		fprintf(out, ", %s", hdc(wparam));
		break;
	case WM_IME_CHAR:
		fprintf(out, ", $%04x, $%08lx", wparam, lparam);
		break;
	case WM_IME_COMPOSITION:
		fprintf(out, ", $%04x, %s", wparam, bool(lparam));
		break;
	case WM_IME_COMPOSITIONFULL:
		break;
	case WM_IME_CONTROL:
		fprintf(out, ", %s, $%08lx", imc_command(wparam, pbuf), lparam);
		break;
	case WM_IME_ENDCOMPOSITION:
		break;
	case WM_IME_KEYDOWN:
		fprintf(out, ", %s, $%08lx", vkey(wparam, pbuf), lparam);
		break;
	case WM_IME_KEYUP:
		fprintf(out, ", %s, $%08lx", vkey(wparam, pbuf), lparam);
		break;
	case WM_IME_NOTIFY:
		fprintf(out, ", %s, $%08lx", imn_command(wparam, pbuf), lparam);
		break;
	case WM_IME_SELECT:
		fprintf(out, ", %s, %s", bool(wparam), handle((HANDLE) lparam, pbuf));
		break;
	case WM_IME_SETCONTEXT:
		fprintf(out, ", %s, %s", bool(wparam), isc_show(lparam, pbuf));
		break;
	case WM_IME_STARTCOMPOSITION:
		break;
	case WM_INITDIALOG:
		fprintf(out, ", %s, $%08lx", hwnd(wparam), lparam);
		break;
	case WM_INITMENU:
		fprintf(out, ", %s", hmenu(wparam));
		break;
	case WM_INITMENUPOPUP:
		fprintf(out, ", %s, %d, %s", hmenu(wparam), LOWORD(lparam), bool(HIWORD(lparam)));
		break;
	case WM_INPUTLANGCHANGE:
		fprintf(out, ", %s, %s", bool(wparam), handle((HANDLE)lparam, pbuf));
		break;
	case WM_INPUTLANGCHANGEREQUEST:
		fprintf(out, ", %s, %s", bool(wparam), handle((HANDLE)lparam, pbuf));
		break;
	case WM_KEYDOWN:
		fprintf(out, ", %s, $%08lx", vkey(wparam, pbuf), lparam);
		break;
	case WM_KEYUP:
		fprintf(out, ", %s, $%08lx", vkey(wparam, pbuf), lparam);
		break;
	case WM_KILLFOCUS:
		fprintf(out, ", %s", hwnd(wparam));
		break;
	case WM_LBUTTONDBLCLK:
		fprintf(out, ", %s, %d, %d", vkeyflags(wparam, pbuf), lohi(lparam));
		break;
	case WM_LBUTTONDOWN:
		fprintf(out, ", %s, %d, %d", vkeyflags(wparam, pbuf), lohi(lparam));
		break;
	case WM_LBUTTONUP:
		fprintf(out, ", %s, %d, %d", vkeyflags(wparam, pbuf), lohi(lparam));
		break;
	case WM_MBUTTONDBLCLK:
		fprintf(out, ", %s, %d, %d", vkeyflags(wparam, pbuf), lohi(lparam));
		break;
	case WM_MBUTTONDOWN:
		fprintf(out, ", %s, %d, %d", vkeyflags(wparam, pbuf), lohi(lparam));
		break;
	case WM_MBUTTONUP:
		fprintf(out, ", %s, %d, %d", vkeyflags(wparam, pbuf), lohi(lparam));
		break;
	case WM_MDIACTIVATE:
		if (is_mdiclient(hwnd))
		{
			fprintf(out, ", %s (MDICLIENT)", hwnd(wparam));
		} else
		{
			HWND active;
			HWND deactive;
			BOOL actflag;
			
			WM_MDIACTIVATE_PARAMS(active, deactive, actflag);
			fprintf(out, ", %s, %s, %s", bool(actflag), hwnd(active), hwnd2(deactive));
		}
		break;
	case WM_MDICASCADE:
		fprintf(out, ", %s", mditile(wparam, pbuf));
		break;
	case WM_MDICREATE:
		fprintf(out, ", %s", mdicreatestruct((MDICREATESTRUCT *) lparam, pbuf));
		break;
	case WM_MDIDESTROY:
		fprintf(out, ", %s", hwnd(wparam));
		break;
	case WM_MDIGETACTIVE:
		fprintf(out, ", $%08lx", lparam);
		break;
	case WM_MDIICONARRANGE:
		break;
	case WM_MDIMAXIMIZE:
		fprintf(out, ", %s", hwnd(wparam));
		break;
	case WM_MDINEXT:
		fprintf(out, ", %s, %s", hwnd(wparam), bool(lparam));
		break;
	case WM_MDIREFRESHMENU:
		break;
	case WM_MDIRESTORE:
		fprintf(out, ", %s", hwnd(wparam));
		break;
	case WM_MDISETMENU:
		{
			HMENU frame;
			HMENU window;
			BOOL refresh;
			
			WM_MDISETMENU_PARAMS(refresh, frame, window);
			fprintf(out, ", %s, %s, %s", bool(refresh), hmenu(frame), handle((HANDLE) window, buf2));
		}
		break;
	case WM_MDITILE:
		fprintf(out, ", %s", mditile(wparam, pbuf));
		break;
	case WM_MEASUREITEM:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_MENUCHAR:
		{
			WORD ch;
			WORD flags;
			HMENU menu;
			
			WM_MENUSELECT_PARAMS(ch, flags, menu);
			fprintf(out, ", $%04x, %s, %s", ch, menuflags(flags, pbuf), hmenu(menu));
		}
		break;
	case WM_MENUSELECT:
		{
			WORD id;
			WORD flags;
			HMENU menu;
			
			WM_MENUSELECT_PARAMS(id, flags, menu);
			fprintf(out, ", %d, %s, %s", id, menuflags(flags, pbuf), hmenu(menu));
		}
		break;
	case WM_MOUSEACTIVATE:
		fprintf(out, ", %s, %s, %s", hwnd(wparam), hittest(LOWORD(lparam), pbuf), win32debug_msg_name1(HIWORD(lparam), hwnd_buf));
		break;
	case WM_MOUSEMOVE:
		fprintf(out, ", %s, %d, %d", vkeyflags(wparam, pbuf), lohi(lparam));
		break;
	case WM_MOVE:
		fprintf(out, ", %d, %d", LOWORD(lparam), HIWORD(lparam));
		break;
	case WM_MOVING:
		fprintf(out, ", %s, %s", windowside(wparam, pbuf), rect(lparam));
		break;
	case WM_NCACTIVATE:
		{
			WORD factive;
			BOOL minimized;
			HWND deactive;
			
			WM_NCACTIVATE_PARAMS(factive, minimized, deactive);
			fprintf(out, ", %s, %s, %s", activate(factive, buf2), hwnd(deactive), bool(minimized));
		}
		break;
	case WM_NCCALCSIZE:
		fprintf(out, ", %s, $%08lx", bool(wparam), lparam);
		break;
	case WM_NCCREATE:
		fprintf(out, ", $%08lx", lparam);
		break;
	case WM_NCDESTROY:
		break;
	case WM_NCHITTEST:
		fprintf(out, ", %d, %d", lohi(lparam));
		break;
	case WM_NCLBUTTONDBLCLK:
		fprintf(out, ", %s, %d, %d", hittest(wparam, pbuf), points(lparam));
		break;
	case WM_NCLBUTTONDOWN:
		fprintf(out, ", %s, %d, %d", hittest(wparam, pbuf), points(lparam));
		break;
	case WM_NCLBUTTONUP:
		fprintf(out, ", %s, %d, %d", hittest(wparam, pbuf), points(lparam));
		break;
	case WM_NCMBUTTONDBLCLK:
		fprintf(out, ", %s, %d, %d", hittest(wparam, pbuf), points(lparam));
		break;
	case WM_NCMBUTTONDOWN:
		fprintf(out, ", %s, %d, %d", hittest(wparam, pbuf), points(lparam));
		break;
	case WM_NCMBUTTONUP:
		fprintf(out, ", %s, %d, %d", hittest(wparam, pbuf), points(lparam));
		break;
	case WM_NCMOUSEMOVE:
		fprintf(out, ", %s, %d, %d", hittest(wparam, pbuf), points(lparam));
		break;
	case WM_NCPAINT:
		fprintf(out, ", %s", handle((HANDLE)wparam, pbuf));
		break;
	case WM_NCRBUTTONDBLCLK:
		fprintf(out, ", %s, %d, %d", hittest(wparam, pbuf), points(lparam));
		break;
	case WM_NCRBUTTONDOWN:
		fprintf(out, ", %s, %d, %d", hittest(wparam, pbuf), points(lparam));
		break;
	case WM_NCRBUTTONUP:
		fprintf(out, ", %s, %d, %d", hittest(wparam, pbuf), points(lparam));
		break;
	case WM_NEXTDLGCTL:
		fprintf(out, ", %s, %s", hwnd(wparam), bool(LOWORD(lparam)));
		break;
	case WM_NEXTMENU: /* ??? */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_NOTIFY:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_NOTIFYFORMAT:
		fprintf(out, ", %s, %ld", hwnd(wparam), lparam);
		break;
	case WM_PAINT:
		break;
	case WM_PAINTCLIPBOARD:
		{
			HWND viewer;
			HGLOBAL ps;
			
			WM_PAINTCLIPBOARD_PARAMS(viewer, ps);
			fprintf(out, ", %s, %s", hwnd(viewer), handle((HANDLE) ps, pbuf));
		}
		break;
	case WM_PAINTICON:
		break;
	case WM_PALETTECHANGED:
		fprintf(out, ", %s", hwnd(wparam));
		break;
	case WM_PALETTEISCHANGING:
		fprintf(out, ", %s", hwnd(wparam));
		break;
	case WM_PARENTNOTIFY:
		{
			UINT event;
			UINT id;
			HWND child;
			int x, y;
			
			WM_PARENTNOTIFY_PARAMS(event, id, child, x, y);
			if (event == WM_CREATE || event == WM_DESTROY)
			{
				fprintf(out, ", %s, %d, %s", win32debug_msg_name1(event, pbuf), id, hwnd(child));
			} else
			{
				fprintf(out, ", %s, %d, %d", win32debug_msg_name1(event, pbuf), x, y);
			}
		}
		break;
	case WM_PASTE:
		break;
	case WM_POWER:
		fprintf(out, ", %d", wparam);
		break;
	case WM_POWERBROADCAST:
		fprintf(out, ", %s, $%08lx", powerbroadcast(wparam, pbuf), lparam);
		break;
	case WM_PRINT:
		fprintf(out, ", %s, %s", hdc(wparam), wm_print(lparam, pbuf));
		break;
	case WM_PRINTCLIENT:
		fprintf(out, ", %s, %s", hdc(wparam), wm_print(lparam, pbuf));
		break;
#if 0
	case WM_PSD_ENVSTAMPRECT:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_PSD_FULLPAGERECT:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_PSD_GREEKTEXTRECT:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_PSD_MARGINRECT:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_PSD_PAGESETUPDLG:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_PSD_YAFULLPAGERECT:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_PSD_MINMARGINRECT:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
#endif
	case WM_QUERYDRAGICON:
		break;
	case WM_QUERYENDSESSION:
		{
			HWND w;
			
			WM_QUERYENDSESSION_PARAMS(w);
			fprintf(out, ", %s", hwnd(w));
		}
		break;
	case WM_QUERYNEWPALETTE:
		break;
	case WM_QUERYOPEN:
		break;
	case WM_QUEUESYNC:
		break;
	case WM_QUIT:
		fprintf(out, ", %d", wparam);
		break;
	case WM_RASDIALEVENT: /* TODO */
		break;
	case WM_RBUTTONDBLCLK:
		fprintf(out, ", %s, %d, %d", vkeyflags(wparam, pbuf), lohi(lparam));
		break;
	case WM_RBUTTONDOWN:
		fprintf(out, ", %s, %d, %d", vkeyflags(wparam, pbuf), lohi(lparam));
		break;
	case WM_RBUTTONUP:
		fprintf(out, ", %s, %d, %d", vkeyflags(wparam, pbuf), lohi(lparam));
		break;
	case WM_RENDERALLFORMATS:
		break;
	case WM_RENDERFORMAT:
		fprintf(out, ", %s", clipformat(wparam, pbuf));
		break;
	case WM_SETCURSOR:
		fprintf(out, ", %s, %s, %d", hwnd(wparam), hittest(LOWORD(lparam), pbuf), HIWORD(lparam));
		break;
	case WM_SETFOCUS:
		fprintf(out, ", %s", hwnd(wparam));
		break;
	case WM_SETFONT:
		fprintf(out, ", %s, %s", handle((HANDLE)wparam, hwnd_buf), bool(LOWORD(lparam)));
		break;
	case WM_SETHOTKEY:
		fprintf(out, ", %s, %s", vkey(LOBYTE(wparam), hwnd_buf), wm_sethotkey(HIBYTE(wparam), pbuf));
		break;
	case WM_SETICON:
		fprintf(out, ", %s, %s", bool(wparam), handle((HANDLE)lparam, pbuf));
		break;
	case WM_SETREDRAW:
		fprintf(out, ", %d", wparam);
		break;
	case WM_SETTEXT:
		fprintf(out, ", %s", (LPCTSTR)lparam);
		break;
	/* case WM_SETTINGCHANGE: alias to WM_WININICHANGE */
	case WM_SHOWWINDOW:
		fprintf(out, ", %s, $%08lx", bool(wparam), lparam);
		break;
	case WM_SIZE:
		fprintf(out, ", %s, %d, %d", wm_size(wparam, pbuf), LOWORD(lparam), HIWORD(lparam));
		break;
	case WM_SIZECLIPBOARD:
		{
			HWND viewer;
			HGLOBAL rc;
			
			WM_SIZECLIPBOARD_PARAMS(viewer, rc);
			fprintf(out, ", %s, %s", hwnd(viewer), handle((HANDLE) rc, pbuf));
		}
		break;
	case WM_SIZING:
		fprintf(out, ", %s, %s", windowside(wparam, pbuf), rect(lparam));
		break;
	case WM_SPOOLERSTATUS:
		fprintf(out, ", %d, %ld", wparam, lparam);
		break;
	case WM_STYLECHANGED:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_STYLECHANGING:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_SYSCHAR:
		fprintf(out, ", $%02x, $%08lx", wparam, lparam);
		break;
	case WM_SYSCOLORCHANGE:
		break;
	case WM_SYSCOMMAND:
		fprintf(out, ", %s, %d, %d", syscommand(wparam, pbuf), lohi(lparam));
		break;
	case WM_SYSDEADCHAR:
		fprintf(out, ", $%02x, $%08lx", wparam, lparam);
		break;
	case WM_SYSKEYDOWN:
		fprintf(out, ", %s, $%08lx", vkey(wparam, pbuf), lparam);
		break;
	case WM_SYSKEYUP:
		fprintf(out, ", %s, $%08lx", vkey(wparam, pbuf), lparam);
		break;
	case WM_SYSTEMERROR:
		fprintf(out, ", %d", wparam);
		break;
	case WM_TCARD:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_TIMECHANGE:
		break;
	case WM_TIMER:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_UNDO:
		break;
	case WM_USERCHANGED:
		break;
	case WM_VKEYTOITEM:
		{
			WORD key;
			HWND lbhwnd;
			WORD caretpos;
			
			WM_VKEYTOITEM_PARAMS(key, lbhwnd, caretpos);
			fprintf(out, ", %s, %s, %d", hwnd(lbhwnd), vkey(key, pbuf), caretpos);
		}
		break;
	case WM_VSCROLL:
		{
			WORD code;
			WORD pos;
			HWND ctl;
			
			WM_VSCROLL_PARAMS(code, pos, ctl);
			fprintf(out, ", %s, %d, %s", vscrollcode(code, pbuf), pos, hwnd(ctl));
		}
		break;
	case WM_VSCROLLCLIPBOARD:
		fprintf(out, ", %s, %s, %d", hwnd(wparam), vscrollcode(LOWORD(lparam), pbuf), HIWORD(lparam));
		break;
	case WM_WINDOWPOSCHANGED:
		fprintf(out, ", $%08lx", lparam);
		break;
	case WM_WINDOWPOSCHANGING:
		fprintf(out, ", $%08lx", lparam);
		break;
	case WM_WININICHANGE:
		fprintf(out, ", %s", tstr(lparam));
		break;




	case WM_QUERYAFXWNDPROC: /* ALSO: WM_VBXFIREEVENT */ /* WM_AFXFIRST + 0 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_SIZEPARENT:/* WM_AFXFIRST + 1 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_SETMESSAGESTRING: /* WM_AFXFIRST + 2 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_IDLEUPDATECMDUI: /* WM_AFXFIRST + 3 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_INITIALUPDATE: /* WM_AFXFIRST + 4 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_HELPHITTEST: /* WM_AFXFIRST + 6 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_EXITHELPMODE: /* WM_AFXFIRST + 7 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_RECALCPARENT: /* WM_AFXFIRST + 8 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_SIZECHILD: /* WM_AFXFIRST + 9 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_KICKIDLE: /* WM_AFXFIRST + 10 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_QUERYCENTERWND: /* WM_AFXFIRST + 11 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_DISABLEMODAL: /* WM_AFXFIRST + 12 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_FLOATSTATUS: /* WM_AFXFIRST + 13 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_ACTIVATETOPLEVEL: /* WM_AFXFIRST + 14 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_QUERY3DCONTROLS: /* WM_AFXFIRST + 15 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_RESERVED_0370: /* WM_AFXFIRST + 16 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_RESERVED_0371: /* WM_AFXFIRST + 17 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_RESERVED_0372: /* WM_AFXFIRST + 18 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_SOCKET_NOTIFY: /* WM_AFXFIRST + 19 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_SOCKET_DEAD: /* WM_AFXFIRST + 20 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_POPMESSAGESTRING: /* WM_AFXFIRST + 21 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_OCC_LOADFROMSTREAM: /* WM_AFXFIRST + 22 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_OCC_LOADFROMSTORAGE: /* WM_AFXFIRST + 23 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_OCC_INITNEW: /* WM_AFXFIRST + 24 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_QUEUE_SENTINEL: /* WM_AFXFIRST + 25 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_RESERVED_037A: /* WM_AFXFIRST + 26 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_RESERVED_037B: /* WM_AFXFIRST + 27 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_RESERVED_037C: /* WM_AFXFIRST + 28 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_RESERVED_037D: /* WM_AFXFIRST + 29 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_RESERVED_037E: /* WM_AFXFIRST + 30 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_RESERVED_037F: /* WM_AFXFIRST + 31 */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;

	case WM_RCRESULT: /* PENWIN */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_HOOKRCRESULT: /* PENWIN */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_PENMISCINFO: /* PENWIN */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_SKB: /* PENWIN */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_PENCTL: /* PENWIN */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_PENMISC: /* PENWIN */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_CTLINIT: /* PENWIN */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_PENEVENT: /* PENWIN */
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;

#if 0
	case WM_HANDHELDFIRST:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
	case WM_HANDHELDLAST:
		fprintf(out, ", %d, $%08lx", wparam, lparam);
		break;
#endif
	
	default:
		fprintf(out, ", $%08x, $%08lx", wparam, lparam);
		break;
	}
	fprintf(out, ")\n");
#undef wnd
#undef hdc
#undef lohi
#undef hilo
#undef str
#undef tstr
}

#endif /* WIN32_DEBUG_MSGS */
