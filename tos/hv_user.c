#include "hv_defs.h"
#include "hypdebug.h"
#include "picture.h"

struct userdef {
	OBJECT orig;
	USERBLK user;
	_WORD obj;
};

struct userdef_list {
	struct userdef_list *next;
	RSHDR *hdr;
	OBJECT *objects;
	_WORD nobjs;
	_WORD nuserobjs;
	struct userdef userobjs[1];
};

#define NICELINES 1

static struct userdef_list *userdefs;

/*
 * vdi handle used for drawing USERDEF objects
 * It's safe to directly use the AES handle here,
 * because we save/restore all attributes we change
 */
_WORD aes_handle;

#define TEST_COLOR 15
#define XMAX_PLANES 32
#define XMAX_COLOR 256

typedef _WORD table4[XMAX_COLOR][4];

/*
 * vdi handle used for initializing the color icon tables
 */
#define xvdi_handle aes_handle
static _WORD xscrn_planes;

/* number of bytes per pixel (0 == planeoriented) */
static _WORD xpixelbytes = -1;
/* table for plane oriented images */
static _WORD *colortbl;
/* table for pixel oriented imaged */
static _ULONG colortbl2[XMAX_COLOR];
/* wether an palette has already been set */
LOCAL table4 rgb_palette;

static _WORD const pixtbl[XMAX_COLOR] = {
    0,   2,   3,   6,   4,   7,   5,   8,   9,  10,  11,  14,  12,  15,  13, 255,
   16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
   32 , 33 , 34 , 35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
   48 , 49 , 50 , 51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
   64 , 65 , 66 , 67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
   80 , 81 , 82 , 83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
   96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
  112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
  128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
  144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
  160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
  176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
  192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
  208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
  224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
  240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,   1
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static _BOOL xywh2pxy(_WORD x, _WORD y, _WORD w, _WORD h, _WORD *pxy, _WORD max_w, _WORD max_h)
{
	if (x < 0)
	{
		w += x;
		x = 0;
	}
	if (x >= max_w)
	{
		x = 0;
		w = 0;
	}
	if (x + w > max_w)
	{
		w = max_w - x;
	}
	if (y < 0)
	{
		h += y;
		y = 0;
	}
	if (y >= max_h)
	{
		y = 0;
		h = 0;
	}
	if (y + h > max_h)
	{
		h = max_h - y;
	}
	pxy[0] = x;
	pxy[1] = y;
	pxy[2] = x + w - 1;
	pxy[3] = y + h - 1;
	return w > 0 && h > 0;
}

/*** ---------------------------------------------------------------------- ***/

static void rect2pxy(const GRECT *gr, _WORD *pxy, _WORD max_w, _WORD max_h)
{
	xywh2pxy(gr->g_x, gr->g_y, gr->g_w, gr->g_h, pxy, max_w, max_h);
}

/*** ---------------------------------------------------------------------- ***/

static void save_fillattr(_WORD handle, _WORD attrib[5])
{
	vqf_attributes(handle, attrib);
}

/*** ---------------------------------------------------------------------- ***/

static void restore_fillattr(_WORD handle, _WORD attrib[5])
{
	vsf_interior(handle, attrib[0]);
	vsf_color(handle, attrib[1]);
	vsf_style(handle, attrib[2]);
	vswr_mode(handle, attrib[3]);
	vsf_perimeter(handle, attrib[4]);
}

/*** ---------------------------------------------------------------------- ***/

static void save_lineattr(_WORD handle, _WORD attrib[6])
{
	vql_attributes(handle, attrib);
}

/*** ---------------------------------------------------------------------- ***/

static void restore_lineattr(_WORD handle, _WORD attrib[6])
{
	vsl_type(handle, attrib[0]);
	vsl_color(handle, attrib[1]);
	vswr_mode(handle, attrib[2]);
	vsl_width(handle, attrib[3]);
	vsl_ends(handle, attrib[4], attrib[5]);
}

/*** ---------------------------------------------------------------------- ***/

static void save_textattr(_WORD handle, _WORD attrib[10])
{
	_WORD dummy;
	
	vqt_attributes(handle, attrib);
	vst_effects(handle, 0);
	vst_alignment(handle, ALI_LEFT, ALI_TOP, &dummy, &dummy);
	vst_rotation(handle, 0);
}

/*** ---------------------------------------------------------------------- ***/

static void save_clip(_WORD handle, _WORD clip[5])
{
	_WORD wo[57];
	
	vq_extnd(handle, 1, wo);
	clip[0] = wo[45];
	clip[1] = wo[46];
	clip[2] = wo[47];
	clip[3] = wo[48];
	clip[4] = wo[19];
}

/*** ---------------------------------------------------------------------- ***/

static void restore_clip(_WORD handle, _WORD clip[5])
{
	vs_clip(handle, clip[4], clip);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * set VDI clipping to coordinates in PARMBLK structure.
 * Neccessary on some AES that don't set the clipping
 * rectangle during objc_draw() (i.e. XaAES)
 */
static void set_user_clip(_WORD handle, PARMBLK *pb)
{
	_WORD pxy[4];
	
	pxy[0] = pb->pb_xc;
	pxy[1] = pb->pb_yc;
	pxy[2] = pb->pb_xc + pb->pb_wc - 1;
	pxy[3] = pb->pb_yc + pb->pb_hc - 1;
	vs_clip(handle, TRUE, pxy);
}

/*** ---------------------------------------------------------------------- ***/

static void restore_textattr(_WORD handle, _WORD attrib[10])
{
	_WORD dummy;
	
	vst_color(handle, attrib[1]);
	vst_rotation(handle, attrib[2]);
	vst_alignment(handle, attrib[3], attrib[4], &dummy, &dummy);
	/* original vqt_attributes returns mode-1 */
	vswr_mode(handle, attrib[5] + 1);
	vst_height(handle, attrib[7], &dummy, &dummy, &dummy, &dummy);
}

/*** ---------------------------------------------------------------------- ***/

static void scrfdb(MFDB *fdb)
{
	fdb->fd_addr = (void *) 0;
	GetScreenSize(&fdb->fd_w, &fdb->fd_h);
	fdb->fd_wdwidth = (fdb->fd_w + 15) >> 4;
	fdb->fd_stand = FALSE;
	fdb->fd_nplanes = xscrn_planes;
	fdb->fd_r1 = fdb->fd_r2 = fdb->fd_r3 = 0;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*****************************************************************************/
/* Test how many bytes per pixel are needed in device-dependent format       */
/*****************************************************************************/

static _WORD test_rez(void)
{
	_WORD i, np, color, bpp = 0;
	_WORD pxy[8], rgb[3];
	_UWORD backup[XMAX_PLANES], test[XMAX_PLANES], test2[XMAX_PLANES];
	_WORD black[3] = { 0, 0, 0 };
	_WORD white[3] = { 1000, 1000, 1000 };
	MFDB screen;
	MFDB pixel;
	MFDB stdfm;
	
	pixel.fd_addr = NULL;
	pixel.fd_w = 16;
	pixel.fd_h = 1;
	pixel.fd_wdwidth = 1;
	pixel.fd_stand = FALSE;
	pixel.fd_nplanes = xscrn_planes;
	pixel.fd_r1 = 0;
	pixel.fd_r2 = 0;
	pixel.fd_r3 = 0;
	stdfm = pixel;
	stdfm.fd_stand = TRUE;
	
	if (xscrn_planes >= 8)
	{
		if (xscrn_planes == 8)
		{
			color = 0xff;
			memset(test, 0, sizeof(test));
			memset(test2, 0, sizeof(test2));
			for (np = 0; np < xscrn_planes; np++)
				test2[np] = (color & (1 << np)) << (15 - np);
			
			pixel.fd_addr = test;
			stdfm.fd_addr = test2;
			vr_trnfm(xvdi_handle, &stdfm, &pixel);
			
			for (i = 1; i < xscrn_planes; i++)
				if (test[i])
					break;
			
			if (i >= xscrn_planes && !(test[0] & 0x00ff))
				bpp = 1;
		} else
		{
			_WORD lineattrib[6];
			_WORD clip[5];
			
			save_lineattr(xvdi_handle, lineattrib);
			save_clip(xvdi_handle, clip);
			
			scrfdb(&screen);
			pxy[0] = 0;
			pxy[1] = 0;
			pxy[2] = screen.fd_w - 1;
			pxy[3] = screen.fd_h - 1;
			vs_clip(xvdi_handle, FALSE, pxy);
			
			memset(backup, 0, sizeof(backup));
			
			vswr_mode(xvdi_handle, MD_REPLACE);
			vsl_ends(xvdi_handle, 0, 0);
			vsl_type(xvdi_handle, SOLID);
			vsl_width(xvdi_handle, 1);
			pxy[0] = 0;
			pxy[1] = 0;
			pxy[2] = 0;
			pxy[3] = 0;
			pxy[4] = 0;
			pxy[5] = 0;
			pxy[6] = 0;
			pxy[7] = 0;
			
			v_hide_c(xvdi_handle);
			
			/* save pixel */
			pixel.fd_addr = backup;
			vro_cpyfm(xvdi_handle, S_ONLY, pxy, &screen, &pixel);
			
			/* save old color */
			vq_color(xvdi_handle, TEST_COLOR, 1, rgb);
			
			/* set 1 white pixel */
			vsl_color(xvdi_handle, TEST_COLOR);
			vs_color(xvdi_handle, TEST_COLOR, white);
			v_pline(xvdi_handle, 2, pxy);
			
			/* fetch pixel value */
			memset(test, 0, sizeof(test));
			pixel.fd_addr = test;
			vro_cpyfm(xvdi_handle, S_ONLY, pxy, &screen, &pixel);
			
			for (i = ((xscrn_planes + 15) >> 4) * 2; i < xscrn_planes; i++)
				if (test[i])
					break;
			
			if (i >= xscrn_planes)
			{
				vs_color(xvdi_handle, TEST_COLOR, black);
				v_pline(xvdi_handle, 2, pxy);
				
				memset(test, 0, sizeof(test));
				vro_cpyfm(xvdi_handle, S_ONLY, pxy, &screen, &pixel);
				
				for (i = ((xscrn_planes + 15) >> 4) * 2; i < xscrn_planes; i++)
					if (test[i])
						break;
				
				if (i >= xscrn_planes)
					bpp = (xscrn_planes + 7) >> 3;
			}

			/* restore old color */
			vs_color(xvdi_handle, TEST_COLOR, rgb);
			
			/* restore saved pixel */
			pixel.fd_addr = backup;
			vro_cpyfm(xvdi_handle, S_ONLY, pxy, &pixel, &screen);
			
			restore_lineattr(xvdi_handle, lineattrib);
			restore_clip(xvdi_handle, clip);
			
			v_show_c(xvdi_handle, 1);
		}
	}

	return bpp;
}

static void std_palette(table4 palette)
{
	_WORD color;

	for (color = 0; color < XMAX_COLOR; color++)
	{
		vq_color(xvdi_handle, pixtbl[color], 1, palette[color]);
		palette[color][3] = pixtbl[color];
	}
}
			
/*****************************************************************************/
/* determine pixel values for the selected palette                           */
/*****************************************************************************/
static void xfill_colortbl(void)
{
	_WORD np, color, backup[XMAX_PLANES * 4];
	_WORD pxy[8], rgb[3];
	MFDB screen;
	MFDB pixel;
	MFDB stdfm;
	_UWORD pixel_data[XMAX_PLANES];
	_UWORD pixel_data2[XMAX_PLANES];
	
	pixel.fd_addr = NULL;
	pixel.fd_w = 16;
	pixel.fd_h = 1;
	pixel.fd_wdwidth = 1;
	pixel.fd_stand = FALSE;
	pixel.fd_nplanes = xscrn_planes;
	pixel.fd_r1 = 0;
	pixel.fd_r2 = 0;
	pixel.fd_r3 = 0;
	stdfm = pixel;
	stdfm.fd_stand = TRUE;
	
	if (xscrn_planes >= 8)
	{
		if (xscrn_planes > 8)
		{
			_WORD lineattrib[6];
			_WORD clip[5];
			
			save_lineattr(xvdi_handle, lineattrib);
			save_clip(xvdi_handle, clip);
			
			scrfdb(&screen);

			pxy[0] = 0;
			pxy[1] = 0;
			pxy[2] = screen.fd_w - 1;
			pxy[3] = screen.fd_h - 1;
			vs_clip(xvdi_handle, TRUE, pxy);
			v_hide_c(xvdi_handle);

			if (xpixelbytes == 0 && colortbl == NULL)
			{
				colortbl = (_WORD *)g_try_malloc(xscrn_planes * XMAX_COLOR * sizeof(_WORD));
			}
			
			memset(backup, 0, sizeof(backup));
			if (colortbl != NULL)
				memset(colortbl, 0, xscrn_planes * XMAX_COLOR * sizeof(_WORD));
			
			vswr_mode(xvdi_handle, MD_REPLACE);
			vsl_ends(xvdi_handle, 0, 0);
			vsl_type(xvdi_handle, SOLID);
			vsl_width(xvdi_handle, 1);
			
			/* save pixel value */
			memset(pxy, 0, sizeof(pxy));
			pxy[2] = 1;
			pxy[6] = 1;
			pixel.fd_addr = backup;
			vro_cpyfm(xvdi_handle, S_ONLY, pxy, &screen, &pixel);
			pxy[6] = 0;
			
			/* save old color */
			vq_color(xvdi_handle, TEST_COLOR, 1, rgb);

			for (color = 0; color < XMAX_COLOR; color++)
			{
				vs_color(xvdi_handle, TEST_COLOR, rgb_palette[color]);
				vsl_color(xvdi_handle, TEST_COLOR);
				pxy[2] = 1;
				v_pline(xvdi_handle, 2, pxy);
				
				pixel.fd_addr = pixel_data;
				stdfm.fd_addr = pixel_data2;

				/* vro_cpyfm, because v_get_pixel does not work for TrueColor */
				pxy[2] = 0;
				memset(pixel_data, 0, sizeof(pixel_data));
				vro_cpyfm(xvdi_handle, S_ONLY, pxy, &screen, &pixel);
				
				if (xpixelbytes != 0)
				{
					colortbl2[color] = 0L;
					memcpy(&colortbl2[color], pixel.fd_addr, xpixelbytes);
				} else
				{
					memset(pixel_data2, 0, sizeof(pixel_data2));
					vr_trnfm(xvdi_handle, &pixel, &stdfm);
					for (np = 0; np < xscrn_planes; np++)
						if (pixel_data2[np])
							pixel_data2[np] = 0xffff;
					if (colortbl != NULL)
						memcpy(&colortbl[color * xscrn_planes], pixel_data2, xscrn_planes * sizeof(_WORD));
				}
			}
			
			/* restore old color */
			vs_color(xvdi_handle, TEST_COLOR, rgb);

			/* restore old pixel */
			pixel.fd_addr = backup;
			pxy[2] = 1;
			pxy[6] = 1;
			vro_cpyfm(xvdi_handle, S_ONLY, pxy, &pixel, &screen);
			
			/* restore line attributes */
			restore_lineattr(xvdi_handle, lineattrib);
			restore_clip(xvdi_handle, clip);
			
			v_show_c(xvdi_handle, 1);
		} else
		{
			if (xpixelbytes != 0)
				for (color = 0; color < XMAX_COLOR; color++)
					*(_UBYTE *)&colortbl2[color] = color;
		}
	}
}

/*****************************************************************************/
/* std_to_byte converts an image from standard format to device dependent    */
/* format (for resolutions >= 16 Planes)                                     */
/*****************************************************************************/

static void std_to_byte(_UWORD *col_data, _LONG len, _WORD old_planes, _ULONG *colortbl2, MFDB *s)
{
	_LONG x, i, pos;
	_UWORD np, *new_data, pixel, color, back[XMAX_PLANES];
	_WORD memflag = FALSE;
	_UBYTE *p1, *p2;
	_ULONG colback = 0;
	_UWORD *plane_ptr[XMAX_PLANES];
	
	if ((s->fd_addr = (_UWORD *)g_try_malloc(len * 2 * s->fd_nplanes)) == NULL)
	{
		return;
	}
	memcpy(s->fd_addr, col_data, len * 2 * old_planes);
	new_data = (_UWORD *)s->fd_addr;
	p1 = (_UBYTE *)new_data;

	if (old_planes < 8)
	{
		colback = colortbl2[(1 << old_planes) - 1];
		colortbl2[(1 << old_planes) - 1] = colortbl2[XMAX_COLOR - 1];
	}
	
	for (i = 0; i < old_planes; i++)
		plane_ptr[i] = &col_data[i * len];
	
	pos = 0;
	
	for (x = 0; x < len; x++)
	{
		for (np = 0; np < old_planes; np++)
			back[np] = plane_ptr[np][x];
		
		for (pixel = 0; pixel < 16; pixel++)
		{
			color = 0;
			for (np = 0; np < old_planes; np++)
			{
				color |= ((back[np] & 0x8000) >> (15 - np));
				back[np] <<= 1;
			}
			
			switch (xpixelbytes)
			{
			case 2:
				new_data[pos++] = *(_UWORD *) &colortbl2[color];
				break;

			case 3:
				p2 = (_UBYTE *) &colortbl2[color];
				*(p1++) = *(p2++);
				*(p1++) = *(p2++);
				*(p1++) = *(p2++);
				break;

			case 4:
				((_ULONG *)new_data)[pos++] = colortbl2[color];
				break;
			}
		}
	}

	if (old_planes < 8)
		colortbl2[(1 << old_planes) - 1] = colback;

	if (memflag)
		g_free(col_data);
}

/*****************************************************************************/
/* Convert icon data to current resolution                                   */
/* (e.g. 4 Plane Icon to 24 Plane TrueColor)                                 */
/*****************************************************************************/

static _BOOL xfix_cicon(_UWORD *col_data, _LONG len, _WORD old_planes, _WORD new_planes, MFDB *s)
{
	_LONG x, i, old_len, rest_len, new_len;
	_UWORD np, *new_data, mask, pixel, bit, color, back[XMAX_PLANES];
	MFDB d;
	
	len >>= 1;

	s->fd_nplanes = new_planes;
	if (old_planes == new_planes)
	{
		d = *s;
		d.fd_stand = FALSE;
		s->fd_addr = col_data;
		if ((d.fd_addr = g_try_malloc(len * 2 * new_planes)) == NULL)
			d.fd_addr = s->fd_addr;
		
		vr_trnfm(xvdi_handle, s, &d);
		if (d.fd_addr != s->fd_addr)
		{
			memcpy(s->fd_addr, d.fd_addr, len * 2 * new_planes);
			g_free(d.fd_addr);
		}
		return TRUE;
	}
	
	old_len = old_planes * len;
	new_len = new_planes * len;
	rest_len = new_len - old_len;
	if (new_planes <= 8)
	{
		s->fd_addr = g_try_malloc(new_len * 2);
		if (s->fd_addr != NULL)
		{
			new_data = &((_UWORD *)s->fd_addr)[old_len];
			memset(new_data, 0, rest_len * 2);
			memcpy(s->fd_addr, col_data, old_len * 2);

			col_data = (_UWORD *)s->fd_addr;
			
			for (x = 0; x < len; x++)
			{
				mask = 0xffff;
				
				for (i = 0; i < old_len; i += len)
					mask &= col_data[x + i];
				
				if (mask)
					for (i = 0; i < rest_len; i += len)
						new_data[x + i] |= mask;
			}

			/* convert to device dependent format */
			d = *s;
			d.fd_stand = FALSE;
			if ((d.fd_addr = g_try_malloc(len * 2 * new_planes)) == NULL)
				d.fd_addr = s->fd_addr;
			
			vr_trnfm(xvdi_handle, s, &d);
			if (d.fd_addr != s->fd_addr)
			{
				memcpy(s->fd_addr, d.fd_addr, len * 2 * new_planes);
				g_free(d.fd_addr);
			}
		}
	} else
	{
		/* TrueColor */
		if (xpixelbytes == 0)
		{
			s->fd_addr = NULL;
			if (colortbl != NULL)
			{
				_UWORD *plane_ptr[XMAX_PLANES], *pos;
				_UWORD old_col[XMAX_PLANES];
				_UWORD maxcol = 0;
				
				if (old_planes < 8)
				{
					maxcol = (1 << old_planes) - 1;
					memcpy(old_col, &colortbl[maxcol * new_planes], new_planes * sizeof(_WORD));
					memset(&colortbl[maxcol * new_planes], 0, new_planes * sizeof(_WORD));
				}
				
				if ((new_data = (_UWORD *)g_try_malloc(len * 2 * new_planes)) != NULL)
				{
					_WORD *colp;
					
					memcpy(new_data, col_data, old_len * 2);
					memset(new_data + old_len, 0, rest_len * 2);
	
					for (i = 0; i < new_planes; i++)
						plane_ptr[i] = &new_data[i * len];
		
					for (x = 0; x < len; x++)
					{
						bit = 1;
						for (np = 0; np < old_planes; np++)
							back[np] = plane_ptr[np][x];
						
						for (pixel = 0; pixel < 16; pixel++)
						{
							color = 0;
							for (np = 0; np < old_planes; np++)
							{
								color += ((back[np] & 1) << np);
								back[np] >>= 1;
							}
							
							colp = &colortbl[color * new_planes];
							for (np = 0; np < new_planes; np++)
							{
								pos = plane_ptr[np] + x;
								*pos = (*pos & ~bit) | (colp[np] & bit);
							}
		
							bit <<= 1;
						}
					}
					if (old_planes < 8)
						memcpy(&colortbl[maxcol * new_planes], old_col, new_planes * sizeof(_WORD));
		
					/* convert to device dependent format */
					d = *s;
					s->fd_addr = new_data;
					d.fd_stand = FALSE;
					if ((d.fd_addr = g_try_malloc(len * 2 * new_planes)) == NULL)
						d.fd_addr = s->fd_addr;
					
					vr_trnfm(xvdi_handle, s, &d);
					if (d.fd_addr != s->fd_addr)
					{
						memcpy(s->fd_addr, d.fd_addr, len * 2 * new_planes);
						g_free(d.fd_addr);
					}
				}
			}
		} else
		{
			std_to_byte(col_data, len, old_planes, colortbl2, s);
		}
	}
	if (s->fd_addr == NULL)
	{
		return FALSE;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

gboolean W_Fix_Bitmap(void **data, _WORD width, _WORD height, _WORD planes)
{
	MFDB d;
	
	if (xpixelbytes < 0)
	{
		xpixelbytes = test_rez();
		std_palette(rgb_palette);
		xfill_colortbl();
	}
	if (planes > 1)
	{
		long len;
		
		d.fd_w = width;
		d.fd_h = height;
		d.fd_wdwidth = (d.fd_w + 15) >> 4;
		d.fd_stand = TRUE;
		d.fd_nplanes = xscrn_planes;
		len = (long) d.fd_wdwidth * 2l * d.fd_h;
		
		if (planes > xscrn_planes)
			planes = xscrn_planes;
		xfix_cicon((_UWORD *)*data, len, planes, xscrn_planes, &d);
		*data = d.fd_addr;
	}
	return planes == 1 || planes == xscrn_planes;
}

/*** ---------------------------------------------------------------------- ***/

void W_Release_Bitmap(void **pdata, _WORD width, _WORD height, _WORD planes)
{
	unsigned char *data = (unsigned char *)(*pdata);
	
	UNUSED(width);
	UNUSED(height);
	if (data != NULL)
	{
		if (planes == 1 || planes == xscrn_planes)
		{
			/*
			 * image was converted inplace, head back to buffer start
			 */
			data -= SIZEOF_HYP_PICTURE;
		}
		g_free(data);
		*pdata = NULL;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void xfix_make_selmask(_WORD w, _WORD h, void *dst, const void *src)
{
	_UWORD mask = 0x5555;
	_WORD x, y;
	_UWORD *d = (_UWORD *)dst;
	const _UWORD *s = (const _UWORD *)src;
	
	w = (w + 15) >> 4;
	for (y = h; --y >= 0;)
	{
		for (x = w; --x >= 0; )
			*d++ = (*s++) & mask;
		mask = ~mask;
	}
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#if NICELINES
static _WORD _CDECL draw_niceline(PARMBLK *pb)
{
	GRECT gr;
	GRECT clip;
	_WORD pxy[4];
	_WORD aes_clip[5];
	_WORD vdi_h = aes_handle;
	
	save_clip(vdi_h, aes_clip);
	set_user_clip(vdi_h, pb);
	
	gr.g_x = pb->pb_x;
	gr.g_y = pb->pb_y + pb->pb_h / 2 - 1;
	gr.g_w = pb->pb_w;
	gr.g_h = 2;
	clip.g_x = pb->pb_xc;
	clip.g_y = pb->pb_yc;
	clip.g_w = pb->pb_wc;
	clip.g_h = pb->pb_hc;
	if (rc_intersect(&clip, &gr))
	{
		_WORD attrib[5];
		_WORD max_w, max_h;
		
		save_fillattr(vdi_h, attrib);
		
		vswr_mode(vdi_h, MD_REPLACE);
		vsf_perimeter(vdi_h, FALSE);
		GetScreenSize(&max_w, &max_h);
		rect2pxy(&gr, pxy, max_w, max_h);
		if (GetNumColors() >= 16)
		{
			vsf_color(vdi_h, G_LBLACK);
			vsf_interior(vdi_h, FIS_SOLID);
			vsf_style(vdi_h, 0);
		} else
		{
			vsf_color(vdi_h, G_BLACK);
			vsf_interior(vdi_h, FIS_PATTERN);
			vsf_style(vdi_h, 4);
		}
		vr_recfl(vdi_h, pxy);
		
		restore_fillattr(vdi_h, attrib);
	}
	restore_clip(vdi_h, aes_clip);
	return OS_NORMAL;
}
#endif

/*** ---------------------------------------------------------------------- ***/

static void draw_small_text(const char *text, _WORD len, _WORD *pxy)
{
	_WORD extent[8];
	_WORD textw, texth;
	
	vqt_extentn(aes_handle, text, len, extent);
	textw = extent[2] - extent[0];
	texth = extent[5] - extent[3];
	pxy[0] += (pxy[2] - pxy[0] + 1 - textw) / 2;
	pxy[1] += (pxy[3] - pxy[1] + 1 - texth) / 2;
	v_gtextn(aes_handle, pxy[0], pxy[1], text, len);
}

/*** ---------------------------------------------------------------------- ***/

static void draw_icon_text(ICONBLK *ib, _WORD xpos, _WORD ypos, _WORD selected)
{
	_WORD x, y, w, h;
	_WORD pxy[4];
	_WORD datacol = ICOLSPEC_GET_DATACOL(ib->ib_char);
	_WORD maskcol = ICOLSPEC_GET_MASKCOL(ib->ib_char);
	char c;
	_WORD max_w, max_h;
	_WORD dummy;
	_WORD cw, ch;
	_WORD attrib[5];
	_WORD textattrib[10];
	
	save_fillattr(aes_handle, attrib);
	save_textattr(aes_handle, textattrib);

	GetScreenSize(&max_w, &max_h);
	x = xpos + ib->ib_xtext;
	y = ypos + ib->ib_ytext;
	w = ib->ib_wtext;
	h = ib->ib_htext;
	vsf_interior(aes_handle, FIS_SOLID);
	vsf_style(aes_handle, 0);
	vst_height(aes_handle, 4, &dummy, &dummy, &cw, &ch);
	vswr_mode(aes_handle, MD_TRANS);
	if (w > 0 && h > 0 && ib->ib_ptext && *ib->ib_ptext &&
		xywh2pxy(x, y, w, h, pxy, max_w, max_h))
	{
		if (selected)
		{
			vsf_color(aes_handle, datacol);
			if (datacol != G_WHITE)
				vr_recfl(aes_handle, pxy);
			vst_color(aes_handle, maskcol);
			draw_small_text(ib->ib_ptext, (_WORD)strlen(ib->ib_ptext), pxy);
		} else
		{
			vsf_color(aes_handle, maskcol);
			if (maskcol != G_WHITE)
				vr_recfl(aes_handle, pxy);
			vst_color(aes_handle, datacol);
			draw_small_text(ib->ib_ptext, (_WORD)strlen(ib->ib_ptext), pxy);
		}
	}
	
	c = ICOLSPEC_GET_CHARACTER(ib->ib_char);
	if (c != 0)
	{
		x = xpos + ib->ib_xicon + ib->ib_xchar;
		y = ypos + ib->ib_yicon + ib->ib_ychar;
		if (xywh2pxy(x, y, cw, ch, pxy, max_w, max_h))
		{
			vst_color(aes_handle, selected ? maskcol : datacol);
			v_gtextn(aes_handle, pxy[0], pxy[1], &c, 1);
		}
	}

	restore_textattr(aes_handle, textattrib);
	restore_fillattr(aes_handle, attrib);
}

/*** ---------------------------------------------------------------------- ***/

static CICON *cicon_best_match(CICONBLK *cib, _WORD planes)
{
	_WORD best_match = 0;
	CICON *best = NULL;
	CICON *list;
	
	for (list = cib->mainlist; list != NULL; list = list->next_res)
	{
		if (list->num_planes > best_match && list->num_planes <= planes)
		{
			best_match = list->num_planes;
			best = list;
		}
	}
	return best;
}

/*** ---------------------------------------------------------------------- ***/

static _WORD _CDECL draw_cicon(PARMBLK *pb)
{
	MFDB src, dst;
	_WORD pxy[8];
	struct userdef *u = (struct userdef *)pb->pb_parm;
	CICONBLK *ciconblk = u->orig.ob_spec.ciconblk;
	ICONBLK *ib = &ciconblk->monoblk;
	_WORD x = pb->pb_x + ib->ib_xicon;
	_WORD y = pb->pb_y + ib->ib_yicon;
	_WORD w = ib->ib_wicon;
	_WORD h = ib->ib_hicon;
	_WORD datacol = ICOLSPEC_GET_DATACOL(ib->ib_char);
	_WORD maskcol = ICOLSPEC_GET_MASKCOL(ib->ib_char);
	_WORD colind[2];
	_WORD selected = pb->pb_currstate & OS_SELECTED;
	CICON *cicon;
	void *pdata, *pmask;
	_WORD aes_clip[5];
	
	save_clip(aes_handle, aes_clip);
	set_user_clip(aes_handle, pb);
	
	scrfdb(&dst);
	src.fd_w = w;
	src.fd_h = h;
	src.fd_wdwidth = (src.fd_w + 15) >> 4;
	src.fd_stand = TRUE;
	src.fd_nplanes = 1;
	src.fd_r1 = src.fd_r2 = src.fd_r3 = 0;
	if (xywh2pxy(x, y, w, h, &pxy[4], dst.fd_w, dst.fd_h))
	{
		pxy[0] = pxy[4] - x;
		pxy[1] = pxy[5] - y;
		pxy[2] = pxy[6] - x;
		pxy[3] = pxy[7] - y;
		
		cicon = ciconblk->mainlist;
		if (selected)
		{
			if (cicon->sel_data != NULL)
			{
				pmask = cicon->sel_mask;
				pdata = cicon->sel_data;
				selected = FALSE;
			} else
			{
				pmask = cicon->col_mask;
				pdata = cicon->col_data;
	
				/*
				 * for >1 planes, draw selection by darken the image
				 * instead of inverting, if the icon didn't have a mask
				 */
				if (cicon->sel_mask != NULL && cicon->num_planes > 1)
					selected = 2;
			}
		} else
		{
			pmask = cicon->col_mask;
			pdata = cicon->col_data;
		}
		
		src.fd_addr = pmask;
		src.fd_nplanes = 1;
		src.fd_stand = TRUE;
		colind[0] = selected == 1 ? datacol : maskcol;
		colind[1] = G_WHITE;
		vrt_cpyfm(aes_handle, MD_TRANS, pxy, &src, &dst, colind);
		
		src.fd_nplanes = cicon->num_planes;
		if (src.fd_nplanes == 1)
		{
			src.fd_addr = pdata;
			src.fd_stand = TRUE;
			colind[0] = selected ? maskcol : datacol;
			colind[1] = G_WHITE;
			vrt_cpyfm(aes_handle, MD_TRANS, pxy, &src, &dst, colind);
		} else
		{
			_WORD mode;
			
			if (dst.fd_nplanes > 8)
				mode = S_AND_D;
			else
				mode = S_OR_D;
			src.fd_addr = pdata;
			src.fd_stand = FALSE;
			src.fd_nplanes = xscrn_planes;
			vro_cpyfm(aes_handle, mode, pxy, &src, &dst);
			if (selected == 2)
			{
				src.fd_nplanes = 1;
				src.fd_addr = cicon->sel_mask;
				src.fd_stand = TRUE;
				colind[0] = G_BLACK;
				colind[1] = G_WHITE;
				vrt_cpyfm(aes_handle, MD_TRANS, pxy, &src, &dst, colind);
			}
		}
	}
	
	draw_icon_text(ib, pb->pb_x, pb->pb_y, selected);
	
	restore_clip(aes_handle, aes_clip);
	
	return pb->pb_currstate & ~OS_SELECTED;
}

static _WORD const rgb_to_vdi_tab[256] = {
   0,    4,    8,   12,   16,   20,   24,   28,   32,   36,   40,   43,   47,   51,   55,   59,
  63,   67,   71,   75,   79,   83,   86,   90,   94,   98,  102,  106,  110,  114,  118,  122,
 126,  129,  133,  137,  141,  145,  149,  153,  157,  161,  165,  168,  172,  176,  180,  184,
 188,  192,  196,  200,  204,  208,  211,  215,  219,  223,  227,  231,  235,  239,  243,  247,
 251,  254,  258,  262,  266,  270,  274,  278,  282,  286,  290,  294,  298,  301,  305,  309,
 313,  317,  321,  325,  329,  333,  337,  341,  345,  349,  352,  356,  360,  364,  368,  372,
 376,  380,  384,  388,  392,  396,  400,  403,  407,  411,  415,  419,  423,  427,  431,  435,
 439,  443,  447,  450,  454,  458,  462,  466,  470,  474,  478,  482,  486,  490,  494,  498,
 501,  505,  509,  513,  517,  521,  525,  529,  533,  537,  541,  545,  549,  552,  556,  560,
 564,  568,  572,  576,  580,  584,  588,  592,  596,  600,  603,  607,  611,  615,  619,  623,
 627,  631,  635,  639,  643,  647,  650,  654,  658,  662,  666,  670,  674,  678,  682,  686,
 690,  694,  698,  701,  705,  709,  713,  717,  721,  725,  729,  733,  737,  741,  745,  749,
 752,  756,  760,  764,  768,  772,  776,  780,  784,  788,  792,  796,  800,  803,  807,  811,
 815,  819,  823,  827,  831,  835,  839,  843,  847,  850,  854,  858,  862,  866,  870,  874,
 878,  882,  886,  890,  894,  898,  901,  905,  909,  913,  917,  921,  925,  929,  933,  937,
 941,  945,  949,  952,  956,  960,  964,  968,  972,  976,  980,  984,  988,  992,  996, 1000
};

static PALETTE const std256_palette = {
	{ 0xff, 0xff, 0xff }, /*   0 == WHITE (0) */
	{ 0xff, 0x00, 0x00 }, /*   1 == RED (2) */
	{ 0x00, 0xff, 0x00 }, /*   2 == GREEN (3) */
	{ 0xff, 0xff, 0x00 }, /*   3 == YELLOW (6) */
	{ 0x00, 0x00, 0xff }, /*   4 == BLUE (4) */
	{ 0xff, 0x00, 0xff }, /*   5 == MAGENTA (7) */
	{ 0x00, 0xff, 0xff }, /*   6 == CYAN (5) */
	{ 0xcc, 0xcc, 0xcc }, /*   7 == LWHITE (8) */
	{ 0x88, 0x88, 0x88 }, /*   8 == LBLACK (9) */
	{ 0x88, 0x00, 0x00 }, /*   9 == LRED (10) */
	{ 0x00, 0x88, 0x00 }, /*  10 == LGREEN (11) */
	{ 0x88, 0x88, 0x00 }, /*  11 == LYELLOW (14) */
	{ 0x00, 0x00, 0x88 }, /*  12 == LBLUE (12) */
	{ 0x88, 0x00, 0x88 }, /*  13 == LMAGENTA (15) */
	{ 0x00, 0x88, 0x88 }, /*  14 == LCYAN (13) */
	{ 0x00, 0x00, 0x00 }, /*  15 == BLACK (255) */
	{ 0xfd, 0xfd, 0xfd }, /*  16 == 16 */
	{ 0xec, 0xec, 0xec }, /*  17 == 17 */
	{ 0xdb, 0xdb, 0xdb }, /*  18 == 18 */
	{ 0xca, 0xca, 0xca }, /*  19 == 19 */
	{ 0xb9, 0xb9, 0xb9 }, /*  20 == 20 */
	{ 0xa8, 0xa8, 0xa8 }, /*  21 == 21 */
	{ 0x97, 0x97, 0x97 }, /*  22 == 22 */
	{ 0x87, 0x87, 0x87 }, /*  23 == 23 */
	{ 0x76, 0x76, 0x76 }, /*  24 == 24 */
	{ 0x65, 0x65, 0x65 }, /*  25 == 25 */
	{ 0x54, 0x54, 0x54 }, /*  26 == 26 */
	{ 0x43, 0x43, 0x43 }, /*  27 == 27 */
	{ 0x32, 0x32, 0x32 }, /*  28 == 28 */
	{ 0x21, 0x21, 0x21 }, /*  29 == 29 */
	{ 0x10, 0x10, 0x10 }, /*  30 == 30 */
	{ 0x00, 0x00, 0x00 }, /*  31 == 31 */
	{ 0xfd, 0x00, 0x00 }, /*  32 == 32 */
	{ 0xfd, 0x00, 0x10 }, /*  33 == 33 */
	{ 0xfd, 0x00, 0x21 }, /*  34 == 34 */
	{ 0xfd, 0x00, 0x32 }, /*  35 == 35 */
	{ 0xfd, 0x00, 0x43 }, /*  36 == 36 */
	{ 0xfd, 0x00, 0x54 }, /*  37 == 37 */
	{ 0xfd, 0x00, 0x65 }, /*  38 == 38 */
	{ 0xfd, 0x00, 0x76 }, /*  39 == 39 */
	{ 0xfd, 0x00, 0x87 }, /*  40 == 40 */
	{ 0xfd, 0x00, 0x97 }, /*  41 == 41 */
	{ 0xfd, 0x00, 0xa8 }, /*  42 == 42 */
	{ 0xfd, 0x00, 0xb9 }, /*  43 == 43 */
	{ 0xfd, 0x00, 0xca }, /*  44 == 44 */
	{ 0xfd, 0x00, 0xdb }, /*  45 == 45 */
	{ 0xfd, 0x00, 0xec }, /*  46 == 46 */
	{ 0xfd, 0x00, 0xfd }, /*  47 == 47 */
	{ 0xec, 0x00, 0xfd }, /*  48 == 48 */
	{ 0xdb, 0x00, 0xfd }, /*  49 == 49 */
	{ 0xca, 0x00, 0xfd }, /*  50 == 50 */
	{ 0xb9, 0x00, 0xfd }, /*  51 == 51 */
	{ 0xa8, 0x00, 0xfd }, /*  52 == 52 */
	{ 0x97, 0x00, 0xfd }, /*  53 == 53 */
	{ 0x87, 0x00, 0xfd }, /*  54 == 54 */
	{ 0x76, 0x00, 0xfd }, /*  55 == 55 */
	{ 0x65, 0x00, 0xfd }, /*  56 == 56 */
	{ 0x54, 0x00, 0xfd }, /*  57 == 57 */
	{ 0x43, 0x00, 0xfd }, /*  58 == 58 */
	{ 0x32, 0x00, 0xfd }, /*  59 == 59 */
	{ 0x21, 0x00, 0xfd }, /*  60 == 60 */
	{ 0x10, 0x00, 0xfd }, /*  61 == 61 */
	{ 0x00, 0x00, 0xfd }, /*  62 == 62 */
	{ 0x00, 0x10, 0xfd }, /*  63 == 63 */
	{ 0x00, 0x21, 0xfd }, /*  64 == 64 */
	{ 0x00, 0x32, 0xfd }, /*  65 == 65 */
	{ 0x00, 0x43, 0xfd }, /*  66 == 66 */
	{ 0x00, 0x54, 0xfd }, /*  67 == 67 */
	{ 0x00, 0x65, 0xfd }, /*  68 == 68 */
	{ 0x00, 0x76, 0xfd }, /*  69 == 69 */
	{ 0x00, 0x87, 0xfd }, /*  70 == 70 */
	{ 0x00, 0x97, 0xfd }, /*  71 == 71 */
	{ 0x00, 0xa8, 0xfd }, /*  72 == 72 */
	{ 0x00, 0xb9, 0xfd }, /*  73 == 73 */
	{ 0x00, 0xca, 0xfd }, /*  74 == 74 */
	{ 0x00, 0xdb, 0xfd }, /*  75 == 75 */
	{ 0x00, 0xec, 0xfd }, /*  76 == 76 */
	{ 0x00, 0xfd, 0xfd }, /*  77 == 77 */
	{ 0x00, 0xfd, 0xec }, /*  78 == 78 */
	{ 0x00, 0xfd, 0xdb }, /*  79 == 79 */
	{ 0x00, 0xfd, 0xca }, /*  80 == 80 */
	{ 0x00, 0xfd, 0xb9 }, /*  81 == 81 */
	{ 0x00, 0xfd, 0xa8 }, /*  82 == 82 */
	{ 0x00, 0xfd, 0x97 }, /*  83 == 83 */
	{ 0x00, 0xfd, 0x87 }, /*  84 == 84 */
	{ 0x00, 0xfd, 0x76 }, /*  85 == 85 */
	{ 0x00, 0xfd, 0x65 }, /*  86 == 86 */
	{ 0x00, 0xfd, 0x54 }, /*  87 == 87 */
	{ 0x00, 0xfd, 0x43 }, /*  88 == 88 */
	{ 0x00, 0xfd, 0x32 }, /*  89 == 89 */
	{ 0x00, 0xfd, 0x21 }, /*  90 == 90 */
	{ 0x00, 0xfd, 0x10 }, /*  91 == 91 */
	{ 0x00, 0xfd, 0x00 }, /*  92 == 92 */
	{ 0x10, 0xfd, 0x00 }, /*  93 == 93 */
	{ 0x21, 0xfd, 0x00 }, /*  94 == 94 */
	{ 0x32, 0xfd, 0x00 }, /*  95 == 95 */
	{ 0x43, 0xfd, 0x00 }, /*  96 == 96 */
	{ 0x54, 0xfd, 0x00 }, /*  97 == 97 */
	{ 0x65, 0xfd, 0x00 }, /*  98 == 98 */
	{ 0x76, 0xfd, 0x00 }, /*  99 == 99 */
	{ 0x87, 0xfd, 0x00 }, /* 100 == 100 */
	{ 0x97, 0xfd, 0x00 }, /* 101 == 101 */
	{ 0xa8, 0xfd, 0x00 }, /* 102 == 102 */
	{ 0xb9, 0xfd, 0x00 }, /* 103 == 103 */
	{ 0xca, 0xfd, 0x00 }, /* 104 == 104 */
	{ 0xdb, 0xfd, 0x00 }, /* 105 == 105 */
	{ 0xec, 0xfd, 0x00 }, /* 106 == 106 */
	{ 0xfd, 0xfd, 0x00 }, /* 107 == 107 */
	{ 0xfd, 0xec, 0x00 }, /* 108 == 108 */
	{ 0xfd, 0xdb, 0x00 }, /* 109 == 109 */
	{ 0xfd, 0xca, 0x00 }, /* 110 == 110 */
	{ 0xfd, 0xb9, 0x00 }, /* 111 == 111 */
	{ 0xfd, 0xa8, 0x00 }, /* 112 == 112 */
	{ 0xfd, 0x97, 0x00 }, /* 113 == 113 */
	{ 0xfd, 0x87, 0x00 }, /* 114 == 114 */
	{ 0xfd, 0x76, 0x00 }, /* 115 == 115 */
	{ 0xfd, 0x65, 0x00 }, /* 116 == 116 */
	{ 0xfd, 0x54, 0x00 }, /* 117 == 117 */
	{ 0xfd, 0x43, 0x00 }, /* 118 == 118 */
	{ 0xfd, 0x32, 0x00 }, /* 119 == 119 */
	{ 0xfd, 0x21, 0x00 }, /* 120 == 120 */
	{ 0xfd, 0x10, 0x00 }, /* 121 == 121 */
	{ 0xb9, 0x00, 0x00 }, /* 122 == 122 */
	{ 0xb9, 0x00, 0x10 }, /* 123 == 123 */
	{ 0xb9, 0x00, 0x21 }, /* 124 == 124 */
	{ 0xb9, 0x00, 0x32 }, /* 125 == 125 */
	{ 0xb9, 0x00, 0x43 }, /* 126 == 126 */
	{ 0xb9, 0x00, 0x54 }, /* 127 == 127 */
	{ 0xb9, 0x00, 0x65 }, /* 128 == 128 */
	{ 0xb9, 0x00, 0x76 }, /* 129 == 129 */
	{ 0xb9, 0x00, 0x87 }, /* 130 == 130 */
	{ 0xb9, 0x00, 0x97 }, /* 131 == 131 */
	{ 0xb9, 0x00, 0xa8 }, /* 132 == 132 */
	{ 0xb9, 0x00, 0xb9 }, /* 133 == 133 */
	{ 0xa8, 0x00, 0xb9 }, /* 134 == 134 */
	{ 0x97, 0x00, 0xb9 }, /* 135 == 135 */
	{ 0x87, 0x00, 0xb9 }, /* 136 == 136 */
	{ 0x76, 0x00, 0xb9 }, /* 137 == 137 */
	{ 0x65, 0x00, 0xb9 }, /* 138 == 138 */
	{ 0x54, 0x00, 0xb9 }, /* 139 == 139 */
	{ 0x43, 0x00, 0xb9 }, /* 140 == 140 */
	{ 0x32, 0x00, 0xb9 }, /* 141 == 141 */
	{ 0x21, 0x00, 0xb9 }, /* 142 == 142 */
	{ 0x10, 0x00, 0xb9 }, /* 143 == 143 */
	{ 0x00, 0x00, 0xb9 }, /* 144 == 144 */
	{ 0x00, 0x10, 0xb9 }, /* 145 == 145 */
	{ 0x00, 0x21, 0xb9 }, /* 146 == 146 */
	{ 0x00, 0x32, 0xb9 }, /* 147 == 147 */
	{ 0x00, 0x43, 0xb9 }, /* 148 == 148 */
	{ 0x00, 0x54, 0xb9 }, /* 149 == 149 */
	{ 0x00, 0x65, 0xb9 }, /* 150 == 150 */
	{ 0x00, 0x76, 0xb9 }, /* 151 == 151 */
	{ 0x00, 0x87, 0xb9 }, /* 152 == 152 */
	{ 0x00, 0x97, 0xb9 }, /* 153 == 153 */
	{ 0x00, 0xa8, 0xb9 }, /* 154 == 154 */
	{ 0x00, 0xb9, 0xb9 }, /* 155 == 155 */
	{ 0x00, 0xb9, 0xa8 }, /* 156 == 156 */
	{ 0x00, 0xb9, 0x97 }, /* 157 == 157 */
	{ 0x00, 0xb9, 0x87 }, /* 158 == 158 */
	{ 0x00, 0xb9, 0x76 }, /* 159 == 159 */
	{ 0x00, 0xb9, 0x65 }, /* 160 == 160 */
	{ 0x00, 0xb9, 0x54 }, /* 161 == 161 */
	{ 0x00, 0xb9, 0x43 }, /* 162 == 162 */
	{ 0x00, 0xb9, 0x32 }, /* 163 == 163 */
	{ 0x00, 0xb9, 0x21 }, /* 164 == 164 */
	{ 0x00, 0xb9, 0x10 }, /* 165 == 165 */
	{ 0x00, 0xb9, 0x00 }, /* 166 == 166 */
	{ 0x10, 0xb9, 0x00 }, /* 167 == 167 */
	{ 0x21, 0xb9, 0x00 }, /* 168 == 168 */
	{ 0x32, 0xb9, 0x00 }, /* 169 == 169 */
	{ 0x43, 0xb9, 0x00 }, /* 170 == 170 */
	{ 0x54, 0xb9, 0x00 }, /* 171 == 171 */
	{ 0x65, 0xb9, 0x00 }, /* 172 == 172 */
	{ 0x76, 0xb9, 0x00 }, /* 173 == 173 */
	{ 0x87, 0xb9, 0x00 }, /* 174 == 174 */
	{ 0x97, 0xb9, 0x00 }, /* 175 == 175 */
	{ 0xa8, 0xb9, 0x00 }, /* 176 == 176 */
	{ 0xb9, 0xb9, 0x00 }, /* 177 == 177 */
	{ 0xb9, 0xa8, 0x00 }, /* 178 == 178 */
	{ 0xb9, 0x97, 0x00 }, /* 179 == 179 */
	{ 0xb9, 0x87, 0x00 }, /* 180 == 180 */
	{ 0xb9, 0x76, 0x00 }, /* 181 == 181 */
	{ 0xb9, 0x65, 0x00 }, /* 182 == 182 */
	{ 0xb9, 0x54, 0x00 }, /* 183 == 183 */
	{ 0xb9, 0x43, 0x00 }, /* 184 == 184 */
	{ 0xb9, 0x32, 0x00 }, /* 185 == 185 */
	{ 0xb9, 0x21, 0x00 }, /* 186 == 186 */
	{ 0xb9, 0x10, 0x00 }, /* 187 == 187 */
	{ 0x76, 0x00, 0x00 }, /* 188 == 188 */
	{ 0x76, 0x00, 0x10 }, /* 189 == 189 */
	{ 0x76, 0x00, 0x21 }, /* 190 == 190 */
	{ 0x76, 0x00, 0x32 }, /* 191 == 191 */
	{ 0x76, 0x00, 0x43 }, /* 192 == 192 */
	{ 0x76, 0x00, 0x54 }, /* 193 == 193 */
	{ 0x76, 0x00, 0x65 }, /* 194 == 194 */
	{ 0x76, 0x00, 0x76 }, /* 195 == 195 */
	{ 0x65, 0x00, 0x76 }, /* 196 == 196 */
	{ 0x54, 0x00, 0x76 }, /* 197 == 197 */
	{ 0x43, 0x00, 0x76 }, /* 198 == 198 */
	{ 0x32, 0x00, 0x76 }, /* 199 == 199 */
	{ 0x21, 0x00, 0x76 }, /* 200 == 200 */
	{ 0x10, 0x00, 0x76 }, /* 201 == 201 */
	{ 0x00, 0x00, 0x76 }, /* 202 == 202 */
	{ 0x00, 0x10, 0x76 }, /* 203 == 203 */
	{ 0x00, 0x21, 0x76 }, /* 204 == 204 */
	{ 0x00, 0x32, 0x76 }, /* 205 == 205 */
	{ 0x00, 0x43, 0x76 }, /* 206 == 206 */
	{ 0x00, 0x54, 0x76 }, /* 207 == 207 */
	{ 0x00, 0x65, 0x76 }, /* 208 == 208 */
	{ 0x00, 0x76, 0x76 }, /* 209 == 209 */
	{ 0x00, 0x76, 0x65 }, /* 210 == 210 */
	{ 0x00, 0x76, 0x54 }, /* 211 == 211 */
	{ 0x00, 0x76, 0x43 }, /* 212 == 212 */
	{ 0x00, 0x76, 0x32 }, /* 213 == 213 */
	{ 0x00, 0x76, 0x21 }, /* 214 == 214 */
	{ 0x00, 0x76, 0x10 }, /* 215 == 215 */
	{ 0x00, 0x76, 0x00 }, /* 216 == 216 */
	{ 0x10, 0x76, 0x00 }, /* 217 == 217 */
	{ 0x21, 0x76, 0x00 }, /* 218 == 218 */
	{ 0x32, 0x76, 0x00 }, /* 219 == 219 */
	{ 0x43, 0x76, 0x00 }, /* 220 == 220 */
	{ 0x54, 0x76, 0x00 }, /* 221 == 221 */
	{ 0x65, 0x76, 0x00 }, /* 222 == 222 */
	{ 0x76, 0x76, 0x00 }, /* 223 == 223 */
	{ 0x76, 0x65, 0x00 }, /* 224 == 224 */
	{ 0x76, 0x54, 0x00 }, /* 225 == 225 */
	{ 0x76, 0x43, 0x00 }, /* 226 == 226 */
	{ 0x76, 0x32, 0x00 }, /* 227 == 227 */
	{ 0x76, 0x21, 0x00 }, /* 228 == 228 */
	{ 0x76, 0x10, 0x00 }, /* 229 == 229 */
	{ 0x43, 0x00, 0x00 }, /* 230 == 230 */
	{ 0x43, 0x00, 0x10 }, /* 231 == 231 */
	{ 0x43, 0x00, 0x21 }, /* 232 == 232 */
	{ 0x43, 0x00, 0x32 }, /* 233 == 233 */
	{ 0x43, 0x00, 0x43 }, /* 234 == 234 */
	{ 0x32, 0x00, 0x43 }, /* 235 == 235 */
	{ 0x21, 0x00, 0x43 }, /* 236 == 236 */
	{ 0x10, 0x00, 0x43 }, /* 237 == 237 */
	{ 0x00, 0x00, 0x43 }, /* 238 == 238 */
	{ 0x00, 0x10, 0x43 }, /* 239 == 239 */
	{ 0x00, 0x21, 0x43 }, /* 240 == 240 */
	{ 0x00, 0x32, 0x43 }, /* 241 == 241 */
	{ 0x00, 0x43, 0x43 }, /* 242 == 242 */
	{ 0x00, 0x43, 0x32 }, /* 243 == 243 */
	{ 0x00, 0x43, 0x21 }, /* 244 == 244 */
	{ 0x00, 0x43, 0x10 }, /* 245 == 245 */
	{ 0x00, 0x43, 0x00 }, /* 246 == 246 */
	{ 0x10, 0x43, 0x00 }, /* 247 == 247 */
	{ 0x21, 0x43, 0x00 }, /* 248 == 248 */
	{ 0x32, 0x43, 0x00 }, /* 249 == 249 */
	{ 0x43, 0x43, 0x00 }, /* 250 == 250 */
	{ 0x43, 0x32, 0x00 }, /* 251 == 251 */
	{ 0x43, 0x21, 0x00 }, /* 252 == 252 */
	{ 0x43, 0x10, 0x00 }, /* 253 == 253 */
	{ 0xfd, 0xfd, 0xfd }, /* 254 == 254 */
	{ 0x00, 0x00, 0x00 }  /* 255 == BLACK (1) */
};

/*** ---------------------------------------------------------------------- ***/

_WORD pic_rgb_to_vdi(unsigned char c)
{
	return rgb_to_vdi_tab[c];
}

/*** ---------------------------------------------------------------------- ***/

void pic_stdpalette(PALETTE pal, _WORD planes)
{
	_WORD i;
	_WORD colors;
	
	if (planes > 8)
		planes = 8;
	colors = 1 << planes;
	for (i = 0; i < colors; i++)
		pal[i] = std256_palette[i];
	if (planes < 8)
	{
		pal[colors - 1] = std256_palette[255];
	}
}

/*** ---------------------------------------------------------------------- ***/

void hfix_palette(_WORD vdi_h)
{
	if (xscrn_planes >= 8)
	{
		_WORD pixel;
		_WORD rgbnew[4];
		PALETTE im_palette;
		
		pic_stdpalette(im_palette, 8);
		for (pixel = 0; pixel < 256; pixel++)
		{
			rgbnew[0] = pic_rgb_to_vdi(im_palette[pixel].r);
			rgbnew[1] = pic_rgb_to_vdi(im_palette[pixel].g);
			rgbnew[2] = pic_rgb_to_vdi(im_palette[pixel].b);
			vs_color(vdi_h, pixtbl[pixel], rgbnew);
			if (vdi_h != aes_handle)
				vs_color(aes_handle, pixtbl[pixel], rgbnew);
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void *hfix_objs(RSHDR *hdr, OBJECT *objects, _WORD num_objs)
{
	_WORD i;
	_WORD num_user;
	_WORD dummy;
	struct userdef_list *list;
	
	aes_handle = graf_handle(&dummy, &dummy, &dummy, &dummy);
	{
		_WORD wo[57];
		vq_extnd(aes_handle, 1, wo);
		xscrn_planes = wo[4];
	}
	if (xpixelbytes < 0)
	{
		xpixelbytes = test_rez();
		hfix_palette(xvdi_handle);
		std_palette(rgb_palette);
	}
	xfill_colortbl();
	
	num_user = 0;
	for (i = 0; i < num_objs; i++)
	{
		OBJECT *ob = &objects[i];
		
		switch (ob->ob_type & OBTYPEMASK)
		{
		case G_ICON:
			break;

		case G_CICON:
			{
				CICONBLK *cicon = ob->ob_spec.ciconblk;

				if (cicon_best_match(cicon, xscrn_planes) != NULL)
					++num_user;
			}
			break;

		case G_IMAGE:
			break;

		case G_FTEXT:
		case G_FBOXTEXT:
			break;

		case G_TEXT:
		case G_BOXTEXT:
			break;

		case G_TITLE:
		case G_BUTTON:
			break;

		case G_STRING:
		case G_SHORTCUT:
#if NICELINES
			if ((ob->ob_state & OS_DISABLED) &&
				ob->ob_spec.free_string[0] == '-')
			{
				num_user++;
			}
#endif
			break;
		}
	}

	list = (struct userdef_list *)g_malloc(sizeof(*list) + (num_user - 1) * sizeof(struct userdef));
	if (list == NULL)
		return NULL;
	list->hdr = hdr;
	list->objects = objects;
	list->nobjs = num_objs;
	list->nuserobjs = num_user;
	list->next = userdefs;
	userdefs = list;

#define adduser(f) \
	list->userobjs[num_user].orig = *ob; \
	list->userobjs[num_user].obj = i; \
	list->userobjs[num_user].user.ub_code = f; \
	list->userobjs[num_user].user.ub_parm = (_LONG_PTR)&list->userobjs[num_user]; \
	ob->ob_type = G_USERDEF; \
	ob->ob_spec.userblk = &list->userobjs[num_user].user; \
	num_user++
	
	num_user = 0;
	for (i = 0; i < num_objs; i++)
	{
		OBJECT *ob = &objects[i];
		
		switch (ob->ob_type & OBTYPEMASK)
		{
		case G_ICON:
			break;

		case G_CICON:
			{
				CICONBLK *ciconblk = ob->ob_spec.ciconblk;
				CICON *cicon;
				MFDB d;
				_LONG len;
				
				cicon = cicon_best_match(ciconblk, xscrn_planes);
				if (cicon != NULL)
				{
					if (cicon->num_planes > 1)
					{
						d.fd_w = ciconblk->monoblk.ib_wicon;
						d.fd_h = ciconblk->monoblk.ib_hicon;
						d.fd_wdwidth = (d.fd_w + 15) >> 4;
						d.fd_stand = TRUE;
						d.fd_nplanes = xscrn_planes;
						len = (_LONG) d.fd_wdwidth * 2l * d.fd_h;
						
						xfix_cicon((_UWORD *)cicon->col_data, len, cicon->num_planes, xscrn_planes, &d);
						cicon->col_data = (_WORD *)d.fd_addr;
						if (cicon->sel_data)
						{
							xfix_cicon((_UWORD *)cicon->sel_data, len, cicon->num_planes, xscrn_planes, &d);
							cicon->sel_data = (_WORD *)d.fd_addr;
						} else
						{
							/* prepare darken mask */
							cicon->sel_mask = (_WORD *)g_try_malloc(len);
							if (cicon->sel_mask != NULL)
								xfix_make_selmask(d.fd_w, d.fd_h, cicon->sel_mask, cicon->col_mask);
						}
					}
					ciconblk->mainlist = cicon;
					cicon->next_res = NULL;
					adduser(draw_cicon);
				} else
				{
					ob->ob_spec.iconblk = &ciconblk->monoblk;
					ob->ob_type = (ob->ob_type & OBEXTTYPEMASK) | G_ICON;
				}
			}
			break;

		case G_IMAGE:
			break;

		case G_FTEXT:
		case G_FBOXTEXT:
			break;

		case G_TEXT:
		case G_BOXTEXT:
			break;

		case G_TITLE:
		case G_BUTTON:
			break;

		case G_STRING:
		case G_SHORTCUT:
#if NICELINES
			if ((ob->ob_state & OS_DISABLED) &&
				ob->ob_spec.free_string[0] == '-')
			{
				adduser(draw_niceline);
			}
#endif
			break;
		}
	}
	ASSERT(num_user == list->nuserobjs);
#undef adduser
	return list;
}

/*** ---------------------------------------------------------------------- ***/

static void release_list(struct userdef_list *list)
{
	OBJECT *tree = list->objects;
	_WORD num_objs = list->nobjs;
	_WORD i;

	for (i = 0; i < num_objs; i++)
	{
		OBJECT *ob = &tree[i];
		
		switch (ob->ob_type & OBTYPEMASK)
		{
		case G_ICON:
			break;

		case G_CICON:
			break;

		case G_IMAGE:
			break;
		}
	}
	
	for (i = 0; i < list->nuserobjs; i++)
	{
		tree[list->userobjs[i].obj] = list->userobjs[i].orig;
		switch (list->userobjs[i].orig.ob_type & OBTYPEMASK)
		{
		case G_CICON:
			{
				CICONBLK *ciconblk = list->userobjs[i].orig.ob_spec.ciconblk;
				CICON *cicon;

				cicon = ciconblk->mainlist;
				ASSERT(cicon != NULL);
				if (cicon->num_planes > 1 && cicon->num_planes != xscrn_planes)
				{
					g_free(cicon->col_data);
					if (cicon->sel_data)
						g_free(cicon->sel_data);
				}
				if (cicon->num_planes > 1 && cicon->sel_data == NULL)
					g_free(cicon->sel_mask);
			}
			break;
		}
	}

	if (list->hdr)
	{
		_AESrscmem = list->hdr;
		rsrc_free();
	}
}

/*** ---------------------------------------------------------------------- ***/

void hv_userdef_exit(void)
{
	struct userdef_list *list, *next;
	
	for (list = userdefs; list != NULL; list = next)
	{
		next = list->next;
		release_list(list);
		g_free(list);
	}
	userdefs = NULL;
	g_free(colortbl);
	colortbl = NULL;
}

/*** ---------------------------------------------------------------------- ***/

void hrelease_objs(OBJECT *tree, _WORD num_objs)
{
	/* no-op here; will be released above */
	UNUSED(tree);
	UNUSED(num_objs);
}
