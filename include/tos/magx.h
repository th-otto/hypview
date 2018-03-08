
/* Sconfig(2) -> */
#ifndef _DOSVARS
#define _DOSVARS
typedef struct
{
	char		*in_dos;				/* Address of DOS flags       */
	short		*dos_time;				/* Address of DOS time        */
	short		*dos_date;				/* Address of DOS date        */
	long		res1;
	long		res2;
	long		res3;
	void		*act_pd;				/* Running program            */
	long		res4;
	short		res5;
	void		*res6;
	void		*res7;					/* Internal DOS-memory list   */
	void		(*resv_intmem)(void);	/* Extend DOS memory          */
#ifdef __NO_CDECL
   void *etv_critic;
#else
   long      __CDECL (*etv_critic)(short err);         /* etv_critic of GEMDOS      */
#endif
	char *	((*err_to_str)(signed char e));	/* Conversion code->plaintext */
	long		res8;
	long		res9;
	long		res10;
} DOSVARS;
#endif

#ifndef _AESVARS
#define _AESVARS
typedef struct
{
	long	magic;							/* Must be $87654321               */
	void	*membot;						/* End of the AES-variables        */
	void	*aes_start;						/* Start address                   */
	long	magic2;							/* Is 'MAGX'                       */
	long	date;							/* Creation date ddmmyyyy          */
    void    (*chgres)(int res, int txt);    /* Change resolution               */
    long    (**shel_vector)(void);          /* Resident desktop                */
	char    *aes_bootdrv;                   /* Booting took place from here    */
	short *vdi_device;                      /* VDI-driver used by AES          */
	void	*reservd1;
	void	*reservd2;
	void	*reservd3;
	short	version;						/* e.g. $0201 is V2.1              */
	short	release;						/* 0=alpha..3=release              */
} AESVARS;
#endif

#ifndef _MAGX_COOKIE
#define _MAGX_COOKIE
typedef struct
{
	long		config_status;
	DOSVARS	*dosvars;
	AESVARS	*aesvars;
	void		*res1;
	void		*hddrv_functions;
    long         status_bits;               /* MagiC 3 from 24.5.95 on         */
} MAGX_COOKIE;
#endif
