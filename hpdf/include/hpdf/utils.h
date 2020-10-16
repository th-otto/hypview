/*
 * << Haru Free PDF Library >> -- fpdf_utils.h
 *
 * URL: http://libharu.org
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 * Copyright (c) 2007-2009 Antony Dovgal <tony@daylessday.org>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */

#ifndef _HPDF_UTILS_H
#define _HPDF_UTILS_H

#include "hpdf/types.h"

#ifdef __cplusplus
extern "C" {
#endif									/* __cplusplus */

#define HPDF_UNUSED(a) ((void)(a))


HPDF_INT HPDF_AToI(const char *s);


HPDF_DOUBLE HPDF_AToF(const char *s);


char *HPDF_IToA(char *s, HPDF_INT32 val, char *eptr);


char *HPDF_IToA2(char *s, HPDF_UINT32 val, HPDF_UINT len);


char *HPDF_FToA(char *s, HPDF_REAL val, char *eptr);


const char *HPDF_StrStr(const char *s1, const char *s2, HPDF_UINT maxlen);


HPDF_UINT HPDF_StrLen(const char *s, HPDF_INT maxlen);


HPDF_BYTE *HPDF_StrCpy(char *out, const char *in, char *eptr);


void HPDF_ToBox(HPDF_Box *out, HPDF_INT16 left, HPDF_INT16 bottom, HPDF_INT16 right, HPDF_INT16 top);


void HPDF_UInt16Swap(HPDF_UINT16 *value);


#define HPDF_NEEDS_ESCAPE(c)    (c < 0x20 || \
                                 c > 0x7e || \
                                 c == '\\' || \
                                 c == '%' || \
                                 c == '#' || \
                                 c == '/' || \
                                 c == '(' || \
                                 c == ')' || \
                                 c == '<' || \
                                 c == '>' || \
                                 c == '[' || \
                                 c == ']' || \
                                 c == '{' || \
                                 c == '}' )  \

#define HPDF_IS_WHITE_SPACE(c)   (c == 0x00 || \
                                 c == 0x09 || \
                                 c == 0x0A || \
                                 c == 0x0C || \
                                 c == 0x0D || \
                                 c == 0x20 ) \

/*----------------------------------------------------------------------------*/
/*----- macros for debug -----------------------------------------------------*/

#ifdef LIBHPDF_DEBUG
#define HPDF_PRINT_BINARY(BUF, LEN, CAPTION) HPDF_PrintBinary(BUF, LEN, CAPTION)
#else
#define HPDF_PRINT_BINARY(BUF, LEN, CAPTION)	/* do nothing */
#endif

#ifdef __cplusplus
}
#endif

#endif /* _HPDF_UTILS_H */
