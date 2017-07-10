/*****************************************************************************
 * PICTOOLS.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>
#include <math.h>

/*** ---------------------------------------------------------------------- ***/

unsigned long pic_pal_stddiff(const PICTURE *pic)
{
	_WORD color, ncolors;
	unsigned long sum, diff;
	const struct _rgb *rgbi, *rgbp;
	unsigned int dr, dg, db;
	
	if (pic->pi_planes != 4 && pic->pi_planes != 8)
		return 0;
	ncolors = 1 << pic->pi_planes;
	sum = 0;
	for (color = 0; color < ncolors; color++)
	{
		rgbp = &pic->pi_palette[color];
		rgbi = &std256_palette[color];
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

/*** ---------------------------------------------------------------------- ***/

typedef struct {
	double r;
	double g;
	double b;
} color_RGB;

typedef struct {
	double X;
	double Y;
	double Z;
} color_XYZ;

typedef struct {
	double L;
	double a;
	double b;
} color_Lab;


static void to_RGB(color_RGB *c, unsigned char r, unsigned char g, unsigned char b)
{
	c->r = r / 255.0;
	c->g = g / 255.0;
	c->b = b / 255.0;
}

/*** ---------------------------------------------------------------------- ***/

static void rgb2xyz(color_XYZ *xyz, const color_RGB *rgb)
{
	double r, g, b;
	
	if (rgb->r > 0.04045)
		r = pow((rgb->r + 0.055) / 1.055, 2.4);
	else
		r = rgb->r / 12.92;
	if (rgb->g > 0.04045)
		g = pow((rgb->g + 0.055) / 1.055, 2.4);
	else
		g = rgb->g / 12.92;
	if (rgb->b > 0.04045)
		b = pow((rgb->b + 0.055) / 1.055, 2.4);
	else
		b = rgb->b / 12.92;

	r = r * 100;
	g = g * 100;
	b = b * 100;

	/* sRGB D50
	x = 0.4360747 * rgb[0] + 0.3850649 * rgb[1] + 0.1430804 * rgb[2];
	y = 0.2225045 * rgb[0] + 0.7168786 * rgb[1] + 0.0606169 * rgb[2];
	z = 0.0139322 * rgb[0] + 0.0971045 * rgb[1] + 0.7141733 * rgb[2];
	*/
	
	/* sRGB D65 */
	xyz->X = r * 0.412453 + g * 0.357580 + b * 0.180423;
	xyz->Y = r * 0.212671 + g * 0.715160 + b * 0.072169;
	xyz->Z = r * 0.019334 + g * 0.119193 + b * 0.950227;
}

/*** ---------------------------------------------------------------------- ***/

static void xyz2lab(color_Lab *c, const color_XYZ *xyz)
{
	double X = xyz->X / 95.047;
	double Y = xyz->Y / 100.000;
	double Z = xyz->Z / 108.883;

	if (X > 0.008856)
		X = pow(X, 1.0 / 3);
	else
		X = (7.787 * X) + (16.0 / 116);
	if (Y > 0.008856)
		Y = pow(Y, 1.0 / 3);
	else
		Y = (7.787 * Y) + (16.0 / 116);
	if (Z > 0.008856)
		Z = pow(Z, 1.0 / 3);
	else
		Z = (7.787 * Z) + (16.0 / 116);

	c->L = (116 * Y) - 16;
	c->a = 500 * (X - Y);
	c->b = 200 * (Y - Z);
}

/*** ---------------------------------------------------------------------- ***/

static void rgb2lab(color_Lab *lab, const color_RGB *rgb)
{
	color_XYZ xyz;
	rgb2xyz(&xyz, rgb);
	xyz2lab(lab, &xyz);
}

/*** ---------------------------------------------------------------------- ***/

/*
  Calculates the Delta E (CIE2000) of two colors. Colors are given in Lab tuples.
*/

#define degrees(x) ((x) * (180.0 / M_PI))
#define radians(x) ((x) * (M_PI / 180.0))

static double delta_e_cie2000(const color_Lab *color1, const color_Lab *color2)
{
	const double Kl = 1.0;
	const double Kc = 1.0;
	const double Kh = 1.0;

	double L1 = color1->L;
	double a1 = color1->a;
	double b1 = color1->b;
	double L2 = color2->L;
	double a2 = color2->a;
	double b2 = color2->b;

	double avg_Lp, C1, C2, avg_C1_C2, G, a1p, a2p, C1p, C2p, avg_C1p_C2p, h1p, h2p, avg_Hp, T;
	double diff_h2p_h1p, delta_hp, delta_Lp, delta_Cp, delta_Hp;
	double S_L, S_C, S_H;
	double delta_ro, R_C, R_T;

	double delta_E;

	avg_Lp = (L1 + L2) / 2.0;
	C1 = sqrt(a1 * a1 + b1 * b1);
	C2 = sqrt(a2 * a2 + b2 * b2);
	avg_C1_C2 = (C1 + C2) / 2.0;

	G = 0.5 * (1 - sqrt(pow(avg_C1_C2, 7.0) / (pow(avg_C1_C2, 7.0) + pow(25.0, 7.0))));

	a1p = (1.0 + G) * a1;
	a2p = (1.0 + G) * a2;
	C1p = sqrt(a1p * a1p + b1 * b1);
	C2p = sqrt(a2p * a2p + b2 * b2);
	avg_C1p_C2p = (C1p + C2p) / 2.0;

	h1p = degrees(atan2(b1, a1p));
	if (h1p < 0)
		h1p += 360.0;

	h2p = degrees(atan2(b2, a2p));
	if (h2p < 0)
		h2p += 360.0;

	if (fabs(h1p - h2p) > 180)
		avg_Hp = (h1p + h2p + 360) / 2.0;
	else
		avg_Hp = (h1p + h2p) / 2.0;

	T = 1 - 0.17 * cos(radians(avg_Hp - 30)) + 0.24 * cos(radians(2 * avg_Hp)) + 0.32 * cos(radians(3 * avg_Hp + 6)) - 0.2 * cos(radians(4 * avg_Hp - 63));

	diff_h2p_h1p = h2p - h1p;
	if (fabs(diff_h2p_h1p) <= 180)
		delta_hp = diff_h2p_h1p;
	else if (fabs(diff_h2p_h1p) > 180 && h2p <= h1p)
		delta_hp = diff_h2p_h1p + 360;
	else
		delta_hp = diff_h2p_h1p - 360;

	delta_Lp = L2 - L1;
	delta_Cp = C2p - C1p;
	delta_Hp = 2 * sqrt(C2p * C1p) * sin(radians(delta_hp) / 2.0);

	S_L = 1 + ((0.015 * pow(avg_Lp - 50, 2)) / sqrt(20 + pow(avg_Lp - 50, 2.0)));
	S_C = 1 + 0.045 * avg_C1p_C2p;
	S_H = 1 + 0.015 * avg_C1p_C2p * T;

	delta_ro = 30 * exp(-(pow(((avg_Hp - 275) / 25), 2.0)));
	R_C = sqrt((pow(avg_C1p_C2p, 7.0)) / (pow(avg_C1p_C2p, 7.0) + pow(25.0, 7.0)));
	R_T = -2 * R_C * sin(2 * radians(delta_ro));

	delta_E = sqrt(pow(delta_Lp / (S_L * Kl), 2) + pow(delta_Cp / (S_C * Kc), 2) + pow(delta_Hp / (S_H * Kh), 2) + R_T * (delta_Cp / (S_C * Kc)) * (delta_Hp / (S_H * Kh)));

	return delta_E;
}

/*** ---------------------------------------------------------------------- ***/

static void matchpal(const PALETTE dst, const PALETTE src, unsigned char *pixel, int ncolors)
{
	int i, j;
	color_RGB c;
	color_Lab c1, c2;
	double diff, mindiff;
	const struct _rgb *rgbi, *rgbp;
	_UBYTE best;
	
	for (i = 0; i < ncolors; i++)
	{
		rgbp = &src[i];
		to_RGB(&c, rgbp->r, rgbp->g, rgbp->b);
		rgb2lab(&c1, &c);
		mindiff = 4000000.;
		best = i;
		for (j = 0; j < ncolors && mindiff != 0; j++)
		{
			rgbi = &dst[j];
			to_RGB(&c, rgbi->r, rgbi->g, rgbi->b);
			rgb2lab(&c2, &c);
			diff = delta_e_cie2000(&c1, &c2);
			if (diff < mindiff)
			{
				best = j;
				mindiff = diff;
			}	
		}
#if 0 /* test output */
		if (i < 16)
		{
			rgbi = &dst[best];
			printf("map %d #%02x%02x%02x -> %d #%02x%02x%02x %f\n",
				i, rgbp->r, rgbp->g, rgbp->b,
				best, rgbi->r, rgbi->g, rgbi->b, mindiff);
		}
#endif
		pixel[i] = best;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void copy_block(unsigned char *dst, const unsigned char *src, const PICTURE *pic, const unsigned char *pixels)
{
	_UWORD mask;
	_UBYTE color, colmask;
	_ULONG srclinesize, offset;
	_WORD x, y, p;
	const unsigned char *ptr;
	unsigned char *dptr;
	_WORD w = pic->pi_width;
	_WORD h = pic->pi_height;
	static unsigned char const masktab[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	
	srclinesize = (_ULONG)((w + 15) >> 4) * (_ULONG)pic->pi_planes * 2;
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

/*** ---------------------------------------------------------------------- ***/

gboolean pic_match_stdpal(PICTURE *pic, unsigned char *buf)
{
	unsigned char *tmp;
	unsigned char pixels[256];
	int ncolors;
	
	if (pic->pi_planes == 4)
		ncolors = 16;
	else if (pic->pi_planes == 8)
		ncolors = 256;
	else
		return TRUE;
	tmp = g_new0(unsigned char, pic->pi_picsize);
	if (tmp == NULL)
		return FALSE;
	matchpal(std256_palette, pic->pi_palette, pixels, ncolors);
	copy_block(tmp, buf, pic, pixels);
	memcpy(buf, tmp, pic->pi_picsize);
	g_free(tmp);
	memcpy(pic->pi_palette, std256_palette, sizeof(std256_palette));
	return TRUE;
}
