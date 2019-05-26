/*---------------------------------------------------------------------------

   wpng - simple PNG-writing program                             writepng.h

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

#ifndef __WRITEPNG_H__
#define __WRITEPNG_H__ 1

#include <png.h>
#include <setjmp.h>

#ifndef TRUE
#  define TRUE 1
#  define FALSE 0
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) > (b)? (a) : (b))
#  define MIN(a,b)  ((a) < (b)? (a) : (b))
#endif

typedef struct _writepng_info {
	double gamma;
	long width;
	long height;
	unsigned long rowbytes;
	time_t modtime;
	FILE *outfile;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned char *image_data;
	unsigned char **row_pointers;
	char *title;
	char *author;
	char *desc;
	char *copyright;
	char *email;
	char *url;
	int sample_depth;
	int bpp;
	int interlaced;
	int have_bg;
	jmp_buf jmpbuf;
	int num_palette;
	png_color palette[PNG_MAX_PALETTE_LENGTH];
	png_color bg;
} writepng_info;


/* prototypes for public functions in writepng.c */

void writepng_version_info(void);

writepng_info *writepng_new(void);
int writepng_init(writepng_info *wpnginfo);
void writepng_exit(writepng_info *wpnginfo);

int writepng_output(writepng_info *wpnginfo);

int writepng_encode_image(writepng_info *wpnginfo);

int writepng_encode_row(writepng_info *wpnginfo, unsigned char *row_data);

int writepng_encode_finish(writepng_info *wpnginfo);

void writepng_cleanup(writepng_info *wpnginfo);

#endif /* __WRITEPNG_H__ */
