/*---------------------------------------------------------------------------

   wpng - simple PNG-writing program                             writepng.c

  ---------------------------------------------------------------------------

      Copyright (c) 1998-2007 Greg Roelofs.  All rights reserved.

      This software is provided "as is," without warranty of any kind,
      express or implied.  In no event shall the author or contributors
      be held liable for any damages arising in any way from the use of
      this software.

      The contents of this file are DUAL-LICENSED.  You may modify and/or
      redistribute this software according to the terms of one of the
      following two licenses (at your option):


      LICENSE 1 ("BSD-like with advertising clause"):

      Permission is granted to anyone to use this software for any purpose,
      including commercial applications, and to alter it and redistribute
      it freely, subject to the following restrictions:

      1. Redistributions of source code must retain the above copyright
         notice, disclaimer, and this list of conditions.
      2. Redistributions in binary form must reproduce the above copyright
         notice, disclaimer, and this list of conditions in the documenta-
         tion and/or other materials provided with the distribution.
      3. All advertising materials mentioning features or use of this
         software must display the following acknowledgment:

            This product includes software developed by Greg Roelofs
            and contributors for the book, "PNG: The Definitive Guide,"
            published by O'Reilly and Associates.


      LICENSE 2 (GNU GPL v2 or later):

      This program is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 2 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program; if not, write to the Free Software Foundation,
      Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  ---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "writepng.h"					/* typedefs, common macros, public prototypes */
#include "debug.h"
#include "zlib.h"

#ifndef NO_CONST
#  ifdef __GNUC__
#	 define NO_CONST(p) __extension__({ union { const void *cs; void *s; } x; x.cs = p; x.s; })
#  else
#	 define NO_CONST(p) ((void *)(p))
#  endif
#endif


static void writepng_error_handler(png_structp png_ptr, png_const_charp msg)
{
	writepng_info *wpnginfo;

	(void) msg;
	
	/* This function, aside from the extra step of retrieving the "error
	 * pointer" (below) and the fact that it exists within the application
	 * rather than within libpng, is essentially identical to libpng's
	 * default error handler.  The second point is critical:  since both
	 * setjmp() and longjmp() are called from the same code, they are
	 * guaranteed to have compatible notions of how big a jmp_buf is,
	 * regardless of whether _BSD_SOURCE or anything else has (or has not)
	 * been defined. */

	wpnginfo = (writepng_info *)png_get_error_ptr(png_ptr);
	if (wpnginfo == NULL)
	{									/* we are completely hosed now */
		errout("writepng severe error:  jmpbuf not recoverable; terminating.\n");
		fflush(stderr);
		exit(99);
	}

	/* Now we have our data structure we can use the information in it
	 * to return control to our own higher level code (all the points
	 * where 'setjmp' is called in this file.)  This will work with other
	 * error handling mechanisms as well - libpng always calls png_error
	 * when it can proceed no further, thus, so long as the error handler
	 * is intercepted, application code can do its own error recovery.
	 */
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



void writepng_version_info(void)
{
	errout("   Compiled with libpng %s; using libpng %s.\n", PNG_LIBPNG_VER_STRING, png_libpng_ver);
	errout("   Compiled with zlib %s; using zlib %s.\n", ZLIB_VERSION, zlib_version);
}






writepng_info *writepng_new(void)
{
	writepng_info *wpnginfo = (writepng_info *)malloc(sizeof(*wpnginfo));
	
	if (wpnginfo == NULL)
		return NULL;
	memset(wpnginfo, 0, sizeof(*wpnginfo));

	wpnginfo->outfile = NULL;
	wpnginfo->image_data = NULL;
	wpnginfo->row_pointers = NULL;
	wpnginfo->interlaced = FALSE;
	wpnginfo->have_bg = -1;
	wpnginfo->gamma = 0.0;
	wpnginfo->sample_depth = 8;
	
	return wpnginfo;
}


void writepng_exit(writepng_info *wpnginfo)
{
	writepng_cleanup(wpnginfo);
	free(wpnginfo);
}


int writepng_output(writepng_info *wpnginfo)
{
	unsigned long srcrowbytes;
	int rc;
	
	if (wpnginfo == NULL || wpnginfo->image_data == NULL || wpnginfo->outfile == NULL)
		return EINVAL;
	if ((rc = writepng_init(wpnginfo)) != 0)
		return rc;

	/* calculate rowbytes on basis of image type; note that this becomes much
	 * more complicated if we choose to support PBM type, ASCII PNM types, or
	 * 16-bit-per-sample binary data [currently not an official NetPBM type] */

	srcrowbytes = wpnginfo->rowbytes;
	if (srcrowbytes == 0)
	{
		if (wpnginfo->bpp <= 8)
			srcrowbytes = wpnginfo->width;
		else if (wpnginfo->bpp == 24)
			srcrowbytes = wpnginfo->width * 3;
		else
			srcrowbytes = wpnginfo->width * 4;
	}
	
	rc = 0;
	if (wpnginfo->interlaced)
	{
		long y;

		wpnginfo->row_pointers = (unsigned char **) malloc(wpnginfo->height * sizeof(unsigned char *));
		if (wpnginfo->row_pointers == NULL)
		{
			writepng_cleanup(wpnginfo);
			return errno;
		}
		for (y = 0; y < wpnginfo->height; ++y)
			wpnginfo->row_pointers[y] = wpnginfo->image_data + y * srcrowbytes;
		rc = writepng_encode_image(wpnginfo);
	} else								/* not interlaced:  write progressively (row by row) */
	{
		long y;
		unsigned char *image_data = wpnginfo->image_data;
		
		for (y = wpnginfo->height; y > 0; --y)
		{
			if ((rc = writepng_encode_row(wpnginfo, image_data)) != 0)
			{
				break;
			}
			image_data += srcrowbytes;
		}
		if (rc == 0) 
		{
			rc = writepng_encode_finish(wpnginfo);
		}
	}

	writepng_cleanup(wpnginfo);

	return rc;
}


/* returns 0 for success, 2 for libpng problem, 4 for out of memory, 11 for
 *  unexpected image type; note that outfile might be stdout */

int writepng_init(writepng_info *wpnginfo)
{
	png_structp png_ptr;				/* note:  temporary variables! */
	png_infop info_ptr;
	int color_type, interlace_type;


	/* could also replace libpng warning-handler (final NULL), but no need: */

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, wpnginfo, writepng_error_handler, writepng_warning_handler);
	if (!png_ptr)
		return ENOMEM;						/* out of memory */

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		return ENOMEM;						/* out of memory */
	}

	/* make sure we save our pointers for use in writepng_encode_image() */

	wpnginfo->png_ptr = png_ptr;
	wpnginfo->info_ptr = info_ptr;


	/* setjmp() must be called in every function that calls a PNG-writing
	 * libpng function, unless an alternate error handler was installed--
	 * but compatible error handlers must either use longjmp() themselves
	 * (as in this program) or some other method to return control to
	 * application code, so here we go: */

	if (setjmp(wpnginfo->jmpbuf))
	{
		png_destroy_write_struct(&wpnginfo->png_ptr, &wpnginfo->info_ptr);
		return EFAULT;
	}

	/* make sure outfile is (re)opened in BINARY mode */

	png_init_io(png_ptr, wpnginfo->outfile);


	/* set the compression levels--in general, always want to leave filtering
	 * turned on (except for palette images) and allow all of the filters,
	 * which is the default; want 32K zlib window, unless entire image buffer
	 * is 16K or smaller (unknown here)--also the default; usually want max
	 * compression (NOT the default); and remaining compression flags should
	 * be left alone */

	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	/* set the image parameters appropriately */

	if (wpnginfo->bpp <= 8)
	{
		color_type = wpnginfo->num_palette > 2 ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_GRAY;
	} else if (wpnginfo->bpp == 24)
	{
		color_type = PNG_COLOR_TYPE_RGB;
	} else if (wpnginfo->bpp == 32)
	{
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
	} else
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return EINVAL;
	}

	interlace_type = wpnginfo->interlaced ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE;

	png_set_IHDR(png_ptr, info_ptr, wpnginfo->width, wpnginfo->height,
				 wpnginfo->sample_depth, color_type, interlace_type,
				 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	if (wpnginfo->gamma > 0.0)
		png_set_gAMA(png_ptr, info_ptr, wpnginfo->gamma);

	if (wpnginfo->num_palette && color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_PLTE(png_ptr, info_ptr, wpnginfo->palette, wpnginfo->num_palette);
	}
	
	if (wpnginfo->bpp > 8 && wpnginfo->have_bg >= 0)
	{									/* we know it's RGBA, not gray+alpha */
		png_color_16 background;

		background.red = wpnginfo->bg.red;
		background.green = wpnginfo->bg.green;
		background.blue = wpnginfo->bg.blue;
		png_set_bKGD(png_ptr, info_ptr, &background);
	}

	if (wpnginfo->bpp <= 8 && wpnginfo->have_bg >= 0)
	{
		png_color_16 background;
		png_byte trans;
		
		background.red = wpnginfo->bg.red;
		background.green = wpnginfo->bg.green;
		background.blue = wpnginfo->bg.blue;
		trans = wpnginfo->have_bg;
		png_set_tRNS(png_ptr, info_ptr, &trans, 1, &background);
	}

	if (wpnginfo->modtime != 0)
	{
		png_time modtime;

		png_convert_from_time_t(&modtime, wpnginfo->modtime);
		png_set_tIME(png_ptr, info_ptr, &modtime);
	}

	{
		png_text text[6];
		int num_text = 0;

		if (wpnginfo->title)
		{
			text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
			text[num_text].key = (png_charp)NO_CONST("Title");
			text[num_text].text = wpnginfo->title;
			++num_text;
		}
		if (wpnginfo->author)
		{
			text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
			text[num_text].key = (png_charp)NO_CONST("Author");
			text[num_text].text = wpnginfo->author;
			++num_text;
		}
		if (wpnginfo->desc)
		{
			text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
			text[num_text].key = (png_charp)NO_CONST("Description");
			text[num_text].text = wpnginfo->desc;
			++num_text;
		}
		if (wpnginfo->copyright)
		{
			text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
			text[num_text].key = (png_charp)NO_CONST("Copyright");
			text[num_text].text = wpnginfo->copyright;
			++num_text;
		}
		if (wpnginfo->email)
		{
			text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
			text[num_text].key = (png_charp)NO_CONST("E-mail");
			text[num_text].text = wpnginfo->email;
			++num_text;
		}
		if (wpnginfo->url)
		{
			text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
			text[num_text].key = (png_charp)NO_CONST("URL");
			text[num_text].text = wpnginfo->url;
			++num_text;
		}
		if (num_text != 0)
			png_set_text(png_ptr, info_ptr, text, num_text);
	}


	/* write all chunks up to (but not including) first IDAT */

	png_write_info(png_ptr, info_ptr);

	/* if we wanted to write any more text info *after* the image data, we
	 * would set up text struct(s) here and call png_set_text() again, with
	 * just the new data; png_set_tIME() could also go here, but it would
	 * have no effect since we already called it above (only one tIME chunk
	 * allowed) */


	/* set up the transformations:  for now, just pack low-bit-depth pixels
	 * into bytes (one, two or four pixels per byte) */

	png_set_packing(png_ptr);
/*  png_set_shift(png_ptr, &sig_bit);  to scale low-bit-depth values */


	/* OK, that's all we need to do for now; return happy */

	return 0;
}



/* returns 0 for success, 2 for libpng (longjmp) problem */

int writepng_encode_image(writepng_info *wpnginfo)
{
	png_structp png_ptr = (png_structp) wpnginfo->png_ptr;
	png_infop info_ptr = (png_infop) wpnginfo->info_ptr;


	/* as always, setjmp() must be called in every function that calls a
	 * PNG-writing libpng function */

	if (setjmp(wpnginfo->jmpbuf))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		wpnginfo->png_ptr = NULL;
		wpnginfo->info_ptr = NULL;
		return EFAULT;
	}


	/* and now we just write the whole image; libpng takes care of interlacing
	 * for us */

	png_write_image(png_ptr, wpnginfo->row_pointers);


	/* since that's it, we also close out the end of the PNG file now--if we
	 * had any text or time info to write after the IDATs, second argument
	 * would be info_ptr, but we optimize slightly by sending NULL pointer: */

	png_write_end(png_ptr, NULL);

	return 0;
}





/* returns 0 if succeeds, 2 if libpng problem */

int writepng_encode_row(writepng_info *wpnginfo, unsigned char *row_data)
{
	png_structp png_ptr = (png_structp) wpnginfo->png_ptr;
	png_infop info_ptr = (png_infop) wpnginfo->info_ptr;

	/* as always, setjmp() must be called in every function that calls a
	 * PNG-writing libpng function */

	if (setjmp(wpnginfo->jmpbuf))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		wpnginfo->png_ptr = NULL;
		wpnginfo->info_ptr = NULL;
		return EFAULT;
	}

	/* image_data points at our one row of image data */

	png_write_row(png_ptr, row_data);

	return 0;
}





/* returns 0 if succeeds, 2 if libpng problem */

int writepng_encode_finish(writepng_info *wpnginfo)	/* NON-interlaced! */
{
	png_structp png_ptr = (png_structp) wpnginfo->png_ptr;
	png_infop info_ptr = (png_infop) wpnginfo->info_ptr;


	/* as always, setjmp() must be called in every function that calls a
	 * PNG-writing libpng function */

	if (setjmp(wpnginfo->jmpbuf))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		wpnginfo->png_ptr = NULL;
		wpnginfo->info_ptr = NULL;
		return EFAULT;
	}


	/* close out PNG file; if we had any text or time info to write after
	 * the IDATs, second argument would be info_ptr: */

	png_write_end(png_ptr, NULL);

	return 0;
}





void writepng_cleanup(writepng_info *wpnginfo)
{
	if (wpnginfo)
	{
		if (wpnginfo->row_pointers)
		{
			free(wpnginfo->row_pointers);
			wpnginfo->row_pointers = NULL;
		}
		if (wpnginfo->png_ptr && wpnginfo->info_ptr)
		{
			png_destroy_write_struct(&wpnginfo->png_ptr, &wpnginfo->info_ptr);
			wpnginfo->png_ptr = NULL;
			wpnginfo->info_ptr = NULL;
		}
	}
}
