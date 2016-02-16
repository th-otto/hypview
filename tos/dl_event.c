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
#include "hypview.h"


#include "av.h"
#if USE_BUBBLEGEM
#include "bubble.h"
#endif
#if USE_DOCUMENTHISTORY
#include "dhst.h"
#endif
#if USE_GEMSCRIPT
#include "gscript.h"
#endif

short doneFlag = FALSE, quitApp = FALSE;

#define setmsg(a,b,c,d,e,f,g,h) \
	msg[0] = a; \
	msg[1] = b; \
	msg[2] = c; \
	msg[3] = d; \
	msg[4] = e; \
	msg[5] = f; \
	msg[6] = g; \
	msg[7] = h

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void DoKeybd(EVNT *event)
{
	_WORD ascii = event->key, scan;
	_WORD kstate = event->kstate;
	
	ConvertKeypress(&ascii, &kstate);

	scan = (ascii >> 8) & 0xff;
	ascii = ascii & 0xff;
	
	if (kstate == (K_CTRL | K_ALT))
	{
		if (ascii == ' ')
		{
			FlipIconify();
			event->mwhich &= ~MU_KEYBD;
		}
	} else if (kstate == (K_CTRL | K_ALT | K_RSHIFT | K_LSHIFT))
	{
		if (ascii == ' ')
		{
			_WORD msg[8];
			_WORD top;
			wind_get_int(DESK, WF_TOP, &top);
			setmsg(WM_ALLICONIFY, gl_apid, 0, top, -1, -1, -1, -1);
			appl_write(gl_apid, 16, msg);
			event->mwhich &= ~MU_KEYBD;
		}
	} else if (kstate == K_CTRL)
	{
		switch (ascii)
		{
		case 'Q':
#if USE_MENU
			ChooseMenu(ME_FILE, ME_QUIT);
#else
			doneFlag = TRUE;
#endif
			event->mwhich &= ~MU_KEYBD;
			break;
		case 'W':
			CycleItems();
			event->mwhich &= ~MU_KEYBD;
			break;
		case 'U':
			{
				_WORD msg[8];
				_WORD top;
				wind_get_int(DESK, WF_TOP, &top);
				setmsg(WM_CLOSED, gl_apid, 0, top, 0, 0, 0, 0);
				appl_write(gl_apid, 16, &msg[0]);
				event->mwhich &= ~MU_KEYBD;
			}
			break;
		}
	} else if (kstate == 0)
	{
		if (scan == KbHELP)				/* HELP */
		{
#if 0
			STGuideHelp();
			event->mwhich &= ~MU_KEYBD;
#endif
		}
	}

	HYP_DBG(("Key: %d Scan: %d  Ascii: %d -- %c %c %c %c %s%s%s%s%s",
		event->key, scan, ascii, ascii,
		((unsigned char *)key_table->unshift)[scan],
		((unsigned char *)key_table->shift)[scan],
		((unsigned char *)key_table->caps)[scan],
		((kstate & K_CTRL) ? "+CTRL" : ""),
		((kstate & K_ALT) ? "+ALT" : ""),
		((kstate & K_LSHIFT) ? "+LSHIFT" : ""),
		((kstate & K_RSHIFT) ? "+RSHIFT" : ""),
		((kstate & 0x10 /*KbNUM */) ? "+NUM" : "")));
	UNUSED(scan);
}

/*** ---------------------------------------------------------------------- ***/

static void DoMessage(EVNT *event)
{
	switch ((_UWORD)event->msg[0])
	{
#if USE_MENU
	case MN_SELECTED:
		ChooseMenu(event->msg[3], event->msg[4]);
		break;
#endif
	case AP_TERM:
		if (_app)
			quitApp = TRUE;
#if USE_MENU
		ChooseMenu(ME_FILE, ME_QUIT);
#else
		doneFlag = TRUE;
#endif
		break;
#if USE_DRAGDROP
	case AP_DRAGDROP:
		DragDrop(event->msg);
		break;
#endif
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
	case AV_PROTOKOLL:
		DoVA_Message(event->msg);
		break;
	case VA_DRAGACCWIND:
		DoVA_DRAGACCWIND(event->msg);
		break;
	case VA_START:						/* pass command line */
		DoVA_START(event->msg);
		break;
	case AV_SENDCLICK:					/* mouse click reported (BubbleGEM) */
		event->mwhich = MU_BUTTON;
		event->mx = event->msg[3];
		event->my = event->msg[4];
		event->mbutton = event->msg[5];
		event->kstate = event->msg[6];
		event->mclicks = event->msg[7];
		DoEventDispatch(event);
		break;
	case AV_SENDKEY:					/* key press reported (BubbleGEM) */
		event->mwhich = MU_KEYBD;
		event->kstate = event->msg[3];
		event->key = event->msg[4];
		DoEventDispatch(event);
		break;

	case AV_EXIT:
		DoVA_Message(event->msg);
#if USE_GEMSCRIPT
		gemscript_handle_message(event->msg);
#endif
		break;

#if USE_BUBBLEGEM
	case BUBBLEGEM_REQUEST:
		Bubble(event->msg[4], event->msg[5]);
		break;
	case BUBBLEGEM_ACK:
		break;
#endif
#if USE_DOCUMENTHISTORY
	case DHST_ACK:
		DhstFree(event->msg);
		break;
#endif

#if USE_GEMSCRIPT
	case GS_REQUEST:
	case GS_REPLY:
	case GS_QUIT:
	case GS_COMMAND:
	case GS_ACK:
	case GS_CLOSEMACRO:
	case GS_MACRO:
		gemscript_handle_message(event->msg);
		break;
#endif

	default:
		HYP_DBG(("Message :%d %x erhalten", event->msg[0], event->msg[0]));
		break;
	}

}

/*** ---------------------------------------------------------------------- ***/

void DoEventDispatch(EVNT *event)
{
	if ((event->mwhich & MU_MESAG) && event->msg[2] > 0)				/* extended message ?? */
	{
		short *xmsg;

		xmsg = (short *)g_malloc(event->msg[2]);	/* reserve memory */
		if (xmsg != NULL)
		{
			appl_read(gl_apid, event->msg[2], xmsg);	/* read "message" */
			g_free(xmsg);
		}
	}

	DoUserEvents(event);

	if (event->mwhich & MU_BUTTON)
		DoButton(event);

	if (event->mwhich & MU_KEYBD)
		DoKeybd(event);

	ItemEvent(event);

	if (event->mwhich & MU_MESAG)
		DoMessage(event);
}

/*** ---------------------------------------------------------------------- ***/

void DoEvent(void)
{
	EVNT event;

	memset(&event, 0, sizeof(event));
	event.mwhich = evnt_multi_gemlib(EVENTS, MBCLICKS, MBMASK, MBSTATE, MBLOCK1, MBLOCK2, event.msg, WAIT,
		&event.mx, &event.my, &event.mbutton, &event.kstate, &event.key, &event.mclicks);
	DoEventDispatch(&event);
	if (doneFlag)
	{
		RemoveItems();
		doneFlag = FALSE;
	}
}
