/*
 * << Haru Free PDF Library >> -- hpdf_gstate.c
 *
 * URL: http://libharu.org
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 * Copyright (c) 2007-2009 Antony Dovgal <tony@daylessday.org>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */

#include "hpdf/conf.h"
#include "hpdf/utils.h"
#include "hpdf/gstate.h"

HPDF_GState HPDF_GState_New(HPDF_MMgr mmgr, HPDF_GState current)
{
	HPDF_GState gstate;

	if (current && current->depth >= HPDF_LIMIT_MAX_GSTATE)
	{
		HPDF_SetError(mmgr->error, HPDF_EXCEED_GSTATE_LIMIT, 0);

		return NULL;
	}

	gstate = (HPDF_GState) HPDF_GetMem(mmgr, sizeof(HPDF_GState_Rec));
	if (!gstate)
		return NULL;

	if (current)
	{
		gstate->trans_matrix = current->trans_matrix;
		gstate->line_width = current->line_width;
		gstate->line_cap = current->line_cap;
		gstate->line_join = current->line_join;
		gstate->miter_limit = current->miter_limit;
		gstate->dash_mode = current->dash_mode;
		gstate->flatness = current->flatness;

		gstate->char_space = current->char_space;
		gstate->word_space = current->word_space;
		gstate->h_scaling = current->h_scaling;
		gstate->text_leading = current->text_leading;
		gstate->rendering_mode = current->rendering_mode;
		gstate->text_rise = current->text_rise;

		gstate->cs_stroke = current->cs_stroke;
		gstate->cs_fill = current->cs_fill;
		gstate->rgb_fill = current->rgb_fill;
		gstate->rgb_stroke = current->rgb_stroke;
		gstate->cmyk_fill = current->cmyk_fill;
		gstate->cmyk_stroke = current->cmyk_stroke;
		gstate->gray_fill = current->gray_fill;
		gstate->gray_stroke = current->gray_stroke;

		gstate->font = current->font;
		gstate->font_size = current->font_size;
		gstate->writing_mode = current->writing_mode;

		gstate->prev = current;
		gstate->depth = current->depth + 1;
	} else
	{
		gstate->trans_matrix.a = 1;
		gstate->trans_matrix.b = 0;
		gstate->trans_matrix.c = 0;
		gstate->trans_matrix.d = 1;
		gstate->trans_matrix.x = 0;
		gstate->trans_matrix.y = 0;
		gstate->line_width = HPDF_DEF_LINEWIDTH;
		gstate->line_cap = HPDF_DEF_LINECAP;
		gstate->line_join = HPDF_DEF_LINEJOIN;
		gstate->miter_limit = HPDF_DEF_MITERLIMIT;
		gstate->dash_mode.ptn[0] = 0;
		gstate->dash_mode.ptn[1] = 0;
		gstate->dash_mode.ptn[2] = 0;
		gstate->dash_mode.ptn[3] = 0;
		gstate->dash_mode.ptn[4] = 0;
		gstate->dash_mode.ptn[5] = 0;
		gstate->dash_mode.ptn[6] = 0;
		gstate->dash_mode.ptn[7] = 0;
		gstate->dash_mode.num_ptn = 0;
		gstate->dash_mode.phase = 0;
		gstate->flatness = HPDF_DEF_FLATNESS;

		gstate->char_space = HPDF_DEF_CHARSPACE;
		gstate->word_space = HPDF_DEF_WORDSPACE;
		gstate->h_scaling = HPDF_DEF_HSCALING;
		gstate->text_leading = HPDF_DEF_LEADING;
		gstate->rendering_mode = HPDF_DEF_RENDERING_MODE;
		gstate->text_rise = HPDF_DEF_RISE;

		gstate->cs_stroke = HPDF_CS_DEVICE_GRAY;
		gstate->cs_fill = HPDF_CS_DEVICE_GRAY;
		gstate->rgb_fill.r = 0;
		gstate->rgb_fill.g = 0;
		gstate->rgb_fill.b = 0;
		gstate->rgb_stroke.r = 0;
		gstate->rgb_stroke.g = 0;
		gstate->rgb_stroke.b = 0;
		gstate->cmyk_fill.c = 0;
		gstate->cmyk_fill.m = 0;
		gstate->cmyk_fill.y = 0;
		gstate->cmyk_fill.k = 0;
		gstate->cmyk_stroke.c = 0;
		gstate->cmyk_stroke.m = 0;
		gstate->cmyk_stroke.y = 0;
		gstate->cmyk_stroke.k = 0;
		gstate->gray_fill = 0;
		gstate->gray_stroke = 0;

		gstate->font = NULL;
		gstate->font_size = 0;
		gstate->writing_mode = HPDF_WMODE_HORIZONTAL;

		gstate->prev = NULL;
		gstate->depth = 1;
	}

	return gstate;
}


HPDF_GState HPDF_GState_Free(HPDF_MMgr mmgr, HPDF_GState gstate)
{
	HPDF_GState current = NULL;

	if (gstate)
	{
		current = gstate->prev;
		HPDF_FreeMem(mmgr, gstate);
	}

	return current;
}
