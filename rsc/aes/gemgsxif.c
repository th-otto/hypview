/*
 * gemgsxif.c - AES's interface to VDI (gsx)
 *
 * Copyright 2002-2016 The EmuTOS development team
 *			 1999, Caldera Thin Clients, Inc.
 *			 1987, Digital Research Inc.
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 *
 *                  Historical Copyright
 *	-------------------------------------------------------------
 *	GEM Application Environment Services		  Version 1.1
 *	Serial No.  XXXX-0000-654321		  All Rights Reserved
 *	Copyright (C) 1985			Digital Research Inc.
 *	-------------------------------------------------------------
 */

/*		GEMGSXIF.C		05/06/84 - 06/13/85 	Lee Lorenzen			*/
/*		merge High C vers. w. 2.2				8/21/87 		mdf 	*/

/*
 * Calls used in Crystal:
 *
 * vsf_interior();
 * vr_recfl();
 * vst_height();
 * vsl_type();
 * vsl_udsty();
 * vsl_width();
 * v_pline();
 * vs_clip();
 * vex_butv();
 * vex_motv();
 * vex_curv();
 * vex_timv();
 * vr_cpyfm();
 * v_opnwk();
 * v_clswk();
 * vq_extnd();
 * v_clsvwk()
 * v_opnvwk()
 */

#include "aes.h"
#include "debug.h"
#ifdef __atarist__
#include <osbind.h>
#endif
#include "s_endian.h"

#define aestrap_intercepted() 0


/*
 * calculate memory size to buffer a display area, given its
 * width (in words), height (in pixels), and the number of planes
 */
#define memsize(wdwidth,h,nplanes)	((_LONG)(wdwidth)*(h)*(nplanes)*2)

/*
 * the minimum menu/alert buffer size
 *
 * in the documentation for older versions of Atari TOS, it's said to be
 * one-quarter of the screen size, but in fact TOS uses 8192 bytes rather
 * than 8000.  this difference shows up when running QED in medium res:
 * the space required for the largest menu item is 8160 bytes, and there
 * are no menu droppings under e.g. Atari TOS 1.04.
 */
#define MIN_MENU_BUFFER_SIZE	(8*1024)	/* 1/4 of 32kB */

/*
 * the following specify the maximum dimensions of a form_alert, in
 * characters, derived by trial.  note that the actual maximum height
 * is 78 pixels (for 8-pixel chars) and 150 pixels (for 16-pixel chars),
 * so these are conservative numbers.
 */
#define MAX_ALERT_WIDTH  50 	/* includes worst-case screen alignment */
#define MAX_ALERT_HEIGHT 10


/*
 * counting semaphore
 * == 0 implies ON
 * >  0 implies OFF
 */
int gl_moff;

static _BOOL gl_graphic;
static MFDB gl_tmp;
static VEX_MOTV old_mcode;
static VEX_BUTV old_bcode;
static _LONG gl_mlen;
static VEX_CURV drwaddr;
short gl_restype;
MFORM gl_cmform;				/* current aes mouse form   */
MFORM gl_omform;				/* old aes mouse form       */





static _LONG form_alert_bufsize(void)
{
	int w = MAX_ALERT_WIDTH * gl_wchar;
	int h = MAX_ALERT_HEIGHT * gl_hchar;

	if (w > gl_width)		/* e.g. max size form alert in ST low */
		w = gl_width;

	return (_LONG)h * w * gl_nplanes / 8;
}

/* this function calculates the size of the menu/alert screen buffer.
 * as in older versions of Atari TOS, we use a value of one-quarter of
 * the screen size, with adjustment upwards if necessary to allow for
 * both the defined minimum buffer size and for the maximum size of
 * form alert.
 */
static _ULONG gsx_mcalc(void)
{
	_LONG mem;
	
	gsx_fix(&gl_tmp, 0x0L, 0, 0);			/* store screen info	*/
#if 0 /* function used by EmuTOS */
	gl_mlen = memsize(gl_tmp.fd_wdwidth,gl_tmp.fd_h,gl_tmp.fd_nplanes) / 4;
#endif
#if 0 /* Function used by TOS 2.x/3.x */
	gl_mlen = (int32_t)(gl_ws.ws_yres + 1) * (gl_ws.ws_xres + 1);
	gl_mlen = (gl_nplanes * gl_mlen) / 16;
#endif
#if 1
	/*
	 * this ensures a full screenheight of 25 characters
	 */
	mem = (_LONG) ((_UWORD) gl_wchar) * (_LONG) 25 * (_LONG) ((_UWORD) gl_height) * (_LONG) ((_UWORD) gl_nplanes);
	mem = mem / 8;
	gl_mlen = mem;
#endif

	if (gl_mlen < MIN_MENU_BUFFER_SIZE)
		gl_mlen = MIN_MENU_BUFFER_SIZE;

	mem = form_alert_bufsize();
	if (gl_mlen < mem)
		gl_mlen = mem;

	return gl_mlen;
}



/*
 * Save 25 columns and full height of the screen memory
 */
_BOOL gsx_malloc(void)
{
	_ULONG	 mlen;

	mlen = gsx_mcalc(); 					/* need side effects now	 */
	gl_tmp.fd_addr = dos_alloc_anyram(mlen);
	if (gl_tmp.fd_addr == NULL)
	{
		gl_mlen = 0;
		dos_conws("Unable to alloc AES blt buffer!\r\n");
		return FALSE;
	}
	return TRUE;
}



void gsx_mfree(void)
{
	dos_free(gl_tmp.fd_addr);
	gl_tmp.fd_addr = 0;
}



void gsx_mret(void **pmaddr, _LONG *pmlen)
{
	*pmaddr = gl_tmp.fd_addr;
	*pmlen = gl_mlen;
}



static void gsx_wsopen(void)
{
	_WORD i;
	_WORD intin[16];

	static short const restype[] =  { 0, 1, 1, 1, 2, 3, 4, 5, 6 };
	static short const restable[] = { 0, 2, 5, 7, 3, 4, 6, 8, 9 };
	
	for (i = 0; i < 10; i++)
		intin[i] = 1;
	intin[10] = 2;					/* device coordinate space */
	i = 0;
	while (restype[i] != gl_restype)
		i++;
	intin[0] = restable[i];
	
	v_opnwk(intin, &gl_handle, &gl_ws.ws_xres);

#if 0
	/* TODO: if Falcon: */
	/* check mode code */
	setres();
#endif
	/* default to 1280 x 960 */
	gl_restype = 5;
	if (gl_ws.ws_xres == 639)
	{
		if (gl_ws.ws_yres == 199)                         /* 640x200 */
			gl_restype = 2;
		else if (gl_ws.ws_yres == 399)                    /* 640x400 */
			gl_restype = 3;
		else
			gl_restype = 4;                               /* 640x480 */
	} else if (gl_ws.ws_xres == 319)
	{
		if (gl_ws.ws_yres == 199)                         /* 320x200 */
			gl_restype = 1;
		else
			gl_restype = 6;                               /* 320x480 */
	}
	gl_graphic = TRUE;
}



void gsx_wsclose(void)
{
	v_clswk(gl_handle);
}



void gsx_wsclear(void)
{
	v_clrwk(gl_handle);
}



void ratinit(void)
{
	v_show_c(gl_handle, 0); /* show cursor: force show */
	gl_moff = 0;
}


void ratexit(void)
{
	gsx_moff();
}


static void gsx_setmb(VEX_BUTV boff, VEX_MOTV moff, VEX_CURV *pdrwaddr)
{
	vex_butv(gl_handle, boff, &old_bcode);
	vex_motv(gl_handle, moff, &old_mcode);
#if 0 /* dont' replace cursor draw vector */
	vex_curv(gl_handle, justretf, pdrwaddr);
#endif
	UNUSED(pdrwaddr);
}



static void gsx_resetmb(void)
{
	VEX_BUTV ignored;
	VEX_MOTV ignored2;
	
	vex_butv(gl_handle, old_bcode, &ignored);
	vex_motv(gl_handle, old_mcode, &ignored2);
#if 0
	{
		VEX_CURV ignored3;
		vex_curv(gl_handle, drwaddr, &ignored3);
	}
#endif
}


/* AES mouse wheel handler called by the VDI */
#ifdef OS_ATARI
void aes_wheel(void)
{
}
#else
void aes_wheel(_WORD wheel_number, _WORD wheel_amount)
{
#if NYI
	forkq(wheel_change, MAKE_ULONG(wheel_number, wheel_amount));
#endif
	(void) wheel_number;
	(void) wheel_amount;
}
#endif


/* AES button handler called by the VDI */
#ifdef OS_ATARI
void far_bcha(void)
{
}
#else
void far_bcha(_WORD newmask)
{
#if NYI
	b_click(newmask);
#endif
	(void) newmask;
}
#endif


/* AES mouse handler called by the VDI */
#ifdef OS_ATARI
void far_mcha(void)
{
}
#else
void far_mcha(_WORD x, _WORD y)
{
#if NYI
	forkq(mchange, MAKE_ULONG(x, y));
#endif
	(void) x;
	(void) y;
}
#endif


/* AES timer handler called by the VDI */
void tikcod(void)
{
#if NYI
	/* bump up the absolute clock */
	++TICKS;
	/* are we timing now? */
	if (CMP_TICK != 0)
	{
		++NUM_TICK;
		if (--CMP_TICK == 0)
		{
			/*
			 * we need to establish a forkq that will
			 * pick us up
			 */
			 if (forkq(tchange, NUM_TICK) == FALSE)
			 	++CMP_TICK; /* no event recorded, reset the counter */
		}
	}
	b_delay(1);
	(*tiksav)();
#endif
}


void gsx_init(void)
{
	_WORD status;
	
	gsx_wsopen();
	gsx_start();
	gsx_setmb(far_bcha, far_mcha, &drwaddr);
	vq_mouse(gl_handle, &status, &xrat, &yrat);

	/*
	 * if NVDI3 has been installed, it will see the following VDI call.
	 * since it doesn't understand vex_wheelv() (which is a Milan extension),
	 * it will attempt to issue a warning form_alert().  this leads to a
	 * crash due to a stack smash: the USP and SSP are both on the AES
	 * process 0 stack at this time, and e.g. a VBL interrupt will save
	 * registers which can overwrite dynamic variables.
	 *
	 * we circumvent this by avoiding calling vex_wheelv() if the trap #2
	 * interrupt has been intercepted.
	 */
	if (!aestrap_intercepted())
	{
		VEX_WHEELV old_wheelv; /* Ignored */
		vex_wheelv(gl_handle, aes_wheel, &old_wheelv);
	}
}



void gsx_graphic(_BOOL tographic)
{
	if (gl_graphic != tographic)
	{
		gl_graphic = tographic;
		if (gl_graphic)
		{
			v_exit_cur(gl_handle);
			gsx_setmb(far_bcha, far_mcha, &drwaddr);
		} else
		{
			v_enter_cur(gl_handle);
			gsx_resetmb();
		}
	}
}



static void bb_set(_WORD sx, _WORD sy, _WORD sw, _WORD sh, _WORD *pts1, _WORD *pts2, MFDB *pfd, MFDB *psrc, MFDB *pdst)
{
	_WORD oldsx;
	_LONG size;

	/* get on word boundary */
	oldsx = sx;
	sx = sx & ~0x0f;
	sw = ((oldsx - sx) + (sw + 15)) & ~0x0f;

	size = memsize(sw / 16, sh, gl_tmp.fd_nplanes);

	if (size > gl_mlen)		/* buffer too small */
	{
		/* adjust height to fit buffer: this will leave droppings! */
		sh = gl_mlen * sh / size;

		/* issue warning message for backup only, not for subsequent restore */
		if (pdst == &gl_tmp)
		{
			KINFO(("Menu/alert buffer too small: need at least %ld bytes\n", size));
		}
	}

	gl_tmp.fd_stand = TRUE;
	gl_tmp.fd_wdwidth = sw / 16;
	gl_tmp.fd_w = sw;
	gl_tmp.fd_h = sh;

	gsx_moff();
	pts1[0] = sx;
	pts1[1] = sy;
	pts1[2] = sx + sw - 1;
	pts1[3] = sy + sh - 1;
	pts2[0] = 0;
	pts2[1] = 0;
	pts2[2] = sw - 1;
	pts2[3] = sh - 1;

	gsx_fix(pfd, NULL, 0, 0);
	if (pts2 < pts1)
		pts1 = pts2;
	vro_cpyfm(gl_handle, S_ONLY, pts1, psrc, pdst);
	gsx_mon();
}


void bb_save(GRECT *ps)
{
	_WORD ptsin[8];
	MFDB gl_src;
	
	bb_set(ps->g_x, ps->g_y, ps->g_w, ps->g_h, &ptsin[0], &ptsin[4], &gl_src, &gl_src, &gl_tmp);
}


void bb_restore(GRECT *pr)
{
	_WORD ptsin[8];
	MFDB gl_dst;
	
	bb_set(pr->g_x, pr->g_y, pr->g_w, pr->g_h, &ptsin[4], &ptsin[0], &gl_dst, &gl_tmp, &gl_dst);
}


_WORD gsx_tick(VEX_TIMV tcode, VEX_TIMV *ptsave)
{
	_WORD time_conv;
	
	vex_timv(gl_handle, tcode, ptsave, &time_conv);
	return time_conv;
}


void gsx_xmfset(MFORM *pmfnew)
{
	gsx_moff();
	vsc_form(gl_handle, &pmfnew->mf_xhot);
	gsx_mon();
}


void gsx_mfset(MFORM *pmfnew)
{
	gsx_moff();
	gl_omform = gl_cmform;
	gl_cmform = *pmfnew;
	vsc_form(gl_handle, &gl_cmform.mf_xhot);
	gsx_mon();
}



void gsx_mxmy(_WORD *pmx, _WORD *pmy)
{
	*pmx = xrat;
	*pmy = yrat;
}



_WORD gsx_button(void)
{
	return button;
}


void gsx_moff(void)
{
	if (!gl_moff)
	{
		v_hide_c(gl_handle);
		gl_mouse = FALSE;
	}
	
	gl_moff++;
}


void gsx_mon(void)
{
	gl_moff--;
	if (!gl_moff)
	{
		v_show_c(gl_handle, 1);
		gl_mouse = TRUE;
	}
}


_WORD gsx_kstate(void)
{
	_WORD state;
	vq_key_s(gl_handle, &state);
	return state;
}


_WORD gsx_char(void)
{
	_WORD achar = 0;
	_WORD echoxy[2];
	
	/* set sample mode for keyboard */
	vsin_mode(gl_handle, 4, 2);

	/* request max length of 1, returning scancodes, no echo */
	vsm_string16(gl_handle, -1, 0, echoxy, &achar);
	
	return achar;
}


/* Get the number of planes (or bit depth) of the current screen */
_WORD gsx_nplanes(void)
{
	_WORD workout[57];
	
	vq_extnd(gl_handle, 1, workout);
	return workout[4];
}
