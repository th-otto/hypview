/*
 *************************************************************************
 *
 *       Copyright 1999, Caldera Thin Clients, Inc.
 *       Copyright (C) 2016 The EmuTOS development team
 *
 *       This software is licenced under the GNU Public License.
 *       Please see LICENSE.TXT for further information.
 *
 *                  Historical Copyright
 *       -----------------------------------------------------------
 *       GEM Application Environment Services            Version 2.3
 *       Serial No.  XXXX-0000-654321            All rights Reserved
 *       Copyright (C) 1986                    Digital Research Inc.
 *       -----------------------------------------------------------
 *************************************************************************
 */
#ifndef WS_H
#define WS_H 1

typedef struct wsstr
{
	_WORD		ws_xres;
	_WORD		ws_yres;
	_WORD		ws_noscale;
	_WORD		ws_wpixel;
	_WORD		ws_hpixel;
	_WORD		ws_ncheights;
	_WORD		ws_nlntypes;
	_WORD		ws_nlnwidths;
	_WORD		ws_nmktypes;
	_WORD		ws_nmksizes;
	_WORD		ws_nfaces;
	_WORD		ws_npatts;
	_WORD		ws_nhatchs;
	_WORD		ws_ncolors;
	_WORD		ws_ngdps;
	_WORD		ws_supgdps[10];
	_WORD		ws_attgdps[10];
	_WORD		ws_color;
	_WORD		ws_rotate;
	_WORD		ws_fill;
	_WORD		ws_cell;
	_WORD		ws_npals;
	_WORD		ws_nloc;
	_WORD		ws_nval;
	_WORD		ws_nchoice;
	_WORD		ws_nstring;
	_WORD		ws_type;
	_WORD		ws_pts0;
	_WORD		ws_chminh;
	_WORD		ws_pts2;
	_WORD		ws_chmaxh;
	_WORD		ws_lnminw;
	_WORD		ws_pts5;
	_WORD		ws_lnmaxw;
	_WORD		ws_pts7;
	_WORD		ws_pts8;
	_WORD		ws_mkminw;
	_WORD		ws_pts10;
	_WORD		ws_mkmaxw;
} WS;

extern WS gl_ws;

#endif
