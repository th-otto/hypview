#ifndef __TOS_FAKE_H__
#define __TOS_FAKE_H__

#ifndef _WORD
#  define _WORD short
#endif
#ifndef _UWORD
#  define _UWORD unsigned short
#endif
#ifndef _UBYTE
#  define _UBYTE unsigned char
#endif
#ifndef _LONG
#  define _LONG long
#endif
#ifndef _ULONG
#  define _ULONG long
#endif


#define G_WHITE            0
#define G_BLACK            1
#define G_RED              2
#define G_GREEN            3
#define G_BLUE             4
#define G_CYAN             5
#define G_YELLOW           6
#define G_MAGENTA          7
#define G_LWHITE           8
#define G_LBLACK           9
#define G_LRED            10
#define G_LGREEN          11
#define G_LBLUE           12
#define G_LCYAN           13
#define G_LYELLOW         14
#define G_LMAGENTA        15

#ifndef __GRECT
# define __GRECT
typedef struct {
	_WORD g_x;
	_WORD g_y;
	_WORD g_w;
	_WORD g_h;
} GRECT;
#endif

#ifndef __MFDB
#define __MFDB
typedef struct mfdb
{
   void *fd_addr;                /* pointer to data */
   _WORD  fd_w;                  /* picture width, in pixel */
   _WORD  fd_h;                  /* picture height, in pixel */
   _WORD  fd_wdwidth;            /* number of _WORDs in a row */
   _WORD  fd_stand;              /* 0 = device dependent */
                                 /* 1 = standard format */
   _WORD  fd_nplanes;            /* number of planes */
   _WORD  fd_r1, fd_r2, fd_r3;   /* reserved */
} MFDB;
#endif

typedef struct
{
	_WORD x;
	_WORD y;
	_WORD bstate;
	_WORD kstate;
} EVNTDATA;

#endif /* __TOS_FAKE_H__ */
