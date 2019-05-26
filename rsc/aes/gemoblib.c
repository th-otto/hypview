#include "aes.h"
#include "debug.h"

_BOOL act3dtxt;					/* look of 3D activator text */
_BOOL act3dface;				/* selected look of 3D activator */
_BOOL ind3dtxt;					/* look of 3D indicator text */
_BOOL ind3dface;				/* selected look of 3D indicators */
_UWORD gl_indbutcol;			/* indicator button color */
_UWORD gl_actbutcol;			/* activator button color */
_UWORD gl_alrtcol;				/* alert background color */
_BOOL gl_aes3d;

/*****************************************************************/
/*                   COLOR ICON DECLARATIONS                     */
/*****************************************************************/

static _WORD gl_colmask[128];					/* global mask used by color icons */

				/* WARNING:  The size of this array has been */
				/* set to 128 words and no bound checking is */
				/* done by the code currently.  This can  */
				/* handle icons of 64 by 64 pixels.       */

/* This is the color table of RGB values that VDI expects in pixel-
 * packed mode (This whole table can be moved to a separate file).
 * The values were derived from 4 bit R, G, and B values that were
 * converted to 5 bit values.  The original values are from VDI's 
 * palette settings.
 */
static _UWORD const rgb_tab[] = {
	0xFFDF, 0xF800, 0x07C0, 0xFFC0, 0x001F, 0xF81F, 0x07DF, 0xB596,
	0x8410, 0xA000, 0x0500, 0xA500, 0x0014, 0xA014, 0x0514, 0x0000,
	0xFFDF, 0xE71C, 0xD69A, 0xC618, 0xB596, 0xA514, 0x9492, 0x8410,
	0x738E, 0x630C, 0x528A, 0x4208, 0x3186, 0x2104, 0x1082, 0x0000,
	0xF800, 0xF802, 0xF804, 0xF806, 0xF808, 0xF80A, 0xF80C, 0xF80E,
	0xF810, 0xF812, 0xF814, 0xF816, 0xF818, 0xF81A, 0xF81C, 0xF81F,
	0xE01F, 0xD01F, 0xC01F, 0xB01F, 0xA01F, 0x901F, 0x801F, 0x701F,
	0x601F, 0x501F, 0x401F, 0x301F, 0x201F, 0x101F, 0x001F, 0x009F,
	0x011F, 0x019F, 0x021F, 0x029F, 0x031F, 0x039F, 0x041F, 0x049F,
	0x051F, 0x059F, 0x061F, 0x069F, 0x071F, 0x07DF, 0x07DC, 0x07DA,
	0x07D8, 0x07D6, 0x07D4, 0x07D2, 0x07D0, 0x07CE, 0x07CC, 0x07CA,
	0x07C8, 0x07C6, 0x07C4, 0x07C2, 0x07C0, 0x17C0, 0x27C0, 0x37C0,
	0x47C0, 0x57C0, 0x67C0, 0x77C0, 0x87C0, 0x97C0, 0xA7C0, 0xB7C0,
	0xC7C0, 0xD7C0, 0xE7C0, 0xFFC0, 0xFF00, 0xFE80, 0xFE00, 0xFD80,
	0xFD00, 0xFC80, 0xFC00, 0xFB80, 0xFB00, 0xFA80, 0xFA00, 0xF980,
	0xF900, 0xF880, 0xB000, 0xB002, 0xB004, 0xB006, 0xB008, 0xB00A,
	0xB00C, 0xB00E, 0xB010, 0xB012, 0xB014, 0xB016, 0xA016, 0x9016,
	0x8016, 0x7016, 0x6016, 0x5016, 0x4016, 0x3016, 0x2016, 0x1016,
	0x0016, 0x0096, 0x0116, 0x0196, 0x0216, 0x0296, 0x0316, 0x0396,
	0x0416, 0x0496, 0x0516, 0x0596, 0x0594, 0x0592, 0x0590, 0x058E,
	0x058C, 0x058A, 0x0588, 0x0586, 0x0584, 0x0582, 0x0580, 0x1580,
	0x2580, 0x3580, 0x4580, 0x5580, 0x6580, 0x7580, 0x8580, 0x9580,
	0xA580, 0xB580, 0xB500, 0xB480, 0xB400, 0xB380, 0xB300, 0xB280,
	0xB200, 0xB180, 0xB100, 0xB080, 0x7000, 0x7002, 0x7004, 0x7006,
	0x7008, 0x700A, 0x700C, 0x700E, 0x600E, 0x500E, 0x400E, 0x300E,
	0x200E, 0x100E, 0x000E, 0x008E, 0x010E, 0x018E, 0x020E, 0x028E,
	0x030E, 0x038E, 0x038C, 0x038A, 0x0388, 0x0386, 0x0384, 0x0382,
	0x0380, 0x1380, 0x2380, 0x3380, 0x4380, 0x5380, 0x6380, 0x7380,
	0x7300, 0x7280, 0x7200, 0x7180, 0x7100, 0x7080, 0x4000, 0x4002,
	0x4004, 0x4006, 0x4008, 0x3008, 0x2008, 0x1008, 0x0008, 0x0088,
	0x0108, 0x0188, 0x0208, 0x0206, 0x0204, 0x0202, 0x0200, 0x1200,
	0x2200, 0x3200, 0x4200, 0x4180, 0x4100, 0x4080, 0xFFDF, 0x0000
};

/******************** END COLOR *******************************/


/*
 * AES #48 - objc_sysvar - Manipulation of 3D objects. 
 *
 *	Routine to get or set object extension settings
 */
_WORD ob_sysvar(_UWORD mode, _UWORD which, _WORD inval1, _WORD inval2, _WORD *outval1, _WORD *outval2)
{
	int ret;
	ret = TRUE;							/* assume OK */

	if (mode)
	{									/* if set */
		switch (which)
		{
		case LK3DIND:
		case LK3DACT:
			if (inval1 != -1)
			{
				if (which == LK3DIND)
				{
					ind3dtxt = inval1;
				} else
				{
					act3dtxt = inval1;
				}
			}

			if (inval2 != -1)
			{
				if (which == LK3DIND)
				{
					ind3dface = inval2;
				} else
				{
					act3dface = inval2;
				}
			}
			break;

		case INDBUTCOL:
		case ACTBUTCOL:
		case BACKGRCOL:
			if (gl_ws.ws_ncolors <= inval1)
				return FALSE;			/* return error if invalid */
			if (which == INDBUTCOL)
				gl_indbutcol = inval1;	/* set indicator button color */
			else if (which == ACTBUTCOL)
				gl_actbutcol = inval1;	/* set activator button color */
			else
				gl_alrtcol = inval1;	/* set alert background color */
			break;

		default:
			ret = FALSE;				/* error */
			break;
		}

	} else
	{
		switch (which)
		{
		case LK3DIND:
			*outval1 = ind3dtxt;
			*outval2 = ind3dface;
			break;
		case LK3DACT:
			*outval1 = act3dtxt;
			*outval2 = act3dface;
			break;
		case INDBUTCOL:
			*outval1 = gl_indbutcol;
			break;
		case ACTBUTCOL:
			*outval1 = gl_actbutcol;
			break;
		case BACKGRCOL:
			*outval1 = gl_alrtcol;
			break;
		case AD3DVALUE:
			*outval1 = ADJ3DPIX;		/* horizontal */
			*outval2 = ADJ3DPIX;		/* vertical */
			break;
		default:
			ret = FALSE;				/* error */
			break;
		}
	}
	return ret;
}


/*
 *	Routine to take an unformatted raw string and based on a
 *	template string build a formatted string.
 */
void ob_format(_WORD just, char *raw_str, char *tmpl_str, char *fmtstr)
{
	char *pfbeg, *ptbeg, *prbeg;
	char *ptend, *prend;
	_WORD inc, ptlen, prlen;

	if (*raw_str == '@')
		*raw_str = '\0';

	pfbeg = fmtstr;
	ptbeg = tmpl_str;
	prbeg = raw_str;

	ptlen = strlen(tmpl_str);
	prlen = strlen(raw_str);

	inc = 1;
	pfbeg[ptlen] = '\0';
	if (just == TE_RIGHT)
	{
		inc = -1;
		pfbeg = pfbeg + ptlen - 1;
		ptbeg = ptbeg + ptlen - 1;
		prbeg = prbeg + prlen - 1;
	}

	ptend = ptbeg + (inc * ptlen);
	prend = prbeg + (inc * prlen);

	while (ptbeg != ptend)
	{
		if (*ptbeg != '_')
		{
			*pfbeg = *ptbeg;
		} else
		{
			if (prbeg != prend)
			{
				*pfbeg = *prbeg;
				prbeg += inc;
			} else
			{
				*pfbeg = '_';
			}
		}
		pfbeg += inc;
		ptbeg += inc;
	}
}


/*
 *	Routine to load up and call a user-defined object draw or change
 *	routine.
 */
static _WORD ob_user(OBJECT *tree, _WORD obj, GRECT *pt, USERBLK *userblk, _WORD curr_state, _WORD new_state)
{
	PARMBLK pb;

	pb.pb_tree = tree;
	pb.pb_obj = obj;
	pb.pb_prevstate = curr_state;
	pb.pb_currstate = new_state;
	rc_copy(pt, (GRECT *)&pb.pb_x);
	gsx_gclip((GRECT *)&pb.pb_xc);
	pb.pb_parm = userblk->ub_parm;
	return userblk->ub_code(&pb);
}



/*
 *  Draw highlights around a rectangle depending on its state
 */
static void draw_hi(GRECT *prect, _WORD state, _WORD clip, _WORD th, _WORD icol)
{
	_WORD pts[12], col;
	GRECT r;

	gsx_moff();
	vsl_type(gl_handle, USERLINE);
	vsl_udsty(gl_handle, -1);			/* solid line style */

	if (clip)
		gsx_sclip(prect);

	rc_copy(prect, &r);

	if (th > 0)
	{									/* if border is inside, */
		r.g_x += th;					/* object area is 1 pixel */
		r.g_y += th;					/* in all around */
		r.g_w -= th << 1;
		r.g_h -= th << 1;
	}

	/* Draw hilite:
	 *   +--------------+
	 *   |[2,3] - [4,5] |
	 *   |  |           |
	 *   |[0,1]         |
	 *   |              |
	 *   +--------------+
	 */
	pts[0] = pts[2] = r.g_x;
	pts[3] = pts[5] = r.g_y;
	pts[1] = pts[3] + r.g_h - 2;
	pts[4] = pts[2] + r.g_w - 2;

	col = (state & OS_SELECTED) ? G_BLACK : G_WHITE;
	gsx_attr(FALSE, MD_REPLACE, col);
	v_pline(gl_handle, 3, pts);

	/* Draw shadow:
	 *   +--------------+
	 *   |              |
	 *   |         [4,5]|
	 *   |           |  |
	 *   | [0,1] - [2,3]|
	 *   +--------------+
	 */
	pts[0]++;
	pts[3] = ++pts[1];
	pts[2] = ++pts[4];
	pts[5]++;

	if (state & OS_SELECTED)
	{
		col = G_LWHITE;
		if (icol == col || gl_ws.ws_ncolors < col)
			col = G_WHITE;
	} else
	{
		col = G_LBLACK;
		if (icol == col || gl_ws.ws_ncolors < col)
			col = G_BLACK;
	}
	gsx_attr(FALSE, MD_REPLACE, col);
	v_pline(gl_handle, 3, pts);

	gsx_mon();
}


/*
 * Routine to XOR color in any resolution as if there is only 16 colors.
 * FROM THE VDI MANUAL: Section 6 (Raster Operations) Table 6-2
 * Pixel VDI #  Default color
 * ----- -----  -------------
 * 0000	    0	white
 * 0001	    2	red
 * 0010	    3	green
 * 0011	    6	yellow
 * 0100	    4	blue
 * 0101	    7	magenta
 * 0110	    5	cyan
 * 0111	    8	low white
 * 1000	    9	grey
 * 1001	    10	light red
 * 1010	    11	light green
 * 1011	    14	light yellow
 * 1100	    12	light blue
 * 1101	    15	light magenta
 * 1110	    13  light cyan
 * 1111	    1	black
 */


static _WORD xor16(_WORD col)
{
	static _WORD const xor16tab[] = {
    /*  G_WHITE,  G_BLACK,  G_RED,   G_GREEN,    G_BLUE,    G_CYAN,  G_YELLOW,   G_MAGENTA */
		G_BLACK,  G_WHITE,  G_LCYAN, G_LMAGENTA, G_LYELLOW, G_LRED,  G_LBLUE,    G_LGREEN,
    /*  G_LWHITE, G_LBLACK, G_LRED,  G_LGREEN,   G_LBLUE,   G_LCYAN, G_LYELLOW,  G_LMAGENTA */
		G_LBLACK, G_LWHITE, G_CYAN,  G_MAGENTA,  G_YELLOW,  G_RED,   G_BLUE,     G_GREEN
	};
	static _WORD const xor4tab[] = { G_BLACK, G_WHITE, G_GREEN, G_RED };

	if (col >= 16 || col >= gl_ws.ws_ncolors)	/* ws_ncolors */
		col = G_WHITE;
	else if (gl_ws.ws_ncolors <= 4)
		col = xor4tab[col];
	else
		col = xor16tab[col];

	return col;
}


/*
 * Return 1 if XOR writing mode is OK to toggle an object's OS_SELECTED state,
 * otherwise return 0.
 *
 * All strings and titles are OK to XOR, since they're always white/black.
 * Other NON-3d objects are OK to XOR if 16 or less colors are available,
 * or if they are all black & white.
 *
 * (used by just_draw() and ob_change())
 */
static _BOOL xor_ok(_WORD type, _WORD flags, OBSPEC spec)
{
	_UWORD i;
	_WORD tcol, icol, dummy;
	
	if (type == G_STRING || type == G_TITLE || type == G_IBOX)
		return 1;
	if (type == G_IMAGE || (flags & OF_FL3DIND))
		return 0;
	if (gl_ws.ws_ncolors <= 16)
		return 1;

	/* Get color word */
	switch (type)
	{
	case G_BOXTEXT:
	case G_FBOXTEXT:
	case G_TEXT:
	case G_FTEXT:
		i = spec.tedinfo->te_color;
		break;
	case G_BOX:
	case G_BOXCHAR:
	case G_IBOX:
		i = spec.index;
		break;
	default:							/* black border, black text, transparent, hollow, white fill */
		i = COLSPEC_MAKE(G_BLACK, G_BLACK, FALSE, IP_HOLLOW, G_WHITE);
		break;
	}
	gr_crack(i, &dummy, &tcol, &dummy, &icol, &dummy);

	return tcol < G_RED && icol < G_RED;
}


/*
 *	Routine to draw an object from an object tree.
 */
static void just_draw(OBJECT *tree, _WORD obj, _WORD sx, _WORD sy)
{
	_WORD bcol, tcol, ipat, icol, tmode, th;
	_WORD state, obtype, len, flags;
	OBSPEC spec;
	_WORD tmpx, tmpy, tmpth;
	_WORD thick;
	char ch;
	GRECT t, c;
	GRECT *pt;
	BITBLK bi;
	ICONBLK ib;
	TEDINFO edblk;
	_BOOL mvtxt;
	_BOOL chcol;
	_BOOL three_d;
	char rawstr[MAX_LEN];
	char tmpstr[MAX_LEN];
	char fmtstr[MAX_LEN];
	
	pt = &t;

	ch = ob_sst(tree, obj, &spec, &state, &obtype, &flags, pt, &th);

	if (flags & OF_HIDETREE)
		return;

	thick = th;

	pt->g_x = sx;
	pt->g_y = sy;

	/*
	 * Adjust 3d object extents & get color change/move text flags
	 */
	if (gl_aes3d && (flags & OF_FL3DIND))
	{
		three_d = TRUE;					/* object is 3D */
		tmpx = ADJ3DPIX;
		pt->g_x -= tmpx;				/* expand object to accomodate */
		pt->g_y -= tmpx;				/*  hi-lights for 3D */
		pt->g_w += (tmpx << 1);
		pt->g_h += (tmpx << 1);

		if ((flags & OF_FL3DBAK) == 0)
		{								/* if it's a 3D indicator */
			mvtxt = ind3dtxt;
			chcol = ind3dface;
		} else
		{								/* if it's a 3D activator */
			mvtxt = act3dtxt;
			chcol = act3dface;
		}
	} else
	{
		/* For non-3d objects, force color change if XOR is not ok. */
		three_d = FALSE;
		chcol = !xor_ok(obtype, flags, spec);
		mvtxt = FALSE; /* quiet compiler */
	}

	/*
	 * do trivial reject with full extent including outline, shadow,
	 * & thickness
	 * BUG: this rejects USERDEFs which draw more than a 3 border outline.
	 * It also rejects TEDINFOs with an outline thickness of more than 3 pixels.
	 */
	if (gl_clip.g_w && gl_clip.g_h)
	{
		rc_copy(pt, &c);
		if (state & OS_OUTLINED)
			gr_inside(&c, -3);
		else
			gr_inside(&c, th < 0 ? 3 * th : -3 * th);
		if (!gsx_chkclip(&c))
			return;
	}
	
	/*
	 * for all tedinfo types get copy of ted and crack the
	 * color word and set the text color
	 */
	rc_copy(pt, &c);
	if (obtype != G_STRING)
	{
		tmpth = (th < 0) ? 0 : th;
		tmode = MD_REPLACE;
		tcol = G_BLACK;

		switch (obtype)
		{
		case G_BOXTEXT:
		case G_FBOXTEXT:
		case G_TEXT:
		case G_FTEXT:
			edblk = *spec.tedinfo;
			gr_crack(edblk.te_color, &bcol, &tcol, &ipat, &icol, &tmode);

			/*
			 * if it's a 3D background object, draw it in 3D color
			 */
			/*
			 * this can get complicated, because gr_text will always draw a white
			 * background in replace mode. So we have to change TEXT objects
			 * into BOXTEXT objects with the right color background and with
			 * transparent text (and no borders)
			 */
			if (gl_aes3d && (flags & (OF_FL3DIND | OF_FL3DBAK)) && icol == G_WHITE && ipat == IP_HOLLOW)
			{
				ipat = IP_SOLID;
				switch (flags & (OF_FL3DIND | OF_FL3DBAK))
				{
				case OF_FL3DIND:
					icol = gl_indbutcol;
					break;
				case (OF_FL3DIND | OF_FL3DBAK):
					icol = gl_actbutcol;
					break;
				case OF_FL3DBAK:
					icol = gl_alrtcol;
					/* fall through */
				default:				/* should never happen!! */
					break;
				}
				icol = gl_alrtcol;
				if (tmode == MD_REPLACE)
				{
					if (obtype == G_TEXT)
					{
						obtype = G_BOXTEXT;
						tmpth = th = 0;
					} else if (obtype == G_FTEXT)
					{
						obtype = G_FBOXTEXT;
						tmpth = th = 0;
					}
					tmode = MD_TRANS;
				}
			}
			break;
		default:
			edblk.te_ptext = NULL;
			edblk.te_ptmplt = NULL;
			edblk.te_font = IBM;
			edblk.te_just = TE_LEFT;
			break;
		}
		
		/*
		 * for all box types crack the color if not ted and
		 * draw the box with border
		 */
		switch (obtype)
		{
		case G_BOX:
		case G_BOXCHAR:
		case G_IBOX:
			gr_crack(LOWORD(spec.index), &bcol, &tcol, &ipat, &icol, &tmode);
			if (gl_aes3d && obtype != G_IBOX && ipat == IP_HOLLOW && icol == G_WHITE)
			{
				switch (flags & (OF_FL3DIND | OF_FL3DBAK))
				{
				case OF_FL3DIND:
					icol = gl_indbutcol;
					ipat = IP_SOLID;
					break;
				case (OF_FL3DIND | OF_FL3DBAK):
					icol = gl_actbutcol;
					ipat = IP_SOLID;
					break;
				case OF_FL3DBAK:
					icol = gl_alrtcol;
					ipat = IP_SOLID;
					/* fall through */
				default:
					break;
				}
			}
			/* fall through */
		case G_BUTTON:
			if (obtype == G_BUTTON)
			{
				bcol = G_BLACK;
				if (three_d)
				{
					ipat = IP_SOLID;
					if ((flags & OF_FL3DBAK) == 0)
					{
						icol = gl_indbutcol;
					} else
					{
						icol = gl_actbutcol;
					}
				} else
				{
					ipat = IP_HOLLOW;
					icol = G_WHITE;
				}
			}
			/* fall through */
		case G_BOXTEXT:
		case G_FBOXTEXT:
			/* draw box's border */
			if (th != 0)
			{
				gsx_attr(FALSE, MD_REPLACE, bcol);
				gr_box(pt->g_x, pt->g_y, pt->g_w, pt->g_h, th);
			}
			/* draw filled box */
			if (obtype != G_IBOX)
			{
				gr_inside(pt, tmpth);
				if (gl_aes3d && chcol && (state & OS_SELECTED))
				{
					/* Explicitly set a 4-bit XOR fill color.
					 * If pattern is hollow, make it solid
					 * so the color is visible.
					 */
					if (ipat == IP_HOLLOW)
					{
						ipat = IP_SOLID;
						icol = G_BLACK;
					} else
					{
						icol = xor16(icol);
					}
				}

				gr_rect(icol, ipat, pt);
				gr_inside(pt, -tmpth);
			}
			break;
		}

		if (gl_aes3d && chcol && (state & OS_SELECTED))
		{
			tmode = MD_TRANS;
			tcol = xor16(tcol);
		}

		gsx_attr(TRUE, tmode, tcol);
		
		/* do whats left for all the other types */
		switch (obtype)
		{
		case G_FTEXT:
		case G_FBOXTEXT:
			strmaxcpy(rawstr, sizeof(rawstr), edblk.te_ptext);
			strmaxcpy(tmpstr, sizeof(tmpstr), edblk.te_ptmplt);
			ob_format(edblk.te_just, rawstr, tmpstr, fmtstr);
			/* fall through */
		case G_BOXCHAR:
			edblk.te_ptext = fmtstr;
			if (obtype == G_BOXCHAR)
			{
				fmtstr[0] = ch;
				fmtstr[1] = '\0';
				edblk.te_just = TE_CNTR;
				edblk.te_font = IBM;
			}
			/* fall through */
		case G_TEXT:
		case G_BOXTEXT:
			gr_inside(pt, tmpth);

			/* Draw text of 3D objects */
			if (three_d)
			{
				if (!(state & OS_SELECTED) && mvtxt)
				{
					pt->g_x -= 1;
					pt->g_y -= 1;
					gr_gtext(edblk.te_just, edblk.te_font, edblk.te_ptext, pt);
					pt->g_x += 1;
					pt->g_y += 1;
				} else
				{
					gr_gtext(edblk.te_just, edblk.te_font, edblk.te_ptext, pt);
				}
			} else
			{
				gr_gtext(edblk.te_just, edblk.te_font, edblk.te_ptext, pt);
			}
			gr_inside(pt, -tmpth);
			break;
		case G_IMAGE:
			bi = *spec.bitblk;
			if (gl_aes3d && (state & OS_SELECTED))
			{
				/* If selected, XOR the background before drawing the image */
				bb_fill(MD_XOR, FIS_SOLID, IP_SOLID, pt->g_x, pt->g_y, pt->g_w, pt->g_h);
				bi.bi_color = xor16(bi.bi_color);
			}
			gsx_blt(bi.bi_pdata, bi.bi_x, bi.bi_y, bi.bi_wb * 8,
					ORGADDR, pt->g_x, pt->g_y, gl_width, bi.bi_wb * 8, bi.bi_hl, MD_TRANS, bi.bi_color, G_WHITE);
			break;
		case G_ICON:
			ib = *spec.iconblk;
			ib.ib_xicon += pt->g_x;
			ib.ib_yicon += pt->g_y;
			ib.ib_xtext += pt->g_x;
			ib.ib_ytext += pt->g_y;
			gr_gicon(state, ib.ib_pmask, ib.ib_pdata, ib.ib_ptext,
					ib.ib_char, ib.ib_xchar, ib.ib_ychar, (GRECT *)&ib.ib_xicon, (GRECT *)&ib.ib_xtext);
			state &= ~OS_SELECTED;
			break;
		case G_CICON:
			/*
			 * Identical to the monochrome icon case (above)
			 * except for the gr_cicon() call.
			 */
			ib = spec.ciconblk->monoblk;
			ib.ib_xicon += pt->g_x;
			ib.ib_yicon += pt->g_y;
			ib.ib_xtext += pt->g_x;
			ib.ib_ytext += pt->g_y;
			gr_cicon(state, ib.ib_pmask, ib.ib_pdata, ib.ib_ptext,
					 ib.ib_char, ib.ib_xchar, ib.ib_ychar, (GRECT *)&ib.ib_xicon, (GRECT *)&ib.ib_xtext, spec.ciconblk);
			state &= ~OS_SELECTED;
			break;
		case G_USERDEF:
			state = ob_user(tree, obj, pt, spec.userblk, state, state);
			break;
		}
	}
	
	/*
	 * draw text for string/title/button
	 */
	if (obtype == G_STRING ||
		obtype == G_TITLE ||
		obtype == G_BUTTON)
	{
		vdi_wchar_t wtext[MAX_LEN];
		
		len = vdi_str2arrayn(spec.free_string, wtext, MAX_LEN);
		if (len)
		{
			if (gl_aes3d && (state & OS_SELECTED) && obtype == G_BUTTON && chcol)
				tcol = G_WHITE;
			else
				tcol = G_BLACK;
			gsx_attr(TRUE, MD_TRANS, tcol);

			tmpy = pt->g_y + ((pt->g_h - gl_hchar) / 2);
			if (obtype == G_BUTTON)
			{
				tmpx = pt->g_x + ((pt->g_w - (len * gl_wchar)) / 2);

				if (three_d)
				{
					if (!(state & OS_SELECTED) && mvtxt)
					{
						tmpx -= 1;
						tmpy -= 1;
					}
				}
			} else
			{
				tmpx = pt->g_x;
			}

			gsx_tblt(IBM, tmpx, tmpy, wtext, len);
		}
	}

	/*
	 * handle changes in appearance due to object state
	 */
	if (state)
	{
		if (state & OS_OUTLINED)
		{
			if (!gl_aes3d || (flags & (OF_FL3DBAK | OF_FL3DIND)) != OF_FL3DBAK)
			{
				gsx_attr(FALSE, MD_REPLACE, G_BLACK);
				gr_box(pt->g_x - 3, pt->g_y - 3, pt->g_w + 6, pt->g_h + 6, 1);
				gsx_attr(FALSE, MD_REPLACE, G_WHITE);
				gr_box(pt->g_x - 2, pt->g_y - 2, pt->g_w + 4, pt->g_h + 4, 2);
			} else
			{
				/* draw a 3D outline for 3D background objects */
				gsx_attr(FALSE, MD_REPLACE, G_LBLACK);
				gsx_cline(pt->g_x + pt->g_w + 2, pt->g_y - 3, pt->g_x + pt->g_w + 2, pt->g_y + pt->g_h + 2);
				gsx_cline(pt->g_x + pt->g_w + 1, pt->g_y - 2, pt->g_x + pt->g_w + 1, pt->g_y + pt->g_h + 1);
				gsx_cline(pt->g_x + pt->g_w, pt->g_y - 1, pt->g_x + pt->g_w, pt->g_y + pt->g_h);
				gsx_cline(pt->g_x - 3, pt->g_y + pt->g_h + 2, pt->g_x + pt->g_w + 2, pt->g_y + pt->g_h + 2);
				gsx_cline(pt->g_x - 2, pt->g_y + pt->g_h + 1, pt->g_x + pt->g_w + 2, pt->g_y + pt->g_h + 1);
				gsx_cline(pt->g_x - 1, pt->g_y + pt->g_h, pt->g_x + pt->g_w + 2, pt->g_y + pt->g_h);

				gsx_attr(FALSE, MD_REPLACE, G_WHITE);
				gsx_cline(pt->g_x - 3, pt->g_y - 3, pt->g_x + pt->g_w + 2, pt->g_y - 3);
				gsx_cline(pt->g_x - 3, pt->g_y - 2, pt->g_x + pt->g_w + 1, pt->g_y - 2);
				gsx_cline(pt->g_x - 3, pt->g_y - 1, pt->g_x + pt->g_w, pt->g_y - 1);
				gsx_cline(pt->g_x - 3, pt->g_y - 3, pt->g_x - 3, pt->g_y + pt->g_h + 2);
				gsx_cline(pt->g_x - 2, pt->g_y - 3, pt->g_x - 2, pt->g_y + pt->g_h + 1);
				gsx_cline(pt->g_x - 1, pt->g_y - 3, pt->g_x - 1, pt->g_y + pt->g_h);
			}
		}

		if (th > 0) 					/* ensure t is inside the border */
			gr_inside(pt, th);
		else
			th = -th;

		if ((state & OS_SHADOWED) && th)
		{
			vsf_color(gl_handle, bcol);
			/* draw the vertical line */
			bb_fill(MD_REPLACE, FIS_SOLID, IP_HOLLOW, pt->g_x, pt->g_y + pt->g_h + th, pt->g_w + th, 2 * th);
			/* draw the horizontal line */
			bb_fill(MD_REPLACE, FIS_SOLID, IP_HOLLOW, pt->g_x + pt->g_w + th, pt->g_y, 2 * th, pt->g_h + (3 * th));
		}

		if (state & OS_CHECKED)
		{
			vdi_wchar_t ch = 0x08;   /* a check mark */
			gsx_attr(TRUE, MD_TRANS, G_BLACK);
			gsx_tblt(IBM, pt->g_x + 2, pt->g_y, &ch, 1);
		}

		if (state & OS_CROSSED)
		{
			gsx_attr(FALSE, MD_TRANS, G_WHITE);
			gsx_cline(pt->g_x, pt->g_y, pt->g_x + pt->g_w - 1, pt->g_y + pt->g_h - 1);
			gsx_cline(pt->g_x, pt->g_y + pt->g_h - 1, pt->g_x + pt->g_w - 1, pt->g_y);
		}

		if (state & OS_DISABLED)
		{
			if (gl_aes3d && (flags & (OF_FL3DIND | OF_FL3DBAK)) == OF_FL3DBAK)
				vsf_color(gl_handle, gl_alrtcol);
			else
				vsf_color(gl_handle, G_WHITE);

			bb_fill(MD_TRANS, FIS_PATTERN, IP_4PATT, pt->g_x, pt->g_y, pt->g_w, pt->g_h);
		}

		if ((state & OS_SELECTED)
			&& (!gl_aes3d || (!(chcol || three_d)))
			)
		{
			bb_fill(MD_XOR, FIS_SOLID, IP_SOLID, pt->g_x, pt->g_y, pt->g_w, pt->g_h);
		}
	}

	if (three_d)
	{
		if (state & OS_SELECTED)
			draw_hi(&c, OS_SELECTED, FALSE, thick, icol);
		else
			draw_hi(&c, OS_NORMAL, FALSE, thick, icol);
	}
}



/*
 * AES #42 - objc_draw - Draw an AES object tree.
 *
 *	Object draw routine that walks tree and draws appropriate objects.
 */
void ob_draw(OBJECT *tree, _WORD obj, _WORD depth)
{
	_WORD last, pobj;
	_WORD sx, sy;

	last = obj == ROOT ? NIL : tree[obj].ob_next;

	pobj = get_par(tree, obj);

	if (pobj != NIL)
		ob_offset(tree, pobj, &sx, &sy);
	else
		sx = sy = 0;

	gsx_moff();
	everyobj(tree, obj, last, just_draw, sx, sy, depth);
	gsx_mon();
}


/*
 *	Routine to find the object that is previous to us in the
 *	tree.  The idea is we get our parent and then walk down
 *	his list of children until we find a sibling who points to
 *	us.  If we are the first child or we have no parent then
 *	return NIL.
 */
static _WORD get_prev(OBJECT *tree, _WORD parent, _WORD obj)
{
	_WORD nobj, pobj;

	pobj = tree[parent].ob_head;
	if (pobj == obj)
		return NIL;
	for (;;)
	{
		nobj = tree[pobj].ob_next;
		if (nobj == obj)
			return pobj;
		if (nobj == parent) 	/* next object is the parent, */
			return NIL; 		/*	so we're not in this chain */
		pobj = nobj;
	}
}


/*
 * AES #43 - objc_find - Find which object that lies at the specified screen position.
 *
 *	Routine to find out which object a certain mx,my value is
 *	over.  Since each parent object contains its children the
 *	idea is to walk down the tree, limited by the depth parameter,
 *	and find the last object the mx,my location was over.
 */
/************************************************************************/
/* o b _ f i n d                                                        */
/************************************************************************/
_WORD ob_find(OBJECT *tree, _WORD currobj, _WORD depth, _WORD mx, _WORD my)
{
	_WORD lastfound;
	_WORD dummy;
	_WORD dosibs;
	_BOOL done;
	_WORD state;
	GRECT t, o;
	_WORD parent, childobj, flags;
	GRECT *pt;

	pt = &t;

	lastfound = NIL;

	if (currobj == ROOT)
	{
		r_set(&o, 0, 0, 0, 0);
	} else
	{
		parent = get_par(tree, currobj);
		ob_actxywh(tree, parent, &o);
	}

	done = FALSE;
	dosibs = FALSE;

	while (!done)
	{
		/*
		 * if inside this obj, might be inside a child, so check
		 */

		state = tree[currobj].ob_state;
		if (gl_aes3d && !(state & OS_SHADOWED))
		{
			ob_gclip(tree, currobj, &dummy, &dummy, &pt->g_x, &pt->g_y, &pt->g_w, &pt->g_h);
		} else
		{
			ob_relxywh(tree, currobj, pt);
			pt->g_x += o.g_x;
			pt->g_y += o.g_y;
		}
		
		flags = tree[currobj].ob_flags;
		if (inside(mx, my, pt) && !(flags & OF_HIDETREE))
		{
			lastfound = currobj;

			childobj = tree[currobj].ob_tail;
			if (childobj != NIL && depth)
			{
				currobj = childobj;
				depth--;
				o.g_x = pt->g_x;
				o.g_y = pt->g_y;
				dosibs = TRUE;
			} else
			{
				done = TRUE;
			}
		} else
		{
			if (dosibs && lastfound != NIL)
			{
				currobj = get_prev(tree, lastfound, currobj);
				if (currobj == NIL)
					done = TRUE;
			} else
			{
				done = TRUE;
			}
		}
	}
	/*
	 * if no one was found, this will return NIL
	 */
	return lastfound;
}


/*
 * AES #40 - objc_add - Insert object in an object tree.
 *
 *	Routine to add a child object to a parent object.  The child
 *	is added at the end of the parent's current sibling list.
 *	It is also initialized.
 */
void ob_add(OBJECT *tree, _WORD parent, _WORD child)
{
	_WORD lastkid;

	if (parent != NIL && child != NIL)
	{
		/* initialize child */
		tree[child].ob_next = parent;

		lastkid = tree[parent].ob_tail;
		if (lastkid == NIL)
			/* this is parent's 1st kid, so both head and tail pt to it */
			tree[parent].ob_head = child;
		else
			/* add kid to end of kid list */
			tree[lastkid].ob_next = child;
		tree[parent].ob_tail = child;
	}
}


/*
 * AES #41 - objc_delete - Remove object from an object tree.
 *
 *	Routine to delete an object from the tree.
 */
_BOOL ob_delete(OBJECT *tree, _WORD obj)
{
	_WORD parent;
	_WORD prev, nextsib;

	if (obj == ROOT)
		return FALSE;			/* can't delete the root object! */
	
	nextsib = tree[obj].ob_next;
	parent = get_par(tree, obj);

	if (tree[parent].ob_head == obj)
	{
		/* this is head child in list */
		if (tree[parent].ob_tail == obj)
		{
			/* this is only child in list, so fix head & tail ptrs */
			nextsib = NIL;
			tree[parent].ob_tail = NIL;
		}
		/* move head ptr to next child in list */
		tree[parent].ob_head = nextsib;
	} else
	{
		/* it's somewhere else, so move next around it */
		prev = get_prev(tree, parent, obj);
		if (prev == NIL)  /* object not found */
		{
			KINFO(("objc_delete() failed: object %d not found\n", obj));
			return 0;
		}
		tree[prev].ob_next = nextsib;
		if (tree[parent].ob_tail == obj)
			/* this is last child in list, so move tail ptr to prev child in list*/
			tree[parent].ob_tail = prev;
	}
	return TRUE;
}


/*
 * AES #45 - objc_order - Alter order of object in object tree.
 *
 *	Routine to change the order of an object relative to its
 *	siblings in the tree.  0 is the head of the list and NIL
 *	is the tail of the list.
 */
_BOOL ob_order(OBJECT *tree, _WORD mov_obj, _WORD new_pos)
{
	_WORD parent;
	_WORD chg_obj, ii;

	if (mov_obj == ROOT)
		return FALSE;
	parent = get_par(tree, mov_obj);
	if (parent == NIL)
		return FALSE;
	
	if (ob_delete(tree, mov_obj) == FALSE)
		return FALSE;
	chg_obj = tree[parent].ob_head;
	if (new_pos == 0)
	{
		/* put mov_obj at head of list */
		tree[mov_obj].ob_next = chg_obj;
		tree[parent].ob_head = mov_obj;
	} else
	{
		/* find new_pos */
		if (new_pos == NIL)
		{
			chg_obj = tree[parent].ob_tail;
		} else
		{
			for (ii = 1; ii < new_pos; ii++)
				chg_obj = tree[chg_obj].ob_next;
		}
		/* now add mov_obj after chg_obj */
		tree[mov_obj].ob_next = tree[chg_obj].ob_next;
		tree[chg_obj].ob_next = mov_obj;
	}
	if (tree[mov_obj].ob_next == parent)
		tree[parent].ob_tail = mov_obj;
	return TRUE;
}



/************************************************************************/
/* o b _ e d i t                                                        */
/************************************************************************/
/* see OBED.C */

/*
 * AES #47 - objc_change - Alter display of an object within specified limits.
 *
 *	Routine to change the state of an object and redraw that
 *	object using the current clip rectangle.
 */
_BOOL ob_change(OBJECT *tree, _WORD obj, _WORD new_state, _WORD redraw)
{
	_WORD flags, obtype, th;
	GRECT t;
	_WORD curr_state;
	OBSPEC spec;
	GRECT *pt;

	pt = &t;

	ob_sst(tree, obj, &spec, &curr_state, &obtype, &flags, pt, &th);
	
	if (curr_state == new_state)
		return FALSE;

	tree[obj].ob_state = new_state;

	if (redraw)
	{
		ob_offset(tree, obj, &pt->g_x, &pt->g_y);

		gsx_moff();

		th = (th < 0) ? 0 : th;

		if (obtype == G_USERDEF)
		{
			ob_user(tree, obj, pt, spec.userblk, curr_state, new_state);
			redraw = FALSE;
		} else if (obtype != G_ICON && ((new_state ^ curr_state) & OS_SELECTED))
		{
			/* For non-icon objects, see if we can toggle selection by XOR.
			 * G_IMAGE objects must be DEselected by XOR draw, *and*
			 * the image must be redrawn by just_draw().  If they're selected,
			 * just_draw() does the XOR box before redrawing the image.
			 */
			_BOOL xok = xor_ok(obtype, flags, spec);

			if (!gl_aes3d || xok || (obtype == G_IMAGE && !(new_state & OS_SELECTED)))
			{
				bb_fill(MD_XOR, FIS_SOLID, IP_SOLID, pt->g_x + th, pt->g_y + th, pt->g_w - (2 * th), pt->g_h - (2 * th));
				redraw = gl_aes3d && !xok;
			}
		}

		if (redraw)
			just_draw(tree, obj, pt->g_x, pt->g_y);

		gsx_mon();
	}
	
	return TRUE;
}


_UWORD ob_fs(OBJECT *tree, _WORD ob, _WORD *pflag)
{
	*pflag = tree[ob].ob_flags;
	return tree[ob].ob_state;
}


/************************************************************************/
/* o b _ a c t x y w h                                                  */
/* fill GRECT with x/y/w/h of object (absolute x/y)                     */
/************************************************************************/
void ob_actxywh(OBJECT *tree, _WORD obj, GRECT * pt)
{
	if (gl_aes3d)
	{
		_WORD x, y;

		ob_gclip(tree, obj, &x, &y, &pt->g_x, &pt->g_y, &pt->g_w, &pt->g_h);
	} else
	{
		ob_offset(tree, obj, &pt->g_x, &pt->g_y);
		pt->g_w = tree[obj].ob_width;
		pt->g_h = tree[obj].ob_height;
	}
}


/************************************************************************/
/* o b _ r e l x y w h                                                  */
/************************************************************************/
void ob_relxywh(OBJECT *tree, _WORD obj, GRECT *pt)
{
	pt->g_x = tree[obj].ob_x;
	pt->g_y = tree[obj].ob_y;
	pt->g_w = tree[obj].ob_width;
	pt->g_h = tree[obj].ob_height;
}


/*
 * ob_setxywh: copy values from GRECT into object
 */
void ob_setxywh(OBJECT *tree, _WORD obj, const GRECT *pt)
{
	tree[obj].ob_x = pt->g_x;
	tree[obj].ob_y = pt->g_y;
	tree[obj].ob_width = pt->g_w;
	tree[obj].ob_height = pt->g_h;
}


/*
 * AES #44 - objc_offset - Calculate the true position of an object on the screen.
 *
 *	Routine to find the x,y offset of a particular object relative
 *	to the physical screen.  This involves accumulating the offsets
 *	of all the objects parents up to and including the root.
 */
void ob_offset(OBJECT *tree, _WORD obj, _WORD *pxoff, _WORD *pyoff)
{
	*pxoff = *pyoff = 0;
	do
	{
		/* have our parent-- add in his x, y */
		*pxoff += tree[obj].ob_x;
		*pyoff += tree[obj].ob_y;
		obj = get_par(tree, obj);
	} while (obj != NIL);
}


/*
 * Return X, Y, W, and H deltas between basic object
 * rectangle and object clip rectangle.  These are the
 * numbers that you add to the x, y, w, h in the OBJECT
 * to get the clip rectangle.
 */
void ob_dxywh(OBJECT *tree, _WORD obj, _WORD *pdx, _WORD *pdy, _WORD *pdw, _WORD *pdh)
{
	GRECT relr, actr;

	ob_relxywh(tree, obj, &relr);
	ob_gclip(tree, obj, &relr.g_x, &relr.g_y, &actr.g_x, &actr.g_y, &actr.g_w, &actr.g_h);
	*pdx = actr.g_x - relr.g_x;
	*pdy = actr.g_y - relr.g_y;
	*pdw = actr.g_w - relr.g_w;
	*pdh = actr.g_h - relr.g_h;
}

#define ADJOUTLPIX 3
#define ADJSHADPIX 2

/*
 * Return a clip rectangle describing an object's screen coordinates.
 */
void ob_gclip(OBJECT *tree, _WORD obj, _WORD *pxoff, _WORD *pyoff, _WORD *pxcl, _WORD *pycl, _WORD *pwcl, _WORD *phcl)
{
	GRECT r;
	OBSPEC spec;
	_WORD state, type, flags, border;
	_WORD x, y, off3d, offset;

	/*
	 * Get the object's base rectangle, & other stuff.
	 * NOTE: ob_sst() accounts for OF_EXIT/OF_DEFAULT button & G_TITLE outlines.
	 */
	ob_offset(tree, obj, &x, &y);
	ob_sst(tree, obj, &spec, &state, &type, &flags, &r, &border);
	*pxoff = r.g_x = x;
	*pyoff = r.g_y = y;

	/* Get gr_inside() offset */
	off3d = (flags & OF_FL3DIND) ? ADJ3DPIX : 0;

	if (state & OS_OUTLINED)
		offset = -ADJOUTLPIX - off3d;
	else if (border >= 0)				/* interior or no border */
		offset = -off3d, border = -border;
	else								/* outside border */
		offset = border - off3d;

	if (offset)
		gr_inside(&r, offset);

	/* Adjust for shadow */
	if (state & OS_SHADOWED)
	{
		border = border * ADJSHADPIX - offset + border;
		if (border < 0)
		{
			r.g_w -= border;
			r.g_h -= border;
		}
	}
	r_get(&r, pxcl, pycl, pwcl, phcl);
}


/***************************************************************************/
/*                                                                         */
/*                      COLOR ICON ROUTINES                                */
/*                                                                         */
/***************************************************************************/

static _UWORD get_rgb(_WORD index)
{
	_WORD rindex;						/* reversed bits of index */

	rindex = reverse(index);

	return rgb_tab[rindex];
}



/* MY_TRANS()
 * C code stolen from the apgsxif.s file.  This is call is identical to
 * gsx_trans except that gsx_trans is hardwired to one-plane transforms.
 */
static void my_trans(_WORD *saddr, _UWORD sw, _WORD *daddr, _UWORD dw, _UWORD h, _UWORD nplanes)
{
	MFDB gl_src, gl_dst;
	
	gsx_fix(&gl_src, saddr, sw, h);
	gl_src.fd_stand = TRUE;
	gl_src.fd_nplanes = nplanes;

	gsx_fix(&gl_dst, daddr, dw, h);
	gl_dst.fd_stand = FALSE;
	gl_dst.fd_nplanes = nplanes;
	vr_trnfm(gl_handle, &gl_src, &gl_dst);
}


/* does the transform form but also checks to see if this is pixel-packed
 * mode.  If pixel-packed, then after the transformation, the blacks and
 * whites must be swapped.  Also inverts mask.
 * Special Note for Pixel-Packed mode:  
 *	In pixel-packed mode, VDI no longer expects indexes as color data;
 *	data must be in 5-bit red, 5-bit green (plus one bit), and 5-bit
 *	blue.  So we must take the color indexes and convert them into
 *	this format.  After we expand data in this routine, we do a
 *	transform form on the data.  The result of the transform is that
 *	each word represents a pixel, and the value corresponds to a color
 *	index (0-255, but with reversed bits).  The rgb_tab has the correct
 *	RGB settings per index.  Therefore, we substitute indexes (i.e.
 *	reversed indexes) with RGB.
 */
static void tran_check(_WORD *saddr, _WORD *daddr, int w, int h, int nplanes)
{
	int no_words, i;
	_UWORD *wptr;

	my_trans(saddr, w, daddr, w, h, nplanes);

	if (nplanes == 16)
	{
		no_words = w / 16 * h * nplanes;
		wptr = (_UWORD *)daddr;
		for (i = 0; i < no_words; i++)
		{
			switch (wptr[i])
			{
			case 0x0000:
				wptr[i] = 0xFFFF;
				break;
			case 0xFFFF:
				wptr[i] = 0x0000;
				break;
			default:
				wptr[i] = get_rgb(wptr[i]);
				break;
			}
		}
	}
}


#if 0 /* not used */
/*	Takes a list of icons and returns the first icon that 
 *	has the same number of planes.  Returns a null pointer if no match.
 */
CICON *match_planes(CICON *iconlist, int planes)
{
	CICON *tempicon;

	tempicon = iconlist;
	while (tempicon)
	{
		if (tempicon->num_planes == planes)
			break;
		else
			tempicon = tempicon->next_res;
	}
	return tempicon;
}
#endif


/*	Takes a list of icons and returns the first icon that 
 *	has equal or smaller planes.  Returns a null pointer if no match.
 */
static CICON *find_eq_or_less(CICON *iconlist, int planes)
{
	CICON *tempicon, *lasticon;

	tempicon = iconlist;
	lasticon = NULL;
	while (tempicon)
	{
		if (tempicon->num_planes == planes)
			break;
		if (tempicon->num_planes < planes)
		{
			if (!lasticon || (lasticon->num_planes < tempicon->num_planes))
				lasticon = tempicon;
		}
		tempicon = tempicon->next_res;
	}
	if (tempicon)
		return tempicon;
	return lasticon;
}


/*	Takes a ptr to a mask and copies it to another while dithering
 *	the data.  Note that this does not check the limits of the gl_mask.
 */
static void convert_mask(_WORD *mask, _WORD *dst_mask, _WORD width, _WORD height)
{
	int i, j, wdwide;

	wdwide = (width + 15) >> 4;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < wdwide; j++)
		{
			dst_mask[j + i * wdwide] = mask[j + i * wdwide] & 0x5555;
		}
		++i;
		if (i < height)
		{
			for (j = 0; j < wdwide; j++)
			{
				dst_mask[j + i * wdwide] = mask[j + i * wdwide] & 0xAAAA;
			}
		}
	}
}


/*	Takes the blit code and makes sure that the number of planes is
 *	set.  This code was taken from gsx_blt() and modified so that the
 *	number of planes is passed in and the source and destination MFDB's
 *	had that value set correctly.  Otherwise, it is the same code.
 */
static void gsx_cblt(_WORD *saddr, _UWORD sx, _UWORD sy, _UWORD sw, _WORD *daddr, _UWORD dx, _UWORD dy, _UWORD dw, _UWORD w, _UWORD h, _UWORD rule, _WORD numplanes)
{
	_WORD ppts[8];
	MFDB gl_src, gl_dst;
	
	gsx_fix(&gl_src, saddr, sw, h);
	gsx_fix(&gl_dst, daddr, dw, h);

	gl_src.fd_nplanes = numplanes;
	gl_dst.fd_nplanes = gl_nplanes;

	gsx_moff();

	ppts[0] = sx;
	ppts[1] = sy;
	ppts[2] = sx + w - 1;
	ppts[3] = sy + h - 1;
	ppts[4] = dx;
	ppts[5] = dy;
	ppts[6] = dx + w - 1;
	ppts[7] = dy + h - 1;
	vro_cpyfm(gl_handle, rule, ppts, &gl_src, &gl_dst);
	gsx_mon();
}


/*
 *	Routine to draw a color icon, which is a graphic image with a text
 *	string underneath it.  Note that this routine is very similar to
 *	gr_icon().   It has an extra parameter which is the list of color
 *	icons for different resolutions.
 */
void gr_cicon(_WORD state, _WORD *pmask, _WORD *pdata, const char *ptext, vdi_wchar_t ch, _WORD chx, _WORD chy, GRECT *pi, GRECT *pt, CICONBLK *cicon)
{
	_WORD fgcol, bgcol, tmp;
	/* crack the color/char definition word */
	CICON *color;
	int col_select;						/* is there a color select icon */

	color = find_eq_or_less(cicon->mainlist, gl_nplanes);

	fgcol = (ch >> 12) & 0x000f;
	bgcol = (ch >> 8) & 0x000f;
	ch &= 0x0ff;
	/* invert if selected */
	if (state & OS_SELECTED)
	{
		tmp = fgcol;
		fgcol = bgcol;
		bgcol = tmp;
	}

	col_select = 0;
	/* substitute mask if color avail */
	if (color)
	{
		if ((state & OS_SELECTED) && color->sel_data)
		{
			col_select = 1;
			pdata = color->sel_data;
			pmask = color->sel_mask;
		} else
		{
			pdata = color->col_data;
			pmask = color->col_mask;
		}
		if (state & OS_SELECTED)
		{
			tmp = fgcol;
			fgcol = bgcol;
			bgcol = tmp;
		}
	}

	/* do mask unless its on a white background */
	if (!((state & OS_WHITEBAK) && bgcol == G_WHITE))
	{
		/* for pixel-packed mode, must blit in black (to zero out backgd) */
		if (gl_nplanes == 16 && color)
			gsx_blt(pmask, 0, 0, pi->g_w, ORGADDR, pi->g_x, pi->g_y,
					gl_width, pi->g_w, pi->g_h, MD_TRANS, G_BLACK, fgcol);
		else
			gsx_blt(pmask, 0, 0, pi->g_w, ORGADDR, pi->g_x, pi->g_y,
					gl_width, pi->g_w, pi->g_h, MD_TRANS, bgcol, fgcol);

		/* only print bar if string is not null */
		if (ptext[0])
		{
			if (color && (state & OS_SELECTED))
				gr_rect(fgcol, IP_SOLID, pt);
			else
				gr_rect(bgcol, IP_SOLID, pt);
		}
	}

	/* draw the data */
	if (color)
	{
		/* NOTE: The call below is commented out because it does a transform form
		 * on the color data every time it is drawn.  The code in the color icon 
		 * resource load should do all of the transforms.  Uncomment this line for
		 * testing purposes only.
		 */
		/* my_trans( pdata, pi->g_w, pdata,pi->g_w,pi->g_h ); */

		gsx_cblt(pdata, 0, 0, pi->g_w, ORGADDR, pi->g_x, pi->g_y, gl_width, pi->g_w, pi->g_h, S_OR_D, color->num_planes);
		if (state & OS_SELECTED)
		{
			tmp = fgcol;
			fgcol = bgcol;
			bgcol = tmp;
			if (!col_select)
			{							/* check if we need to darken */
				convert_mask(pmask, gl_colmask, pi->g_w, pi->g_h);
				gsx_blt(gl_colmask, 0, 0, pi->g_w, ORGADDR, pi->g_x, pi->g_y,
						gl_width, pi->g_w, pi->g_h, MD_TRANS, 1, 0);
			}
		}
	} else
	{
		gsx_blt(pdata, 0, 0, pi->g_w, ORGADDR, pi->g_x, pi->g_y,
				gl_width, pi->g_w, pi->g_h, MD_TRANS, fgcol, bgcol);
	}
	
	/* draw the character */
	gsx_attr(TRUE, MD_TRANS, fgcol);
	if (ch)
	{
		gsx_tblt(SMALL, pi->g_x + chx, pi->g_y + chy, &ch, 1);
	}
	/* draw the label */
	gr_gtext(TE_CNTR, SMALL, ptext, pt);
}


/* FIX_MONO()
 * Do fixups on the monochrome icon then pass back a ptr to the next part.
 */
static CICON *fix_mono(CICONBLK *ptr, long *plane_size, int * tot_res)
{
	_WORD width, height;
	long size;

	width = ptr->monoblk.ib_wicon;
	height = ptr->monoblk.ib_hicon;
	/* in the file, first link contains number of CICON structures */
	*tot_res = (int)(intptr_t)(ptr->mainlist);
	/* BUG: works only for width being multiple of 16, should be rounded up */
	*plane_size = size = (long) ((width / 16) * height * 2);
	/* data follows CICONBLK structure */
	ptr->monoblk.ib_pdata = (_WORD *)(ptr + 1);
	/* mask follows data */
	ptr->monoblk.ib_pmask = (_WORD *)((char *)(ptr + 1) + size);
	/* text follows mask */
	ptr->monoblk.ib_ptext = (char *) (ptr + 1) + 2 * size;
	/* icon text for color icons is limited to 12 chars, next CICONBLK structure follows it */
	return (CICON *)(ptr->monoblk.ib_ptext + CICON_STR_SIZE);
}


/* FIX_RES()
 * Does fixups on the resolution dependent color icons.  Returns
 * a pointer past the last icon fixed up.
 */
static CICON *fix_res(CICON *ptr, long mono_size, CICON ***next_res)
{
	char *end;
	_WORD *select;

	*next_res = &ptr->next_res;

	select = ptr->sel_data;
	ptr->col_data = (_WORD *)((char *) ptr + sizeof(CICON));
	ptr->col_mask = (_WORD *)((char *) ptr + sizeof(CICON) + ((long) ptr->num_planes * mono_size));
	end = (char *)ptr->col_mask + mono_size;			/* push pointer past the mask */
	if (select)
	{									/* there are some selected icons */
		ptr->sel_data = (_WORD *)end;
		end = end + ((long) ptr->num_planes * mono_size);
		ptr->sel_mask = (_WORD *)end;
		end = end + mono_size;
	}
	return (CICON *)end;
}


/* FIXUP_CICON()
 * Does fixups on the pointers in the color icon structures.
 */
static void fixup_cicon(CICON * ptr, int tot_icons, CICONBLK ** carray)
{
	int tot_resicons;
	int i, j;
	long mono_size;						/* size of a single mono icon in bytes */
	CICON **next_res;
	CICONBLK *cicon;

	for (i = 0; i < tot_icons; i++)
	{
		cicon = (CICONBLK *) ptr;
		carray[i] = cicon;
		ptr = fix_mono((CICONBLK *) ptr, &mono_size, &tot_resicons);
		if (tot_resicons)
		{
			next_res = NULL; /* quiet compiler */
			cicon->mainlist = ptr;
			for (j = 0; j < tot_resicons; j++)
			{
				ptr = fix_res(ptr, mono_size, &next_res);
				*next_res = ptr;
			}
			*next_res = NULL;
		}
	}
}




/* Takes saddr, the source address for color icon data with splanes of info,
 * and puts it into daddr, expanding it to dplanes.  It handles the expansion
 * cases.
 * DLF 8/31/92
 * Special Note for Pixel-packed mode:
 *	True-color behaves differently for blits.  The mask must be blitted
 *	in black because black is 0.  You can then OR the color data into
 *	black and preserve the colors.  Furthermore, you must make sure that
 *	outside of the mask, there are 0's.  In this routine, we must make
 *	the blacks be 1's (in the same way as 4 and 8 plane), but in
 *	tran_check(), 0x0000's are reversed for 0xffff and vice versa.
 * NOTE:  This has been optimized in  an assembly file.
 */
static void expand_data(_WORD *saddr, _WORD *daddr, _WORD *mask, int splanes, int dplanes, int w, int h)
{
	int plane_size, orig_size, end_size;
	int i, n;
	_WORD *stemp, *dtemp;

	plane_size = ((w + 15) >> 4) * h;
	orig_size = plane_size * splanes;
	end_size = plane_size * dplanes;
	for (i = 0; i < orig_size; i++)		/* copy source */
		daddr[i] = saddr[i];

	for (i = orig_size; i < end_size; i++)	/* fill with 1's */
		daddr[i] = -1;
	/* now "and" splanes into the next plane */
	dtemp = &daddr[orig_size];
	for (n = 0; n < splanes; n++)
	{
		stemp = &saddr[n * plane_size];
		for (i = 0; i < plane_size; i++)
			dtemp[i] = stemp[i] & dtemp[i];
	}
	/* now copy plane into the rest of the planes */
	stemp = &daddr[orig_size];
	for (n = 0; n < (dplanes - splanes - 1); n++)
	{
		dtemp = &daddr[orig_size + ((n + 1) * plane_size)];
		for (i = 0; i < plane_size; i++)
			dtemp[i] = stemp[i];
	}
	/* Now "and" the mask into all planes */
	for (n = 0; n < dplanes; n++)
	{
		dtemp = &daddr[n * plane_size];
		/* We special-case pixel-packed because we need to make the outside
		 * of the mask all 1's, since later on in tran_check(), all 1's
		 * become 0's (which is black in pixel packed mode ).
		 */
		if (dplanes == 16)
		{
			for (i = 0; i < plane_size; i++)
				dtemp[i] = ~mask[i] | dtemp[i];
		} else
		{
			for (i = 0; i < plane_size; i++)
				dtemp[i] = mask[i] & dtemp[i];
		}
	}
}


/* TRANS_CICON()
 * Takes an array of color icons (and the number of icons), and changes
 * all of the color data from device-independent form to device-dependent.
 * If there is no match for
 * the current resolution, it will create one out of the next smallest icon.
 * In either case, the memory will be malloced so that we can avoid the
 * transform form in place.  Must deallocate at a later time.
 * The data is put in a buffer of the same size or larger (if no match for
 * current resolution).  
 */
static void trans_cicon(int tot_icons, CICONBLK **carray)
{
	int i;
	CICONBLK *ciconblk;
	CICON *ctemp;
	int w, h;
	_WORD *databuffer, *selbuffer, *tempbuffer;
	int32_t tot_size;

	for (i = 0; i < tot_icons; i++)
	{
		ciconblk = carray[i];
		w = ciconblk->monoblk.ib_wicon;
		h = ciconblk->monoblk.ib_hicon;
		ctemp = find_eq_or_less(ciconblk->mainlist, gl_nplanes);

		ciconblk->mainlist = ctemp;
		if (ctemp)
		{
			tot_size = w / 8 * h * gl_nplanes;

			/* if not same size, allocate bigger buffers and expand */
			if (ctemp->num_planes != gl_nplanes)
			{
				tempbuffer = (_WORD *) dos_alloc_anyram(tot_size);
				databuffer = (_WORD *) dos_alloc_anyram(tot_size);
				if (!tempbuffer || !databuffer)
				{
					ciconblk->mainlist = NULL;
					return;				/* just quit */
				}
				expand_data(ctemp->col_data, tempbuffer, ctemp->col_mask, ctemp->num_planes, gl_nplanes, w, h);
				tran_check(tempbuffer, databuffer, w, h, gl_nplanes);
				ctemp->col_data = databuffer;
				if (ctemp->sel_data)
				{
					selbuffer = (_WORD *) dos_alloc_anyram(tot_size);
					if (selbuffer)
					{
						expand_data(ctemp->sel_data, tempbuffer, ctemp->sel_mask, ctemp->num_planes, gl_nplanes, w, h);
						tran_check(tempbuffer, selbuffer, w, h, gl_nplanes);
					}
					ctemp->sel_data = selbuffer;
				}
				dos_free(tempbuffer);
			} else
			{							/* just allocate same size, copy over because */
				/* we don't want to transform form in place */
				databuffer = (_WORD *) dos_alloc_anyram(tot_size);
				if (!databuffer)
				{
					ciconblk->mainlist = NULL;
					return;
				}
				my_trans(ctemp->col_data, w, databuffer, w, h, gl_nplanes);
				ctemp->col_data = databuffer;
				if (ctemp->sel_data)
				{
					selbuffer = (_WORD *) dos_alloc_anyram(tot_size);
					if (selbuffer)
					{
						my_trans(ctemp->sel_data, w, selbuffer, w, h, gl_nplanes);
					}
					ctemp->sel_data = selbuffer;
				}
			}

			ctemp->num_planes = gl_nplanes;
			ctemp->next_res = NULL;		/* won't need the rest of the list */
		}
	}
}


/* GET_COLOR_RSC()
 * This is the main routine that fixes up the color icons.
 * Pass in a pointer to the array of CICONBLK's.  Count the number of 
 * entries (up until an entry has a -1L) to get the number of CICONBLK's.
 */

void get_color_rsc(CICONBLK **cicondata)
{
	CICON *ptr;
	CICONBLK **array_ptr;
	int totalicons;

	array_ptr = cicondata;
	totalicons = 0;
	while (*array_ptr++ != (CICONBLK *)-1L)
		totalicons++;

	ptr = (CICON *) array_ptr;

	fixup_cicon(ptr, totalicons, (CICONBLK **) cicondata);	/* fixup pointers */
	trans_cicon(totalicons, (CICONBLK **) cicondata);	/* transform form */
}




/* Frees extra memory allocated when fixing up the icons.  There should
 * be freeable memory for every icon, since fixup even mallocs memory
 * for non-expanded data (so that we avoid the transform form in place)
 */
void free_cicon(CICONBLK **carray)
{
	int i;
	CICONBLK *ciconblk, **ptr;
	CICON *ctemp;
	_WORD tot_icons;

	ptr = carray;
	tot_icons = 0;
	while (*ptr++ != (CICONBLK *)-1L)
		tot_icons++;

	for (i = 0; i < tot_icons; i++)
	{
		ciconblk = carray[i];
		ctemp = ciconblk->mainlist;
		if (ctemp)
		{
			dos_free(ctemp->col_data);
			if (ctemp->sel_data)
				dos_free(ctemp->sel_data);
		}
	}
}
