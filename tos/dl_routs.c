/*
 * HypView - (c)      - 2006 Philipp Donze
 *               2006 -      Philipp Donze & Odd Skancke
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hv_defs.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void ConvertKeypress(_WORD *key, _WORD *kstate)
{
	short ascii = *key & 0xff;
	short scan = (*key >> 8) & 0x7f;	/* scancode always < 128 */

	if (!scan)							/* if no scancode..., */
		return;							/* nothing to do */

	if ((scan >= KbAlt1) && (scan <= 0x83))	/* Alt+numpad key... */
		scan -= 0x76;					/* convert to regular key */

	if (((scan >= 99) && (scan <= 114))	/* numpad keys */
		|| (scan == 74) || (scan == 78))
		*kstate |= KbNUM;

	if ((scan >= 115) && (scan <= 119))	/* ctrl+combinations... */
	{
		*kstate |= KbCTRL;
		switch (scan)
		{
		case 115:
			scan = KbLEFT;
			break;
		case 116:
			scan = KbRIGHT;
			break;
		case 117:						/* CTRL + END */
			break;
		case 118:						/* CTRL + PAGE DOWN */
			break;
		case 119:
			scan = KbHOME;
			break;
		}
	}

	if (*kstate & (KbCTRL | KbALT))		/* CTRL and ALT combinations */
	{
		if (ascii < 32)
			ascii = ((const char *) key_table->caps)[scan];	/* convert to uppercase */
	}

	*key = (scan << 8) + ascii;
}

/*** ---------------------------------------------------------------------- ***/

void CopyMaximumChars(OBJECT * obj, char *str)
{
	short max_size = obj->ob_spec.tedinfo->te_txtlen - 1;

	strncpy(obj->ob_spec.tedinfo->te_ptext, str, max_size);
	obj->ob_spec.tedinfo->te_ptext[max_size] = 0;
}

/*** ---------------------------------------------------------------------- ***/

/*
    Modifies <start> to point to the first parameter, and returns
    pointer to next parameter.
    To handle all parameters, do something like:

	{
		char *next, *ptr = data;
		do
		{
			next = ParseData(ptr);
			DoSomething(ptr);
			ptr = next;
		} while (*next);
	}
*/
char *ParseData(char *start)
{
	_BOOL in_quote = FALSE;
	_BOOL more = FALSE;

	while (*start)
	{
		if (*start == ' ')
		{
			if (!in_quote)
			{
				*start = 0;
				more = TRUE;
			} else
				start++;
		} else if (*start == '\'')
		{
			memmove(start, start + 1, strlen(start + 1) + 1);
			if (*start == '\'')
				start++;
			else
				in_quote = !in_quote;
		} else
			start++;
	}
	if (more)
		return start + 1;
	else
		return start;
}

/*** ---------------------------------------------------------------------- ***/

short rc_intersect_my(GRECT *p1, GRECT *p2)
{
	short tx, ty, tw, th;

	tw = min((p2->g_x + p2->g_w), (p1->g_x + p1->g_w));
	th = min(p2->g_y + p2->g_h, p1->g_y + p1->g_h);
	tx = max(p2->g_x, p1->g_x);
	ty = max(p2->g_y, p1->g_y);
	p2->g_x = tx;
	p2->g_y = ty;
	p2->g_w = tw - tx;
	p2->g_h = th - ty;
	return ((tw > tx) && (th > ty));
}
