/*****************************************************************************
 * ICO.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>

#define tobyte(pixels) (((pixels) + 7) >> 3)
#define toword(pixels) (((pixels) + 15) >> 4)

typedef struct {
	_UBYTE magic[2];
	_UBYTE type[2];
	_UBYTE count[2];
	struct {
		_UBYTE width;		/* width in pixel */
		_UBYTE height;		/* height in pixel */
		_UBYTE colors;		/* number of colors: 2, 8, 16, 256 */
		_UBYTE dummy;
		_UBYTE xhotspot[2];
		_UBYTE yhotspot[2];
		_UBYTE datasize[4]; /* size of BITMAPINFO + data */
		_UBYTE offset[4];	/* offset to BITMAPINFO (22 for 1 icon) */
	} ico[1]; /* repeated for #count */
#define ICO_FILE_HEADER_SIZE 22
	
	_UBYTE biSize[4];	/* size of BITMAPINFO (12/40/64) */
	_UBYTE biWidth[4];
	_UBYTE biHeight[4];
	_UBYTE biPlanes[2];
	_UBYTE biBitCount[2];
	_UBYTE biCompression[4];
	_UBYTE biSizeImage[4];
	_UBYTE biXPelsPerMeter[4];
	_UBYTE biYPelsPerMeter[4];
	_UBYTE biClrUsed[4];
	_UBYTE biClrImportant[4];
	_UBYTE bmiColors[256][4];
} ICO_HEADER;

/*** ---------------------------------------------------------------------- ***/

#define put_long(l) \
	*buf++ = (_UBYTE)((l)	   ); \
	*buf++ = (_UBYTE)((l) >>  8); \
	*buf++ = (_UBYTE)((l) >> 16); \
	*buf++ = (_UBYTE)((l) >> 24)
#define put_word(w) \
	*buf++ = (_UBYTE)((w)	   ); \
	*buf++ = (_UBYTE)((w) >>  8)
#define put_byte(b) \
	*buf++ = (_UBYTE)(b)

#define get_long() \
	(((_ULONG)(buf[3]) << 24) | \
	 ((_ULONG)(buf[2]) << 16) | \
	 ((_ULONG)(buf[1]) <<  8) | \
	 ((_ULONG)(buf[0])		)), buf += 4
#define get_word() \
	(((_UWORD)(buf[1]) <<  8) | \
	 ((_UWORD)(buf[0])		)), buf += 2
#define get_byte() \
	*buf++

/*** ---------------------------------------------------------------------- ***/

gboolean pic_type_ico(PICTURE *pic, const unsigned char *buf, long size)
{
	short width, height, colors, i;
	short planes;
	short mapentrysize = 0;
	long clrused = 0;
	short magic, type, count;
	long planesize, datasize, dsize, hsize, bmphsize, cmapsize;
	const unsigned char *rp;

	if (size < (ICO_FILE_HEADER_SIZE + 12))
	{
		return FALSE;
	}
	magic = get_word();
	type = get_word();
	count = get_word();
	if (magic != 0 || (type != 1 && type != 2) || count <= 0 || count >= 16)
		return FALSE;
	width = get_byte();
	height = get_byte();
	colors = get_byte();
	if (colors == 0)
		colors = 256;
	buf += 5; /* skip stuff2 */
	dsize = get_long();
	(void) dsize;
	hsize = get_long();
	if (hsize < ICO_FILE_HEADER_SIZE)
	{
		return FALSE;
	}
	
	switch (colors)
	{
		case 0: pic->pi_planes = 0; break;
		case 2: pic->pi_planes = 1; break;
		case 8: pic->pi_planes = 4; break;
		case 16: pic->pi_planes = 4; break;
		case 256: pic->pi_planes = 8; break;
		default:
			return FALSE;
	}
	buf += hsize - ICO_FILE_HEADER_SIZE;
	size -= hsize;
	if (size < 12)
	{
		return FALSE;
	}
	bmphsize = get_long();
	if (size <= bmphsize)
	{
		return FALSE;
	}
	pic->pi_height = height;
	switch ((int) bmphsize)
	{
	case 12: /* OS/2 1.x, BITMAPCOREHEADER */
		width = get_word();
		height = get_word();
		if (height < 0)
			height = -height;
		else
			pic->pi_topdown = FALSE;
		planes = get_word();
		pic->pi_planes = get_word();
		pic->pi_compressed = 0;
		switch (pic->pi_planes)
		{
		case 1:
		case 4:
		case 8:
			mapentrysize = 3;
			break;
		case 24:
		case 32:
			break;
		default:
			return FALSE;
		}
		break;
	case 40: /* BITMAPINFOHEADER */
	case 52: /* BITMAPV2INFOHEADER */
	case 56: /* BITMAPV3INFOHEADER */
	case 64: /* OS/2 BITMAPINFOHEADER2 */
	case 108: /* BITMAPV4HEADER */
	case 124: /* BITMAPV5HEADER */
		width = (short) get_long();
		height = (short) get_long();
		if (height < 0)
			height = -height;
		else
			pic->pi_topdown = FALSE;
		planes = get_word();
		pic->pi_planes = get_word();
		pic->pi_compressed = (short) get_long();
		pic->pi_datasize = get_long();
		pic->pi_pix_width = (short) get_long();
		pic->pi_pix_height = (short) get_long();
		clrused = get_long();
		buf += 4; /* skip ClrImportant */
		
		buf += bmphsize - 40; /* skip remaining header */
		switch (pic->pi_planes)
		{
		case 1:
		case 4:
		case 8:
			mapentrysize = 4;
			break;
		case 24:
		case 32:
			break;
		default:
			return FALSE;
		}
		break;
	default:
		return FALSE;
	}
	
	/*
	 * height in BITMAPHEADER sometimes includes the mask height
	 */
	if (height == pic->pi_height * 2)
		height = pic->pi_height;
	
	pic->pi_type = FT_ICO;
	pic->pi_width = width;
	pic->pi_height = height;
	if (planes != 1 || pic->pi_planes == 0 || pic->pi_compressed != 0)
	{
		pic->pi_unsupported = TRUE;
		return TRUE;
	}
	pic_calcsize(pic);

	planesize = bmp_rowsize(pic, pic->pi_planes) * height;
	if (planesize == 0)
		return FALSE;
	if (mapentrysize > 0)
	{
		if (colors == 0)
			colors = clrused;
		if (clrused <= 0)
			clrused = colors;
		else if (clrused > colors)
			return FALSE;
	}
	cmapsize = clrused * mapentrysize;
	datasize = planesize + bmp_rowsize(pic, 1) * height + hsize + cmapsize + bmphsize;

	if (pic->pi_filesize < datasize /* || pic->pi_filesize > (datasize + 2) */)
	{
		return FALSE;
	}
	
	pic->pi_dataoffset = hsize + bmphsize + cmapsize;
	pic->pi_datasize = planesize;
	
	rp = buf;				/* skip rest of header, header size already read above */
	size -= bmphsize;
	if (size < cmapsize)
	{
		return FALSE;
	}
	if (mapentrysize > 0)
	{
		for (i = 0; i < clrused; i++)		/* Farbpalette uebernehmen */
		{
			pic->pi_palette[i].r = rp[2];	/* R-Anteil */
			pic->pi_palette[i].g = rp[1];	/* G-Anteil */
			pic->pi_palette[i].b = rp[0];	/* B-Anteil */
			rp += mapentrysize;
		}
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean ico_unpack(_UBYTE *dest, const _UBYTE *src, PICTURE *pic)
{
	return bmp_unpack(dest, src, pic);
}

/*** ---------------------------------------------------------------------- ***/

_LONG ico_header(_UBYTE **dest_p, PICTURE *pic, const unsigned char *maptab)
{
	_ULONG len, headlen;
	_WORD ncolors;
	_ULONG cmapsize;
	unsigned char *buf;
	struct _rgb {
		_UBYTE r;
		_UBYTE g;
		_UBYTE b;
	};
#if 0
	static struct _rgb const win16_palette[16] = {
		{	0,	 0,   0 },
		{ 128,	 0,   0 },
		{	0, 128,   0 },
		{ 128, 128,   0 },
		{	0,	 0, 128 },
		{ 128,	 0, 128 },
		{	0, 128, 128 },
		{ 128, 128, 128 },
		{ 192, 192, 192 },
		{ 255,	 0,   0 },
		{	0, 255,   0 },
		{ 255, 255,   0 },
		{	0,	 0, 255 },
		{ 255,	 0, 255 },
		{	0, 255, 255 },
		{ 255, 255, 255 }
	};
#endif

	ncolors = 1 << pic->pi_planes;
	cmapsize = 4 * ncolors;
	headlen = ICO_FILE_HEADER_SIZE + 40 + cmapsize;
	len = headlen + pic->pi_datasize;

	if (*dest_p == NULL)
		buf = g_new(unsigned char, len);
	else
		buf = *dest_p;
	if (buf == NULL)
		return 0;
	*dest_p = buf;
	pic->pi_dataoffset = headlen;
	
	put_word(0); /* magic */
	put_word(1); /* type */
	put_word(1); /* count */
	put_byte(pic->pi_width);
	put_byte(pic->pi_height);
	put_byte(ncolors);
	put_byte(0);
	put_word(1);					/* planes */
	put_word(pic->pi_planes);		/* bits per pixel */
	put_long(len - ICO_FILE_HEADER_SIZE);
	put_long((int32_t)ICO_FILE_HEADER_SIZE);
	
	put_long(40l);
	put_long((_LONG)(pic->pi_width));
	put_long((_LONG)(pic->pi_height * 2)); 	/* 1 for data, 1 for mask */
	put_word(1);					/* planes */
	put_word(pic->pi_planes);		/* bits per pixel */
	put_long((_LONG)(pic->pi_compressed));
	put_long((_LONG)(pic->pi_datasize));
	put_long(0l);					/* pix_width */
	put_long(0l);					/* pix_height */
	put_long((_LONG)(ncolors));
	put_long(0l);

	bmp_put_palette(buf, pic, maptab);
	
	return headlen;
}

/*** ---------------------------------------------------------------------- ***/

_LONG ico_pack(_UBYTE *dest, const _UBYTE *data, const _UBYTE *mask, PICTURE *pic, const unsigned char *maptab)
{
	long datasize;
	
	pic->pi_compressed = 0 /* BMP_RGB */;
	datasize = bmp_pack_data_and_mask(dest, data, mask, pic, FALSE, maptab);
	return datasize;
}
