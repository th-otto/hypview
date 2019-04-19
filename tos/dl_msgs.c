#include "hv_defs.h"

#include "hypdebug.h"
#include "tos/av.h"
#include "tos/bgh.h"
#include "tos/mem.h"
#include "tos/xacc.h"
#include "tos/bubble.h"
#include "tos/dhst.h"
#include "tos/gscript.h"
#include "tos/pchelp.h"
#include "tos/olga.h"
#include "tos/ssp.h"
#include "tos/seproto.h"
#include "tos/viewprot.h"


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
		case ACC_CLOSE: return "ACC_CLOSE";
		case ACC_ACC: return "ACC_ACC";
		case ACC_EXIT: return "ACC_EXIT";
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
		case ACC_REQUEST: return "ACC_REQUEST";
		case ACC_REPLY: return "ACC_REPLY";

		case AC_HELP: return "AC_HELP";
		/* case AC_REPLY: return "AC_REPLY"; same as ACC_CLOSE */
		/* case AC_VERSION: return "AC_VERSION"; same as ACC_ACC */
		/* case AC_COPY: return "AC_COPY"; same as ACC_EXIT */

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


static char *appl_xname(_WORD id, char *name)
{
	_WORD fid;
	_WORD type;
	_BOOL found = FALSE;
	if (appl_xsearch(APP_FIRST, name, &type, &fid))
	{
		do {
			if (fid == id)
				found = TRUE;
		} while (!found && appl_xsearch(APP_NEXT, name, &type, &fid));
	} else
	{
		strcpy(name, "?");
		sprintf(name + 2, "\\%d", id);
		if (appl_find(name) == 0)
			found = TRUE;
	}
	if (!found)
		sprintf(name, "<%d>", id);
	return name;
}


void gem_print_message(const _WORD *msg)
{
	char name[20];
	
#define str(w) printnull(*((const char *const*)&msg[w]))
#define lng(w) (*((const unsigned long *)&msg[w]))

	nf_debugprintf("%s(from %s): %s(",
		gl_program_name,
		appl_xname(msg[1], name),
		gem_message_name(msg[0]));
	switch ((_UWORD)msg[0])
	{
	case MN_SELECTED:
	case CH_EXIT:
	case WM_HSLID:
	case WM_VSLID:
		nf_debugprintf("%d, %d", msg[3], msg[4]);
		break;
	case WM_REDRAW:
	case WM_MOVED:
	case WM_UNICONIFY:
	case WM_SIZED:
	case WM_ICONIFY:
	case WM_ALLICONIFY:
	case WM_REPOSED:
		nf_debugprintf("%d, {%d, %d, %d, %d}", msg[3], msg[4], msg[5], msg[6], msg[7]);
		break;
	case WM_TOPPED:
	case WM_CLOSED:
	case WM_FULLED:
	case WM_BOTTOM:
	case WM_UNSHADED:
	case WM_SHADED:
	case WM_ONTOP:
	case WM_OFFTOP:
	case WM_UNTOPPED:
	case WM_NEWTOP:
	case WM_BACKDROP:
		nf_debugprintf("%d", msg[3]);
		break;
	case WM_ARROWED:
		nf_debugprintf("%d, %d, %d, %d, %d", msg[3], msg[4], msg[5], msg[6], msg[7]);
		break;
	case WM_TOOLBAR:
		nf_debugprintf("%d, %d, %d, $%x, %d", msg[3], msg[4], msg[5], msg[6], msg[7]);
		break;
	case AC_OPEN:
		nf_debugprintf("%d", msg[4]);
		break;
	case AC_CLOSE:
	case RESCHG_COMPLETED:
		nf_debugprintf("%d", msg[3]);
		break;
	case AP_DRAGDROP:
		nf_debugprintf("%d, %d, %d, $%x, '%c%c'", msg[3], msg[4], msg[5], msg[6], (msg[7] >> 8) & 0xff, msg[7] & 0xff);
		break;
	case AP_TERM:
		nf_debugprintf("%s", gem_message_name(msg[5]));
		break;
	case CT_KEY:
	case SH_WDRAW:
		nf_debugprintf("$%04x", msg[3]);
		break;
	case SHUT_COMPLETED:
		break;
	case AP_RESCHG:
	case AP_TFAIL:
	case WM_ISTOP:
	/* case CT_UPDATE: */
	/* case CT_MOVE: */
	case CT_NEWTOP:
	/* case CT_SWITCH: */
	case SH_EXIT:
	case SH_START:
	case SC_CHANGED:
	case SM_M_SPECIAL:
	case SM_M_RES2:
	case SM_M_RES3:
	case SM_M_RES4:
	case SM_M_RES5:
	case SM_M_RES6:
	case SM_M_RES7:
	case SM_M_RES8:
	case SM_M_RES9:
	case WM_MOUSEWHEEL:
	default:
		nf_debugprintf("$%04x, $%04x, $%04x, $%04x, $%04x",
			msg[3],
			msg[4],
			msg[5],
			msg[6],
			msg[7]);
		break;

	case PRN_CHANGED:
		nf_debugprintf("%d, %d", msg[3], msg[4]);
		break;
	case FNT_CHANGED:
	case COLORS_CHANGED:
	case PA_EXIT:
		break;
	case THR_EXIT:
		nf_debugprintf("%d, %ld", msg[3], lng(4));
		break;
	case WM_WHEEL:
		nf_debugprintf("%d, %d, %d, $%x, $%x", msg[3], msg[4], msg[5], msg[6], msg[7]);
		break;
	
	case AV_PROTOKOLL:
		nf_debugprintf("$%x, %s", msg[3], str(6));
		break;
	case VA_PROTOSTATUS:
		nf_debugprintf("$%x, $%x, $%x, %s", msg[3], msg[4], msg[5], str(6));
		break;
	case AV_GETSTATUS:
		break;
	case AV_STATUS:
	case VA_SETSTATUS:
		nf_debugprintf("%s", str(3));
		break;
	case AV_SENDCLICK:
		nf_debugprintf("%d, %d, $%x, $%x, %d", msg[3], msg[4], msg[5], msg[6], msg[7]);
		break;
	case AV_SENDKEY:
		nf_debugprintf("$%x, $%x", msg[3], msg[4]);
		break;
	case VA_START:
	case VA_OBJECT:
	case VA_PATH_UPDATE:
	case VA_COPY_COMPLETE:
	case AV_PATH_UPDATE:
	case AV_STARTED:
	case AV_FILEINFO:
	case VA_FILECHANGED:
	case AV_DELFILE:
		nf_debugprintf("%s", str(3));
		break;
	case AV_ASKFILEFONT:
	case AV_ASKCONFONT:
	case AV_ASKOBJECT:
	case AV_OPENCONSOLE:
	case VA_WINDOPEN:
		break;
	case VA_FILEFONT:
	case VA_CONFONT:
	case AV_WHAT_IZIT:
		nf_debugprintf("%d, %d", msg[3], msg[4]);
		break;
	case VA_CONSOLEOPEN:
	case AV_ACCWINDOPEN:
	case AV_ACCWINDCLOSED:
	case VA_DRAG_COMPLETE:
	case AV_EXIT:
	case VA_XOPEN:
	case VA_VIEWED:
	case VA_FILECOPIED:
	case VA_FILEDELETED:
		nf_debugprintf("%d", msg[3]);
		break;
	case AV_OPENWIND:
		nf_debugprintf("%s, %s", str(3), str(5));
		break;
	case AV_STARTPROG:
		nf_debugprintf("%s, %s, $%x", str(3), str(5), msg[7]);
		break;
	case VA_PROGSTART:
		nf_debugprintf("%d, %d, $%x", msg[3], msg[4], msg[7]);
		break;
	case VA_DRAGACCWIND:
		nf_debugprintf("%d, %d, %d, %s", msg[3], msg[4], msg[5], str(6));
		break;
	case AV_COPY_DRAGGED:
		nf_debugprintf("$%x, %s", msg[3], str(4));
		break;
	case VA_THAT_IZIT:
		nf_debugprintf("%d, %d, %s", msg[3], msg[4], str(5));
		break;
	case AV_DRAG_ON_WINDOW:
		nf_debugprintf("%d, %d, $%x, %s", msg[3], msg[4], msg[5], str(6));
		break;
	case VA_FONTCHANGED:
		nf_debugprintf("%d, %d, %d, %d", msg[3], msg[4], msg[5], msg[6]);
		break;
	case AV_XWIND:
	case AV_COPYFILE:
		nf_debugprintf("%s, %s, $%x", str(3), str(5), msg[7]);
		break;
	case AV_VIEW:
		nf_debugprintf("%s, %d", str(3), msg[5]);
		break;
	case AV_SETWINDPOS:
		nf_debugprintf("{%d, %d, %d, %d}", msg[3], msg[4], msg[5], msg[6]);
		break;
	
	/* case ACC_OPEN: */
	case ACC_CLOSE:
	case ACC_EXIT:
		break;
	case ACC_ID:
	case ACC_ACC:
		nf_debugprintf("$%x, %s, %d, %d", msg[3], str(4), msg[6], msg[7]);
		break;
	case ACC_ACK:
	case ACC_GETFIELDS:
		nf_debugprintf("%d", msg[3]);
		break;
	case ACC_TEXT:
		nf_debugprintf("%s", str(4));
		break;
	case ACC_KEY:
		nf_debugprintf("$%x, $%x", msg[3], msg[4]);
		break;
	case ACC_META:
	case ACC_IMG:
		nf_debugprintf("%d, %s, %ld", msg[3], str(4), lng(6));
		break;
	case ACC_REQUEST:
	case ACC_REPLY:
		nf_debugprintf("$%x, $%lx, $%lx", msg[3], lng(4), lng(6));
		break;
	case ACC_GETDSI:
	case ACC_DSINFO:
	case ACC_FILEINFO:
	case ACC_FIELDINFO:
	case ACC_FORCESDF:
	case ACC_GETSDF:
		nf_debugprintf("$%lx", lng(4));
		break;

	case AC_HELP:
		nf_debugprintf("%s, $%x, %s", str(3), msg[5], str(6));
	/* case AC_REPLY: */
	/* case AC_VERSION: */
	/* case AC_COPY: */
		break;
	
	case BUBBLEGEM_REQUEST:
		nf_debugprintf("%d, %d, %d, $%x", msg[3], msg[4], msg[5], msg[6]);
		break;
	case BUBBLEGEM_SHOW:
		nf_debugprintf("%d, %d, %s, $%x", msg[3], msg[4], str(5), msg[7]);
		break;
	case BUBBLEGEM_ACK:
		nf_debugprintf("%s, $%x", str(5), msg[7]);
		break;
	case BUBBLEGEM_ASKFONT:
		break;
	case BUBBLEGEM_FONT:
		nf_debugprintf("%d, %d", msg[3], msg[4]);
		break;
	case BUBBLEGEM_HIDE:
		break;
	
	case DHST_ADD:
		nf_debugprintf("$%lx", lng(3));
		break;
	case DHST_ACK:
		nf_debugprintf("$%lx, %d", lng(3), msg[7]);
		break;
		
	case GS_REQUEST:
	case GS_REPLY:
	case GS_COMMAND:
	case GS_ACK:
	case GS_QUIT:
	case GS_OPENMACRO:
	case GS_MACRO:
	case GS_WRITE:
	case GS_CLOSEMACRO:
		break;

	case 0x995:
		break;

	case 0x1000:
	case 0x1001:
	case 0x1002:
	case 0x1003:
	case 0x1004:
	case 0x1005:
	case 0x1006:
	case 0x1007:
	case 0x1008:
	case 0x1009:
	case 0x100a:
	case 0x100b:
	case 0x100c:
		break;

	case OLGA_INIT:
	case OLGA_UPDATE:
	case OLGA_ACK:
	case OLGA_RENAME:
	case OLGA_OPENDOC:
	case OLGA_CLOSEDOC:
	case OLGA_LINK:
	case OLGA_UNLINK:
	case OLGA_UPDATED:
	case OLGA_RENAMELINK:
	case OLGA_LINKRENAMED:
	case OLGA_GETOBJECTS:
	case OLGA_OBJECTS:
	case OLGA_BREAKLINK:
	case OLGA_LINKBROKEN:
	case OLGA_START:
	case OLGA_GETINFO:
	case OLGA_INFO:
	case OLGA_IDLE:
	case OLGA_ACTIVATE:
	case OLGA_EMBED:
	case OLGA_EMBEDDED:
	case OLGA_UNEMBED:
	case OLGA_GETSETTINGS:
	case OLGA_SETTINGS:
	case OLGA_REQUESTNOTIFICATION:
	case OLGA_RELEASENOTIFICATION:
	case OLGA_NOTIFY:
	case OLGA_NOTIFIED:
	case OLGA_SERVERTERMINATED:
	case OLGA_CLIENTTERMINATED:
	case OLGA_INPLACEUPDATE:
	case OLGA_ID4UPDATE:
	case OLGA_GETEXTENSION:
	case OLGA_EXTENSION:
	case OLGA_GETSERVERPATH:
	case OLGA_SERVERPATH:
		break;

	case OLE_INIT:
	case OLE_EXIT:
	case OLE_NEW:
		break;
	
	case SSP_SRASR:
	case SSP_SSIR:
	case SSP_SPASI:
	case SSP_SSUR:
	case SSP_SPASA:
	case SSP_SSA:
		break;

	case SE_INIT:
	case SE_OK:
	case SE_ACK:
	case SE_OPEN:
	case SE_ERROR:
	case SE_ERRFILE:
	case SE_PROJECT:
	case SE_QUIT:
	case SE_TERMINATE:
	case SE_CLOSE:
	case ES_INIT:
	case ES_OK:
	case ES_ACK:
	case ES_COMPILE:
	case ES_MAKE:
	case ES_MAKEALL:
	case ES_LINK:
	case ES_EXEC:
	case ES_MAKEEXEC:
	case ES_PROJECT:
	case ES_QUIT:
	case ES_SHLCTRL:
		break;

	case VIEW_FILE:
	case VIEW_FAILED:
	case VIEW_OPEN:
	case VIEW_CLOSED:
	case VIEW_DATA:
	case VIEW_GETMFDB:
	case VIEW_SETMFDB:
	case VIEW_MFDB:
		break;
	
	case FONT_CHANGED:
	case FONT_SELECT:
		nf_debugprintf("%d, %d, %d, %d, $%x", msg[3], msg[4], msg[5], msg[6], msg[7]);
		break;
	case FONT_ACK:
		nf_debugprintf("%d", msg[3]);
		break;
	case XFONT_CHANGED:
		nf_debugprintf("%ld, %d, %d, $%x", lng(3), msg[5], msg[6], msg[7]);
		break;
	}
	nf_debugprintf(")\n");

#undef str
#undef lng
}
