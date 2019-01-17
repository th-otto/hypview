/*****************************************************************************
 * BMP.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>

typedef struct {
	/* file header */
	unsigned char magic[2];		/* BM */
	unsigned char filesize[4];
	unsigned char xHotSpot[2];     
	unsigned char yHotSpot[2];
	unsigned char offbits[4];		/* offset to data */

	/* info header */
	union {
		struct {
			unsigned char hsize[4];			/* size of info header (40) */
			unsigned char width[4];			/* width in pixels */
			unsigned char height[4];		/* height in pixels */
			unsigned char planes[2];		/* always 1 */
			unsigned char bitcount[2];		/* bits per pixel: 1, 4, 8, 24 or 32 */
			unsigned char compression[4];	/* compression method */
#define BMP_RGB 0
#define BMP_RLE8 1
#define BMP_RLE4 2
#define BMP_BITFIELDS 3
#define BMP_JPEG 4
#define BMP_PNG 5
			unsigned char sizeImage[4];		/* size of data */
			unsigned char xPelsPerMeter[4];
			unsigned char yPelsPerMeter[4];
			unsigned char clrUsed[4];		/* # of colors used */
			unsigned char clrImportant[4];	/* # of important colors */
			unsigned char RedMask[4];
			unsigned char GreenMask[4];
			unsigned char BlueMask[4];
			unsigned char AlphaMask[4];
		} bitmapinfoheader;
		struct {
			unsigned char hsize[4];			/* size of info header (12) */
			unsigned char width[2];			/* width in pixels */
			unsigned char height[2];		/* height in pixels */
			unsigned char planes[2];		/* always 1 */
			unsigned char bitcount[2];		/* bits per pixel: 1, 4, 8, 24 or 32 */
		} bitmapcoreheader;
	} bmp_info_header;
} BMP_HEADER;


#define BMP_HEADER_SIZE (14 + 64)
#define BMP_HEADER_BUFSIZE (BMP_HEADER_SIZE + 256 * 4)


#define put_long(l) \
	*buf++ = (unsigned char)((l)	   ); \
	*buf++ = (unsigned char)((l) >>  8); \
	*buf++ = (unsigned char)((l) >> 16); \
	*buf++ = (unsigned char)((l) >> 24)
#define put_word(w) \
	*buf++ = (unsigned char)((w)	   ); \
	*buf++ = (unsigned char)((w) >>  8)
#define put_byte(b) \
	*buf++ = (unsigned char)(b)

#define get_long() \
	(((unsigned long)(buf[3]) << 24) | \
	 ((unsigned long)(buf[2]) << 16) | \
	 ((unsigned long)(buf[1]) <<  8) | \
	 ((unsigned long)(buf[0])		)), buf += 4
#define get_word() \
	(((unsigned short)(buf[1]) <<  8) | \
	 ((unsigned short)(buf[0])		)), buf += 2
#define get_byte() \
	*buf++

typedef struct _rle4 {
	short enc_count;
	short abs_count;
	unsigned char c1, c2;
	gboolean need_pad;
	gboolean need_byte;
	gboolean decode_ok;
	gboolean (*get_c)(struct _rle4 *, unsigned char *);
	union {
		struct {
			FILE *fp;
		} f;
		struct {
			const unsigned char *buf;
			long bufsize;
		} m;
	} b;
} RLE4;

unsigned char const bmp_coltab8[256] = {
 255,	0,	 1,   2,   4,	6,	 3,   5,   7,	8,	 9,  10,  12,  14,	11,  13,
  16,  17,	18,  19,  20,  21,	22,  23,  24,  25,	26,  27,  28,  29,	30,  31,
  32,  33,	34,  35,  36,  37,	38,  39,  40,  41,	42,  43,  44,  45,	46,  47,
  48,  49,	50,  51,  52,  53,	54,  55,  56,  57,	58,  59,  60,  61,	62,  63,
  64,  65,	66,  67,  68,  69,	70,  71,  72,  73,	74,  75,  76,  77,	78,  79,
  80,  81,	82,  83,  84,  85,	86,  87,  88,  89,	90,  91,  92,  93,	94,  95,
  96,  97,	98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,  15
};
unsigned char const bmp_revtab8[256] = {
   1,   2,   3,   6,   4,   7,   5,   8,   9,  10,  11,  14,  12,  15,  13, 255,
  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,   0
};

	/* FF 88 99 BB	AA EE CC DD  00 77 11 33  22 66 44 55 */
#if 0
unsigned char const bmp_coltab4[16] = { 15,  9, 10, 11, 12, 13, 14, 8, 7, 1,  2,  3,  4,  5,  6,  0 };
unsigned char const bmp_revtab4[16] = { 15,  9, 10, 11, 12, 13, 14, 8, 7, 1,  2,  3,  4,  5,  6,  0 };
#elif 0
unsigned char const bmp_coltab4[16] = { 15,  1,  2,  3, 12,  5,  6, 7, 8, 9, 10, 11,  4, 13, 14,  0 };
unsigned char const bmp_revtab4[16] = { 15,  1,  2,  3, 12,  5,  6, 7, 8, 9, 10, 11,  4, 13, 14,  0 };
#else
unsigned char const bmp_coltab4[16] = {  0,  1,  2,  3,  4,  5,  6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
unsigned char const bmp_revtab4[16] = {  0,  1,  2,  3,  4,  5,  6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
#endif

static struct _rgb const win2_palette[2] = {
	{	0,	 0,   0 },
	{ 255, 255, 255 }
};

unsigned char const bmp_idtab[256] = {
 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

/*** ---------------------------------------------------------------------- ***/

long bmp_rowsize(PICTURE *pic, _WORD planes)
{
	long bmp_bytes = pic->pi_width;
	
	switch (planes)
	{
	case 1:
		/* get_dib_stride */
		bmp_bytes = ((((bmp_bytes) + 7) >> 3) + 3) & ~3;
		break;
	case 4:
		bmp_bytes = (((bmp_bytes + 1) >> 1) + 3) & ~3;
		break;
	case 8:
		bmp_bytes = (bmp_bytes + 3) & ~3;
		break;
	case 24:
		bmp_bytes = ((bmp_bytes * 3) + 3) & ~3;
		break;
	case 32:
		bmp_bytes = bmp_bytes << 2;
		break;
	default:
		bmp_bytes = 0;
		break;
	}
	return bmp_bytes;
}

/*** ---------------------------------------------------------------------- ***/

#if 0
static void make_revtab8(void)
{
	int i;
	
	for (i = 0; i < 256; i++)
		bmp_revtab8[bmp_coltab8[i]] = i;
}
#endif

/*** ---------------------------------------------------------------------- ***/

gboolean pic_type_bmp(PICTURE *pic, const unsigned char *buf, long size)
{
	unsigned long fsize;
	unsigned long bmphsize;
	short planes;
	short mapentrysize = 0;
	long clrused = 0;
	short i;
	long bmp_bytes;
	
	if (size < 18)
		return FALSE;
	if (get_byte() != 'B' || get_byte() != 'M')
		return FALSE;
	fsize = get_long();
	/*
	 * don't check filesize in header, there are too many files
	 * where this doesn't match.
	 * Eg. most of the BMPs supplied with Windows are
	 * 2 bytes too long
	 */
	/* if (fsize != pic->pi_filesize)
		return FALSE; */
	(void) fsize;
	buf += 4; /* skip hotspot info */
	pic->pi_dataoffset = get_long();
	bmphsize = get_long();
	if (size <= (14 + (long)bmphsize))
		return FALSE;
	switch ((int)bmphsize)
	{
	case 12: /* OS/2 1.x, BITMAPCOREHEADER */
		pic->pi_width = get_word();
		pic->pi_height = get_word();
		if (pic->pi_height < 0)
			pic->pi_height = -pic->pi_height;
		else
			pic->pi_topdown = FALSE;
		planes = get_word();
		pic->pi_planes = get_word();
		pic->pi_compressed = BMP_RGB;
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
			pic->pi_type = FT_BMP;
			pic->pi_unsupported = TRUE;
			return TRUE;
		}
		break;
	case 40: /* BITMAPINFOHEADER */
	case 52: /* BITMAPV2INFOHEADER */
	case 56: /* BITMAPV3INFOHEADER */
	case 64: /* OS/2 BITMAPINFOHEADER2 */
	case 108: /* BITMAPV4HEADER */
	case 124: /* BITMAPV5HEADER */
		pic->pi_width = (short) get_long();
		pic->pi_height = (short) get_long();
		if (pic->pi_height < 0)
			pic->pi_height = -pic->pi_height;
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
			pic->pi_type = FT_BMP;
			pic->pi_unsupported = TRUE;
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	pic->pi_type = FT_BMP;
	if (planes != 1)
	{
		pic->pi_unsupported = TRUE;
		return TRUE;
	}
	pic_calcsize(pic);
	bmp_bytes = bmp_rowsize(pic, pic->pi_planes);
	if (bmp_bytes == 0)
		return FALSE;
	if (pic->pi_compressed != BMP_RGB)
	{
		if (pic->pi_planes == 4 && pic->pi_compressed != BMP_RLE4)
		{
			pic->pi_unsupported = TRUE;
			return TRUE;
		}
		if (pic->pi_planes == 8 && pic->pi_compressed != BMP_RLE8)
		{
			pic->pi_unsupported = TRUE;
			return TRUE;
		}
		if (pic->pi_planes != 4 && pic->pi_planes != 8)
		{
			pic->pi_unsupported = TRUE;
			return TRUE;
		}
	} else
	{
		pic->pi_datasize = bmp_bytes * pic->pi_height;
	}

	if (mapentrysize > 0)
	{
		if (clrused <= 0)
			clrused = 1 << pic->pi_planes;
		else if (clrused > (1 << pic->pi_planes))
			return FALSE;
		for (i = 0; i < clrused; i++)
		{
			pic->pi_palette[i].b = get_byte();
			pic->pi_palette[i].g = get_byte();
			pic->pi_palette[i].r = get_byte();
			if (mapentrysize == 4)
				buf++;
		}
	}
	pic->pi_type = FT_BMP;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void rle4_init(RLE4 *rle)
{
	rle->abs_count = 0;
	rle->enc_count = 0;
	rle->need_pad = FALSE;
	rle->need_byte = FALSE;
	rle->decode_ok = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

#if 0
static gboolean rle4_file_getc(RLE4 *rle, unsigned char *p)
{
	int c;

	if ((c = getc(rle->b.f.fp)) == EOF)
	{
		rle->decode_ok = FALSE;
		return FALSE;
	}
	*p = (unsigned char)c;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void rle4_file_init(RLE4 *rle, FILE *fp)
{
	rle4_init(rle);
	rle->b.f.fp = fp;
	rle->get_c = rle4_file_getc;
}
#endif

/*** ---------------------------------------------------------------------- ***/

static gboolean rle4_mem_getc(RLE4 *rle, unsigned char *p)
{
	if (rle->b.m.bufsize == 0)
	{
		rle->decode_ok = FALSE;
		return FALSE;
	}
	*p = *(rle->b.m.buf)++;
	--rle->b.m.bufsize;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void rle4_mem_init(RLE4 *rle, const unsigned char *buf, long bufsize)
{
	rle4_init(rle);
	rle->b.m.buf = buf;
	rle->b.m.bufsize = bufsize;
	rle->get_c = rle4_mem_getc;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean rle4_decode(RLE4 *rle, unsigned char *p)
{
	unsigned char cc;

	for (;;)
	{
		if (rle->enc_count > 0)
		{
			cc = rle->c1;
			rle->c1 =  rle->c2;
			rle->c2 = cc;
			rle->enc_count--;
		} else if (rle->abs_count > 0)
		{
			if (rle->need_byte)
			{
				if (rle->get_c(rle, &cc) == FALSE)
					return FALSE;
				rle->c1 = cc & 0x0f;
				cc >>= 4;
				rle->need_byte = FALSE;
			} else
			{
				cc = rle->c1;
				rle->need_byte = TRUE;
			}
			rle->abs_count--;
			if (rle->abs_count == 0 && rle->need_pad)
			{
				if (rle->get_c(rle, &rle->c2) == FALSE)
					return FALSE;
			}
		} else
		{
			if (rle->get_c(rle, &cc) == FALSE)
				return FALSE;
			if (cc == 0)
			{
				if (rle->get_c(rle, &cc) == FALSE)
					return FALSE;
				switch (cc)
				{
				case 0:
					continue;
				case 1:
					rle->decode_ok = TRUE;
					return FALSE;
				case 2:
					if (rle->get_c(rle, &rle->c1) == FALSE)
						return FALSE;
					if (rle->get_c(rle, &rle->c2) == FALSE)
						return FALSE;
					/* continue; */
					rle->decode_ok = FALSE;
					return FALSE;
				default:
					rle->abs_count = cc;
					rle->need_pad = rle->abs_count & 2;
					rle->need_byte = TRUE;
					break;
				}
			} else
			{
				rle->enc_count = cc;
				if (rle->get_c(rle, &cc) == FALSE)
					return FALSE;
				rle->c1 = cc >> 4;
				rle->c2 = cc & 0x0f;
			}
			continue;
		}
		*p = cc;
		break;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean bmp_unpack(unsigned char *dest, const unsigned char *src, PICTURE *pic, gboolean with_mask)
{
	short i, j, k, l;
	const unsigned char *rp;
	unsigned char color;
	unsigned char *gp;
	unsigned short mask;
	unsigned long bmp_bytes;
	unsigned long dst_rowsize;
	const unsigned char *maskp;
	unsigned long mask_bytes;
	unsigned char bg;
	
	memset(dest, 0, pic->pi_picsize);
	bmp_bytes = bmp_rowsize(pic, pic->pi_planes);
	dst_rowsize = pic_rowsize(pic, pic->pi_planes);
	if (!pic->pi_topdown)
		dest += pic->pi_picsize;
	maskp = src + bmp_bytes * pic->pi_height;
	mask_bytes = bmp_rowsize(pic, 1);
	
	if (pic->pi_compressed && pic->pi_planes == 4)
	{
		RLE4 rle;
		unsigned char nibbles[16];

		rle4_mem_init(&rle, src, pic->pi_datasize);
		for (i = pic->pi_height; --i >= 0; )
		{
			if (!pic->pi_topdown)
				dest -= dst_rowsize;
			for (k = pic->pi_width; k > 0; )
			{
				memset(nibbles, 0, sizeof(nibbles));
				if (--k >= 0) rle4_decode(&rle, &nibbles[0]);
				if (--k >= 0) rle4_decode(&rle, &nibbles[4]);
				if (--k >= 0) rle4_decode(&rle, &nibbles[1]);
				if (--k >= 0) rle4_decode(&rle, &nibbles[5]);
				if (--k >= 0) rle4_decode(&rle, &nibbles[2]);
				if (--k >= 0) rle4_decode(&rle, &nibbles[6]);
				if (--k >= 0) rle4_decode(&rle, &nibbles[3]);
				if (--k >= 0) rle4_decode(&rle, &nibbles[7]);

				gp = dest;						/* da sollen die Daten hin */
				rp = nibbles;
				for (j = 0, mask = 0x80; j < 4; j++, mask >>= 2)
				{
					color = bmp_coltab4[rp[j]];
					if (color & 0x01) gp[0] |= mask;
					if (color & 0x02) gp[2] |= mask;
					if (color & 0x04) gp[4] |= mask;
					if (color & 0x08) gp[6] |= mask;
				}
				rp = nibbles + 4;
				for (j = 0, mask = 0x40; j < 4; j++, mask >>= 2)
				{
					color = bmp_coltab4[rp[j]];
					if (color & 0x01) gp[0] |= mask;
					if (color & 0x02) gp[2] |= mask;
					if (color & 0x04) gp[4] |= mask;
					if (color & 0x08) gp[6] |= mask;
				}
				if (k > 0)
				{
					if (--k >= 0) rle4_decode(&rle, &nibbles[8]);
					if (--k >= 0) rle4_decode(&rle, &nibbles[12]);
					if (--k >= 0) rle4_decode(&rle, &nibbles[9]);
					if (--k >= 0) rle4_decode(&rle, &nibbles[13]);
					if (--k >= 0) rle4_decode(&rle, &nibbles[10]);
					if (--k >= 0) rle4_decode(&rle, &nibbles[14]);
					if (--k >= 0) rle4_decode(&rle, &nibbles[11]);
					if (--k >= 0) rle4_decode(&rle, &nibbles[15]);
					rp = nibbles + 8;
					for (j = 0, mask = 0x80; j < 4; j++, mask >>= 2)
					{
						color = bmp_coltab4[rp[j]];
						if (color & 0x01) gp[1] |= mask;
						if (color & 0x02) gp[3] |= mask;
						if (color & 0x04) gp[5] |= mask;
						if (color & 0x08) gp[7] |= mask;
					}
					rp = nibbles + 12;
					for (j = 0, mask = 0x40; j < 4; j++, mask >>= 2)
					{
						color = bmp_coltab4[rp[j]];
						if (color & 0x01) gp[1] |= mask;
						if (color & 0x02) gp[3] |= mask;
						if (color & 0x04) gp[5] |= mask;
						if (color & 0x08) gp[7] |= mask;
					}
				}

				dest += 8;
			}
			if (pic->pi_topdown)
				dest += dst_rowsize;
		}
#if 0
		/* read end mark */
		rle4_decode(&rle, &nibbles[0]);
#endif
	} else if (pic->pi_compressed && pic->pi_planes == 8)
	{
		err_bmp_rle(pic->pi_name);
	} else if (pic->pi_compressed == BMP_RGB)
	{
		/* uncompressed data */
		switch (pic->pi_planes)
		{
		case 8:
			if (pic->pi_transparent >= 0)
				pic->pi_transparent = bmp_coltab8[pic->pi_transparent];
			for (i = pic->pi_height; --i >= 0; )
			{
				rp = src;
				if (!pic->pi_topdown)
					dest -= dst_rowsize;
				gp = dest;
				for (k = pic->pi_width; k > 0; k -= 16)
				{
					for (j = 0, mask = 0x80; j < 8 && j < k; j++, mask >>= 1)
					{
						color = bmp_coltab8[rp[j]];
						if (with_mask)
						{
							int x = pic->pi_width - k + j;
							if ((maskp[x >> 3] & (0x80 >> (x & 7))))
								color = pic->pi_transparent >= 0 ? pic->pi_transparent : 0;
						}
						if (color & 0x01) gp[0] |= mask;
						if (color & 0x02) gp[2] |= mask;
						if (color & 0x04) gp[4] |= mask;
						if (color & 0x08) gp[6] |= mask;
						if (color & 0x10) gp[8] |= mask;
						if (color & 0x20) gp[10] |= mask;
						if (color & 0x40) gp[12] |= mask;
						if (color & 0x80) gp[14] |= mask;
					}
					if (k > 8)
					{
						for (mask = 0x80; j < 16 && j < k; j++, mask >>= 1)
						{
							color = bmp_coltab8[rp[j]];
							if (with_mask)
							{
								int x = pic->pi_width - k + j;
								if ((maskp[x >> 3] & (0x80 >> (x & 7))))
									color = pic->pi_transparent >= 0 ? pic->pi_transparent : 0;
							}
							if (color & 0x01) gp[1] |= mask;
							if (color & 0x02) gp[3] |= mask;
							if (color & 0x04) gp[5] |= mask;
							if (color & 0x08) gp[7] |= mask;
							if (color & 0x10) gp[9] |= mask;
							if (color & 0x20) gp[11] |= mask;
							if (color & 0x40) gp[13] |= mask;
							if (color & 0x80) gp[15] |= mask;
						}
					}

					gp += 16;
					rp += 16;
				}
				src += bmp_bytes;
				maskp += mask_bytes;
				if (pic->pi_topdown)
					dest += dst_rowsize;
			}
			{
				PALETTE pal;
				int i;
				
				for (i = 0; i < 256; i++)
				{
					pal[i] = pic->pi_palette[bmp_revtab8[i]];
				}
				for (i = 0; i < 256; i++)
					pic->pi_palette[i] = pal[i];
			}
			break;
		
		case 4:
			if (pic->pi_transparent >= 0)
				pic->pi_transparent = bmp_coltab4[pic->pi_transparent];
			bg = pic->pi_transparent > 0 ? pic->pi_transparent : 15;
			for (i = pic->pi_height; --i >= 0; )
			{
				rp = src;
				if (!pic->pi_topdown)
					dest -= dst_rowsize;
				gp = dest;
				for (k = pic->pi_width; k > 0; k -= 16)
				{
					short lim = (k + 1) / 2;
					
					for (j = 0, mask = 0x80; j < 4 && j < lim; j++, mask >>= 2)
					{
						color = bmp_coltab4[(rp[j] >> 4) & 0x0f];
#if 1
						if (with_mask)
						{
							int x = pic->pi_width - k + j * 2;
							if ((maskp[x >> 3] & (0x80 >> (x & 7))))
								color = bg;
						}
#endif
						if (color & 0x01) gp[0] |= mask;
						if (color & 0x02) gp[2] |= mask;
						if (color & 0x04) gp[4] |= mask;
						if (color & 0x08) gp[6] |= mask;
					}
					if (k > 8)
					{
						for (mask = 0x80; j < 8 && j < lim; j++, mask >>= 2)
						{
							color = bmp_coltab4[(rp[j] >> 4) & 0x0f];
#if 1
							if (with_mask)
							{
								int x = pic->pi_width - k + j * 2;
								if ((maskp[x >> 3] & (0x80 >> (x & 7))))
									color = bg;
							}
#endif
							if (color & 0x01) gp[1] |= mask;
							if (color & 0x02) gp[3] |= mask;
							if (color & 0x04) gp[5] |= mask;
							if (color & 0x08) gp[7] |= mask;
						}
					}

					for (j = 0, mask = 0x40; j < 4 && j < lim; j++, mask >>= 2)
					{
						color = bmp_coltab4[rp[j] & 0x0f];
#if 1
						if (with_mask)
						{
							int x = pic->pi_width - k + j * 2 + 1;
							if ((maskp[x >> 3] & (0x80 >> (x & 7))))
								color = bg;
						}
#endif
						if (color & 0x01) gp[0] |= mask;
						if (color & 0x02) gp[2] |= mask;
						if (color & 0x04) gp[4] |= mask;
						if (color & 0x08) gp[6] |= mask;
					}
					if (k > 8)
					{
						for (mask = 0x40; j < 8 && j < lim; j++, mask >>= 2)
						{
							color = bmp_coltab4[rp[j] & 0x0f];
#if 1
							if (with_mask)
							{
								int x = pic->pi_width - k + j * 2 + 1;
								if ((maskp[x >> 3] & (0x80 >> (x & 7))))
									color = bg;
							}
#endif
							if (color & 0x01) gp[1] |= mask;
							if (color & 0x02) gp[3] |= mask;
							if (color & 0x04) gp[5] |= mask;
							if (color & 0x08) gp[7] |= mask;
						}
					}
					gp += 8;
					rp += 8;
				}
				src += bmp_bytes;
				maskp += mask_bytes;
				if (pic->pi_topdown)
					dest += dst_rowsize;
			}
			{
				PALETTE pal;
				int i;
				
				for (i = 0; i < 16; i++)
				{
					pal[i] = pic->pi_palette[bmp_revtab4[i]];
				}
				for (i = 0; i < 16; i++)
				{
					pic->pi_palette[i] = pal[i];
				}
			}
			(void) bg;
			break;
		
		case 1:
			k = (short)((pic->pi_width + 7) >> 3);
			if (pic->pi_palette[1].r == 0 &&
				pic->pi_palette[1].g == 0 &&
				pic->pi_palette[1].b == 0)
			{
				for (i = pic->pi_height; --i >= 0; )
				{
					if (!pic->pi_topdown)
						dest -= dst_rowsize;
					for (l = 0; l < k; l++)
					{
						dest[l] = src[l];
						if (with_mask)
							dest[l] &= ~maskp[l];
					}	
					for (; l < (short)dst_rowsize; l++)
						dest[l] = 0;
					src += bmp_bytes;
					maskp += mask_bytes;
					if (pic->pi_topdown)
						dest += dst_rowsize;
				}
			} else
			{
				for (i = pic->pi_height; --i >= 0; )
				{
					if (!pic->pi_topdown)
						dest -= dst_rowsize;
					for (l = 0; l < k; l++)
					{
						dest[l] = ~src[l];
						if (with_mask)
							dest[l] &= ~maskp[l];
					}
					for (; l < (short)dst_rowsize; l++)
						dest[l] = 0xff;
					src += bmp_bytes;
					maskp += mask_bytes;
					if (pic->pi_topdown)
						dest += dst_rowsize;
				}
			}
			break;

		case 24:
		case 32:
			for (i = pic->pi_height; --i >= 0; )
			{
				if (!pic->pi_topdown)
					dest -= dst_rowsize;
				memcpy(dest, src, bmp_bytes);
				src += bmp_bytes;
				maskp += mask_bytes;
				if (pic->pi_topdown)
					dest += dst_rowsize;
			}
			break;
			
		default:
			return FALSE;
		}
	} else
	{
		return FALSE;
	}
	
	if (with_mask && !(pic->pi_compressed && pic->pi_planes == 4))
	{
		unsigned long mask_rowsize = pic_rowsize(pic, 1);
		
		if (!pic->pi_topdown)
			dest += pic->pi_picsize + mask_rowsize * pic->pi_height;
		maskp = src;

		k = (short)((pic->pi_width + 7) >> 3);
		for (i = pic->pi_height; --i >= 0; )
		{
			if (!pic->pi_topdown)
				dest -= mask_rowsize;
			for (l = 0; l < k; l++)
				dest[l] = ~src[l];
			for (; l < (short)mask_rowsize; l++)
				dest[l] = 0xff;
			src += mask_bytes;
			if (pic->pi_topdown)
				dest += mask_rowsize;
		}
	}
	
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

long bmp_pack_planes(unsigned char *dest, const unsigned char *src, PICTURE *pic, gboolean update_header, const unsigned char *maptab)
{
	short i, j, k;
	unsigned char *rp;
	unsigned char color;
	const unsigned char *gp;
	unsigned short mask;
	long datasize = 0;
	unsigned char *buf = dest;
	unsigned long dstrowsize;
	
	dest += pic->pi_dataoffset;
	
	dstrowsize = bmp_rowsize(pic, pic->pi_planes);
	if (pic->pi_compressed != BMP_RGB)
	{
	} else
	{
		/* uncompressed data */
		
		/*
		 * BMP is stored upside down,
		 * start from the end
		 */
		src += pic->pi_picsize;
		switch (pic->pi_planes)
		{
		case 8:
			for (i = pic->pi_height; --i >= 0; )
			{
				src -= pic_rowsize(pic, pic->pi_planes);
				rp = dest;
				gp = src;
				memset(rp, 0, dstrowsize);
				for (k = pic->pi_width; k > 0; k -= 16)
				{
					for (j = 0, mask = 0x80; j < 8 && j < k; j++, mask >>= 1)
					{
						color = 0;
						if (gp[0] & mask) color |= 0x01;
						if (gp[2] & mask) color |= 0x02;
						if (gp[4] & mask) color |= 0x04;
						if (gp[6] & mask) color |= 0x08;
						if (gp[8] & mask) color |= 0x10;
						if (gp[10] & mask) color |= 0x20;
						if (gp[12] & mask) color |= 0x40;
						if (gp[14] & mask) color |= 0x80;
						rp[j] = maptab[color];
					}
					if (k > 8)
					{
						for (mask = 0x80; j < 16 && j < k; j++, mask >>= 1)
						{
							color = 0;
							if (gp[1] & mask) color |= 0x01;
							if (gp[3] & mask) color |= 0x02;
							if (gp[5] & mask) color |= 0x04;
							if (gp[7] & mask) color |= 0x08;
							if (gp[9] & mask) color |= 0x10;
							if (gp[11] & mask) color |= 0x20;
							if (gp[13] & mask) color |= 0x40;
							if (gp[15] & mask) color |= 0x80;
							rp[j] = maptab[color];
						}
					}

					gp += 16;
					rp += 16;
				}
				dest += dstrowsize;
			}
			break;
		case 4:
			for (i = pic->pi_height; --i >= 0; )
			{
				src -= pic_rowsize(pic, pic->pi_planes);
				rp = dest;
				gp = src;
				memset(rp, 0, dstrowsize);
				for (k = pic->pi_width; k > 0; k -= 16)
				{
					short lim = (k + 1) / 2;
					
					for (j = 0, mask = 0x80; j < 4 && j < lim; j++, mask >>= 2)
					{
						color = 0;
						if (gp[0] & mask) color |= 0x01;
						if (gp[2] & mask) color |= 0x02;
						if (gp[4] & mask) color |= 0x04;
						if (gp[6] & mask) color |= 0x08;
						rp[j] = maptab[color] << 4;
					}
					if (k > 8)
					{
						for (mask = 0x80; j < 8 && j < lim; j++, mask >>= 2)
						{
							color = 0;
							if (gp[1] & mask) color |= 0x01;
							if (gp[3] & mask) color |= 0x02;
							if (gp[5] & mask) color |= 0x04;
							if (gp[7] & mask) color |= 0x08;
							rp[j] = maptab[color] << 4;
						}
					}

					for (j = 0, mask = 0x40; j < 4 && j < lim; j++, mask >>= 2)
					{
						color = 0;
						if (gp[0] & mask) color |= 0x01;
						if (gp[2] & mask) color |= 0x02;
						if (gp[4] & mask) color |= 0x04;
						if (gp[6] & mask) color |= 0x08;
						rp[j] |= maptab[color];
					}
					if (k > 8)
					{
						for (mask = 0x40; j < 8 && j < lim; j++, mask >>= 2)
						{
							color = 0;
							if (gp[1] & mask) color |= 0x01;
							if (gp[3] & mask) color |= 0x02;
							if (gp[5] & mask) color |= 0x04;
							if (gp[7] & mask) color |= 0x08;
							rp[j] |= maptab[color];
						}
					}
					gp += 8;
					rp += 8;
				}
				dest += dstrowsize;
			}
			break;
		case 1:
			{
				short l;

				j = (((pic->pi_width) + 7) >> 3);
				k = (short) pic_rowsize(pic, pic->pi_planes);
				for (i = pic->pi_height; --i >= 0; )
				{
					src -= k;
					rp = dest;
					memset(rp, 0, dstrowsize);
					for (l = 0; l < j; l++)
						*rp++ = ~src[l];
					dest += dstrowsize;
				}
			}
			break;
		default:
			return FALSE;
		}
		datasize = (long)pic->pi_height * dstrowsize;
	}

	if (update_header)
	{
		/*
		 * update filesize & datasize in header
		 */
		buf += 2;
		put_long(datasize + pic->pi_dataoffset);
		buf += 28;
		put_long(datasize);
	}
	
	return datasize;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * specialized version for writing icon data:
 * we need to use a pixel value of zero for transparent
 * pixels, no matter what color[0] actually maps to
 */
long bmp_pack_data_and_mask(unsigned char *dest, const unsigned char *src, const unsigned char *masksrc, PICTURE *pic, gboolean update_header, const unsigned char *maptab)
{
	short i, j, k;
	unsigned char *rp;
	unsigned char color;
	const unsigned char *gp;
	const unsigned char *mp;
	unsigned short mask;
	long datasize = 0;
	long masksize = 0;
	unsigned char *buf = dest;
	unsigned long dstrowsize;
	unsigned long maskrowsize;
	
	maskrowsize = bmp_rowsize(pic, 1);
	masksize = maskrowsize * pic->pi_height;
	dstrowsize = bmp_rowsize(pic, pic->pi_planes);
	if (masksrc == NULL)
	{
		datasize = bmp_pack_planes(dest, src, pic, update_header, maptab);
		memset(dest + pic->pi_dataoffset + datasize, 0, masksize);
	} else
	{
		datasize = (long)pic->pi_height * dstrowsize;
		bmp_pack_mask(dest + datasize, masksrc, pic);
		dest += pic->pi_dataoffset;
		masksrc = dest + datasize;
		
		if (pic->pi_compressed != BMP_RGB)
		{
		} else
		{
			/* uncompressed data */
			
			/*
			 * BMP is stored upside down,
			 * start from the end
			 */
			src += pic->pi_picsize;
			switch (pic->pi_planes)
			{
			case 8:
				for (i = pic->pi_height; --i >= 0; )
				{
					src -= pic_rowsize(pic, pic->pi_planes);
					rp = dest;
					gp = src;
					mp = masksrc;
					memset(rp, 0, dstrowsize);
					for (k = pic->pi_width; k > 0; k -= 16)
					{
						for (j = 0, mask = 0x80; j < 8 && j < k; j++, mask >>= 1)
						{
							if (!(mp[0] & mask))
							{
								color = 0;
								if (gp[0] & mask) color |= 0x01;
								if (gp[2] & mask) color |= 0x02;
								if (gp[4] & mask) color |= 0x04;
								if (gp[6] & mask) color |= 0x08;
								if (gp[8] & mask) color |= 0x10;
								if (gp[10] & mask) color |= 0x20;
								if (gp[12] & mask) color |= 0x40;
								if (gp[14] & mask) color |= 0x80;
								rp[j] = maptab[color];
							}
						}
						if (k > 8)
						{
							for (mask = 0x80; j < 16 && j < k; j++, mask >>= 1)
							{
								if (!(mp[1] & mask))
								{
									color = 0;
									if (gp[1] & mask) color |= 0x01;
									if (gp[3] & mask) color |= 0x02;
									if (gp[5] & mask) color |= 0x04;
									if (gp[7] & mask) color |= 0x08;
									if (gp[9] & mask) color |= 0x10;
									if (gp[11] & mask) color |= 0x20;
									if (gp[13] & mask) color |= 0x40;
									if (gp[15] & mask) color |= 0x80;
									rp[j] = maptab[color];
								}
							}
						}

						gp += 16;
						rp += 16;
						mp += 2;
					}
					dest += dstrowsize;
					masksrc += maskrowsize;
				}
				break;
			case 4:
				for (i = pic->pi_height; --i >= 0; )
				{
					src -= pic_rowsize(pic, pic->pi_planes);
					rp = dest;
					gp = src;
					mp = masksrc;
					memset(rp, 0, dstrowsize);
					for (k = pic->pi_width; k > 0; k -= 16)
					{
						short lim = (k + 1) / 2;
						
						for (j = 0, mask = 0x80; j < 4 && j < lim; j++, mask >>= 2)
						{
							if (!(mp[0] & mask))
							{
								color = 0;
								if (gp[0] & mask) color |= 0x01;
								if (gp[2] & mask) color |= 0x02;
								if (gp[4] & mask) color |= 0x04;
								if (gp[6] & mask) color |= 0x08;
								rp[j] = maptab[color] << 4;
							}
						}
						if (k > 8)
						{
							for (mask = 0x80; j < 8 && j < lim; j++, mask >>= 2)
							{
								if (!(mp[1] & mask))
								{
									color = 0;
									if (gp[1] & mask) color |= 0x01;
									if (gp[3] & mask) color |= 0x02;
									if (gp[5] & mask) color |= 0x04;
									if (gp[7] & mask) color |= 0x08;
									rp[j] = maptab[color] << 4;
								}
							}
						}

						for (j = 0, mask = 0x40; j < 4 && j < lim; j++, mask >>= 2)
						{
							if (!(mp[0] & mask))
							{
								color = 0;
								if (gp[0] & mask) color |= 0x01;
								if (gp[2] & mask) color |= 0x02;
								if (gp[4] & mask) color |= 0x04;
								if (gp[6] & mask) color |= 0x08;
								rp[j] |= maptab[color];
							}
						}
						if (k > 8)
						{
							for (mask = 0x40; j < 8 && j < lim; j++, mask >>= 2)
							{
								if (!(mp[1] & mask))
								{
									color = 0;
									if (gp[1] & mask) color |= 0x01;
									if (gp[3] & mask) color |= 0x02;
									if (gp[5] & mask) color |= 0x04;
									if (gp[7] & mask) color |= 0x08;
									rp[j] |= maptab[color];
								}
							}
						}
						gp += 8;
						rp += 8;
						mp += 2;
					}
					dest += dstrowsize;
					masksrc += maskrowsize;
				}
				break;
			case 1:
				{
					short l;

					j = (((pic->pi_width) + 7) >> 3);
					k = (short) pic_rowsize(pic, pic->pi_planes);
					for (i = pic->pi_height; --i >= 0; )
					{
						src -= k;
						rp = dest;
						memset(rp, 0, dstrowsize);
						for (l = 0; l < j; l++)
						{
							*rp++ = ~src[l];
						}
						dest += dstrowsize;
					}
				}
				break;
			default:
				return FALSE;
			}
		}
	}
	
	datasize += masksize;
	if (update_header)
	{
		/*
		 * update filesize & datasize in header
		 */
		buf += 2;
		put_long(datasize + pic->pi_dataoffset);
		buf += 28;
		put_long(datasize);
	}
	
	return datasize;
}

/*** ---------------------------------------------------------------------- ***/

long bmp_pack_mask(unsigned char *dest, const unsigned char *src, PICTURE *pic)
{
	short i, j;
	long datasize = 0;
	unsigned long dstrowsize;
	unsigned long k;
	unsigned char *rp;
	
	dest += pic->pi_dataoffset;
	
	dstrowsize = bmp_rowsize(pic, 1);
	k = pic_rowsize(pic, 1);
	if (pic->pi_compressed != BMP_RGB)
	{
	} else
	{
		/* uncompressed data */
		
		/*
		 * BMP is stored upside down,
		 * start from the end
		 */
		src += k * pic->pi_height;
		{
			short l;

			j = (((pic->pi_width) + 7) >> 3);
			for (i = pic->pi_height; --i >= 0; )
			{
				src -= k;
				rp = dest;
				memset(rp, 0, dstrowsize);
				for (l = 0; l < j; l++)
					*rp++ = ~src[l];
				dest += dstrowsize;
			}
		}
		datasize = (long)pic->pi_height * dstrowsize;
	}

	return datasize;
}

/*** ---------------------------------------------------------------------- ***/

long bmp_pack(unsigned char *dest, const unsigned char *src, PICTURE *pic, gboolean update_header, const unsigned char *maptab)
{
	_WORD planes = pic->pi_planes;
	long datasize;
	
	if (pic->pi_compressed)
	{
		switch (planes)
		{
			case 1: pic->pi_compressed = BMP_RGB; break;
			case 4: pic->pi_compressed = BMP_RLE4; break;
			case 8: pic->pi_compressed = BMP_RLE8; break;
		}
		pic->pi_compressed = BMP_RGB; /* compresssion NYI */
	} else
	{
		pic->pi_compressed = BMP_RGB;
	}
	datasize = bmp_pack_planes(dest, src, pic, update_header, maptab);
	pic->pi_datasize = datasize;
	return datasize;
}

/*** ---------------------------------------------------------------------- ***/

unsigned char *bmp_put_palette(unsigned char *buf, PICTURE *pic, const unsigned char *maptab)
{
	short ncolors, i;

	ncolors = pic->pi_planes <= 8 ? (1 << pic->pi_planes) : 0;

	switch (pic->pi_planes)
	{
	case 8:
		for (i = 0; i < ncolors; i++)
		{
			put_byte(pic->pi_palette[maptab[i]].b);
			put_byte(pic->pi_palette[maptab[i]].g);
			put_byte(pic->pi_palette[maptab[i]].r);
			put_byte(0);
		}
		break;
	case 4:
		for (i = 0; i < ncolors; i++)
		{
			put_byte(pic->pi_palette[maptab[i]].b);
			put_byte(pic->pi_palette[maptab[i]].g);
			put_byte(pic->pi_palette[maptab[i]].r);
			put_byte(0);
		}
		break;
	case 1:
		/*
		 * Pixel values for BMP monochrome bitmaps are
		 * usually inverted from what we use internally
		 * (and bmp_pack already did that) so dont
		 * use our standard palette
		 */
		for (i = 0; i < ncolors; i++)
		{
			put_byte(win2_palette[i].b);
			put_byte(win2_palette[i].g);
			put_byte(win2_palette[i].r);
			put_byte(0);
		}
		break;
	}
	
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

long bmp_header(unsigned char **dest_p, PICTURE *pic, const unsigned char *maptab)
{
	unsigned long len, headlen;
	short ncolors;
	unsigned long cmapsize;
	unsigned char *buf;
	long bmp_bytes;
	
	ncolors = pic->pi_planes <= 8 ? (1 << pic->pi_planes) : 0;
	cmapsize = 4 * ncolors;

	pic->pi_compressed = 0; /* compression NYI */
	headlen = 14 + 40 + cmapsize;
	bmp_bytes = bmp_rowsize(pic, pic->pi_planes);
	if (bmp_bytes == 0)
		return 0;
	pic->pi_datasize = bmp_bytes * pic->pi_height;
	len = headlen + pic->pi_datasize;

	if (*dest_p == NULL)
		buf = g_new(unsigned char, len);
	else
		buf = *dest_p;
	if (buf == NULL)
		return 0;
	*dest_p = buf;
	
	put_byte('B');
	put_byte('M');

	pic->pi_dataoffset = headlen;
	pic->pi_topdown = FALSE;
	put_long(len);
	put_long(0l);
	put_long(headlen);

	put_long(40l); /* biSize */
	put_long((long)(pic->pi_width)); /* biWidth */
	put_long((long)(pic->pi_height)); /* biHeight */
	put_word(1);					/* biPlanes */
	put_word(pic->pi_planes);		/* biBitsPerPixel */
	put_long((long)(pic->pi_compressed)); /* biCompression */
	put_long((long)(pic->pi_datasize)); /* biSizeImage */
	put_long(0l);					/* biXPelsPerMeter */
	put_long(0l);					/* biYPelsPerMeter */
	put_long((long)(ncolors)); /* biClrUsed */
	put_long(0l); /* biClrImportant */
	
	bmp_put_palette(buf, pic, maptab);
	
	return headlen;
}
