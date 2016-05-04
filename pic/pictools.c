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

#if 0 /* experimental */
static double H(double q)
{
	double value;

	if (q > 0.008856)
	{
		value = pow(q, 0.333333);
		return value;
	} else
	{
		value = 7.787 * q + 0.137931;
		return value;
	}
}

static double RGB2LAB(int R_value, int G_value, int B_value, int R_value2, int G_value2, int B_value2)
{
	double RGB[3];
	double XYZ[3];
	double Lab[3];
	double RGB2[3];
	double XYZ2[3];
	double Lab2[3];
	double adapt[3];
	double value;

	/* maybe change to global, XYZ[0] = X_value */

	adapt[0] = 0.950467;
	adapt[1] = 1.000000;
	adapt[2] = 1.088969;

	RGB[0] = R_value / 255.0;
	RGB[1] = G_value / 255.0;
	RGB[2] = B_value / 255.0;

	XYZ[0] = 0.412424 * RGB[0] + 0.357579 * RGB[1] + 0.180464 * RGB[2];
	XYZ[1] = 0.212656 * RGB[0] + 0.715158 * RGB[1] + 0.0721856 * RGB[2];
	XYZ[2] = 0.0193324 * RGB[0] + 0.119193 * RGB[1] + 0.950444 * RGB[2];

	Lab[0] = 116 * H(XYZ[1] / adapt[1]) - 16;
	Lab[1] = 500 * (H(XYZ[0] / adapt[0]) - H(XYZ[1] / adapt[1]));
	Lab[2] = 200 * (H(XYZ[1] / adapt[1]) - H(XYZ[2] / adapt[2]));

	RGB2[0] = R_value2 / 255.0;
	RGB2[1] = G_value2 / 255.0;
	RGB2[2] = B_value2 / 255.0;

	XYZ2[0] = 0.412424 * RGB2[0] + 0.357579 * RGB2[1] + 0.180464 * RGB2[2];
	XYZ2[1] = 0.212656 * RGB2[0] + 0.715158 * RGB2[1] + 0.0721856 * RGB2[2];
	XYZ2[2] = 0.0193324 * RGB2[0] + 0.119193 * RGB2[1] + 0.950444 * RGB2[2];

	Lab2[0] = 116 * H(XYZ2[1] / adapt[1]) - 16;
	Lab2[1] = 500 * (H(XYZ2[0] / adapt[0]) - H(XYZ2[1] / adapt[1]));
	Lab2[2] = 200 * (H(XYZ2[1] / adapt[1]) - H(XYZ2[2] / adapt[2]));

	value = pow((Lab[0] - Lab2[0]), 2) + pow((Lab[1] - Lab2[1]), 2) + pow((Lab[2] - Lab2[2]), 2);

/*
	if ( Lab2[0] > 903.3*0.008856 ) 
		trans[1] = pow ( (Lab2[0]+16)*0.00862, 3);
	else
		trans[1] = Lab2[0] * 0.001107;

	if ( trans[1] > 0.008856 )
		transf[1] = (Lab2[0]+16)*0.00862;
	else
		transf[1] = (903.3*trans[1]+16)*0.00862;

	transf[0] = Lab2[1] * 0.002 + transf[1];
	transf[2] = transf[1] - Lab2[2] * 0.005;

	if ( pow( transf[0], 3 ) > 0.008856 )
		trans[0] = pow( transf[0], 3 );
	else
		trans[0] =  ((116 * transf[0]) - 16) * 0.001107;

	if ( pow( transf[2], 3 ) > 0.008856 )
		trans[2] = pow( transf[2], 3 );
	else
		trans[2] =  ((116 * transf[2]) - 16) * 0.001107;

	newXYZ[0] = trans[0] * adapt[0];
	newXYZ[1] = trans[1] * adapt[1];
	newXYZ[2] = trans[2] * adapt[2];

	newRGB[0] = 3.24071 * newXYZ[0] + (-1.53726) * newXYZ[1] + (-0.498571) * newXYZ[2];
	newRGB[1] = (-0.969258) * newXYZ[0] + 1.87599 * newXYZ[1] + 0.0415557 * newXYZ[2];
	newRGB[2] = 0.0556352 * newXYZ[0] + (-0.203996) * newXYZ[1] + 1.05707 * newXYZ[2];
	
	newRGB[0] *= 255;
	newRGB[1] *= 255;
	newRGB[2] *= 255;

	printf("r=%d g=%d b=%d nr=%.f ng=%.f nb=%.f\n",R_value,G_value,B_value,newRGB[0],newRGB[1],newRGB[2]);
*/
	return value;
}

static void rgb2lab(unsigned char r, unsigned char g, unsigned char b, double lab[3])
{
	double eps = 216. / 24389.;
	double k = 24389. / 27.;
	/* reference white D50 */
	double xr = 0.964221;
	double yr = 1.0;
	double zr = 0.825211;
	/* reference white D65 */
	/* xr = 0.95047; yr = 1.0; zr = 1.08883; */
	double rgb[3];
	double x, y, z, fx, fy, fz;

	/* RGB to XYZ */
	rgb[0] = r / 255.0;
	rgb[1] = g / 255.0;
	rgb[2] = b / 255.0;

	/* assuming sRGB (D65) */
	rgb[0] = (rgb[0] <= 0.04045) ? (rgb[0] / 12.92) : pow((rgb[0] + 0.055) / 1.055, 2.4);
	rgb[1] = (rgb[1] <= 0.04045) ? (rgb[1] / 12.92) : pow((rgb[1] + 0.055) / 1.055, 2.4);
	rgb[2] = (rgb[2] <= 0.04045) ? (rgb[2] / 12.92) : pow((rgb[2] + 0.055) / 1.055, 2.4);

	/* sRGB D50 */
	x = 0.4360747 * rgb[0] + 0.3850649 * rgb[1] + 0.1430804 * rgb[2];
	y = 0.2225045 * rgb[0] + 0.7168786 * rgb[1] + 0.0606169 * rgb[2];
	z = 0.0139322 * rgb[0] + 0.0971045 * rgb[1] + 0.7141733 * rgb[2];
	/* sRGB D65 */
	/* x =  0.412453*rgb[0] + 0.357580*rgb[1] + 0.180423*rgb[2];
	   y =  0.212671*rgb[0] + 0.715160*rgb[1] + 0.072169*rgb[2];
	   z =  0.019334*rgb[0] + 0.119193*rgb[1] + 0.950227*rgb[2];
	 */

	/* XYZ to Lab */
	xr = x / xr;
	yr = y / yr;
	zr = z / zr;

	fx = (xr > eps) ? pow(xr, 1 / 3) : ((k * xr + 16) / 116);
	fy = (yr > eps) ? pow(yr, 1 / 3) : ((k * yr + 16) / 116);
	fz = (zr > eps) ? pow(zr, 1 / 3) : ((k * zr + 16) / 116);

	lab[0] = round((116 * fy) - 16);
	lab[1] = round(500 * (fx - fy));
	lab[2] = round(200 * (fy - fz));
}
#endif

/*** ---------------------------------------------------------------------- ***/

static void matchpal(const PALETTE dst, const PALETTE src, unsigned char *pixel, int ncolors)
{
	int i, j;
	_UWORD dr, dg, db;
	_ULONG diff, mindiff;
	const struct _rgb *rgbi, *rgbp;
	_UBYTE best;
	
	for (i = 0; i < ncolors; i++)
	{
		rgbp = &src[i];
		mindiff = 0x4000000UL;
		best = i;
		for (j = 0; j < ncolors && mindiff != 0; j++)
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
#if 0 /* test output */
		if (i < 16)
		{
			rgbi = &dst[best];
			printf("map %d #%02x%02x%02x -> %d #%02x%02x%02x %lu\n",
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
