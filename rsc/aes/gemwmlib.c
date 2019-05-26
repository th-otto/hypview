/*		GEMWMLIB.C		4/23/84 - 07/11/85		Lee Lorenzen			*/
/*		merge High C vers. w. 2.2				8/24/87 		mdf 	*/
/*		fix wm_delete bug						10/8/87 		mdf 	*/

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
#include "gsxdefs.h"
#include "gem_rsc.h"
#include "debug.h"

#define N_PTRINTS (sizeof(void *) / sizeof(short))

/*
 *	defines
 */
#define NUM_MWIN NUM_WIN

#define XFULL	0
#define YFULL	gl_hbox
#define WFULL	gl_width
#define HFULL	(gl_height - gl_hbox)

#define DROP_SIZE 2						/* Windows have drop shadows */


_WORD gl_wtop;
OBJECT *gl_awind;

_WORD wtcolor[NUM_ELEM];					/* topped window object colors  */
_WORD wbcolor[NUM_ELEM];					/* background window object colors */

/*
 * current desktop background pattern.
 */
OBJECT *gl_newdesk;
/*
 * current object within gl_newdesk.
 */
_WORD gl_newroot;

static OBJECT W_TREE[NUM_MWIN];
static OBJECT W_ACTIVE[NUM_ELEM];

static TEDINFO gl_aname;
static TEDINFO gl_ainfo;
static _WORD wind_msg[8];
static _BOOL wasclr;


static _WORD const gl_watype[NUM_ELEM] =
{
	G_IBOX, 		/* W_BOX		*/
	G_BOX,			/* W_TITLE		*/
	G_BOXCHAR,		/* W_CLOSER 	*/
	G_BOXTEXT,		/* W_NAME		*/
	G_BOXCHAR,		/* W_FULLER 	*/
	G_BOXTEXT,		/* W_INFO		*/
	G_IBOX, 		/* W_DATA		*/
	G_IBOX, 		/* W_WORK		*/
	G_BOXCHAR,		/* W_SIZER		*/
	G_BOX,			/* W_VBAR		*/
	G_BOXCHAR,		/* W_UPARROW	*/
	G_BOXCHAR,		/* W_DNARROW	*/
	G_BOX,			/* W_VSLIDE 	*/
	G_BOX,			/* W_VELEV		*/
	G_BOX,			/* W_HBAR		*/
	G_BOXCHAR,		/* W_LFARROW	*/
	G_BOXCHAR,		/* W_RTARROW	*/
	G_BOX,			/* W_HSLIDE 	*/
	G_BOX,			/* W_HELEV		*/
#if AESVERSION >= 0x330
	G_BOX,			/* W_MENUBAR	*/
#endif
};

static int32_t const gl_waspec[NUM_ELEM] =
{
	0x00011101L,	/* W_BOX		*/
	0x00011101L,	/* W_TITLE		*/
	0x05011101L,	/* W_CLOSER 	*/
	0x0L,			/* W_NAME		*/
	0x07011101L,	/* W_FULLER 	*/
	0x0L,			/* W_INFO		*/
	0x00001101L,	/* W_DATA		*/
	0x00001101L,	/* W_WORK		*/
	0x06011101L,	/* W_SIZER		*/
	0x00011101L,	/* W_VBAR		*/
	0x01011101L,	/* W_UPARROW	*/
	0x02011101L,	/* W_DNARROW	*/
	0x00011111L,	/* W_VSLIDE 	*/
	0x00011101L,	/* W_VELEV		*/
	0x00011101L,	/* W_HBAR		*/
	0x04011101L,	/* W_LFARROW	*/
	0x03011101L,	/* W_RTARROW	*/
	0x00011111L,	/* W_HSLIDE 	*/
	0x00011101L 	/* W_HELEV		*/
#if AESVERSION >= 0x330
	0x00011101L 	/* W_MENUBAR	*/
#endif
};

static TEDINFO const gl_asamp =
{
	NULL, NULL, NULL, IBM, MD_REPLACE, TE_LEFT, SYS_FG, 0x0, 1, 80, 80
};



static void w_nilit(_WORD num, OBJECT *olist)
{
	while (num--)
	{
		olist[num].ob_next = olist[num].ob_head = olist[num].ob_tail = NIL;
	}
}


/*
 *	Routine to add a child object to a parent object.  The child
 *	is added at the end of the parent's current sibling list.
 *	It is also initialized.
 */
static void w_obadd(OBJECT *olist, _WORD parent, _WORD child)
{
	_WORD lastkid;

	if (parent != NIL && child != NIL)
	{
		olist[child].ob_next = parent;

		lastkid = olist[parent].ob_tail;
		if (lastkid == NIL)
			olist[parent].ob_head = child;
		else
			olist[lastkid].ob_next = child;

		olist[parent].ob_tail = child;
	}
}


static void w_setup(AESPD *ppd, _WORD w_handle, _WORD kind)
{
	WINDOW *pwin;
	int i;
	
	pwin = srchwp(w_handle);
	pwin->w_owner = ppd;
	pwin->w_flags |= VF_INUSE;
	pwin->w_kind = kind;
	pwin->w_hslide = pwin->w_vslide = 0;	/* slider at left/top	*/
	pwin->w_hslsiz = pwin->w_vslsiz = -1;	/* use default size 	*/

	/* initialize window object colors */
	for (i = 0; i < NUM_ELEM; i++)
	{
		pwin->w_tcolor[i] = wtcolor[i];
		pwin->w_bcolor[i] = wbcolor[i];
	}
}


static GRECT *w_getxptr(_WORD which, _WORD w_handle)
{
	/* FIXME: Probably remove the GRECT typecasts in this function */
	WINDOW *pwin = srchwp(w_handle);

	switch (which)
	{
	case WS_CURR:
	case WS_TRUE:
		return (GRECT *)&W_TREE[w_handle].ob_x;
	case WS_PREV:
		return &pwin->w_prev;
	case WS_WORK:
		return &pwin->w_work;
	case WS_FULL:
		return &pwin->w_full;
	}

	return NULL;
}


/*
 * Get the size (x,y,w,h) of the window
 */
void w_getsize(_WORD which, _WORD w_handle, GRECT *pt)
{
	rc_copy(w_getxptr(which, w_handle), pt);
#if DROP_SIZE
	if (which == WS_TRUE && pt->g_w && pt->g_h)
	{
		pt->g_w += DROP_SIZE;
		pt->g_h += DROP_SIZE;
	}
#endif
}


static void w_setsize(_WORD which, _WORD w_handle, const GRECT *pt)
{
	rc_copy(pt, w_getxptr(which, w_handle));
}


/*
 * setcol() - set the color of an object.
 */
static void setcol(_WORD ndx, WINDOW *wp, _BOOL topped)
{
	_WORD color;
	OBJECT *obj;
	
	if (topped)
		color = wp->w_tcolor[ndx];
	else
		color = wp->w_bcolor[ndx];
	obj = &W_ACTIVE[ndx];
	if (obj->ob_type == G_BOXTEXT)
	{
		obj->ob_spec.tedinfo->te_color = color;
	} else
	{
		obj->ob_spec.index = (obj->ob_spec.index & 0xffff0000L) | ((int32_t)color & 0x0000ffffL);
	}
}


static void w_adjust(_WORD parent, _WORD obj, _WORD x, _WORD y, _WORD w, _WORD h)
{
	OBJECT *pobj;
	
	pobj = &W_ACTIVE[obj];
	pobj->ob_x = x;
	pobj->ob_y = y;
	pobj->ob_width = w;
	pobj->ob_height = h;

	pobj->ob_head = pobj->ob_tail = NIL;
	w_obadd(&W_ACTIVE[ROOT], parent, obj);
}


static void w_hvassign(_BOOL isvert, _WORD parent, _WORD obj, _WORD vx, _WORD vy, _WORD hx, _WORD hy, _WORD w, _WORD h)
{
	if (isvert)
		w_adjust(parent, obj, vx, vy, gl_wbox, h);
	else
		w_adjust(parent, obj, hx, hy, w, gl_hbox);
}


/*
 *	Walk the list and draw the parts of the window tree owned by this window.
 */
static void do_walk(_WORD wh, OBJECT *tree, _WORD obj, _WORD depth, const GRECT *pc)
{
	ORECT *po;
	GRECT t;
	GRECT c;
	
	if (wh == NIL)
		return;

	/* clip to screen */
	if (pc)
	{
		rc_copy(pc, &c);
		rc_intersect(&gl_rfull, &c);
		pc = &c;
	} else
	{
		pc = &gl_rfull;
	}
	
	/* walk owner rectangle list */
	for (po = srchwp(wh)->w_rlist; po; po = po->o_link)
	{
		rc_copy(&po->o_gr, &t);
		/* intersect owner rectangle with clip rectangles */
		if (rc_intersect(pc, &t))
		{
			if (wh == gl_wtop)
				w_getsize(WS_TRUE, wh, &t); /* hmm.... */
			/* set clip and draw */
			gsx_sclip(&t);
			ob_draw(tree, obj, depth);
		}
	}
}


/*
 *	Draw the desktop background pattern underneath the current set of windows.
 */
void w_drawdesk(const GRECT *dirty)
{
	OBJECT *tree;
	_WORD depth;
	_WORD root;

	if (gl_newdesk)
	{
		tree = gl_newdesk;
		depth = MAX_DEPTH;
		root = gl_newroot;
	} else
	{
		tree = W_TREE;
		depth = 0;
		root = ROOT;
	}
	/* account for drop shadow: BUGFIX in 2.1 */
	{
#if DROP_SIZE
		GRECT pt;
		
		rc_copy(dirty, &pt);
		pt.g_w += DROP_SIZE;
		pt.g_h += DROP_SIZE;
		dirty = &pt;
#endif
		do_walk(DESK, tree, root, depth, dirty);
	}
}


static void w_cpwalk(_WORD wh, _WORD obj, _WORD depth, _WORD usetrue)
{
	GRECT c;

	/* start with window's true size as clip */
	if (usetrue)
	{
		w_getsize(WS_TRUE, wh, &c);
	} else
	{
		/* use global clip */
		gsx_gclip(&c);
		/* add in drop shadow */
#if DROP_SIZE
		c.g_w += DROP_SIZE;
		c.g_h += DROP_SIZE;
#endif
	}

	w_bldactive(wh);
	do_walk(wh, W_ACTIVE, obj, depth, &c);
}


static void w_strchg(_WORD w_handle, _WORD obj, char *pstring)
{
	WINDOW *pwin;
	
	pwin = srchwp(w_handle);
	if (obj == W_NAME)
		gl_aname.te_ptext = pwin->w_pname = pstring;
	else
		gl_ainfo.te_ptext = pwin->w_pinfo = pstring;

	w_cpwalk(w_handle, obj, MAX_DEPTH, TRUE);
}


static void w_barcalc(_BOOL isvert, _WORD space, _WORD sl_value, _WORD sl_size, _WORD min_sld, GRECT *ptv, GRECT *pth)
{
	if (sl_size == -1)
		sl_size = min_sld;
	else
		sl_size = max(min_sld, mul_div(sl_size, space, 1000));

	sl_value = mul_div(space - sl_size, sl_value, 1000);

	if (isvert)
		r_set(ptv, 0, sl_value, gl_wbox, sl_size);
	else
		r_set(pth, sl_value, 0, sl_size, gl_hbox);
}


static void w_bldbar(_UWORD kind, _BOOL istop, _WORD w_bar, WINDOW *wp, _WORD x, _WORD y, _WORD w, _WORD h)
{
	_WORD sl_value, sl_size;
	_BOOL isvert;
	_WORD obj;
	_UWORD upcmp, dncmp, slcmp;
	_WORD w_up;
	_WORD w_dn, w_slide, space, min_sld;
	_WORD w_elev;
	
	isvert = w_bar == W_VBAR;
	if (isvert)
	{
		sl_value = wp->w_vslide;
		sl_size = wp->w_vslsiz;
		upcmp = UPARROW;
		dncmp = DNARROW;
		slcmp = VSLIDE;
		w_up = W_UPARROW;
		w_dn = W_DNARROW;
		w_slide = W_VSLIDE;
		min_sld = gl_hbox;
		w_elev = W_VELEV;
	} else
	{
		sl_value = wp->w_hslide;
		sl_size = wp->w_hslsiz;
		upcmp = LFARROW;
		dncmp = RTARROW;
		slcmp = HSLIDE;
		w_up = W_LFARROW;
		w_dn = W_RTARROW;
		w_slide = W_HSLIDE;
		min_sld = gl_wbox;
		w_elev = W_HELEV;
	}
	
	setcol(w_bar, wp, istop);
	setcol(w_up, wp, istop);
	setcol(w_dn, wp, istop);
	setcol(w_slide, wp, istop);
	setcol(w_elev, wp, istop);
	w_hvassign(isvert, W_DATA, w_bar, x, y, x, y, w, h);
	x = y = 0;
	if (istop)
	{
		if (kind & upcmp)
		{
			w_adjust(w_bar, w_up, x, y, gl_wbox, gl_hbox);
			if (isvert)
			{
				y += (gl_hbox - 1);
				h -= (gl_hbox - 1);
			} else
			{
				x += (gl_wbox - 1);
				w -= (gl_wbox - 1);
			}
		}
		if (kind & dncmp)
		{
			w -= (gl_wbox - 1);
			h -= (gl_hbox - 1);
			w_hvassign(isvert, w_bar, w_dn, x, y + h - 1, x + w - 1, y, gl_wbox, gl_hbox);
		}
		if (kind & slcmp)
		{
			w_hvassign(isvert, w_bar, w_slide, x, y, x, y, w, h);
			space = isvert ? h : w;

			w_barcalc(isvert, space, sl_value, sl_size, min_sld, (GRECT *)&W_ACTIVE[W_VELEV].ob_x, (GRECT *)&W_ACTIVE[W_HELEV].ob_x);

			obj = isvert ? W_VELEV : W_HELEV;
			W_ACTIVE[obj].ob_head = W_ACTIVE[obj].ob_tail = NIL;
			w_obadd(&W_ACTIVE[ROOT], w_slide, obj);
		}
	}
}


static _WORD w_top(void)
{
	return (gl_wtop != NIL) ? gl_wtop : DESK;
}


void w_setactive(void)
{
	GRECT d;
	_WORD wh;
	AESPD *ppd;

	wh = w_top();
	w_getsize(WS_WORK, wh, &d);
	ppd = srchwp(wh)->w_owner;

	/* BUGFIX 2.1: don't chg own if null */
	if (ppd != NULL)
		ct_chgown(ppd, &d);
}


void w_bldactive(_WORD w_handle)
{
	_BOOL istop;
	_WORD kind;
	_WORD havevbar;
	_WORD havehbar;
	GRECT t;
	_WORD tempw;
	_WORD offx, offy;
	GRECT *pt;
	WINDOW *pw;

	if (w_handle == NIL)
		return;

	pw = srchwp(w_handle);
	pt = &t;

	/* set if it is on top */
	istop = gl_wtop == w_handle;
	/* get the kind of window */
	kind = pw->w_kind;
	w_nilit(NUM_ELEM, W_ACTIVE);

	/* start adding pieces & adjusting sizes */
	gl_aname.te_ptext = pw->w_pname;
	gl_ainfo.te_ptext = pw->w_pinfo;
	w_getsize(WS_CURR, w_handle, pt);
	W_ACTIVE[W_BOX].ob_x = t.g_x;
	W_ACTIVE[W_BOX].ob_y = t.g_y;
	W_ACTIVE[W_BOX].ob_width = t.g_w;
	W_ACTIVE[W_BOX].ob_height = t.g_h;
	offx = pt->g_x;
	offy = pt->g_y;

	/* do title area */
	setcol(W_BOX, pw, istop);
	pt->g_x = pt->g_y = 0;
	if (kind & (NAME | CLOSER | FULLER))
	{
		setcol(W_TITLE, pw, istop);
		w_adjust(W_BOX, W_TITLE, pt->g_x, pt->g_y, pt->g_w, gl_hbox);
		tempw = pt->g_w;
		if ((kind & CLOSER) && istop)
		{
			setcol(W_CLOSER, pw, istop);
			w_adjust(W_TITLE, W_CLOSER, pt->g_x, pt->g_y, gl_wbox, gl_hbox);
			pt->g_x += gl_wbox;
			tempw -= gl_wbox;
		}
		if ((kind & FULLER) && istop)
		{
			tempw -= gl_wbox;
			setcol(W_FULLER, pw, istop);
			w_adjust(W_TITLE, W_FULLER, pt->g_x + tempw, pt->g_y, gl_wbox, gl_hbox);
		}
		if (kind & NAME)
		{
			setcol(W_NAME, pw, istop);
			w_adjust(W_TITLE, W_NAME, pt->g_x, pt->g_y, tempw, gl_hbox);
#if 0
			W_ACTIVE[W_NAME].ob_state = istop ? OS_NORMAL : OS_DISABLED;

			/* comment out following line to enable pattern in window title */
			gl_aname.te_color = (istop && !issub) ? WTS_FG : WTN_FG;
#endif
		}
		pt->g_x = 0;
		pt->g_y += (gl_hbox - 1);
		pt->g_h -= (gl_hbox - 1);
	}

	/* do info area */
	if (kind & INFO)
	{
		setcol(W_INFO, pw, istop);
		w_adjust(W_BOX, W_INFO, pt->g_x, pt->g_y, pt->g_w, gl_hbox);
		pt->g_y += (gl_hbox - 1);
		pt->g_h -= (gl_hbox - 1);
	}

	/* do data area */
	w_adjust(W_BOX, W_DATA, pt->g_x, pt->g_y, pt->g_w, pt->g_h);

	/* do work area */
	pt->g_x++;
	pt->g_y++;
	pt->g_w -= 2;
	pt->g_h -= 2;
	havevbar = kind & (UPARROW | DNARROW | VSLIDE | SIZER);
	havehbar = kind & (LFARROW | RTARROW | HSLIDE | SIZER);
	if (havevbar)
		pt->g_w -= (gl_wbox - 1);
	if (havehbar)
		pt->g_h -= (gl_hbox - 1);

	pt->g_x += offx;
	pt->g_y += offy;

	pt->g_x = pt->g_y = 1;
	w_adjust(W_DATA, W_WORK, pt->g_x, pt->g_y, pt->g_w, pt->g_h);

	/* do vertical bar area */
	if (havevbar)
	{
		pt->g_x += pt->g_w;
		w_bldbar(kind, istop, W_VBAR, pw, pt->g_x, 0, pt->g_w + 2, pt->g_h + 2);
	}

	/* do horizontal bar area */
	if (havehbar)
	{
		pt->g_y += pt->g_h;
		w_bldbar(kind, istop, W_HBAR, pw, 0, pt->g_y, pt->g_w + 2, pt->g_h + 2);
	}

	/* do sizer area */
	if (havevbar && havehbar)
	{
		setcol(W_SIZER, pw, istop);
		w_adjust(W_DATA, W_SIZER, pt->g_x, pt->g_y, gl_wbox, gl_hbox);
		W_ACTIVE[W_SIZER].ob_spec.index &= 0xffffL;
		W_ACTIVE[W_SIZER].ob_spec.index |= (istop && (kind & SIZER)) ? 0x06010000L: 0x00010000L;
	}
}


/* 
 * ap_sendmsg() - send message to current process
 */
void ap_sendmsg(_WORD *ap_msg, _WORD type, AESPD *towhom, _WORD w3, _WORD w4, _WORD w5, _WORD w6, _WORD w7)
{
	ap_msg[0] = type;
	ap_msg[1] = rlr->p_pid;
	ap_msg[2] = 0;
	ap_msg[3] = w3;
	ap_msg[4] = w4;
	ap_msg[5] = w5;
	ap_msg[6] = w6;
	ap_msg[7] = w7;
	ap_rdwr(AQWRT, towhom->p_pid, 16, ap_msg);
}


/*
 *	Walk down ORECT list and accumulate the union of all the owner rectangles.
 */
static _BOOL w_union(ORECT *po, GRECT *pt)
{
	if (!po)
		return FALSE;

	rc_copy(&po->o_gr, pt);

	po = po->o_link;
	while (po)
	{
		rc_union(&po->o_gr, pt);
		po = po->o_link;
	}

	return TRUE;
}


static void w_redraw(_WORD w_handle, const GRECT *dirty)
{
	GRECT t, d;
	GRECT *pt;
	WINDOW *wp;
	
	wp = srchwp(w_handle);
	pt = &t;
	
	/* make sure work rect and word rect intersect */
	rc_copy(dirty, pt);
	w_getsize(WS_WORK, w_handle, &d);
	if (rc_intersect(pt, &d))
	{
		/* make sure window owns a rectangle */
		if (w_union(wp->w_rlist, &d))
		{
			/* intersect redraw rect with union of owner rects */
			if (rc_intersect(&d, pt))
				ap_sendmsg(wind_msg, WM_REDRAW, wp->w_owner, w_handle, pt->g_x, pt->g_y, pt->g_w, pt->g_h);
		}
	}
}


/*
 *	Draw windows from top to bottom.  If top is 0, then start at the topmost
 *	window.  If bottom is 0, then start at the bottomost window.  For the
 *	first window drawn, just do the insides, since DRAW_CHANGE has already
 *	drawn the outside borders.
 */
void w_update(_WORD bottom, const GRECT *pt, _WORD top, _BOOL moved)
{
	_WORD i, ni;
	_BOOL done;
	GRECT c;
	
	/* limit to screen */
	rc_copy(pt, &c);
	rc_intersect(&gl_rfull, &c);
	pt = &c;
	gsx_moff();

	/* update windows from top to bottom */
	if (bottom == DESK)
		bottom = W_TREE[ROOT].ob_head;

	/* if there are windows */
	if (bottom != NIL)
	{
		/* start at the top */
		if (top == DESK)
			top = W_TREE[ROOT].ob_tail;
		/* draw windows from top to bottom */
		do
		{
			if (!(moved && top == gl_wtop))
			{
				/* set clip and draw a window's border */
				gsx_sclip(pt);
				/* let appl. draw inside */
				w_cpwalk(top, 0, MAX_DEPTH, FALSE);
				w_redraw(top, pt);
			}
			
			/* scan to find prev */
			i = bottom;
			done = i == top;
			while (i != top)
			{
				ni = W_TREE[i].ob_next;
				if (ni == top)
					top = i;
				else
					i = ni;
			}
		} while (!done);
	}

	gsx_mon();
}


#if 0
static void w_setmen(_WORD pid)
{
	_WORD npid;

	npid = menu_tree[pid] ? pid : 0;
	if (gl_mntree != menu_tree[npid])
		mn_bar(menu_tree[npid], TRUE, npid);

	npid = desk_tree[pid] ? pid : 0;
	if (gl_newdesk != desk_tree[npid])
	{
		gl_newdesk = desk_tree[npid];
		gl_newroot = desk_root[npid];
		w_drawdesk(&gl_rscreen);
	}
}


/*
 *	Routine to draw menu of top most window as the current menu bar.
 */
static void w_menufix(void)
{
	_WORD pid;

	pid = D.w_win[w_top()].w_owner->p_pid;
	w_setmen(pid);
}
#endif


/*
 *	Draw the tree of windows given a major change in some window.  It
 *	may have been sized, moved, fulled, topped, or closed.  An attempt
 *	should be made to minimize the amount of redrawing of other windows
 *	that has to occur.  W_REDRAW() will actually issue window redraw
 *	requests based on the rectangle that needs to be cleaned up.
 */
static void draw_change(_WORD w_handle, const GRECT *pt)
{
	/* set new sizes */
	w_setsize(WS_CURR, w_handle, pt);
}


/*
 *	Walk down ORECT list looking for the next rect that still has
 *	size when clipped with the passed in clip rectangle.
 */
static void w_owns(WINDOW *pwin, ORECT *po, GRECT *pt, GRECT *poutwds)
{
	while (po)
	{
		rc_copy(&po->o_gr, poutwds);
		pwin->w_rnext = po = po->o_link;
		if (rc_intersect(pt, poutwds) /* &&
			rc_intersect(&gl_rfull, poutwds) */)
			return;
	}

	poutwds->g_w = poutwds->g_h = 0;
}


/*
 *	Start the window manager up by initializing internal variables
 */
_BOOL wm_start(void)
{
	_WORD i;
	ORECT *po;
	OBJECT *tree;
	
	/* init owner rectangles */
	or_start();

	/* init window extent objects */
	memset(W_TREE, 0, sizeof(W_TREE));
	w_nilit(NUM_MWIN, &W_TREE[ROOT]);

	for (i = 0; i < NUM_MWIN; i++)
	{
		D.w_win[i].w_flags = 0x0;
		D.w_win[i].w_rlist = NULL;
		W_TREE[i].ob_type = G_IBOX;
	}
	W_TREE[ROOT].ob_type = G_BOX;
	tree = aes_rsc_tree[DESKTOP];
	W_TREE[ROOT].ob_spec = tree[ROOT].ob_spec;

	/* init window element objects */
	memset(W_ACTIVE, 0, sizeof(W_ACTIVE));
	w_nilit(NUM_ELEM, W_ACTIVE);
	for (i = 0; i < NUM_ELEM; i++)
	{
		W_ACTIVE[i].ob_type = gl_watype[i];
		W_ACTIVE[i].ob_spec.index = gl_waspec[i];
	}
	W_ACTIVE[ROOT].ob_state = OS_SHADOWED;

	/* init rectangle list */
	D.w_win[0].w_rlist = po = get_orect();
	po->o_link = NULL;
	r_set(&po->o_gr, XFULL, YFULL, WFULL, HFULL);
	w_setup(rlr, DESK, OF_NONE);
	w_setsize(WS_CURR, DESK, &gl_rscreen);
	w_setsize(WS_PREV, DESK, &gl_rscreen);
	w_setsize(WS_FULL, DESK, &gl_rfull);
	w_setsize(WS_WORK, DESK, &gl_rfull);
	
	/* init global variables */
	gl_wtop = NIL;
	gl_awind = W_ACTIVE;
	gl_newdesk = NULL;

	/* init tedinfo parts of title and info lines */
	gl_aname = gl_asamp;
	gl_ainfo = gl_asamp;
	gl_aname.te_just = TE_CNTR;
	W_ACTIVE[W_NAME].ob_spec.tedinfo = &gl_aname;
	W_ACTIVE[W_INFO].ob_spec.tedinfo = &gl_ainfo;

	return TRUE;
}


/*
 * wm_init() -	initializes window colors, then start up the window
 *		manager.
 *	     -	this is called by geminit; only at boot time; so that
 *		window color defaults don't get munched when apps are
 *		launched; etc.
 */
void wm_init(void)
{
	_WORD i;

	for (i = 0; i < NUM_ELEM; i++)
	{
		wtcolor[i] = wbcolor[i] = 0x1101;
	}
	wtcolor[W_NAME] |= 0xa0;
	wtcolor[W_VSLIDE] |= 0x10;
	wtcolor[W_HSLIDE] |= 0x10;
	wbcolor[W_VSLIDE] |= 0x10;
	wbcolor[W_HSLIDE] |= 0x10;

	wm_start();
}


/*
 * AES #100 - wind_create - Initializes a new window 
 *
 * wm_create() - allocates the application's full-size window and 
 *		 returns the window's handle.
 *	       - returns FAILURE (-1) if no handle is available or
 *		 if an error occurred.
 */
_WORD wm_create(_UWORD kind, const GRECT *pt)
{
	_WORD i;

	for (i = 0; i < NUM_WIN && (D.w_win[i].w_flags & VF_INUSE); i++)
		;
	if (i < NUM_WIN)
	{
		w_setup(rlr, i, kind);
		w_setsize(WS_CURR, i, &gl_rzero);
		w_setsize(WS_PREV, i, &gl_rzero);
		w_setsize(WS_FULL, i, pt);
		return i;
	}

	return -1;
}


/*
 *	Opens or closes a window.
 */
static void wm_opcl(_WORD wh, const GRECT *pt, _BOOL isadd)
{
	GRECT t;

	rc_copy(pt, &t);
	wm_update(BEG_UPDATE);
	if (isadd)
	{
		D.w_win[wh].w_flags |= VF_INTREE;
		w_obadd(&W_TREE[ROOT], ROOT, wh);
	} else
	{
		ob_delete(W_TREE, wh);
		D.w_win[wh].w_flags &= ~VF_INTREE;
	}
	draw_change(wh, &t);
	if (isadd)
		w_setsize(WS_PREV, wh, pt);
	wm_update(END_UPDATE);
}


/*
 * AES #101 - wind_open - Open window
 *
 * wm_open() - opens a window in its given size and location.
 *	     - returns FALSE (0) if given handle is invalid,
 *	       or if window has already been opened.
 *	     - returns TRUE (1) if everything is fine.
 */
/* BUG: does not return anything */
void wm_open(_WORD w_handle, const GRECT *pt)
{
	wm_opcl(w_handle, pt, TRUE);
}


/*
 * AES #102 - wind_close - Close window
 *
 * wm_close() - closes an opened window
 *	      - returns FALSE (0) if given handle is invalid,
 *	        or if window has already been closed.
 *	      - returns TRUE (1) if everything is fine.
 *
 */
/* BUG: does not return anything */
void wm_close(_WORD w_handle)
{
	if (w_handle > 0)
		wm_opcl(w_handle, &gl_rzero, FALSE);
}


/*
 * AES #103 - wind_delete - Delete window
 *
 * wm_delete() - closes the window if it is not already closed,
 *		 and frees the window structure.
 *	       - returns FALSE (0) if given handle is invalid.
 *	       - returns TRUE (1) if everything is fine.
 *
 */
_WORD wm_delete(_WORD w_handle)
{
	newrect(W_TREE, w_handle, 0, 0);			/* give back recs. */
	w_setsize(WS_CURR, w_handle, &gl_rscreen);
	w_setsize(WS_PREV, w_handle, &gl_rscreen);
	w_setsize(WS_FULL, w_handle, &gl_rfull);
	w_setsize(WS_WORK, w_handle, &gl_rfull);
	D.w_win[w_handle].w_flags = 0;			/* &= ~VF_INUSE; */
	D.w_win[w_handle].w_owner = NULL;
	return TRUE;
}


/*
 * AES #105 - wind_get - Obtains various properties of a window.
 *
 * wm_get() - returns information of window in the given array
 *	    - returns FALSE (0) if given handle is invalid
 *	    - returns TRUE (1) if everything is fine
 *
 */
_WORD wm_get(_WORD w_handle, _WORD w_field, _WORD *poutwds, const _WORD *iw)
{
	_WORD which;
	GRECT t;
	ORECT *po;
	WINDOW *pwin;

	pwin = srchwp(w_handle);
	which = -1;

	switch (w_field)
	{
	/* Some functions may be allowed without a real window handle */
	case WF_BOTTOM:
	case WF_TOP:
	case WF_NEWDESK:
	case WF_SCREEN:
	case WF_WHEEL:
	case WF_OPTS:
	case WF_XAAES:
	case WF_DCOLOR:
	case WF_WINX:
	case WF_WINXCFG:
	case WF_DDELAY:
		break;
	default:
		if (pwin == NULL || (w_handle > DESK && !(pwin->w_flags & VF_INUSE)))
		{
			KDEBUG(("WARNING:wind_get for %-*.*s: Invalid window handle %d", AP_NAMELEN, AP_NAMELEN, rlr->p_name, w_handle));
			/* clear args, Prevents FIRST/NEXTXYWH loops with invalid handle from looping forever. */
			poutwds[1] = 0;
			poutwds[2] = 0;
			poutwds[3] = 0;
			poutwds[4] = 0;
	
			/* Invalid window handle, return error */
			return FALSE;
		}
		break;
	}
	switch (w_field)
	{
	case WF_WORKXYWH:
		which = WS_WORK;
		break;
	case WF_CURRXYWH:
		which = WS_CURR;
		break;
	case WF_PREVXYWH:
		which = WS_PREV;
		break;
	case WF_FULLXYWH:
		which = WS_FULL;
		break;
	case WF_HSLIDE:
		poutwds[0] = pwin->w_hslide;
		break;
	case WF_VSLIDE:
		poutwds[0] = pwin->w_vslide;
		break;
	case WF_HSLSIZE:
		poutwds[0] = pwin->w_hslsiz;
		break;
	case WF_VSLSIZE:
		poutwds[0] = pwin->w_vslsiz;
		break;
	case WF_TOP:
		poutwds[0] = w_top();
		break;
	case WF_FIRSTXYWH:
	case WF_NEXTXYWH:
		w_getsize(WS_WORK, w_handle, &t);
		po = w_field == WF_FIRSTXYWH ? pwin->w_rlist : pwin->w_rnext;
		/* FIXME: GRECT typecasting again */
		w_owns(pwin, po, &t, (GRECT *)poutwds);
		break;
	case WF_SCREEN:
		gsx_mret((void **)poutwds, (_LONG *)(poutwds + N_PTRINTS));
		break;
	case WF_NEWDESK:
		if (gl_newdesk)
		{
			*((OBJECT **)poutwds) = gl_newdesk;
			poutwds[N_PTRINTS] = gl_newroot;
		} else
		{
			*((OBJECT **)poutwds) = W_TREE;
			poutwds[N_PTRINTS] = ROOT;
		}
		break;
	case WF_COLOR:
		if (iw[0] < 0 || iw[0] >= NUM_ELEM)
		{
			KDEBUG(("WARNING:wind_get for %-*.*s: Invalid window element %d", AP_NAMELEN, AP_NAMELEN, rlr->p_name, iw[0]));
			return FALSE;
		}
		poutwds[1] = pwin->w_tcolor[iw[0]];
		poutwds[2] = pwin->w_bcolor[iw[0]];
		poutwds[3] = 0; /* 3d-flags not yet implemented */
		break;
	case WF_DCOLOR:
		if (iw[0] < 0 || iw[0] >= NUM_ELEM)
		{
			KDEBUG(("WARNING:wind_get for %-*.*s: Invalid window element %d", AP_NAMELEN, AP_NAMELEN, rlr->p_name, iw[0]));
			return FALSE;
		}
		poutwds[1] = wtcolor[iw[0]];
		poutwds[2] = wbcolor[iw[0]];
		poutwds[3] = 0; /* 3d-flags not yet implemented */
		break;
	default:
		KDEBUG(("WARNING:wind_get for %-*.*s: Invalid window field %d", AP_NAMELEN, AP_NAMELEN, rlr->p_name, w_field));
		return FALSE;
	}

	if (which != -1)
		w_getsize(which, w_handle, (GRECT *)poutwds);
	
	return TRUE;
}


#if 0
static _WORD wm_gsizes(_WORD w_field, _WORD *psl, _WORD *psz)
{
	if (w_field == WF_HSLSIZ || w_field == WF_HSLIDE)
	{
		*psl = W_ACTIVE[W_HELEV].ob_x;
		*psz = W_ACTIVE[W_HELEV].ob_width;
		return W_HBAR;
	}

	if (w_field == WF_VSLSIZ || w_field == WF_VSLIDE)
	{
		*psl = W_ACTIVE[W_VELEV].ob_y;
		*psz = W_ACTIVE[W_VELEV].ob_height;
		return W_VBAR;
	}

	return 0;
}
#endif


/*
 * AES #106 - wind_set - Alter various window attributes.
 *
 * wm_set() - changes information of a window
 *	    - returns FALSE (0) if given handle is invalid
 *	    - returns TRUE (1) if everything is fine
 *
 */
_WORD wm_set(_WORD w_handle, _WORD w_field, const _WORD *pinwds)
{
	_WORD which;
	_WORD osl, osz, nsz;
	_WORD blen, minw, gadget;
	GRECT t;
	WINDOW *pwin;

	which = -1;
	
	/* grab the window sync */
	wm_update(BEG_UPDATE);

	pwin = srchwp(w_handle);
	switch (w_field)
	{
	/* Some functions may be allowed without a real window handle */
	case WF_NEWDESK:
	case WF_SCREEN:
	case WF_WHEEL:
	case WF_OPTS:
	case WF_XAAES:
	case WF_DCOLOR:
	case WF_WINX:
	case WF_WINXCFG:
	case WF_DDELAY:
		break;
	default:
		if (pwin == NULL || !(pwin->w_flags & VF_INUSE))
		{
			KDEBUG(("WARNING:wind_set for %-*.*s: Invalid window handle %d", AP_NAMELEN, AP_NAMELEN, rlr->p_name, w_handle));
			/* Invalid window handle, return error */
			return FALSE;
		}
		break;
	}
	switch (w_field)
	{
	case WF_NAME:
		which = W_NAME;
		break;
	case WF_INFO:
		which = W_INFO;
		break;
	case WF_CURRXYWH:
		draw_change(w_handle, (const GRECT *)pinwds);
		break;
	case WF_TOP:
		if (w_handle != gl_wtop)
		{
			wasclr = !(pwin->w_flags & VF_BROKEN);
			ob_order(W_TREE, w_handle, NIL);
			w_getsize(WS_CURR, w_handle, &t);
			draw_change(w_handle, &t);
		}
		break;
	case WF_NEWDESK:
		/* BUG: should check that handle really was desktop */
		gl_newdesk = *(OBJECT *const *) pinwds;
		gl_newroot = pinwds[N_PTRINTS];
		break;
	case WF_HSLSIZE:
	case WF_VSLSIZE:
	case WF_HSLIDE:
	case WF_VSLIDE:
		nsz = max(-1, pinwds[0]);
		nsz = min(1000, nsz);
		if (w_field == WF_HSLSIZE || w_field == WF_HSLIDE)
		{
			if (w_field == WF_HSLSIZE)
			{
				osz = pwin->w_hslsiz = nsz;
				osl = pwin->w_hslide;
			} else
			{
				osl = pwin->w_hslide = nsz;
				osz = pwin->w_hslsiz;
			}
			blen = W_ACTIVE[W_HSLIDE].ob_width;
			gadget = W_HSLIDE;
			minw = gl_wbox;
		} else
		{
			if (w_field == WF_VSLSIZE)
			{
				osz = pwin->w_vslsiz = nsz;
				osl = pwin->w_vslide;
			} else
			{
				osl = pwin->w_vslide = nsz;
				osz = pwin->w_vslsiz;
			}
			blen = W_ACTIVE[W_VSLIDE].ob_height;
			gadget = W_VSLIDE;
			minw = gl_hbox;
		}
		if (w_handle == gl_wtop)
			w_cpwalk(w_handle, gadget, MAX_DEPTH, TRUE);
		UNUSED(osz);
		UNUSED(osl);
		UNUSED(blen);
		UNUSED(minw);
		break;
	case WF_COLOR:
		if (pinwds[0] < 0 || pinwds[0] >= NUM_ELEM)
		{
			KDEBUG(("WARNING:wind_set for %-*.*s: Invalid window element %d", AP_NAMELEN, AP_NAMELEN, rlr->p_name, pinwds[0]));
			return FALSE;
		}
		if (pinwds[1] != -1)
			pwin->w_tcolor[pinwds[0]] = pinwds[1];
		if (pinwds[2] != -1)
			pwin->w_bcolor[pinwds[0]] = pinwds[2];
		w_cpwalk(w_handle, pinwds[0], MAX_DEPTH, TRUE);
		break;

	case WF_DCOLOR:
		if (pinwds[0] < 0 || pinwds[0] >= NUM_ELEM)
		{
			KDEBUG(("WARNING:wind_set for %-*.*s: Invalid window element %d", AP_NAMELEN, AP_NAMELEN, rlr->p_name, pinwds[0]));
			return FALSE;
		}
		if (pinwds[1] != -1)
			wtcolor[pinwds[0]] = pinwds[1];
		if (pinwds[2] != -1)
			wbcolor[pinwds[0]] = pinwds[2];
		break;

	default:
		KDEBUG(("WARNING:wind_set for %-*.*s: Invalid window field %d", AP_NAMELEN, AP_NAMELEN, rlr->p_name, w_field));
		return FALSE;
	}

	if (which != -1)
		w_strchg(w_handle, which, *(char *const *)pinwds);

	/* give up the sync */
	wm_update(END_UPDATE);
	
	return TRUE;
}


/*
 * AES #106 - wind_find - Find the ID of a window at the given coordinates.
 *
 * wm_find() - Given an x and y location, will figure out which window the mouse is in
 *
 */
_WORD wm_find(_WORD x, _WORD y)
{
	return ob_find(W_TREE, ROOT, 2, x, y);
}


/*
 * AES #108 - wind_calc - Calculates the limits or the total space requirement of a window 
 *
 *	Given a width and height of a Work Area and the Kind of window desired,
 *	calculate the required window size including the Border Area.
 *	OR
 *	Given the width and height of a window including the Border Area and the
 *	Kind of window desired, calculate the result size of the window Work Area.
 */
_WORD wm_calc(_WORD wtype, _WORD kind, const GRECT *in, GRECT *out)
{
	_WORD tb, bb, lb, rb;

	tb = bb = rb = lb = 1;

	if (kind & (NAME | CLOSER | FULLER))
		tb += gl_hbox + -1;

	if (kind & INFO)
		tb += gl_hbox - 1;

	if (kind & (UPARROW | DNARROW | VSLIDE | SIZER))
		rb += gl_wbox + -1;

	if (kind & (LFARROW | RTARROW | HSLIDE | SIZER))
		bb += gl_hbox + -1;

	/* negate values to calc Border Area */
	if (wtype == WC_BORDER)
	{
		lb = -lb;
		tb = -tb;
		rb = -rb;
		bb = -bb;
	}

	out->g_x = in->g_x + lb;
	out->g_y = in->g_y + tb;
	out->g_w = in->g_w - lb - rb;
	out->g_h = in->g_h - tb - bb;
	return TRUE;
}


/*
 * AES #109 - wind_new - Close all windows.
 *
 * wm_new() - Delete all the window structures and clean 
 *	      up the window update semaphore.  This 
 *	      routine is very critical, so don't call 
 *	      it when you are not sure.  You must call 
 *	      this guy right after you return from the 
 *	      child process.	Only at this moment the 
 *	      system can be able to recover itself.
 */
_WORD wm_new(void)
{
	return TRUE;
}
