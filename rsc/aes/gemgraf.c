/*		GEMGRAF.C		04/11/84 - 09/17/85 	Lee Lorenzen			*/
/*		merge High C vers. w. 2.2				8/21/87 		mdf 	*/
/*		fix gr_gicon null text					11/18/87		mdf 	*/

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
#include "debug.h"

#define YRES_LIMIT	380 	/* screens with yres less than this are considered */
							/*	'small' for the purposes of get_char_height()  */

_WORD gl_nplanes;					/* number of planes in current res */
_WORD gl_width;
_WORD gl_height;
_WORD gl_wchar;
_WORD gl_hchar;
_WORD gl_wschar;
_WORD gl_hschar;
_WORD gl_wptschar;
_WORD gl_hptschar;
_WORD gl_wsptschar;
_WORD gl_hsptschar;
_WORD gl_wbox;
_WORD gl_hbox;
GRECT gl_clip;
_WORD gl_handle;
_WORD gl_mode;
_WORD gl_tcolor;
_WORD gl_lcolor;
_WORD gl_fis;
_WORD gl_patt;
_WORD gl_font;
GRECT gl_rscreen;
GRECT gl_rfull;
GRECT gl_rzero;
GRECT gl_rcenter;
GRECT gl_rmenu;
WS gl_ws;


/*
 *	Routine to set the clip rectangle.	If the w,h of the clip is 0,
 *	then no clip should be set.  Otherwise, set the appropriate clip.
 */
void gsx_sclip(const GRECT *pt)
{
	_WORD pts[4];

	rc_copy(pt, &gl_clip);
	
	if (pt->g_w > 0 && pt->g_h > 0)
	{
		pts[0] = pt->g_x;
		pts[1] = pt->g_y;
		pts[2] = pt->g_x + pt->g_w - 1;
		pts[3] = pt->g_y + pt->g_h - 1;
		vs_clip(gl_handle, TRUE, pts);
	} else
	{
		vs_clip(gl_handle, FALSE, pts);
	}
}


/*
 *	Routine to get the current clip setting
 */
void gsx_gclip(GRECT *pt)
{
	rc_copy(&gl_clip, pt);
}


/*
 *	Routine to return TRUE iff the specified rectangle intersects the
 *	current clip rectangle ... or clipping is off (?)
 */
_BOOL gsx_chkclip(GRECT *pt)
{
	/* if clipping is on */
	if (gl_clip.g_w && gl_clip.g_h)
	{
		if ((pt->g_y + pt->g_h) < gl_clip.g_y)
			return FALSE;					/* all above */
		if ((pt->g_x + pt->g_w) < gl_clip.g_x)
			return FALSE;					/* all left */
		if ((gl_clip.g_y + gl_clip.g_h) <= pt->g_y)
			return FALSE;					/* all below */
		if ((gl_clip.g_x + gl_clip.g_w) <= pt->g_x)
			return FALSE;					/* all right */
	}

	return TRUE;
}


static void gsx_xline(_WORD ptscount, _WORD *ppoints)
{
	static _UWORD const hztltbl[2] = { 0x5555, 0xaaaa };
	static _UWORD const verttbl[4] = { 0x5555, 0xaaaa, 0xaaaa, 0x5555 };
	_WORD linexy, i;
	_WORD st;

	for (i = 1; i < ptscount; i++)
	{
		if (ppoints[0] == ppoints[2])
		{
			/*
			 * The line style "5555" is used if X and Y are both even or
			 * both odd. "AAAA" is used when X and Y are different.
			 */
			st = verttbl[((ppoints[0] & 1) | ((ppoints[1] & 1) << 1))];
		} else
		{
			linexy = (ppoints[0] < ppoints[2]) ? ppoints[1] : ppoints[3];
			st = hztltbl[linexy & 1];
		}
		vsl_udsty(gl_handle, st);
		v_pline(gl_handle, 2, ppoints);
		ppoints += 2;
	}
	vsl_udsty(gl_handle, -1);
}


/*
 *	Routine to draw a clipped polyline, hiding the mouse as you do it
 */
void gsx_cline(_UWORD x1, _UWORD y1, _UWORD x2, _UWORD y2)
{
	_WORD pxy[4];

	pxy[0] = x1;
	pxy[1] = y1;
	pxy[2] = x2;
	pxy[3] = y2;

	gsx_moff();
	v_pline(gl_handle, 2, pxy);
	gsx_mon();
}


/*
 *	Routine to set the writing mode.
 */
static void gsx_mode(_UWORD mode)
{
	if (mode != gl_mode)
	{
		gl_mode = mode;
		vswr_mode(gl_handle, mode);
	}
}


/*
 *	Routine to set the text, writing mode, and color attributes.
 */
void gsx_attr(_BOOL text, _UWORD mode, _UWORD color)
{
	gsx_mode(mode);

	if (text)
	{
		if (color != gl_tcolor)
		{
			gl_tcolor = color;
			vst_color(gl_handle, color);
		}
	} else
	{
		if (color != gl_lcolor)
		{
			gl_lcolor = color;
			vsl_color(gl_handle, color);
		}
	}
}


/*
 *	Routine to set up the points for drawing a box.
 */
static void gsx_bxpts(const GRECT *pt, _WORD *pts)
{
	pts[0] = pt->g_x;
	pts[1] = pt->g_y;
	pts[2] = pt->g_x + pt->g_w - 1;
	pts[3] = pt->g_y;
	pts[4] = pts[2];
	pts[5] = pt->g_y + pt->g_h - 1;
	pts[6] = pt->g_x;
	pts[7] = pts[5];
	pts[8] = pt->g_x;
	pts[9] = pt->g_y;
}


/*
 *	Routine to draw a box using the current attributes.
 */
void gsx_box(const GRECT *pt)
{
	_WORD pts[5 * 2];
	
	gsx_bxpts(pt, pts);
	/* do the top, right side and bottom */
	v_pline(gl_handle, 4, pts);
	/* now the left side line pattern, to avoid clipping problems */
	pts[2] = pts[6];
	pts[3] = pts[7] - 1;
	v_pline(gl_handle, 2, pts);
}


/*
 *	Routine to draw a box that will look right on a dithered surface.
 */
void gsx_xbox(GRECT *pt)
{
	_WORD pts[5 * 2];
	
	gsx_bxpts(pt, pts);
	gsx_xline(5, pts);
}


/*
 *	Routine to draw a portion of the corners of a box that will look
 *	right on a dithered surface.
 */
void gsx_xcbox(GRECT *pt)
{
	_WORD wa, ha;
	_WORD pts[3 * 2];
	
	wa = 2 * gl_wbox;
	ha = 2 * gl_hbox;
	pts[0] = pt->g_x;
	pts[1] = pt->g_y + ha;
	pts[2] = pt->g_x;
	pts[3] = pt->g_y;
	pts[4] = pt->g_x + wa;
	pts[5] = pt->g_y;
	gsx_xline(3, pts);

	pts[0] = pt->g_x + pt->g_w - wa;
	pts[1] = pt->g_y;
	pts[2] = pt->g_x + pt->g_w - 1;
	pts[3] = pt->g_y;
	pts[4] = pts[2];
	pts[5] = pt->g_y + ha;
	gsx_xline(3, pts);

	pts[0] = pt->g_x + pt->g_w - 1;
	pts[1] = pt->g_y + pt->g_h - ha;
	pts[2] = pts[0];
	pts[3] = pt->g_y + pt->g_h - 1;
	pts[4] = pt->g_x + pt->g_w - wa;
	pts[5] = pts[3];
	gsx_xline(3, pts);

	pts[0] = pt->g_x + wa;
	pts[1] = pt->g_y + pt->g_h - 1;
	pts[2] = pt->g_x;
	pts[3] = pts[1];
	pts[4] = pt->g_x;
	pts[5] = pt->g_y + pt->g_h - ha;
	gsx_xline(3, pts);
}


/*
 *	Routine to fix up the MFDB of a particular raster form
 */
void gsx_fix(MFDB *pfd, _WORD *theaddr, _WORD w, _WORD h)
{
	if (theaddr == ORGADDR)
	{
		pfd->fd_w = gl_width;
		pfd->fd_h = gl_height;
		pfd->fd_nplanes = gl_nplanes;
	} else
	{
		pfd->fd_w = w;
		pfd->fd_h = h;
		pfd->fd_nplanes = 1;
	}
	pfd->fd_wdwidth = (pfd->fd_w + 15) >> 4;
	pfd->fd_stand = FALSE;
	pfd->fd_addr = theaddr;
}


/*
 *	Routine to blit, to and from a specific area
 */
void gsx_blt(_WORD *saddr, _UWORD sx, _UWORD sy, _UWORD sw,
			 _WORD *daddr, _UWORD dx, _UWORD dy, _UWORD dw, _UWORD w, _UWORD h,
			 _UWORD rule, _WORD fgcolor, _WORD bgcolor)
{
	_WORD pts[4 * 2];
	MFDB gl_src;
	MFDB gl_dst;
	
	gsx_fix(&gl_src, saddr, sw, h);
	gsx_fix(&gl_dst, daddr, dw, h);

	gsx_moff();
	pts[0] = sx;
	pts[1] = sy;
	pts[2] = sx + w - 1;
	pts[3] = sy + h - 1;
	pts[4] = dx;
	pts[5] = dy;
	pts[6] = dx + w - 1;
	pts[7] = dy + h - 1;
	if (fgcolor < 0)
	{
		vro_cpyfm(gl_handle, rule, pts, &gl_src, &gl_dst);
	} else
	{
		_WORD colors[2];
		
		colors[0] = fgcolor;
		colors[1] = bgcolor;
		vrt_cpyfm(gl_handle, rule, pts, &gl_src, &gl_dst, colors);
	}
	gsx_mon();
}


/*
 *	Routine to blit around something on the screen
 */
void bb_screen(_WORD scrule, _WORD scsx, _WORD scsy, _WORD scdx, _WORD scdy, _WORD scw, _WORD sch)
{
	gsx_blt(ORGADDR, scsx, scsy, 0, ORGADDR, scdx, scdy, 0, scw, sch, scrule, -1, -1);
}


/*
 *	Routine to transform a standard form to device specific form.
 */
void gsx_trans(_WORD *saddr, _UWORD sw, _WORD *daddr, _UWORD dw, _UWORD h, _WORD fg, _WORD bg)
{
	MFDB gl_src;
	MFDB gl_dst;
	
	gsx_fix(&gl_src, saddr, sw, h);
	gl_src.fd_stand = TRUE;
	gl_src.fd_nplanes = 1;

	gsx_fix(&gl_dst, daddr, dw, h);
	vr_trnfm(gl_handle, &gl_src, &gl_dst);
	(void) fg;
	(void) bg;
}


/*
 *	Routine to initialize all the global variables dealing with
 *	a particular workstation open.
 */
void gsx_start(void)
{
	_WORD dummy;
	_WORD attrib[10];
	
	/* force update */
	gl_mode = gl_tcolor = gl_lcolor = -1;
	gl_fis = gl_patt = gl_font = -1;

	gl_clip.g_x = 0;
	gl_clip.g_y = 0;
	gl_width = gl_clip.g_w = gl_ws.ws_xres + 1;
	gl_height = gl_clip.g_h = gl_ws.ws_yres + 1;
	gl_nplanes = gsx_nplanes();

 	KDEBUG(("VDI video mode = %dx%d %d-bit\n", gl_width, gl_height, gl_nplanes));

	/*
	 * The driver may have more than two fonts. The large font
	 * to the services is the default font in the driver. This
	 * font will give us 80 chars across the screen in all but
	 * 320 pels in x, when it will be the 40 column font.
	 */
	vqt_attributes(gl_handle, attrib);
	gl_wptschar = attrib[6];
	gl_ws.ws_chmaxh = gl_hptschar = attrib[7];
	gl_wchar = attrib[8];
	gl_hchar = attrib[9];
	
	vst_height(gl_handle, gl_ws.ws_chminh, &gl_wsptschar, &gl_hsptschar, &gl_wschar, &gl_hschar);
	vst_height(gl_handle, gl_ws.ws_chmaxh, &dummy, &dummy, &dummy, &dummy);

	gl_hbox = gl_hchar + 3;
	gl_wbox = mul_div(gl_hbox, gl_ws.ws_hpixel, gl_ws.ws_wpixel);
	if (gl_wbox < gl_wchar + 2)
		gl_wbox = gl_wchar + 2;

	KDEBUG(("gsx_start(): gl_wchar=%d, gl_hchar=%d, gl_wbox=%d, gl_hbox=%d\n", gl_wchar, gl_hchar, gl_wbox, gl_hbox));

	vsl_type(gl_handle, USERLINE);
	vsl_width(gl_handle, 1);
	vsl_udsty(gl_handle, -1);
	r_set(&gl_rscreen, 0, 0, gl_width, gl_height);
	r_set(&gl_rfull, 0, gl_hbox, gl_width, gl_height - gl_hbox);
	r_set(&gl_rzero, 0, 0, 0, 0);
	r_set(&gl_rcenter, (gl_width - gl_wbox) / 2, (gl_height - 2 * gl_hbox) / 2, gl_wbox, gl_hbox);
	r_set(&gl_rmenu, 0, 0, gl_width, gl_hbox);
}


/*
 *	Routine to do a filled bit blit (a rectangle).
 */
void bb_fill(_WORD mode, _WORD fis, _WORD patt, _WORD hx, _WORD hy, _WORD hw, _WORD hh)
{
	_WORD pts[4];
	
	pts[0] = hx;
	pts[1] = hy;
	pts[2] = hx + hw - 1;
	pts[3] = hy + hh - 1;

	gsx_attr(TRUE, mode, gl_tcolor);
	if (fis != gl_fis)
	{
		vsf_interior(gl_handle, fis);
		gl_fis = fis;
	}
	if (patt != gl_patt)
	{
		vsf_style(gl_handle, patt);
		gl_patt = patt;
	}
	vr_recfl(gl_handle, pts);
}


static _UWORD ch_width(_WORD fn)
{
	if (fn == IBM)
		return gl_wchar;

	if (fn == SMALL)
		return gl_wschar;

	return 0;
}


static _UWORD ch_height(_WORD fn)
{
	if (fn == IBM)
		return gl_hchar;

	if (fn == SMALL)
		return gl_hschar;

	return 0;
}


_WORD gsx_tcalc(_WORD font, const char *ptext, _WORD *ptextw, _WORD *ptexth, vdi_wchar_t *textout)
{
	_WORD wc, hc;
	_WORD numchs;
	
	wc = ch_width(font);
	hc = ch_height(font);
	
	if (hc == 0)
	{
		KINFO(("gsx_tcalc: ERROR: font neither IBM nor SMALL\n"));
		*ptextw = 0;
		*ptexth = 0;
		return 0;
	}
	
	/* figure out the width of the text string in pixels */
	numchs = vdi_str2arrayn(ptext, textout, MAX_LEN);
	*ptextw = min(*ptextw, numchs * wc);

	/* figure out the height of the text */
	*ptexth = min(*ptexth, hc);
	if ((*ptexth / hc) == 0)
		return 0;
	return *ptextw / wc;
}


void gsx_tblt(_WORD tb_f, _WORD x, _WORD y, const vdi_wchar_t *wtext, _WORD tb_nc)
{
	if (tb_f == IBM)
	{
		if (tb_f != gl_font)
		{
			vst_height(gl_handle, gl_hptschar, &gl_wptschar, &gl_hptschar, &gl_wchar, &gl_hchar);
			gl_font = tb_f;
		}
		y += gl_hptschar;
	} else if (tb_f == SMALL)
	{
		if (tb_f != gl_font)
		{
			vst_height(gl_handle, gl_hsptschar, &gl_wsptschar, &gl_hsptschar, &gl_wschar, &gl_hschar);
			gl_font = tb_f;
		}
		y += gl_hsptschar;
	} else
	{
		KINFO(("gsx_tblt: ERROR: font neither IBM nor SMALL\n"));
		return;
	}

	v_gtext16n(gl_handle, x, y, wtext, tb_nc);
}


/*
 *	Routine to convert a rectangle to its inside dimensions.
 */
void gr_inside(GRECT *pt, _WORD th)
{
	pt->g_x += th;
	pt->g_y += th;
	pt->g_w -= 2 * th;
	pt->g_h -= 2 * th;
}


/*
 *	Routine to draw a colored, patterned, rectangle.
 */
void gr_rect(_UWORD icolor, _UWORD ipattern, GRECT *pt)
{
	_WORD fis;

	fis = FIS_PATTERN;
	if (ipattern == IP_HOLLOW)
		fis = FIS_HOLLOW;
	else if (ipattern == IP_SOLID)
		fis = FIS_SOLID;

	vsf_color(gl_handle, icolor);
	bb_fill(MD_REPLACE, fis, ipattern, pt->g_x, pt->g_y, pt->g_w, pt->g_h);
}


/*
 *	Routine to adjust the x,y starting point of a text string to
 *	account for its justification.	The number of characters in
 *	the string is also returned.
 */
_WORD gr_just(_WORD just, _WORD font, const char *ptext, _WORD w, _WORD h, GRECT *pt, vdi_wchar_t *textout)
{
	_WORD numchs;

	/* figure out the width of the text string in pixels */
	numchs = gsx_tcalc(font, ptext, &pt->g_w, &pt->g_h, textout);

	h -= pt->g_h;
	if (h > 0)				/* check height */
		pt->g_y += (h + 1) / 2;

	w -= pt->g_w;
	if (w > 0)
	{
		/* do justification */
		if (just == TE_CNTR)
			pt->g_x += (w + 1) / 2;
		else if (just == TE_RIGHT)
			pt->g_x += w;
#if 0
		{
			_WORD diff;
			
			/* try to byte align */
			if (font == IBM && (w > 16) && ((diff = (pt->g_x & 0x0007)) != 0))
			{
				if (just == TE_LEFT)
				{
					pt->g_x += 8 - diff;
				} else if (just == TE_CNTR)
				{
					if (diff > 3)
						diff -= 8;
					pt->g_x -= diff;
				} else if (just == TE_RIGHT)
				{
					pt->g_x -= diff;
				}
			}
		}
#endif
	}

	return numchs;
}


/*
 *	Routine to draw a string of graphic text.
 */
void gr_gtext(_WORD just, _WORD font, const char *ptext, GRECT *pt)
{
	_WORD numchs;
	GRECT t;
	vdi_wchar_t wtext[MAX_LEN];
	
	/* figure out where & how to put out the text */
	rc_copy(pt, &t);
	numchs = gr_just(just, font, ptext, t.g_w, t.g_h, &t, wtext);
	if (numchs > 0)
	{
		gsx_tblt(font, t.g_x, t.g_y, wtext, numchs);
	}
}


/*
 *	Routine to crack out the border color, text color, inside pattern,
 *	and inside color from a single color information word.
 */
void gr_crack(_UWORD color, _WORD *pbc, _WORD *ptc, _WORD *pip, _WORD *pic, _WORD *pmd)
{
	/* 4 bit encoded border color */
	*pbc = (color >> 12) & 0x000f;

	/* 4 bit encoded text color */
	*ptc = (color >> 8) & 0x000f;

	/* 1 bit used to set text writing mode */
	*pmd = (color & 0x80) ? MD_REPLACE : MD_TRANS;

	/* 3 bit encoded pattern */
	*pip = (color >> 4) & 0x0007;

	/* 4 bit encoded inside color */
	*pic = color & 0x000f;
}


static void gr_gblt(_WORD *pimage, GRECT *pi, _WORD col1, _WORD col2)
{
	gsx_blt(pimage, 0, 0, pi->g_w, ORGADDR, pi->g_x, pi->g_y, gl_width, pi->g_w, pi->g_h, MD_TRANS, col1, col2);
}


/*
 *	Routine to draw an icon, which is a graphic image with a text
 *	string underneath it.
 */
void gr_gicon(_WORD state, _WORD *pmask, _WORD *pdata, const char *ptext, vdi_wchar_t ch,
			  _WORD chx, _WORD chy, GRECT *pi, GRECT *pt)
{
	_WORD ifgcol, ibgcol;
	_WORD tfgcol, tbgcol, tmp;

	/* crack the color/char definition word */
	tfgcol = ifgcol = (ch >> 12) & 0x000f;
	tbgcol = ibgcol = (ch >> 8) & 0x000f;
	ch &= 0x00ff;

	/* invert if selected */
	if (state & OS_SELECTED)
	{
		tmp = tfgcol;
		tfgcol = tbgcol;
		tbgcol = tmp;
		if (!(state & OS_DRAW3D))
		{
			tmp = ifgcol;
			ifgcol = ibgcol;
			ibgcol = tmp;
		}
	}

	/* do mask unless it's on a white background */
	if (!((state & OS_WHITEBAK) && ibgcol == G_WHITE))
		gr_gblt(pmask, pi, ibgcol, ifgcol);

	if (!((state & OS_WHITEBAK) && tbgcol == G_WHITE))
	{
		if (pt->g_w)
			gr_rect(tbgcol, IP_SOLID, pt);
	}

	/* draw the data */
	gr_gblt(pdata, pi, ifgcol, ibgcol);

	if (!gl_aes3d && (state & OS_SELECTED) && (state & OS_DRAW3D))
	{
		pi->g_x--;
		pi->g_y--;
		gr_gblt(pmask, pi, ifgcol, ibgcol);
		pi->g_x += 2;
		pi->g_y += 2;
		gr_gblt(pmask, pi, ifgcol, ibgcol);
		pi->g_x--;
		pi->g_y--;
	}

	/* draw the character */
	gsx_attr(TRUE, MD_TRANS, ifgcol);
	if (ch)
	{
		gsx_tblt(SMALL, pi->g_x + chx, pi->g_y + chy, &ch, 1);
	}

	/* draw the label */
	gsx_attr(TRUE, MD_TRANS, tfgcol);
	gr_gtext(TE_CNTR, SMALL, ptext, pt);
}


/*
 * Routine to draw a box of a certain thickness using the current attribute settings.
 */
void gr_box(_WORD x, _WORD y, _WORD w, _WORD h, _WORD th)
{
	GRECT t, n;

	r_set(&t, x, y, w, h);
	if (th != 0)
	{
		if (th < 0)
			th--;
		gsx_moff();
		do
		{
			th += (th > 0) ? -1 : 1;
			rc_copy(&t, &n);
			gr_inside(&n, th);
			gsx_box(&n);
		} while (th != 0);
		gsx_mon();
	}
}
