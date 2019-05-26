/*		GEMCTRL.C		5/14/84 - 08/22/85		Lee Jay Lorenzen		*/
/*		GEM 2.0 		11/06/85				Lowell Webster			*/
/*		merge High C vers. w. 2.2 & 3.0 		8/19/87 		mdf 	*/
/*		fix menu bar hang						11/16/87		mdf 	*/

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
#include "gempd.h"

#define THEDESK 3

#define MBDOWN 0x0001
#define BELL 0x07						/* bell         */

_BOOL gl_mouse;
MOBLK gl_ctwait;
_WORD appl_msg[8];

/*
 *	Create a process for the Screen Control Manager and start him 
 *	executing. Also do all the initialization that is required.  
 *	Also zero out the desk accessory count.
 */
AESPD *ictlmgr(void)
{
	AESPD *p;
	
	gl_dacnt = 0;
	gl_dabase = 0;
	/* create process to execute it */
	p = &D.g_pd[curpid];
	p->p_pid = curpid++;
	return p;
}
