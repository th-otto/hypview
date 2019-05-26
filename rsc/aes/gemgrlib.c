/*		GEMGRLIB.C		4/11/84 - 09/20/85		Gregg Morris			*/
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
#include "gem_rsc.h"



/*
 *	Routine to watch the mouse while the button is down and
 *	it stays inside/outside of the specified rectangle.
 *	Return TRUE as long as the mouse is down.  Block until the
 *	mouse moves into or out of the specified rectangle.
 */

#if 0
static _BOOL gr_stilldn(MOBLK *out)
{
	_BOOL status;
	MOBLK tmpmoblk;

	/* form a MOBLK */
	tmpmoblk = *out;

	for (;;)
	{
		forker();
		if (!(button & 0x01))
		{
			status = FALSE;
			break;
		} else
		{
			if (tmpmoblk.m_out != inside(xrat, yrat, &tmpmoblk.m_gr))
			{
				status = TRUE;
				break;
			}
		}
	}
	return status;
}
#elif 0
static _BOOL gr_stilldn(MOBLK *out)
{
	_WORD rets[6];
	_WORD event;
	MOBLK tmpmoblk;
	
	/* form a MOBLK */
	tmpmoblk = *out;

	for (;;)
	{
		event = ev_multi(MU_BUTTON | MU_M1 | MU_TIMER, &tmpmoblk, NULL, 0L, combine_cms(1, 1, 0), NULL, rets);

		if (event & MU_BUTTON)			/* button up */
			return FALSE;

		if (event & MU_M1)
			return TRUE;

		if (event & MU_TIMER)
		{
			if (!(rets[2] & 0x01))
				return FALSE;
		}
	}
}
#else
static _BOOL gr_stilldn(MOBLK *out)
{
	int16_t rets[6];
	int16_t event;
	MOBLK tmpmoblk;

	/* form a MOBLK */
	tmpmoblk = *out;

	event = ev_multi(MU_BUTTON | MU_M1, &tmpmoblk, NULL, 0L, combine_cms(1, 1, 0), NULL, rets);

	if (event & MU_BUTTON)			/* button up */
		return FALSE;
	return TRUE;
}
#endif


static void gr_setup(_WORD color)
{
	gsx_sclip(&gl_rscreen);
	gsx_attr(FALSE, MD_XOR, color);
}


static void gr_clamp(_WORD xorigin, _WORD yorigin, _WORD wmin, _WORD hmin, _WORD *pneww, _WORD *pnewh)
{
	_WORD mx, my;
	
	gsx_mxmy(&mx, &my);
	*pneww = max(mx - xorigin + 1, wmin);
	*pnewh = max(my - yorigin + 1, hmin);
}


static void gr_scale(_WORD xdist, _WORD ydist, _WORD *pcnt, _WORD *pxstep, _WORD *pystep)
{
	_WORD i;
	_WORD dist;

	gr_setup(G_BLACK);

	dist = xdist + ydist;

	i = -1;
	do
	{
		++i;
	} while ((dist >= 1) != 0);

	if ((*pcnt = i) != 0)
	{
		*pxstep = max(1, xdist / i);
		*pystep = max(1, ydist / i);
	} else
	{
		*pxstep = *pystep = 1;
	}
}


void gr_stepcalc(_WORD orgw, _WORD orgh, const GRECT *pt, _WORD *pcx, _WORD *pcy, _WORD *pcnt, _WORD *pxstep, _WORD *pystep)
{
	*pcx = (pt->g_w / 2) - (orgw / 2);
	*pcy = (pt->g_h / 2) - (orgh / 2);

	gr_scale(*pcx, *pcy, pcnt, pxstep, pystep);

	*pcx += pt->g_x;
	*pcy += pt->g_y;
}


static void gr_xor(_WORD clipped, _WORD cnt, _WORD cx, _WORD cy, _WORD cw, _WORD ch, _WORD xstep, _WORD ystep, _WORD dowdht)
{
	GRECT tmprect;

	do
	{
		tmprect.g_x = cx;
		tmprect.g_y = cy;
		tmprect.g_w = cw;
		tmprect.g_h = ch;

		if (clipped)
			gsx_xcbox(&tmprect);
		else
			gsx_xbox(&tmprect);

		cx -= xstep;
		cy -= ystep;
		if (dowdht)
		{
			cw += (2 * xstep);
			ch += (2 * ystep);
		}
	} while (cnt--);
}


static void gr_draw(_WORD have2box, GRECT *po, GRECT *poff)
{
	GRECT t;

	gsx_xbox(po);
	if (have2box)
	{
		r_set(&t, po->g_x + poff->g_x, po->g_y + poff->g_y, po->g_w + poff->g_w, po->g_h + poff->g_h);
		gsx_xbox(&t);
	}
}


static _WORD gr_wait(GRECT *po, GRECT *poff)
{
	_WORD have2box;
	_WORD down;
	MOBLK mo;

	have2box = !rc_equal(&gl_rzero, poff);

	/* draw/erase old */
	gsx_moff();
	gr_draw(have2box, po, poff);
	gsx_mon();

	/* wait for change */
	mo.m_out = TRUE;
	r_set(&mo.m_gr, xrat, yrat, 1, 1);
	down = gr_stilldn(&mo);

	/* draw/erase old */
	gsx_moff();
	gr_draw(have2box, po, poff);
	gsx_mon();

	/* return exit event */
	return down;
}


/*
 *	Stretch the free corner of an XOR box(w,h) that is pinned at
 *	another corner, based on mouse movement until the button comes up.
 *	Also draw a second box offset from the stretching box.
 */
void gr_rubwind(_WORD xorigin, _WORD yorigin, _WORD wmin, _WORD hmin, GRECT *poff, _WORD *pwend, _WORD *phend)
{
	_WORD down;
	GRECT o;

	wm_update(TRUE);
	gr_setup(G_BLACK);

	r_set(&o, xorigin, yorigin, 0, 0);

	/* clamp size of rubber box to no smaller than wmin, hmin */
	do
	{
		gr_clamp(o.g_x, o.g_y, wmin, hmin, &o.g_w, &o.g_h);
		down = gr_wait(&o, poff);
	} while (down);

	*pwend = o.g_w;
	*phend = o.g_h;
	wm_update(FALSE);
}


/*
 * AES #70 - graf_rubberbox - Graphics rubberbox
 *
 *	Stretch the free corner of a XOR box(w,h) that is pinned at
 *	another corner, based on mouse movement until the button comes up.
 *	This is also called a rubber-band box.
 */
void gr_rubbox(_WORD xorigin, _WORD yorigin, _WORD wmin, _WORD hmin, _WORD *pwend, _WORD *phend)
{
	gr_rubwind(xorigin, yorigin, wmin, hmin, &gl_rzero, pwend, phend);
}


/*
 * AES #71 - graf_dragbox - Graphics drag box
 *
 *	Drag a moving XOR box(w,h) that tracks relative to the mouse until the
 *	button comes up.  The starting x and y represent the location of the
 *	upper left hand corner of the rectangle relative to the mouse position.
 *	This relative distance should be maintained.  A constraining rectangle
 *	is also given.  The box should not be able to be dragged out of the
 *	constraining rectangle.
 */
void gr_dragbox(_WORD w, _WORD h, _WORD sx, _WORD sy, const GRECT *pc, _WORD *pdx, _WORD *pdy)
{
	_WORD offx, offy, down;
	GRECT o;
	_WORD mx, my;
	
	wm_update(BEG_UPDATE);
	gr_setup(G_BLACK);

	gr_clamp(sx + 1, sy + 1, 0, 0, &offx, &offy);
	r_set(&o, sx, sy, w, h);

	/* get box's x,y from mouse's x,y then constrain result */
	do
	{
		gsx_mxmy(&mx, &my);
		o.g_x = mx - offx;
		o.g_y = my - offy;
		rc_constrain(pc, &o);
		down = gr_wait(&o, &gl_rzero);
	} while (down);

	*pdx = o.g_x;
	*pdy = o.g_y;
	wm_update(END_UPDATE);
}


static void gr_2box(_WORD flag1, _WORD cnt, const GRECT *pt, _WORD xstep, _WORD ystep, _WORD flag2)
{
	_WORD i;

	gsx_moff();
	for (i = 0; i < 2; i++)
		gr_xor(flag1, cnt, pt->g_x, pt->g_y, pt->g_w, pt->g_h, xstep, ystep, flag2);
	gsx_mon();
}


/*
 * AES #72 - graf_movebox - Graphics move box
 *
 *	Draw a moving XOR box(w,h) that moves from a point whose upper
 *	left corner is at src_x, src_y to a point at dst_x, dst_y
 */
void gr_movebox(_WORD w, _WORD h, _WORD srcx, _WORD srcy, _WORD dstx, _WORD dsty)
{
	_WORD signx, signy;
	_WORD cnt;
	_WORD xstep, ystep;
	GRECT t;

	r_set(&t, srcx, srcy, w, h);

	signx = (srcx < dstx) ? -1 : 1;
	signy = (srcy < dsty) ? -1 : 1;

	gr_scale(signx * (srcx - dstx), signy * (srcy - dsty), &cnt, &xstep, &ystep);

	gr_2box(FALSE, cnt, &t, signx * xstep, signy * ystep, FALSE);
}


/*
 * AES #73 - graf_growbox - Graphics grow box
 *
 *	Draw a small box that moves from the origin x,y to a spot centered
 *	within the rectangle and then expands to match the size of the rectangle.
 */
void gr_growbox(const GRECT *po, const GRECT *pt)
{
	GRECT xorb;
	_WORD cnt, xstep, ystep;

	gr_stepcalc(po->g_w, po->g_h, pt, &xorb.g_x, &xorb.g_y, &cnt, &xstep, &ystep);
	gr_movebox(po->g_w, po->g_h, po->g_x, po->g_y, xorb.g_x, xorb.g_y);
	xorb.g_w = po->g_w;
	xorb.g_h = po->g_h;
	gr_2box(TRUE, cnt, &xorb, xstep, ystep, TRUE);
}


/*
 * AES #74 - graf_shrinkbox - Graphics shrink box
 *
 *	Draw a box that shrinks from the rectangle given down around a small
 *	box centered within the rectangle and then moves to the origin point.
 */
void gr_shrinkbox(const GRECT *po, const GRECT *pt)
{
	_WORD cx, cy;
	_WORD cnt, xstep, ystep;

	gr_stepcalc(po->g_w, po->g_h, pt, &cx, &cy, &cnt, &xstep, &ystep);
	gr_2box(TRUE, cnt, pt, -xstep, -ystep, TRUE);
	gr_movebox(po->g_w, po->g_h, cx, cy, po->g_x, po->g_y);
}


/*
 * AES #75 - graf_watchbox - Graphics watch box
 */
_WORD gr_watchbox(OBJECT *tree, _WORD obj, _WORD instate, _WORD outstate)
{
	_WORD state;
	MOBLK t;

	gsx_sclip(&gl_rscreen);
	ob_actxywh(tree, obj, &t.m_gr);

	t.m_out = FALSE;
	do
	{
		state = t.m_out ? outstate : instate;
		ob_change(tree, obj, state, TRUE);
		t.m_out = !t.m_out;
	} while (gr_stilldn(&t));

	return t.m_out;
}


/*
 * AES #76 - graf_slidebox - Graphics slide box
 */
_WORD gr_slidebox(OBJECT *tree, _WORD parent, _WORD obj, _WORD isvert)
{
	GRECT *pt, *pc;
	GRECT t, c;
	int32_t divnd, divis;
	OBJECT *objc;
	_WORD pflags, cflags;
	_WORD ret, setxy;

	pt = &t;
	pc = &c;

	/* get the parent real position */
	ob_actxywh(tree, parent, pc);
	/* get the relative position    */
	ob_relxywh(tree, obj, pt);

	objc = tree;
	pflags = objc[parent].ob_flags;
	cflags = objc[obj].ob_flags;

	if (gl_aes3d && (pflags & OF_FL3DIND))
		pflags = 1;
	else
		pflags = 0;
	
	if (gl_aes3d && (cflags & OF_FL3DIND))
		cflags = 1;
	else
		cflags = 0;

	setxy = 0;

	if (cflags)							/* if the child is 3D, then check       */
	{
		if (!pflags)					/* if parent is not 3D, then the child is   */
		{								/* ADJ3DPIX off, we need to adjust it       */
			pt->g_x -= ADJ3DPIX;
			pt->g_y -= ADJ3DPIX;
			setxy = 1;
		}
		pt->g_w += (ADJ3DPIX << 1);
		pt->g_h += (ADJ3DPIX << 1);
	}

	gr_dragbox(pt->g_w, pt->g_h, pt->g_x + pc->g_x, pt->g_y + pc->g_y, pc, &pt->g_x, &pt->g_y);

	if (isvert)							/* vertical movement    */
	{
		divnd = pt->g_y - pc->g_y;
		divis = pc->g_h - pt->g_h;
	} else								/* horizontal movement  */
	{
		divnd = pt->g_x - pc->g_x;
		divis = pc->g_w - pt->g_w;
	}

	if (setxy)
	{
		if (divnd)
			divnd += ADJ3DPIX;
	}

	if (divis)
	{
		ret = mul_div(divnd, 1000, divis);
		if (ret > 1000)
			ret = 1000;
		return ret;
	}
	return 0;
}


/*
 * AES #78 - graf_mouse - Change the appearance of the mouse pointer.
 *
 * Graf mouse
 */
void gr_mouse(_WORD mkind, MFORM *grmaddr)
{
	MFORM omform;

	if ((_UWORD) mkind > USER_DEF)
	{
		switch (mkind)
		{
		case M_OFF:
			gsx_moff();
			break;

		case M_ON:
			gsx_mon();
			break;

#if AESVERSION >= 0x320
		case M_SAVE:						/* save mouse form  */
			rlr->p_mouse = gl_cmform;
			break;

		case M_RESTORE:						/* restore saved mouse form */
			omform = rlr->p_mouse;
			gsx_mfset(&omform);
			break;

		case M_PREV:						/* restore previous mouse form  */
			omform = gl_omform;
			gsx_mfset(&omform);
			break;
#endif
		}
	} else
	{
		if (mkind != USER_DEF)				/* set new mouse form   */
		{
			if (mkind < MICE00 || mkind > MICE07)
				mkind = MICE00;
			grmaddr = (MFORM *)aes_rsc_bitblk[mkind]->bi_pdata;
		}

		gsx_mfset(grmaddr);
	}
}


/*
 * AES #79 - graf_mkstate - Graphics mouse and keyboard status
 */
_WORD gr_mkstate(_WORD *pmx, _WORD *pmy, _WORD *pmstat, _WORD *pkstat)
{
	*pmx = xrat;
	*pmy = yrat;
	*pmstat = button;
	*pkstat = kstate;
	return TRUE;
}


void set_mouse_to_arrow(void)
{
	gr_mouse(ARROW, NULL);
}


void set_mouse_to_hourglass(void)
{
	gr_mouse(HOURGLASS, NULL);
}
