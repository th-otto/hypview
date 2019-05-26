/*		GEMEVLIB.C		1/28/84 - 09/12/85		Lee Jay Lorenzen		*/
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
 *		Copyright (C) 1986						Digital Research Inc.
 *		-------------------------------------------------------------
 */

#include "aes.h"

static _WORD const gl_dcrates[5] = { 450, 330, 275, 220, 165 };

static _WORD gl_dcindex;
/*
 * # of ticks to wait to see if a second click will occur
 */
_WORD gl_dclick;
_WORD gl_ticktime;



/*
 * AES #26 - evnt_dclick - Obtain or set the time delay between the two clicks of a double-elick.
 */
_WORD ev_dclick(_WORD rate, _WORD setit)
{
	if (setit && (unsigned int)rate < (sizeof(gl_dcrates) / sizeof(gl_dcrates[0])))
	{
		gl_dcindex = rate;
		gl_dclick = gl_dcrates[gl_dcindex] / gl_ticktime;
	}

	return gl_dcindex;
}
