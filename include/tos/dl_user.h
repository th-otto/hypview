/*
 * HypView - (c) 2001 - 2006 Philipp Donze
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

#ifndef _dl_user_h
#define _dl_user_h
/********************************************
 *   begin options                          *
 ********************************************/
/* Use a global VDI workstation? */
#define	USE_GLOBAL_VDI			1
/*	Save color palette on init?	*/
#define	SAVE_COLORS				0

/* Install/remove menubar on initialization? (Needs a resource object MENU) */
#define	USE_MENU		0

/* Use window dialog management routines? (Based on WDialog) */
#define	USE_DIALOG		1

/* Automatically set the title when iconifying according to RSC file */
#define	SET_ICONIFY_NAME	0
/* Open a separate VDI workstation for each new window */
#define	OPEN_VDI_WORKSTATION	0
/* Use a logical raster for computations (=not the pixel raster) */
#define	USE_LOGICALRASTER	1
/* Use toolbar routines */
#define	USE_TOOLBAR		1

/* Support Drag&Drop protokol */
#define	USE_DRAGDROP        1

/* Enable support for long edit fields (As in MagiC)	*/
#define	USE_LONGEDITFIELDS  0

/* BubbleGEM help system */
#define	USE_BUBBLEGEM       0

/* Document history protocol */
#define	USE_DOCUMENTHISTORY	1

/* Application name in uppercase letters */
#define	PROGRAM_UNAME		"HYPVIEW"

/* Number of supported Drag&Drop formats */
#define	MAX_DDFORMAT		8

/* Maximum number of simultaneous iconified windows */
#define	MAX_ICONIFY_PLACE	16
/* Maximum number of recursion levels for modal dialogs */
#define	MAX_MODALRECURSION	10


/* event_multi parameters	*/
#define EVENTS		MU_MESAG|MU_KEYBD|MU_BUTTON
#define MBCLICKS	2|0x0100
#define MBMASK		3
#define MBSTATE		0
#define MBLOCK1		0, 0, 0, 0, 0
#define MBLOCK2		0, 0, 0, 0, 0
#define WAIT		0

#if !OPEN_VDI_WORKSTATION && !USE_GLOBAL_VDI
#error "Need at least one VDI workstation! (USE_GLOBAL_VDI or OPEN_VDI_WORKSTATION)"
#endif

/********************************************
 *   end options                            *
 ********************************************/
#endif     /* _dl_user_h */
