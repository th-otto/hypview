#ifndef CC_FOR_BUILD
#include "hypdefs.h"
#endif

#ifdef __WIN32__

#define CONTINUATION_CHAR                           \
  if ((*(const unsigned char *)p & 0xc0) != 0x80) /* 10xxxxxx */ \
    goto error;                                     \
  val <<= 6;                                        \
  val |= (*(const unsigned char *)p) & 0x3f

#define UNICODE_VALID(Char)                   \
    ((Char) < 0x110000UL &&                     \
     (((Char) & 0xFFFFF800UL) != 0xD800UL) &&     \
     ((Char) < 0xFDD0UL || (Char) > 0xFDEFUL) &&  \
     ((Char) & 0xFFFEUL) != 0xFFFEUL)

const char *hyp_utf8_getchar(const char *p, h_unichar_t *ch)
{
	const char *last;

	if (*(const unsigned char *) p < 0x80)
	{
		*ch = *(const unsigned char *) p;
		return p + 1;
	}
	last = p;
	if ((*(const unsigned char *) p & 0xe0) == 0xc0)	/* 110xxxxx */
	{
		if ((*(const unsigned char *) p & 0x1e) == 0)
			goto error;
		*ch = (*(const unsigned char *) p & 0x1f) << 6;
		p++;
		if ((*(const unsigned char *) p & 0xc0) != 0x80)	/* 10xxxxxx */
			goto error;
		*ch |= (*(const unsigned char *) p) & 0x3f;
	} else
	{
		h_unichar_t val = 0;
		h_unichar_t min = 0;
		
		if ((*(const unsigned char *) p & 0xf0) == 0xe0)	/* 1110xxxx */
		{
			min = (1 << 11);
			val = *(const unsigned char *) p & 0x0f;
			goto TWO_REMAINING;
		} else if ((*(const unsigned char *) p & 0xf8) == 0xf0)	/* 11110xxx */
		{
			min = ((h_unichar_t)1 << 16);
			val = *(const unsigned char *) p & 0x07;
		} else
		{
			goto error;
		}
		
		p++;
		CONTINUATION_CHAR;
	  TWO_REMAINING:
		p++;
		CONTINUATION_CHAR;
		p++;
		CONTINUATION_CHAR;

		if (val < min)
			goto error;

		if (!UNICODE_VALID(val))
			goto error;
		*ch = val;
	}

	return p + 1;

  error:
    *ch = 0xffff;
	return last + 1;
}

/*** ---------------------------------------------------------------------- ***/

const char *g_utf8_skipchar(const char *p)
{
	const char *last;

	if (*(const unsigned char *) p < 0x80)
		return p + 1;
	last = p;
	if ((*(const unsigned char *) p & 0xe0) == 0xc0)	/* 110xxxxx */
	{
		if ((*(const unsigned char *) p & 0x1e) == 0)
			goto error;
		p++;
		if ((*(const unsigned char *) p & 0xc0) != 0x80)	/* 10xxxxxx */
			goto error;
	} else
	{
		h_unichar_t val = 0;
		h_unichar_t min = 0;
		
		if ((*(const unsigned char *) p & 0xf0) == 0xe0)	/* 1110xxxx */
		{
			min = (1 << 11);
			val = *(const unsigned char *) p & 0x0f;
			goto TWO_REMAINING;
		} else if ((*(const unsigned char *) p & 0xf8) == 0xf0)	/* 11110xxx */
		{
			min = ((h_unichar_t)1 << 16);
			val = *(const unsigned char *) p & 0x07;
		} else
		{
			goto error;
		}
		
		p++;
		CONTINUATION_CHAR;
	  TWO_REMAINING:
		p++;
		CONTINUATION_CHAR;
		p++;
		CONTINUATION_CHAR;

		if (val < min)
			goto error;

		if (!UNICODE_VALID(val))
			goto error;
	}

	return p + 1;

  error:
	return last + 1;
}

/*** ---------------------------------------------------------------------- ***/

size_t g_utf8_str_len(const char *p, size_t len)
{
	size_t l = 0;
	const char *end;
	
	if (p == NULL)
		return l;
	if (len == STR0TERM)
		len = strlen(p);
	end = p + len;
	while (p < end && *p)
	{
		p = g_utf8_skipchar(p);
		l++;
	}
	return l;
}

/*** ---------------------------------------------------------------------- ***/

char *hyp_wchar_to_utf8(const wchar_t *str, size_t wlen)
{
	size_t len;
	char *dst, *p;
	h_unichar_t wc;
	
	if (str == NULL)
		return NULL;
	if (wlen == STR0TERM)
		wlen = wcslen(str);
	len = wlen * HYP_UTF8_CHARMAX + 1;
	dst = p = g_new(char, len);
	if (dst == NULL)
		return NULL;
	while (wlen)
	{
		wc = *str++;
		hyp_put_unichar(p, wc);
		wlen--;
	}
	*p++ = '\0';
	return g_renew(char, dst, p - dst);
}

/*** ---------------------------------------------------------------------- ***/

wchar_t *hyp_utf8_to_wchar(const char *str, size_t len, size_t *lenp)
{
	size_t wlen;
	wchar_t *dst, *p;
	h_unichar_t ch;
	const char *end;
	
	if (str == NULL)
		return NULL;
	if (len == STR0TERM)
		len = strlen(str);
	wlen = g_utf8_str_len(str, len);
	dst = g_new(wchar_t, wlen + 1);
	if (dst == NULL)
		return NULL;
	end = str + len;
	p = dst;
	while (str < end)
	{
		str = hyp_utf8_getchar(str, &ch);
		*p++ = (wchar_t) ch;
	}
	*p = 0;
	if (lenp)
		*lenp = wlen;
	return dst;
}

/*** ---------------------------------------------------------------------- ***/

FILE *hyp_utf8_fopen(const char *filename, const char *mode)
{
	wchar_t *wstr;
	FILE *fp;
	size_t len;
	wchar_t *wmode;
	
	wmode = hyp_utf8_to_wchar(mode, STR0TERM, &len);
	wstr = hyp_utf8_to_wchar(filename, STR0TERM, &len);
	if (wmode == NULL || wstr == NULL)
	{
		g_free(wstr);
		g_free(wmode);
		return NULL;
	}
	fp = _wfopen(wstr, wmode);
	g_free(wstr);
	g_free(wmode);
	return fp;
}

#else

FILE *hyp_utf8_fopen(const char *filename, const char *mode)
{
	return fopen(filename, mode);
}

#endif /* __WIN32__ */
