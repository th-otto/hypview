/*
 * resource set indices for gem
 *
 * created by ORCS 2.16
 */

/*
 * Number of Strings:        79
 * Number of Bitblks:        14
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       14
 * Number of Free Strings:   25
 * Number of Free Images:    14
 * Number of Objects:        66
 * Number of Trees:          3
 * Number of Userblks:       0
 * Number of Images:         14
 * Total file size:          5508
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "gem"
#endif
#undef RSC_ID
#ifdef gem
#define RSC_ID gem
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 79
#define NUM_FRSTR 25
#define NUM_UD 0
#define NUM_IMAGES 14
#define NUM_BB 14
#define NUM_FRIMG 14
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 14
#define NUM_OBS 66
#define NUM_TREE 3
#endif



#define FSELECTR                           0 /* form/dialog */
#define FSTITLE                            1 /* STRING in tree FSELECTR */
#define FSDIRECT                           3 /* FBOXTEXT in tree FSELECTR */ /* max len 38 */
#define FSDRIVE                            4 /* FTEXT in tree FSELECTR */ /* max len 0 */
#define FSSELECT                           5 /* FBOXTEXT in tree FSELECTR */ /* max len 11 */
#define FSDRIVES                           6 /* IBOX in tree FSELECTR */
#define FS1STDRV                           7 /* BOXCHAR in tree FSELECTR */
#define FSLSTDRV                          32 /* BOXCHAR in tree FSELECTR */
#define FCLSBOX                           34 /* BOXCHAR in tree FSELECTR */
#define FTITLE                            35 /* BOXTEXT in tree FSELECTR */ /* max len 0 */
#define SCRLBAR                           36 /* BOX in tree FSELECTR */
#define FUPAROW                           37 /* BOXCHAR in tree FSELECTR */
#define FDNAROW                           38 /* BOXCHAR in tree FSELECTR */
#define FSVSLID                           39 /* BOX in tree FSELECTR */
#define FSVELEV                           40 /* BOX in tree FSELECTR */
#define FILEBOX                           41 /* BOX in tree FSELECTR */
#define F1NAME                            42 /* FBOXTEXT in tree FSELECTR */ /* max len 12 */
#define F2NAME                            43 /* FBOXTEXT in tree FSELECTR */ /* max len 12 */
#define F3NAME                            44 /* FBOXTEXT in tree FSELECTR */ /* max len 12 */
#define F4NAME                            45 /* FBOXTEXT in tree FSELECTR */ /* max len 12 */
#define F5NAME                            46 /* FBOXTEXT in tree FSELECTR */ /* max len 12 */
#define F6NAME                            47 /* FBOXTEXT in tree FSELECTR */ /* max len 12 */
#define F7NAME                            48 /* FBOXTEXT in tree FSELECTR */ /* max len 12 */
#define F8NAME                            49 /* FBOXTEXT in tree FSELECTR */ /* max len 12 */
#define F9NAME                            50 /* FBOXTEXT in tree FSELECTR */ /* max len 12 */
#define FSOK                              51 /* BUTTON in tree FSELECTR */
#define FSCANCEL                          52 /* BUTTON in tree FSELECTR */

#define DIALERT                            1 /* form/dialog */
#define ALICON                             1 /* BOX in tree DIALERT */
#define MSGOFF                             2 /* STRING in tree DIALERT */
#define BUTOFF                             7 /* BUTTON in tree DIALERT */

#define DESKTOP                            2 /* form/dialog */
#define APPTITLE                           2 /* TEXT in tree DESKTOP */ /* max len 1 */

#define AL00CRT                            0 /* Alert string */
/* [1][The disk in drive %c: is|physically write-protected.][Cancel|Retry] */

#define AL01CRT                            1 /* Alert string */
/* [2][Drive %c: is not responding.|Please check the disk drive,|or insert a disk.][Cancel|Retry] */

#define AL02CRT                            2 /* Alert string */
/* [1][Data on the disk in drive %c:|may be damaged.][Cancel|Retry] */

#define AL03CRT                            3 /* Alert string */
/* [2][This application cannot read|data on the disk in drive %c:.][Cancel|Retry] */

#define AL04CRT                            4 /* Alert string */
/* [1][Your output device is not|receiving data.][Cancel|Retry] */

#define AL05CRT                            5 /* Alert string */
/* [3][An error has occurred in GEM.|Please contact the EmuTOS|Development Team.][Cancel] */

#define AL18ERR                            6 /* Alert string */
/* [2][This application cannot|find the folder or file|you just tried to access.][  OK  ] */

#define AL04ERR                            7 /* Alert string */
/* [1][This application does not|have room to open another|document.  To make room,|close any document that|you do not need.][  OK  ] */

#define AL05ERR                            8 /* Alert string */
/* [1][An item with this name|already exists in the|directory, or this item|is set to Read Only status.][  OK  ] */

#define AL15ERR                            9 /* Alert string */
/* [1][The drive you specified|does not exist.][ Cancel ] */

#define AL08ERR                           10 /* Alert string */
/* [1][There is not enough memory|in your computer for the|application you just tried|to run.][  OK  ] */

#define ALXXERR                           11 /* Alert string */
/* [3][TOS error #%u.][Cancel] */

#define ALNOFUNC                          12 /* Alert string */
/* [3][Unsupported AES function #%d.][Cancel] */

#define ALRTDSWAP                         13 /* Alert string */
/* [3][Please insert disk %c|into drive A:.][  OK  ] */

#define ITEMSLCT                          14 /* Free string */
/* ITEM SELECTOR */

#define ST9VAL                            15 /* Free string */
/* 0..9 */

#define STAVAL                            16 /* Free string */
/* A..Z Äéèêíôö•µ∂∑∏û¬..‹ */

#define STNVAL                            17 /* Free string */
/* 0..9A..Z Äéèêíôö•µ∂∑∏û¬..‹ */

#define STPVAL                            18 /* Free string */
/* 0..9a..zA..ZÄ..ˇ\?*:._ */

#define STLPVAL                           19 /* Free string */
/* 0..9a..zA..ZÄ..ˇ\:_ */

#define STFVAL                            20 /* Free string */
/* a..z0..9A..ZÄ..ˇ:?*_ */

#define STLFVAL                           21 /* Free string */
/* a..z0..9A..ZÄ..ˇ_ */

#define STLAVAL                           22 /* Free string */
/* a..zA..Z Ä..ˇ */

#define STLNVAL                           23 /* Free string */
/* 0..9a..zA..Z Ä..ˇ */

#define NOMEMORY                          24 /* Alert string */
/* [1][The system does not have |enough memory for this|directory.][  OK  ] */

#define NOTEBB                             0 /* Free image */

#define QUESTBB                            1 /* Free image */

#define STOPBB                             2 /* Free image */

#define NOTEBB_TOS                         3 /* Free image */

#define QUESTBB_TOS                        4 /* Free image */

#define STOPBB_TOS                         5 /* Free image */

/* ARROW */
#define MICE00                             6 /* Mouse cursor */

/* TEXT_CRSR */
#define MICE01                             7 /* Mouse cursor */

/* HOURGLASS */
#define MICE02                             8 /* Mouse cursor */

/* POINT_HAND */
#define MICE03                             9 /* Mouse cursor */

/* FLAT_HAND */
#define MICE04                            10 /* Mouse cursor */

/* THIN_CROSS */
#define MICE05                            11 /* Mouse cursor */

/* THICK_CROSS */
#define MICE06                            12 /* Mouse cursor */

/* OUTLN_CROSS */
#define MICE07                            13 /* Mouse cursor */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD gem_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD gem_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD gem_rsc_free(void);
#endif
