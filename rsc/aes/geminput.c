/*		GEMINPUT.C		1/28/84 - 09/12/85		Lee Jay Lorenzen		*/
/*		merge High C vers. w. 2.2				8/21/87 		mdf 	*/

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
#include "gemlib.h"
#include "gsxdefs.h"

#define MB_DOWN 0x01


_WORD button;
_WORD xrat;
_WORD yrat;
_WORD kstate;
_WORD mclick;
_WORD mtrans;

_WORD pr_button;
_WORD pr_xrat;
_WORD pr_yrat;
_WORD pr_mclick;


AESPD *gl_mowner;		/* current mouse owner  */
AESPD *gl_kowner;		/* current keybd owner  */
AESPD *gl_cowner;		/* current control rect. owner */

/*
 * screen manager process that controls the mouse when it is
 * outside the control rectangle.
 */
AESPD *ctl_pd;

/*
 * # of times into the desired button state
 */
_WORD gl_bclick;

/*
 * the current desired button state
 */
_WORD gl_bdesired;

/*
 * the current true button state
 */
_WORD gl_btrue;

/*
 * number of pending events desiring more than a single click
 */
_WORD gl_bpend;

#if AESVERSION >= 0x330
_WORD gl_button;
#endif

/*
 * the current amount of time before the
 * button event is considered finished
 */
_WORD gl_bdelay;

int32_t TICKS, CMP_TICK, NUM_TICK;
