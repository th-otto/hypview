/*****************************************************************************/
/*                                                                           */
/* PORTAB.H                                                                  */
/*                                                                           */
/* Use of this file may make your code compatible with all C compilers       */
/* listed.                                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* ENVIRONMENT                                                               */
/*****************************************************************************/

#ifndef __PORTAB_H__
#define __PORTAB_H__

#include "windows_.h"

#ifndef __STDIO_H__
#  include <stdio.h>
#endif
#ifndef __STRING_H__
#  include <string.h>
#endif
#ifndef __STDLIB_H__
#  include <stdlib.h>
#endif
#include <limits.h>

#define GEMDOS     1                          /* Digital Research GEMDOS     */

#define M68000     1                          /* Motorola Processing Unit    */
#define I8086      0                          /* Intel Processing Unit       */

#define TURBO_C    0                          /* Turbo C Compiler            */
#define PCC        1                          /* Portable C-Compiler         */

#define GEM1       0x0001                     /* ATARI GEM version           */
#define GEM2       0x0002                     /* MSDOS GEM 2.x versions      */
#define GEM3       0x0004                     /* MSDOS GEM/3 version         */
#define XGEM       0x0100                     /* OS/2,FlexOS X/GEM version   */

#ifndef GEM
#if (defined(GEMDOS) && GEMDOS)
#define GEM        GEM1                       /* GEMDOS default is GEM1      */
#endif /* GEMDOS */

#if defined(MSDOS) && MSDOS
#define GEM        GEM3                       /* MSDOS default is GEM3       */
#endif /* MSDOS */

#if defined(OS2) && OS2
#define GEM        XGEM                       /* OS/2 default is X/GEM       */
#endif /* MSDOS */

#if defined(FLEXOS) || defined(UNIX)
#define GEM        XGEM                       /* FlexOS default is X/GEM     */
#endif /* FLEXOS */
#endif /* GEM */

#ifndef __GNUC_PREREQ
# ifdef __GNUC__
#   define __GNUC_PREREQ(maj, min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#   define __GNUC_PREREQ(maj, min) 0
# endif
#endif

/*****************************************************************************/
/* STANDARD TYPE DEFINITIONS                                                 */
/*****************************************************************************/

#define _CHAR   signed char                   /* Signed byte                 */
#define _UCHAR  unsigned char                 /* Unsigned byte               */

#define _BYTE	signed char
#define _UBYTE	unsigned char

#if !defined(__PUREC__) && !defined(__USE_GEMLIB)
#define __USE_GEMLIB 1
#endif
#ifdef __USE_GEMLIB
#define _WORD   short                         /* Signed word (16 bits)       */
#define _UWORD  unsigned short                /* Unsigned word               */
#else
#define _WORD   int                           /* Signed word (16 bits)       */
#define _UWORD  unsigned int                  /* Unsigned word               */
#endif
#ifndef _VOID
#define _VOID void
#endif
#ifndef _BOOL
#define _BOOL int
#endif

#ifndef _LPVOID
#define _LPVOID void *
#endif

#ifndef _LPBYTE
#  define _LPBYTE char *
#endif

#define _LONG   long                          /* Signed long (32 bits)       */
#define _ULONG  unsigned long                 /* Unsigned long               */

#define EXTERN  extern                        /* External variable           */
#define LOCAL   static                        /* Local to module             */
#define MLOCAL  LOCAL                         /* Local to module             */
#define GLOBAL                                /* Global variable             */

/*****************************************************************************/
/* COMPILER DEPENDENT DEFINITIONS                                            */
/*****************************************************************************/

#if GEMDOS                                    /* GEMDOS compilers            */

#if TURBO_C
#define graf_mbox graf_movebox                /* Wrong GEM binding           */
#define graf_rubbox graf_rubberbox            /* Wrong GEM binding           */
#endif /* TURBO_C */

#endif /* GEMDOS */


#define CONST    const
#define VOLATILE volatile
#undef CDECL
#define CDECL    cdecl
#ifndef __CDECL
#define __CDECL  cdecl
#endif
#ifdef __NO_CDECL
#define _CDECL
#else
#define _CDECL	 __CDECL
#endif
#define _PASCAL  pascal

/*****************************************************************************/
/* MISCELLANEOUS DEFINITIONS                                                 */
/*****************************************************************************/

#ifndef FALSE
#define FALSE   0                             /* Function FALSE value        */
#define TRUE    1                             /* Function TRUE  value        */
#endif

#define FAILURE (-1)                          /* Function failure return val */
#define SUCCESS 0                             /* Function success return val */
#define FOREVER for (;;)                      /* Infinite loop declaration   */
#define EOS     '\0'                          /* End of string value         */


#ifndef EOF
#define EOF     (-1)                          /* EOF value                   */
#endif

#if defined(ULONG_MAX) && ULONG_MAX > 0x7ffffffful
#  define _LONG_PTR  intptr_t
#else
#  define _LONG_PTR _LONG
#endif

#define LOCAL static
#define RLOCAL LOCAL
#define GLOBAL /**/
#define EXTERN extern
#define _HUGE 
#define EXP_PTR
#define EXP_PROC

#define FUNK_NULL 0l

#ifndef UNUSED
# define UNUSED(x) (void)(x)
#endif

#if __GNUC_PREREQ(2, 7)
#  define PACKED __attribute__((packed))
#endif

#ifndef PACKED
#  define PACKED /**/
#endif

#define BigEndian 1
#ifndef BigEndian
#  define BigEndian (is_big_endian())
#  define IfBigEndian if (BigEndian)
#  define IfNotBigEndian if (!BigEndian)
_BOOL is_big_endian (void);
#else
#  if BigEndian
#    define IfBigEndian /**/
#    define IfNotBigEndian if (stdout){} else
#  else
#    define IfBigEndian if (stdout){} else
#    define IfNotBigEndian /**/
#  endif
#endif

#define ATARI 1
#define PU_MOTOROLA  1			/* Motorola Processing Unit */
#define NO_GEM 0

#ifndef __attribute__
#  ifndef __GNUC__
#    define __attribute__(x)
#  endif
#endif

#ifdef __GNUC__
#  define NOINLINE __attribute__((noinline))
#else
#  define NOINLINE /**/
#endif

#ifdef __PUREC__
#  define ANONYMOUS_STRUCT_DUMMY(x) struct x { int dummy; };
#endif

#ifndef ANONYMOUS_STRUCT_DUMMY
#  define ANONYMOUS_STRUCT_DUMMY(x)
#endif

#define STDC_HEADERS 1
#define HAVE_STRING_H 1
#define HAVE_STRSTR

#define ALL_FILE_MASK "*.*"

#define INLINE

#ifndef NO_CONST
#  ifdef __GNUC__
#    define NO_CONST(p) __extension__({ union { CONST void *cs; void *s; } x; x.cs = p; x.s; })
#  else
#    define NO_CONST(p) ((void *)(p))
#  endif
#endif

#if defined(HAVE_GTK) || defined(HAVE_GLIB)
#include <glib.h>
#else
#include <stdint.h>
#endif
#ifndef __G_LIB_H__
typedef int gboolean;
typedef uint32_t gunichar;
typedef unsigned short gunichar2;
typedef void *gpointer;
typedef const void *gconstpointer;
#endif

#ifdef __cplusplus
#  define EXTERN_C_BEG extern "C" {
#  define EXTERN_C_END }
#else
#  define EXTERN_C_BEG
#  define EXTERN_C_END
#endif

#define HOST_BYTE_ORDER BYTE_ORDER_BIG_ENDIAN

#endif /* __PORTAB_H__ */
