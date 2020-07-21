/*
 * GEM resource C output of hyptree
 *
 * created by ORCS 2.17
 */

#if !defined(__GNUC__) || !defined(__mc68000__)
#include <portab.h>
#endif

#ifndef __STDC__
# ifdef __PUREC__
#  define __STDC__ 1
# endif
#endif

#ifdef OS_WINDOWS
#  include <portaes.h>
#  define SHORT _WORD
#  ifdef __WIN32__
#    define _WORD signed short
#  else
#    define _WORD signed int
 #   pragma option -zE_FARDATA
#  endif
#else
#  ifdef __TURBOC__
#    include <aes.h>
#    define CP (_WORD *)
#  endif
#endif

#ifdef OS_UNIX
#  include <portaes.h>
#  define SHORT _WORD
#else
#  ifdef __GNUC__
#    ifndef __PORTAES_H__
#      if __GNUC__ < 4
#        include <aesbind.h>
#        ifndef _WORD
#          define _WORD int
#        endif
#        define CP (char *)
#      else
#        include <mt_gem.h>
#        ifndef _WORD
#          define _WORD short
#        endif
#        define CP (short *)
#      endif
#      define CW (short *)
#    endif
#  endif
#endif


#ifdef __SOZOBONX__
#  include <xgemfast.h>
#else
#  ifdef SOZOBON
#    include <aes.h>
#  endif
#endif

#ifdef MEGAMAX
#  include <gembind.h>
#  include <gemdefs.h>
#  include <obdefs.h>
#  define _WORD int
#  define SHORT int
#endif

#ifndef _VOID
#  define _VOID void
#endif

#ifndef OS_NORMAL
#  define OS_NORMAL 0x0000
#endif
#ifndef OS_SELECTED
#  define OS_SELECTED 0x0001
#endif
#ifndef OS_CROSSED
#  define OS_CROSSED 0x0002
#endif
#ifndef OS_CHECKED
#  define OS_CHECKED 0x0004
#endif
#ifndef OS_DISABLED
#  define OS_DISABLED 0x0008
#endif
#ifndef OS_OUTLINED
#  define OS_OUTLINED 0x0010
#endif
#ifndef OS_SHADOWED
#  define OS_SHADOWED 0x0020
#endif
#ifndef OS_WHITEBAK
#  define OS_WHITEBAK 0x0040
#endif
#ifndef OS_DRAW3D
#  define OS_DRAW3D 0x0080
#endif

#ifndef OF_NONE
#  define OF_NONE 0x0000
#endif
#ifndef OF_SELECTABLE
#  define OF_SELECTABLE 0x0001
#endif
#ifndef OF_DEFAULT
#  define OF_DEFAULT 0x0002
#endif
#ifndef OF_EXIT
#  define OF_EXIT 0x0004
#endif
#ifndef OF_EDITABLE
#  define OF_EDITABLE 0x0008
#endif
#ifndef OF_RBUTTON
#  define OF_RBUTTON 0x0010
#endif
#ifndef OF_LASTOB
#  define OF_LASTOB 0x0020
#endif
#ifndef OF_TOUCHEXIT
#  define OF_TOUCHEXIT 0x0040
#endif
#ifndef OF_HIDETREE
#  define OF_HIDETREE 0x0080
#endif
#ifndef OF_INDIRECT
#  define OF_INDIRECT 0x0100
#endif
#ifndef OF_FL3DIND
#  define OF_FL3DIND 0x0200
#endif
#ifndef OF_FL3DBAK
#  define OF_FL3DBAK 0x0400
#endif
#ifndef OF_FL3DACT
#  define OF_FL3DACT 0x0600
#endif
#ifndef OF_MOVEABLE
#  define OF_MOVEABLE 0x0800
#endif
#ifndef OF_POPUP
#  define OF_POPUP 0x1000
#endif

#ifndef R_CICONBLK
#  define R_CICONBLK 17
#endif
#ifndef R_CICON
#  define R_CICON 18
#endif

#ifndef G_SWBUTTON
#  define G_SWBUTTON 34
#endif
#ifndef G_POPUP
#  define G_POPUP 35
#endif
#ifndef G_EDIT
#  define G_EDIT 37
#endif
#ifndef G_SHORTCUT
#  define G_SHORTCUT 38
#endif
#ifndef G_SLIST
#  define G_SLIST 39
#endif
#ifndef G_EXTBOX
#  define G_EXTBOX 40
#endif
#ifndef G_OBLINK
#  define G_OBLINK 41
#endif

#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    ifdef __PUREC__
#      define _WORD int
#    else
#      define _WORD short
#    endif
#  endif
#endif

#ifndef _UBYTE
#  define _UBYTE char
#endif

#ifndef _BOOL
#  define _BOOL int
#endif

#ifndef _LONG
#  ifdef LONG
#    define _LONG LONG
#  else
#    define _LONG long
#  endif
#endif

#ifndef _ULONG
#  ifdef ULONG
#    define _ULONG ULONG
#  else
#    define _ULONG unsigned long
#  endif
#endif

#ifndef _LONG_PTR
#  define _LONG_PTR _LONG
#endif

#ifndef C_UNION
#ifdef __PORTAES_H__
#  define C_UNION(x) { (_LONG_PTR)(x) }
#endif
#ifdef __GEMLIB__
#  define C_UNION(x) { (_LONG_PTR)(x) }
#endif
#ifdef __PUREC__
#  define C_UNION(x) { (_LONG_PTR)(x) }
#endif
#ifdef __ALCYON__
#  define C_UNION(x) x
#endif
#endif
#ifndef C_UNION
#  define C_UNION(x) (_LONG_PTR)(x)
#endif

#ifndef SHORT
#  define SHORT short
#endif

#ifndef CP
#  define CP (SHORT *)
#endif

#ifndef CW
#  define CW (_WORD *)
#endif


#undef RSC_STATIC_FILE
#define RSC_STATIC_FILE 1

#include "hyptree.h"

#ifndef RSC_NAMED_FUNCTIONS
#  define RSC_NAMED_FUNCTIONS 0
#endif

#ifndef __ALCYON__
#undef defRSHInit
#undef defRSHInitBit
#undef defRSHInitStr
#ifndef RsArraySize
#define RsArraySize(array) (_WORD)(sizeof(array)/sizeof(array[0]))
#define RsPtrArraySize(type, array) (type *)array, RsArraySize(array)
#endif
#define defRSHInit( aa, bb ) RSHInit( aa, bb, RsPtrArraySize(OBJECT *, rs_trindex), RsArraySize(rs_object) )
#define defRSHInitBit( aa, bb ) RSHInitBit( aa, bb, RsPtrArraySize(BITBLK *, rs_frimg) )
#define defRSHInitStr( aa, bb ) RSHInitStr( aa, bb, RsPtrArraySize(_UBYTE *, rs_frstr) )
#endif

#ifdef __STDC__
#ifndef W_Cicon_Setpalette
extern _BOOL W_Cicon_Setpalette(_WORD *_palette);
#endif
#ifndef hrelease_objs
extern void hrelease_objs(OBJECT *_ob, _WORD _num_objs);
#endif
#ifndef hfix_objs
extern void *hfix_objs(RSHDR *_hdr, OBJECT *_ob, _WORD _num_objs);
#endif
#endif

#ifndef RLOCAL
#  if RSC_STATIC_FILE
#    ifdef LOCAL
#      define RLOCAL LOCAL
#    else
#      define RLOCAL static
#    endif
#  else
#    define RLOCAL
#  endif
#endif


#ifndef N_
#  define N_(x)
#endif


#if RSC_STATIC_FILE
#undef NUM_STRINGS
#undef NUM_BB
#undef NUM_IB
#undef NUM_CIB
#undef NUM_CIC
#undef NUM_TI
#undef NUM_FRSTR
#undef NUM_FRIMG
#undef NUM_OBS
#undef NUM_TREE
#undef NUM_UD
#define NUM_STRINGS 28
#define NUM_BB		0
#define NUM_IB		0
#define NUM_CIB     1
#define NUM_CIC     1
#define NUM_TI		0
#define NUM_FRSTR	8
#define NUM_FRIMG	0
#define NUM_OBS     28
#define NUM_TREE	3
#define NUM_UD		0
#endif


static char hyptree_string_0[] = "";
static char hyptree_string_1[] = " HYPTREE";
static char hyptree_string_2[] = " File";
static char hyptree_string_3[] = "  About HypTree...";
static char hyptree_string_4[] = "---------------------";
static char hyptree_string_5[] = "  Desk Accessory 1";
static char hyptree_string_6[] = "  Desk Accessory 2";
static char hyptree_string_7[] = "  Desk Accessory 3";
static char hyptree_string_8[] = "  Desk Accessory 4";
static char hyptree_string_9[] = "  Desk Accessory 5";
static char hyptree_string_10[] = "  Desk Accessory 6";
static char hyptree_string_11[] = "  Open...  ^O";
static char hyptree_string_12[] = "  Close    ^U";
static char hyptree_string_13[] = "--------------";
static char hyptree_string_14[] = "  Quit     ^Q";
static char hyptree_string_15[] = "OK";
static char hyptree_string_16[] = "ProgrammnameXXXXXXXXXXXXXXXXXXXXXXXX";
static char hyptree_string_17[] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
static char hyptree_string_18[] = "Program to display the tree";
static char hyptree_string_19[] = "structure of ST-Guide hypertexts.";
static char hyptree_string_20[] = "Select hypertext to load:";
static char hyptree_string_21[] = "[1][Command could not be executed.|There is not enough memory.][Cancel]";
static char hyptree_string_22[] = "[1][Please install the system|extension WDIALOG.PRG][Cancel]";
static char hyptree_string_23[] = "[1][Can\'t open a VDI workstation.][Cancel]";
static char hyptree_string_24[] = "from: %s";
static char hyptree_string_25[] = "Programinfo...";
static char hyptree_string_26[] = "[1][Tree %u not found!][Abort]";
static char hyptree_string_27[] = "About HypTree";


/* mask of DI_ICON */
static _UBYTE hyptree_RS0_MMASK[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFE, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF};

/* data of DI_ICON */
static _UBYTE hyptree_RS0_MDATA[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFC, 
0x40, 0x00, 0x00, 0x04, 0x55, 0x55, 0x55, 0x56, 0x40, 0x00, 0x00, 0x06, 0x7F, 0xFF, 0xFF, 0xFE, 
0x40, 0x00, 0x00, 0x06, 0x40, 0x1F, 0xE0, 0x06, 0x40, 0x7F, 0xF8, 0x06, 0x40, 0xFF, 0xFC, 0x06, 
0x40, 0xF0, 0xFC, 0x06, 0x40, 0xF0, 0x7C, 0x06, 0x40, 0x78, 0x7C, 0x06, 0x40, 0x38, 0xFC, 0x06, 
0x40, 0x01, 0xF8, 0x06, 0x40, 0x03, 0xF0, 0x06, 0x40, 0x07, 0xE0, 0x06, 0x40, 0x07, 0xC0, 0x06, 
0x40, 0x07, 0xC0, 0x06, 0x40, 0x07, 0xC0, 0x06, 0x40, 0x03, 0x80, 0x06, 0x40, 0x00, 0x00, 0x06, 
0x40, 0x03, 0x80, 0x06, 0x40, 0x07, 0xC0, 0x06, 0x40, 0x07, 0xC0, 0x06, 0x40, 0x03, 0x80, 0x06, 
0x40, 0x00, 0x00, 0x06, 0x7F, 0xFF, 0xFF, 0xFE, 0x1F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00};

/* color data of DI_ICON */
static _UBYTE hyptree_RS0_4CDATA[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xF8, 
0x7F, 0xFF, 0xFF, 0xFA, 0x40, 0x00, 0x00, 0x02, 0x7F, 0xFF, 0xFF, 0xFA, 0x40, 0x00, 0x00, 0x02, 
0x40, 0x00, 0x00, 0x02, 0x40, 0x1F, 0xE0, 0x02, 0x40, 0x7F, 0xF8, 0x02, 0x40, 0xF0, 0xF8, 0x02, 
0x40, 0xE0, 0x78, 0x02, 0x40, 0xE0, 0x78, 0x02, 0x40, 0x70, 0x78, 0x02, 0x40, 0x00, 0xF8, 0x02, 
0x40, 0x01, 0xF0, 0x02, 0x40, 0x03, 0xE0, 0x02, 0x40, 0x07, 0xC0, 0x02, 0x40, 0x07, 0x80, 0x02, 
0x40, 0x07, 0x80, 0x02, 0x40, 0x07, 0x80, 0x02, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 
0x40, 0x03, 0x80, 0x02, 0x40, 0x07, 0x80, 0x02, 0x40, 0x07, 0x80, 0x02, 0x40, 0x00, 0x00, 0x02, 
0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0x3F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xF8, 
0x7F, 0xFF, 0xFF, 0xFA, 0x40, 0x00, 0x00, 0x02, 0x7F, 0xFF, 0xFF, 0xFA, 0x40, 0x00, 0x00, 0x02, 
0x40, 0x00, 0x00, 0x02, 0x40, 0x1F, 0xE0, 0x02, 0x40, 0x7F, 0xF8, 0x02, 0x40, 0xF0, 0xF8, 0x02, 
0x40, 0xE0, 0x78, 0x02, 0x40, 0xE0, 0x78, 0x02, 0x40, 0x70, 0x78, 0x02, 0x40, 0x00, 0xF8, 0x02, 
0x40, 0x01, 0xF0, 0x02, 0x40, 0x03, 0xE0, 0x02, 0x40, 0x07, 0xC0, 0x02, 0x40, 0x07, 0x80, 0x02, 
0x40, 0x07, 0x80, 0x02, 0x40, 0x07, 0x80, 0x02, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 
0x40, 0x03, 0x80, 0x02, 0x40, 0x07, 0x80, 0x02, 0x40, 0x07, 0x80, 0x02, 0x40, 0x00, 0x00, 0x02, 
0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0x3F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xF8, 
0x7F, 0xFF, 0xFF, 0xFA, 0x40, 0x00, 0x00, 0x02, 0x7F, 0xFF, 0xFF, 0xFA, 0x40, 0x00, 0x00, 0x02, 
0x40, 0x00, 0x00, 0x02, 0x40, 0x1F, 0xE0, 0x02, 0x40, 0x7F, 0xF8, 0x02, 0x40, 0xF0, 0xF8, 0x02, 
0x40, 0xE0, 0x78, 0x02, 0x40, 0xE0, 0x78, 0x02, 0x40, 0x70, 0x78, 0x02, 0x40, 0x00, 0xF8, 0x02, 
0x40, 0x01, 0xF0, 0x02, 0x40, 0x03, 0xE0, 0x02, 0x40, 0x07, 0xC0, 0x02, 0x40, 0x07, 0x80, 0x02, 
0x40, 0x07, 0x80, 0x02, 0x40, 0x07, 0x80, 0x02, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 
0x40, 0x03, 0x80, 0x02, 0x40, 0x07, 0x80, 0x02, 0x40, 0x07, 0x80, 0x02, 0x40, 0x00, 0x00, 0x02, 
0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0x3F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFC, 
0x40, 0x00, 0x00, 0x06, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x40, 0x00, 0x00, 0x06, 
0x40, 0x00, 0x00, 0x06, 0x40, 0x1F, 0xE0, 0x06, 0x40, 0x7F, 0xF8, 0x06, 0x40, 0xFF, 0xFC, 0x06, 
0x40, 0xF0, 0xFC, 0x06, 0x40, 0xF0, 0x7C, 0x06, 0x40, 0x78, 0x7C, 0x06, 0x40, 0x38, 0xFC, 0x06, 
0x40, 0x01, 0xF8, 0x06, 0x40, 0x03, 0xF0, 0x06, 0x40, 0x07, 0xE0, 0x06, 0x40, 0x07, 0xC0, 0x06, 
0x40, 0x07, 0xC0, 0x06, 0x40, 0x07, 0xC0, 0x06, 0x40, 0x03, 0x80, 0x06, 0x40, 0x00, 0x00, 0x06, 
0x40, 0x03, 0x80, 0x06, 0x40, 0x07, 0xC0, 0x06, 0x40, 0x07, 0xC0, 0x06, 0x40, 0x03, 0x80, 0x06, 
0x40, 0x00, 0x00, 0x06, 0x7F, 0xFF, 0xFF, 0xFE, 0x3F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00};

/* color mask of DI_ICON */
static _UBYTE hyptree_RS0_4CMASK[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFC, 
0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 
0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 
0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 
0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 
0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 
0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 
0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x3F, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00};

static char *rs_frstr[NUM_FRSTR] = {
	hyptree_string_20,
	hyptree_string_21,
	hyptree_string_22,
	hyptree_string_23,
	hyptree_string_24,
	hyptree_string_25,
	hyptree_string_26,
	hyptree_string_27
};


static CICON rs_cicon[] = {
	{ 4, (_WORD *) hyptree_RS0_4CDATA, (_WORD *) hyptree_RS0_4CMASK, 0, 0, 0 }
};


static CICONBLK rs_ciconblk[] = {
	{ { (_WORD *) hyptree_RS0_MMASK, (_WORD *) hyptree_RS0_MDATA, hyptree_string_0, 4096,13,5, 0,0,32,32, 13,7,6,8 }, &rs_cicon[0] } /* DI_ICON */
};


static OBJECT rs_object[NUM_OBS] = {
/* DIAL_LIBRARY */

	{ -1, 1, 1, G_BOX, OF_FL3DBAK, OS_NORMAL, C_UNION(0x1100L), 0,0, 1111,2057 },
	{ 0, -1, -1, G_CICON, OF_LASTOB, OS_NORMAL, C_UNION(&rs_ciconblk[0]), 0,0, 8192,8192 }, /* DI_ICON */

/* MAINMENU */

	{ -1, 1, 5, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x0L), 0,0, 160,25 },
	{ 5, 2, 2, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0,0, 160,513 },
	{ 1, 3, 4, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x0L), 2,0, 15,769 },
	{ 4, -1, -1, G_TITLE, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_1), 0,0, 9,769 },
	{ 2, -1, -1, G_TITLE, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_2), 9,0, 6,769 }, /* ME_FILE */
	{ 0, 6, 15, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x0L), 0,769, 160,19 },
	{ 15, 7, 14, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 2,0, 21,8 },
	{ 8, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_3), 0,0, 21,1 }, /* ME_ABOUT */
	{ 9, -1, -1, G_STRING, OF_NONE, OS_DISABLED, C_UNION(hyptree_string_4), 0,1, 21,1 },
	{ 10, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_5), 0,2, 21,1 },
	{ 11, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_6), 0,3, 21,1 },
	{ 12, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_7), 0,4, 21,1 },
	{ 13, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_8), 0,5, 21,1 },
	{ 14, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_9), 0,6, 21,1 },
	{ 6, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_10), 0,7, 21,1 },
	{ 5, 16, 19, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 11,0, 14,4 },
	{ 17, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_11), 0,0, 14,1 }, /* ME_OPEN */
	{ 18, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_12), 0,1, 14,1 }, /* ME_CLOSE */
	{ 19, -1, -1, G_STRING, OF_NONE, OS_DISABLED, C_UNION(hyptree_string_13), 0,2, 14,1 },
	{ 15, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(hyptree_string_14), 0,3, 14,1 }, /* ME_QUIT */

/* ABOUT_DIALOG */

	{ -1, 1, 5, G_BOX, OF_FL3DBAK, OS_OUTLINED, C_UNION(0x21100L), 0,0, 38,10 },
	{ 2, -1, -1, G_BUTTON, 0x607, OS_WHITEBAK, C_UNION(hyptree_string_15), 28,8, 8,1 }, /* PROG_OK */
	{ 3, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_16), 1,2, 36,1 }, /* PROG_NAME */
	{ 4, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_17), 1,3, 36,1 }, /* PROG_DATE */
	{ 5, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(hyptree_string_18), 1,5, 36,1 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(hyptree_string_19), 1,6, 33,1 }
};


static OBJECT *rs_trindex[NUM_TREE] = {
	&rs_object[0], /* DIAL_LIBRARY */
	&rs_object[2], /* MAINMENU */
	&rs_object[22] /* ABOUT_DIALOG */
};





#if RSC_STATIC_FILE

#if RSC_NAMED_FUNCTIONS
#ifdef __STDC__
_WORD hyptree_rsc_load(_WORD wchar, _WORD hchar)
#else
_WORD hyptree_rsc_load(wchar, hchar)
_WORD wchar;
_WORD wchar;
#endif
{
#ifndef RSC_HAS_PALETTE
#  define RSC_HAS_PALETTE 0
#endif
#ifndef RSC_USE_PALETTE
#  define RSC_USE_PALETTE 0
#endif
#if RSC_HAS_PALETTE || RSC_USE_PALETTE
	W_Cicon_Setpalette(&rgb_palette[0][0]);
#endif
#if NUM_OBS != 0
	{
		_WORD Obj;
		OBJECT *tree;
		for (Obj = 0, tree = rs_object; Obj < NUM_OBS; Obj++, tree++)
		{
			tree->ob_x = wchar * (tree->ob_x & 0xff) + (tree->ob_x >> 8);
			tree->ob_y = hchar * (tree->ob_y & 0xff) + (tree->ob_y >> 8);
			tree->ob_width = wchar * (tree->ob_width & 0xff) + (tree->ob_width >> 8);
			tree->ob_height = hchar * (tree->ob_height & 0xff) + (tree->ob_height >> 8);
		}
		hfix_objs(NULL, rs_object, NUM_OBS);
	}
#endif
	return 1;
}


#ifdef __STDC__
_WORD hyptree_rsc_gaddr(_WORD type, _WORD idx, void *gaddr)
#else
_WORD hyptree_rsc_gaddr(type, idx, gaddr)
_WORD type;
_WORD idx;
void *gaddr;
#endif
{
	switch (type)
	{
#if NUM_TREE != 0
	case R_TREE:
		if (idx < 0 || idx >= NUM_TREE)
			return 0;
		*((OBJECT **)gaddr) = rs_trindex[idx];
		break;
#endif
#if NUM_OBS != 0
	case R_OBJECT:
		if (idx < 0 || idx >= NUM_OBS)
			return 0;
		*((OBJECT **)gaddr) = &rs_object[idx];
		break;
#endif
#if NUM_TI != 0
	case R_TEDINFO:
		if (idx < 0 || idx >= NUM_TI)
			return 0;
		*((TEDINFO **)gaddr) = &rs_tedinfo[idx];
		break;
#endif
#if NUM_IB != 0
	case R_ICONBLK:
		if (idx < 0 || idx >= NUM_IB)
			return 0;
		*((ICONBLK **)gaddr) = &rs_iconblk[idx];
		break;
#endif
#if NUM_BB != 0
	case R_BITBLK:
		if (idx < 0 || idx >= NUM_BB)
			return 0;
		*((BITBLK **)gaddr) = &rs_bitblk[idx];
		break;
#endif
#if NUM_FRSTR != 0
	case R_STRING:
		if (idx < 0 || idx >= NUM_FRSTR)
			return 0;
		*((char **)gaddr) = (char *)(rs_frstr[idx]);
		break;
#endif
#if NUM_FRIMG != 0
	case R_IMAGEDATA:
		if (idx < 0 || idx >= NUM_FRIMG)
			return 0;
		*((BITBLK **)gaddr) = rs_frimg[idx];
		break;
#endif
#if NUM_OBS != 0
	case R_OBSPEC:
		if (idx < 0 || idx >= NUM_OBS)
			return 0;
		*((_LONG **)gaddr) = &rs_object[idx].ob_spec.index;
		break;
#endif
#if NUM_TI != 0
	case R_TEPTEXT:
		if (idx < 0 || idx >= NUM_TI)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_tedinfo[idx].te_ptext);
		break;
#endif
#if NUM_TI != 0
	case R_TEPTMPLT:
		if (idx < 0 || idx >= NUM_TI)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_tedinfo[idx].te_ptmplt);
		break;
#endif
#if NUM_TI != 0
	case R_TEPVALID:
		if (idx < 0 || idx >= NUM_TI)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_tedinfo[idx].te_pvalid);
		break;
#endif
#if NUM_IB != 0
	case R_IBPMASK:
		if (idx < 0 || idx >= NUM_IB)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_iconblk[idx].ib_pmask);
		break;
#endif
#if NUM_IB != 0
	case R_IBPDATA:
		if (idx < 0 || idx >= NUM_IB)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_iconblk[idx].ib_pdata);
		break;
#endif
#if NUM_IB != 0
	case R_IBPTEXT:
		if (idx < 0 || idx >= NUM_IB)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_iconblk[idx].ib_ptext);
		break;
#endif
#if NUM_BB != 0
	case R_BIPDATA:
		if (idx < 0 || idx >= NUM_BB)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_bitblk[idx].bi_pdata);
		break;
#endif
#if NUM_FRSTR != 0
	case R_FRSTR:
		if (idx < 0 || idx >= NUM_FRSTR)
			return 0;
		*((char ***)gaddr) = (char **)(&rs_frstr[idx]);
		break;
#endif
#if NUM_FRIMG != 0
	case R_FRIMG:
		if (idx < 0 || idx >= NUM_FRIMG)
			return 0;
		*((BITBLK ***)gaddr) = &rs_frimg[idx];
		break;
#endif
	default:
		return 0;
	}
	return 1;
}


#ifdef __STDC__
_WORD hyptree_rsc_free(void)
#else
_WORD hyptree_rsc_free()
#endif
{
#if NUM_OBS != 0
	hrelease_objs(rs_object, NUM_OBS);
#endif
	return 1;
}

#endif /* RSC_NAMED_FUNCTIONS */

#else /* !RSC_STATIC_FILE */
#if 0
_WORD rs_numstrings = 28;
_WORD rs_numfrstr = 8;

_WORD rs_nuser = 0;
_WORD rs_numimages = 0;
_WORD rs_numbb = 0;
_WORD rs_numfrimg = 0;
_WORD rs_numib = 0;
_WORD rs_numcib = 1;
_WORD rs_numti = 0;
_WORD rs_numobs = 28;
_WORD rs_numtree = 3;

char rs_name[] = "hyptree.rsc";

_WORD _rsc_format = 2; /* RSC_FORM_SOURCE2 */
#endif
#endif /* RSC_STATIC_FILE */
