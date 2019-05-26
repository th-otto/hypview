#define AESVERSION 0x320

#define NUM_ACCS 1			/* for atari in rom		*/
#define MAX_ACCS 6			/* maximum number of desk accessory   */
#define NUM_PDS (NUM_ACCS + 2)		/* acc's + ctrlpd + dos appl.	*/
#define EVBS_PER_PD     5               /* EVBs per AES process */
#define NUM_EVBS (NUM_PDS * 5)		/* 5 * the number of PDs	*/
#define NUM_IEVBS (2 * EVBS_PER_PD)		/* 5 * the number of internalPDs*/
#define NUM_EEVBS (NUM_ACCS * EVBS_PER_PD)		/* 5 * the number of externalPDs */

#define KBD_SIZE 8

#define CARTRIDGE	1	/* if rom cartridge exists */

#define AES3D  1
#define MULTITOS 0
#define CONF_WITH_PCGEM 0
#define NYI 0

#ifndef MULTILANG_SUPPORT
#define MULTILANG_SUPPORT (OS_COUNTRY == OS_CONF_MULTILANG)
#endif

#ifndef AES3D
#define AES3D (AESVERSION >= 0x330)
#endif

#ifndef COLORICON_SUPPORT
#define COLORICON_SUPPORT (AESVERSION >= 0x330)
#endif

#define	NUM_WIN	8		/* # window structures per block of memory */

#define STACK_SIZE  500
#define NFORKS      96
#define QUEUE_SIZE 256

#define CMDLEN 128

#define LEN_ZPATH 67                    /* max path length, incl drive */
#define LEN_ZFNAME 13                   /* max fname length, incl '\' separator */
#define MAXPATHLEN (LEN_ZPATH+LEN_ZFNAME+1) /* convenient shorthand */
