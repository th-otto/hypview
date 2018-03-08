#define DICBIT    13					/* 12(-lh4-) or 13(-lh5-) */
#define DICSIZ (1U << DICBIT)
#define MATCHBIT   8					/* bits for MAXMATCH - THRESHOLD */
#define MAXMATCH 256					/* formerly F (not more than UCHAR_MAX + 1) */
#define THRESHOLD  3					/* choose optimal value */
#define PERC_FLAG 0x8000U

#define NC (UCHAR_MAX + MAXMATCH + 2 - THRESHOLD)	/* alphabet = {0, 1, 2, ..., NC - 1} */
#define CBIT 9							/* $\lfloor \log_2 NC \rfloor + 1$ */
#define CODE_BIT  16					/* codeword length */

#define NP (DICBIT + 1)					/* 0xE */
#define NT (CODE_BIT + 3)				/* 0x13 */
#define PBIT 4							/* smallest integer such that (1U << PBIT) > NP */
#define TBIT 5							/* smallest integer such that (1U << TBIT) > NT */
#if NT > NP
#define NPT NT							/* 19 */
#else
#define NPT NP
#endif

#define INIT_CRC  0  /* CCITT: 0xFFFF */
#define CRCPOLY  0xA001					/* ANSI CRC-16 */
						 /* CCITT: 0x8408 */
