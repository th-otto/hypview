/*****************************************************************************
 *		TOS/DBGWIN.C
 *****************************************************************************/

#include "hv_defs.h"
#include "dbgwin.h"

#if DBG_WIND

#include <stdarg.h>
#include "hypdebug.h"
#include <mint/arch/nf_ops.h>

#undef wind_open
#undef wind_create
#undef wind_close
#undef wind_set
#undef wind_set_ptr
#undef wind_set_str
#undef wind_get
#undef wind_get_grect
#undef wind_delete

typedef enum {
	W_DBG_UNUSED,
	W_DBG_CREATED,
	W_DBG_OPEN
} w_dbg_state;

#define N_DBG_HNDLS 1000


static struct {
	w_dbg_state state;
	_WORD kind;
	_WORD set;
} w_dbg_windows[N_DBG_HNDLS];
static const char *wf_first_file;
static int wf_first_line;

static void dbg_wind_alert(const char *format, ...)
{
	char msg[1024];
	char buf[1024];
	va_list args;
	
	va_start(args, format);
	vsprintf(msg, format, args);
	va_end(args);
	sprintf(buf, "[1][%s][Ok]", msg);
	form_alert(1, buf);
}


static void dbg_wind_init(void)
{
	static int inited;
	if (inited)
		return;
	w_dbg_windows[0].state = W_DBG_OPEN;
	w_dbg_windows[0].kind = 0;
	inited = 1;
}


_WORD dbg_wind_open(_WORD hndl, _WORD x, _WORD y, _WORD w, _WORD h, const char *file, int line)
{
	_WORD res;

	dbg_wind_init();
	
	if (hndl < 0 || hndl >= N_DBG_HNDLS)
	{
		dbg_wind_alert("%s|line %d|wind_open(%d)|hndl out of range", hyp_basename(file), line, hndl);
	} else if (w_dbg_windows[hndl].state != W_DBG_CREATED)
	{
		dbg_wind_alert("%s|line %d|wind_open(%d)|%s", hyp_basename(file), line, hndl, w_dbg_windows[hndl].state == W_DBG_UNUSED ? "hndl invalid" : "already open");
	} else
	{
		if ((w_dbg_windows[hndl].kind & NAME) &&
			!(w_dbg_windows[hndl].set & NAME))
		{
			dbg_wind_alert("%s|line %d|wind_open(%d)|NAME not yet set", hyp_basename(file), line, hndl);
		}
		if ((w_dbg_windows[hndl].kind & INFO) &&
			!(w_dbg_windows[hndl].set & INFO))
		{
			dbg_wind_alert("%s|line %d|wind_open(%d)|INFO not yet set", hyp_basename(file), line, hndl);
		}
	}
	res = wind_open(hndl, x, y, w, h);
	if (res == 0)
	{
		dbg_wind_alert("%s|line %d|wind_open(%d)|failed", hyp_basename(file), line, hndl);
	} else
	{
		if (hndl >= 0 && hndl < N_DBG_HNDLS)
			w_dbg_windows[hndl].state = W_DBG_OPEN;
	}
	return res;
}


_WORD dbg_wind_create(_WORD kind, _WORD x, _WORD y, _WORD w, _WORD h, const char *file, int line)
{
	_WORD hndl;
	
	dbg_wind_init();
	hndl = wind_create(kind, x, y, w, h);
	if (hndl < 0)
	{
		dbg_wind_alert("%s|line %d|wind_create($%x)|creation failed", hyp_basename(file), line, kind);
	} else
	{
		if (hndl >= 0 && hndl < N_DBG_HNDLS)
		{
			w_dbg_windows[hndl].state = W_DBG_CREATED;
			w_dbg_windows[hndl].kind = kind;
			w_dbg_windows[hndl].set = 0;
		}
	}
	return hndl;
}


_WORD dbg_wind_close(_WORD hndl, const char *file, int line)
{
	_WORD res;

	dbg_wind_init();
	
	if (hndl < 0 || hndl >= N_DBG_HNDLS)
	{
		dbg_wind_alert("%s|line %d|wind_close(%d)|hndl out of range", hyp_basename(file), line, hndl);
	} else
	{
		if (w_dbg_windows[hndl].state != W_DBG_OPEN)
		{
			dbg_wind_alert("%s|line %d|wind_close(%d)|%s", hyp_basename(file), line, hndl, w_dbg_windows[hndl].state == W_DBG_UNUSED ? "hndl invalid" : "not open");
		}
	}
	res = wind_close(hndl);
	if (res == 0)
	{
		dbg_wind_alert("%s|line %d|wind_close(%d)|failed", hyp_basename(file), line, hndl);
	} else
	{
		if (hndl >= 0 && hndl < N_DBG_HNDLS)
			w_dbg_windows[hndl].state = W_DBG_CREATED;
	}
	return res;
}


_WORD dbg_wind_delete(_WORD hndl, const char *file, int line)
{
	_WORD res;

	dbg_wind_init();

	if (hndl < 0 || hndl >= N_DBG_HNDLS)
	{
		dbg_wind_alert("%s|line %d|wind_delete(%d)|hndl out of range", hyp_basename(file), line, hndl);
	} else
	{
		if (w_dbg_windows[hndl].state != W_DBG_CREATED)
		{
			dbg_wind_alert("%s|line %d|wind_delete(%d)|%s", hyp_basename(file), line, hndl, w_dbg_windows[hndl].state == W_DBG_UNUSED ? "hndl invalid" : "still open");
		}
	}
	res = wind_delete(hndl);
	if (res == 0)
	{
		dbg_wind_alert("%s|line %d|wind_delete(%d)|failed", hyp_basename(file), line, hndl);
	} else
	{
		if (hndl >= 0 && hndl < N_DBG_HNDLS)
			w_dbg_windows[hndl].state = W_DBG_UNUSED;
	}
	return res;
}


_WORD dbg_wind_set_ptr(_WORD hndl, _WORD field, void *p1, void *p2, const char *file, int line)
{
	return dbg_wind_set(hndl, field, (_WORD)((long)p1 >> 16), (_WORD)(long)p1, (_WORD)((long)p2 >> 16), (_WORD)(long)p2, file, line);
}


static const char *dbg_wind_setname(_WORD field)
{
	static char buf[10];
	
	switch (field)
	{
		case WF_KIND: return "WF_KIND";
		case WF_NAME: return "WF_NAME";
		case WF_INFO: return "WF_INFO";
		case WF_WORKXYWH: return "WF_WORKXYWH";
		case WF_CURRXYWH: return "WF_CURRXYWH";
		case WF_PREVXYWH: return "WF_PREVXYWH";
		case WF_FULLXYWH: return "WF_FULLXYWH";
		case WF_HSLIDE: return "WF_HSLIDE";
		case WF_VSLIDE: return "WF_VSLIDE";
		case WF_TOP: return "WF_TOP";
		case WF_FIRSTXYWH: return "WF_FIRSTXYWH";
		case WF_NEXTXYWH: return "WF_NEXTXYWH";
		case WF_FIRSTAREAXYWH: return "WF_FIRSTAREAXYWH";
		case WF_NEWDESK: return "WF_NEWDESK";
		case WF_HSLSIZE: return "WF_HSLSIZE";
		case WF_VSLSIZE: return "WF_VSLSIZE";
		case WF_SCREEN: return "WF_SCREEN";
		case WF_COLOR: return "WF_COLOR";
		case WF_DCOLOR: return "WF_DCOLOR";
		case WF_OWNER: return "WF_OWNER";
		case WF_BEVENT: return "WF_BEVENT";
		case WF_BOTTOM: return "WF_BOTTOM";
		case WF_ICONIFY: return "WF_ICONIFY";
		case WF_UNICONIFY: return "WF_UNICONIFY";
		case WF_UNICONIFYXYWH: return "WF_UNICONIFYXYWH";
		case WF_TOOLBAR: return "WF_TOOLBAR";
		case WF_FTOOLBAR: return "WF_FTOOLBAR";
		case WF_NTOOLBAR: return "WF_NTOOLBAR";
		case WF_MENU: return "WF_MENU";
		case WF_WIDGET: return "WF_WIDGET";
		case WF_WHEEL: return "WF_WHEEL";
		case WF_OPTS: return "WF_OPTS";
		case WF_CALCF2W: return "WF_CALCF2W";
		case WF_CALCW2F: return "WF_CALCW2F";
		case WF_CALCF2U: return "WF_CALCF2U";
		case WF_CALCU2F: return "WF_CALCU2F";
		case WF_MAXWORKXYWH: return "WF_MAXWORKXYWH";
		case WF_M_BACKDROP: return "WF_M_BACKDROP";
		case WF_M_OWNER: return "WF_M_OWNER";
		case WF_M_WINDLIST: return "WF_M_WINDLIST";
		case WF_MINXYWH: return "WF_MINXYWH";
		case WF_INFOXYWH: return "WF_INFOXYWH";
		case WF_WIDGETS: return "WF_WIDGETS";
		case WF_USER_POINTER: return "WF_USER_POINTER";
		case WF_WIND_ATTACH: return "WF_WIND_ATTACH";
		case WF_TOPMOST: return "WF_TOPMOST";
		case WF_OBFLAG: return "WF_OBFLAG";
		case WF_OBTYPE: return "WF_OBTYPE";
		case WF_OBSPEC: return "WF_OBSPEC";
		case X_WF_MENU: return "X_WF_MENU";
		case X_WF_DIALOG: return "X_WF_DIALOG";
		case X_WF_DIALWID: return "X_WF_DIALWID";
		case X_WF_DIALHT: return "X_WF_DIALHT";
		case X_WF_DFLTDESK: return "X_WF_DFLTDESK";
		case X_WF_MINMAX: return "X_WF_MINMAX";
		case X_WF_HSPLIT: return "X_WF_HSPLIT";
		case X_WF_VSPLIT: return "X_WF_VSPLIT";
		case X_WF_SPLMIN: return "X_WF_SPLMIN";
		case X_WF_HSLIDE2: return "X_WF_HSLIDE2";
		case X_WF_VSLIDE2: return "X_WF_VSLIDE2";
		case X_WF_HSLSIZE2: return "X_WF_HSLSIZE2";
		case X_WF_VSLSIZE2: return "X_WF_VSLSIZE2";
		case X_WF_DIALFLGS: return "X_WF_DIALFLGS";
		case X_WF_OBJHAND: return "X_WF_OBJHAND";
		case X_WF_DIALEDIT: return "X_WF_DIALEDIT";
		case X_WF_DCOLSTAT: return "X_WF_DCOLSTAT";
		case WF_WINX: return "WF_WINX";
		case WF_WINXCFG: return "WF_WINXCFG";
		case WF_DDELAY: return "WF_DDELAY";
		case WF_SHADE: return "WF_SHADE";
		case WF_STACK: return "WF_STACK";
		case WF_TOPALL: return "WF_TOPALL";
		case WF_BOTTOMALL: return "WF_BOTTOMALL";
		case WF_XAAES: return "WF_XAAES";
	}
	sprintf(buf, "<%d>", field);
	return buf;
}


_WORD dbg_wind_set(_WORD hndl, _WORD field, _WORD a, _WORD b, _WORD c, _WORD d, const char *file, int line)
{
	_WORD res;
	_WORD need;
	_WORD mustbeopen;
	
	dbg_wind_init();
	hyp_debug("dbgwin: %s:%d: wind_set(%d, %s)", hyp_basename(file), line, hndl, dbg_wind_setname(field));
	if (hndl < 0 || hndl >= N_DBG_HNDLS)
	{
		/* Some functions may be allowed without a real window handle */
		switch (field)
		{
		case WF_WHEEL:
		case WF_TOP:
		case WF_TOPMOST:
			break;
		default:
			dbg_wind_alert("%s|line %d|wind_set(%d)|hndl out of range", hyp_basename(file), line, hndl);
			break;
		}
	} else if (w_dbg_windows[hndl].state == W_DBG_UNUSED)
	{
		dbg_wind_alert("%s|line %d|wind_set(%d, %s)|%s", hyp_basename(file), line, hndl, dbg_wind_setname(field), "hndl invalid");
	} else
	{
		need = 0;
		mustbeopen = 0;
		switch (field)
		{
		case WF_BOTTOM:
		case WF_TOP:
			mustbeopen = 1;
			break;
		case WF_HSLIDE:
		case WF_HSLSIZE:
			need = HSLIDE;
			break;
		case WF_VSLIDE:
		case WF_VSLSIZE:
			need = VSLIDE;
			break;
		case WF_KIND:
			w_dbg_windows[hndl].kind = a;
			break;
		}
		if (mustbeopen && w_dbg_windows[hndl].state == W_DBG_CREATED)
		{
			dbg_wind_alert("%s|line %d|wind_set(%d, %s)|%s", hyp_basename(file), line, hndl, dbg_wind_setname(field), "not open");
		}
		if ((w_dbg_windows[hndl].kind & need) != need)
		{
			dbg_wind_alert("%s|line %d|wind_set(%d, %s)|%s", hyp_basename(file), line, hndl, dbg_wind_setname(field), "invalid field");
		}
	}
	res = wind_set(hndl, field, a, b, c, d);
	if (res == 0)
	{
		hyp_debug("dbwgin: %s:%d: wind_set(%d, %s) failed", hyp_basename(file), line, hndl, dbg_wind_setname(field));
		dbg_wind_alert("%s|line %d|wind_set(%d, %s)|%s", hyp_basename(file), line, hndl, dbg_wind_setname(field), "failed");
	} else
	{
		if (hndl >= 0 && hndl < N_DBG_HNDLS)
		{
			switch (field)
			{
			case WF_NAME:
				w_dbg_windows[hndl].set |= NAME;
				break;
			case WF_INFO:
				w_dbg_windows[hndl].set |= INFO;
				break;
			}
		}
	}
	return res;
}


_WORD dbg_wind_get(_WORD hndl, _WORD field, _WORD *a, _WORD *b, _WORD *c, _WORD *d, const char *file, int line)
{
	_WORD res;

	dbg_wind_init();
	if (hndl < 0 || hndl >= N_DBG_HNDLS)
	{
		/* Some functions may be allowed without a real window handle */
		switch (field)
		{
		case WF_BOTTOM:
		case WF_FULLXYWH:
		case WF_TOP:
		case WF_NEWDESK:
		case WF_SCREEN:
		case WF_WHEEL:
		case WF_OPTS:
		case WF_XAAES:
			break;
		default:
			dbg_wind_alert("%s|line %d|wind_get(%d)|hndl out of range", hyp_basename(file), line, hndl);
			break;
		}
	} else if (w_dbg_windows[hndl].state == W_DBG_UNUSED)
	{
		dbg_wind_alert("%s|line %d|wind_get(%d, %s)|%s", hyp_basename(file), line, hndl, dbg_wind_setname(field), "hndl invalid");
	}
	if (field == WF_FIRSTXYWH && wf_first_file != NULL)
	{
		dbg_wind_alert("%s|line %d|wind_get(%d, %s)|already started in|%s line %d", hyp_basename(file), line, hndl, dbg_wind_setname(field), hyp_basename(wf_first_file), wf_first_line);
	}
	res = wind_get(hndl, field, a, b, c, d);
	if (field == WF_FIRSTXYWH || field == WF_NEXTXYWH)
	{
		nf_debugprintf("%s:%d: wind_get(%d, %s): %d {%d,%d,%d,%d}\n", hyp_basename(file), line, hndl, dbg_wind_setname(field), res, *a, *b, *c, *d);
	}
	if (res == 0)
	{
		switch (field)
		{
		case WF_NEXTXYWH:
		case WF_FIRSTXYWH:
			wf_first_file = NULL;
			break;
		default:
			dbg_wind_alert("%s|line %d|wind_get(%d, %s)|%s", hyp_basename(file), line, hndl, dbg_wind_setname(field), "failed");
			break;
		}
	} else
	{
		switch (field)
		{
		case WF_FIRSTXYWH:
			wf_first_file = file;
			wf_first_line = line;
			/* fall through */
		case WF_NEXTXYWH:
			if (*c == 0 || *d == 0)
				wf_first_file = NULL;
			break;
		}
	}
	return res;
}

#endif /* DBG_WIND */
