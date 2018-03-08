#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#undef HAVE_GLIB

#include "hv_defs.h"
#include "diallib.h"
#include "tos/av.h"
#include "tos/mem.h"

#ifndef ENSMEM
#  define ENSMEM (-39)
#endif
#ifndef EFILNF
#  define EFILNF (-33)
#endif
int acc_memsave;


BASEPAGE *SetActPD(BASEPAGE *newpd)
{
	/* dummy; not needed here */
	return newpd;
}


static _WORD Appl_init(void)
{
	_WORD level, dummy;
	
	if ((gl_apid = appl_init()) < 0)
		return ENSMEM;
	acc_memsave = !_app && _AESnumapps == 1;
	
	if (appl_xgetinfo(AES_SHELL, &level, &dummy, &dummy, &dummy) && (level & 0x00FF) >= 9)
		shel_write(SHW_MSGREC, 1, 1, "", "");			/* wir koennen AP_TERM! */
	
	return gl_apid;
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
	UNUSED(arg);
	UNUSED(startit);
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
	return id;
}


static char *make_cmdline(int argc, const char *const argv[], int first)
{
	int i;
	size_t cmlen = 0;
	char *cmd, *dst;
	const char *src;
	
	if (argv == NULL || argv[0] == NULL || argv[first] == NULL)
		return NULL;
	for (i = first; i < argc; i++)
		cmlen += strlen(argv[i]) * 2 + 3;
	cmd = (char *)g_alloc_shared(cmlen);
	if (cmd == NULL)
		return NULL;
	
	dst = cmd;
	for (i = first; i < argc; i++)
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
		if (*src == '\0' || strchr(src, '\'') || strchr(src, ' '))
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


int main(int argc, const char **argv)
{
	_WORD event;
	_WORD viewer = -1;
	_WORD msg[8];
	int ret;
	_WORD buf[8];
	char *cmd;
	_WORD d;
	int open_windows;
	_WORD events;
	char **pcmd;
	
	ret = 0;
	
	if (!_app)
	{
		if ((gl_apid = Appl_init()) < 0)
			return ENSMEM;
	
		for (;;)
		{
			evnt_mesag(buf);
			switch (buf[0])
			{
			case AC_OPEN:
				form_alert(1, "[1][ This program should not be installed as accessory][Oops]");
				break;
			case CH_EXIT:
			case AP_TERM:
				goto done;
			}
		}
	}
	
	if (argc <= 0)
		return 1;

	if ((gl_apid = Appl_init()) < 0)
		return ENSMEM;

	viewer = help_viewer_id();
	if (viewer < 0)
	{
		ret = EFILNF;				/* no known help viewer running */
		goto done;
	}
	
	/*
	 * build cmdline;
	 */
	if (argc > 1)
	{
		cmd = make_cmdline(argc, argv, 1);
		if (cmd == NULL)
		{
			ret = ENSMEM;
			goto done;
		}
	} else
	{
		cmd = NULL;
	}
	
	/*
	 * Message for ACC to work on
	 */
	
	msg[0] = VA_START;
	msg[1] = gl_apid;
	msg[2] = 0;
	pcmd = (char **) &msg[3];
	*pcmd = cmd;
	msg[5] = 0;
	msg[6] = 0;
	msg[7] = 0;

	/*
	 * ACC activate
	 */
	appl_write(viewer, 16, msg);

	/*
	 * wait, until ACC opens, or timeout
	 */
	open_windows = 0;
	events = MU_TIMER | MU_MESAG;
	for (;;)
	{
		event = evnt_multi(events,
						   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, buf, 2000, &d, &d, &d, &d, &d, &d);
		if (event & MU_TIMER)
		{
			ret = EFILNF;
			goto done;
		}
		if (event & MU_MESAG)
		{
			pcmd = (char **) &buf[3];
			if (buf[0] == AV_STARTED && *pcmd == cmd)
			{
				g_free_shared(cmd);
				cmd = NULL;
				
				/*
				 * ACC started.
				 * On Single-TOS, wait until its window is closed,
				 * otherwise TOS will kill the ACC when we exit.
				 * On Multi-Tos, its safe to exit.
				 */
				if (_AESnumapps > 1)
					break;
			}
			if (buf[0] == AV_ACCWINDCLOSED)
			{
				open_windows--;
				if (open_windows <= 0)
					break;
			}
			if (buf[0] == AV_ACCWINDOPEN)
			{
				open_windows++;
				events &= ~MU_TIMER;
			}
			if (buf[0] == AV_STARTPROG)
			{
				char *p;
				
				/*
				 * Program may be started:
				 *
				 * Command line to call programm
				 * initialise
				 */
				cmd = (char *)Malloc(128);
				pcmd = (char **) &buf[5];
				p = *pcmd;
				if (p)
				{
					strncpy(cmd + 1, p, 126);
					cmd[127] = '\0';
					*cmd = strlen(p);
				} else
				{
					cmd[0] = cmd[1] = 0;
				}
				/*
				 * Here we must determine whether,
				 * we're dealing with a TOS 
				 * or GEM Application...
				 */
				pcmd = (char **) &buf[3];
				shel_write(SHW_EXEC, 1, 1, *pcmd, cmd);
				open_windows = 0;
			}
		}
	}
	
done:
	appl_exit();
	return ret;
}

/*** ---------------------------------------------------------------------- ***/

const char *hyp_basename(const char *path)
{
	const char *p;
	const char *base = NULL;
	
	if (path == NULL)
		return path;
	p = path;
	while (*p != '\0')
	{
		if (G_IS_DIR_SEPARATOR(*p))
			base = p + 1;
		++p;
	}
	if (base != NULL)
		return base;
	
	if (isalpha(path[0]) && path[1] == ':')
	{
    	/* can only be X:name, without slash */
    	path += 2;
	}
	
	return path;
}

/*** ---------------------------------------------------------------------- ***/

#ifndef g_strdup
char *g_strdup(const char *str)
{
	char *dst;
	
	if (str == NULL)
		return NULL;
	dst = g_new(char, strlen(str) + 1);
	if (dst == NULL)
		return NULL;
	return strcpy(dst, str);
}
#endif

/*** ---------------------------------------------------------------------- ***/

#ifndef g_strndup
char *g_strndup(const char *str, size_t len)
{
	char *dst;
	
	if (str == NULL)
		return NULL;
	if (len == STR0TERM)
		len = strlen(str);
	dst = g_new(char, len + 1);
	if (dst == NULL)
		return NULL;
	memcpy(dst, str, sizeof(char) * len);
	dst[len] = '\0';
	return dst;
}
#endif

#include "../../dl_mem.c"
#include "../../dl_help.c"
