/*	GSXDEFS.H	05/06/84 - 12/08/84	Lee Lorenzen		*/

/*
 *       Copyright 1999, Caldera Thin Clients, Inc.                      
 *       This software is licenced under the GNU Public License.         
 *       Please see LICENSE.TXT for further information.                 
 *                                                                       
 *                  Historical Copyright
 *	-------------------------------------------------------------
 *	GEM Application Environment Services		  Version 3.0
 *	Serial No.  XXXX-0000-654321		  All Rights Reserved
 *	Copyright (C) 1986			Digital Research Inc.
 *	-------------------------------------------------------------
 */

#ifndef GSXDEFS_H
#define GSXDEFS_H 1

#include "ws.h"

typedef void (*PFVOID)(void);

extern int gl_moff;
extern _BOOL gl_mouse;					/* mouse on flag        */
extern VEX_TIMV tiksav;


/*
 * gemgsxif.c
 */

_BOOL gsx_malloc(void);
void gsx_mfree(void);
void gsx_mret(void **pmaddr, long *pmlen);


void gsx_init(void);
void gsx_graphic(_BOOL tographic);
void gsx_wsclose(void);
void gsx_wsclear(void);
void ratinit(void);
void ratexit(void);
void bb_save(GRECT *ps);
void bb_restore(GRECT *ps);
void gsx_mfset(MFORM *pmfnew);
void gsx_xmfset(MFORM *pmfnew);
void gsx_mxmy(_WORD *pmx, _WORD *pmy);
_WORD gsx_tick(VEX_TIMV tcode, VEX_TIMV *ptsave);
_WORD gsx_button(void);
_WORD gsx_kstate(void);
void gsx_moff(void);
void gsx_mon(void);
_WORD gsx_char(void);
_WORD gsx_nplanes(void);
_BOOL sound(_BOOL isfreq, _WORD freq, _WORD duration);


/*
 * apgsxif.c
 */
void gsx_fix(MFDB *pfd, _WORD *theaddr, _WORD wb, _WORD h);
void gsx_blt(_WORD *saddr, _UWORD sx, _UWORD sy, _UWORD sw, _WORD *daddr, _UWORD dx, _UWORD dy, _UWORD dw, _UWORD w, _UWORD h, _UWORD rule, _WORD fgcolor, _WORD bgcolor);
void gsx_sclip(const GRECT *pt);
void gsx_gclip(GRECT *pt);
_BOOL gsx_chkclip(GRECT *pt);
void gsx_cline(_UWORD x1, _UWORD y1, _UWORD x2, _UWORD y2);
void gsx_attr(_BOOL text, _UWORD mode, _UWORD color);
void gsx_box(const GRECT *pt);
void bb_screen(_WORD scrule, _WORD scsx, _WORD scsy, _WORD scdx, _WORD scdy, _WORD scw, _WORD sch);
void gsx_trans(_WORD *saddr, _UWORD sw, _WORD *daddr, _UWORD dw, _UWORD h, _WORD fg, _WORD bg);
void gsx_start(void);
void bb_fill(_WORD mode, _WORD fis, _WORD patt, _WORD hx, _WORD hy, _WORD hw, _WORD hh);
_WORD gsx_tcalc(_WORD font, const char *ptext, _WORD *ptextw, _WORD *ptexth, vdi_wchar_t *textout);
void gsx_tblt(_WORD tb_f, _WORD x, _WORD y, const vdi_wchar_t *wtext, _WORD len);
void gsx_xbox(GRECT *pt);
void gsx_xcbox(GRECT *pt);

#endif /* GSXDEFS_H */
