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

#undef TOP
#undef LEFT
#undef RIGHT
#undef BOTTOM

#define TOP 	0
#define LEFT	1
#define RIGHT	2
#define BOTTOM	3

static ORECT *rul;
static ORECT gl_mkrect;


void or_start(void)
{
	_WORD i;

	rul = NULL;
	for (i = 0; i < NUM_ORECT; i++)
	{
		D.g_olist[i].o_link = rul;
		rul = &D.g_olist[i];
	}
}


ORECT *get_orect(void)
{
	ORECT *po;

	if ((po = rul) != NULL)
		rul = rul->o_link;

	return po;
}


static ORECT *mkpiece(_WORD tlrb, ORECT *newr, ORECT *old)
{
	ORECT *rl;

	rl = get_orect();
	rl->o_link = old;

	/* do common calcs */
	rl->o_gr.g_x = old->o_gr.g_x;
	rl->o_gr.g_w = old->o_gr.g_w;
	rl->o_gr.g_y = max(old->o_gr.g_y, newr->o_gr.g_y);
	rl->o_gr.g_h = min(old->o_gr.g_y + old->o_gr.g_h, newr->o_gr.g_y + newr->o_gr.g_h) - rl->o_gr.g_y;

	/* use/override calcs */
	switch (tlrb)
	{
	case TOP:
		rl->o_gr.g_y = old->o_gr.g_y;
		rl->o_gr.g_h = newr->o_gr.g_y - old->o_gr.g_y;
		break;
	case LEFT:
		rl->o_gr.g_w = newr->o_gr.g_x - old->o_gr.g_x;
		break;
	case RIGHT:
		rl->o_gr.g_x = newr->o_gr.g_x + newr->o_gr.g_w;
		rl->o_gr.g_w = (old->o_gr.g_x + old->o_gr.g_w) - (newr->o_gr.g_x + newr->o_gr.g_w);
		break;
	case BOTTOM:
		rl->o_gr.g_y = newr->o_gr.g_y + newr->o_gr.g_h;
		rl->o_gr.g_h = (old->o_gr.g_y + old->o_gr.g_h) - (newr->o_gr.g_y + newr->o_gr.g_h);
		break;
	}

	return rl;
}


static ORECT *brkrct(ORECT *newr, ORECT *r, ORECT *p)
{
	_WORD i;
	_WORD have_piece[4];

	/* break up rectangle r based on newr, adding new orects to list p */
	if ((newr->o_gr.g_x < r->o_gr.g_x + r->o_gr.g_w) &&
		(newr->o_gr.g_x + newr->o_gr.g_w > r->o_gr.g_x) &&
		(newr->o_gr.g_y < r->o_gr.g_y + r->o_gr.g_h) &&
		(newr->o_gr.g_y + newr->o_gr.g_h > r->o_gr.g_y))
	{
		/* there was overlap so we need newr rectangles */
		have_piece[TOP] = newr->o_gr.g_y > r->o_gr.g_y;
		have_piece[LEFT] = newr->o_gr.g_x > r->o_gr.g_x;
		have_piece[RIGHT] = (newr->o_gr.g_x + newr->o_gr.g_w) < (r->o_gr.g_x + r->o_gr.g_w);
		have_piece[BOTTOM] = (newr->o_gr.g_y + newr->o_gr.g_h) < (r->o_gr.g_y + r->o_gr.g_h);

		for (i = 0; i < 4; i++)
		{
			if (have_piece[i])
				p = (p->o_link = mkpiece(i, newr, r));
		}

		/* take out the old guy */
		p->o_link = r->o_link;
		r->o_link = rul;
		rul = r;
		return p;
	}

	return NULL;
}


/* tree = place holder for everyobj */
static void mkrect(OBJECT *tree, _WORD wh, _WORD junkx, _WORD junky)
{
	WINDOW *pwin;
	ORECT *newr;
	ORECT *r, *p;

	UNUSED(tree);
	UNUSED(junkx);
	UNUSED(junky);
	pwin = &D.w_win[wh];

	/* get the new rect that is used for breaking this windows rects */
	newr = &gl_mkrect;

	p = (ORECT *)&pwin->w_rlist;
	r = p->o_link;

	/* redo rectangle list */
	while (r)
	{
		if ((p = brkrct(newr, r, p)) != NULL)
		{
			/* we broke a rectangle which means this can't be blt */
			pwin->w_flags |= VF_BROKEN;
		} else
		{
			p = r;
		}
		r = p->o_link;
	}
}


void newrect(OBJECT *tree, _WORD wh, _WORD junkx, _WORD junky)
{
	WINDOW *pwin;
	ORECT *r, *newr;
	ORECT *r0;

	UNUSED(junkx);
	UNUSED(junky);
	pwin = &D.w_win[wh];
	r0 = pwin->w_rlist;

	/* dump rectangle list */
	if (r0)
	{
		for (r = r0; r->o_link; r = r->o_link)
			;
		r->o_link = rul;
		rul = r0;
	}

	/* zero the rectangle list */
	pwin->w_rlist = NULL;

	/* start out with no broken rectangles */
	pwin->w_flags &= ~VF_BROKEN;

	/* if no size then return */
	w_getsize(WS_TRUE, wh, &gl_mkrect.o_gr);
	if (!(gl_mkrect.o_gr.g_w && gl_mkrect.o_gr.g_h))
		return;

	/* init. a global orect for use during mkrect calls */
	gl_mkrect.o_link = NULL;

	/* break other window's rects with our current rect */
	everyobj(tree, ROOT, wh, mkrect, 0, 0, MAX_DEPTH);

	/* get an orect in this window's list */
	newr = get_orect();
	newr->o_link = NULL;
	w_getsize(WS_TRUE, wh, &newr->o_gr);
	pwin->w_rlist = newr;
}
