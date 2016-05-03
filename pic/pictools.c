/*****************************************************************************
 * PICTOOLS.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>


unsigned long pic_pal_stddiff(const PICTURE *pic)
{
	_WORD color, ncolors;
	unsigned long sum, diff;
	const struct _rgb *rgbi, *rgbp;
	unsigned int dr, dg, db;
	const unsigned char *tab;
	
	if (pic->pi_planes != 4 && pic->pi_planes != 8)
		return 0;
	tab = pic->pi_planes == 4 ? bmp_coltab4 : bmp_coltab8;
	ncolors = 1 << pic->pi_planes;
	sum = 0;
	for (color = 0; color < ncolors; color++)
	{
		rgbp = &pic->pi_palette[color];
		rgbi = &std256_palette[tab[color]];
		if (rgbi->r >= rgbp->r)
			dr = rgbi->r - rgbp->r;
		else
			dr = rgbp->r - rgbi->r;
		if (rgbi->g >= rgbp->g)
			dg = rgbi->g - rgbp->g;
		else
			dg = rgbp->g - rgbi->g;
		if (rgbi->b >= rgbp->b)
			db = rgbi->b - rgbp->b;
		else
			db = rgbp->b - rgbi->b;
		diff = (unsigned long)(dr * dr) + (unsigned long)(dg * dg) + (unsigned long)(db * db);
		sum += diff;
	}
	return sum;
}


static void matchpal(const PALETTE dst, const PALETTE src, unsigned char *pixel)
{
	_WORD i, j;
	_UWORD dr, dg, db;
	_ULONG diff, mindiff;
	const struct _rgb *rgbi, *rgbp;
	_UBYTE best;
	
	for (i = 0; i < 256; i++)
	{
		rgbp = &src[i];
		mindiff = 0x4000000UL;
		best = i;
		for (j = 0; j < 256; j++)
		{
			rgbi = &dst[j];
			if (rgbi->r >= rgbp->r)
				dr = rgbi->r - rgbp->r;
			else
				dr = rgbp->r - rgbi->r;
			if (rgbi->g >= rgbp->g)
				dg = rgbi->g - rgbp->g;
			else
				dg = rgbp->g - rgbi->g;
			if (rgbi->b >= rgbp->b)
				db = rgbi->b - rgbp->b;
			else
				db = rgbp->b - rgbi->b;
			diff = (_ULONG)(dr * dr) + (_ULONG)(dg * dg) + (_ULONG)(db * db);
			if (diff < mindiff)
			{
				best = j;
				mindiff = diff;
			}	
		}
		pixel[i] = best;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void copy_block(unsigned char *dst, const unsigned char *src, const PICTURE *pic, const unsigned char *pixels)
{
	_UWORD mask;
	_UBYTE color, colmask;
	_ULONG srclinesize, dstlinesize, offset;
	_WORD x, y, p;
	const unsigned char *ptr;
	unsigned char *dptr;
	_WORD w = pic->pi_width;
	_WORD h = pic->pi_height;
	static unsigned char const masktab[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	
	srclinesize = (_ULONG)((w + 15) >> 4) * (_ULONG)pic->pi_planes * 2;
	dstlinesize = srclinesize;
	printf("size %lu %lu\n", pic->pi_picsize, dstlinesize * h);
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			mask = masktab[x & 0x07];
			offset = y * srclinesize + (((x >> 4) * pic->pi_planes) << 1);
			if (x & 0x08)
				offset++;
			ptr = src + offset;
			color = 0;
			colmask = 0x01;
			for (p = 0; p < pic->pi_planes; p++)
			{
				if (*ptr & mask)
					color |= colmask;
				ptr += 2;
				colmask <<= 1;
			}
			
			color = pixels[color];
			dptr = dst + offset;
			colmask = 0x01;
			for (p = 0; p < pic->pi_planes; p++)
			{
				if (color & colmask)
					*dptr |= mask;
				dptr += 2;
				colmask <<= 1;
			}
		}
	}
}

gboolean pic_match_stdpal(PICTURE *pic, unsigned char *buf)
{
	unsigned char *tmp;
	unsigned char pixels[256];
	
	tmp = g_new0(unsigned char, pic->pi_picsize);
	if (tmp == NULL)
		return FALSE;
	matchpal(std256_palette, pic->pi_palette, pixels);
	copy_block(tmp, buf, pic, pixels);
	memcpy(buf, tmp, pic->pi_picsize);
	g_free(tmp);
	memcpy(pic->pi_palette, std256_palette, sizeof(std256_palette));
	return TRUE;
}
