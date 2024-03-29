/*
 * << Haru Free PDF Library 2.0.0 >> -- arc_demo.c
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
#include <setjmp.h>
#include "hpdf.h"
#include "grid_sheet.h"

static jmp_buf env;

static void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
	(void) user_data;
	printf("ERROR: error_no=%04X (%s), detail_no=%u\n", (unsigned int) error_no, HPDF_ErrorStr(error_no), (unsigned int) detail_no);
	longjmp(env, 1);
}


int main(int argc, char **argv)
{
	HPDF_Doc pdf;
	HPDF_Page page;
	char fname[256];
	HPDF_Point pos;
	HPDF_REAL xcenter = 100;
	HPDF_REAL ycenter = 100;
	HPDF_REAL xrad = 90;
	HPDF_REAL yrad = 60;
	
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

	/* add a new page object. */
	page = HPDF_AddPage(pdf);

	HPDF_Page_SetHeight(page, 220);
	HPDF_Page_SetWidth(page, 200);

	/* draw grid to the page */
	print_grid(pdf, page);

	/* draw pie chart
	 *
	 *   A: 45% Red
	 *   B: 25% Blue
	 *   C: 15% green
	 *   D: other yellow
	 */

	/* A */
	HPDF_Page_SetRGBFill(page, 1.0, 0, 0);
	HPDF_Page_MoveTo(page, xcenter, ycenter);
	HPDF_Page_LineTo(page, xcenter + xrad, ycenter);
	HPDF_Page_Arc(page, xcenter, ycenter, xrad, yrad, 0, 360 * 0.45);
	HPDF_Page_GetCurrentPos(page, &pos);
	HPDF_Page_LineTo(page, xcenter, ycenter);
	HPDF_Page_Fill(page);

	/* B */
	HPDF_Page_SetRGBFill(page, 0, 0, 1.0);
	HPDF_Page_MoveTo(page, xcenter, ycenter);
	HPDF_Page_LineTo(page, pos.x, pos.y);
	HPDF_Page_Arc(page, xcenter, ycenter, xrad, yrad, 360 * 0.45, 360 * 0.7);
	HPDF_Page_GetCurrentPos(page, &pos);
	HPDF_Page_LineTo(page, xcenter, ycenter);
	HPDF_Page_Fill(page);

	/* C */
	HPDF_Page_SetRGBFill(page, 0, 1.0, 0);
	HPDF_Page_MoveTo(page, xcenter, ycenter);
	HPDF_Page_LineTo(page, pos.x, pos.y);
	HPDF_Page_Arc(page, xcenter, ycenter, xrad, yrad, 360 * 0.7, 360 * 0.85);
	HPDF_Page_GetCurrentPos(page, &pos);
	HPDF_Page_LineTo(page, xcenter, ycenter);
	HPDF_Page_Fill(page);

	/* D */
	HPDF_Page_SetRGBFill(page, 1.0, 1.0, 0);
	HPDF_Page_MoveTo(page, xcenter, ycenter);
	HPDF_Page_LineTo(page, pos.x, pos.y);
	HPDF_Page_Arc(page, xcenter, ycenter, xrad, yrad, 360 * 0.85, 360);
	HPDF_Page_GetCurrentPos(page, &pos);
	HPDF_Page_LineTo(page, xcenter, ycenter);
	HPDF_Page_Fill(page);

	/* draw center circle */
	HPDF_Page_SetGrayStroke(page, 0);
	HPDF_Page_SetGrayFill(page, 1);
	HPDF_Page_Circle(page, xcenter, ycenter, 30);
	HPDF_Page_Fill(page);

	/* save the document to a file */
	HPDF_SaveToFile(pdf, fname);

	/* clean up */
	HPDF_Free(pdf);

	return 0;
}
