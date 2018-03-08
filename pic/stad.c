/*****************************************************************************
 * STAD.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>

#define STAD_MAGIC1 0x704d3835l
#define STAD_MAGIC2 0x704d3836l

typedef struct {				/* Header fuer STAD-Bilder */
	_UBYTE kennung[4];			/* "pM85" oder "pM86" */
} STAD_HEADER;

#define put_long(l) \
	*buf++ = (_UBYTE)((l) >> 24); \
	*buf++ = (_UBYTE)((l) >> 16); \
	*buf++ = (_UBYTE)((l) >>  8); \
	*buf++ = (_UBYTE)((l)	   )
#define get_long() \
	(((_ULONG)(buf[0]) << 24) | \
	 ((_ULONG)(buf[1]) << 16) | \
	 ((_ULONG)(buf[2]) <<  8) | \
	 ((_ULONG)(buf[3])		)), buf += 4


/*** ---------------------------------------------------------------------- ***/

gboolean pic_type_stad(PICTURE *pic, const _UBYTE *buf, _LONG size)
{
	_ULONG magic;

	if (size <= (_LONG)sizeof(STAD_HEADER))
		return FALSE;
	magic = get_long();
	if (magic == STAD_MAGIC1)
	{
		pic->pi_compressed = 0;
	} else if (magic == STAD_MAGIC2)
	{
		pic->pi_compressed = 1;
	} else
	{
		return FALSE;
	}
	pic->pi_datasize = pic->pi_filesize - sizeof(STAD_HEADER);
	pic->pi_dataoffset = sizeof(STAD_HEADER);
	pic->pi_planes = 1;
	pic_stdsize(pic);
	pic->pi_type = FT_STAD;
	UNUSED(buf);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean stad_unpack(_UBYTE *dst, const _UBYTE *src, PICTURE *pic)
{
	_UBYTE *start;
	_WORD picsize = 32000;
	_UBYTE kennbyte;
	_UBYTE packbyte;
	_UBYTE specialbyte;
	_WORD lines;
	_WORD count;
	_UBYTE c, c1;

	start = dst;

	kennbyte = *src++;
	packbyte = *src++;
	specialbyte = *src++;
	if (pic->pi_compressed) /* vertical */
	{
		lines = 400;
		for (;;)
		{
			c = *src++;
			if (c == kennbyte)
			{
				c1 = packbyte;
				count = *src++;
			} else if (c == specialbyte)
			{
				c1 = *src++;
				count = *src++;
				if (c1 == 0 && count == 0)
					return TRUE;
			} else
			{
				c1 = c;
				count = 0;
			}
			do
			{
				if (--picsize <= 0)
				{
					if (picsize == 0 && count == 1)
						break;
					return FALSE;
				}
				*start = c1;
				start += 80;
				if (--lines == 0)
				{
					start = dst;
					start++;
					dst = start;
					lines = 400;
				}
			} while (--count >= 0);
		}
	} else
	{
		for (;;)
		{
			c = *src++;
			if (c == kennbyte)
			{
				c1 = packbyte;
				count = *src++;
			} else if (c == specialbyte)
			{
				c1 = *src++;
				count = *src++;
				if (c1 == 0 && count == 0)
					break;
			} else
			{
				c1 = c;
				count = 0;
			}
			do
			{
				if (--picsize <= 0)
				{
					if (picsize == 0 && count == 1)
						break;
					return FALSE;
				}
				*start++ = c1;
			} while (--count >= 0);
		}
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

_UBYTE *stad_pack(const _UBYTE *_src, _UBYTE *horbuf, _UBYTE *verbuf, PICTURE *pic)
{
	_WORD i, j;
	const _UBYTE *src;
	_UBYTE *buf;
	const _UBYTE *start;
	_UBYTE c;
	_WORD min;
	_UBYTE kennbyte;
	_UBYTE packbyte;
	_UBYTE specialbyte;
	_WORD picsize;
	_WORD lines;
	_LONG horcount, vercount;
	gboolean endflag;
	_WORD countbuf[256];

	for (i = 0; i < 256; i++)
		countbuf[i] = 0;
	src = _src;
	for (i = 32000; --i >= 0; )
	{
		c = *src++;
		countbuf[c]++;
	}

	j = 1;
	for (i = 0; i < 256; i++)
		if ((min = countbuf[i]) != 0)
			break;
	for (i++; i < 256; i++)
	{
		if (countbuf[i] < min && countbuf[i] != 0)
		{
			min = countbuf[i];
			j = i;
		}
	}
	kennbyte = (_UBYTE) j;

	min = countbuf[0];
	j = 0;
	for (i++; i < 256; i++)
	{
		if (countbuf[i] > min)
		{
			min = countbuf[i];
			j = i;
		}
	}
	packbyte = (_UBYTE) j;

	min = countbuf[0];
	j = 2;
	for (i++; i < 256; i++)
	{
		if (countbuf[i] < min && kennbyte < i)
			j = i;
	}
	if (j == kennbyte)
	{
		++j;
		if (j == 256)
			j = 1;
	}
	specialbyte = (_UBYTE) j;


	/* horizontal packen */
	src = _src;
	buf = horbuf;
	put_long(STAD_MAGIC1);
	*buf++ = kennbyte;
	*buf++ = packbyte;
	*buf++ = specialbyte;
	horcount = 7;
	picsize = 32000;
	endflag = FALSE;
	c = *src++;
	do
	{
		i = 0;
		do
		{
			if (--picsize <= 0)
			{
				endflag = TRUE;
				break;
			}
			if (*src++ != c)
				break;
			i++;
		} while (i < 256);
		if (i >= 2)
		{
			if (i == 256)
				--i;
			if (c == packbyte)
			{
				*buf++ = kennbyte;
				*buf++ = i;
				horcount += 2;
			} else
			{
				*buf++ = specialbyte;
				*buf++ = c;
				*buf++ = i;
				horcount += 3;
			}
		} else if (i != 0)
		{
			if (c == specialbyte || c == kennbyte || c == 0)
			{
				*buf++ = specialbyte;
				*buf++ = c;
				*buf++ = i;
				horcount += 3;
			} else
			{
				*buf++ = c;
				*buf++ = c;
				horcount += 2;
			}
		} else
		{
			if (c == specialbyte || c == kennbyte)
			{
				*buf++ = specialbyte;
				*buf++ = c;
				*buf++ = i;
				horcount += 3;
			} else
			{
				*buf++ = c;
				horcount += 1;
			}
		}
		if (horcount > 32000)
			break;
		c = src[-1];
	} while (endflag == FALSE);
	*buf++ = specialbyte;
	*buf++ = 0;
	*buf++ = 0;
	UNUSED(buf);
	horcount += 3;


	/* vertikal packen */
	src = _src;
	start = src - 80;
	buf = verbuf;
	put_long(STAD_MAGIC2);
	*buf++ = kennbyte;
	*buf++ = packbyte;
	*buf++ = specialbyte;
	vercount = 7;
	picsize = 32000;
	lines = 400;
	endflag = FALSE;
	c = *src;
	do
	{
		i = 0;
		do
		{
			if (--picsize <= 0)
			{
				endflag = TRUE;
				break;
			}
			if (--lines == 0)
			{
				src = start;
				src++;
				start = src;
				lines = 400;
			}
			src += 80;
			if (*src != c)
				break;
			i++;
		} while (i < 256);
		if (i >= 2)
		{
			if (i == 256)
				--i;
			if (c == packbyte)
			{
				*buf++ = kennbyte;
				*buf++ = i;
				vercount += 2;
			} else
			{
				*buf++ = specialbyte;
				*buf++ = c;
				*buf++ = i;
				vercount += 3;
			}
		} else if (i != 0)
		{
			if (c == specialbyte || c == kennbyte || c == 0)
			{
				*buf++ = specialbyte;
				*buf++ = c;
				*buf++ = i;
				vercount += 3;
			} else
			{
				*buf++ = c;
				*buf++ = c;
				vercount += 2;
			}
		} else
		{
			if (c == specialbyte || c == kennbyte)
			{
				*buf++ = specialbyte;
				*buf++ = c;
				*buf++ = i;
				vercount += 3;
			} else
			{
				*buf++ = c;
				vercount += 1;
			}
		}
		if (vercount > 32000)
			break;
		c = *src;
	} while (endflag == FALSE);
	*buf++ = specialbyte;
	*buf++ = 0;
	*buf++ = 0;
	vercount += 3;
	UNUSED(buf);

	if (horcount < vercount)
	{
		pic->pi_datasize = horcount;
		return horbuf;
	} else
	{
		pic->pi_datasize = vercount;
		return verbuf;
	}
}
