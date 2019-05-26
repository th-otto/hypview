/*
 * GEM resource C output of gem
 *
 * created by ORCS 2.16
 */

#if !defined(__GNUC__) || !defined(__mc68000__)
#include <portab.h>
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
#    include <portaes.h>
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
#    define _WORD short
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

#include "gem.h"

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
extern _VOID hrelease_objs(OBJECT *_ob, _WORD _num_objs);
#endif
#ifndef hfix_objs
extern _VOID *hfix_objs(RSHDR *_hdr, OBJECT *_ob, _WORD _num_objs);
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
#define NUM_STRINGS 79
#define NUM_BB		14
#define NUM_IB		0
#define NUM_CIB     0
#define NUM_CIC     0
#define NUM_TI		14
#define NUM_FRSTR	25
#define NUM_FRIMG	14
#define NUM_OBS     66
#define NUM_TREE	3
#define NUM_UD		0
#endif


#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

static TEDINFO rs_tedinfo[NUM_TI];
static OBJECT rs_object[NUM_OBS];
static char gem_rsc_string_space[172];

static char const gem_rsc_string_0[] = "_";
static char const gem_rsc_string_1[] = N_("Directory:");
#define gem_rsc_string_2 &gem_rsc_string_space[0]
static char const gem_rsc_string_3[] = "______________________________________";
static char const gem_rsc_string_4[] = "P";
#define gem_rsc_string_5 &gem_rsc_string_space[39]
static char const gem_rsc_string_6[] = N_("Drive:");
static char const gem_rsc_string_7[] = "";
#define gem_rsc_string_8 &gem_rsc_string_space[40]
static char const gem_rsc_string_9[] = N_("Selection: ________.___");
static char const gem_rsc_string_10[] = "f";
static char const gem_rsc_string_11[] = "";
static char const gem_rsc_string_12[] = "";
static char const gem_rsc_string_13[] = "";
#define gem_rsc_string_14 &gem_rsc_string_space[52]
static char const gem_rsc_string_15[] = "_ ________.___ ";
static char const gem_rsc_string_16[] = "xF";
#define gem_rsc_string_17 &gem_rsc_string_space[65]
static char const gem_rsc_string_18[] = "_ ________.___ ";
static char const gem_rsc_string_19[] = "xF";
#define gem_rsc_string_20 &gem_rsc_string_space[78]
static char const gem_rsc_string_21[] = "_ ________.___ ";
static char const gem_rsc_string_22[] = "xF";
#define gem_rsc_string_23 &gem_rsc_string_space[91]
static char const gem_rsc_string_24[] = "_ ________.___ ";
static char const gem_rsc_string_25[] = "xF";
#define gem_rsc_string_26 &gem_rsc_string_space[104]
static char const gem_rsc_string_27[] = "_ ________.___ ";
static char const gem_rsc_string_28[] = "xF";
#define gem_rsc_string_29 &gem_rsc_string_space[117]
static char const gem_rsc_string_30[] = "_ ________.___ ";
static char const gem_rsc_string_31[] = "xF";
#define gem_rsc_string_32 &gem_rsc_string_space[130]
static char const gem_rsc_string_33[] = "_ ________.___ ";
static char const gem_rsc_string_34[] = "xF";
#define gem_rsc_string_35 &gem_rsc_string_space[143]
static char const gem_rsc_string_36[] = "_ ________.___ ";
static char const gem_rsc_string_37[] = "xF";
#define gem_rsc_string_38 &gem_rsc_string_space[156]
static char const gem_rsc_string_39[] = "_ ________.___ ";
static char const gem_rsc_string_40[] = "xF";
static char const gem_rsc_string_41[] = N_("OK");
static char const gem_rsc_string_42[] = N_("Cancel");
static char const gem_rsc_string_43[] = "";
static char const gem_rsc_string_44[] = "";
static char const gem_rsc_string_45[] = "";
static char const gem_rsc_string_46[] = "";
static char const gem_rsc_string_47[] = "";
static char const gem_rsc_string_48[] = "";
static char const gem_rsc_string_49[] = "";
static char const gem_rsc_string_50[] = "";
static char const gem_rsc_string_51[] = "X";
static char const gem_rsc_string_52[] = "";
static char const gem_rsc_string_53[] = "";
static char const gem_rsc_string_54[] = N_("[1][The disk in drive %c: is|physically write-protected.][Cancel|Retry]");
static char const gem_rsc_string_55[] = N_("[2][Drive %c: is not responding.|Please check the disk drive,|or insert a disk.][Cancel|Retry]");
static char const gem_rsc_string_56[] = N_("[1][Data on the disk in drive %c:|may be damaged.][Cancel|Retry]");
static char const gem_rsc_string_57[] = N_("[2][This application cannot read|data on the disk in drive %c:.][Cancel|Retry]");
static char const gem_rsc_string_58[] = N_("[1][Your output device is not|receiving data.][Cancel|Retry]");
static char const gem_rsc_string_59[] = N_("[3][An error has occurred in GEM.|Please contact the EmuTOS|Development Team.][Cancel]");
static char const gem_rsc_string_60[] = N_("[2][This application cannot|find the folder or file|you just tried to access.][  OK  ]");
static char const gem_rsc_string_61[] = N_("[1][This application does not|have room to open another|document.  To make room,|close any document that|you do not need.][  OK  ]");
static char const gem_rsc_string_62[] = N_("[1][An item with this name|already exists in the|directory, or this item|is set to Read Only status.][  OK  ]");
static char const gem_rsc_string_63[] = N_("[1][The drive you specified|does not exist.][ Cancel ]");
static char const gem_rsc_string_64[] = N_("[1][There is not enough memory|in your computer for the|application you just tried|to run.][  OK  ]");
static char const gem_rsc_string_65[] = N_("[3][TOS error #%u.][Cancel]");
static char const gem_rsc_string_66[] = N_("[3][Unsupported AES function #%d.][Cancel]");
static char const gem_rsc_string_67[] = N_("[3][Please insert disk %c|into drive A:.][  OK  ]");
static char const gem_rsc_string_68[] = N_("ITEM SELECTOR");
static char const gem_rsc_string_69[] = "0..9";
static char const gem_rsc_string_70[] = N_("A..Z \200\216\217\220\222\231\232\245\265\266\267\270\236\302..\334");
static char const gem_rsc_string_71[] = N_("0..9A..Z \200\216\217\220\222\231\232\245\265\266\267\270\236\302..\334");
static char const gem_rsc_string_72[] = N_("0..9a..zA..Z\200..\377\\?*:._");
static char const gem_rsc_string_73[] = N_("0..9a..zA..Z\200..\377\\:_");
static char const gem_rsc_string_74[] = N_("a..z0..9A..Z\200..\377:?*_");
static char const gem_rsc_string_75[] = N_("a..z0..9A..Z\200..\377_");
static char const gem_rsc_string_76[] = N_("a..zA..Z \200..\377");
static char const gem_rsc_string_77[] = N_("0..9a..zA..Z \200..\377");
static char const gem_rsc_string_78[] = N_("[1][The system does not have |enough memory for this|directory.][  OK  ]");


/* data of NOTEBB */
static _UWORD const gem_rsc_IMAGE0[] = {
0x0000, 0x0000, 0x0001, 0x8000, 0x0002, 0x4000, 0x0002, 0x4000, 
0x0004, 0x2000, 0x0005, 0xA000, 0x0009, 0x9000, 0x000B, 0xD000, 
0x0013, 0xC800, 0x0017, 0xE800, 0x0026, 0x6400, 0x002C, 0x3400, 
0x004C, 0x3200, 0x005C, 0x3A00, 0x009C, 0x3900, 0x00BC, 0x3D00, 
0x013C, 0x3C80, 0x017E, 0x7E80, 0x027E, 0x7E40, 0x02FE, 0x7F40, 
0x04FE, 0x7F20, 0x05FE, 0x7FA0, 0x09FF, 0xFF90, 0x0BFF, 0xFFD0, 
0x13FE, 0x7FC8, 0x17FC, 0x3FE8, 0x27FC, 0x3FE4, 0x2FFE, 0x7FF4, 
0x4FFF, 0xFFF2, 0x4000, 0x0002, 0x7FFF, 0xFFFE, 0x0000, 0x0000};

/* data of QUESTBB */
static _UWORD const gem_rsc_IMAGE1[] = {
0x0000, 0x0000, 0x7FFF, 0xFFFE, 0x4000, 0x0002, 0x4FFF, 0xFFF2, 
0x2FFF, 0xFFF4, 0x27F8, 0x3FE4, 0x17E0, 0x1FE8, 0x13C0, 0x0FC8, 
0x0BC3, 0x07D0, 0x09E7, 0x8790, 0x05FF, 0x87A0, 0x04FF, 0x8720, 
0x02FF, 0x0F40, 0x027E, 0x1E40, 0x017C, 0x3E80, 0x013C, 0x7C80, 
0x00BC, 0x7D00, 0x009F, 0xF900, 0x005E, 0x7A00, 0x004C, 0x3200, 
0x002C, 0x3400, 0x0026, 0x6400, 0x0017, 0xE800, 0x0013, 0xC800, 
0x000B, 0xD000, 0x0009, 0x9000, 0x0005, 0xA000, 0x0004, 0x2000, 
0x0002, 0x4000, 0x0002, 0x4000, 0x0001, 0x8000, 0x0000, 0x0000};

/* data of STOPBB */
static _UWORD const gem_rsc_IMAGE2[] = {
0x0000, 0x0000, 0x0000, 0x0000, 0x003F, 0xFC00, 0x0040, 0x0200, 
0x009F, 0xF900, 0x013F, 0xFC80, 0x027F, 0xFE40, 0x04FF, 0xFF20, 
0x09FF, 0xFF90, 0x13FF, 0xFFC8, 0x27EF, 0xF7E4, 0x2FC7, 0xE3F4, 
0x2FE3, 0xC7F4, 0x2FF1, 0x8FF4, 0x2FF8, 0x1FF4, 0x2FFC, 0x3FF4, 
0x2FFC, 0x3FF4, 0x2FF8, 0x1FF4, 0x2FF1, 0x8FF4, 0x2FE3, 0xC7F4, 
0x2FC7, 0xE3F4, 0x27EF, 0xF7E4, 0x13FF, 0xFFC8, 0x09FF, 0xFF90, 
0x04FF, 0xFF20, 0x027F, 0xFE40, 0x013F, 0xFC80, 0x009F, 0xF900, 
0x0040, 0x0200, 0x003F, 0xFC00, 0x0000, 0x0000, 0x0000, 0x0000};

/* data of NOTEBB_TOS */
static _UWORD const gem_rsc_IMAGE3[] = {
0x0003, 0xC000, 0x0006, 0x6000, 0x000D, 0xB000, 0x001B, 0xD800, 
0x0037, 0xEC00, 0x006F, 0xF600, 0x00DC, 0x3B00, 0x01BC, 0x3D80, 
0x037C, 0x3EC0, 0x06FC, 0x3F60, 0x0DFC, 0x3FB0, 0x1BFC, 0x3FD8, 
0x37FC, 0x3FEC, 0x6FFC, 0x3FF6, 0xDFFC, 0x3FFB, 0xBFFC, 0x3FFD, 
0xBFFC, 0x3FFD, 0xDFFC, 0x3FFB, 0x6FFC, 0x3FF6, 0x37FC, 0x3FEC, 
0x1BFF, 0xFFD8, 0x0DFF, 0xFFB0, 0x06FC, 0x3F60, 0x037C, 0x3EC0, 
0x01BC, 0x3D80, 0x00DC, 0x3B00, 0x006F, 0xF600, 0x0037, 0xEC00, 
0x001B, 0xD800, 0x000D, 0xB000, 0x0006, 0x6000, 0x0003, 0xC000};

/* data of QUESTBB_TOS */
static _UWORD const gem_rsc_IMAGE4[] = {
0x3FFF, 0xFFFC, 0xC000, 0x0003, 0x9FFF, 0xFFF9, 0xBFFF, 0xFFFD, 
0xDFF8, 0x3FFB, 0x5FE0, 0x0FFA, 0x6FC0, 0x07F6, 0x2F83, 0x83F4, 
0x3787, 0xC3EC, 0x1787, 0xC3E8, 0x1BFF, 0x83D8, 0x0BFF, 0x07D0, 
0x0DFE, 0x0FB0, 0x05FC, 0x1FA0, 0x06FC, 0x3F60, 0x02FC, 0x3F40, 
0x037C, 0x3EC0, 0x017C, 0x3E80, 0x01BF, 0xFD80, 0x00BF, 0xFD00, 
0x00DC, 0x3B00, 0x005C, 0x3A00, 0x006C, 0x3600, 0x002F, 0xF400, 
0x0037, 0xEC00, 0x0017, 0xE800, 0x001B, 0xD800, 0x000B, 0xD000, 
0x000D, 0xB000, 0x0005, 0xA000, 0x0006, 0x6000, 0x0003, 0xC000};

/* data of STOPBB_TOS */
static _UWORD const gem_rsc_IMAGE5[] = {
0x007F, 0xFE00, 0x00C0, 0x0300, 0x01BF, 0xFD80, 0x037F, 0xFEC0, 
0x06FF, 0xFF60, 0x0DFF, 0xFFB0, 0x1BFF, 0xFFD8, 0x37FF, 0xFFEC, 
0x6FFF, 0xFFF6, 0xDFFF, 0xFFFB, 0xB181, 0x860D, 0xA081, 0x0205, 
0xA4E7, 0x3265, 0xA7E7, 0x3265, 0xA3E7, 0x3265, 0xB1E7, 0x3205, 
0xB8E7, 0x320D, 0xBCE7, 0x327D, 0xA4E7, 0x327D, 0xA0E7, 0x027D, 
0xB1E7, 0x867D, 0xBFFF, 0xFFFD, 0xDFFF, 0xFFFB, 0x6FFF, 0xFFF6, 
0x37FF, 0xFFEC, 0x1BFF, 0xFFD8, 0x0DFF, 0xFFB0, 0x06FF, 0xFF60, 
0x037F, 0xFEC0, 0x01BF, 0xFD80, 0x00C0, 0x0300, 0x007F, 0xFE00};

/* data of MICE00 */
static _UWORD const gem_rsc_IMAGE6[] = {
0x0000, 0x0000, 0x0001, 0x0000, 0x0001, 0xE000, 0xF000, 0xF800, 
0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFC0, 0xFE00, 0xFE00, 0xEF00, 
0x0F00, 0x0780, 0x0780, 0x03C0, 0x03C0, 0x4000, 0x6000, 0x7000, 
0x7800, 0x7C00, 0x7E00, 0x7F00, 0x7F80, 0x7C00, 0x6C00, 0x4600, 
0x0600, 0x0300, 0x0300, 0x0180, 0x0180};

/* data of MICE01 */
static _UWORD const gem_rsc_IMAGE7[] = {
0x0007, 0x0007, 0x0001, 0x0000, 0x0001, 0xFFFF, 0xFFFF, 0x07E0, 
0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 
0x03C0, 0x03C0, 0x07E0, 0xFFFF, 0xFFFF, 0x7C3E, 0x0660, 0x03C0, 
0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 
0x0180, 0x0180, 0x03C0, 0x0660, 0x7C3E};

/* data of MICE02 */
static _UWORD const gem_rsc_IMAGE8[] = {
0x0008, 0x0008, 0x0001, 0x0000, 0x0001, 0xFFFF, 0xFFFF, 0xFFFF, 
0x7FFE, 0x7FFE, 0x3FFC, 0x1FF8, 0x0EF0, 0x0FF0, 0x1FF8, 0x3FFC, 
0x7FFE, 0x7FFE, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x7FFE, 0x2004, 
0x1008, 0x1448, 0x0AB0, 0x0560, 0x02C0, 0x0340, 0x04A0, 0x0910, 
0x1088, 0x12A8, 0x3554, 0x7FFE, 0x0000};

/* data of MICE03 */
static _UWORD const gem_rsc_IMAGE9[] = {
0x0000, 0x0000, 0x0001, 0x0000, 0x0001, 0xF000, 0xF800, 0x7C00, 
0x3E07, 0x1F0F, 0x0F9E, 0x07DE, 0x07FE, 0x1FFF, 0x3FFF, 0x7FFF, 
0x7FFE, 0x3FFE, 0x1FFE, 0x0FFF, 0x01FF, 0xF000, 0x8800, 0x4400, 
0x2207, 0x1109, 0x0892, 0x0452, 0x0632, 0x1913, 0x2481, 0x5241, 
0x4982, 0x2602, 0x1806, 0x0E03, 0x0180};

/* data of MICE04 */
static _UWORD const gem_rsc_IMAGE10[] = {
0x0008, 0x0008, 0x0001, 0x0000, 0x0001, 0x0000, 0x0380, 0x03F0, 
0x33FE, 0x7BFF, 0x7BFF, 0x3FFF, 0xFFFF, 0xFFFF, 0x7FFF, 0x3FFF, 
0x3FFF, 0x1FFE, 0x07FC, 0x07FC, 0x0FFE, 0x0000, 0x0380, 0x0270, 
0x324E, 0x4A49, 0x4A49, 0x2649, 0xF249, 0x9801, 0x4C01, 0x2001, 
0x2001, 0x1802, 0x0404, 0x0404, 0x0FFE};

/* data of MICE05 */
static _UWORD const gem_rsc_IMAGE11[] = {
0x0007, 0x0007, 0x0001, 0x0000, 0x0001, 0x0000, 0x0380, 0x0380, 
0x0380, 0x0380, 0x0380, 0x7FFC, 0x7FFC, 0x7FFC, 0x0380, 0x0380, 
0x0380, 0x0380, 0x0380, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
0x0100, 0x0100, 0x0100, 0x0100, 0x1FF0, 0x0100, 0x0100, 0x0100, 
0x0100, 0x0000, 0x0000, 0x0000, 0x0000};

/* data of MICE06 */
static _UWORD const gem_rsc_IMAGE12[] = {
0x0008, 0x0008, 0x0001, 0x0000, 0x0001, 0x0000, 0x07C0, 0x07C0, 
0x07C0, 0x07C0, 0x07C0, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 
0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x07C0, 0x0000, 0x0000, 0x0380, 
0x0380, 0x0380, 0x0380, 0x0380, 0x7FFC, 0x7FFC, 0x7FFC, 0x0380, 
0x0380, 0x0380, 0x0380, 0x0380, 0x0000};

/* data of MICE07 */
static _UWORD const gem_rsc_IMAGE13[] = {
0x0008, 0x0008, 0x0001, 0x0000, 0x0001, 0x07E0, 0x07E0, 0x0660, 
0x0660, 0x0660, 0xFE7F, 0xFE7F, 0xC003, 0xC003, 0xFE7F, 0xFE7F, 
0x0660, 0x0660, 0x0660, 0x07E0, 0x07E0, 0x0000, 0x03C0, 0x0240, 
0x0240, 0x0240, 0x0240, 0x7E7E, 0x4002, 0x4002, 0x7E7E, 0x0240, 
0x0240, 0x0240, 0x0240, 0x03C0, 0x0000};


const char *const rs_fstr[] = {
	gem_rsc_string_54,
	gem_rsc_string_55,
	gem_rsc_string_56,
	gem_rsc_string_57,
	gem_rsc_string_58,
	gem_rsc_string_59,
	gem_rsc_string_60,
	gem_rsc_string_61,
	gem_rsc_string_62,
	gem_rsc_string_63,
	gem_rsc_string_64,
	gem_rsc_string_65,
	gem_rsc_string_66,
	gem_rsc_string_67,
	gem_rsc_string_68,
	gem_rsc_string_69,
	gem_rsc_string_70,
	gem_rsc_string_71,
	gem_rsc_string_72,
	gem_rsc_string_73,
	gem_rsc_string_74,
	gem_rsc_string_75,
	gem_rsc_string_76,
	gem_rsc_string_77,
	gem_rsc_string_78
};


static BITBLK const rs_bitblk[NUM_BB] = {
	{ CP gem_rsc_IMAGE0, 4, 32, 0, 0, 1 },
	{ CP gem_rsc_IMAGE1, 4, 32, 0, 0, 1 },
	{ CP gem_rsc_IMAGE2, 4, 32, 0, 0, 1 },
	{ CP gem_rsc_IMAGE3, 4, 32, 0, 0, 1 },
	{ CP gem_rsc_IMAGE4, 4, 32, 0, 0, 1 },
	{ CP gem_rsc_IMAGE5, 4, 32, 0, 0, 1 },
	{ CP gem_rsc_IMAGE6, 2, 37, 0, 0, 3 },
	{ CP gem_rsc_IMAGE7, 2, 37, 0, 0, 3 },
	{ CP gem_rsc_IMAGE8, 2, 37, 0, 0, 3 },
	{ CP gem_rsc_IMAGE9, 2, 37, 0, 0, 3 },
	{ CP gem_rsc_IMAGE10, 2, 37, 0, 0, 3 },
	{ CP gem_rsc_IMAGE11, 2, 37, 0, 0, 3 },
	{ CP gem_rsc_IMAGE12, 2, 37, 0, 0, 3 },
	{ CP gem_rsc_IMAGE13, 2, 37, 0, 0, 3 }
};


static const BITBLK *const rs_frimg[NUM_FRIMG] = {
	&rs_bitblk[0],
	&rs_bitblk[1],
	&rs_bitblk[2],
	&rs_bitblk[3],
	&rs_bitblk[4],
	&rs_bitblk[5],
	&rs_bitblk[6],
	&rs_bitblk[7],
	&rs_bitblk[8],
	&rs_bitblk[9],
	&rs_bitblk[10],
	&rs_bitblk[11],
	&rs_bitblk[12],
	&rs_bitblk[13]
};


static TEDINFO const rs_tedinfo_rom[NUM_TI] = {
	{ (char*)gem_rsc_string_2, (char*)gem_rsc_string_3, (char*)gem_rsc_string_4, IBM, 1, TE_LEFT, 0x1100, 0x0, 0, 39,39 }, /* FSDIRECT */
	{ (char*)gem_rsc_string_5, (char*)gem_rsc_string_6, (char*)gem_rsc_string_7, IBM, 6, TE_CNTR, 0x1180, 0x0, -1, 1,7 }, /* FSDRIVE */
	{ (char*)gem_rsc_string_8, (char*)gem_rsc_string_9, (char*)gem_rsc_string_10, IBM, 1, TE_LEFT, 0x1100, 0x0, 0, 12,24 }, /* FSSELECT */
	{ (char*)gem_rsc_string_11, (char*)gem_rsc_string_12, (char*)gem_rsc_string_13, IBM, 6, TE_CNTR, 0x11A1, 0x0, -1, 1,1 }, /* FTITLE */
	{ (char*)gem_rsc_string_14, (char*)gem_rsc_string_15, (char*)gem_rsc_string_16, IBM, 1, TE_LEFT, 0x1100, 0x0, 0, 13,16 }, /* F1NAME */
	{ (char*)gem_rsc_string_17, (char*)gem_rsc_string_18, (char*)gem_rsc_string_19, IBM, 1, TE_LEFT, 0x1100, 0x0, 0, 13,16 }, /* F2NAME */
	{ (char*)gem_rsc_string_20, (char*)gem_rsc_string_21, (char*)gem_rsc_string_22, IBM, 1, TE_LEFT, 0x1100, 0x0, 0, 13,16 }, /* F3NAME */
	{ (char*)gem_rsc_string_23, (char*)gem_rsc_string_24, (char*)gem_rsc_string_25, IBM, 1, TE_LEFT, 0x1100, 0x0, 0, 13,16 }, /* F4NAME */
	{ (char*)gem_rsc_string_26, (char*)gem_rsc_string_27, (char*)gem_rsc_string_28, IBM, 1, TE_LEFT, 0x1100, 0x0, 0, 13,16 }, /* F5NAME */
	{ (char*)gem_rsc_string_29, (char*)gem_rsc_string_30, (char*)gem_rsc_string_31, IBM, 1, TE_LEFT, 0x1100, 0x0, 0, 13,16 }, /* F6NAME */
	{ (char*)gem_rsc_string_32, (char*)gem_rsc_string_33, (char*)gem_rsc_string_34, IBM, 1, TE_LEFT, 0x1100, 0x0, 0, 13,16 }, /* F7NAME */
	{ (char*)gem_rsc_string_35, (char*)gem_rsc_string_36, (char*)gem_rsc_string_37, IBM, 1, TE_LEFT, 0x1100, 0x0, 0, 13,16 }, /* F8NAME */
	{ (char*)gem_rsc_string_38, (char*)gem_rsc_string_39, (char*)gem_rsc_string_40, IBM, 1, TE_LEFT, 0x1100, 0x0, 0, 13,16 }, /* F9NAME */
	{ (char*)gem_rsc_string_51, (char*)gem_rsc_string_52, (char*)gem_rsc_string_53, IBM, 1, TE_CNTR, 0x1100, 0x0, 1, 2,1 } /* APPTITLE */
};


static OBJECT const rs_object_rom[NUM_OBS] = {
/* FSELECTR */

	{ -1, 1, 52, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0,0, 40,20 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(gem_rsc_string_0), 1,2048, 1,1 }, /* FSTITLE */
	{ 3, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(gem_rsc_string_1), 1,2, 10,1 },
	{ 4, -1, -1, G_FBOXTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[0]), 1,1027, 38,1 }, /* FSDIRECT */
	{ 5, -1, -1, G_FTEXT, OF_NONE, OS_NORMAL, C_UNION(&rs_tedinfo[1]), 27,5, 11,1 }, /* FSDRIVE */
	{ 6, -1, -1, G_FBOXTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(&rs_tedinfo[2]), 1,5, 24,1 }, /* FSSELECT */
	{ 33, 7, 32, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 27,6, 11,9 }, /* FSDRIVES */
	{ 8, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x41FF1100L), 0,0, 3,1 }, /* FS1STDRV */
	{ 9, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x42FF1100L), 0,1, 3,1 },
	{ 10, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x43FF1100L), 0,2, 3,1 },
	{ 11, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x44FF1100L), 0,3, 3,1 },
	{ 12, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x45FF1100L), 0,4, 3,1 },
	{ 13, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x46FF1100L), 0,5, 3,1 },
	{ 14, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x47FF1100L), 0,6, 3,1 },
	{ 15, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x48FF1100L), 0,7, 3,1 },
	{ 16, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x49FF1100L), 0,8, 3,1 },
	{ 17, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x4AFF1100L), 4,0, 3,1 },
	{ 18, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x4BFF1100L), 4,1, 3,1 },
	{ 19, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x4CFF1100L), 4,2, 3,1 },
	{ 20, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x4DFF1100L), 4,3, 3,1 },
	{ 21, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x4EFF1100L), 4,4, 3,1 },
	{ 22, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x4FFF1100L), 4,5, 3,1 },
	{ 23, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x50FF1100L), 4,6, 3,1 },
	{ 24, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x51FF1100L), 4,7, 3,1 },
	{ 25, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x52FF1100L), 4,8, 3,1 },
	{ 26, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x53FF1100L), 8,0, 3,1 },
	{ 27, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x54FF1100L), 8,1, 3,1 },
	{ 28, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x55FF1100L), 8,2, 3,1 },
	{ 29, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x56FF1100L), 8,3, 3,1 },
	{ 30, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x57FF1100L), 8,4, 3,1 },
	{ 31, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x58FF1100L), 8,5, 3,1 },
	{ 32, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x59FF1100L), 8,6, 3,1 },
	{ 6, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x5AFF1100L), 8,7, 3,1 }, /* FSLSTDRV */
	{ 51, 34, 41, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 3,7, 22,12 },
	{ 35, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x5FF1101L), 0,0, 2,1 }, /* FCLSBOX */
	{ 36, -1, -1, G_BOXTEXT, OF_TOUCHEXIT, OS_NORMAL, C_UNION(&rs_tedinfo[3]), 2,0, 20,1 }, /* FTITLE */
	{ 41, 37, 39, G_BOX, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x11100L), 19,1, 3,11 }, /* SCRLBAR */
	{ 38, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x1FF1100L), 0,0, 3,2 }, /* FUPAROW */
	{ 39, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x2FF1100L), 0,9, 3,2 }, /* FDNAROW */
	{ 36, 40, 40, G_BOX, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0xFF1111L), 0,2, 3,7 }, /* FSVSLID */
	{ 39, -1, -1, G_BOX, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x11101L), 0,0, 3,1 }, /* FSVELEV */
	{ 33, 42, 50, G_BOX, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0xFF1100L), 0,1, 19,11 }, /* FILEBOX */
	{ 43, -1, -1, G_FBOXTEXT, OF_TOUCHEXIT, OS_NORMAL, C_UNION(&rs_tedinfo[4]), 2,1, 15,1 }, /* F1NAME */
	{ 44, -1, -1, G_FBOXTEXT, OF_TOUCHEXIT, OS_NORMAL, C_UNION(&rs_tedinfo[5]), 2,2, 15,1 }, /* F2NAME */
	{ 45, -1, -1, G_FBOXTEXT, OF_TOUCHEXIT, OS_NORMAL, C_UNION(&rs_tedinfo[6]), 2,3, 15,1 }, /* F3NAME */
	{ 46, -1, -1, G_FBOXTEXT, OF_TOUCHEXIT, OS_NORMAL, C_UNION(&rs_tedinfo[7]), 2,4, 15,1 }, /* F4NAME */
	{ 47, -1, -1, G_FBOXTEXT, OF_TOUCHEXIT, OS_NORMAL, C_UNION(&rs_tedinfo[8]), 2,5, 15,1 }, /* F5NAME */
	{ 48, -1, -1, G_FBOXTEXT, OF_TOUCHEXIT, OS_NORMAL, C_UNION(&rs_tedinfo[9]), 2,6, 15,1 }, /* F6NAME */
	{ 49, -1, -1, G_FBOXTEXT, OF_TOUCHEXIT, OS_NORMAL, C_UNION(&rs_tedinfo[10]), 2,7, 15,1 }, /* F7NAME */
	{ 50, -1, -1, G_FBOXTEXT, OF_TOUCHEXIT, OS_NORMAL, C_UNION(&rs_tedinfo[11]), 2,8, 15,1 }, /* F8NAME */
	{ 41, -1, -1, G_FBOXTEXT, OF_TOUCHEXIT, OS_NORMAL, C_UNION(&rs_tedinfo[12]), 2,9, 15,1 }, /* F9NAME */
	{ 52, -1, -1, G_BUTTON, 0x7, OS_NORMAL, C_UNION(gem_rsc_string_41), 28,16, 8,1 }, /* FSOK */
	{ 0, -1, -1, G_BUTTON, 0x25, OS_NORMAL, C_UNION(gem_rsc_string_42), 28,18, 8,1 }, /* FSCANCEL */

/* DIALERT */

	{ -1, 1, 9, G_BOX, OF_NONE, OS_SHADOWED, C_UNION(0x21100L), 0,0, 80,9 },
	{ 2, -1, -1, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 3,1, 4,4 }, /* ALICON */
	{ 3, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(gem_rsc_string_43), 9,1, 40,1 }, /* MSGOFF */
	{ 4, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(gem_rsc_string_44), 9,2, 50,1 },
	{ 5, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(gem_rsc_string_45), 9,3, 50,1 },
	{ 6, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(gem_rsc_string_46), 9,4, 50,1 },
	{ 7, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(gem_rsc_string_47), 9,5, 50,1 },
	{ 8, -1, -1, G_BUTTON, 0x7, OS_NORMAL, C_UNION(gem_rsc_string_48), 9,7, 16,1 }, /* BUTOFF */
	{ 9, -1, -1, G_BUTTON, 0x7, OS_NORMAL, C_UNION(gem_rsc_string_49), 26,7, 16,1 },
	{ 0, -1, -1, G_BUTTON, 0x27, OS_NORMAL, C_UNION(gem_rsc_string_50), 43,7, 16,1 },

/* DESKTOP */

	{ -1, 1, 2, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1143L), 0,0, 80,25 },
	{ 2, -1, -1, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0,0, 80,513 },
	{ 0, -1, -1, G_TEXT, OF_LASTOB, OS_NORMAL, C_UNION(&rs_tedinfo[13]), 0,0, 80,769 } /* APPTITLE */
};


static OBJECT *const rs_trindex[NUM_TREE] = {
	&rs_object[0], /* FSELECTR */
	&rs_object[53], /* DIALERT */
	&rs_object[63] /* DESKTOP */
};





#if RSC_STATIC_FILE

#if RSC_NAMED_FUNCTIONS
#ifdef __STDC__
_WORD gem_rsc_load(_WORD wchar, _WORD hchar)
#else
_WORD gem_rsc_load(wchar, hchar)
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
_WORD gem_rsc_gaddr(_WORD type, _WORD idx, void *gaddr)
#else
_WORD gem_rsc_gaddr(type, idx, gaddr)
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
_WORD gem_rsc_free(void)
#else
_WORD gem_rsc_free()
#endif
{
#if NUM_OBS != 0
	hrelease_objs(rs_object, NUM_OBS);
#endif
	return 1;
}

#endif /* RSC_NAMED_FUNCTIONS */

#else /* !RSC_STATIC_FILE */
_WORD rs_numstrings = 79;
_WORD rs_numfrstr = 25;

_WORD rs_nuser = 0;
_WORD rs_numimages = 14;
_WORD rs_numbb = 14;
_WORD rs_numfrimg = 14;
_WORD rs_numib = 0;
_WORD rs_numcib = 0;
_WORD rs_numti = 14;
_WORD rs_numobs = 66;
_WORD rs_numtree = 3;

char rs_name[] = "gem.rsc";

_WORD _rsc_format = 2; /* RSC_FORM_SOURCE2 */
#endif /* RSC_STATIC_FILE */
