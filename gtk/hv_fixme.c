#include "hv_gtk.h"

void W_Release_Bitmap(void **pdata, _WORD width, _WORD height, _WORD planes)
{
	UNUSED(pdata);
	UNUSED(width);
	UNUSED(height);
	UNUSED(planes);
}

void gfx_dither_16_to_2(void *addr, long plane_size, short dither);

void gfx_dither_16_to_2(void *addr, long plane_size, short dither)
{
	UNUSED(addr);
	UNUSED(plane_size);
	UNUSED(dither);
}

void mono_bitmap(void *src, void *dst, long planesize, short color);

void mono_bitmap(void *src, void *dst, long planesize, short color)
{
	UNUSED(src);
	UNUSED(dst);
	UNUSED(planesize);
	UNUSED(color);
}
