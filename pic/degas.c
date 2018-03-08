/*****************************************************************************
 * DEGAS.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>


#define tobyte(pixels) (((pixels) + 7) >> 3)

typedef struct {                /* Header fuer Degas-Bilder */
    _UBYTE rez[2];              /* Bildaufloesung, Bit 15 = compressed-flag */
	TOSPALETTE palette;         /* Farbpalette */
} DEGAS_HEADER;


/*** ---------------------------------------------------------------------- ***/

gboolean pic_type_degas(PICTURE *pic, const _UBYTE *buf, _LONG size)
{
	_WORD pictype;
	_WORD compressed;
	const DEGAS_HEADER *header = (const DEGAS_HEADER *)buf;

	pictype = header->rez[1];
	compressed = header->rez[0] & 0x80;
	if (pic->pi_filesize <= 32066l &&
		size >= 34 &&
		/* is_palette(header->palette) && */
		(pic->pi_filesize >= 32034l || compressed))
	{
		switch (pictype)
		{
			case 0: pic->pi_type = FT_DEGAS_LOW; break;
			case 1: pic->pi_type = FT_DEGAS_MED; break;
			case 2: pic->pi_type = FT_DEGAS_HIGH; break;
			default: return FALSE;
		}
		pic->pi_planes = pic_calcplanes(pictype);
		pic_stdsize(pic);
		pic_getpalette(pic->pi_palette, &header->palette);
		pic->pi_datasize = pic->pi_filesize - sizeof(DEGAS_HEADER);
		pic->pi_dataoffset = sizeof(DEGAS_HEADER);
		pic->pi_compressed = compressed ? TRUE : FALSE;
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean degas_unpack(_UBYTE *dest, const _UBYTE *src, PICTURE *pic)
{
	_UBYTE *planeptr[4];
	_WORD bytes;
	_WORD p;
	_UBYTE **pp, *cp, *lp;
	_WORD words;
	_LONG linesize;
	_UBYTE *linebuf;
	_WORD width = pic->pi_width;
	_WORD height = pic->pi_height;
	_WORD planes = pic->pi_planes;
	_WORD compressed = pic->pi_compressed;

	planeptr[0] = dest;
	planeptr[1] = dest + 2;
	planeptr[2] = dest + 4;
	planeptr[3] = dest + 6;
	bytes = tobyte(width);
	words = (bytes >> 1);
	linesize = (_LONG)words * 2 * planes;
	linebuf = g_new(_UBYTE, linesize);
	if (linebuf == NULL)
		return FALSE;
	while (--height >= 0)
	{
		lp = linebuf;
		for (p = planes; --p >= 0; )
		{
			/* uncompress 1 row */
			_UBYTE c, c1;

			words = bytes;
			if (compressed)
			{
				while (words > 0)
				{
					c = *src++;
					if (c < 0x80)
					{
						words -= c;
						words--;
						while (c)
						{
							*lp++ = *src++;
							c--;
						}
						*lp++ = *src++;
					} else
					{
						c = 257 - c;
						words -= c;
						c1 = *src++;
						while (c)
						{
							*lp++ = c1;
							c--;
						}
					}
				}
			} else
			{
				while (--words >= 0)
					*lp++ = *src++;
				/* if (bytes & 1)
					src++; */
			}
			if (bytes & 1)
			{
				*lp++ = 0;
			}
		}

		lp = linebuf;
		for (p = planes, pp = planeptr; --p >= 0; )
		{
			cp = *pp;
			for (words = bytes; (words -= 2) >= 0; )
			{
				cp[0] = *lp++;
				cp[1] = *lp++;
				cp += planes;
				cp += planes;
			}
			*pp++ = cp;
		}
	}
	g_free(linebuf);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

_UBYTE *degas_pack(const _UBYTE *src, PICTURE *pic)
{
	const _UBYTE *planeptr[4];
	_WORD bytes;
	_WORD p;
	const _UBYTE **pp;
	const _UBYTE *cp;
	_UBYTE *lp;
	_WORD words;
	_UBYTE *linebuf;
	_UBYTE *dest;
	_WORD width = pic->pi_width;
	_WORD height = pic->pi_height;
	_WORD planes = pic->pi_planes;
	_WORD compressed = pic->pi_compressed;
	_LONG linesize;
	_UBYTE *_dest;
	
	bytes = tobyte(width);
	words = (bytes >> 1);
	linesize = (_LONG)words * 2 * planes;
	linebuf = g_new(_UBYTE, linesize);
	if (linebuf == NULL)
		return NULL;
	_dest = g_new(_UBYTE, (linesize + (linesize + 255) / 256) * height);
	if (_dest == NULL)
	{
		g_free(linebuf);
		return NULL;
	}
	
	dest = _dest;
	
	planeptr[0] = src;
	planeptr[1] = src + 2;
	planeptr[2] = src + 4;
	planeptr[3] = src + 6;
	bytes = tobyte(width);
	words = (bytes >> 1);
	linesize = (_LONG)words * 2 * planes;
	while (--height >= 0)
	{
		lp = linebuf;
		for (p = planes, pp = planeptr; --p >= 0; )
		{
			cp = *pp;
			for (words = bytes; (words -= 2) >= 0; )
			{
				*lp++ = cp[0];
				*lp++ = cp[1];
				cp += planes;
				cp += planes;
			}
			if (bytes & 1)
			{
				*lp++ = cp[0];
				*lp++ = 0;
				cp += planes;
				cp += planes;
			}
			*pp++ = cp;
		}
		
		lp = linebuf;
		for (p = planes; --p >= 0; )
		{
			/* pack 1 row */
			if (compressed)
			{
				_UBYTE *start;
				_WORD count;
				_UBYTE c;
				
				words = bytes;
				while (words > 0)
				{
					count = 0;
					start = lp;
					for (;;)
					{
						++count;
						c = *lp++;
						if (--words == 0)
						{
							lp = start;
							*dest++ = (_UBYTE)(count - 1);
							while (--count >= 0)
								*dest++ = *lp++;
							break;
						}
						if (*lp != c)
							continue;
						if (lp[1] != c)
							continue;
						lp = start;
						if (count > 1)
						{
							--count;
							*dest++ = (_UBYTE)(count - 1);
							while (--count >= 0)
								*dest++ = *lp++;
						}
						words++;
						lp++;
						count = -1;
						for (;;)
						{
							++count;
							if (--words == 0)
								break;
							if (*lp++ != c)
							{
								--lp;
								break;
							}
						}
						*dest++ = (_UBYTE)(256 - count);
						*dest++ = c;
						break;
					} 
				}
			} else
			{
				for (words = bytes; --words >= 0; )
					*dest++ = *lp++;
				if (bytes & 1)
					*dest++ = 0;
			}
		}
	}
	g_free(linebuf);
	pic->pi_datasize = (_LONG)(dest - _dest);
	return _dest;
}

/*** ---------------------------------------------------------------------- ***/

_LONG degas_header(_UBYTE *buf, PICTURE *pic)
{
	DEGAS_HEADER *header = (DEGAS_HEADER *)buf;

	header->rez[1] = pic_calcrez(pic->pi_planes);
	header->rez[0] = pic->pi_compressed ? 0x80 : 0x00;
	pic_setpalette(header->palette, pic->pi_palette);
	pic->pi_datasize = 32000;
	return DEGAS_HEADER_SIZE;
}
