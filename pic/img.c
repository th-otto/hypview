/*****************************************************************************
 * IMG.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>

typedef struct {                /* Header fuer GEM-Bilder */
    _UBYTE version[2];          /* Versionsnummer */
    _UBYTE head_len[2];         /* Laenge des Headers in words */
    _UBYTE plane_num[2];        /* Anzahl der planes */
    _UBYTE pat_len[2];          /* Laenge der verwendeten Muster in bytes */
    _UBYTE pix_wid[2];          /* Pixelbreite des verwendeten Geraetes in um */
    _UBYTE pix_hght[2];         /* Pixelhoehe des verwendeten Geraetes in um */
    _UBYTE pix_num[2];          /* Pixel-breite einer Zeile */
    _UBYTE scan_num[2];         /* Anzahl der Zeilen */
} GEM_HEADER;

#define XIMG_MAGIC 0x58494d47l

typedef struct ximg_header {
	_UBYTE magic[4];
	_WORD model;
	_WORD palette[256][3];
} XIMG_HEADER;
#define SIZEOF_XIMG_HEADER(planes) (4 + 2 + (1 << (planes)) * 3 * 2)


#define tobyte(pixels) (((pixels) + 7) >> 3)
#define toword(pixels) ((((pixels) + 15) >> 4) << 1)

#define put_long(l) \
	*outptr++ = (_UBYTE)((l) >> 24); \
	*outptr++ = (_UBYTE)((l) >> 16); \
	*outptr++ = (_UBYTE)((l) >>  8); \
	*outptr++ = (_UBYTE)((l)      )
#define put_word(w) \
	*outptr++ = (_UBYTE)((w) >>  8); \
	*outptr++ = (_UBYTE)((w)      )
#define put_byte(b) \
	*outptr++ = (_UBYTE)(b)
#define get_long() \
	(((_ULONG)(buf[0]) << 24) | \
	 ((_ULONG)(buf[1]) << 16) | \
	 ((_ULONG)(buf[2]) <<  8) | \
	 ((_ULONG)(buf[3])      )), buf += 4
#define get_word() \
	(((_UWORD)(buf[0]) <<  8) | \
	 ((_UWORD)(buf[1])      )), buf += 2


/*** ---------------------------------------------------------------------- ***/

gboolean pic_type_img(PICTURE *pic, const _UBYTE *buf, _LONG size)
{
	_WORD w;
	_LONG magic;
	
	if (size <= GEM_HEADER_SIZE)
		return FALSE;
	w = get_word();
	if (w != 1)
		return FALSE;
	pic->pi_dataoffset = get_word();
	pic->pi_dataoffset <<= 1;
	if (pic->pi_dataoffset < GEM_HEADER_SIZE)
		return FALSE;
	pic->pi_planes = get_word();
	pic->pi_pat_len = get_word();
	if (pic->pi_pat_len != 2 && pic->pi_pat_len != 1)
		return FALSE;
	pic->pi_pix_width = get_word();
	pic->pi_pix_height = get_word();
	pic->pi_width = get_word();
	pic->pi_height = get_word();
	pic->pi_compressed = 1;
	pic_stdpalette(pic->pi_palette, pic->pi_planes);
	pic->pi_type = FT_IMG;
	if (pic->pi_dataoffset >= (GEM_HEADER_SIZE + 6) && pic->pi_planes <= 8)
	{
		magic = get_long();
		if (magic == XIMG_MAGIC)
		{
			_WORD model;
			const _UBYTE *end;
			_WORD i, ncolors;
			_WORD vdipal[256 * 3];
			
			model = get_word();
			if (model == 0)
			{
				end = buf + pic->pi_dataoffset - (GEM_HEADER_SIZE + 6) - 6;
				ncolors = 1 << pic->pi_planes;
				i = 0;
				while (i < ncolors && buf <= end)
				{
					vdipal[i * 3 + 0] = get_word();
					vdipal[i * 3 + 1] = get_word();
					vdipal[i * 3 + 2] = get_word();
					i++;
				}
				pic_vdi_to_rgbpal(pic->pi_palette, vdipal, i, 3);
			}
		}
	}
	if (pic->pi_planes != 1 && pic->pi_planes != 2 && pic->pi_planes != 4 && pic->pi_planes != 8 && pic->pi_planes != 24 && pic->pi_planes != 32)
	{
		if (pic->pi_planes >= 1 && pic->pi_planes <= 8)
		{
			pic->pi_unsupported = TRUE;
			pic_calcsize(pic);
			return TRUE;
		}
		return FALSE;
	}
	pic->pi_datasize = pic->pi_filesize - pic->pi_dataoffset;
	pic_calcsize(pic);
	UNUSED(buf);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

LOCAL _UBYTE const masks[8] = { 0xff, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe };

gboolean img_unpack(_UBYTE *dst, const _UBYTE *src, PICTURE *pic)
{
	_WORD width = pic->pi_width;
	_WORD height = pic->pi_height;
	_WORD planes = pic->pi_planes;
	_WORD patlen = pic->pi_pat_len;
	_UWORD bytes = tobyte(width);
	_WORD planecount;
	_UWORD bytecount;
	_ULONG planeoff;
	_ULONG lineoff;
	_ULONG nextplane;
	_ULONG offset;
	_WORD line_count;
	const _UBYTE *line_start;
	_UBYTE c, c2;
	_UBYTE lastmask;

	planeoff = planes;
	planeoff += planeoff;
	planeoff -= 2;

	bytecount = (bytes + 1) & ~1; /* round up to words */
	lastmask = masks[width & 7];

	lineoff = bytecount;
	lineoff = lineoff * planes - 2;

	offset = bytecount;
	offset *= planes;
	offset -= planeoff;
	offset -= 2;
	nextplane = offset;

	bytecount = 0;
	offset = 0;
	line_count = 0;
	planecount = planes;
	line_start = NULL;

	for (;;)
	{
		if (bytecount >= bytes)
		{
			bytecount = 0;
			if (offset & 1)
			{
				dst[offset] = 0;
				offset++;
				offset += planeoff;
			}
			offset -= lineoff;
			if (--planes == 0)
			{
				planes = planecount;
				offset += nextplane;
				if (--line_count > 0)
					src = line_start;
				else
					line_count = 0;
				if (--height == 0)
					break;
			}
		} else
		{
			c = *src++;
			if (c == 0x80)
			{
				/* bitstring */
				c = *src++;
				while (c)
				{
					++bytecount;
					if (bytecount < bytes)
					{
						dst[offset] = *src;
					} else if (bytecount == bytes)
					{
						dst[offset] = *src & lastmask;
						if (!(offset & 1))
							dst[offset + 1] = 0;
					}
					src++;
					offset++;
					if (!(offset & 1))
						offset += planeoff;
					c--;
				}
			} else if (c != 0)
			{
				/* solidrun */
				c2 = c & 0x80 ? 0xff : 0x00;
				c &= 0x7f;
				while (c)
				{
					++bytecount;
					if (bytecount < bytes)
					{
						dst[offset] = c2;
					} else if (bytecount == bytes)
					{
						dst[offset] = c2 & lastmask;
						if ((!offset & 1))
							dst[offset + 1] = 0;
					}
					offset++;
					if (!(offset & 1))
						offset += planeoff;
					c--;
				}
			} else
			{
				c = *src++;
				if (c != 0)
				{
					/* patternrun */
					_UWORD patsize;
					_WORD patcount;

					patsize = patlen;
					patsize *= c;
					patcount = 0;
					while (patsize)
					{
						++bytecount;
						if (bytecount < bytes)
						{
							dst[offset] = src[patcount];
						} else if (bytecount == bytes)
						{
							dst[offset] = src[patcount] & lastmask;
							if (!(offset & 1))
								dst[offset + 1] = 0;
						}
						++patcount;
						if (patcount == patlen)
							patcount = 0;
						offset++;
						if (!(offset & 1))
							offset += planeoff;
						patsize--;
					}
					src += patlen;
				} else
				{
					c = *src;
					if (c == 0xff)
					{
						src++;
						c = *src++;
						line_count = c;
						line_start = src;
					}
				}
			}
		}
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean img_unpack_safe(_UBYTE *dst, const _UBYTE *src, PICTURE *pic)
{
	_WORD width = pic->pi_width;
	_WORD height = pic->pi_height;
	_WORD planes = pic->pi_planes;
	_WORD patlen = pic->pi_pat_len;
	_UWORD bytes = tobyte(width);
	_WORD planecount;
	_UWORD bytecount;
	_ULONG planeoff;
	_ULONG lineoff;
	_ULONG nextplane;
	_ULONG offset;
	_WORD line_count;
	const _UBYTE *line_start;
	_UBYTE c, c2;
	_UBYTE lastmask;
	const _UBYTE *srcend;
	_ULONG dstend;
	
	planeoff = planes;
	planeoff += planeoff;
	planeoff -= 2;

	bytecount = (bytes + 1) & ~1; /* round up to words */
	lastmask = masks[width & 7];

	lineoff = bytecount;
	lineoff = lineoff * planes - 2;

	offset = bytecount;
	offset *= planes;
	offset -= planeoff;
	offset -= 2;
	nextplane = offset;

	bytecount = 0;
	offset = 0;
	line_count = 0;
	planecount = planes;
	line_start = NULL;
	
	srcend = src + pic->pi_datasize;
	dstend = pic->pi_picsize;
	
	for (;;)
	{
		if (bytecount >= bytes)
		{
			bytecount = 0;
			if (offset & 1)
			{
				if (offset >= dstend)
					return FALSE;
				dst[offset] = 0;
				offset++;
				offset += planeoff;
			}
			offset -= lineoff;
			if (--planes == 0)
			{
				planes = planecount;
				offset += nextplane;
				if (--line_count > 0)
					src = line_start;
				else
					line_count = 0;
				if (--height == 0)
					break;
			}
		} else
		{
			if (src >= srcend)
				return FALSE;
			c = *src++;
			if (c == 0x80)
			{
				/* bitstring */
				if (src >= srcend)
					return FALSE;
				c = *src++;
				while (c)
				{
					++bytecount;
					if (bytecount < bytes)
					{
						if (src >= srcend)
							return FALSE;
						if (offset >= dstend)
							return FALSE;
						dst[offset] = *src;
					} else if (bytecount == bytes)
					{
						if (src >= srcend)
							return FALSE;
						if (offset >= dstend)
							return FALSE;
						dst[offset] = *src & lastmask;
						if (!(offset & 1))
							dst[offset + 1] = 0;
					}
					src++;
					offset++;
					if (!(offset & 1))
						offset += planeoff;
					c--;
				}
			} else if (c != 0)
			{
				/* solidrun */
				c2 = c & 0x80 ? 0xff : 0x00;
				c &= 0x7f;
				while (c)
				{
					++bytecount;
					if (bytecount < bytes)
					{
						if (offset >= dstend)
							return FALSE;
						dst[offset] = c2;
					} else if (bytecount == bytes)
					{
						if (offset >= dstend)
							return FALSE;
						dst[offset] = c2 & lastmask;
						if (!(offset & 1))
							dst[offset + 1] = 0;
					}
					offset++;
					if (!(offset & 1))
						offset += planeoff;
					c--;
				}
			} else
			{
				if (src >= srcend)
					return FALSE;
				c = *src++;
				if (c != 0)
				{
					/* patternrun */
					_UWORD patsize;
					_WORD patcount;

					patsize = patlen;
					patsize *= c;
					patcount = 0;
					while (patsize)
					{
						++bytecount;
						if (bytecount < bytes)
						{
							if (offset >= dstend)
								return FALSE;
							if ((src + patcount) >= srcend)
								return FALSE;
							dst[offset] = src[patcount];
						} else if (bytecount == bytes)
						{
							if (offset >= dstend)
								return FALSE;
							if ((src + patcount) >= srcend)
								return FALSE;
							dst[offset] = src[patcount] & lastmask;
							if (!(offset & 1))
								dst[offset + 1] = 0;
						}
						++patcount;
						if (patcount == patlen)
							patcount = 0;
						offset++;
						if (!(offset & 1))
							offset += planeoff;
						patsize--;
					}
					src += patlen;
				} else
				{
					if (src >= srcend)
						return FALSE;
					c = *src;
					if (c == 0xff)
					{
						src++;
						if (src >= srcend)
							return FALSE;
						c = *src++;
						line_count = c;
						line_start = src;
					}
				}
			}
		}
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

LOCAL _WORD do_compress(const _UBYTE *start, const _UBYTE *end)
{
	_WORD i;
	_UBYTE c;

	for (i = 0, c = *start; &start[i] < end; i++)
	{
		if (start[i] != start[i & 1])
			break;
	}
	if (i < 3)
	{
		i = 0;
	} else
	{
		if ((c != 0 && c != 0xff) || start[1] != c)
		{
	 		i &= -2;
			if (i < 6)
				i = 0;
		}
	}
	return i;
}


LOCAL _UBYTE *put_uncompressed(_UBYTE *outptr, const _UBYTE *start, const _UBYTE *end)
{
	size_t n = end - start;

	if (n != 0)
	{
		while (n > 127)
		{
			_UBYTE c = 127;

			put_byte(0x80);
			c = 127;
			put_byte(c);
			while (c)
			{
				put_byte(*start);
				start++;
				c--;
			}
			n -= 127;
		}
		put_byte(0x80);
		put_byte(n);
		while (start != end)
		{
			put_byte(*start);
			start++;
		}
	}
	return outptr;
}


LOCAL _UBYTE *put_compressed(_UBYTE *outptr, const _UBYTE *start, _WORD n)
{
	_UBYTE c1;
	_UBYTE c2;

	c1 = *start++;
	c2 = *start;
	if (c1 == 0 && c2 == 0)
	{
		while (n > 127)
		{
			put_byte(127);
			n -= 127;
		}
		put_byte(n);
	} else if (c1 == 0xff && c2 == 0xff)
	{
		while (n > 127)
		{
			put_byte(0xff);
			n -= 127;
		}
		put_byte(n + 0x80);
	} else
	{
		n >>= 1;
		while (n > 127)
		{
			put_byte(0x00);
			put_byte(127);
			put_byte(c1);
			put_byte(c2);
			n -= 127;
		}
		put_byte(0x00);
		put_byte(n);
		put_byte(c1);
		put_byte(c2);
	}
	return outptr;
}


LOCAL _UBYTE *compress_line(_UBYTE *outptr, const _UBYTE *buf, _WORD rect_bytes)
{
	const _UBYTE *pend;
	const _UBYTE *start;
	const _UBYTE *lastend;
	_WORD x;

	pend = buf + rect_bytes;
	start = buf;
	lastend = buf;
	while (lastend != pend)
	{
		if ((x = do_compress(lastend, pend)) != 0)
		{
			outptr = put_uncompressed(outptr, start, lastend);
			outptr = put_compressed(outptr, lastend, x);
			lastend += x;
			start = lastend;
		} else
		{
			lastend++;
		}
	}
	outptr = put_uncompressed(outptr, start, lastend);
	return outptr;
}


LOCAL _WORD cmp_lines(const _UBYTE *buf, _WORD maxlines, _WORD rect_bytes, _WORD plane_bytes)
{
	_WORD j;
	_WORD i;
	const _UBYTE *p1;
	const _UBYTE *p2;

	if (maxlines > 255)
		maxlines = 255;
	for (i = 0; i < maxlines; i++)
	{
		p1 = buf;
		p2 = ((_LONG)plane_bytes * i) + buf;
		for (j = rect_bytes; j-- != 0; )
		{
			if (*p1++ != *p2++)
				return i;
		}
	}
	return i;
}


LOCAL _UBYTE *do_1line(_UBYTE *outptr, const _UBYTE *buf, _WORD rect_bytes, _WORD num_planes)
{
	_WORD i;
	const _WORD *p1;
	_WORD *p2;
	_WORD *linebuf;
	_WORD linesize;
	
	if (num_planes == 1)
	{
		outptr = compress_line(outptr, buf, rect_bytes);
	} else
	{
		p1 = (const _WORD *)buf;
		linesize = (rect_bytes + 1) >> 1;
		linebuf = g_new(_WORD, linesize);
		if (linebuf != NULL)
		{
			p2 = (_WORD *)linebuf;
			for (i = linesize; i != 0; i--)
			{
				*p2 = *p1;
				p2++;
				p1 += num_planes;
			}
			outptr = compress_line(outptr, (const _UBYTE *)linebuf, rect_bytes);
			g_free(linebuf);
		}
	}
	return outptr;
}


LOCAL _UBYTE *_packimg(
	_UBYTE *outptr,
	const _UBYTE *buf,
	_WORD rect_width,
	_WORD rect_height,
	_WORD num_planes)
{
	_WORD i;
	_WORD linecount;

	while (rect_height > 0)
	{
		linecount = cmp_lines(buf, rect_height, toword(rect_width) * num_planes, toword(rect_width) * num_planes);
		if (linecount > 1)
		{
			put_byte(0);
			put_byte(0);
			put_byte(0xff);
			put_byte(linecount);
		}
		for (i = 0; i < num_planes; i++)
		{
			outptr = do_1line(outptr, buf, tobyte(rect_width), num_planes);
			buf += 2;
		}
		buf += toword(rect_width) * ulmul(num_planes, linecount) - (num_planes << 1);
		rect_height -= linecount;
	}
	return outptr;
}


_LONG img_pack(_UBYTE *dest, const _UBYTE *src, PICTURE *pic)
{
	_UBYTE *end;

	dest += pic->pi_dataoffset;
	end = _packimg(dest, src, pic->pi_width, pic->pi_height, pic->pi_planes);
	return end - dest;
}

/*** ---------------------------------------------------------------------- ***/

_LONG img_header(_UBYTE **dest_p, PICTURE *pic)
{
	_WORD width = pic->pi_width;
	_WORD height = pic->pi_height;
	_WORD planes = pic->pi_planes;
	_LONG size;
	_UBYTE *outptr;
	_WORD headlen;

	headlen = 0;
	size = toword(width);
	/*
	 * worst (uncompressed) case is 2 additional bytes
	 * for every 128 bytes in a line
	 */
	size += ((size + 127) / 128) * 2;
	size = size * (_LONG)planes * height;
	if (planes > 1 && planes <= 8)
		headlen += SIZEOF_XIMG_HEADER(planes);
	headlen += GEM_HEADER_SIZE;
	pic->pi_dataoffset = headlen;
	size += headlen;
	if (*dest_p == NULL)
		outptr = g_new(_UBYTE, size);
	else
		outptr = *dest_p;
	if (outptr != NULL)
	{
		*dest_p = outptr;
		put_word(1);			/* version */
		headlen >>= 1;
		put_word(headlen);
		put_word(planes);
		put_word(2);			/* pat_len */
		put_word(pic->pi_pix_width);
		put_word(pic->pi_pix_height);
		put_word(width);
		put_word(height);
		if (planes > 1 && planes <= 8)
		{
			_WORD ncolors, i;
			_WORD vdipal[256 * 3];
			_WORD color;
			
			ncolors = (1 << planes);
			put_long(XIMG_MAGIC);
			put_word(0); /* model */
			
			pic_rgb_to_vdipal(vdipal, pic->pi_palette, ncolors, NULL);
			for (i = 0; i < ncolors; i++)
			{
				color = vdipal[i * 3 + 0];
				put_word(color);
				color = vdipal[i * 3 + 1];
				put_word(color);
				color = vdipal[i * 3 + 2];
				put_word(color);
			}
		}
		size = headlen << 1;
	} else
	{
		size = 0;
	}
	UNUSED(outptr);
	return size;
}
