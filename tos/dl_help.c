/*
 * $Id: dl_help.c,v 1.3 2006/02/12 15:34:22 pdonze Exp $
 * 
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
#include "av.h"
#include <ctype.h>


_WORD help_viewer_id(void)
{
	_WORD viewer;
	
	/*  Look for environment variable which points to the help viewer   */
	viewer = appl_locate(getenv("HELPVIEWER"), FALSE);

	/*
	 * ACC search: ST-Guide, 1stGuide and 1stView in this
	 * order
	 */
	if (viewer < 0)
		viewer = appl_locate("HYPVIEW;HYP_VIEW;ST-GUIDE;HELPACC;1STGUIDE;1STVIEW", FALSE);
	return viewer;
}

