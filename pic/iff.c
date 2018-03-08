/*****************************************************************************
 * IFF.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>

typedef struct {                /* Header fuer IFF-Bilder */
    _UBYTE kennung[4];          /* "FORM" */
    _UBYTE filelen[4];          /* Dateilaenge */
} IFF_HEADER;

#define get_long(p) \
	(((_ULONG)(p[0]) << 24) | \
	 ((_ULONG)(p[1]) << 16) | \
	 ((_ULONG)(p[2]) <<  8) | \
	 ((_ULONG)(p[3])      ))
#define get_word(p) \
	(((_UWORD)(p[0]) <<  8) | \
	 ((_UWORD)(p[1])      ))
	
/*** ---------------------------------------------------------------------- ***/

gboolean pic_type_iff(PICTURE *pic, const _UBYTE *buf, _LONG size)
{
	_LONG len;
	const _UBYTE *bmhd_chunk;
	const _UBYTE *cmap_chunk;
	_LONG offset;
	_WORD i;
	const IFF_HEADER *header = (const IFF_HEADER *)buf;

	if (size <= IFF_HEADER_SIZE ||
		header->kennung[0] != 'F' ||
		header->kennung[1] != 'O' ||
		header->kennung[2] != 'R' ||
		header->kennung[3] != 'M')
		return FALSE;
	len = get_long(header->filelen);
	if (len != (pic->pi_filesize - IFF_HEADER_SIZE))
		return FALSE;
	offset = IFF_HEADER_SIZE;
	buf += offset;
	offset += 4;
	if (offset > size)
		return FALSE;
	if (buf[0] != 'I' ||
		buf[1] != 'L' ||
		buf[2] != 'B' ||
		buf[3] != 'M')
		return FALSE;
	buf += 4;

	offset += 4;
	if (offset > size)
		return FALSE;
	if (buf[0] != 'B' ||
		buf[1] != 'M' ||
		buf[2] != 'H' ||
		buf[3] != 'D')
		return FALSE;
	buf += 4;
	
	offset += 4;
	if (offset > size)
		return FALSE;
	len = get_long(buf);
	buf += 4;
	bmhd_chunk = buf;
	
	offset += len;
	buf += len;
	offset += 4;
	if (offset > size)
		return FALSE;
	if (buf[0] != 'C' ||
		buf[1] != 'M' ||
		buf[2] != 'A' ||
		buf[3] != 'P')
	{
		cmap_chunk = NULL;
	} else
	{
		cmap_chunk = buf + 8;
	}
	offset -= 4;
	
	for (;;)
	{
		offset += 4;
		if (offset > size)
			return FALSE;
		if (buf[0] == 'B' &&
			buf[1] == 'O' &&
			buf[2] == 'D' &&
			buf[3] == 'Y')
			break;
		buf += 4;

		offset += 4;
		if (offset > size)
			return FALSE;
		len = get_long(buf);
		offset += len;
		buf += 4;
		buf += len;
	}
	buf += 4;
	offset += 4;
	pic->pi_dataoffset = offset;
	pic->pi_datasize = get_long(buf);
	buf += 4;
	UNUSED(buf);
		
	pic->pi_width = get_word(bmhd_chunk);
	bmhd_chunk += 2;
	pic->pi_height = get_word(bmhd_chunk);
	bmhd_chunk += 6;
	pic->pi_planes = *bmhd_chunk;
	if (pic->pi_planes > 4)
		pic->pi_planes = 4;
	bmhd_chunk += 2;
	pic->pi_compressed = *bmhd_chunk;
	
	if (cmap_chunk != NULL)
	{
		buf = cmap_chunk;
		for (i = 0; i < 16; i++)
		{
			pic->pi_palette[i].r = buf[0];
			pic->pi_palette[i].g = buf[1];
			pic->pi_palette[i].b = buf[2];
			buf += 3;
		}
	} else
	{
		pic_stdpalette(pic->pi_palette, 4);
	}
	pic->pi_type = FT_IFF;
    pic_calcsize(pic);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

#define put_long(buf, l) \
	*buf++ = (_UBYTE)((l) >> 24); \
	*buf++ = (_UBYTE)((l) >> 16); \
	*buf++ = (_UBYTE)((l) >>  8); \
	*buf++ = (_UBYTE)((l)      )
#define put_word(buf, w) \
	*buf++ = (_UBYTE)((w) >>  8); \
	*buf++ = (_UBYTE)(w)
#define put_byte(buf, b) \
	*buf++ = (_UBYTE)(b)

_LONG iff_header(_UBYTE *buf, PICTURE *pic)
{
	_ULONG len, headlen;
	_WORD ncolors, i;
	_ULONG cmapsize;

	ncolors = 1 << pic->pi_planes;
	cmapsize = 3 * ncolors;

	put_long(buf, 0x464f524dl); /* "FORM" */
	headlen = 48 + cmapsize;
	len = headlen + pic->pi_datasize;
	put_long(buf, len);

	put_long(buf, 0x494c424dl); /* "ILBM" */

	put_long(buf, 0x424d4844l); /* "BMHD" */
	put_long(buf, 20l);
	put_word(buf, pic->pi_width);
	put_word(buf, pic->pi_height);
	put_long(buf, 0l);
	put_byte(buf, pic->pi_planes);
	put_byte(buf, 0);
	put_byte(buf, pic->pi_compressed);
	put_byte(buf, 0);
	put_word(buf, 0);
	put_word(buf, 0x0a0b);
	put_word(buf, pic->pi_width);
	put_word(buf, pic->pi_height);

	put_long(buf, 0x434d4150l); /* "CMAP" */
	put_long(buf, cmapsize);
	for (i = 0; i < ncolors; i++)
	{
		put_byte(buf, pic->pi_palette[i].r);
		put_byte(buf, pic->pi_palette[i].g);
		put_byte(buf, pic->pi_palette[i].b);
	}

	put_long(buf, 0x424f4459l); /* BODY */
	put_long(buf, pic->pi_datasize);
	UNUSED(buf);
	
	return headlen + IFF_HEADER_SIZE;
}
