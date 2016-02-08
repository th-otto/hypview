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
#include "av.h"
#include "tos/mem.h"
#include "hypdebug.h"


/*
 * basic AV-Protokoll: register to server
 */

static _WORD av_server_id = -1;			/* program ID of server */
long av_server_cfg = 0;

static char *av_name = NULL;			/* our program name */
/* for AV_STARTPROG */
static char *av_parameter = NULL;
static void (*va_progstart)(_WORD ret);


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * work around a bug in XaAES that changes our current directory
 */
_WORD shel_xwrite(_WORD sh_wdoex, _WORD sh_wisgr, _WORD sh_wiscr, const void *sh_wpcmd, const char *sh_wptail)
{
	_UBYTE path[PATH_MAX];
	_WORD ret;
	_WORD drive;
	
	drive = Dgetdrv();
	if (Dgetcwd(path, drive + 1, (int)sizeof(path)) < 0)
		Dgetpath(path, drive + 1);
	ret = shel_write(sh_wdoex, sh_wisgr, sh_wiscr, sh_wpcmd, sh_wptail);
	Dsetdrv(drive);
	Dsetpath(path);
	return ret;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL appl_xsearch(_WORD stype, char *name, _WORD *type, _WORD *id)
{
	_BOOL has_appl_search;
	_WORD dummy, info;
	
	has_appl_search = gl_ap_version >= 0x400 || __magix >= 0x200 ||
		(appl_xgetinfo(4, &dummy, &dummy, &info, &dummy) && info);
	if (has_appl_search)
		return appl_search(stype, name, type, id) != 0;
	*type = 0;
	*id = -1;
	*name = '\0';
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL Protokoll_Send(_WORD apid, _UWORD prot, _UWORD a1, _UWORD a2, _UWORD a3, _UWORD a4, _UWORD a5)
{
	_WORD message[8];
	
	if (apid >= 0)
	{
		message[0] = prot;
		message[1] = gl_apid;
		message[2] = 0;
		message[3] = a1;
		message[4] = a2;
		message[5] = a3;
		message[6] = a4;
		message[7] = a5;
		if (appl_write(apid, 16, message) > 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL Protokoll_Broadcast(_WORD *message, _BOOL send_to_self)
{
	_WORD info, dummy;
	char name[9];
	_WORD id, type;
	
	message[1] = gl_apid;
	message[2] = 0;
	/* is shel_write() broadcast available? */
	if (_AESversion >= 0x400 ||
		__magix >= 0x500 ||
		(appl_xgetinfo(AES_SHELL, &info, &dummy, &dummy, &dummy) && ((info & 0xff) >= SHW_GLOBMSG)))
	{
		shel_xwrite(SHW_GLOBMSG, 0, 1, (char *)message, NULL);
		if (send_to_self)
			appl_write(gl_apid, 16, message);
	} else if (appl_xsearch(APP_FIRST, name, &type, &id))
	{
		do
		{
			if ((id != gl_apid) || send_to_self)
			{
				appl_write(id, 16, message);
			}
		} while (appl_xsearch(APP_NEXT, name, &type, &id));
	} else
	{
		/*
		 * there is no safe way to find out how
		 * many accessories are active in SingleTOS,
		 * and sending to non-existing apps
		 * sometimes crashes AES
		 */
		for (id = 0; id <= gl_apid; id++)
		{
			if ((id != gl_apid) || send_to_self)
			{
				appl_write(id, 16, message);
			}
		}
		return FALSE;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void appl_makeappname(char *app_name, const char *p)
{
	_WORD i;
	
	for (i = 0; i < 8 && p[i] != '\0' && p[i] != '.'; i++)
	{
		app_name[i] = toupper(p[i]);
	}
	for (; i < 8; i++)
		app_name[i] = ' ';
	app_name[8] = '\0';
}

/*** ---------------------------------------------------------------------- ***/

_WORD appl_locate(const char *pathlist, _BOOL startit)
{
	char app_name[9];
	_WORD id = -1;
	const char *p, *pend;
	char *path;
	
	id = -1;
	pend = pathlist;
	while (pend != NULL && id < 0)
	{
		p = strchr(pend, ';');
		if (p == NULL)
			p = strchr(pend, ',');
		if (p == NULL)
		{
			path = g_strdup(pend);
			pend = NULL;
		} else
		{
			path = g_strndup(pend, p - pend);
			pend = p + 1;
		}
		p = hyp_basename(path);
		appl_makeappname(app_name, p);
		g_free(path);
		id = appl_find(app_name);
	}
	if (id < 0 && startit && (_AESnumapps != 1 || !_app))
	{
		pend = pathlist;
		while (pend != NULL && id < 0)
		{
			p = strchr(pend, ';');
			if (p == NULL)
				p = strchr(pend, ',');
			if (p == NULL)
			{
				path = g_strdup(pend);
				pend = NULL;
			} else
			{
				path = g_strndup(pend, p - pend);
				pend = p + 1;
			}
			if (path[0] == '*' && path[1] == ':')
				path[0] = GetBootDrive();
			if (__magix)
			{
				id = shel_xwrite(SHW_EXEC, 1, SHW_PARALLEL, path, "\0");
				evnt_timer_gemlib(500);
			} else
			{
				id = shel_xwrite(SHW_EXEC, 1, 1, path, "\0");
			}
			g_free(path);
		}
	}
	return id;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server for the configuration string thet was set with
 * AV_STATUS
 */
void SendAV_GETSTATUS(void)
{
	Protokoll_Send(av_server_id, AV_GETSTATUS, 0, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * send a configuration string that is saved by the server
 */
void SendAV_STATUS(const char *string)
{
	g_free_shared(av_parameter);
	av_parameter = g_strdup_shared(string);
	if (av_parameter != NULL)
	{
		Protokoll_Send(av_server_id, AV_STATUS, PROT_NAMEPTR(av_parameter), 0, 0, 0);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * send a key event that was not handled
 */
void SendAV_SENDKEY(short kbd_state, short code)
{
	Protokoll_Send(av_server_id, AV_SENDKEY, kbd_state, code, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server about default font to use for displaying text
 */
void SendAV_ASKFILEFONT(void)
{
	Protokoll_Send(av_server_id, AV_ASKFILEFONT, 0, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server about font to use for console text
 */
void SendAV_ASKCONFONT(void)
{
	Protokoll_Send(av_server_id, AV_ASKCONFONT, 0, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server about currently selected object
 */
void SendAV_ASKOBJECT(void)
{
	Protokoll_Send(av_server_id, AV_ASKOBJECT, 0, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * tell server to open console window
 */
void SendAV_OPENCONSOLE(void)
{
	Protokoll_Send(av_server_id, AV_OPENCONSOLE, 0, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * tell server to open a directory window
 */
void SendAV_OPENWIND(const char *path, const char *wildcard)
{
	char *ptr2;
	
	g_free_shared(av_parameter);
	av_parameter = g_strdup2_shared(path, wildcard, &ptr2);
	if (av_parameter != NULL)
	{
		Protokoll_Send(av_server_id, AV_OPENWIND, PROT_NAMEPTR(av_parameter), PROT_NAMEPTR(ptr2), 0);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * create a quoted command line suitable for use
 * by drag&drop protokoll, or AV protokoll.
 * Quote arguments only when neccessary,
 * some servers (like GEMINI) otherwise don't
 * parse the parameters correctly
 */
char *av_cmdline(const char *const argv[], gboolean incl_argv0)
{
	int i;
	size_t cmlen = 0;
	char *cmd, *dst;
	const char *src;
	int first;
	
	first = incl_argv0 ? 0 : 1;
	if (argv == NULL || argv[0] == NULL || argv[first] == NULL)
		return NULL;
	for (i = first; argv[i]; i++)
		cmlen += strlen(argv[i]) * 2 + 3;
	cmd = g_new(char, cmlen);
	if (cmd == NULL)
		return NULL;
	dst = cmd;
	for (i = first; argv[i]; i++)
	{
		if (i > first)
			*dst++ = ' ';
		src = argv[i];
		if (strchr(src, '\'') || strchr(src, ' '))
		{
			*dst++ = '\'';
			while (*src)
			{
				if (*src == '\'')
					*dst++ = '\'';
				*dst++ = *src++;
			}
			*dst++ = '\'';
		} else
		{
			while (*src)
			{
				*dst++ = *src++;
			}
		}
	}
	*dst = '\0';
	return cmd;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * tell server to open a document or start a program
 */
void SendAV_STARTPROG(const char *path, const char *commandline, void (*progstart)(_WORD ret))
{
	char *ptr2;
	
	g_free_shared(av_parameter);
	av_parameter = g_strdup2_shared(path, commandline, &ptr2);
	if (av_parameter != NULL || path == NULL)
	{
		va_progstart = progstart;
		Protokoll_Send(av_server_id, AV_STARTPROG, PROT_NAMEPTR(av_parameter), PROT_NAMEPTR(ptr2), 0x1234);
	}
}

/*** ---------------------------------------------------------------------- ***/

void SendVA_START(_WORD dst_app, const char *path)
{
	g_free_shared(av_parameter);
	av_parameter = g_strdup_shared(path);
	if (av_parameter != NULL || path == NULL)
	{
		Protokoll_Send(dst_app, VA_START, PROT_NAMEPTR(av_parameter), PROT_NAMEPTR(NULL), 0);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * register a new window with server (for window-cycle and  Drag&Drop)
 */
void SendAV_ACCWINDOPEN(short handle)
{
	Protokoll_Send(av_server_id, AV_ACCWINDOPEN, handle, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * uregister a window handle
 */
void SendAV_ACCWINDCLOSED(short handle)
{
	Protokoll_Send(av_server_id, AV_ACCWINDCLOSED, handle, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server to copy the file
 */
void SendAV_COPY_DRAGGED(short kbd_state, const char *path)
{
	g_free_shared(av_parameter);
	av_parameter = g_strdup_shared(path);
	if (av_parameter != NULL || path == NULL)
	{
		Protokoll_Send(av_server_id, AV_COPY_DRAGGED, kbd_state, PROT_NAMEPTR(av_parameter), 0, 0);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server to update a directory window
 */
void SendAV_PATH_UPDATE(const char *path)
{
	g_free_shared(av_parameter);
	av_parameter = g_strdup_shared(path);
	if (av_parameter != NULL || path == NULL)
	{
		Protokoll_Send(av_server_id, AV_PATH_UPDATE, PROT_NAMEPTR(av_parameter), 0, 0, 0);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server about object at position x,y
 */
void SendAV_WHAT_IZIT(short x, short y)
{
	Protokoll_Send(av_server_id, AV_WHAT_IZIT, x, y, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * tell server that an object has been dragged to a window
 */
void SendAV_DRAG_ON_WINDOW(short x, short y, short kbd_state, const char *path)
{
	g_free_shared(av_parameter);
	av_parameter = g_strdup_shared(path);
	if (av_parameter != NULL || path == NULL)
	{
		Protokoll_Send(av_server_id, AV_DRAG_ON_WINDOW, x, y, kbd_state, PROT_NAMEPTR(av_parameter));
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * reply to VA_START
 */
void SendAV_STARTED(const _WORD *msg)
{
	Protokoll_Send(msg[1], AV_STARTED, msg[3], msg[4], msg[5], msg[6], msg[7]);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Tell server to open a window
 */
void SendAV_XWIND(const char *path, const char *wild_card, short bits)
{
	char *ptr2;
	
	g_free_shared(av_parameter);
	av_parameter = g_strdup2_shared(path, wild_card, &ptr2);
	if (av_parameter != NULL || path == NULL)
	{
		Protokoll_Send(av_server_id, AV_XWIND, PROT_NAMEPTR(av_parameter), PROT_NAMEPTR(ptr2), bits);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Tell server to start a viewer registered for a file
 */
void SendAV_VIEW(const char *path)
{
	g_free_shared(av_parameter);
	av_parameter = g_strdup_shared(path);
	if (av_parameter != NULL || path == NULL)
	{
		Protokoll_Send(av_server_id, AV_XWIND, PROT_NAMEPTR(av_parameter), 0, 0, 0);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Tell server to show the file-/directory information
 */
void SendAV_FILEINFO(const char *path)
{
	g_free_shared(av_parameter);
	av_parameter = g_strdup_shared(path);
	if (av_parameter != NULL || path == NULL)
	{
		Protokoll_Send(av_server_id, AV_FILEINFO, PROT_NAMEPTR(av_parameter), 0, 0, 0);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Ask server to copy the files
 */
void SendAV_COPYFILE(const char *file_list, const char *dest_path, short bits)
{
	char *ptr2;
	
	g_free_shared(av_parameter);
	av_parameter = g_strdup2_shared(file_list, dest_path, &ptr2);
	if (av_parameter != NULL && ptr2 != NULL)
	{
		Protokoll_Send(av_server_id, AV_COPYFILE, PROT_NAMEPTR(av_parameter), PROT_NAMEPTR(ptr2), bits);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Ask server to delete the files
 */
void SendAV_DELFILE(const char *file_list)
{
	g_free_shared(av_parameter);
	av_parameter = g_strdup_shared(file_list);
	if (av_parameter != NULL)
	{
		Protokoll_Send(av_server_id, AV_DELFILE, PROT_NAMEPTR(av_parameter), 0, 0, 0);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Tell server where to open the next window
 */
void SendAV_SETWINDPOS(short x, short y, short w, short h)
{
	Protokoll_Send(av_server_id, AV_SETWINDPOS, x, y, w, h, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Tell server about mouse event that was not handled
 */
void SendAV_SENDCLICK(EVNTDATA *m, short ev_return)
{
	Protokoll_Send(av_server_id, AV_SENDCLICK, m->x, m->y, m->bstate, m->kstate, ev_return);
}

/*** ---------------------------------------------------------------------- ***/

/**************************************************
 * VA-commands, notification messages from server *
 **************************************************/

/*
 * reply to AV_PROTOKOLL
 */
static void DoVA_PROTOSTATUS(_WORD msg[8])
{
	union {
		short msg[2];
		long l;
	} cfg;
	
	cfg.msg[0] = msg[3];
	cfg.msg[1] = msg[4];
	if (msg[1] == av_server_id)
	{
		av_server_cfg = cfg.l;
		HYP_DBG(("AV-Server name: '%s' protocol: %lx", printnull(*(char **) &msg[6]), av_server_cfg));
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/************************************************
 * AV-commands, status messages send to server  *
 ************************************************/

/*
 * register to server (including type of protokoll)
 */
void DoAV_PROTOKOLL(short flags)
{
	char servername[9];
	_WORD type;
	_WORD msg[8];
	
	if (av_server_id < 0)
	{
		av_server_id = appl_locate(getenv("AVSERVER"), FALSE);
		if (av_server_id < 0)
			av_server_id = appl_locate("AVSERVER", FALSE);
		if (av_server_id < 0)
			if (!appl_xsearch(2, servername, &type, &av_server_id))
				av_server_id = -1;
		if (av_server_id < 0 && gl_apid != 0)
			av_server_id = 0;
	}
	
	if (av_name == NULL)
		return;
	/*
	 * send this to ALL applications, not only AV-server,
	 * since they might need to know wether we understand quoting
	 */
	msg[0] = AV_PROTOKOLL;
	msg[1] = gl_apid;
	msg[2] = 0;
	msg[3] = flags;
	msg[4] = 0;
	msg[5] = 0;
	msg[6] = (_UWORD)((_ULONG)(av_name) >> 16);
	msg[7] = (_UWORD)(_ULONG)(av_name);
	Protokoll_Broadcast(msg, FALSE);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * unregister from server
 */
void DoAV_EXIT(void)
{
	Protokoll_Send(av_server_id, AV_EXIT, gl_apid, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

void DoVA_Message(_WORD msg[8])
{
	switch (msg[0])
	{
	case VA_PROTOSTATUS:					/* server acknowledges registration */
		DoVA_PROTOSTATUS(msg);
		break;
	case AV_STARTED:
		g_free_shared(av_parameter);
		av_parameter = NULL;
		break;
	case VA_PROGSTART:
		g_free_shared(av_parameter);
		av_parameter = NULL;
		if (va_progstart)
		{
			_WORD ret;
			
			if (msg[3] == 0)
				ret = -1;
			else
				ret = msg[4];
			va_progstart(ret);
			va_progstart = 0;
		}
		break;
	case AV_PROTOKOLL:
		Protokoll_Send(msg[1], VA_PROTOSTATUS,
			VA_PROT_SENDKEY | VA_PROT_ACCWINDOPEN | VA_PROT_OPENWIND | VA_PROT_PATH_UPDATE | VA_PROT_DRAG_ON_WINDOW | VA_PROT_EXIT | VA_PROT_STARTED | VA_PROT_QUOTING,
			0, 0,
			PROT_NAMEPTR(av_name));
		break;
	default:
		break;
	}
}

/*** ---------------------------------------------------------------------- ***/

void va_proto_init(const char *myname)
{
	if (myname && av_name == NULL)
	{
		av_name = g_alloc_shared(9);
		if (av_name == NULL)
			return;
		appl_makeappname(av_name, myname);
	}
	DoAV_PROTOKOLL(AV_PROTOKOLL_QUOTING | AV_PROTOKOLL_START | AV_PROTOKOLL_STARTED);
}

/*** ---------------------------------------------------------------------- ***/

void va_proto_exit(void)
{
	DoAV_EXIT();
}
