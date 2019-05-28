/*		GEMFMLIB.C		03/15/84 - 06/16/85 	Gregg Morris			*/
/*		merge High C vers. w. 2.2 & 3.0 		8/20/87 		mdf 	*/

/*
 *		Copyright 1999, Caldera Thin Clients, Inc.
 *				  2002-2016 The EmuTOS development team
 *
 *		This software is licenced under the GNU Public License.
 *		Please see LICENSE.TXT for further information.
 *
 *				   Historical Copyright
 *		-------------------------------------------------------------
 *		GEM Application Environment Services			  Version 2.3
 *		Serial No.	XXXX-0000-654321			  All Rights Reserved
 *		Copyright (C) 1987						Digital Research Inc.
 *		-------------------------------------------------------------
 */

#include "aes.h"
#include "gem_rsc.h"
#include "dos.h"

#undef DELETE /* clashes with Win32 */

#define FORWARD 0
#define BACKWARD 1
#define DEFLT 2
#define BELL 0x07						/* bell         */

#define BACKSPACE 0x0E08				/* backspace        */
#define SPACE 0x3920					/* ASCII <space>    */
#define ARROW_UP 0x4800					/* up arrow     */
#define ARROW_DOWN 0x5000				/* down arrow       */
#define ARROW_LEFT 0x4B00				/* left arrow       */
#define ARROW_RIGHT 0x4D00				/* right arrow      */
#define DELETE 0x5300					/* keypad delete    */
#define TAB 0x0F09						/* tab          */
#define BACKTAB 0x0F00					/* backtab      */
#define RETURN 0x1C0D					/* carriage return  */

#define ENTER 0x720D					/* enter key on keypad  */


/* Global variables: */
_WORD ml_ocnt;    /* Needs to be 0 initially! */


/* Local variables: */
static OBJECT *ml_mnhold;
static GRECT ml_ctrl;
static AESPD *ml_pmown;

/*
 * The following arrays are used by eralert() to generate values to
 * pass to fm_show().  eralert() is passed an alert number N which is
 * used to index into these arrays:
 *	ml_alrt[N]	contains the object number of the alert string within
 *				the resource
 *	ml_pwlv[N]	contains a 2-byte value PPLL where:
 *				LL is the alert level (0-3 currently supported)
 *				PP is a drive letter indicator; if non-zero, a
 *				drive letter corresponding to drive number D is
 *				passed to fm_show()
 *
 * These arrays must be synchronized with "err_tbl" in gemdosif.S.
 */
static _WORD const ml_alrt[] = { AL00CRT, AL01CRT, AL02CRT, AL03CRT, AL04CRT, AL05CRT, ALRTDSWAP };
static _WORD const ml_pwlv[] = { 0x0102, 0x0102, 0x0102, 0x0101, 0x0002, 0x0001, 0x0101 };


/*	0 = end mouse control	*/
/*	1 = mouse control	*/
void fm_own(_BOOL beg_ownit)
{
	if (beg_ownit)
	{
		wm_update(BEG_UPDATE);
		if (ml_ocnt == 0)
		{
			ml_mnhold = gl_mntree;		/* save the current menu   */
			gl_mntree = NULL;			/* no menu         */
			/* save the control rect */
			get_ctrl(&ml_ctrl);
			/* save the mouse and keyboard owner */
			get_mkown(&ml_pmown);

			/* change mouse ownership and the control rect */
			ct_chgown(rlr, &gl_rscreen);
		}
		ml_ocnt++;
	} else
	{
		ml_ocnt--;
		if (ml_ocnt == 0)
		{
			ct_chgown(ml_pmown, &ml_ctrl);	/* restore mouse owner     */
			gl_mntree = ml_mnhold;			/* restore menu tree       */
		}
		wm_update(END_UPDATE);
	}
}


/*
 *	Routine to find the next editable text field, or a field that
 *	is marked as a default return field.
 */
static _WORD find_obj(OBJECT *tree, _WORD start_obj, _WORD which)
{
	_WORD obj, flag, inc;
	_WORD theflag;
	
	obj = 0;
	flag = OF_EDITABLE;
	inc = 1;

	switch (which)
	{
	case BACKWARD:
		inc = -1;
		obj = start_obj + inc;
		break;

	case FORWARD:						/* check if it is OF_LASTOB before inc */
		if (!(tree[start_obj].ob_flags & OF_LASTOB))
			obj = start_obj + inc;
		else
			obj = -1;
		break;

	case DEFLT:
		flag = OF_DEFAULT;
		break;
	}

	while (obj >= 0)
	{
		theflag = tree[obj].ob_flags;

		if (!(theflag & OF_HIDETREE) && !(tree[obj].ob_state & OS_DISABLED) && (theflag & flag))
			return obj;

		if (theflag & OF_LASTOB)
			obj = NIL;
		else
			obj += inc;
	}

	return start_obj;
}



static _WORD fm_inifld(OBJECT *tree, _WORD start_fld)
{
	/* position cursor on the starting field */
	if (start_fld == 0)
		start_fld = find_obj(tree, 0, FORWARD);

	return start_fld;
}


/*
 * AES #55 - form_keybd - Process keyboard input in a dialog box form.
 */
_WORD fm_keybd(OBJECT *tree, _WORD obj, _WORD *pchar, _WORD *pnew_obj)
{
	_WORD direction;

	/* handle character */
	direction = -1;

	switch (*pchar)
	{
	case RETURN:
	case ENTER:
		obj = 0;
		direction = DEFLT;
		break;
	case BACKTAB:
	case ARROW_UP:
		direction = BACKWARD;
		break;
	case TAB:		/* shift-tab, ctrl-tab, alt-tab have the same scancode */
	case ARROW_DOWN:
		direction = FORWARD;
		break;
	}

	if (direction != -1)
	{
		*pchar = 0;
		*pnew_obj = find_obj(tree, obj, direction);
		if (direction == DEFLT && *pnew_obj != 0)
		{
			ob_change(tree, *pnew_obj, tree[*pnew_obj].ob_state | OS_SELECTED, TRUE);
			return FALSE;
		}
	}

	return TRUE;
}


/*
 * AES #56 - form_button - Simulate the clicking on an object.
 */
_WORD fm_button(OBJECT *tree, _WORD new_obj, _WORD clks, _WORD *pnew_obj)
{
	_WORD tobj;
	_UWORD orword;
	_WORD parent, state, flags;
	_WORD cont, tstate, tflags;
	_WORD lrets[6];

	cont = TRUE;
	orword = 0;

	state = ob_fs(tree, new_obj, &flags);

	/* handle touchexit case: if double click, then set high bit */
	if (flags & OF_TOUCHEXIT)
	{
		if (clks == 2)
			orword = 0x8000;
		cont = FALSE;
	}

	/* handle selectable case */
	if ((flags & OF_SELECTABLE) && !(state & OS_DISABLED))
	{
		/* if it's a radio button */
		if (flags & OF_RBUTTON)
		{
			/* check siblings to find and turn off the old OF_RBUTTON */
			parent = get_par(tree, new_obj);
			tobj = tree[parent].ob_head;
			while (tobj != parent)
			{
				tstate = ob_fs(tree, tobj, &tflags);
				if ((tflags & OF_RBUTTON) && ((tstate & OS_SELECTED) || tobj == new_obj))
				{
					if (tobj == new_obj)
						state = tstate |= OS_SELECTED;
					else
						tstate &= ~OS_SELECTED;
					ob_change(tree, tobj, tstate, TRUE);
				}
				tobj = tree[tobj].ob_next;
			}
		} else
		{
			/* turn on new object */
			if (gr_watchbox(tree, new_obj, state ^ OS_SELECTED, state))
				state ^= OS_SELECTED;
		}
		/* if not touchexit then wait for button up */
		if (cont && (flags & (OF_SELECTABLE | OF_EDITABLE)))
			ev_button(1, 0x0001, 0x0000, lrets);
	}

	/* see if this selection gets us out */
	if ((state & OS_SELECTED) && (flags & OF_EXIT))
		cont = FALSE;

	/* handle click on another editable field */
	if (cont && ((flags & OF_HIDETREE) || (state & OS_DISABLED) || !(flags & OF_EDITABLE)))
		new_obj = 0;

	*pnew_obj = new_obj | orword;

	return cont;
}


/*
 * AES #50 - form_do - Process the dialog with input from the user.
 *
 *	ForM DO routine to allow the user to interactively fill out a
 *	form.  The cursor is placed at the starting field. This routine
 *	returns the object that caused the exit to occur
 */
_WORD fm_do(OBJECT *tree, _WORD start_fld)
{
	_WORD edit_obj;
	_WORD next_obj;
	_WORD which, cont;
	_WORD idx;
	_WORD rets[6];

	/* grab ownership of screen and mouse */
	fm_own(TRUE);

	/* flush keyboard */
	fq();

	/* set clip so we can draw chars, and invert buttons */
	gsx_sclip(&gl_rfull);

	/* determine which is the starting field to edit */
	next_obj = fm_inifld(tree, start_fld);
	edit_obj = 0;

	/* interact with user */
	cont = TRUE;
	while (cont)
	{
		/* position cursor on the selected editing field */
		if (next_obj != 0 && edit_obj != next_obj)
		{
			edit_obj = next_obj;
			next_obj = 0;
			ob_edit(tree, edit_obj, 0, &idx, EDINIT);
		}

		/* wait for mouse or key */
		which = ev_multi(MU_KEYBD | MU_BUTTON, NULL, NULL, 0L, combine_cms(2, 0xff, 1), NULL, rets);

		/* handle keyboard event */
		if (which & MU_KEYBD)
		{
			cont = fm_keybd(tree, edit_obj, &rets[4], &next_obj);
			if (rets[4])
				ob_edit(tree, edit_obj, rets[4], &idx, EDCHAR);
		}

		/* handle button event */
		if (which & MU_BUTTON)
		{
			next_obj = ob_find(tree, ROOT, MAX_DEPTH, rets[0], rets[1]);
			if (next_obj == NIL)
			{
				sound(TRUE, 440, 2);
				next_obj = 0;
			} else
			{
				cont = fm_button(tree, next_obj, rets[5], &next_obj);
			}
		}

		/* handle end of field clean up */
		if (!cont || (next_obj != 0 && next_obj != edit_obj))
		{
			ob_edit(tree, edit_obj, 0, &idx, EDEND);
		}
	}

	/* give up mouse and screen ownership */
	fm_own(FALSE);

	/* return exit object */
	return next_obj;
}


/*
 * AES #51 - form_dial - Reserve or release memory for a dialog object.
 *
 *	Form DIALogue routine to handle visual effects of drawing and
 *	undrawing a dialogue
 */
_WORD fm_dial(_WORD fmd_type, const GRECT *pi, const GRECT *pt)
{
	/* adjust tree position */
	gsx_sclip(&gl_rscreen);
	switch (fmd_type)
	{
	case FMD_START:
		/* grab screen sync or some other mutual exclusion method */
		break;
	case FMD_GROW:
		/* growing box */
		gr_growbox(pi, pt);
		break;
	case FMD_SHRINK:
		/* shrinking box */
		gr_shrinkbox(pi, pt);
		break;
	case FMD_FINISH:
		/* update certain portion of the screen */
		w_drawdesk(pt);
		w_update(DESK, pt, DESK, FALSE);
		break;
	}

	return TRUE;
}


/*
 * issue form_alert(), with optional drive letter
 */
_WORD fm_show(_WORD string, _WORD level, _WORD arg)
{
	const char *ad_alert;
	
	ad_alert = rs_str(string);
	sprintf(D.alert_str, ad_alert, arg);
	ad_alert = D.alert_str;
	return fm_alert(level, ad_alert, 0);
}


/*
 * eralert() is called by the graphics critical alert handler
 * in gemdosif.S when a DOS error occurs.  it issues an alert
 * selected by the lookup tables ml_alrt[] & ml_pwlv[].
 *
 * input:	n	alert number (0-5), not checked
 *			d	drive number
 * returns: FALSE	user selected button 1 of the alert
 *					(assumed to be Cancel)
 *			TRUE	user selected button 2 or 3 of the alert
 *					(assumed to be Retry)
 */
_WORD eralert(_WORD n, _WORD d)
{
	_WORD level;
	_WORD drive_let;

	drive_let = 'A' + d;

	level = ml_pwlv[n] & 0xff;
	return fm_show(ml_alrt[n], level, drive_let) != 1;
}


/*
 * AES #53 - form_error - Display an alert box form for TOS errors. 
 *
 * n = dos error number
 */
_BOOL fm_error(_WORD n)
{
	_WORD string;

	if (n > 63)							/* nothing for xtal errors */
		return FALSE;

	switch (n)
	{
	case E_FILENOTFND:				/* file not found   */
	case E_NOFILES:					/* no more files    */
	case E_PATHNOTFND:				/* path not found   */
		string = AL18ERR;
		break;
	case E_NOHANDLES:				/* too many open files  */
		string = AL04ERR;
		break;
	case E_NOACCESS:				/* access denied    */
		string = AL05ERR;
		break;
	case E_NOMEMORY:				/* insufficient memory  */
	case E_BADENVIR:				/* invalid environmeny  */
	case E_BADFORMAT:				/* invalid format   */
		string = AL08ERR;
		break;
	case E_BADDRIVE:				/* invalid drive    */
		string = AL15ERR;
		break;
	default:
		string = ALXXERR;
		break;
	}

	return fm_show(string, 1, n) != 1;
}
