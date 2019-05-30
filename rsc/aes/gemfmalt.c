/*		GEMFMALT.C				09/01/84 - 06/20/85 	Lee Lorenzen	*/
/*		merge High C vers. w. 2.2 & 3.0 		8/20/87 		mdf 	*/

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

#include "gem_rsc.h"


#define NUM_ALOBJS	 (MAX_LINENUM + MAX_BUTNUM + 2)					/* MUST match # of objects in the DIALERT tree! */
#define INTER_WSPACE 0
#define INTER_HSPACE 0

static int emutos_like = FALSE;


/*
 *	Routine to break a string into smaller strings.  Breaks occur
 *	whenever an | or a ] is encountered.
 *
 *	Input:	start		starting object
 *			maxnum		maximum number of substrings
 *			maxlen		maximum length of substring
 *			alert		starting point in alert string
 *	Output: pnum		number of substrings found
 *			plen		maximum length of substring found
 *	Returns:			pointer to next character to process
 */
#define endstring(a)	( ((a)==']') || ((a)=='\0') )
#define endsubstring(a) ( ((a)=='|') || ((a)==']') || ((a)=='\0') )

static const char *fm_strbrk(OBJECT *start, _WORD maxnum, _WORD maxlen, const char *alert, _WORD *pnum, _WORD *plen)
{
	int i, j, len;
	OBJECT *obj;
	char *p;

	*plen = 0;

	if (*alert == '[')					/* ignore a leading [ */
		alert++;

	for (i = 0, obj = start; i < maxnum; i++, obj++, alert++)
	{
		p = obj->ob_spec.free_string;
		for (j = 0; j < maxlen; j++)
		{
			if (endsubstring(*alert))
				break;
			*p++ = *alert++;
		}
		*p = '\0';

		len = (int)(p - obj->ob_spec.free_string);
		if (len > *plen)				/* track max substring length */
			*plen = len;

		if (!endsubstring(*alert))		/* substring was too long */
		{
			KDEBUG(("form_alert(): substring > %d bytes long\n", maxlen));
			for (;;)					/* eat rest of substring */
			{
				if (endsubstring(*alert))
					break;
				alert++;
			}
		}
		if (endstring(*alert))			/* end of all substrings */
			break;
	}
	if (i >= maxnum)					/* too many substrings */
	{
		KDEBUG(("form_alert(): more than %d substrings\n", maxnum));
	}
	
	for (;;)							/* eat any remaining characters */
	{
		if (endstring(*alert))
			break;
		alert++;
	}

	*pnum = i < maxnum ? (i + 1) : maxnum;	/* count of substrings found */

	if (*alert)							/* if not at null byte, */
		alert++;						/* point to next one    */

	return alert;
}


/*
 *	Routine to parse a string into an icon #, multiple message
 *	strings, and multiple button strings.  For example,
 *
 *		[0][This is some text|for the screen.][Ok|Cancel]
 *
 *	becomes:
 *		icon# = 0;
 *		1st msg line = This is some text
 *		2nd msg line = for the screen.
 *		1st button = Ok
 *		2nd button = Cancel
 *
 *	Input:	tree		address of tree
 *			palstr		pointer to alert string
 *	Output: pnummsg 	number of message lines
 *			plenmsg 	length of biggest line
 *			pnumbut 	number of buttons
 *			plenbut 	length of biggest button
 */
static void fm_parse(OBJECT *tree, const char *palstr, _WORD *picnum, _WORD *pnummsg, _WORD *plenmsg, _WORD *pnumbut, _WORD *plenbut)
{
	const char *alert = palstr;

	*picnum = alert[1] - '0';

	alert = fm_strbrk(tree + MSGOFF, MAX_LINENUM, MAX_LINELEN, alert + 3, pnummsg, plenmsg);
	if (*plenmsg > TOS_MAX_LINELEN)
	{
		KDEBUG(("form_alert(): warning: alert line(s) exceed TOS standard length\n"));
	}
	
	fm_strbrk(tree + BUTOFF, MAX_BUTNUM, MAX_BUTLEN, alert, pnumbut, plenbut);
	if (*plenbut > TOS_MAX_BUTLEN)
	{
		KDEBUG(("form_alert(): warning: alert button(s) exceed TOS standard length\n"));
	}
	
	*plenbut += 1;						/* allow 1/2 character space inside each end of button */
}


/*
 *	Routine to build the alert
 *
 *	Inputs are:
 *		tree			the alert dialog
 *		haveicon		boolean, 1 if icon specified
 *		nummsg			number of message lines
 *		mlenmsg 		length of longest line
 *		numbut			number of buttons
 *		mlenbut 		length of biggest button
 */
static void fm_build(OBJECT *tree, _BOOL haveicon, _WORD nummsg, _WORD mlenmsg, _WORD numbut, _WORD mlenbut)
{
	_WORD i, j, icw = 4, ich = 0, allbut;
	GRECT al, ic, bt, ms;
	OBJECT *obj;

	/*
	 * we use the GRECTs as workareas for building the object character coordinates:
	 *  'al'    the entire alert
	 *  'ms'    the first message line
	 *  'bt'    the first button
	 *  'ic'    the icon
	 */
	r_set(&al, 0, 0, 1 + INTER_WSPACE, 1 + INTER_HSPACE);
	r_set(&ms, 1 + INTER_WSPACE, 1 + INTER_HSPACE, mlenmsg, 1);
	r_set(&bt, 1 + INTER_WSPACE, 2 + INTER_HSPACE + nummsg, mlenbut, 1);
	r_set(&ic, 0, 0, 0, 0);

	/*
	 * if we have an icon, we must initialise 'ic' and adjust:
	 *  the width of the alert
	 *  the horizontal position of the first message line.
	 * since the alert at this stage is sized in characters, we must
	 * convert the icon height from pixels to characters, based on
	 * the current character height.
	 */
	if (haveicon)
	{
		ich = (aes_rsc_bitblk[NOTEBB]->bi_hl + gl_hchar - 1) / gl_hchar;
		icw = aes_rsc_bitblk[NOTEBB]->bi_wb;
		r_set(&ic, 1 + INTER_WSPACE, 1 + INTER_HSPACE, icw, ich);
		al.g_w += ic.g_w + 1 + INTER_WSPACE;
		ms.g_x = ic.g_x + ic.g_w + 1 + INTER_WSPACE;
	}

	/*
	 * final adjustments(1): alert width / button horizontal position
	 *  if the message lines need more space than the buttons, set the
	 *  alert width from the message length, and adjust the horizontal
	 *  position of the first button for a symmetrical effect.
	 *  otherwise, set the alert width from the button sizes and leave
	 *  the message lines left justified.
	 */
	allbut = numbut * mlenbut + 2 * (numbut - 1);
	if (mlenmsg + al.g_w > allbut + 1 + INTER_WSPACE)
	{
		al.g_w += mlenmsg + 1 + INTER_WSPACE;
		bt.g_x = (al.g_w - allbut) / 2;
	} else
	{
		al.g_w = allbut + 2 * (1 + INTER_WSPACE);
		bt.g_x = 1 + INTER_WSPACE;
	}

	/*
	 * final adjustments(2): button vertical position / alert height
	 *  ensure no overlap by putting the buttons below the icon,
	 *  subject to leaving at least a 1-line gap between the messages
	 *  and the buttons.
	 */
	bt.g_y = max(ic.g_y + ic.g_h, nummsg + 1) + 1 + INTER_HSPACE;
	al.g_h = max(bt.g_y + bt.g_h, ic.g_y + ic.g_h) + 1 + INTER_HSPACE;

	if (!emutos_like)
	{
		i = (mlenbut * numbut) + ((numbut - 1) * 2);
		i = max(i, mlenmsg);				/* find the max char length */
		/* find the max char height */
		j = max(nummsg, 1);
		r_set(&al, 0, 0, i + 2, j);			/* this is the alert box    */
		/* this is the message object   */
		r_set(&ms, 2, 0x0300, mlenmsg, 1);
	
		if (haveicon)
		{
			r_set(&ic, 1, 1, icw, ich);
			al.g_w += icw + 1;
			al.g_h = max(al.g_h, 1 + ich);
			ms.g_x += icw;
		}
	
		al.g_h += 3;
	
		/* center the buttons */
	
		i = (al.g_w - ((numbut - 1) * 2) - (mlenbut * numbut)) / 2;
	
		/* set the button   */
	
		r_set(&bt, i, al.g_h - 2, mlenbut, 1);
	
		/* now make the al.g_h smaller  */
		if (!gl_aes3d)
		{
			al.g_h -= 1;
			al.g_h += ((gl_hchar / 2) << 8);
		}
		tree[ROOT].ob_state = OS_OUTLINED;
		OBSPEC_SET_FRAMESIZE(tree[ROOT].ob_spec, 1); /* inside 1 */
	} else
	{
		tree[ROOT].ob_state = OS_SHADOWED;
		OBSPEC_SET_FRAMESIZE(tree[ROOT].ob_spec, 1); /* inside 2 */
	}
	
	/* init. root object */
	ob_setxywh(tree, ROOT, &al);
	for (i = 0, obj = tree; i < NUM_ALOBJS; i++, obj++)
		obj->ob_next = obj->ob_head = obj->ob_tail = NIL;

	/* add icon object */
	if (haveicon)
	{
		ob_setxywh(tree, ALICON, &ic);
		ob_add(tree, ROOT, ALICON);
	}

	/* add msg objects */
	for (i = 0; i < nummsg; i++)
	{
		ob_setxywh(tree, MSGOFF + i, &ms);
		ms.g_y++;
		ob_add(tree, ROOT, MSGOFF + i);
	}

	/* add button objects with 1 space between them */
	for (i = 0, obj = tree + BUTOFF; i < numbut; i++, obj++)
	{
		obj->ob_flags = OF_SELECTABLE | OF_EXIT | (gl_aes3d ? OF_FL3DACT : 0);
		obj->ob_state = OS_NORMAL;
		ob_setxywh(tree, BUTOFF + i, &bt);
		bt.g_x += mlenbut + 2;
		ob_add(tree, ROOT, BUTOFF + i);
	}

	/* set last object flag */
	(--obj)->ob_flags |= OF_LASTOB;
}


/*
 * AES #52 - form_alert - Display an alert box.
 */
_WORD fm_alert(_WORD defbut, const char *palstr, _UWORD flags)
{
	_WORD i;
	_WORD inm, nummsg, mlenmsg, numbut, mlenbut, image;
	OBJECT *tree;
	GRECT d, t;
	_BOOL display_only = flags & 1;

	/* init tree pointer */
	tree = aes_rsc_tree[DIALERT];
	emutos_like = flags & 2;
	
	if (gl_aes3d)
	{
		_UWORD color;
		_LONG spec;

		spec = tree[ROOT].ob_spec.index;
		spec &= 0xFFFFFF80L;
		spec |= IP_SOLID << 4;
	
		if (gl_alrtcol >= gl_ws.ws_ncolors)
			color = G_WHITE;
		else
			color = gl_alrtcol;
	
		spec |= color;
		tree[ROOT].ob_spec.index = spec;
	}

	fm_parse(tree, palstr, &inm, &nummsg, &mlenmsg, &numbut, &mlenbut);
	fm_build(tree, inm != 0, nummsg, mlenmsg, numbut, mlenbut);

	if (defbut)
	{
		tree[BUTOFF + defbut - 1].ob_flags |= OF_DEFAULT;
	}

	if (inm != 0)
	{
		if (emutos_like)
		{
			switch (inm)
			{
			case 1:
				image = NOTEBB;
				break;
			case 2:
				image = QUESTBB;
				break;
			default:
				image = STOPBB;
				break;
			}
		} else
		{
			switch (inm)
			{
			case 1:
				image = NOTEBB_TOS;
				break;
			case 2:
				image = QUESTBB_TOS;
				break;
			default:
				image = STOPBB_TOS;
				break;
			}
		}
		tree[ALICON].ob_spec.bitblk = (BITBLK *)NO_CONST(aes_rsc_bitblk[image]);

		/* fix up icon, 32x32 */
		tree[ALICON].ob_type = G_IMAGE;
		tree[ALICON].ob_width = tree[ALICON].ob_spec.bitblk->bi_wb * 8;
		tree[ALICON].ob_height = tree[ALICON].ob_spec.bitblk->bi_hl;
	}

	/* convert to pixels */
	for (i = 0; i < NUM_ALOBJS; i++)
		rs_obfix(tree, i);

	if (gl_aes3d)
	{
		_WORD x1, y1, x, y, w, h;
		
		for (i = 0; i < MAX_BUTNUM; i++)
			tree[BUTOFF + i].ob_height = gl_hbox;
	
		/* recalculate the box height   */
	
		if (!defbut)
			defbut = 1;
	
		ob_gclip(tree, BUTOFF + defbut - 1, &x1, &y1, &x, &y, &w, &h);
		y = y - tree[ROOT].ob_y;
		y += h + 2;
		tree[ROOT].ob_height = y;
	}

	/* center tree on screen */
	ob_center(tree, &d);

	/* Fix 2003-09-25: Limit drawing to the screen! */
	rc_intersect(&gl_rfull, &d);

	/* save screen underneath the alert */
	wm_update(BEG_UPDATE);
	gsx_gclip(&t);
	if (!display_only)
		bb_save(&d);

	/* draw the alert */
	gsx_sclip(&d);
	ob_draw(tree, ROOT, MAX_DEPTH);
	ctlmouse(TRUE);						/* turn on the mouse    */

	/* let user pick button */
	if (display_only)
		i = BUTOFF - 1;
	else
		i = fm_do(tree, ROOT);

	ctlmouse(FALSE);					/* back to the way it was */

	/* restore saved screen */
	if (!display_only)
	{
		gsx_sclip(&d);
		bb_restore(&d);
		gsx_sclip(&t);
	}
	wm_update(END_UPDATE);

	/* return selection */
	return i - BUTOFF + 1;
}
