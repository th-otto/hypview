/*      GEMLIB.H        03/15/84 - 06/07/85     Lee Lorenzen            */
/*      EVNTLIB.H       03/15/84 - 05/16/84     Lee Lorenzen            */
/*      2.0             10/30/85 - 10/30/85     Lowell Webster          */
/*      3.0             06/19/86 - 6/25/86      MDF                     */

/*
 *       Copyright 1999, Caldera Thin Clients, Inc.
 *                 2002-2016 The EmuTOS development team
 *
 *       This software is licenced under the GNU Public License.
 *       Please see LICENSE.TXT for further information.
 *
 *                  Historical Copyright
 *       -------------------------------------------------------------
 *       GEM Application Environment Services              Version 2.3
 *       Serial No.  XXXX-0000-654321              All Rights Reserved
 *       Copyright (C) 1986                      Digital Research Inc.
 *       -------------------------------------------------------------
 */

#ifndef GEMLIB_H
#define GEMLIB_H 1

#include "dta.h"

#define NUM_ANODES 32				/* # of appl. nodes	*/
#define PATH_LEN 121				/* define maximum path length */
#define SAVE_ATARI 128				/* atari specific bytes in DESKTOP.INF file size of DESKTOP.INF	file */

#define EXTENSION  6

/* size of DESKTOP.INF file */
#define SIZE_AFILE ((NUM_ANODES*PATH_LEN)+(NUM_ANODES*EXTENSION)+SAVE_ATARI )	
						
typedef struct moblk
{
    _WORD m_out;
    GRECT m_gr;
} MOBLK;


#define MU_SDMSG 0x0040
#define MU_MUTEX 0x0080


/*	FSELLIB.H	05/05/84 - 01/07/85	Lee Lorenzen		*/

#define RMODE_RD 0
#define RMODE_WR 1
#define RMODE_RW 2

/*	WINDLIB.H	05/05/84 - 01/26/85	Lee Lorenzen		*/

#define WS_FULL 0
#define WS_CURR 1
#define WS_PREV 2
#define WS_WORK 3
#define WS_TRUE 4



#define	TPARTS	(NAME|CLOSER|FULLER|MOVER)
#define	VPARTS	(UPARROW|DNARROW|VSLIDE)
#define	HPARTS	(LFARROW|RTARROW|HSLIDE)


#define	HASHSIZ	8		/* size of hash table */
#define	NUMRECT	80		/* # RLISTs per block of memory allocated */
#define	SHADOW	2		/* thickness of drop shadow in pixels */


/* Bit masks of pieces of a window */
#define TOP		0x0001
#define LEFT	0x0002
#define RIGHT	0x0004
#define BOTTOM	0x0008


/* Indices into object tree of window */
#define	W_BOX	    0		/* background of window */
#define	W_TITLE	    1		/* parent of closer, name and fuller */
#define	W_CLOSER    2		/* closer */
#define	W_NAME	    3		/* name and mover bar */
#define	W_FULLER    4		/* fuller */
#define	W_INFO	    5		/* info line */
#define	W_DATA	    6		/* holds remaining window elements */
#define	W_WORK	    7		/* application work area */
#define	W_SIZER	    8		/* sizer */
#define	W_VBAR	    9		/* holds vertical slider elements */
#define	W_UPARROW   10		/* vertical slider up arrow */
#define	W_DNARROW   11		/* vertical slider down arrow */
#define	W_VSLIDE    12		/* vertical slider background */
#define	W_VELEV	    13		/* vertical slider thumb/elevator */
#define	W_HBAR	    14		/* holds horizontal slider elements */
#define	W_LFARROW   15		/* horizontal slider left arrow */
#define	W_RTARROW   16		/* horizontal slider right arrow */
#define	W_HSLIDE    17		/* horizontal slider background */
#define	W_HELEV	    18		/* horizontal slider thumb/elevator */
#define	W_MENUBAR	19		/* menu bar (added Jul 23 91 - ml.) */

#if AESVERSION >= 0x330
#define	NUM_ELEM	20		/* maximum number of objects in a window */
#else
#define	NUM_ELEM	19		/* maximum number of objects in a window */
#endif


typedef struct sh_struct
{
    _WORD sh_doexec;            /* TRUE during normal processing; set */
                                /*  to FALSE to shutdown the system   */
    _WORD sh_dodef;             /* if TRUE then run the default startup   */
                                /*  app: normally EMUDESK, but can be an  */
                                /*  autorun program if so configured.     */
                                /*  if FALSE, run a normal application.   */
    _WORD sh_isdef;             /* if TRUE then using the default startup   */
                                /*  app: normally EMUDESK, but can be an    */
                                /*  autorun program if so configured.       */
                                /*  if FALSE, running a normal application. */
    _WORD sh_isgem;             /* TRUE if the application to be run is a GEM */
                                /*  application; FALSE if character-mode      */
    char sh_desk[LEN_ZFNAME];   /* the name of the default startup app */
    char sh_cdir[LEN_ZPATH];    /* the current directory for the default startup app */
} SHELL;


/* Callers of totop() */
#define	WMOPEN	0		/* called from wm_open() */
#define	WMCLOSE	1		/* called from wm_close() */
#define	WMSET	2		/* called from wm_set() */


/* Memory descriptor structure */
typedef	struct	memhdr {
    int	    numused;		/* number of window structures used */
    struct  memhdr *mnext;	/* pointer to next memory descriptor */
} MEMHDR;


/* Rectangle list structure */
typedef	struct	rlist {
    struct {
	unsigned rused : 1;	/* bit 0 ->1: slot is currently used */
    } rstatus;
    GRECT   rect;		/* the rectangle dimensions */
    MEMHDR  *rwhere;		/* ptr to memory this RLIST resides in */
    struct  rlist *rnext;	/* ptr to next RLIST */
} RLIST;


typedef struct orect
{
	struct orect *o_link;
	GRECT    o_gr;
} ORECT;


#define VF_INUSE   0x0001	/* this window has been created */
#define VF_BROKEN  0x0002
#define VF_INTREE  0x0004	/* this windows has been opened */
#define VF_SUBWIN  0x0008

typedef struct window
{
	_WORD		w_flags;
	AESPD		*w_owner;
	_WORD		w_kind;
	char		*w_pname;
	char		*w_pinfo;
	GRECT		w_full;
	GRECT		w_work;
	GRECT		w_prev;
	_WORD		w_hslide;
	_WORD		w_vslide;
	_WORD		w_hslsiz;
	_WORD		w_vslsiz;
	_WORD		w_tcolor[NUM_ELEM];	
	_WORD		w_bcolor[NUM_ELEM];	
	ORECT		*w_rlist;	/* owner rectangle list	*/
	ORECT		*w_rnext;	/* used for search first search next */
} WINDOW;

#define NUM_ORECT (NUM_WIN * 10)	/* is this enough???	*/

/* system foreground and* background rules but transparent */
#define SYS_FG 0x1100
/* window title selected using pattern 2 & replace mode text */
#define WTS_FG 0x11a1
/* window title normal */
#define WTN_FG 0x1100

/* system has 3 pds to start - app, control mgr, acc	*/

#define THEGLO struct glstr
THEGLO
{
	UDA		g_uda;			/* must be first */
	UDA		g_2uda;
	UDA		g_3uda;
	AESPD g_pd[NUM_PDS];
	CDA	g_cda[NUM_PDS];
	EVB	g_evb[NUM_EVBS];

	FPD	g_fpdx[NFORKS];

	ORECT 	g_olist[NUM_ORECT];

	char	g_rawstr[MAX_LEN];	/* used in gemoblib.s	*/
	char	g_tmpstr[MAX_LEN];	/* used in gemoblib.s	*/
	char	g_valstr[MAX_LEN];	/* used in gembind.s	*/
	char	g_fmtstr[MAX_LEN];	/* used in gemoblib.s	*/

	char	g_loc1[256];		/* MAX alert length	*/
	char	alert_str[256];		/* must be long enough for longest alert in gem.rsc */

	char    g_scrap[164];		/* current scrap directory */

	char	g_dir[CMDLEN];		/* current desktop directory */

	AES_GLOBAL g_sysglo;

	char	g_path[CMDLEN];		/* new element		*/

	char	s_cmd[CMDLEN];
	char	s_save[SIZE_AFILE];	/* SIZE_AFILE		*/
	char	s_tail[CMDLEN];

	WINDOW	w_win[NUM_WIN];
};

extern THEGLO D;

#endif /* GEMLIB_H */
