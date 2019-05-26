/*      GEMSHLIB.C      4/18/84 - 09/13/85      Lee Lorenzen            */
/*      merge High C vers. w. 2.2               8/24/87         mdf     */
/*      fix sh_envrn                            11/17/87        mdf     */

/*
 *       Copyright 1999, Caldera Thin Clients, Inc.
 *                 2002-2016 The EmuTOS development team
 *
 *       This software is licenced under the GNU Public License.
 *       Please see doc/license.txt for further information.
 *
 *                  Historical Copyright
 *       -------------------------------------------------------------
 *       GEM Application Environment Services              Version 2.3
 *       Serial No.  XXXX-0000-654321              All Rights Reserved
 *       Copyright (C) 1987                      Digital Research Inc.
 *       -------------------------------------------------------------
 */

#include "aes.h"
#include "gemlib.h"
#include "gsxdefs.h"
#include "dos.h"
#include "gem_rsc.h"


#define CMD_BAT    0xFA
#define CMD_COPY   0xFB
#define CMD_FORMAT 0xFC
#define CMD_PRINT  0xFD
#define CMD_TYPE   0xFE

/* if TRUE then do an an exec on the current command else exit and return to DOS  */
_WORD sh_doexec;

/* used to signal if the currently running appl is a GEM app */
_WORD sh_isgem;

/* same as above for previously running DOS app. */
_WORD gl_shgem;

char *ad_envrn;

char *ad_shcmd;

char *ad_shtail;

/* cart program */
_BOOL sh_iscart;

char *ad_path;

/*
 *	Convert the screen to graphics-mode in preparation for the 
 *	running of a GEM-based graphic application.
 */
_BOOL sh_tographic(void)
{
	gsx_graphic(TRUE);					/* convert to graphic   */
	gsx_sclip(&gl_rscreen);				/* set initial clip rect */
	gsx_malloc();						/* allocate screen space BUG: not checked */
	gsx_mfset((MFORM *)aes_rsc_bitblk[MICE02]->bi_pdata);				/* put mouse to hourglass */
	ratinit();							/* start up the mouse   */

	return TRUE;
}
