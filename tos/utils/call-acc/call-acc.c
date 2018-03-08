#include "hypdefs.h"
#include "diallib.h"
#include "tos/av.h"
#include "tos/mem.h"

#ifndef ENSMEM
#  define ENSMEM (-39)
#endif
#ifndef EFILNF
#  define EFILNF (-33)
#endif


static _WORD Appl_init(void)
{
	_WORD level, dummy;
	
	if ((gl_apid = appl_init()) < 0)
		return ENSMEM;
	
	if (appl_xgetinfo(AES_SHELL, &level, &dummy, &dummy, &dummy) && (level & 0x00FF) >= 9)
		shel_write(SHW_MSGREC, 1, 1, "", "");			/* wir koennen AP_TERM! */
	
	return gl_apid;
}


int main(int argc, const char **argv)
{
	_WORD event;
	_WORD viewer = -1;
	_WORD msg[8];
	int ret;
	_WORD buf[8];
	char *cmd;
	int i;
	_WORD d;
	_UWORD ud;
	size_t len;
	int open_windows;
	_WORD events;
	
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
		len = 1;
		for (i = 1; i < argc; i++)
			len += strlen(argv[i]) + 1;
		
		cmd = g_alloc_shared(len);
		if (cmd == NULL)
		{
			ret = ENSMEM;
			goto done;
		}
		*cmd = '\0';
		for (i = 1; i < argc; i++)
		{
			if (i > 1)
				strcat(cmd, " ");
			strcat(cmd, argv[i]);
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
	*((char **) &msg[3]) = cmd;
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
						   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, buf, 2000, 0, &d, &d, &ud, &ud, &d, &d);
		if (event & MU_TIMER)
		{
			ret = EFILNF;
			goto done;
		}
		if (event & MU_MESAG)
		{
			if (buf[0] == AV_STARTED && (*((char **) &buf[3])) == cmd)
			{
				g_free_shared(cmd);
				cmd = NULL;
				
				/*
				 * ACC started.
				 * On Single-TOS, wait until its window is closed,
				 * otherwise TOS will kill the ACC when we exit.
				 * On Multi-Tos, its safe to exit.
				 */
				if (gl_ap_count > 1)
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
				cmd = Malloc(128);
				p = *(char **) &buf[5];
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
				shel_write(SHW_EXEC, 1, 1, *(char **) &buf[3], cmd);
				open_windows = 0;
			}
		}
	}
	
done:
	appl_exit();
	return ret;
}
