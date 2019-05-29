/*****************************************************************************
 * PNG.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "picture.h"


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
 * Do this without libpng functions, because
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
			type = *buf++;
			compress_method = *buf;
			switch (type)
			{
			case 0: /* grayscale */
				pic->pi_planes = depth;
				break;
			case 2: /* RGB */
				pic->pi_planes = depth * 3;
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
			pic->pi_dataoffset = 0;
			pic->pi_compressed = TRUE;
			pic_calcsize(pic);
			if (compress_method != 0)
			{
				pic->pi_unsupported = TRUE;
				return TRUE;
			}
			if (depth < 1 || depth > 8)
			{
				pic->pi_unsupported = TRUE;
				return TRUE;
			}
			if (plte_size == 0)
				return TRUE;
		} else if (magic == 0x504c5445ul) /* 'PLTE' */
		{
			if ((int)(length / 3) > plte_size || buf + length > end)
				return FALSE;
			plte_size = (int)(length / 3);
			for (i = 0; i < plte_size; i++)
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


#ifdef HAVE_PNG

#include "png.h"


typedef struct _writepng_info {
	double gamma;
	png_uint_32 width;
	png_uint_32 height;
	FILE *outfile;
	png_structp png_ptr;
	png_infop info_ptr;
	png_int_t bpp;
	png_int_t interlaced;
	int have_bg;
	png_uint_32 format;
	jmp_buf jmpbuf;
	png_int_t colormap_entries;
	png_color palette[PNG_MAX_PALETTE_LENGTH];
	png_color bg;
	size_t output_bytes;
	size_t memory_bytes;
	unsigned char *memory;
	unsigned char *rowbuf;
} writepng_info;



static void writepng_error_handler(png_structp png_ptr, png_const_charp msg)
{
	writepng_info *wpnginfo;

	(void) msg;
	wpnginfo = (writepng_info *)png_get_error_ptr(png_ptr);
	if (wpnginfo == NULL)
	{									/* we are completely hosed now */
		/* errout("writepng severe error:  jmpbuf not recoverable; terminating.\n"); */
		exit(99);
	}
	longjmp(wpnginfo->jmpbuf, 1);
}


static void writepng_warning_handler(png_structp png_ptr, png_const_charp msg)
{
	/*
	 * Silently ignore any warning messages from libpng.
	 * They stupidly tend to introduce new warnings with every release,
	 * with the default warning handler writing to stdout and/or stderr,
	 * messing up the output of the CGI scripts.
	 */
	(void) png_ptr;
	(void) msg;
}


static gboolean png_write_pic(writepng_info *wpnginfo, const unsigned char *src, PICTURE *pic)
{
	png_int_t color_type, interlace_type;
	short i, j, k;
	unsigned short mask;
	unsigned char color;
	unsigned char *row, *rp;
	long srcrowsize;
	long dstrowsize;
	const unsigned char *gp;

	wpnginfo->have_bg = -1;
	wpnginfo->bpp = pic->pi_planes;
	wpnginfo->width = pic->pi_width;
	wpnginfo->height = pic->pi_height;

	png_set_compression_level(wpnginfo->png_ptr, 9 /* Z_BEST_COMPRESSION */);

	if (pic->pi_planes <= 8)
	{
		wpnginfo->colormap_entries = 1 << pic->pi_planes;
		color_type = pic->pi_planes > 1 ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_GRAY;
		wpnginfo->format = pic->pi_planes > 1 ? PNG_FORMAT_RGB_COLORMAP : PNG_FORMAT_GRAY;
	} else if (pic->pi_planes == 24)
	{
		color_type = PNG_COLOR_TYPE_RGB;
		wpnginfo->format = PNG_FORMAT_RGB;
		wpnginfo->bpp = 8;
	} else if (pic->pi_planes == 32)
	{
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		wpnginfo->format = PNG_FORMAT_RGBA;
		wpnginfo->bpp = 8;
	} else
	{
		png_destroy_write_struct(&wpnginfo->png_ptr, &wpnginfo->info_ptr);
		g_free(wpnginfo);
		return FALSE;
	}
	if (wpnginfo->outfile == NULL)
	{
		wpnginfo->memory_bytes = PNG_IMAGE_PNG_SIZE_MAX(*wpnginfo);
		wpnginfo->memory = g_new(unsigned char, wpnginfo->memory_bytes);
		if (wpnginfo->memory == NULL)
		{
			g_free(wpnginfo);
			return FALSE;
		}
	}
	
	interlace_type = wpnginfo->interlaced ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE;

	png_set_IHDR(wpnginfo->png_ptr, wpnginfo->info_ptr, wpnginfo->width, wpnginfo->height,
				 wpnginfo->bpp, color_type, interlace_type,
				 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	if (wpnginfo->gamma > 0.0)
		png_set_gAMA(wpnginfo->png_ptr, wpnginfo->info_ptr, wpnginfo->gamma);

	if (wpnginfo->colormap_entries && color_type == PNG_COLOR_TYPE_PALETTE)
	{
		for (i = 0; i < wpnginfo->colormap_entries; i++)
		{
			wpnginfo->palette[i].red = pic->pi_palette[i].r;
			wpnginfo->palette[i].green = pic->pi_palette[i].g;
			wpnginfo->palette[i].blue = pic->pi_palette[i].b;
		}
		png_set_PLTE(wpnginfo->png_ptr, wpnginfo->info_ptr, wpnginfo->palette, wpnginfo->colormap_entries);
	}
	
	if (pic->pi_planes > 8 && wpnginfo->have_bg >= 0)
	{									/* we know it's RGBA, not gray+alpha */
		png_color_16 background;

		background.red = wpnginfo->bg.red;
		background.green = wpnginfo->bg.green;
		background.blue = wpnginfo->bg.blue;
		png_set_bKGD(wpnginfo->png_ptr, wpnginfo->info_ptr, &background);
	}

	if (pic->pi_planes <= 8 && wpnginfo->have_bg >= 0)
	{
		png_color_16 background;
		png_byte trans;
		
		background.red = wpnginfo->bg.red;
		background.green = wpnginfo->bg.green;
		background.blue = wpnginfo->bg.blue;
		trans = wpnginfo->have_bg;
		png_set_tRNS(wpnginfo->png_ptr, wpnginfo->info_ptr, &trans, 1, &background);
	}

	png_write_info(wpnginfo->png_ptr, wpnginfo->info_ptr);

	/* png_set_packing(png_ptr); */

	srcrowsize = pic_rowsize(pic, pic->pi_planes);
	dstrowsize = png_rowsize(pic, pic->pi_planes);
	row = wpnginfo->rowbuf = g_new(unsigned char, dstrowsize);
	if (row == NULL)
	{
		png_destroy_write_struct(&wpnginfo->png_ptr, &wpnginfo->info_ptr);
		g_free(wpnginfo->memory);
		g_free(wpnginfo);
		return FALSE;
	}

	switch (pic->pi_planes)
	{
	case 8:
		for (i = pic->pi_height; --i >= 0; )
		{
			rp = row;
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
					rp[j] = color;
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
						rp[j] = color;
					}
				}

				gp += 16;
				rp += 16;
			}
			png_write_row(wpnginfo->png_ptr, row);
			src += srcrowsize;
		}
		break;

	case 4:
		for (i = pic->pi_height; --i >= 0; )
		{
			rp = row;
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
					rp[j] = color << 4;
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
						rp[j] = color << 4;
					}
				}

				for (j = 0, mask = 0x40; j < 4 && j < lim; j++, mask >>= 2)
				{
					color = 0;
					if (gp[0] & mask) color |= 0x01;
					if (gp[2] & mask) color |= 0x02;
					if (gp[4] & mask) color |= 0x04;
					if (gp[6] & mask) color |= 0x08;
					rp[j] |= color;
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
						rp[j] |= color;
					}
				}
				gp += 8;
				rp += 8;
			}
			png_write_row(wpnginfo->png_ptr, row);
			src += srcrowsize;
		}
		break;

	case 1:
		{
			short l;

			j = (((pic->pi_width) + 7) >> 3);
			gp = src;
			for (i = pic->pi_height; --i >= 0; )
			{
				rp = row;
				for (l = 0; l < j; l++)
					*rp++ = ~gp[l];
				png_write_row(wpnginfo->png_ptr, row);
				gp += srcrowsize;
			}
		}
		break;
	}
	png_write_end(wpnginfo->png_ptr, NULL);
	if (wpnginfo->outfile)
	{
		pic->pi_datasize = ftell(wpnginfo->outfile);
	} else
	{
		pic->pi_datasize = wpnginfo->output_bytes;
		pic->pi_buf = wpnginfo->memory;
	}

	if (wpnginfo->png_ptr && wpnginfo->info_ptr)
	{
		png_destroy_write_struct(&wpnginfo->png_ptr, &wpnginfo->info_ptr);
		wpnginfo->png_ptr = NULL;
		wpnginfo->info_ptr = NULL;
	}
	
	g_free(row);
	g_free(wpnginfo);

	return TRUE;
}


gboolean png_fwrite(FILE *fp, const unsigned char *src, PICTURE *pic)
{
	writepng_info *wpnginfo;
	png_structp png_ptr;				/* note:  temporary variables! */
	png_infop info_ptr;

	wpnginfo = g_new0(writepng_info, 1);
	if (wpnginfo == NULL)
		return FALSE;

	wpnginfo->outfile = fp;
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, wpnginfo, writepng_error_handler, writepng_warning_handler);
	if (!png_ptr)
	{
		g_free(wpnginfo);
		return FALSE;						/* out of memory */
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		g_free(wpnginfo);
		return FALSE;						/* out of memory */
	}

	wpnginfo->png_ptr = png_ptr;
	wpnginfo->info_ptr = info_ptr;

	if (setjmp(wpnginfo->jmpbuf))
	{
		png_destroy_write_struct(&wpnginfo->png_ptr, &wpnginfo->info_ptr);
		g_free(wpnginfo->memory);
		g_free(wpnginfo->rowbuf);
		g_free(wpnginfo);
		return FALSE;
	}

	png_init_io(png_ptr, wpnginfo->outfile);
	
	return png_write_pic(wpnginfo, src, pic);
}


static void PNGCBAPI image_memory_flush(png_structp png_ptr)
{
	UNUSED(png_ptr);
}


static void PNGCBAPI image_memory_write(png_structp png_ptr, png_bytep data, size_t size)
{
	writepng_info *wpnginfo = (writepng_info *)png_get_io_ptr(png_ptr);
	size_t ob = wpnginfo->output_bytes;

	/* Check for overflow; this should never happen: */
	if (size <= ((png_alloc_size_t)-1) - ob)
	{
		/* I don't think libpng ever does this, but just in case: */
		if (size > 0)
		{
			if (wpnginfo->memory_bytes >= ob + size) /* writing */
				memcpy(wpnginfo->memory + ob, data, size);

			wpnginfo->output_bytes += size;
		}
	} else
	{
		png_error(png_ptr, "png_image_write_to_memory: PNG too big");
	}
}


static void PNGCBAPI image_memory_read(png_structp png_ptr, png_bytep data, size_t size)
{
	writepng_info *rpnginfo = (writepng_info *)png_get_io_ptr(png_ptr);
	size_t read_count = rpnginfo->output_bytes;

	while (size > 0)
	{
		size_t avail;

		if (read_count >= rpnginfo->memory_bytes)
		{
			break;
		}
		avail = rpnginfo->memory_bytes - read_count;
		if (avail > size)
			avail = size;

		memcpy(data, rpnginfo->memory + read_count, avail);
		read_count += avail;
		size -= avail;
		data += avail;
	}

	rpnginfo->output_bytes = read_count;
}


unsigned char *png_pack(const unsigned char *src, PICTURE *pic)
{
	writepng_info *wpnginfo;
	png_structp png_ptr;				/* note:  temporary variables! */
	png_infop info_ptr;

	wpnginfo = g_new0(writepng_info, 1);
	if (wpnginfo == NULL)
		return FALSE;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, wpnginfo, writepng_error_handler, writepng_warning_handler);
	if (!png_ptr)
	{
		g_free(wpnginfo);
		return FALSE;						/* out of memory */
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		g_free(wpnginfo);
		return FALSE;						/* out of memory */
	}

	wpnginfo->png_ptr = png_ptr;
	wpnginfo->info_ptr = info_ptr;

	if (setjmp(wpnginfo->jmpbuf))
	{
		png_destroy_write_struct(&wpnginfo->png_ptr, &wpnginfo->info_ptr);
		g_free(wpnginfo->memory);
		g_free(wpnginfo->rowbuf);
		g_free(wpnginfo);
		pic->pi_buf = NULL;
		return FALSE;
	}

	png_set_write_fn(png_ptr, wpnginfo, image_memory_write, image_memory_flush);
	
	if (png_write_pic(wpnginfo, src, pic) == FALSE)
		return NULL;
	return pic->pi_buf;
}


gboolean png_unpack(unsigned char *dest, const unsigned char *src, PICTURE *pic, gboolean with_mask)
{
	writepng_info *rpnginfo;
	png_structp png_ptr;				/* note:  temporary variables! */
	png_infop info_ptr;
	png_int_t color_type;
	size_t srcrowbytes;
	size_t dstrowbytes;
	int pass, passes;
	png_uint_32 y;
	short j, k, l;
	const unsigned char *rp;
	unsigned short mask;
	unsigned char *gp;
	unsigned char color;
	unsigned char *deststart;
	
	rpnginfo = g_new0(writepng_info, 1);
	if (rpnginfo == NULL)
		return FALSE;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, rpnginfo, writepng_error_handler, writepng_warning_handler);
	if (!png_ptr)
	{
		g_free(rpnginfo);
		return FALSE;						/* out of memory */
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		g_free(rpnginfo->rowbuf);
		g_free(rpnginfo);
		return FALSE;						/* out of memory */
	}

	rpnginfo->png_ptr = png_ptr;
	rpnginfo->info_ptr = info_ptr;
	rpnginfo->memory = (unsigned char *)NO_CONST(src);
	rpnginfo->memory_bytes = pic->pi_filesize;
	deststart = dest;

	if (setjmp(rpnginfo->jmpbuf))
	{
		png_destroy_read_struct(&rpnginfo->png_ptr, &rpnginfo->info_ptr, NULL);
		g_free(rpnginfo->rowbuf);
		g_free(rpnginfo);
		return FALSE;
	}

	png_set_read_fn(png_ptr, rpnginfo, image_memory_read);
	
	png_read_info(png_ptr, info_ptr);
	srcrowbytes = png_get_rowbytes(png_ptr, info_ptr);
	
	rpnginfo->width = png_get_image_width(png_ptr, info_ptr);
	rpnginfo->height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	rpnginfo->interlaced = png_get_interlace_type(png_ptr, info_ptr);
	pic->pi_width = rpnginfo->width;
	pic->pi_height = rpnginfo->height;
	
	if (!with_mask && (color_type & PNG_COLOR_MASK_ALPHA))
		png_set_strip_alpha(png_ptr);

#ifdef PNG_READ_INTERLACING_SUPPORTED
	passes = png_set_interlace_handling(png_ptr);
#else
	passes = png_get_interlace_type(png_ptr, info_ptr) == PNG_INTERLACE_ADAM7 ? PNG_INTERLACE_ADAM7_PASSES : 1;
#endif

	rpnginfo->rowbuf = g_new(unsigned char, srcrowbytes);
	dstrowbytes = pic_rowsize(pic, pic->pi_planes);
	
	png_start_read_image(png_ptr);
	
	for (pass = 0; pass < passes; ++pass)
	{
		dest = deststart;
		y = rpnginfo->height;
#ifndef PNG_READ_INTERLACING_SUPPORTED
		if (passes == PNG_INTERLACE_ADAM7_PASSES)
			y = PNG_PASS_ROWS(y, pass);
#endif
		while (y--)
		{
			png_read_row(png_ptr, rpnginfo->rowbuf, NULL);
			switch (pic->pi_planes)
			{
			case 8:
				rp = rpnginfo->rowbuf;
				gp = dest;
				for (k = pic->pi_width; k > 0; k -= 16)
				{
					for (j = 0, mask = 0x80; j < 8 && j < k; j++, mask >>= 1)
					{
						color = rp[j];
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
							color = rp[j];
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
				dest += dstrowbytes;
				break;

			case 4:
				rp = rpnginfo->rowbuf;
				gp = dest;
				for (k = pic->pi_width; k > 0; k -= 16)
				{
					short lim = (k + 1) / 2;
					
					for (j = 0, mask = 0x80; j < 4 && j < lim; j++, mask >>= 2)
					{
						color = (rp[j] >> 4) & 0x0f;
						if (color & 0x01) gp[0] |= mask;
						if (color & 0x02) gp[2] |= mask;
						if (color & 0x04) gp[4] |= mask;
						if (color & 0x08) gp[6] |= mask;
					}
					if (k > 8)
					{
						for (mask = 0x80; j < 8 && j < lim; j++, mask >>= 2)
						{
							color = (rp[j] >> 4) & 0x0f;
							if (color & 0x01) gp[1] |= mask;
							if (color & 0x02) gp[3] |= mask;
							if (color & 0x04) gp[5] |= mask;
							if (color & 0x08) gp[7] |= mask;
						}
					}

					for (j = 0, mask = 0x40; j < 4 && j < lim; j++, mask >>= 2)
					{
						color = rp[j] & 0x0f;
						if (color & 0x01) gp[0] |= mask;
						if (color & 0x02) gp[2] |= mask;
						if (color & 0x04) gp[4] |= mask;
						if (color & 0x08) gp[6] |= mask;
					}
					if (k > 8)
					{
						for (mask = 0x40; j < 8 && j < lim; j++, mask >>= 2)
						{
							color = rp[j] & 0x0f;
							if (color & 0x01) gp[1] |= mask;
							if (color & 0x02) gp[3] |= mask;
							if (color & 0x04) gp[5] |= mask;
							if (color & 0x08) gp[7] |= mask;
						}
					}
					gp += 8;
					rp += 8;
				}
				dest += dstrowbytes;
				break;
		
			case 1:
				rp = rpnginfo->rowbuf;
				k = (short)((pic->pi_width + 7) >> 3);
				for (l = 0; l < k; l++)
				{
					dest[l] = ~rp[l];
				}
				for (; l < (short)dstrowbytes; l++)
					dest[l] = 0xff;
				dest += dstrowbytes;
				break;
			}
		}
	}

	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&rpnginfo->png_ptr, &rpnginfo->info_ptr, NULL);
	g_free(rpnginfo->rowbuf);
	g_free(rpnginfo);
		
	return TRUE;
}


long png_rowsize(PICTURE *pic, _WORD planes)
{
	return (_LONG)(((pic->pi_width) + 7) >> 3) * (_LONG)planes;
}

#endif /* HAVE_PNG */
