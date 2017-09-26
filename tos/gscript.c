/*****************************************************************************
 * TOS/GSCRIPT.C
 *****************************************************************************/

#include "hv_defs.h"
#include "hypdebug.h"
#include "tos/gscript.h"
#include "tos/av.h"
#include "tos/mem.h"

#define TRACE_PROT 0

#undef TRACE
#if TRACE_PROT
#  define TRACE(x) hyp_debug x
#else
#  define TRACE(x) 
#endif

/*
 * protocol-version we support (currently 1.2)
 */
#define GEMSCRIPT_VERSION 0x120


typedef struct {
	_LONG	len;			/* Laenge der Struktur in Bytes */
	_WORD	version;		/* Versionsnummer des Protokolles beim Sender */
	_WORD	msgs;			/* Bitmap der unterstuetzten Nachrichten (GSM_xxx) */
	_LONG	ext;			/* benutzte Endung, etwa '.SIC' */
} GS_INFO;

/* Messages */	
#define GS_REQUEST		0x1350
#define GS_REPLY		0x1351
#define GS_COMMAND		0x1352
#define GS_ACK			0x1353
#define GS_QUIT			0x1354
#define GS_OPENMACRO	0x1355
#define GS_MACRO		0x1356
#define GS_WRITE		0x1357
#define GS_CLOSEMACRO	0x1358


typedef struct _gs_proc GS_PROC;

struct _gs_proc {
	_WORD gs_apid;
	_WORD gs_id;
	_WORD gs_macro_id;
	_BOOL gs_in_macro;
	GS_INFO *gs_interpreter_info;
	char *gs_memptr;
	char *gs_erg;
	_BOOL gs_in_command;
	GS_RESULT_FUNC gs_result_func;
	GS_PROC *gs_next;
};

typedef struct _command COMMAND;
struct _command {
	SCRIPT_COMMAND func;
	COMMAND *next;
	char name[1];
};

static GS_PROC *procs;
static COMMAND *commands;
static GS_INFO *my_info;


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void gs_freemem(GS_PROC *proc)
{
	if (proc->gs_memptr != NULL)
	{
		g_free_shared(proc->gs_memptr);
		proc->gs_memptr = NULL;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void gs_free_erg(GS_PROC *proc)
{
	if (proc->gs_erg != NULL)
	{
		g_free_shared(proc->gs_erg);
		proc->gs_erg = NULL;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void gs_free(GS_PROC *proc)
{
	if (proc->gs_interpreter_info != NULL)
	{
		g_free(proc->gs_interpreter_info);
		proc->gs_interpreter_info = NULL;
	}
	proc->gs_apid = -1;
	proc->gs_id = -1;
	proc->gs_in_macro = FALSE;
	proc->gs_in_command = FALSE;
	proc->gs_result_func = FUNK_NULL;
	gs_freemem(proc);
	gs_free_erg(proc);
}

/*** ---------------------------------------------------------------------- ***/

static void gs_remove(_WORD apid, _BOOL all, _WORD gs_id)
{
	GS_PROC *proc;
	
	proc = procs;
	while (proc != NULL)
	{
		if (proc->gs_apid == apid && (all || gs_id == proc->gs_id))
			gs_free(proc);
		proc = proc->gs_next;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void gs_delete(GS_PROC **proc)
{
	GS_PROC *a;

	a = *proc;
	gs_free(a);
	*proc = a->gs_next;
	g_free(a);
}

/*** ---------------------------------------------------------------------- ***/

static void gs_delete_all(void)
{
	while (procs != NULL)
		gs_delete(&procs);
}

/*** ---------------------------------------------------------------------- ***/

static GS_PROC *gs_find_id(_WORD apid, _WORD gs_id, _BOOL any)
{
	GS_PROC *proc;
	
	proc = procs;
	while (proc != NULL)
	{
		if (proc->gs_apid == apid && (any || gs_id == proc->gs_id))
			return proc;
		proc = proc->gs_next;
	}
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

static GS_PROC *gs_new(_WORD apid, _WORD gs_id)
{
	GS_PROC *proc;
	
	for (proc = procs; proc != NULL; proc = proc->gs_next)
	{
		if (proc->gs_apid == apid && proc->gs_id == gs_id)
			break;
	}
	if (proc == NULL)
		for (proc = procs; proc != NULL; proc = proc->gs_next)
		{
			if (proc->gs_apid == -1)
				break;
		}
	if (proc == NULL)
	{
		proc = (GS_PROC *)g_try_malloc(sizeof(GS_PROC));
		if (proc != NULL)
		{
			proc->gs_next = procs;
			procs = proc;
		}
	}
	if (proc != NULL)
	{
		proc->gs_apid = apid;
		proc->gs_id = gs_id;
		proc->gs_interpreter_info = NULL;
		proc->gs_macro_id = 0;
		proc->gs_in_macro = FALSE;
		proc->gs_in_command = FALSE;
		proc->gs_result_func = FUNK_NULL;
		proc->gs_memptr = NULL;
		proc->gs_erg = NULL;
	}
	return proc;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL arg_split(const char *cmd, _WORD *pargc, char ***pargv)
{
	char **argv;
	size_t size;
	const char *p;
	char *d;
	_WORD argc;
	
	size = 0;
	p = cmd;
	argc = 0;
	while (*p != '\0')
	{
		if (*p >= '\002' && *p <= '\006')
		{
			while (*p++ != '\0')
				;
		} else
		{
			while (*p++ != '\0')
				size++;
			size++;
			argc++;
		}
	}
	if (argc == 0)
		return FALSE;
	size += (argc + 1) * sizeof(char *);
	argv = (char **)g_try_malloc(size);
	if (argv == NULL)
		return FALSE;
	*pargv = argv;
	*pargc = argc;
	d = (char *)(argv + argc + 1);
	p = cmd;
	while (*p != '\0')
	{
		if (*p >= '\002' && *p <= '\006')
		{
			while (*p++ != '\0')
				;
		} else
		{
			*argv = d;
			while ((*d++ = *p++) != '\0')
				;
			if (**argv == '\001')				/* Leerer Parameter */
				**argv = '\0';
			argv++;
		}
	}
	*argv = NULL;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static char *gs_erg(const char *str)
{
	size_t len;
	char *erg;
	
	len = strlen(str) + 2;
	erg = (char *)g_alloc_shared(len);
	if (erg != NULL)
	{
		strcpy(erg, str);
		erg[len - 1] = '\0';
	}
	return erg;
}

/*** ---------------------------------------------------------------------- ***/

static _WORD gs_handle_command(_WORD argc, const char *const *argv, char **erg)
{
	COMMAND *cmd;
	const char *arg;
	
	for (cmd = commands; cmd != NULL; cmd = cmd->next)
		if (strcasecmp(argv[0], cmd->name) == 0)
		{
			return cmd->func(argc, argv, erg) ? GSACK_OK : GSACK_ERROR;
		}
	if (strcasecmp(argv[0], "Open") == 0)
	{
		if (argc == 1)
		{
			SendVA_START(gl_apid, NULL, FUNK_NULL);
		} else
		{
			while (--argc)
			{
				arg = *++argv;
				SendVA_START(gl_apid, arg, FUNK_NULL);
			}
		}
		*erg = g_strdup("1");
		return GSACK_OK;
	}
	if (strcasecmp(argv[0], "Quit") == 0)
	{
		Protokoll_Send(gl_apid, AP_TERM, 0, 0, AP_TERM, 0, 0);
		*erg = g_strdup("1");
		return GSACK_OK;
	}
	if (strcasecmp(argv[0], "AppGetLongName") == 0)
	{
		*erg = g_strdup(gl_program_name);
		return GSACK_OK;
	}
	return GSACK_UNKNOWN;
}

/*** ---------------------------------------------------------------------- ***/

static _WORD gs_getallcommands(GS_PROC *proc)
{
	COMMAND *cmd;
	size_t len;
	char *p;
	_BOOL has_appgetlongname = FALSE;
	_BOOL has_quit = FALSE;
	_BOOL has_open = FALSE;
	
	len = 0;
	for (cmd = commands; cmd != NULL; cmd = cmd->next)
	{
		len += strlen(cmd->name) + 1;
		if (strcasecmp(cmd->name, "AppGetLongName") == 0)
			has_appgetlongname = TRUE;
		if (strcasecmp(cmd->name, "Quit") == 0)
			has_quit = TRUE;
		if (strcasecmp(cmd->name, "Open") == 0)
			has_open = TRUE;
	}
	if (!has_appgetlongname)
		len += 14 + 1;
	if (!has_quit)
		len += 4 + 1;
	if (!has_open)
		len += 4 + 1;
	len += 14 + 1;
	proc->gs_erg = (char *)g_alloc_shared(len + 1);
	if (proc->gs_erg == NULL)
		return GSACK_ERROR;
	p = proc->gs_erg;
	for (cmd = commands; cmd != NULL; cmd = cmd->next)
	{
		strcpy(p, cmd->name);
		p += strlen(cmd->name) + 1;
	}
	if (!has_appgetlongname)
	{
		strcpy(p, "AppGetLongName");
		p += 14 + 1;
	}
	if (!has_quit)
	{
		strcpy(p, "Quit");
		p += 4 + 1;
	}
	if (!has_open)
	{
		strcpy(p, "Open");
		p += 4 + 1;
	}
	strcpy(p, "GetAllCommands");
	p += 14 + 1;
	*p = '\0';
	return GSACK_OK;
}

/*** ---------------------------------------------------------------------- ***/

static void gs_stop_macro(GS_PROC *proc)
{
	if (proc->gs_in_macro)
	{
		Protokoll_Send(proc->gs_apid, GS_CLOSEMACRO, 0, 0, proc->gs_macro_id, 0, 0);
		proc->gs_in_macro = FALSE;
	}
	gs_freemem(proc);
}

/*** ---------------------------------------------------------------------- ***/

static void gs_stop_macro_id(_WORD apid, _WORD macro_id, _BOOL all)
{
	GS_PROC *proc;
	
	for (proc = procs; proc != NULL; proc = proc->gs_next)
		if (proc->gs_apid == apid && (all || proc->gs_macro_id == macro_id))
		{
			proc->gs_in_macro = FALSE;
			gs_freemem(proc);
		}
}

/*** ---------------------------------------------------------------------- ***/

_BOOL gemscript_handle_message(const _WORD *message)
{
	GS_PROC *proc;
	GS_INFO *gs_info;
	char *cmd;
	char *erg;
	_BOOL ok;
	
	switch (message[0])
	{
	case GS_REQUEST:
		TRACE(("GS_REQUEST(%d, %d)", message[1], message[7]));
		ok = FALSE;
		gs_info = NULL;
		if ((proc = gs_new(message[1], message[7])) != NULL)
		{
			gs_info = *((GS_INFO *const *)(&message[3]));
			if (gs_info != NULL)
			{
				if (gs_info->len >= (_WORD)sizeof(GS_INFO))
				{
					if (proc->gs_interpreter_info != NULL)
						g_free(proc->gs_interpreter_info);
					if ((proc->gs_interpreter_info = (GS_INFO *)g_try_malloc(gs_info->len)) != NULL)
					{
						memcpy(proc->gs_interpreter_info, gs_info, gs_info->len);
						ok = TRUE;
					}
				}
			}
		}
		Protokoll_Send(message[1], GS_REPLY, PROT_NAMEPTR(my_info), 0, ok ? 0 : 1, message[7]);
		break;
	
	case GS_REPLY:
		TRACE(("GS_REPLY(%d, %d)", message[1], message[7]));
		gs_info = *((GS_INFO *const *)(&message[3]));
		if (gs_info != NULL)
		{
			proc = gs_new(message[1], message[7]);
			if (proc != NULL)
			{
				if (proc->gs_interpreter_info != NULL)
					g_free(proc->gs_interpreter_info);
				if ((proc->gs_interpreter_info = (GS_INFO *)g_try_malloc(gs_info->len)) != NULL)
					memcpy(proc->gs_interpreter_info, gs_info, gs_info->len);
			}
		}
		break;
	
	case GS_QUIT:
		TRACE(("GS_QUIT(%d, %d)", message[1], message[7]));
		gs_remove(message[1], FALSE, message[7]);
		break;
	
	case AV_EXIT:
		TRACE(("AV_EXIT(%d)", message[1]));
		gs_remove(message[1], TRUE, 0);
		break;
	
	case GS_COMMAND:
		cmd = *((char *const *)(&message[3]));
		TRACE(("GS_COMMAND(%d, %d, $%08lx=%s)", message[1], message[7], cmd, printnull(cmd)));
		proc = gs_find_id(message[1], message[7], FALSE);
		if (proc == NULL)
			proc = gs_find_id(message[1], message[7], TRUE);
		if (proc != NULL)
		{
			gs_free_erg(proc);
			if (cmd != NULL)
			{
				_WORD argc;
				char **argv;
				
				if (arg_split(cmd, &argc, &argv))
				{
					erg = NULL;
					ok = gs_handle_command(argc, (const char *const *)argv, &erg);
					if (erg != NULL)
					{
						proc->gs_erg = gs_erg(erg);
						g_free(erg);
					} else if (strcasecmp(argv[0], "GetAllCommands") == 0)
					{
						ok = gs_getallcommands(proc);
					}
					TRACE(("GS_ACK to %d: $%08lx, $%08lx", message[1], cmd, proc->gs_erg));
					Protokoll_Send(message[1], GS_ACK, PROT_NAMEPTR(cmd), PROT_NAMEPTR(proc->gs_erg), ok);
					g_free(argv);
				} else
				{
					proc->gs_erg = gs_erg("not enough memory");
					ok = GSACK_ERROR;
					TRACE(("GS_ACK to %d: $%08lx, $%08lx", message[1], cmd, proc->gs_erg));
					Protokoll_Send(message[1], GS_ACK, PROT_NAMEPTR(cmd), PROT_NAMEPTR(proc->gs_erg), ok);
				}
			} else
			{
				proc->gs_erg = gs_erg("no command");
				ok = GSACK_ERROR;
				TRACE(("GS_ACK to %d: $%08lx, $%08lx", message[1], cmd, proc->gs_erg));
				Protokoll_Send(message[1], GS_ACK, PROT_NAMEPTR(cmd), PROT_NAMEPTR(proc->gs_erg), ok);
			}
		}
		break;
	
	case GS_ACK:
		cmd = *((char *const *)(&message[3]));
		erg = *((char *const *)(&message[5]));
		TRACE(("GS_ACK from %d: $%08lx $%08lx %d", message[1], cmd, erg, message[7]));
		proc = NULL;
		if (erg != NULL)
		{
			proc = procs;
			while (proc != NULL)
			{
				if (erg == proc->gs_erg)
				{
					gs_free_erg(proc);
					break;
				}
				proc = proc->gs_next;
			}
		}
		if (proc == NULL)
			proc = gs_find_id(message[1], 0, TRUE);
		if (proc != NULL)
		{
			if (proc->gs_in_macro)
			{
				gs_free_erg(proc);
				gs_freemem(proc);
				if (message[7] == GSACK_ERROR)
					gs_stop_macro(proc);
			} else if (proc->gs_in_command)
			{
				gs_freemem(proc);
				/* ACK von GS_COMMAND, das wir selber verschickt haben */
				if (proc->gs_result_func != FUNK_NULL)
					proc->gs_result_func(erg, message[7]);
				if (erg != NULL)
					Protokoll_Send(message[1], GS_ACK, PROT_NAMEPTR(NULL), PROT_NAMEPTR(erg), GSACK_OK);
				proc->gs_result_func = FUNK_NULL;
				proc->gs_in_command = FALSE;
			} else
			{
				gs_free_erg(proc);
			}
		}
		break;
	
	case GS_CLOSEMACRO:
		TRACE(("GS_CLOSEMACRO(%d, %d)", message[1], message[5]));
		gs_stop_macro_id(message[1], message[5], FALSE);
		break;
	
	case GS_MACRO:
		erg = *((char *const *)(&message[3]));
		TRACE(("GS_MACRO(%d, %s)", message[1], erg ? erg : "(null)"));
		proc = gs_find_id(message[1], 0, TRUE);
		if (proc != NULL)
		{
			if (erg != NULL)
			{
				/* dann kam's von unserem GS_OPENMACRO */
				g_free_shared(erg);
				proc->gs_memptr = NULL;
			}
			if (message[6] == 0)
			{
				proc->gs_macro_id = message[5];
				proc->gs_in_macro = TRUE;
			}
			Protokoll_Send(message[1], GS_ACK, PROT_NAMEPTR(erg), PROT_NAMEPTR(NULL), GSACK_OK);
		} else
		{
			Protokoll_Send(message[1], GS_CLOSEMACRO, PROT_NAMEPTR(erg), message[5], 0, 0);
		}
		break;
	
	default:
		return FALSE;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void gemscript_stop(GS_TARGET apid)
{
	GS_PROC *proc;
	
	while ((proc = gs_find_id(apid, 0, TRUE)) != NULL)
	{
		gs_stop_macro(proc);
		Protokoll_Send(proc->gs_apid, GS_QUIT, 0, 0, 0, 0, proc->gs_id);
		gs_free(proc);
	}
}

/*** ---------------------------------------------------------------------- ***/

void gemscript_exit(void)
{
	GS_PROC *proc;
	COMMAND *cmd;
	
	for (proc = procs; proc != NULL; proc = proc->gs_next)
	{
		gs_stop_macro(proc);
		if (proc->gs_apid >= 0)
			Protokoll_Send(proc->gs_apid, GS_QUIT, 0, 0, 0, 0, proc->gs_id);
	}
	gs_delete_all();
	while (commands != NULL)
	{
		cmd = commands->next;
		g_free(commands);
		commands = cmd;
	}
	if (my_info != NULL)
	{
		g_free_shared(my_info);
		my_info = NULL;
	}
}

/*** ---------------------------------------------------------------------- ***/

void gemscript_start(GS_TARGET apid)
{
	_WORD message[8];
	GS_INFO **pinfo;
	
	message[0] = GS_REQUEST;
	message[1] = gl_apid;
	message[2] = 0;
	pinfo = (GS_INFO **)&message[3];
	*pinfo = my_info;
	message[5] = 0;
	message[6] = 0;
	message[7] = 0x4f52; /* 'OR' */
	appl_write(apid, 16, message);
}

/*** ---------------------------------------------------------------------- ***/

_BOOL gemscript_info(GS_TARGET apid, _UWORD *version, _UWORD *msgs)
{
	GS_PROC *proc;
	GS_INFO *info;
	
	if ((proc = gs_find_id(apid, 0, TRUE)) != NULL && (info = proc->gs_interpreter_info) != NULL)
	{
		*version = info->version;
		*msgs = info->msgs;
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static char *gs_copy_command(_WORD argc, const char *const *argv)
{
	size_t size, len;
	_WORD i;
	char *command;
	char *dst;
	const char *src;
	
	if (argc <= 0)
		return NULL;
	size = 0;
	for (i = 0; i < argc; i++)
	{
		len = strlen(argv[i]);
		if (len == 0)
			size += 2;
		else
			size += len + 1;
	}
	size += 1;
	command = (char *)g_alloc_shared(size);
	if (command != NULL)
	{
		dst = command;
		for (i = 0; i < argc; i++)
		{
			src = argv[i];
			if (*src == '\0')
				*dst++ ='\001'; /* Leerer Parameter */
			while ((*dst++ = *src++) != '\0')
				;
		}
		*dst = '\0';
	}
	return command;
}

/*** ---------------------------------------------------------------------- ***/

void gemscript_send_command(GS_TARGET apid, _WORD argc, const char *const *argv, GS_RESULT_FUNC func)
{
	GS_PROC *proc;
	char *command;
	
	if ((command = gs_copy_command(argc, argv)) != NULL)
	{
		proc = gs_find_id(apid, 0, TRUE);
		if (proc != NULL)
		{
			gs_freemem(proc);
			proc->gs_memptr = command;
			proc->gs_result_func = func;
			proc->gs_in_command = TRUE;
			Protokoll_Send(proc->gs_apid, GS_COMMAND, PROT_NAMEPTR(command), 0, 0, proc->gs_id);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

_BOOL gemscript_init(void)
{
	if ((my_info = (GS_INFO *)g_alloc_shared(sizeof(GS_INFO))) != NULL)
	{
		memset(my_info, 0, sizeof(*my_info));
		my_info->len = sizeof(GS_INFO);
		my_info->version = 0x120;
		my_info->msgs = GSM_COMMAND | GSM_WRITE;
#if 0
		{
			_WORD message[8];
			
			message[0] = GS_REQUEST;
			message[1] = gl_apid;
			message[2] = 0;
			*((GS_INFO **)&message[3]) = my_info;
			message[5] = 0;
			message[6] = 0;
			message[7] = 0x4856; /* 'HV' */
			Protokoll_Broadcast(message, FALSE);
		}
#endif
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL gemscript_register_command(const char *command, SCRIPT_COMMAND func)
{
	COMMAND *cmd;
	
	for (cmd = commands; cmd != NULL; cmd = cmd->next)
	{
		if (strcasecmp(cmd->name, command) == 0)
		{
			cmd->func = func;
			return TRUE;
		}
	}
	cmd = (COMMAND *)g_malloc(sizeof(*cmd) + strlen(command));
	if (cmd == NULL)
		return FALSE;
	strcpy(cmd->name, command);
	cmd->func = func;
	cmd->next = commands;
	commands = cmd;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL Protokoll_Waitfor(_WORD msg, _WORD timeout, EVNT *event)
{
	for (;;)
	{
		memset(event, 0, sizeof(*event));
		event->mwhich = evnt_multi_gemlib(MU_MESAG|MU_TIMER,
			0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			event->msg,
			timeout,
			&event->mx, &event->my, &event->mbutton, &event->kstate, &event->key, &event->mclicks);
		if (event->mwhich & MU_MESAG)
		{
			if (event->msg[0] == msg)
				return TRUE;
			DoEventDispatch(event);
		}
		if (event->mwhich & MU_TIMER)
			return FALSE;
	}
}

/*** ---------------------------------------------------------------------- ***/

void gemscript_record(_WORD argc, const char *const *argv)
{
	GS_PROC *proc;
	size_t size;
	const char *arg;
	_WORD i;
	char *mem, *p;
	EVNT event;
	
	if (argc <= 0)
		return;
	size = 0;
	for (i = 0; i < argc; i++)
	{
		arg = argv[i];
		if (arg == NULL || *arg == '\0')
		{
			size += 2;
		} else
		{
			while (*arg++ != '\0')
				size++;
			size++;
		}
	}
	size++;
	
	for (proc = procs; proc != NULL; proc = proc->gs_next)
	{
		if (proc->gs_in_macro && proc->gs_apid != gl_apid)
		{
			_WORD gs_apid;
			
			mem = (char *)g_alloc_shared(size);
			if (mem == NULL)
				break;
			p = mem;
			for (i = 0; i < argc; i++)
			{
				arg = argv[i];
				if (arg == NULL || *arg == '\0')
				{
					*p++ = '\001';
					*p++ = '\0';
				} else
				{
					while ((*p++ = *arg++) != '\0')
						;
				}
			}
			*p = '\0';
			proc->gs_memptr = mem;
			gs_apid = proc->gs_apid;
			TRACE(("GS_WRITE(%s)", argv[0]));
			Protokoll_Send(gs_apid, GS_WRITE, PROT_NAMEPTR(mem), proc->gs_macro_id, 0, 0);
			while (Protokoll_Waitfor(GS_ACK, 1000, &event))
			{
				if (event.msg[1] == gs_apid)
				{
					if (event.msg[7] == GSACK_ERROR)
						gs_stop_macro(proc);
					break;
				} else
				{
					gemscript_handle_message(event.msg);
					/* FIXME: proc might just have gone */
				}
			}
			gs_freemem(proc);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void gemscript_stop_recording(void)
{
	GS_PROC *proc;
	
	for (proc = procs; proc != NULL; proc = proc->gs_next)
		gs_stop_macro(proc);
}

/*** ---------------------------------------------------------------------- ***/

void gemscript_start_recording(void)
{
	/* TODO: open fileselector, send GS_OPENMACRO */
}
