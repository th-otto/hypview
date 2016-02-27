#include "hv_defs.h"
#include <mint/cookie.h>
#include <mint/arch/nf_ops.h>
#include "tos/av.h"
#include "hv_vers.h"

#define PROGRAM_NAME "Remarker"
char const gl_program_name[] = PROGRAM_NAME;
char const gl_compile_date[12] = __DATE__;

_WORD pwchar;
_WORD phchar;
_WORD pwbox;
_WORD phbox;
_WORD vdi_handle;
_WORD workin[16];
_WORD workout[57];
_WORD ext_workout[57];
_WORD aes_handle;
short doneFlag;

_WORD thewin;
GRECT winpos = { 700, 100, 200, 200 };
static OBJECT theobj[] = {
	{ -1, -1, -1, G_BOX, OF_LASTOB, 0, 0, 0, 0, 0, 0 }
};

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
	_WORD attrib[10];
	
	/* gl_apid = appl_init(); moved to hypmain.h */
	if (gl_apid < 0)
		return FALSE;

	aes_handle = graf_handle(&pwchar, &phchar, &pwbox, &phbox);
	vqt_attributes(aes_handle, attrib);
	
	vq_extnd(aes_handle, 0, workout);
	vq_extnd(aes_handle, 1, ext_workout);

	{
		_WORD dummy, level;
		
		if (appl_xgetinfo(AES_SHELL, &level, &dummy, &dummy, &dummy) && (level & 0x00FF) >= 9)
			shel_write(SHW_MSGREC, 1, 1, "", "");			/* we understand AP_TERM! */
	}

	return TRUE;
}


int DoInitSystem(void)
{
	short i;
	_WORD dummy;

	for (i = 0; i < 10; i++)
		workin[i] = 1;
	workin[10] = 2;
	vdi_handle = aes_handle;
	v_opnvwk(workin, &vdi_handle, workout);
	if (!vdi_handle)
	{
		form_alert(1, "[1][Can't open a VDI workstation.][Cancel]");
		return FALSE;
	}
	vq_extnd(vdi_handle, 1, ext_workout);

	/* set default alignment */
	vst_alignment(vdi_handle, TA_LEFT, TA_TOP, &dummy, &dummy);

	/* set default fill attributes */
	vsf_color(vdi_handle, G_WHITE);
	vsf_interior(vdi_handle, FIS_SOLID);
	vsf_perimeter(vdi_handle, 0);

	/* set default line attribtes */
	vsl_udsty(vdi_handle, 0xAAAA);		/* dotted line */
	vsl_width(vdi_handle, 1);
	vsl_ends(vdi_handle, 0, 0);
	vsl_type(vdi_handle, SOLID);
	vsl_color(vdi_handle, G_BLACK);

	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void DoExitSystem(void)
{
	if (_app)
		va_proto_exit();
	if (vdi_handle)
	{
		v_clsvwk(vdi_handle);
		vdi_handle = 0;
	}

	appl_exit();
}

/*** ---------------------------------------------------------------------- ***/

void singletos_fail_loop(void)
{
	if (_AESnumapps == 1 && !_app)
	{
		_WORD msg[8];
		
		for (;;)
			evnt_mesag(msg);
	}
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

_BOOL wind_set_top(_WORD whandle)
{
	_WORD top;
	
	wind_get_int(DESK, WF_TOP, &top);
	wind_set_int(whandle, WF_TOP, 0);
	return whandle != top;
}

/*** ---------------------------------------------------------------------- ***/

void delete_win(void)
{
	if (thewin > 0)
	{
		wind_close(thewin);
		wind_delete(thewin);
		SendAV_ACCWINDCLOSED(thewin);
		thewin = -1;
	}
}

/*** ---------------------------------------------------------------------- ***/

void create_win(void)
{
	_WORD stguide;
	
	thewin = wind_create_grect(NAME | CLOSER | MOVER, &winpos);
	wind_set_str(thewin, WF_NAME, gl_program_name);
	wind_open_grect(thewin, &winpos);
	graf_mouse(ARROW, NULL);
	SendAV_ACCWINDOPEN(thewin);
	stguide = appl_find("ST-GUIDE");
	if (stguide >= 0)
	{
		SendVA_START(stguide, "-S1", FUNK_NULL);
		SendVA_START(stguide, "", FUNK_NULL);
	}
}

/*** ---------------------------------------------------------------------- ***/

void DoVA_START(_WORD msg[8])
{
	SendAV_STARTED(msg);
	wind_set_top(thewin);
}

/*** ---------------------------------------------------------------------- ***/

void DoVA_DRAGACCWIND(_WORD msg[8])
{
	UNUSED(msg);
}

/*** ---------------------------------------------------------------------- ***/

static void redraw(_WORD *msg)
{
	_WORD ret;
	GRECT box;
	
	wind_update(BEG_UPDATE);
	graf_mouse(M_OFF, NULL);
	wind_get_grect(msg[3], WF_WORKXYWH, (GRECT *)&theobj[0].ob_x);
	ret = wind_get_grect(msg[3], WF_FIRSTXYWH, &box);
	while (ret != 0 && box.g_w && box.g_h)
	{
		if (rc_intersect((GRECT *) &msg[4], &box))
		{
			objc_draw_grect(theobj, 0, MAX_DEPTH, &box);
		}
		ret = wind_get_grect(msg[3], WF_NEXTXYWH, &box);
	}
	graf_mouse(M_ON, NULL);
	wind_update(END_UPDATE);
}

/*** ---------------------------------------------------------------------- ***/

void DoMessage(_WORD *msg)
{
	gem_print_message(msg);

	switch ((_UWORD)msg[0])
	{
	case AC_OPEN:
		va_proto_init(NULL);
		create_win();
		break;
	
	case AC_CLOSE:
		thewin = -1;
		break;
	
	case AP_TERM:
	case WM_CLOSED:
		delete_win();
		doneFlag = TRUE;
		break;
	
	case WM_REDRAW:
		if (msg[3] == thewin)
			redraw(msg);
		break;

	case WM_TOPPED:
		wind_set_top(msg[3]);
		break;

	case WM_MOVED:
		wind_set_grect(msg[3], WF_CURRXYWH, (GRECT *) &msg[4]);
		break;
		
	case WM_BOTTOMED:
		wind_set_int(msg[3], WF_BOTTOM, 0);
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

int main(int argc, char **argv)
{
	const char *applname = "REMARKER";
	
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

	if (DoInitSystem() == FALSE)
	{
		singletos_fail_loop();
		appl_exit();
		return 1;
	}
	
	menu_register(-1, applname);
	va_proto_init(applname);
	
	if (!_app)							/* running as ACC? */
		menu_register(gl_apid, "  " PROGRAM_NAME);	/* ...register to menu */

	if (_app)
		create_win();
		
	while (!_app || !doneFlag)
	{
		_WORD msg[8];
		
		evnt_mesag(msg);
		DoMessage(msg);
	}

	delete_win();
	
	DoExitSystem();

	return 0;
}
