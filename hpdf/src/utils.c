/*
 * << Haru Free PDF Library >> -- hpdf_utils.c
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

#include "hpdf/utils.h"
#include "hpdf/consts.h"
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

HPDF_INT HPDF_AToI(const char *s)
{
	HPDF_BOOL flg = HPDF_FALSE;
	HPDF_INT v = 0;

	if (!s)
	{
		return 0;
	}

	/* increment pointer until the charactor of 's' is not
	 * white-space-charactor.
	 */
	while (*s)
	{
		if (HPDF_IS_WHITE_SPACE(*s))
		{
			s++;
		} else
		{
			if (*s == '-')
			{
				flg = HPDF_TRUE;
				s++;
			}
			break;
		}
	}

	while (*s >= '0' && *s <= '9')
	{
		v *= 10;
		v += *s - '0';
		s++;
	}

	if (flg)
		v *= -1;

	return v;
}


HPDF_DOUBLE HPDF_AToF(const char *s)
{
	HPDF_BOOL flg = HPDF_FALSE;
	HPDF_INT i = 0;
	HPDF_DOUBLE v;
	HPDF_INT tmp = 1;

	/* increment pointer until the charactor of 's' is not
	 * white-space-charactor.
	 */
	while (*s)
	{
		if (HPDF_IS_WHITE_SPACE(*s))
		{
			s++;
		} else
		{
			if (*s == '-')
			{
				flg = HPDF_TRUE;
				s++;
			}
			break;
		}
	}

	while (*s >= '0' && *s <= '9')
	{
		if (i > 3276)
			break;

		i *= 10;
		i += *s - '0';
		s++;
	}

	if (*s == '.')
	{
		s++;
		while (*s >= '0' && *s <= '9')
		{
			if (i > 214748364L)
				break;

			i *= 10;
			i += *s - '0';
			s++;
			tmp *= 10;
		}
	}

	v = (HPDF_DOUBLE) i / tmp;

	if (flg)
		v *= -1;

	return v;
}


char *HPDF_IToA(char *s, long val, char *eptr)
{
	char *t;
	char buf[HPDF_INT_LEN + 1];

	if (val < 0)
	{
		if (val < HPDF_LIMIT_MIN_INT)
			val = HPDF_LIMIT_MIN_INT;
		*s++ = '-';
		val = -val;
	} else if (val > HPDF_LIMIT_MAX_INT)
	{
		val = HPDF_LIMIT_MAX_INT;
	} else if (val == 0)
	{
		*s++ = '0';
	}

	t = buf + HPDF_INT_LEN;
	*t-- = 0;

	while (val > 0)
	{
		*t = (char) ((char) (val % 10) + '0');
		val /= 10;
		t--;
	}

	t++;
	while (s < eptr && *t != 0)
		*s++ = *t++;
	*s = 0;

	return s;
}


char *HPDF_IToA2(char *s, HPDF_UINT32 val, HPDF_UINT len)
{
	char *t;
	char *u;

	if (val > HPDF_LIMIT_MAX_INT)
		val = HPDF_LIMIT_MAX_INT;

	u = s + len - 1;
	*u = 0;
	t = u - 1;
	while (val > 0 && t >= s)
	{
		*t = (char) ((char) (val % 10) + '0');
		val /= 10;
		t--;
	}

	while (s <= t)
		*t-- = '0';

	return s + len - 1;
}


char *HPDF_FToA(char *s, HPDF_REAL val, char *eptr)
{
	HPDF_INT32 int_val;
	HPDF_INT32 fpart_val;
	char buf[HPDF_REAL_LEN + 1];
	char *sptr = s;
	char *t;
	HPDF_UINT32 i;

	if (val > HPDF_LIMIT_MAX_REAL)
		val = HPDF_LIMIT_MAX_REAL;
	else if (val < HPDF_LIMIT_MIN_REAL)
		val = HPDF_LIMIT_MIN_REAL;

	t = buf + HPDF_REAL_LEN;
	*t-- = 0;

	if (val < 0)
	{
		*s++ = '-';
		val = -val;
	}

	/* separate an integer part and a decimal part. */
	int_val = val + 0.000005f;
	fpart_val = (val - int_val + 0.000005f) * 100000.0f;

	/* process decimal part */
	for (i = 0; i < 5; i++)
	{
		*t = (fpart_val % 10) + '0';
		fpart_val /= 10;
		t--;
	}

	/* process integer part */
	*t-- = '.';
	*t = '0';
	if (int_val == 0)
		t--;

	while (int_val > 0)
	{
		*t = (int_val % 10) + '0';
		int_val /= 10;
		t--;
	}

	t++;
	while (s <= eptr && *t != 0)
		*s++ = *t++;
	s--;

	/* delete an excessive decimal portion. */
	while (s > sptr)
	{
		if (*s == '0')
			*s = 0;
		else
		{
			if (*s == '.')
				*s = 0;
			break;
		}
		s--;
	}

	return *s == 0 ? s : ++s;
}


char *HPDF_StrCpy(char *out, const char *in, char *eptr)
{
	if (in != NULL)
	{
		while (eptr > out && *in != 0)
			*out++ = *in++;
	}

	*out = 0;

	return out;
}


HPDF_UINT HPDF_StrLen(const char *s, HPDF_INT maxlen)
{
	HPDF_INT len = 0;

	if (!s)
		return 0;

	while (*s != 0 && (maxlen < 0 || len < maxlen))
	{
		s++;
		len++;
	}

	return len;
}


const char *HPDF_StrStr(const char *s1, const char *s2, HPDF_UINT maxlen)
{
	HPDF_UINT len = HPDF_StrLen(s2, -1);

	if (!s1)
		return NULL;

	if (len == 0)
		return s1;

	if (maxlen == 0)
		maxlen = HPDF_StrLen(s1, -1);

	if (maxlen < len)
		return NULL;

	maxlen -= len;
	maxlen++;

	while (maxlen > 0)
	{
		if (memcmp(s1, s2, len) == 0)
			return s1;

		s1++;
		maxlen--;
	}

	return NULL;
}


void HPDF_ToBox(HPDF_Box *box, HPDF_INT16 left, HPDF_INT16 bottom, HPDF_INT16 right, HPDF_INT16 top)
{
	box->left = left;
	box->bottom = bottom;
	box->right = right;
	box->top = top;
}


void HPDF_UInt16Swap(HPDF_UINT16 *value)
{
	HPDF_BYTE u[2];

	memcpy(u, (HPDF_BYTE *) value, 2);
	*value = ((HPDF_UINT16) u[0] << 8) | (HPDF_UINT16) u[1];
}
