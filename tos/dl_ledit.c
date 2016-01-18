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
#include <mint/cookie.h>
#include "magx.h"

#if USE_LONGEDITFIELDS

static short magic_version = -1;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void InitScrollTED(OBJECT *obj, XTED *xted, short txt_len)
{
	TEDINFO *tedinfo = obj->ob_spec.tedinfo;
	char *src, *dst, *txt, *tmplt;
	short tmplt_len = 0;
	short i;

	if (magic_version == -1)			/* MagiC available ? */
	{
		txt = g_new(char, (txt_len << 1) + tmplt_len + 2);
		if (txt == NULL)
		{
			form_alert(1, rs_string(DI_MEMORY_ERROR));
			return;
		}
		*txt = 0;
		tedinfo->te_ptext = txt;
		tedinfo->te_txtlen = txt_len + 1;
		return;
	} else if (magic_version < 0x520)
	{
		txt_len = min(128, txt_len);
	}
	
	if (tedinfo->te_ptmplt == NULL)		/* already initialized? */
		return;

	src = tedinfo->te_ptmplt;
	while (*src)
	{
		if (*src != '_')
			tmplt_len++;
		src++;
	}

	txt = g_new(char, (txt_len << 1) + tmplt_len + 2);
	if (txt == NULL)
	{
		form_alert(1, rs_string(DI_MEMORY_ERROR));
		return;
	}
	*txt = 0;
	tmplt = &txt[txt_len + 1];
	src = tedinfo->te_ptmplt;
	dst = tmplt;
	i = txt_len;
	while (*src)
	{
		if (*src == '_')
		{
			do
			{
				src++;
			} while (*src == '_');
			for (; i > 0; i--)
				*dst++ = '_';
		} else
			*dst++ = *src++;
	}
	*dst = 0;

	xted->xte_ptmplt = tmplt;
	xted->xte_pvalid = tedinfo->te_pvalid;
	xted->xte_vislen = tedinfo->te_tmplen - 1;
	xted->xte_scroll = 0;
	tedinfo->te_ptext = txt;
	tedinfo->te_txtlen = txt_len + 1;
	tedinfo->te_ptmplt = NULL;
	tedinfo->te_tmplen = tmplt_len + txt_len + 1;

	tedinfo->te_just = TE_LEFT;			/* important! */
	tedinfo->te_pvalid = (void *) xted;
}

/*** ---------------------------------------------------------------------- ***/

void DoInitLongEdit(void)
{
	short i;
	MAGX_COOKIE *cookie;
	OBJECT *tree;
	
	if (Cookie_ReadJar(C_MagX, (long *) &cookie))
		magic_version = cookie->aesvars->version;

	for (i = 0; i < long_edit_count; i++)
	{
		hyp_view_rsc_gaddr(R_TREE, long_edit[i].tree, &tree);
		InitScrollTED(&tree[long_edit[i].obj], &long_edit[i].xted, long_edit[i].len);
	}
}

/*** ---------------------------------------------------------------------- ***/

void DoExitLongEdit(void)
{
	short i;

	for (i = 0; i < long_edit_count; i++)
		g_free(long_edit[i].xted.xte_ptmplt);
}

#endif
