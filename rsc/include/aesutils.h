#ifndef __AESUTILS_H__
#define __AESUTILS_H__

#include <stdarg.h>

#undef min
#undef max
#define max(x,y)	(((x) > (y)) ? (x) : (y))
#define min(x,y)	(((x) < (y)) ? (x) : (y))


EXTERN_C_BEG


/* 	Routine to set the variables x,y,w,h to the values found
 *	in an x,y,w,h block (grect)
 */
static INLINE void r_get(const GRECT *gr, _WORD *px, _WORD *py, _WORD *pw, _WORD *ph)
{
	*px = gr->g_x;
	*py = gr->g_y;
	*pw = gr->g_w;
	*ph = gr->g_h;
}


/* 	Routine to set a x,y,w,h block to the x,y,w,h  
 *	values passed in.
 */
static INLINE void r_set(GRECT *gr, _WORD x, _WORD y, _WORD w, _WORD h)
{
	gr->g_x = x;
	gr->g_y = y;
	gr->g_w = w;
	gr->g_h = h;
}

/*
 * aesutils.c
 */
const char *scasb(const char *p, char b);
_BOOL inside(_WORD x, _WORD y, const GRECT *pt);
void rc_constrain(const GRECT *pc, GRECT *pt);
char *strscn(const char *src, char *dst, char stp);
_WORD strchk(const char *s, const char *t);
void fmt_str(const char *instr, char *outstr);
void unfmt_str(const char *instr, char *outstr);
void fs_sset(OBJECT *tree, _WORD obj, const char *pstr, char **ptext, _WORD *ptxtlen);
void inf_sset(OBJECT *tree, _WORD obj, char *pstr);
void fs_sget(OBJECT *tree, _WORD obj, char *pstr);
void inf_fldset(OBJECT *tree, _WORD obj, _UWORD testfld, _UWORD testbit, _UWORD truestate, _UWORD falsestate);
void merge_str(char *pdst, const char *ptmp, va_list parms);
_WORD wildcmp(const char *pwild, const char *ptest);
size_t strmaxcpy(char *dst, size_t len, const char *src);
unsigned int reverse(unsigned int index);
_BOOL streq(const char *p1, const char *p2);
_WORD inf_gindex(OBJECT *tree, _UWORD baseobj, _UWORD numobj);
_WORD inf_what(OBJECT *tree, _WORD ok);

int aes_toupper(int ch);

/*
 * mul_div - signed integer multiply and divide
 *
 * mul_div (m1,m2,d1)
 *
 * ( ( m1 * m2 ) / d1 ) + 1/2
 *
 * m1 = signed 16 bit integer
 * m2 = unsigned 15 bit integer
 * d1 = signed 16 bit integer
 */

/*
 * mul_div - signed integer multiply and divide
 * return ( m1 * m2 ) / d1
 * While the operands are WORD, the intermediate result is LONG.
 */
static INLINE _WORD mul_div(_WORD m1, _WORD m2, _WORD d1)
{
#if defined(__mc68000__) && defined(__GNUC__)
    __asm__ (
      "muls %1,%0\n\t"
      "divs %2,%0"
    : "+d"(m1)
    : "idm"(m2), "idm"(d1)
    );
    return m1;
#else
	return ((_LONG) m1 * m2) / d1;
#endif
}

EXTERN_C_END

#endif /* __AESUTILS_H__ */
