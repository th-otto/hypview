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
 * split argument line according to Drag&Drop quoting rules.
 * Can also be used for parameters send via AV protocol
 */
char **split_av_parameter(char *start)
{
	_BOOL in_quote = FALSE;
	int argc = 0;
	char **argv = NULL;
	char *ptr;
	
	if (start == NULL)
		return NULL;
	ptr = start;
	start = NULL;
	while (*ptr != '\0')
	{
		if (*ptr == ' ')
		{
			if (!in_quote)
			{
				*ptr++ = '\0';
				while (*ptr == ' ')
					ptr++;
				if (start)
				{
					argv = g_renew(char *, argv, argc + 2);
					argv[argc++] = g_strdup(start);
				}
				start = NULL;
			} else
			{
				ptr++;
			}
		} else if (*ptr == '\'')
		{
			memmove(ptr, ptr + 1, strlen(ptr + 1) + 1);
			if (!in_quote)
			{
				if (start)
				{
					argv = g_renew(char *, argv, argc + 2);
					argv[argc++] = g_strdup(start);
				}
				in_quote = TRUE;
				start = ptr;
			} else if (*ptr != '\'')
			{
				in_quote = FALSE;
				if (*ptr != '\0')
					*ptr++ = '\0';
				if (start)
				{
					argv = g_renew(char *, argv, argc + 2);
					argv[argc++] = g_strdup(start);
				}
				start = NULL;
			} else
			{
				ptr++;
			}
		} else
		{
			if (start == NULL)
				start = ptr;
			ptr++;
		}
	}
	if (start)
	{
		if (in_quote)
		{
			/* unterminated quoted argument */
		}
		argv = g_renew(char *, argv, argc + 2);
		argv[argc++] = g_strdup(start);
	} else
	{
		argv = g_renew(char *, argv, argc + 1);
	}
	argv[argc] = NULL;
	return argv;
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
