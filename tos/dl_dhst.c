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
#include "hypview.h"
#include <mint/cookie.h>
#include "dhst.h"
#include "tos/mem.h"

static DHSTINFO *dhst_info;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#if USE_DOCUMENTHISTORY

void DhstAddFile(const char *path)
{
	_LONG value;

	if (Cookie_ReadJar(C_DHST /*'DHST' */, &value))
	{
		_WORD msg[8];
		void *ret;
		DHSTINFO *info;
		char *p;
		
		g_free_shared(dhst_info);
		dhst_info = NULL;
		ret = g_alloc_shared(sizeof(DHSTINFO) + strlen(gl_program_name) + DL_PATHMAX * 2);
		if (ret == NULL)
		{
			form_alert(1, rs_string(DI_MEMORY_ERROR));
			return;
		}
		info = (DHSTINFO *) ret;
		p = (char *) ret + sizeof(DHSTINFO);
		strcpy(p, gl_program_name);
		info->appname = p;
		p += strlen(p) + 1;
		info->apppath = p;

		if (!shel_read(p, p + DL_PATHMAX))
		{
			g_free_shared(info);
			return;
		}
		p += strlen(p) + 1;
		info->docpath = p;
		strcpy(p, path);
		info->docname = hyp_basename(info->docpath);

		msg[0] = DHST_ADD;
		msg[1] = gl_apid;
		msg[2] = 0;
		*(DHSTINFO **) (&msg[3]) = info;
		msg[5] = 0;
		msg[6] = 0;
		msg[7] = 0;
		appl_write((short) value, 16, &msg[0]);
		dhst_info = info;
	}
}

/*** ---------------------------------------------------------------------- ***/

void DhstFree(_WORD msg[8])
{
	DHSTINFO *info = *(DHSTINFO **)&msg[3];
	
	if (info == dhst_info)
	{
		g_free_shared(info);
		dhst_info = NULL;
	}
}

#endif
