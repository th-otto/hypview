/*
 * HypView - (c)      - 2006 Philipp Donze
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

#include "hv_defs.h"
#include "hypdebug.h"
#include "tos/av.h"
#include "tos/bgh.h"
#include "tos/mem.h"
#include "tos/xacc.h"
#include "tos/bubble.h"
#include "tos/dhst.h"
#include "tos/gscript.h"

/* PureC Hilfe-Protokoll */
#define AC_HELP		0x0401
#define AC_REPLY	0x0402
#define AC_VERSION	0x0403
#define AC_COPY		0x0404

/* OLGA protocol */
#define OLE_INIT							0x4950
#define OLE_EXIT							0x4951
#define OLE_NEW 							0x4952

#define OLGA_INIT 							0x1236
#define OLGA_UPDATE 						0x1238
#define OLGA_ACK							0x1239
#define OLGA_RENAME 						0x123A
#define OLGA_OPENDOC						0x123B
#define OLGA_CLOSEDOC 						0x123C
#define OLGA_LINK 							0x123D
#define OLGA_UNLINK 						0x123E
#define OLGA_UPDATED						0x123F
#define OLGA_RENAMELINK 					0x1240
#define OLGA_LINKRENAMED					0x1241
#define OLGA_GETOBJECTS 					0x1242
#define OLGA_OBJECTS						0x1243
#define OLGA_BREAKLINK						0x1244
#define OLGA_LINKBROKEN 					0x1245
#define OLGA_START							0x1246
#define OLGA_GETINFO						0x1247
#define OLGA_INFO 							0x1248
#define OLGA_IDLE 							0x1249
#define OLGA_ACTIVATE 						0x124A
#define OLGA_EMBED							0x124B
#define OLGA_EMBEDDED 						0x124C
#define OLGA_UNEMBED						0x124D
#define OLGA_GETSETTINGS					0x124E
#define OLGA_SETTINGS 						0x124F
#define OLGA_REQUESTNOTIFICATION			0x1250
#define OLGA_RELEASENOTIFICATION			0x1251
#define OLGA_NOTIFY 						0x1252
#define OLGA_NOTIFIED 						0x1253
#define OLGA_SERVERTERMINATED	 			0x1254
#define OLGA_CLIENTTERMINATED 				0x1255
#define OLGA_INPLACEUPDATE					0x1256
#define OLGA_ID4UPDATE						0x1257
#define OLGA_GETEXTENSION					0x1258
#define OLGA_EXTENSION						0x1259
#define OLGA_GETSERVERPATH					0x125A
#define OLGA_SERVERPATH						0x125B

/* System Services Protocol */
#define SSP_SRASR 0x126f
#define SSP_SSIR  0x1270
#define SSP_SPASI 0x1271
#define SSP_SSUR  0x1272
#define SSP_SPASA 0x1273
#define SSP_SSA   0x1274

/* SE-Protocol */
#define SE_INIT 0x4200
#define SE_OK 0x4201
#define SE_ACK 0x4202
#define SE_OPEN 0x4203
#define SE_ERROR 0x4204
#define SE_ERRFILE 0x4205
#define SE_PROJECT 0x4206
#define SE_QUIT 0x4207
#define SE_TERMINATE 0x4208
#define SE_CLOSE 0x4209
#define ES_INIT 0x4240
#define ES_OK 0x4241
#define ES_ACK 0x4242
#define ES_COMPILE 0x4243
#define ES_MAKE 0x4244
#define ES_MAKEALL 0x4245
#define ES_LINK 0x4246
#define ES_EXEC 0x4247
#define ES_MAKEEXEC 0x4248
#define ES_PROJECT 0x4249
#define ES_QUIT 0x424a
#define ES_SHLCTRL 0x424b

/* View protocol */
#define VIEW_FILE    0x5600
#define VIEW_FAILED  0x5601
#define VIEW_OPEN    0x5602
#define VIEW_CLOSED  0x5603
#define VIEW_DATA    0x5604
#define VIEW_GETMFDB 0x5610
#define VIEW_SETMFDB 0x5611
#define VIEW_MFDB    0x5612

/* Font protocol */
#define FONT_CHANGED  0x7a18
#define FONT_SELECT   0x7a19
#define FONT_ACK      0x7a1a
#define XFONT_CHANGED 0x7a1b

static hyp_nodenr last_node;

/*******************************************************/
/****** Events                                    ******/
/*******************************************************/

void DoButton(EVNT *event)
{
	UNUSED(event);

#if USE_BUBBLEGEM
	if (event->button == 2)
		Bubble(event->mx, event->my);
#endif
}

const char *gem_message_name(_UWORD id)
{
	static char buf[10];

	switch (id)
	{
		case MN_SELECTED: return "MN_SELECTED";
		case WM_REDRAW: return "WM_REDRAW";
		case WM_TOPPED: return "WM_TOPPED";
		case WM_CLOSED: return "WM_CLOSED";
		case WM_FULLED: return "WM_FULLED";
		case WM_ARROWED: return "WM_ARROWED";
		case WM_HSLID: return "WM_HSLID";
		case WM_VSLID: return "WM_VSLID";
		case WM_SIZED: return "WM_SIZED";
		case WM_MOVED: return "WM_MOVED";
		case WM_NEWTOP: return "WM_NEWTOP";
		case WM_UNTOPPED: return "WM_UNTOPPED";
		case WM_ONTOP: return "WM_ONTOP";
		case WM_OFFTOP: return "WM_OFFTOP";
		case WM_BOTTOM: return "WM_BOTTOM";
		case WM_ICONIFY: return "WM_ICONIFY";
		case WM_UNICONIFY: return "WM_UNICONIFY";
		case WM_ALLICONIFY: return "WM_ALLICONIFY";
		case WM_TOOLBAR: return "WM_TOOLBAR";
		case WM_REPOSED: return "WM_REPOSED";
		case AC_OPEN: return "AC_OPEN";
		case AC_CLOSE: return "AC_CLOSE";
		case WM_ISTOP: return "WM_ISTOP";
		/* case CT_UPDATE: return "CT_UPDATE"; same as AP_TERM */
		/* case CT_MOVE: return "CT_MOVE"; same as AP_TFAIL */
		case CT_NEWTOP: return "CT_NEWTOP";
		case CT_KEY: return "CT_KEY";
		/* case CT_SWITCH: return "CT_SWITCH"; same as CT_KEY */
		case AP_TERM: return "AP_TERM";
		case AP_TFAIL: return "AP_TFAIL";
		case AP_RESCHG: return "AP_RESCHG";
		case SHUT_COMPLETED: return "SHUT_COMPLETED";
		case RESCHG_COMPLETED: return "RESCHG_COMPLETED";
		case AP_DRAGDROP: return "AP_DRAGDROP";
		case SH_EXIT: return "SH_EXIT";
		case SH_START: return "SH_START";
		case SH_WDRAW: return "SH_WDRAW";
		case SC_CHANGED: return "SC_CHANGED";
		case PRN_CHANGED: return "PRN_CHANGED";
		case FNT_CHANGED: return "FNT_CHANGED";
		case COLORS_CHANGED: return "COLORS_CHANGED";
		case THR_EXIT: return "THR_EXIT";
		case PA_EXIT: return "PA_EXIT";
		case CH_EXIT: return "CH_EXIT";
		case WM_BACKDROP: return "WM_BACKDROP";
		case SM_M_SPECIAL: return "SM_M_SPECIAL";
		case SM_M_RES2: return "SM_M_RES2";
		case SM_M_RES3: return "SM_M_RES3";
		case SM_M_RES4: return "SM_M_RES4";
		case SM_M_RES5: return "SM_M_RES5";
		case SM_M_RES6: return "SM_M_RES6";
		case SM_M_RES7: return "SM_M_RES7";
		case SM_M_RES8: return "SM_M_RES8";
		case SM_M_RES9: return "SM_M_RES9";
		case WM_WHEEL: return "WM_WHEEL";
		case WM_MOUSEWHEEL: return "WM_MOUSEWHEEL";
		case WM_SHADED: return "WM_SHADED";
		case WM_UNSHADED: return "WM_UNSHADED";

		case AV_PROTOKOLL: return "AV_PROTOKOLL";
		case VA_PROTOSTATUS: return "VA_PROTOSTATUS";
		case AV_GETSTATUS: return "AV_GETSTATUS";
		case AV_STATUS: return "AV_STATUS";
		case VA_SETSTATUS: return "VA_SETSTATUS";
		case AV_SENDCLICK: return "AV_SENDCLICK";
		case AV_SENDKEY: return "AV_SENDKEY";
		case VA_START: return "VA_START";
		case AV_ASKFILEFONT: return "AV_ASKFILEFONT";
		case VA_FILEFONT: return "VA_FILEFONT";
		case AV_ASKCONFONT: return "AV_ASKCONFONT";
		case VA_CONFONT: return "VA_CONFONT";
		case AV_ASKOBJECT: return "AV_ASKOBJECT";
		case VA_OBJECT: return "VA_OBJECT";
		case AV_OPENCONSOLE: return "AV_OPENCONSOLE";
		case VA_CONSOLEOPEN: return "VA_CONSOLEOPEN";
		case AV_OPENWIND: return "AV_OPENWIND";
		case VA_WINDOPEN: return "VA_WINDOPEN";
		case AV_STARTPROG: return "AV_STARTPROG";
		case VA_PROGSTART: return "VA_PROGSTART";
		case AV_ACCWINDOPEN: return "AV_ACCWINDOPEN";
		case VA_DRAGACCWIND: return "VA_DRAGACCWIND";
		case AV_ACCWINDCLOSED: return "AV_ACCWINDCLOSED";
		case AV_COPY_DRAGGED: return "AV_COPY_DRAGGED";
		case VA_COPY_COMPLETE: return "VA_COPY_COMPLETE";
		case AV_PATH_UPDATE: return "AV_PATH_UPDATE";
		case AV_WHAT_IZIT: return "AV_WHAT_IZIT";
		case VA_THAT_IZIT: return "VA_THAT_IZIT";
		case AV_DRAG_ON_WINDOW: return "AV_DRAG_ON_WINDOW";
		case VA_DRAG_COMPLETE: return "VA_DRAG_COMPLETE";
		case AV_EXIT: return "AV_EXIT";
		case AV_STARTED: return "AV_STARTED";
		case VA_FONTCHANGED: return "VA_FONTCHANGED";
		case AV_XWIND: return "AV_XWIND";
		case VA_XOPEN: return "VA_XOPEN";
		case AV_VIEW: return "AV_VIEW";
		case VA_VIEWED: return "VA_VIEWED";
		case AV_FILEINFO: return "AV_FILEINFO";
		case VA_FILECHANGED: return "VA_FILECHANGED";
		case AV_COPYFILE: return "AV_COPYFILE";
		case VA_FILECOPIED: return "VA_FILECOPIED";
		case AV_DELFILE: return "AV_DELFILE";
		case VA_FILEDELETED: return "VA_FILEDELETED";
		case AV_SETWINDPOS: return "AV_SETWINDPOS";
		case VA_PATH_UPDATE: return "VA_PATH_UPDATE";

		case ACC_ID: return "ACC_ID";
		/* case ACC_OPEN: return "ACC_OPEN"; same as AC_HELP */
		/* case ACC_CLOSE: return "ACC_CLOSE"; same as AC_REPLY */
		/* case ACC_ACC: return "ACC_ACC"; same as AC_VERSION */
		/* case ACC_EXIT: return "ACC_EXIT"; same as AC_COPY */
		case ACC_ACK: return "ACC_ACK";
		case ACC_TEXT: return "ACC_TEXT";
		case ACC_KEY: return "ACC_KEY";
		case ACC_META: return "ACC_META";
		case ACC_IMG: return "ACC_IMG";
		case ACC_GETDSI: return "ACC_GETDSI";
		case ACC_DSINFO: return "ACC_DSINFO";
		case ACC_FILEINFO: return "ACC_FILEINFO";
		case ACC_GETFIELDS: return "ACC_GETFIELDS";
		case ACC_FIELDINFO: return "ACC_FIELDINFO";
		case ACC_FORCESDF: return "ACC_FORCESDF";
		case ACC_GETSDF: return "ACC_GETSDF";

		case AC_HELP: return "AC_HELP";
		case AC_REPLY: return "AC_REPLY";
		case AC_VERSION: return "AC_VERSION";
		case AC_COPY: return "AC_COPY";

		case BUBBLEGEM_REQUEST: return "BUBBLEGEM_REQUEST";
		case BUBBLEGEM_SHOW: return "BUBBLEGEM_SHOW";
		case BUBBLEGEM_ACK: return "BUBBLEGEM_ACK";
		case BUBBLEGEM_ASKFONT: return "BUBBLEGEM_ASKFONT";
		case BUBBLEGEM_FONT: return "BUBBLEGEM_FONT";
		case BUBBLEGEM_HIDE: return "BUBBLEGEM_HIDE";

		case DHST_ADD: return "DHST_ADD";
		case DHST_ACK: return "DHST_ACK";

		case GS_REQUEST: return "GS_REQUEST";
		case GS_REPLY: return "GS_REPLY";
		case GS_COMMAND: return "GS_COMMAND";
		case GS_ACK: return "GS_ACK";
		case GS_QUIT: return "GS_QUIT";
		case GS_OPENMACRO: return "GS_OPENMACRO";
		case GS_MACRO: return "GS_MACRO";
		case GS_WRITE: return "GS_WRITE";
		case GS_CLOSEMACRO: return "GS_CLOSEMACRO";

		case 0x995: return "RSDAEMON_MSG";

		case 0x1000: return "WM_SAVE";
		case 0x1001: return "WM_SAVEAS";
		case 0x1002: return "WM_PRINT";
		case 0x1003: return "WM_UNDO";
		case 0x1004: return "WM_CUT";
		case 0x1005: return "WM_COPY";
		case 0x1006: return "WM_PASTE";
		case 0x1007: return "WM_SELECTALL";
		case 0x1008: return "WM_FIND";
		case 0x1009: return "WM_REPLACE";
		case 0x100a: return "WM_FINDNEXT";
		case 0x100b: return "WM_HELP";
		case 0x100c: return "WM_DELETE";

		case OLGA_INIT: return "OLGA_INIT";
		case OLGA_UPDATE: return "OLGA_UPDATE";
		case OLGA_ACK: return "OLGA_ACK";
		case OLGA_RENAME: return "OLGA_RENAME";
		case OLGA_OPENDOC: return "OLGA_OPENDOC";
		case OLGA_CLOSEDOC: return "OLGA_CLOSEDOC";
		case OLGA_LINK: return "OLGA_LINK";
		case OLGA_UNLINK: return "OLGA_UNLINK";
		case OLGA_UPDATED: return "OLGA_UPDATED";
		case OLGA_RENAMELINK: return "OLGA_RENAMELINK";
		case OLGA_LINKRENAMED: return "OLGA_LINKRENAMED";
		case OLGA_GETOBJECTS: return "OLGA_GETOBJECTS";
		case OLGA_OBJECTS: return "OLGA_OBJECTS";
		case OLGA_BREAKLINK: return "OLGA_BREAKLINK";
		case OLGA_LINKBROKEN: return "OLGA_LINKBROKEN";
		case OLGA_START: return "OLGA_START";
		case OLGA_GETINFO: return "OLGA_GETINFO";
		case OLGA_INFO: return "OLGA_INFO";
		case OLGA_IDLE: return "OLGA_IDLE";
		case OLGA_ACTIVATE: return "OLGA_ACTIVATE";
		case OLGA_EMBED: return "OLGA_EMBED";
		case OLGA_EMBEDDED: return "OLGA_EMBEDDED";
		case OLGA_UNEMBED: return "OLGA_UNEMBED";
		case OLGA_GETSETTINGS: return "OLGA_GETSETTINGS";
		case OLGA_SETTINGS: return "OLGA_SETTINGS";
		case OLGA_REQUESTNOTIFICATION: return "OLGA_REQUESTNOTIFICATION";
		case OLGA_RELEASENOTIFICATION: return "OLGA_RELEASENOTIFICATION";
		case OLGA_NOTIFY: return "OLGA_NOTIFY";
		case OLGA_NOTIFIED: return "OLGA_NOTIFIED";
		case OLGA_SERVERTERMINATED: return "OLGA_SERVERTERMINATED";
		case OLGA_CLIENTTERMINATED: return "OLGA_CLIENTTERMINATED";
		case OLGA_INPLACEUPDATE: return "OLGA_INPLACEUPDATE";
		case OLGA_ID4UPDATE: return "OLGA_ID4UPDATE";
		case OLGA_GETEXTENSION: return "OLGA_GETEXTENSION";
		case OLGA_EXTENSION: return "OLGA_EXTENSION";
		case OLGA_GETSERVERPATH: return "OLGA_GETSERVERPATH";
		case OLGA_SERVERPATH: return "OLGA_SERVERPATH";

		case OLE_INIT: return "OLE_INIT";
		case OLE_EXIT: return "OLE_EXIT";
		case OLE_NEW: return "OLE_NEW";

		case SSP_SRASR: return "SSP_SRASR";
		case SSP_SSIR: return "SSP_SSIR";
		case SSP_SPASI: return "SSP_SPASI";
		case SSP_SSUR: return "SSP_SSUR";
		case SSP_SPASA: return "SSP_SPASA";
		case SSP_SSA: return "SSP_SSA";

		case SE_INIT: return "SE_INIT";
		case SE_OK: return "SE_OK";
		case SE_ACK: return "SE_ACK";
		case SE_OPEN: return "SE_OPEN";
		case SE_ERROR: return "SE_ERROR";
		case SE_ERRFILE: return "SE_ERRFILE";
		case SE_PROJECT: return "SE_PROJECT";
		case SE_QUIT: return "SE_QUIT";
		case SE_TERMINATE: return "SE_TERMINATE";
		case SE_CLOSE: return "SE_CLOSE";
		case ES_INIT: return "ES_INIT";
		case ES_OK: return "ES_OK";
		case ES_ACK: return "ES_ACK";
		case ES_COMPILE: return "ES_COMPILE";
		case ES_MAKE: return "ES_MAKE";
		case ES_MAKEALL: return "ES_MAKEALL";
		case ES_LINK: return "ES_LINK";
		case ES_EXEC: return "ES_EXEC";
		case ES_MAKEEXEC: return "ES_MAKEEXEC";
		case ES_PROJECT: return "ES_PROJECT";
		case ES_QUIT: return "ES_QUIT";
		case ES_SHLCTRL: return "ES_SHLCTRL";

		case VIEW_FILE: return "VIEW_FILE";
		case VIEW_FAILED: return "VIEW_FAILED";
		case VIEW_OPEN: return "VIEW_OPEN";
		case VIEW_CLOSED: return "VIEW_CLOSED";
		case VIEW_DATA: return "VIEW_DATA";
		case VIEW_GETMFDB: return "VIEW_GETMFDB";
		case VIEW_SETMFDB: return "VIEW_SETMFDB";
		case VIEW_MFDB: return "VIEW_MFDB";

		case FONT_CHANGED: return "FONT_CHANGED";
		case FONT_SELECT: return "FONT_SELECT";
		case FONT_ACK: return "FONT_ACK";
		case XFONT_CHANGED: return "XFONT_CHANGED";
	}
	
	sprintf(buf, "%d", id);
	return buf;
}


void DoUserEvents(EVNT *event)
{
	if (event->mwhich & MU_MESAG)
	{
		nf_debugprintf("HypView(%d): %s %04x %04x %04x %04x %04x\n",
			event->msg[2],
			gem_message_name(event->msg[0]),
			event->msg[3],
			event->msg[4],
			event->msg[5],
			event->msg[6],
			event->msg[7]);
			
		switch (event->msg[0])
		{
		case AC_OPEN:
			{
				WINDOW_DATA *win;
				
				va_proto_init(NULL);
				win = top_window();
				if (win == NULL && !empty(gl_profile.viewer.last_file))
				{
					win = OpenFileInWindow(NULL, gl_profile.viewer.last_file, NULL, last_node, FALSE, TRUE, FALSE);
				}
	
				if (win == NULL)
				{
					if (!empty(gl_profile.viewer.default_file))
					{
						char *filename = path_subst(gl_profile.viewer.default_file);
						if (OpenFileInWindow(win, filename, NULL, HYP_NOINDEX, FALSE, TRUE, FALSE) == NULL)
						{
							g_freep(&gl_profile.viewer.default_file);
#if 0
							if (gl_profile.profile)
								gl_profile.profile->changed = TRUE;
#endif
							SelectFileLoad(NULL);
						}
						g_free(filename);
					} else
					{
						SelectFileLoad(NULL);
					}
				} else
				{
					SendTopped(win->whandle);
				}

				if (all_list && gl_profile.remarker.run_on_startup)
					StartRemarker(TRUE, FALSE);
			}
			event->mwhich &= ~MU_MESAG;
			break;

		case AC_CLOSE:
			event->mwhich &= ~MU_MESAG;
			{
				/* window handles are no longer valid when we receive this message */
				CHAIN_DATA *ptr;
				for (ptr = all_list; ptr; ptr = ptr->next)
					ptr->whandle = -1;
			}
			RemoveItems();
			va_proto_init(NULL);
			break;

		case AC_HELP:
			{
				char *data = *(char **) &event->msg[3];
	
				if (data != NULL)
				{
					char *name;
					
					HYP_DBG(("AC_HELP from %d: <%s>", event->msg[1], printnull(data)));
					name = hyp_conv_to_utf8(hyp_get_current_charset(), data, STR0TERM);
					search_allref(NULL, name, FALSE);
					g_free(name);
				}
			}
			event->mwhich &= ~MU_MESAG;
			break;

		case WM_CLOSED:
			/*
			 * save the path of the last window closed,
			 * so it can be reopenend again on receive of AC_OPEN.
			 * Only need to do this when running as accessory,
			 * since a regular application will exit when all
			 * windows are closed.
			 */
			if (!_app)
			{								/* find the last window */
				/*
				 * MU_MESAG must not be removed otherwise
				 * the window will not be closed
				 */
				WINDOW_DATA *win;
	
				win = find_window_by_whandle(event->msg[3]);
				if (win && (win->status & WIS_OPEN) && win->owner == gl_apid)
				{
					if (count_window() == 1)
					{
						DOCUMENT *doc;
	
						doc = (DOCUMENT *) win->data;
						last_node = doc->getNodeProc(win);
						g_free(gl_profile.viewer.last_file);
						gl_profile.viewer.last_file = g_strdup(doc->path);
					}
				}
			}
			break;

		case CH_EXIT:
			HypfindFinish(event->msg[3], event->msg[4]);
			event->mwhich &= ~MU_MESAG;
			break;
		}

	} else if (event->mwhich & MU_KEYBD)
	{
		_WORD ascii = event->key;
		_WORD kstate = event->kstate;

		ConvertKeypress(&ascii, &kstate);

		ascii = ascii & 0xff;

		if (kstate & KbCTRL)
		{
			if (ascii == 'W')
			{
				short global_cycle = (kstate & KbSHIFT ? !gl_profile.viewer.av_window_cycle : gl_profile.viewer.av_window_cycle);

				if (av_server_cfg != 0 && global_cycle)
				{
					SendAV_SENDKEY(event->kstate, event->key);
					event->mwhich &= ~MU_KEYBD;
				} else
				{
					CycleItems();
					event->mwhich &= ~MU_KEYBD;
				}
			}
		}
	}
}



/*******************************************************/
/****** menu selection                                 */
/*******************************************************/
#if USE_MENU
void SelectMenu(short title, short entry)
{
	switch (title)
	{
	case ME_PROGRAM:
		switch (entry)
		{
		case ME_ABOUT:
			OpenDialog(HandleAbout, about_tree, rs_string(WDLG_ABOUT), -1, -1, NULL);
			break;
		}
		break;
	case ME_FILE:
		switch (entry)
		{
		case ME_QUIT:
			doneFlag = TRUE;
			break;
		}
		break;
	}
}
#endif



/*******************************************************/
/****** dialog objects with extended edit fields  ******/
/*******************************************************/
#if USE_LONGEDITFIELDS
LONG_EDIT long_edit[] = {
	{ MAIN, MA_EDIT, 20 },
	{ ABOUT, AB_EDIT, 40 }
};

short long_edit_count = (short) (sizeof(long_edit) / sizeof(LONG_EDIT));
#endif



/*******************************************************/
/****** Drag&Drop protokol                        ******/
/*******************************************************/
#if USE_DRAGDROP
/*
	Hier koennen je nach Zielobjekt <obj> die D&D-Daten <data> anders
	ausgewertet werden. <data> hat eines der gewuenschten Formate
	(<format>).
*/
void DD_Object(DIALOG *dial, GRECT *rect, OBJECT *tree, short obj, char *data, unsigned long format)
{
	char **argv;
	int i;
	
	UNUSED(format);
	UNUSED(obj);
	UNUSED(tree);
	UNUSED(rect);
	UNUSED(dial);
	
	argv = split_av_parameter(data);
	if (argv)
	{
		for (i = 0; argv[i]; i++)
			;
	}
	g_strfreev(argv);
}


/*
 * select target format depending on object tree and object
 */
void DD_DialogGetFormat(OBJECT *tree, short obj, unsigned long format[])
{
	short i;

	UNUSED(obj);
	UNUSED(tree);

	for (i = 0; i < MAX_DDFORMAT; i++)
		format[i] = 0L;

	format[0] = 0x41524753L;			/* 'ARGS' */
}
#endif



/*******************************************************/
/****** AV Protokoll                              ******/
/*******************************************************/
/*
 * The server activates the program and passes a command line.
 */
void DoVA_START(_WORD msg[8])
{
	if (modal_items < 0)				/* only do this if we have no modal dialog */
	{
		char *data;

		data = *(char **) &msg[3];
		nf_debugprintf("HypView: got va_start: %s\n", printnull(data));
		if (data != NULL)
		{
			char *arg;
			char **argv;
			char *chapter = NULL;
			char *filename = NULL;
			WINDOW_DATA *win;
			int argc;
			int i;
			gboolean start_if_empty = TRUE;
			
			arg = hyp_conv_to_utf8(hyp_get_current_charset(), data, STR0TERM);
			SendAV_STARTED(msg);
			argv = split_av_parameter(arg);
			argc = g_strv_length(argv);
			i = 0;
			if (argc > 0)
			{
				filename = path_subst(argv[i++]);
				if (strcmp(filename, "-S1") == 0 || strcmp(filename, "-s1") == 0)
				{
					/* start message from remarker or hyptree */
					g_free(filename);
					filename = path_subst(argv[i++]);
					start_if_empty = FALSE;
				} else if (strcmp(filename, "-S0") == 0 || strcmp(filename, "-s0") == 0)
				{
					/* stop message from remarker or hyptree */
					g_free(filename);
					filename = NULL;
					start_if_empty = FALSE;
				}
				if (i < argc)
					chapter = argv[i];
			} else
			{
				filename = NULL;
			}
			win = top_window();
			
			if (empty(filename) && start_if_empty)
			{
				g_free(filename);
				if (win)
				{
					/* no filename, but have window: just put in on top */
					filename = NULL;
					hv_win_open(win);
				} else
				{
					filename = path_subst(empty(gl_profile.viewer.default_file) ? gl_profile.viewer.catalog_file : gl_profile.viewer.default_file);
					chapter = NULL;
				}
			}
			if (empty(chapter))
				chapter = NULL;
			if (!empty(filename))
				win = OpenFileInWindow(win, filename, chapter, HYP_NOINDEX, TRUE, gl_profile.viewer.va_start_newwin, FALSE);
			g_strfreev(argv);
			g_free(filename);
			g_free(arg);
		} else
		{
			WINDOW_DATA *win = top_window();

			SendAV_STARTED(msg);
			if (gl_profile.viewer.va_start_newwin == 2 || !win)
				win = NULL;
			if (!empty(gl_profile.viewer.default_file))
			{
				char *filename = path_subst(gl_profile.viewer.default_file);
				win = OpenFileInWindow(win, filename, NULL, HYP_NOINDEX, FALSE, gl_profile.viewer.va_start_newwin, FALSE);
				g_free(filename);
			} else
			{
				SelectFileLoad(win);
			}
		}
	} else
	{
		SendAV_STARTED(msg);
	}
}


void DoVA_DRAGACCWIND(_WORD msg[8])
{
	if (modal_items < 0)		/* only do this if we have no modal dialog */
	{
		char *data;

		data = *(char **) &msg[6];
		if (data != NULL)
		{
			char *arg;
			WINDOW_DATA *win;
			char **argv;
			
			HYP_DBG(("VA_DRAGACCWIND from %d: <%s>", msg[1], printnull(data)));
			arg = hyp_conv_to_utf8(hyp_get_current_charset(), data, STR0TERM);
			argv = split_av_parameter(arg);

			win = find_window_by_whandle(msg[3]);
			if (win && win->owner != gl_apid)
				win = NULL;
			
			if (argv && !empty(argv[0]))
				win = OpenFileInWindow(win, argv[0], NULL, HYP_NOINDEX, FALSE, gl_profile.viewer.va_start_newwin, FALSE);
			g_strfreev(argv);
			g_free(arg);
		}
	}
}
