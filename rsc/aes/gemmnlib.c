/*		GEMMNLIB.C		04/26/84 - 08/14/86 	Lowell Webster			*/
/*		merge High C vers. w. 2.2				8/21/87 		mdf 	*/
/*		fix mn_bar -- bar too wide				11/19/87		mdf 	*/

/*
 *		Copyright 1999, Caldera Thin Clients, Inc.
 *				  2002-2016 The EmuTOS development team
 *
 *		This software is licenced under the GNU Public License.
 *		Please see LICENSE.TXT for further information.
 *
 *				   Historical Copyright
 *		-------------------------------------------------------------
 *		GEM Application Environment Services			  Version 3.0
 *		Serial No.	XXXX-0000-654321			  All Rights Reserved
 *		Copyright (C) 1987						Digital Research Inc.
 *		-------------------------------------------------------------
 */

#include "aes.h"
#include "gempd.h"


#define MTH 1                                   /* menu thickness       */

#define THESCREEN 0
#define THEBAR 1
#define THEACTIVE 2
#define THEMENUS (tree[THESCREEN].ob_tail)

/*** STATE DEFINITIONS FOR menu_state ***********************************/

#define INBAR   1       /* mouse position       outside menu bar & mo dn */
                        /* multi wait           mo up | in menu bar     */
                        /* moves                -> 5  ,  ->2            */

#define OUTTITLE 2      /* mouse position       over title && mo dn     */
                        /* multiwait            mo up | out title rect  */
                        /* moves                -> 5  , ->1 ->2  ->3    */

#define OUTITEM 3       /* mouse position       over item && mo dn      */
                        /* multi wait           mo up | out item rect   */
                        /* moves                -> 5  , ->1 ->2 ->3 ->4 */

#define INBARECT 4      /* mouse position       out menu rect && bar && mo dn*/
                        /* multi wait   mo up | in menu rect | in menu bar */
                        /* moves        -> 5  , -> 3         , -> 2     */


OBJECT *gl_mntree;
AESPD *gl_mnppd;
GRECT gl_rmnactv;
char *desk_acc[MAX_ACCS];
AESPD *desk_ppd[MAX_ACCS];
_WORD gl_dacnt;
_WORD gl_dabase;
_WORD gl_dabox;
_WORD gl_mnclick;


/*
 *	Change a mouse-wait rectangle based on an object's size.
 */
static void rect_change(OBJECT *tree, MOBLK *prmob, _WORD iob, _WORD x)
{
	ob_actxywh(tree, iob, &prmob->m_gr);
	prmob->m_out = x;
}


/*
 * AES #31 - menu_icheck -  Set or clear check mark (tick) in menu options.
 * AES #32 - menu_ienable - Enable or disable a menu entry.
 * AES #33 - menu_tnormal - Displays a menu title in inverse video or normal.
 *
 *	Routine to change the state of a particular object.  The
 *	change in state will not occur if the object is disabled
 *	and the chkdisabled parameter is set.  The object will
 *	be drawn with its new state only if the dodraw parameter
 *	is set.
 */
_UWORD do_chg(OBJECT *tree, _WORD iitem, _UWORD chgvalue, _WORD dochg, _WORD dodraw, _WORD chkdisabled)
{
	_UWORD curr_state;

	curr_state = tree[iitem].ob_state;
	if (chkdisabled && (curr_state & OS_DISABLED))
		return FALSE;

	if (dochg)
		curr_state |= chgvalue;
	else
		curr_state &= ~chgvalue;

	if (dodraw)
		gsx_sclip(&gl_rzero);

	ob_change(tree, iitem, curr_state, dodraw);
	return TRUE;
}


/*
 *	Routine to set and reset values of certain items if they
 *	are not the current item
 */
static _WORD menu_set(OBJECT *tree, _WORD last_item, _WORD cur_item, _WORD setit)
{
	if (last_item != NIL && last_item != cur_item)
	{
		return do_chg(tree, last_item, OS_SELECTED, setit, TRUE, TRUE);
	}
	return FALSE;
}


/*
 *	Routine to save or restore the portion of the screen underneath
 *	a menu tree.  This involves BLTing out and back
 *	the data that was underneath the menu before it was pulled
 *	down.
 */
static void menu_sr(_WORD saveit, OBJECT *tree, _WORD imenu)
{
	GRECT t;

	gsx_sclip(&gl_rzero);
	ob_actxywh(tree, imenu, &t);
	t.g_x -= MTH;
	t.g_w += 2 * MTH;
	t.g_h += 2 * MTH;
	if (saveit)
		bb_save(&t);
	else
		bb_restore(&t);
}


static _WORD menu_sub(OBJECT *tree, _WORD ititle)
{
	_WORD themenus, imenu;
	_WORD i;

	themenus = tree[THESCREEN].ob_tail;

	/* correlate title # to menu subtree # */
	imenu = tree[themenus].ob_head;
	for (i = ititle - THEACTIVE; i > 1; i--)
	{
		imenu = tree[imenu].ob_next;
	}

	return imenu;
}


/*
 *	Routine to pull a menu down.  This involves saving the data
 *	underneath the menu and drawing in the proper menu sub-tree.
 */
static _WORD menu_down(OBJECT *tree, _WORD ititle)
{
	_WORD imenu;

	/* correlate title # to menu subtree #  */
	imenu = menu_sub(tree, ititle);

	/* draw title selected */
	if (do_chg(tree, ititle, OS_SELECTED, TRUE, TRUE, TRUE))
	{
		/* save area underneath the menu */
		menu_sr(TRUE, tree, imenu);
		/* draw all items in menu */
		ob_draw(tree, imenu, MAX_DEPTH);
	}

	return imenu;
}


_WORD mn_do(_WORD *ptitle, _WORD *pitem)
{
	OBJECT *tree;
	intptr_t buparm;
	_WORD mnu_flags, done;
	_WORD cur_menu, cur_item, last_menu, last_item;
	_WORD cur_title, flag, last_title;
	_UWORD ev_which;
	MOBLK p1mor, p2mor;
	_WORD menu_state, rect;
	_WORD lrets[6];
	_WORD curstate;

	tree = gl_mntree;
	/*
	 * initially wait to go into the active part of the bar or the
	 * button state to change or out of the bar when nothing is down
	 */
	menu_state = INBAR;

	done = FALSE;
	buparm = 0x00010101L;
	cur_title = cur_menu = cur_item = NIL;

	ctlmouse(TRUE);

	rect = 0; /* quiet compiler */
	while (!done)
	{
		mnu_flags = MU_BUTTON | MU_M1;
		flag = TRUE;
		switch (menu_state)
		{
		case INBAR:
			rect = THEACTIVE;
			flag = FALSE;
			mnu_flags |= MU_M2;
			rect_change(tree, &p2mor, THEBAR, TRUE);
			break;
		case OUTTITLE:
			rect = cur_title;
			break;
		case INBARECT:
			rect = THEACTIVE;
			flag = FALSE;
			mnu_flags |= MU_M2;
			rect_change(tree, &p2mor, cur_menu, FALSE);
			break;
		case OUTITEM:
			rect = cur_item;
			buparm = (gsx_button() & 0x0001) ? combine_cms(1, 1, 0) : combine_cms(1, 1, 1);
			break;
		}
		rect_change(tree, &p1mor, rect, flag);

		ev_which = ev_multi(mnu_flags, &p1mor, &p2mor, 0L, buparm, NULL, lrets);

		if (ev_which & MU_BUTTON)
		{
			if (menu_state != OUTTITLE && ((buparm & 1) || gl_mnclick != 0))
				done = TRUE;
			else
				buparm ^= 1;
		}

		/* if not done then do menus */
		if (!done)
		{
			/* save old values */
			last_title = cur_title;
			last_item = cur_item;
			last_menu = cur_menu;
			/* see if over the bar */
			cur_title = ob_find(tree, THEACTIVE, 1, lrets[0], lrets[1]);
			curstate = tree[cur_title].ob_state;
			if (cur_title != NIL && !(curstate & OS_DISABLED))
			{
				menu_state = OUTTITLE;
				if (gl_mnclick == 0 || lrets[5] == 1)
					cur_item = NIL;
				else
					cur_title = last_title;
			} else
			{
				cur_title = last_title;
				if (last_menu != NIL)
				{
					cur_item = ob_find(tree, last_menu, 1, lrets[0], lrets[1]);
					menu_state = cur_item != NIL ? OUTITEM : INBARECT;
				} else
				{
					menu_state = INBAR;
					if (!(curstate & OS_DISABLED))
						done = TRUE;
				}
			}
			/* clean up old state */
			menu_set(tree, last_item, cur_item, FALSE);
			if (menu_set(tree, last_title, cur_title, FALSE))
				menu_sr(FALSE, tree, last_menu);
			/* set up new state */
			if (menu_set(tree, cur_title, last_title, TRUE))
				cur_menu = menu_down(tree, cur_title);
			menu_set(tree, cur_item, last_item, TRUE);
		}
	}
	
	/* decide what should be cleaned up and returned */
	flag = FALSE;
	if (cur_title != NIL)
	{
		menu_sr(FALSE, tree, cur_menu);
		if ((cur_item != NIL) && (do_chg(tree, cur_item, OS_SELECTED, FALSE, FALSE, TRUE)))
		{
			/* only return TRUE when item is enabled and is not NIL */
			*ptitle = cur_title;
			*pitem = cur_item;
			flag = TRUE;
		} else
		{
			do_chg(tree, cur_title, OS_SELECTED, FALSE, TRUE, TRUE);
		}
	}

	ctlmouse(FALSE);

	return flag;
}


/*
 * AES #30 - menu_bar - Display, delete or install a menu bar.
 *
 *	Routine to display the menu bar.  Clipping is turned completely
 *	off so that this operation will be as fast as possible.  The
 *	global variable gl_mntree is also set or reset.
 */
void mn_bar(OBJECT *tree, _WORD showit, _WORD pid)
{
	_WORD i, ob, h, cnt, themenus;
    AESPD *p;

    p = fpdnm(NULL, pid);

	if (showit)
	{
		gl_mntree = tree;
		ob_actxywh(tree, THEACTIVE, &gl_rmnactv);

		/* change the waiting rectangle */

		ch_wrect(&gl_ctwait.m_gr, &gl_rmnactv);

		rc_copy(&gl_rmnactv, &gl_ctwait.m_gr);
		gl_mnppd = p;
		themenus = tree[THESCREEN].ob_tail;
		gl_dabox = tree[themenus].ob_head;
		tree[gl_dabox].ob_head = NIL;
		tree[gl_dabox].ob_tail = NIL;
		h = 0;
		i = 1;
		if (gl_dacnt)
		{
			/* add disabled line and each desk acc */
			cnt = 2 + gl_dacnt;
			gl_dabase = gl_dabox + 3;
		} else
		{
			cnt = 1;
		}
		
		while (i <= cnt)
		{
			ob = gl_dabox + i;
			ob_add(tree, gl_dabox, ob);
			/* fixup each desk accessory line */
			if (i > 2)
				tree[ob].ob_spec.free_string = desk_acc[i - 3];
			h += gl_hchar;
			i++;
		}
		tree[gl_dabox].ob_height = h;
		gsx_sclip(&gl_rzero);
		ob_draw(tree, THEBAR, MAX_DEPTH);
		gsx_attr(FALSE, MD_REPLACE, G_BLACK);	/* not xor mode! */
		gsx_cline(0, gl_hbox - 1, gl_width - 1, gl_hbox - 1);
	} else
	{
		gl_mntree = NULL;
		/* change the waiting rect */
		ch_wrect(&gl_ctwait.m_gr, &gl_rmenu);

		rc_copy(&gl_rmenu, &gl_ctwait.m_gr);
	}

	/* make ctlmgr fix up the size of rect it is waiting for */
	post_button(ctl_pd, 0x0000, 1);
	dsptch();
}


/*
 *	Routine to tell all desk accessories that the currently running
 *	application is about to terminate.
 */
void mn_clsda(void)
{
	_WORD i;

	for (i = 0; i < gl_dacnt; i++)
		if (desk_ppd[i])
			ap_sendmsg(appl_msg, AC_CLOSE, desk_ppd[i], i, 0, 0, 0, 0);
}


/*
 * AES #35 - menu_register - Register a desk accessory in the 'Desk' accessory menu.
 *
 *	Routine to register a desk accessory item on the menu bar.
 *	The return value is the object index of the menu item that
 *	was added.
 */
_WORD mn_register(_WORD pid, char *pstr)
{
	char tmpname[13];

	/* use this to name our process */
	if (pid == -1)
	{
		strmaxcpy(tmpname, sizeof(tmpname), pstr);
		p_nameit(rlr, tmpname);
		return TRUE;
	}
	/* add desk acc. if room */
	if (gl_dacnt < MAX_ACCS)
	{
		desk_ppd[gl_dacnt] = rlr;
		desk_acc[gl_dacnt] = pstr;
		gl_dacnt++;
		return gl_dacnt - 1;
	}
	return -1;
}


/*
 *	Routine to reset all variables related to menu registration
 */
void mn_init(void)
{
	_WORD i;

	for (i = 0; i < NUM_ACCS; i++)
	{
		desk_ppd[i] = NULL;
		desk_acc[i] = NULL;
	}

	gl_dacnt = 0;
}


/*
 * AES #34 - menu_text - Replaces the text of a menu item.
 */
void mn_text(OBJECT *tree, _WORD item, const char *ptext)
{
	strcpy(tree[item].ob_spec.free_string, ptext);
}


/*
 * Change the waiting rectangle for new menu bar
 */
void ch_wrect(GRECT *r, GRECT *n)
{
	AESPD *p;
	EVB *e;
	_LONG p1, p2;

	p = ctl_pd;

	for (e = p->p_cda->c_msleep; e; e = e->e_link)
	{
		p1 = MAKE_ULONG(r->g_x, r->g_y);
		p2 = MAKE_ULONG(r->g_w, r->g_h);
		if (p1 == e->e_parm && p2 == e->e_return)
		{
			e->e_parm = MAKE_ULONG(n->g_x, n->g_y);
			e->e_return = MAKE_ULONG(n->g_w, n->g_h);
		}
	}
}
