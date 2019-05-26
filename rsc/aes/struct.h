/*      STRUCT.H        1/28/84 - 01/18/85      Lee Jay Lorenzen        */

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

#ifndef GEMSTRUCT_H
#define GEMSTRUCT_H

typedef struct aespd  AESPD;			/* process descriptor			*/
typedef struct cqueue CQUEUE;           /* console kbd queue			*/
typedef struct uda UDA;					/* user stack data area 		*/
typedef struct cdastr CDA;              /* console data area structure	*/
typedef struct qpb QPB;					/* queue parameter block		*/
typedef struct evb EVB;					/* event block					*/
typedef struct spb SPB;					/* sync parameter block 		*/
typedef struct fpd FPD; 				/* fork process descriptor		*/

typedef _UWORD	EVSPEC;

/* console kbd queue */
struct cqueue
{
	_WORD	c_buff[KBD_SIZE];
	_WORD	c_front;
	_WORD	c_rear;
	_WORD	c_cnt;
};


#define C_KOWNER 0x0001
#define C_MOWNER 0x0002


/* console data area structure */
struct cdastr {
	_UWORD	c_flags;
	EVB 	*c_iiowait; 	/* waiting for input */
	EVB 	*c_msleep;		/* wait for mouse rect */
	EVB 	*c_bsleep;		/* wait for button */
	CQUEUE	c_q;			/* input queue */
};


/* user stack data area */
struct uda {
	_WORD	u_insuper;					/*	 0	in supervisor flag		 */
	uint32_t	u_regs[15]; 			/*	 2	d0-d7, a0-a6			 */
	uint32_t	*u_spsuper; 			/*	3E	supervisor stack		 */
	uint32_t	*u_spuser;				/*	42	user stack				 */
	uint32_t	*u_oldspsuper;			/*	46	old ssp, used in trapaes [gemdosif.S] */
	uint32_t	u_super[STACK_SIZE];
	uint32_t	u_supstk;
};


#define NOCANCEL 0x0001			/* event is occurring */
#define COMPLETE 0x0002			/* event completed */
#define EVDELAY  0x0004			/* event is delay event */
#define EVMOUT   0x0008			/* event flag for mouse wait outside of rect*/

/* event block structure */
struct evb {
	EVB		*e_nextp;		/* link to next event on PD event list */
	EVB		*e_link;		/* link to next block on event chain */
	EVB		*e_pred;		/* link to prev block on event chain */
	AESPD	*e_pd;			/* owner PD (data for fork) */
	_LONG	e_parm;			/* parameter for request -> event comm */
	_WORD	e_flag;			/* look to above defines */
	EVSPEC	e_mask;			/* mask for event notification */
	_LONG	e_return;		/* return number of click, character or button state */
};

/* pd defines */
/* p_name */
#define AP_NAMELEN	8			/* architectural */
/* p_stat */
#define		WAITIN		0x0001
#define		SWITCHIN	0x8000
#define		PS_RUN			1
#define		PS_MWAIT		2
#define		PS_TRYSUSPEND	4
#define		PS_TOSUSPEND	8
#define		PS_SUSPENDED	16
/* p_flags */
#define AP_OPEN 	0x0001		/* application is between appl_init() & appl_exit() */

/* process descriptor */
struct aespd {
	AESPD	*p_link;		/*	0 */
	AESPD	*p_thread;		/*	4 */
	UDA 	*p_uda; 		/*	8 */

	/* ^^^ variables above are accessed from assembler code */
	
	char	p_name[AP_NAMELEN]; /*	C */

	CDA 	*p_cda; 		/* 14  cio data area		*/
	intptr_t p_ldaddr;		/* 18  long addr. of load	*/
	_WORD	p_pid;			/* 1C */
	_WORD	p_stat; 		/* 1E */

	EVSPEC	p_evbits;		/* 20  event bits in use	*/
	EVSPEC	p_evwait;		/* 22  event wait mask		*/
	EVSPEC	p_evflg;		/* 24  event flags			*/
	_BOOL	p_msgtosend;
	_WORD	p_message[9];
	MFORM	p_mouse;		/* saved mouseform for M_SAVE/M_RESTORE */

	_UWORD	p_flags;		/* 26  process status flags, see above */
	EVB 	*p_evlist;		/* 28 */
	EVB 	*p_qdq; 		/* 2C */
	EVB 	*p_qnq; 		/* 30 */
	char	*p_qaddr;		/* 34 */
	_WORD	p_qindex;		/* 38 */
	char	p_queue[QUEUE_SIZE];   /* 3A */
	char	p_appdir[LEN_ZPATH+2];	/* directory containing the executable */
									/* (includes trailing path separator)  */
};


/* queue parameter block */
struct qpb
{
	AESPD	*qpb_ppd;
	_WORD	qpb_cnt;
	void    *qpb_buf;
};

/* sync parameter block */
struct spb
{
	_WORD	sy_tas;
	AESPD	*sy_owner;
	EVB 	*sy_wait;
};

typedef void (*FCODE)(_LONG fdata);		/* pointer to function used by forkq() */

/* fork process descriptor		*/
struct fpd
{
	FCODE	f_code;
	_LONG	f_data;
};

/*
 * (most of) the set of structures required to manage one AES process
 */
typedef struct accpd
{
	AESPD	ac_pd;
	UDA		ac_uda;
	CDA		ac_cda;
	EVB		ac_evb[EVBS_PER_PD];	/* 5 evb's per process		*/
} ACCPD;


/* GEM memory usage parameter block */

#define GEM_MUPB_MAGIC 0x87654321

typedef struct
{
	uint32_t gm_magic;		/* Magical value, has to be GEM_MUPB_MAGIC */
	void  *gm_end;			/* End of the memory required by GEM */
	void  (*gm_init)(void); /* Start address of GEM */
} GEM_MUPB;

/* async bdos calls */
#define AQRD 1
#define AQWRT 2
#define ADELAY 3
#define AMUTEX 4
#define AKBIN 5
#define AMOUSE 6
#define ABUTTON 7

#define NOT_FOUND 100	 /* try to return from event not on PD list */
#define NOT_COMPLETE 101 /* try to ret from event which has not occured */

#endif /* GEMSTRUCT_H */
