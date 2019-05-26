/*		GEMINIT.C		4/23/84 - 08/14/85		Lee Lorenzen			*/
/*		GEMCLI.C		1/28/84 - 08/14/85		Lee Jay Lorenzen		*/
/*		GEM 2.0 		10/31/85				Lowell Webster			*/
/*		merge High C vers. w. 2.2				8/21/87 		mdf 	*/
/*		fix command tail handling				10/19/87		mdf 	*/

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


#define CACHE_ON	0x00003919L
#define CACHE_OFF	0x00000808L
#define LONGFRAME	*(_WORD *)(0x59eL)

#define STATIC


/* set in jbind.s, checked by dispatcher    */
_WORD adeskp[3];				/* desktop colors & backgrounds */
STATIC _WORD awinp[3];			/* window colors & backgrounds */
_UWORD d_rezword;				/* default resolution for sparrow */
VEX_TIMV tiksav;
_BOOL gl_rschange;


void aes_init(void)
{
	_WORD i;
	OBJECT *tree;

	/****************************************/

	/* init event recorder */
	gl_recd = FALSE;
	gl_rlen = 0;
	gl_rbuf = 0;

	gl_btrue = 0x0;
	gl_bdesired = 0x0;
	gl_bdelay = 0x0;
	gl_bclick = 0x0;

	/* init initial process */
	for (i = 0; i < NUM_PDS; i++)
		pinit(&D.g_pd[i], &D.g_cda[i]);

	D.g_pd[0].p_uda = &D.g_uda;
	D.g_pd[1].p_uda = (UDA *)&D.g_2uda;
	D.g_pd[2].p_uda = (UDA *)&D.g_3uda;
	
	/* if not rlr then initialize his stack pointer */
	D.g_2uda.u_spsuper = &D.g_2uda.u_supstk;
	D.g_3uda.u_spsuper = &D.g_3uda.u_supstk;

	curpid = 0;

	rlr = &D.g_pd[curpid];
	rlr->p_pid = curpid++;
	rlr->p_link = 0;
	rlr->p_stat = PS_RUN;

	/* 
	 * screen manager process init.
	 * This process starts out owning the mouse
	 * and the keyboard. it has a pid == 1
	 */
	gl_mowner = gl_kowner = ctl_pd = ictlmgr();

	rsc_read();							/* read in resource */

#if 0
	pred_dinf();						/* pre read the inf */
#endif

	/* get the resolution and the auto boot name         */

	/* do gsx open work station */
	gsx_init();
	gl_rschange = FALSE;

	/* Init 3D-look of indicators and activators */
	act3dtxt = TRUE;					/* don't move text for activators */
	act3dface = FALSE;					/* no color change when activator is selected */
	ind3dtxt = FALSE;					/* move text for indicators */
	ind3dface = TRUE;					/* change color when indicator is selected */

	if (gl_ws.ws_ncolors <= G_LWHITE)
	{									/* init button color */
		gl_actbutcol = gl_indbutcol = G_WHITE;
		gl_alrtcol = G_WHITE;				/* init alert background color */
	} else
	{
		gl_actbutcol = gl_indbutcol = G_LWHITE;
		gl_alrtcol = G_LWHITE;			/* init alert background color */
	}

	gsx_mfset((MFORM *)aes_rsc_bitblk[MICE02]->bi_pdata);

	/* fix up icons */
	for (i = 0; i < 3; i++)
	{
		BITBLK bi;

		bi = *aes_rsc_bitblk[i];
		gsx_trans(bi.bi_pdata, bi.bi_wb * 8, bi.bi_pdata, bi.bi_wb * 8, bi.bi_hl, G_BLACK, G_WHITE);
	}
	
	sh_tographic();						/* go into graphic mode */
	gl_shgem = TRUE;

	/* take the tick int. */
	gl_ticktime = gsx_tick(tikcod, &tiksav);

	/* set init. click rate */
	ev_dclick(3, TRUE);

	/* get st_desk ptr */
	tree = aes_rsc_tree[DESKTOP];

	/* fix up the GEM rsc. file now that we have an open WS    */
	/* This code is also in gemshlib, but it belongs here so that the correct
	 * default GEM backdrop pattern is set for accessories and autoboot app.
	 */
	/* FIXME: use function from sh_main */
	if (gl_ws.ws_ncolors > 2)				/* set solid pattern in color modes */
		tree[ROOT].ob_spec.index = OBSPEC_MAKE(0, 0, G_BLACK, G_BLACK, FALSE, IP_SOLID, G_GREEN);
	else
		tree[ROOT].ob_spec.index = OBSPEC_MAKE(0, 0, G_BLACK, G_BLACK, FALSE, IP_4PATT, G_GREEN);

	wm_init();

	for (i = 0; i < 3; i++)
		tree[i].ob_width = gl_width;

	tree[ROOT].ob_height = gl_height;
	tree[1].ob_height = gl_hchar + 2;        /* the background bar */
	tree[APPTITLE].ob_height = gl_hchar + 3; /* the title */

	/* off we go !!! */
}


void aes_exit(void)
{
	rsc_free();							/* free up resource */

	gsx_mfree();

	/* give back the tick */
	gl_ticktime = gsx_tick(tiksav, &tiksav);

	v_exit_cur(gl_handle);						/* exit alpha mode */

	/* close workstation */
	gsx_wsclose();
}


/*
 * process init
 */
void pinit(AESPD *ppd, CDA *pcda)
{
	ppd->p_cda = pcda;
	ppd->p_qaddr = ppd->p_queue;
	ppd->p_qindex = 0;
	memset(ppd->p_name, ' ', AP_NAMELEN);
}
