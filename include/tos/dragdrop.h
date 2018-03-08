/*
 * $Id: dragdrop.h,v 1.3 2009/08/31 16:14:35 alanh Exp $
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

/*
	Tabulatorweite: 8
	Kommentare ab: Spalte 			*Spalte 50*
*/
#ifndef _DRAGDROP_H_
#define _DRAGDROP_H_

#ifndef AP_DRAGDROP
#define	AP_DRAGDROP	63
#endif

short	ddcreate( short	app_id, short rcvr_id, short window, short mx, short my, short kbstate, unsigned long format[8], void **oldpipesig );
short	ddstry( short handle, unsigned long format, char *name, long size );
short	ddopen( char *pipe, unsigned long *format, void **oldpipesig );
void	ddclose( short handle, void *oldpipesig );
short	ddrtry( short handle, char *name, unsigned long *format, long *size );
short	ddreply( short handle, short msg );

#endif
