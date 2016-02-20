#include "hv_defs.h"
#include <mint/cookie.h>
#include <mint/arch/nf_ops.h>
#include "tos/av.h"

#define PROGRAM_NAME "vastart"
char const gl_program_name[] = PROGRAM_NAME;
char const gl_compile_date[12] = __DATE__;

_WORD pwchar;
_WORD phchar;
_WORD pwbox;
_WORD phbox;
_WORD aes_handle;
short doneFlag;

/* ------------------------------------------------------------------------- */

char *gl_program_version(void)
{
	return g_strdup(HYPVIEW_VERSION);
}

/*** ---------------------------------------------------------------------- ***/

void GetTextSize(_WORD *width, _WORD *height)
{
	_WORD dummy;
	
	graf_handle(width, height, &dummy, &dummy);
}

/*** ---------------------------------------------------------------------- ***/

int DoAesInit(void)
{
	/* gl_apid = appl_init(); moved to hypmain.h */
	if (gl_apid < 0)
		return FALSE;

	aes_handle = graf_handle(&pwchar, &phchar, &pwbox, &phbox);

	{
		_WORD dummy, level;
		
		if (appl_xgetinfo(AES_SHELL, &level, &dummy, &dummy, &dummy) && (level & 0x00FF) >= 9)
			shel_write(SHW_MSGREC, 1, 1, "", "");			/* we understand AP_TERM! */
	}

	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void DoExitSystem(void)
{
	if (_app)
		va_proto_exit();

	appl_exit();
}

/*** ---------------------------------------------------------------------- ***/

static long getBootDrive(void)
{
	char bootDrive = *((char *) 0x447) + 'A';

	return bootDrive;
}
/*** ---------------------------------------------------------------------- ***/


char GetBootDrive(void)
{
	return (char)Supexec(getBootDrive);
}

/*** ---------------------------------------------------------------------- ***/

void DoVA_START(_WORD msg[8])
{
	SendAV_STARTED(msg);
}

/*** ---------------------------------------------------------------------- ***/

void DoVA_DRAGACCWIND(_WORD msg[8])
{
	UNUSED(msg);
}

/*** ---------------------------------------------------------------------- ***/

void DoMessage(_WORD *msg)
{
	gem_print_message(msg);
	switch ((_UWORD)msg[0])
	{
	case AC_OPEN:
		va_proto_init(NULL);
		break;
	
	case AC_CLOSE:
		break;
	
	case AP_TERM:
	case WM_CLOSED:
		doneFlag = TRUE;
		break;
	
	case WM_REDRAW:
		break;

	case WM_TOPPED:
		break;

	case WM_MOVED:
		break;
		
	case WM_BOTTOMED:
		break;

	case VA_PROTOSTATUS:					/* server acknowledges registration */
	case VA_SETSTATUS:
	case VA_FILEFONT:
	case VA_CONFONT:
	case VA_OBJECT:
	case VA_CONSOLEOPEN:
	case VA_WINDOPEN:
	case VA_PROGSTART:
	case VA_COPY_COMPLETE:
	case VA_THAT_IZIT:
	case VA_DRAG_COMPLETE:
	case VA_FONTCHANGED:
	case VA_XOPEN:
	case VA_VIEWED:
	case VA_FILECHANGED:
	case VA_FILECOPIED:
	case VA_FILEDELETED:
	case VA_PATH_UPDATE:
	case AV_STARTED:
	case VA_START:						/* pass command line */
		DoVA_Message(msg);
		break;
	case AV_PROTOKOLL:
		DoVA_Message(msg);
		break;
	case VA_DRAGACCWIND:
		break;
	case AV_SENDCLICK:					/* mouse click reported (BubbleGEM) */
		break;
	case AV_SENDKEY:					/* key press reported (BubbleGEM) */
		break;

	case AV_EXIT:
		DoVA_Message(msg);
		break;
	
	}
}

/*** ---------------------------------------------------------------------- ***/

static void va_started(void)
{
	doneFlag = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

int main(int argc, char **argv)
{
	const char *applname = "ST-GUIDE";
	const char *recipient_name = "REMARKER";
	_WORD recipient;
	
	static DTA mydta;
	Fsetdta(&mydta);

	Pdomain(1); /* DOM_MINT */
	{
		int i;
		
		for (i = 0; i < argc; i++)
			nf_debugprintf("%s: %d: %s\n", gl_program_name, i, argv[i]);
	}

	gl_apid = appl_init();
	if (gl_apid < 0)
		return 1;
	acc_memsave = !_app && _AESnumapps == 1;
	_mallocChunkSize(0);

	if (DoAesInit() == FALSE)
		return 1;

	menu_register(-1, applname);
	va_proto_init(applname);
	
	if (!_app)							/* running as ACC? */
		menu_register(gl_apid, "  " PROGRAM_NAME);	/* ...register to menu */

	recipient = appl_find(recipient_name);
	if (recipient < 0)
	{
		form_alert(1, "[1][Application not found][ OK ]");
	} else
	{
		char *cmd = av_cmdline(argv, 1, TRUE);
		SendVA_START(recipient, cmd, va_started);
		g_free(cmd);
		while (!_app || !doneFlag)
		{
			_WORD msg[8];
			_WORD dummy;
			_WORD events;
			
			events = evnt_multi_gemlib(MU_MESAG | MU_TIMER,
				0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				msg,
				1000l,
				&dummy, &dummy, &dummy, &dummy, &dummy, &dummy);
			if (events & MU_MESAG)
				DoMessage(msg);
			if (events & MU_TIMER)
				doneFlag = TRUE;
		}
	}
	
	DoExitSystem();

	return 0;
}
