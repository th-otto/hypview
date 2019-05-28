/*		GEMOBED.C		05/29/84 - 06/20/85 			Gregg Morris	*/
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


#undef DELETE /* clashes with Win32 */

#define BACKSPACE 0x0E08                /* backspace        */
#define SPACE 0x3920                    /* ASCII <space>    */
#define UP 0x4800                       /* up arrow         */
#define DOWN 0x5000                     /* down arrow       */
#define ARROW_LEFT 0x4B00               /* left arrow       */
#define ARROW_RIGHT 0x4D00              /* right arrow      */
#define DELETE 0x537f                   /* keypad delete    */
#define TAB 0x0F09                      /* tab              */
#define BACKTAB 0x0F00                  /* backtab          */
#define RETURN 0x1C0D                   /* carriage return  */
#define ENTER 0x720D                    /* enter key on keypad  */
#define ESCAPE 0x011B                   /* escape           */

#define BYTESPACE 0x20                  /* ascii space in bytes */

static TEDINFO edblk;




static void ob_getsp(OBJECT *tree, _WORD obj, TEDINFO *pted)
{
	OBSPEC spec;

	spec = tree[obj].ob_spec;
	if (tree[obj].ob_flags & OF_INDIRECT)
		spec = *(spec.indirect);
	*pted = *(spec.tedinfo);	/* return TEDINFO */
}



/*
 * AES #54 - form_center - Centre an object on the screen.
 */
void ob_center(OBJECT *tree, GRECT *pt)
{
	_WORD xd, yd, wd, hd;
	_WORD iword, th;
	OBSPEC spec;
	GRECT rec;

	wd = tree[ROOT].ob_width;
	hd = tree[ROOT].ob_height;
	xd = (gl_width - wd) / 2;
	yd = gl_hbox + ((gl_height - gl_hbox - hd) / 2);
	tree[ROOT].ob_x = xd;
	tree[ROOT].ob_y = yd;

	/* account for outline */
	if (tree[ROOT].ob_state & OS_OUTLINED)
	{
		/* don't go off the screen */
		xd -= (xd >= 3) ? 3 : xd;
		yd -= (yd >= 3) ? 3 : yd;
		wd += 6;
		hd += 6;
	}

	/* account for shadow */
	if (tree[ROOT].ob_state & OS_SHADOWED)
	{
		ob_sst(tree, ROOT, &spec, &iword, &iword, &iword, &rec, &th);
		th = (th > 0) ? th : -th;
		th = 2 * th;
		wd += th;
		hd += th;
	}

	r_set(pt, xd, yd, wd, hd);
}


/*
 *	Routine to scan thru a string looking for the occurrence of
 *	the specified character.  IDX is updated as we go based on
 *	the '_' characters that are encountered.  The reason for
 *	this routine is so that if a user types a template character
 *	during field entry the cursor will jump to the first 
 *	raw string underscore after that character.
 */
static _WORD scan_to_end(const char *pstr, _WORD idx, char chr)
{
	while (*pstr && *pstr != chr)
	{
		if (*pstr++ == '_')
			idx++;
	}

	return idx;
}


/*
 *	Routine to insert a character in a string by
 */
static void ins_char(char *str, _WORD pos, char chr, _WORD tot_len)
{
	_WORD ii, len;

	len = strlen(str);

	for (ii = len; ii > pos; ii--)
		str[ii] = str[ii - 1];
	str[ii] = chr;
	if (len + 1 < tot_len)
		str[len + 1] = '\0';
	else
		str[tot_len - 1] = '\0';
}


/*
 *	Routine that returns a format/template string relative number
 *	for the position that was input (in raw string relative numbers).
 *	The returned position will always be right before an '_'.
 */
static _WORD find_pos(const char *str, _WORD pos)
{
	_WORD i;

	for (i = 0; pos > 0; i++)
	{
		if (str[i] == '_')
			pos--;
	}

	/* skip to first one */
	while (str[i] && str[i] != '_')
		i++;

	/*
	 * Here we may have come to the end of the string without finding a field.
	 * Backup to the last position where there was one and advance one
	 * position past it...
	 */
	if (str[i] == 0)
	{
		while (str[i] != '_' && i >= 0)
			i--;
		if (str[i])
			i++;
	}
	return i;
}


static void pxl_rect(OBJECT *tree, _WORD obj, _WORD ch_pos, GRECT *pt)
{
	GRECT o;
	vdi_wchar_t wtext[MAX_LEN];
	
	ob_actxywh(tree, obj, &o);
	gr_just(edblk.te_just, edblk.te_font, edblk.te_ptmplt, o.g_w, o.g_h, &o, wtext);

	pt->g_x = o.g_x + (ch_pos * gl_wchar);
	pt->g_y = o.g_y;
	pt->g_w = gl_wchar;
	pt->g_h = gl_hchar;
}


/*
 *	Routine to redraw the cursor or the field being edited.
 */
static void curfld(OBJECT *tree, _WORD obj, _WORD new_pos, _WORD dist)
{
	GRECT oc, t;

	pxl_rect(tree, obj, new_pos, &t);
	if (dist)
	{
		/* the "+1" is necessary or the cursor isn't always redrawn properly */
		t.g_w += (dist - 1) * gl_wchar + 1;
	} else
	{
		gsx_attr(FALSE, MD_XOR, G_BLACK);
		t.g_y -= 3;
		t.g_h += 6;
	}

	/* set the new clip rect */
	gsx_gclip(&oc);
	gsx_sclip(&t);

	/* redraw the field */
	if (dist)
		ob_draw(tree, obj, 0);
	else
		gsx_cline(t.g_x, t.g_y, t.g_x, t.g_y + t.g_h - 1);

	/* turn on cursor in new position */
	gsx_sclip(&oc);
}


/*
 *	Routine to check to see if given character is in the desired range.
 *	The character ranges are stored as enumerated characters (xyz) or
 *	ranges (x..z)
 */
static _BOOL instr(unsigned char chr, const char *str)
{
	unsigned char test1, test2;

	while (*str)
	{
		test1 = test2 = *str++;
		if (*str == '.' && *(str + 1) == '.')
		{
			str += 2;
			test2 = *str++;
		}
		if (chr >= test1 && chr <= test2)
			return TRUE;
	}

	return FALSE;
}


/*
 *	Routine to verify that the character matches the validation
 *	string.  If necessary, upshift it.
 */
static _BOOL check(char *in_char, unsigned char valchar)
{
	_WORD upcase;
	_WORD rstr;

	upcase = TRUE;
	rstr = -1;
	switch (valchar)
	{
	case '9':							/* 0..9 */
		rstr = ST9VAL;
		upcase = FALSE;
		break;
	case 'A':							/* A..Z, <space> */
		rstr = STAVAL;
		break;
	case 'N':							/* 0..9, A..Z, <SPACE> */
		rstr = STNVAL;
		break;
	case 'P':							/* DOS pathname + '\', '?', '*', ':', '.', ',' */
		rstr = STPVAL;
		break;
	case 'p':							/* DOS pathname + '\` + ':' */
		rstr = STLPVAL;
		break;
	case 'F':							/* DOS filename + ':', '?' + '*'    */
		rstr = STFVAL;
		break;
	case 'f':							/* DOS filename */
		rstr = STLFVAL;
		break;
	case 'a':							/* a..z, A..Z, <SPACE> */
		rstr = STLAVAL;
		upcase = FALSE;
		break;
	case 'n':							/* 0..9, a..z, A..Z, <SPACE> */
		rstr = STLNVAL;
		upcase = FALSE;
		break;
	case 'x':							/* anything, but upcase */
		*in_char = aes_toupper(*in_char);
		return TRUE;
	case 'X':							/* anything */
		return TRUE;
	}
	if (rstr >= 0)
	{
		if (instr(*in_char, rs_str(rstr)))
		{
			if (upcase)
				*in_char = aes_toupper(*in_char);
			return TRUE;
		}
	}

	return FALSE;
}


/*
 *	Find STart and FiNish of a raw string relative to the template
 *	string.  The start is determined by the InDeX position given.
 */
static void ob_stfn(_WORD idx, _WORD *Pstart, _WORD *pfinish)
{
	*Pstart = find_pos(D.g_tmpstr, idx);
	*pfinish = find_pos(D.g_tmpstr, strlen(D.g_rawstr));
}


static _WORD ob_delit(_WORD idx)
{
	if (D.g_rawstr[idx])
	{
		strcpy(&D.g_rawstr[idx], &D.g_rawstr[idx + 1]);
		return FALSE;
	}
	return TRUE;
}


/*
 * AES #46 - objc_edit - Edit text in an editable text object.
 */
_WORD ob_edit(OBJECT *tree, _WORD obj, _WORD in_char, _WORD *idx, _WORD kind)
{
	_WORD tmp_back, cur_pos;
	_WORD pos, len, dist;
	_WORD ii, no_redraw, start;
	_WORD finish, nstart, nfinish;
	char bin_char;

	if (kind == EDSTART || obj <= 0)
		return TRUE;

	/* copy TEDINFO struct to local struct */
	ob_getsp(tree, obj, &edblk);

	/* copy passed in strings to local strings */
	strmaxcpy(D.g_tmpstr, sizeof(D.g_tmpstr), edblk.te_ptmplt);
	strmaxcpy(D.g_rawstr, sizeof(D.g_rawstr), edblk.te_ptext);
	len = ii = strmaxcpy(D.g_valstr, sizeof(D.g_valstr), edblk.te_pvalid);

	/* expand out valid str */
	while (ii > 0 && len < edblk.te_tmplen)
		D.g_valstr[len++] = D.g_valstr[ii - 1];
	D.g_valstr[len] = '\0';

	/* init formatted string */
	ob_format(edblk.te_just, D.g_rawstr, D.g_tmpstr, D.g_fmtstr);

	switch (kind)
	{
	case EDINIT:
		*idx = strlen(D.g_rawstr);
		break;
	case EDCHAR:
		/*
		 * at this point, D.g_fmtstr has already been formatted -- it has
		 * both template & data.  now update D.g_fmtstr with in_char;
		 * return it; strip out junk & update ptext string.
		 */
		no_redraw = TRUE;

		/* find cursor & turn it off */
		ob_stfn(*idx, &start, &finish);
		/* turn cursor off */
		cur_pos = start;
		curfld(tree, obj, cur_pos, 0);

		switch (in_char)
		{
		case BACKSPACE:
			if (*idx > 0)
			{
				*idx -= 1;
				no_redraw = ob_delit(*idx);
			}
			break;
		case ESCAPE:
			*idx = 0;
			D.g_rawstr[0] = '\0';
			no_redraw = FALSE;
			break;
		case DELETE:
			if (*idx <= (edblk.te_txtlen - 2))
				no_redraw = ob_delit(*idx);
			break;
		case ARROW_LEFT:
			if (*idx > 0)
				*idx -= 1;
			break;
		case ARROW_RIGHT:
			if (*idx < (int)strlen(D.g_rawstr))
				*idx += 1;
			break;
		default:
			tmp_back = FALSE;
			if (*idx > (edblk.te_txtlen - 2))
			{
				cur_pos--;
				start = cur_pos;
				tmp_back = TRUE;
				*idx -= 1;
			}
			bin_char = in_char & 0x00ff;

			if (bin_char)
			{
				/* make sure char is in specified set */
				if (check(&bin_char, D.g_valstr[*idx]))
				{
					ins_char(D.g_rawstr, *idx, bin_char, edblk.te_txtlen);
					*idx += 1;
					no_redraw = FALSE;
				} else
				{
					/* see if we can skip ahead */
					if (tmp_back)
					{
						*idx += 1;
						cur_pos++;
					}
					pos = scan_to_end(D.g_tmpstr + cur_pos, *idx, bin_char);
					if (pos < (edblk.te_txtlen - 2))
					{
						memset(&D.g_rawstr[*idx], ' ', pos - *idx);
						D.g_rawstr[pos] = '\0';
						*idx = pos;
						no_redraw = FALSE;
					}
				}
			}
			break;
		}

		strcpy(edblk.te_ptext, D.g_rawstr);
		if (!no_redraw)
		{
			ob_format(edblk.te_just, D.g_rawstr, D.g_tmpstr, D.g_fmtstr);
			ob_stfn(*idx, &nstart, &nfinish);
			start = min(start, nstart);
			dist = max(finish, nfinish) - start;
			if (dist)
				curfld(tree, obj, start, dist);
		}
		break;
	case EDEND:
		break;
	}

	/* draw/erase the cursor */
	cur_pos = find_pos(D.g_tmpstr, *idx);
	curfld(tree, obj, cur_pos, 0);

	return TRUE;
}
