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


static char *av_name = NULL;			/* our program name */
#define NUM_AV_BUFFERS 10
static char *av_buffers[NUM_AV_BUFFERS];
static int av_par_num;
/* for AV_STARTPROG */
static void (*va_progstart)(_WORD ret);
static void (*va_started)(void);

typedef struct _av_app AV_APP;
struct _av_app {
	AV_APP *next;
	char name[9];
	_WORD apid;
	_WORD av_server_cfg1;
	_WORD av_server_cfg2;
	_WORD av_server_cfg3;
	_WORD av_proto_cfg;
};

static AV_APP *av_server_app;			/* program ID of server */
static AV_APP *av_apps;

#define setmsg(a,d,e,f,g,h) \
	msg[0] = a; \
	msg[1] = gl_apid; \
	msg[2] = 0; \
	msg[3] = d; \
	msg[4] = e; \
	msg[5] = f; \
	msg[6] = g; \
	msg[7] = h

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static char **get_av_buffer(void)
{
	char **p;
	
	if (++av_par_num >= NUM_AV_BUFFERS)
		av_par_num = 0;
	p = &av_buffers[av_par_num];
	g_free_shared(*p);
	*p = NULL;
	return p;
}

/*** ---------------------------------------------------------------------- ***/

static void free_av_buffer(const _WORD *msg)
{
	const char *p = *((const char *const *)&msg[3]);
	int i;
	
	if (p == NULL)
		return;
	for (i = 0; i < NUM_AV_BUFFERS; i++)
	{
		if (av_buffers[i] == p)
		{
			g_free_shared(av_buffers[i]);
			av_buffers[i] = NULL;
			break;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

static void av_remove_app(_WORD id)
{
	AV_APP **last = &av_apps;
	AV_APP *app;
	
	while ((app = *last) != NULL)
	{
		if (app->apid == id)
		{
			if (app == av_server_app)
			{
				av_server_app = NULL;
			}
			*last = app->next;
			g_free(app);
			return;
		}
		last = &app->next;
	}
}

/*** ---------------------------------------------------------------------- ***/

static AV_APP *av_find_app(_WORD id)
{
	AV_APP *app;
	
	for (app = av_apps; app; app = app->next)
	{
		if (app->apid == id)
			return app;
	}
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

static AV_APP *av_new_app(_WORD id)
{
	AV_APP *app;
	
	app = av_find_app(id);
	if (app == NULL)
	{
		app = g_new(AV_APP, 1);
		if (app)
		{
			app->name[0] = 0;
			app->apid = id;
			app->av_server_cfg1 = 0;
			app->av_server_cfg2 = 0;
			app->av_server_cfg3 = 0;
			app->av_proto_cfg = 0;
			app->next = av_apps;
			av_apps = app;
		}
	}
	return app;
}


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
	_WORD msg[8];
	
	if (apid >= 0)
	{
		setmsg(prot, a1, a2, a3, a4, a5);
		if (appl_write(apid, 16, msg) > 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL Protokoll_SendAv(AV_APP *app, _UWORD prot, _UWORD a1, _UWORD a2, _UWORD a3, _UWORD a4, _UWORD a5)
{
	if (!app)
		return FALSE;
	return Protokoll_Send(app->apid, prot, a1, a2, a3, a4, a5);
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

_WORD appl_locate(const char *pathlist, const char *arg, _BOOL startit)
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
		if (id < 0)
		{
			strlwr(app_name);
			id = appl_find(app_name);
		}
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
			convslash(path);
			if (_AESnumapps != 1)
			{
				char cmdline[128];
				
				if (arg)
					strncpy(cmdline + 1, arg, 126);
				else
					cmdline[1] = '\0';
				cmdline[0] = (char)strlen(cmdline + 1);
				if (__magix)
				{
					id = shel_xwrite(SHW_EXEC, 1, SHW_PARALLEL, path, cmdline);
					evnt_timer_gemlib(500);
				} else
				{
					id = shel_xwrite(SHW_EXEC, 1, 1, path, cmdline);
				}
			} else
			{
				id = SendAV_STARTPROG(path, arg, FUNK_NULL);
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
_BOOL SendAV_GETSTATUS(void)
{
	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_GETSTATUS))
		return FALSE;
	return Protokoll_SendAv(av_server_app, AV_GETSTATUS, 0, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * send a configuration string that is saved by the server
 */
_BOOL SendAV_STATUS(const char *string)
{
	char **p;
	
	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_SENDKEY))
		return FALSE;
	p = get_av_buffer();
	*p = g_strdup_shared(string);
	if (*p != NULL)
	{
		return Protokoll_SendAv(av_server_app, AV_STATUS, PROT_NAMEPTR(*p), 0, 0, 0);
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * send a key event that was not handled
 */
_BOOL SendAV_SENDKEY(short kbd_state, short code)
{
	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_SENDKEY))
		return FALSE;
	return Protokoll_SendAv(av_server_app, AV_SENDKEY, kbd_state, code, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server about default font to use for displaying text
 */
_BOOL SendAV_ASKFILEFONT(void)
{
	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_ASKFILEFONT))
		return FALSE;
	return Protokoll_SendAv(av_server_app, AV_ASKFILEFONT, 0, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server about font to use for console text
 */
_BOOL SendAV_ASKCONFONT(void)
{
	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_ASKCONFONT))
		return FALSE;
	return Protokoll_SendAv(av_server_app, AV_ASKCONFONT, 0, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server about currently selected object
 */
_BOOL SendAV_ASKOBJECT(void)
{
	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_ASKOBJECT))
		return FALSE;
	return Protokoll_SendAv(av_server_app, AV_ASKOBJECT, 0, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * tell server to open console window
 */
_BOOL SendAV_OPENCONSOLE(void)
{
	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_OPENCONSOLE))
		return FALSE;
	return Protokoll_SendAv(av_server_app, AV_OPENCONSOLE, 0, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * tell server to open a directory window
 */
_BOOL SendAV_OPENWIND(const char *path, const char *wildcard)
{
	char *ptr2;
	char **p;
	
	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_OPENWIND))
		return FALSE;
	p = get_av_buffer();
	*p = g_strdup2_shared(path, wildcard, &ptr2);
	if (*p != NULL)
	{
		return Protokoll_SendAv(av_server_app, AV_OPENWIND, PROT_NAMEPTR(*p), PROT_NAMEPTR(ptr2), 0);
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * create a quoted command line suitable for use
 * by drag&drop protokoll, or AV protokoll.
 * Quote arguments only when neccessary,
 * some servers (like GEMINI) otherwise don't
 * parse the parameters correctly
 */
char *av_cmdline(const char *const argv[], int first, gboolean for_remarker)
{
	int i;
	size_t cmlen = 0;
	char *cmd, *dst;
	const char *src;
	
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
		/*
		 * ugly hack for remarker here:
		 * it does not send AV_PROTOKOLL,
		 * but must have the arguments quoted,
		 * otherwise it generates garbage.
		 * Another hack: it must not have switches quoted
		 */
		if (*src == '\0' || strchr(src, '\'') || strchr(src, ' ') ||
			(for_remarker && *src != '-'))
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
_WORD SendAV_STARTPROG(const char *path, const char *commandline, void (*progstart)(_WORD ret))
{
	char *ptr2;
	char **p;
	
	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_STARTPROG))
		return -1;
	/*
	 * if the AV-Server is the desktop,
	 * check wether it is still running,
	 * otherwise sending the command won't work
	 */
	if (av_server_app->apid == 0)
	{
		if (av_server_app->name[0] == '\0' || appl_find(av_server_app->name) < 0)
			return -1;
	}
	p = get_av_buffer();
	*p = g_strdup2_shared(path, commandline, &ptr2);
	if (*p != NULL || path == NULL)
	{
		va_progstart = progstart;
		Protokoll_SendAv(av_server_app, AV_STARTPROG, PROT_NAMEPTR(*p), PROT_NAMEPTR(ptr2), 0x1234);
		return 0;
	}
	return -1;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL SendVA_START(_WORD dst_app, const char *path, void (*started)(void))
{
	char **p = get_av_buffer();

	*p = g_strdup_shared(path);
	va_started = started;
	if (*p != NULL || path == NULL)
	{
		return Protokoll_Send(dst_app, VA_START, PROT_NAMEPTR(*p), PROT_NAMEPTR(NULL), 0);
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * register a new window with server (for window-cycle and  Drag&Drop)
 */
_BOOL SendAV_ACCWINDOPEN(short handle)
{
	if (handle <= 0 || av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_ACCWINDOPEN))
		return FALSE;
	return Protokoll_SendAv(av_server_app, AV_ACCWINDOPEN, handle, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * unregister a window handle
 */
_BOOL SendAV_ACCWINDCLOSED(short handle)
{
	if (handle <= 0 || av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_ACCWINDCLOSED))
		return FALSE;
	return Protokoll_SendAv(av_server_app, AV_ACCWINDCLOSED, handle, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server to copy the file
 */
_BOOL SendAV_COPY_DRAGGED(short kbd_state, const char *path)
{
	char **p;

	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_COPY_DRAGGED))
		return FALSE;
	p = get_av_buffer();
	*p = g_strdup_shared(path);
	if (*p != NULL || path == NULL)
	{
		return Protokoll_SendAv(av_server_app, AV_COPY_DRAGGED, kbd_state, PROT_NAMEPTR(*p), 0, 0);
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server to update a directory window
 */
_BOOL SendAV_PATH_UPDATE(const char *path)
{
	char **p;

	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_PATH_UPDATE))
		return FALSE;
	p = get_av_buffer();
	*p = g_strdup_shared(path);
	if (*p != NULL || path == NULL)
	{
		return Protokoll_SendAv(av_server_app, AV_PATH_UPDATE, PROT_NAMEPTR(*p), 0, 0, 0);
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * ask server about object at position x,y
 */
_BOOL SendAV_WHAT_IZIT(short x, short y)
{
	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_WHAT_IZIT))
		return FALSE;
	return Protokoll_SendAv(av_server_app, AV_WHAT_IZIT, x, y, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * tell server that an object has been dragged to a window
 */
_BOOL SendAV_DRAG_ON_WINDOW(short x, short y, short kbd_state, const char *path)
{
	char **p;

	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_DRAG_ON_WINDOW))
		return FALSE;
	p = get_av_buffer();
	*p = g_strdup_shared(path);
	if (*p != NULL || path == NULL)
	{
		return Protokoll_SendAv(av_server_app, AV_DRAG_ON_WINDOW, x, y, kbd_state, PROT_NAMEPTR(*p));
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * reply to VA_START
 */
_BOOL SendAV_STARTED(const _WORD *msg)
{
	AV_APP *app = av_find_app(msg[1]);
	if (!app || !(app->av_proto_cfg & AV_PROTOKOLL_STARTED))
		return FALSE;
	return Protokoll_SendAv(app, AV_STARTED, msg[3], msg[4], msg[5], msg[6], msg[7]);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Tell server to open a window
 */
_BOOL SendAV_XWIND(const char *path, const char *wild_card, short bits)
{
	char *ptr2;
	char **p;
	
	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_XWIND))
		return FALSE;
	p = get_av_buffer();
	*p = g_strdup2_shared(path, wild_card, &ptr2);
	if (*p != NULL || path == NULL)
	{
		return Protokoll_SendAv(av_server_app, AV_XWIND, PROT_NAMEPTR(*p), PROT_NAMEPTR(ptr2), bits);
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Tell server to start a viewer registered for a file
 */
_BOOL SendAV_VIEW(const char *path)
{
	char **p;

	if (av_server_app == NULL || !(av_server_app->av_server_cfg2 & VA_PROT_VIEW))
		return FALSE;
	p = get_av_buffer();
	*p = g_strdup_shared(path);
	if (*p != NULL || path == NULL)
	{
		return Protokoll_SendAv(av_server_app, AV_VIEW, PROT_NAMEPTR(*p), 0, 0, 0);
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Tell server to show the file-/directory information
 */
_BOOL SendAV_FILEINFO(const char *path)
{
	char **p;

	if (av_server_app == NULL || !(av_server_app->av_server_cfg1 & VA_PROT_FILEINFO))
		return FALSE;
	p = get_av_buffer();
	*p = g_strdup_shared(path);
	if (*p != NULL || path == NULL)
	{
		return Protokoll_SendAv(av_server_app, AV_FILEINFO, PROT_NAMEPTR(*p), 0, 0, 0);
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Ask server to copy the files
 */
_BOOL SendAV_COPYFILE(const char *file_list, const char *dest_path, short bits)
{
	char *ptr2;
	char **p;
	
	if (av_server_app == NULL || !(av_server_app->av_server_cfg2 & VA_PROT_COPYFILE))
		return FALSE;
	p = get_av_buffer();
	*p = g_strdup2_shared(file_list, dest_path, &ptr2);
	if (*p != NULL && ptr2 != NULL)
	{
		return Protokoll_SendAv(av_server_app, AV_COPYFILE, PROT_NAMEPTR(*p), PROT_NAMEPTR(ptr2), bits);
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Ask server to delete the files
 */
_BOOL SendAV_DELFILE(const char *file_list)
{
	char **p;

	if (av_server_app == NULL || !(av_server_app->av_server_cfg2 & VA_PROT_DELFILE))
		return FALSE;
	p = get_av_buffer();
	*p = g_strdup_shared(file_list);
	if (*p != NULL)
	{
		return Protokoll_SendAv(av_server_app, AV_DELFILE, PROT_NAMEPTR(*p), 0, 0, 0);
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Tell server where to open the next window
 */
_BOOL SendAV_SETWINDPOS(short x, short y, short w, short h)
{
	if (av_server_app == NULL || !(av_server_app->av_server_cfg2 & VA_PROT_SETWINDPOS))
		return FALSE;
	return Protokoll_SendAv(av_server_app, AV_SETWINDPOS, x, y, w, h, 0);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Tell server about mouse event that was not handled
 */
_BOOL SendAV_SENDCLICK(EVNTDATA *m, short ev_return)
{
	if (av_server_app == NULL || !(av_server_app->av_server_cfg2 & VA_PROT_SENDCLICK))
		return FALSE;
	return Protokoll_SendAv(av_server_app, AV_SENDCLICK, m->x, m->y, m->bstate, m->kstate, ev_return);
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
	AV_APP *app;
	
	app = av_new_app(msg[1]);
	if (app == av_server_app)
	{
		const char *name = *(const char *const *) &msg[6];
		app->av_server_cfg1 = msg[3];
		app->av_server_cfg2 = msg[4];
		app->av_server_cfg3 = msg[5];
		if (name)
			appl_makeappname(app->name, name);
		HYP_DBG(("AV-Server name: '%s' protocol: %04x %04x", app->name, app->av_server_cfg1, app->av_server_cfg2));
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
	
	if (av_server_app == NULL)
	{
		_WORD id = appl_locate(getenv("AVSERVER"), NULL, FALSE);
		if (id < 0)
			id = appl_locate("AVSERVER", NULL, FALSE);
		if (id < 0)
			if (!appl_xsearch(2, servername, &type, &id))
				id = -1;
		if (id < 0 && gl_apid != 0)
			id = 0;
		if (id >= 0)
		{
			AV_APP *app = av_new_app(id);
			if (app)
			{
				appl_makeappname(app->name, servername);
				av_server_app = app;
			}
		}
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
	Protokoll_SendAv(av_server_app, AV_EXIT, gl_apid, 0, 0, 0, 0);
}

/*** ---------------------------------------------------------------------- ***/

void DoVA_Message(_WORD msg[8])
{
	AV_APP *app;
	
	switch (msg[0])
	{
	case VA_PROTOSTATUS:					/* server acknowledges registration */
		DoVA_PROTOSTATUS(msg);
		break;
	case AV_STARTED:
		free_av_buffer(msg);
		if (va_started)
		{
			va_started();
			va_started = 0;
		}
		break;
	case VA_PROGSTART:
		free_av_buffer(msg);
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
		app = av_new_app(msg[1]);
		if (app)
		{
			const char *name = *(const char *const*)&msg[6];
			app->av_proto_cfg = msg[3];
			if (name)
				appl_makeappname(app->name, name);
		}
		Protokoll_SendAv(app, VA_PROTOSTATUS,
			VA_PROT_SENDKEY | VA_PROT_ACCWINDOPEN | VA_PROT_OPENWIND | VA_PROT_PATH_UPDATE | VA_PROT_DRAG_ON_WINDOW | VA_PROT_EXIT | VA_PROT_STARTED | VA_PROT_QUOTING,
			0, 0,
			PROT_NAMEPTR(av_name));
		break;
	case AV_EXIT:
		av_remove_app(msg[1]);
		break;
	case VA_START:						/* pass command line */
		DoVA_START(msg);
		break;
	case VA_DRAGACCWIND:
		DoVA_DRAGACCWIND(msg);
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
		av_name = (char *)g_alloc_shared(9);
		if (av_name == NULL)
			return;
		appl_makeappname(av_name, myname);
	}
	DoAV_PROTOKOLL(AV_PROTOKOLL_QUOTING | AV_PROTOKOLL_START | AV_PROTOKOLL_STARTED);
}

/*** ---------------------------------------------------------------------- ***/

void va_proto_exit(void)
{
	int i;
	
	for (i = 0; i < NUM_AV_BUFFERS; i++)
	{
		g_free_shared(av_buffers[i]);
		av_buffers[i] = NULL;
	}
	while (av_apps)
		av_remove_app(av_apps->apid);
	DoAV_EXIT();
}
