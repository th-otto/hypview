/*
 * << Haru Free PDF Library 2.0.0 >> -- text_demo.c
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>
#include "hpdf.h"
#include "grid_sheet.h"

static jmp_buf env;

static unsigned char const pattern_data[] = {
 0xff,0xff,0x80,0x80,0x80,0x80,0x80,0x80,0xff,0xff,0x08,0x08,0x08,0x08,0x08,0x08,
 0xff,0xff,0x80,0x80,0x80,0x80,0x80,0x80,0xff,0xff,0x08,0x08,0x08,0x08,0x08,0x08,
};

static void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
	(void) user_data;
	printf("ERROR: error_no=%04X (%s), detail_no=%u\n", (unsigned int) error_no, HPDF_ErrorStr(error_no), (unsigned int) detail_no);
	longjmp(env, 1);
}

int main(int argc, char **argv)
{
	HPDF_Doc pdf;
	HPDF_Font font;
	HPDF_Page page;
	char fname[256];
	HPDF_Image pattern_image;
	HPDF_Array array;
	HPDF_PatternColorspace colorspace;
	HPDF_Pattern pattern;
	
	(void) argc;
	strcpy(fname, argv[0]);
	strcat(fname, ".pdf");

	pdf = HPDF_New(error_handler, 0, 0, 0, NULL);
	if (!pdf)
	{
		printf("error: cannot create PdfDoc object\n");
		return 1;
	}

	if (setjmp(env))
	{
		HPDF_Free(pdf);
		return 1;
	}

	/* set compression mode */
	HPDF_SetCompressionMode(pdf, HPDF_COMP_NONE);

	/* create default-font */
	font = HPDF_GetFont(pdf, "Times-Bold", NULL);

	/* add a new page object. */
	page = HPDF_AddPage(pdf);

	HPDF_Page_SetFontAndSize(page, font, 25);

	HPDF_Page_SetRGBFill(page, 1.0, 0.0, 0.0);
	HPDF_Page_Rectangle(page, 36, 763.08, 322.23, 22.5);
	HPDF_Page_Fill(page);
	
	HPDF_Page_BeginText(page);
	HPDF_Page_MoveTextPos(page, 36, 768.5);
	HPDF_Page_SetRGBFill(page, 1.0, 1.0, 1.0);
	HPDF_Page_ShowText(page, "White text on red background");
	HPDF_Page_EndText(page);
	
	pattern_image = HPDF_Image_LoadRawImageFromMem(pdf->mmgr, pattern_data, pdf->xref, 16, 16, HPDF_CS_DEVICE_GRAY, 1);
	HPDF_Image_SetMask(pattern_image, HPDF_TRUE);
	array = HPDF_Array_New(pattern_image->mmgr);
	HPDF_Dict_Add(pattern_image, "Decode", array);
	HPDF_Array_Add(array, HPDF_Number_New(pattern_image->mmgr, 1));
	HPDF_Array_Add(array, HPDF_Number_New(pattern_image->mmgr, 0));

	pattern = HPDF_Pattern_New(page->mmgr, pdf->xref, HPDF_PATTERN_TYPE_TILED, HPDF_PAINT_TYPE_UNCOLORED, HPDF_TILING_TYPE_NO_DISTORTION, pattern_image);
	colorspace = HPDF_PatternColorspace_New(page->mmgr, HPDF_CS_DEVICE_RGB);
	
#if 1
	HPDF_Page_Rectangle(page, 50, 400, 200, 200);
	HPDF_Page_SetColorspaceFill(page, colorspace);
	HPDF_Page_SetPatternFill(page, pattern, 0.0, 0.0, 1.0);
	HPDF_Page_Fill(page);
#else
	HPDF_Page_SetRGBFill(page, 0.0, 0.0, 1.0);
	HPDF_Page_DrawImage(page, pattern_image, 50, 400, 200, 200);
#endif

	/* save the document to a file */
	HPDF_SaveToFile(pdf, fname);

	/* clean up */
	HPDF_Free(pdf);

	(void)pattern;
	(void)colorspace;

	return 0;
}
