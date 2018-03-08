/*****************************************************************************
 * IMAGIC.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>

#define IMAGIC_MAGIC 0x494d4443l

typedef struct {				/* Header fuer Imagic-Bilder */
	_UBYTE kennung[4];			/* "IMDC" */
	_UBYTE rez[2];				/* Bildaufloesung */
	TOSPALETTE palette;         /* Farbpalette */
	_UBYTE date[2];             /* Datum der Dateierstellung */
	_UBYTE time[2];             /* Uhrzeit der Dateierstellung */
	_UBYTE base[8];             /* Name des Basisbildes */
	_UBYTE datalen[2];			/* Laenge der komprimierten Daten */
	_UBYTE serial[4];			/* Seriennummer des Programms */
	_UBYTE reserved[8];         /* reserviert fuer Erweiterungen */
} IMAGIC_HEADER;


#define put_long(l) \
	*buf++ = (_UBYTE)((l) >> 24); \
	*buf++ = (_UBYTE)((l) >> 16); \
	*buf++ = (_UBYTE)((l) >>  8); \
	*buf++ = (_UBYTE)((l)	   )
#define put_word(w) \
	*buf++ = (_UBYTE)((w) >>  8); \
	*buf++ = (_UBYTE)((w)	   )
#define put_byte(b) \
	*buf++ = (_UBYTE)(b)

#define get_long() \
	(((_ULONG)(buf[0]) << 24) | \
	 ((_ULONG)(buf[1]) << 16) | \
	 ((_ULONG)(buf[2]) <<  8) | \
	 ((_ULONG)(buf[3])		)), buf += 4
#define get_word() \
	(((_UWORD)(buf[0]) <<  8) | \
	 ((_UWORD)(buf[1])		)), buf += 2
#define get_byte() \
	*buf++

/*** ---------------------------------------------------------------------- ***/

gboolean pic_type_imagic(PICTURE *pic, const _UBYTE *buf, _LONG size)
{
	_WORD rez;
	const IMAGIC_HEADER *header = (const IMAGIC_HEADER *)buf;
	_ULONG magic;
	_WORD date, time;
	
	if (size <= (_LONG)sizeof(IMAGIC_HEADER))
		return FALSE;
	magic = get_long();
	if (magic != IMAGIC_MAGIC)
		return FALSE;

	rez = get_word();
	switch (rez)
	{
		case 0: pic->pi_type = FT_IMAGIC_LOW; break;
		case 1: pic->pi_type = FT_IMAGIC_MED; break;
		case 2: pic->pi_type = FT_IMAGIC_HIGH; break;
		default: return FALSE;
	}
	pic->pi_planes = pic_calcplanes(rez);
	pic_stdsize(pic);
	pic_getpalette(pic->pi_palette, &header->palette);
	buf += sizeof(TOSPALETTE);
	date = get_word();
	time = get_word();
	UNUSED(date);
	UNUSED(time);
	/* strncpy(pic->pi_base, (_UBYTE *)buf, sizeof(pic->pi_base)); */
	buf += 8;
	pic->pi_datasize = get_word();
	if (pic->pi_datasize != (pic->pi_filesize - (_LONG)sizeof(IMAGIC_HEADER)))
		return FALSE;
	pic->pi_serial = get_long();
	pic->pi_dataoffset = sizeof(IMAGIC_HEADER);
	pic->pi_compressed = TRUE;
	UNUSED(buf);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean imagic_unpack(_UBYTE *dest, const _UBYTE *src, PICTURE *pic)
{
	_UBYTE c;
	_WORD d6 = 32000;
	_UBYTE *start;
	_UBYTE *end;
	_WORD d1, d2, d3, d4, d5;
	_UBYTE d7;
	_WORD a3, a4, a5, a6;

	UNUSED(pic);
	c = *src++;
	if (c == 0)
	{
		memcpy(dest, src, d6);
		return TRUE;
	}
	start = dest;
	end = dest + d6;
	d2 = *src++;
	d7 = *src++;
	d2 *= 80;
	d1 = c;
	if (c == 0xff)
	{
		d1 = d6;
		d2 = 1;
	}
	a4 = d2;
	d6 = d2;
	d5 = d1;
	a3 = d5 - 1;
	a5 = -d1 * d2 + 1;
	a6 = d2 * a3;
	d1 = 1;
	d2 = 3;
	d4 = 2;
	for (;;)
	{
		c = *src++;
		if (c == d7)
		{
			c = *src++;
			if (c == d7)
				goto zero_2;
			d3 = 0;
			if (c < d2)
			{
				if (c != d4)
				{
					/* chk_01 */
					while (c == d1)
					{
						d3 += 256;
						c = *src++;
					}
					c = *src++;
					/* -> add_byte */
				} else
				{
					c = *src++;
					if (c == 0)
						return TRUE;
					if (c < d2)
					{
						if (c == d4)
						{
							while ((c = *src++) != 0)
								;
							continue;
						} else
						{
							/* chk_s_01 */
							while (c == d1)
							{
								d3 += 256;
								c = *src++;
							}
							c = *src++;
						}
					}
					/* add_s_byte */
					d3 += c;
					do
					{
						dest += a4;
						if (--d5 <= 0)
						{
							d5 = a3 + 1;
							dest += a5;
							if (--d6 <= 0)
							{
								d6 = a4;
								dest += a6;
							}
						}
					} while (--d3 >= 0);
					continue;
				}
			}
			/* add_byte */
			d3 += c;
			c = *src++;
			do
			{
				if (dest < start || dest >= end)
					return TRUE;
				*dest = c;
				dest += a4;
				if (--d5 <= 0)
				{
					d5 = a3 + 1;
					dest += a5;
					if (--d6 <= 0)
					{
						d6 = a4;
						dest += a6;
					}
				}
			} while (--d3 >= 0);
		} else
		{
		zero_2:
			if (dest < start || dest >= end)
				break;
			*dest = c;
			dest += a4;
			if (--d5 > 0)
				continue;
			d5 = a3 + 1;
			dest += a5;
			if (--d6 > 0)
				continue;
			d6 = a4;
			dest += a6;
		}
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

_LONG imagic_pack(_UBYTE *dest, const _UBYTE *src, PICTURE *pic)
{
	_UBYTE c, c1;
	_UBYTE esc;
	gboolean end_squeeze;
	_WORD picsize;
	_WORD count;
	_UBYTE *start = dest;
	const _UBYTE *srcbuf = src;

	UNUSED(pic);
	{
		/* get_escape */
		_WORD countbuf[256];
		_WORD i;
		const _UBYTE *p = src;
		_WORD less;

		for (i = 256; --i >= 0; )
			countbuf[i] = 0;
		for (i = 32000; --i >= 0; )
			countbuf[*p++]++;
		less = countbuf[0];
		esc = 0;
		for (i = 3; i < 256; i++)
		{
			if (countbuf[i] == 0)
			{
				esc = i;
				break;
			}
			if (countbuf[i] < less)
			{
				less = countbuf[i];
				esc = i;
			}
		}
	}

	end_squeeze = FALSE;
	picsize = 32000 - 1;
	c = *src;
	count = -1;
	*dest++ = 0xff;
	*dest++ = 1;
	*dest++ = esc;
	for (;;)
	{
		c1 = c;
		c = *src++;
		if (--picsize < 0)
		{
			end_squeeze = TRUE;
		} else
		{
			if (c == c1)
			{
				++count;
				continue;
			}
		}
		/* write_sqz */
		if (count >= 3)
		{
			*dest++ = esc;
			if (count == esc)
			{
				*dest++ = 0;
			} else
			{
				if (count >= 256)
				{
					count -= 256;
					do {
						*dest++ = 1;
					} while ((count -= 256) >= 0);
					count += 256;
					*dest++ = 0;
				}
			}
			*dest++ = (_UBYTE) count;
			*dest++ = c1;
		} else
		{
			do
			{
				*dest++ = c1;
				if (c1 == esc)
					*dest++ = esc;
			} while (--count >= 0);
		}
		if (end_squeeze)
			break;
		count = 0;
	}
	*dest++ = c;
	if (c == esc)
		*dest++ = esc;
	*dest++ = esc;
	*dest++ = 2;
	*dest++ = 0;

	picsize = (_WORD)(dest - start);
	if (picsize >= 32000)
	{
		dest = start;
		*dest++ = 0;
		picsize = 32000;
		src = srcbuf;
		while (--picsize >= 0)
			*dest++ = *src++;
		picsize = (_WORD)(dest - start);
	}
	return picsize;
}

/*** ---------------------------------------------------------------------- ***/

_LONG imagic_header(_UBYTE *buf, PICTURE *pic)
{
	IMAGIC_HEADER *header = (IMAGIC_HEADER *)buf;
	_WORD date = 0, time = 0;
	
	put_long(IMAGIC_MAGIC);
	put_word(pic_calcrez(pic->pi_planes));
	pic_setpalette(header->palette, pic->pi_palette);
	buf += sizeof(TOSPALETTE);
	put_word(date);
	put_word(time);
	memset(buf, 0, 8); /* basename */
	buf += 8;
	put_word(pic->pi_datasize);
	put_long(pic->pi_serial);
	put_long(0l);
	put_long(0l);
	UNUSED(buf);
	return IMAGIC_HEADER_SIZE;
}
