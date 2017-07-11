/*****************************************************************************
 * PNG.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>


#define get_long() \
	(((_ULONG)(buf[0]) << 24) | \
	 ((_ULONG)(buf[1]) << 16) | \
	 ((_ULONG)(buf[2]) <<  8) | \
	 ((_ULONG)(buf[3])      )), buf += 4
#define get_word() \
	(((_UWORD)(buf[0]) <<  8) | \
	 ((_UWORD)(buf[1])      )), buf += 2
#define get_byte() \
	*buf++

static unsigned char const png_sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };


/*
 * Do this without libpng functions, because e
 * we want to be able to identify PNG files even
 * if libpng is not available
 */
gboolean pic_type_png(PICTURE *pic, const _UBYTE *buf, _LONG size)
{
	_UBYTE depth, type, compress_method;
	const _UBYTE *end;
	const _UBYTE *start;
	_ULONG length, magic;
	int plte_size = 0;
	int i;
	
	if (size < 24)
		return FALSE;
	if (memcmp(buf, png_sig, 8) != 0)
		return FALSE;
	end = buf + size - 8;
	buf += 8;
	while (buf < end)
	{
		length = get_long();
		magic = get_long();
		start = buf;
		if (magic == 0x49484452ul) /* 'IHDR' */
		{
			if (length < 13 || buf + length > end)
				return FALSE;
			pic->pi_width = (_WORD) get_long();
			pic->pi_height = (_WORD) get_long();
			depth = *buf++;
			if (depth < 1 || depth > 8)
				return FALSE;
			type = *buf++;
			compress_method = *buf;
			if (compress_method != 0)
				return FALSE;
			switch (type)
			{
			case 0: /* grayscale */
				pic->pi_planes = depth;
				break;
			case 2: /* RGB */
				pic->pi_planes = depth * 4;
				break;
			case 3:/* palette based */
				pic->pi_planes = depth;
				plte_size = 1 << pic->pi_planes;
				break;
			case 4: /* gray + alpha */
				pic->pi_planes = depth * 2;
				break;
			case 6:/* RGBA */
				pic->pi_planes = depth * 4;
				break;
			default:
				return FALSE;
			}
			pic->pi_type = FT_PNG;
			pic->pi_compressed = TRUE;
			pic_calcsize(pic);
			if (plte_size == 0)
				return TRUE;
		} else if (magic == 0x504c5445ul) /* 'PLTE' */
		{
			if ((int)(length / 3) > plte_size)
				return FALSE;
			plte_size = (int)(length / 3);
			for (i = 0; buf < end && i < plte_size; i++)
			{
				pic->pi_palette[i].r = *buf++;
				pic->pi_palette[i].g = *buf++;
				pic->pi_palette[i].b = *buf++;
			}
			return TRUE;
		}
		buf = start + length + 4; /* skip data + CRC */
	}
	return FALSE;
}
