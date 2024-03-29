/*
 * << Haru Free PDF Library >> -- hpdf_page_operator.c
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
#include "hpdf/pages.h"
#include "hpdf.h"
#include <string.h>

#if defined(__PUREC__) || defined(__APPLE__)
# define PAGEOP_ALL
#endif


/*--- General graphics state ---------------------------------------------*/

#if defined(PAGEOP_ALL) || defined(PAGE_SETLINEWIDTH)
/*
 lineWidth w
 Set the line width in the graphics state
 */
HPDF_STATUS HPDF_Page_SetLineWidth(HPDF_Page page, HPDF_REAL line_width)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_TEXT_OBJECT | HPDF_GMODE_PATH_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (line_width < 0)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	if (HPDF_Stream_WriteReal(attr->stream, line_width) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " w\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->line_width = line_width;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETLINECAP)
/*
 lineCap J
 Set the line cap style in the graphics state 
 */
HPDF_STATUS HPDF_Page_SetLineCap(HPDF_Page page, HPDF_LineCap line_cap)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	if (line_cap >= HPDF_LINECAP_EOF)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, line_cap);

	attr = (HPDF_PageAttr) page->attr;

	if ((ret = HPDF_Stream_WriteInt(attr->stream, (HPDF_UINT) line_cap)) != HPDF_OK)
		return ret;

	if ((ret = HPDF_Stream_WriteStr(attr->stream, " J\012")) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->line_cap = line_cap;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETLINEJOIN)
/*
 lineJoin j
 Set the line join style in the graphics state
 */
HPDF_STATUS HPDF_Page_SetLineJoin(HPDF_Page page, HPDF_LineJoin line_join)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	if (line_join >= HPDF_LINEJOIN_EOF)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, line_join);

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteInt(attr->stream, (HPDF_UINT) line_join) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " j\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->line_join = line_join;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETMITERLIMIT)
/*
 miterLimit M
 Set the miter limit in the graphics state
 */
HPDF_STATUS HPDF_Page_SetMiterLimit(HPDF_Page page, HPDF_REAL miter_limit)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (miter_limit < 1)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	if (HPDF_Stream_WriteReal(attr->stream, miter_limit) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " M\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->miter_limit = miter_limit;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETDASH)
/*
 dashArray dashPhase d
 Set the line dash pattern in the graphics state
 */
HPDF_STATUS HPDF_Page_SetDash(HPDF_Page page, const HPDF_UINT16 *dash_ptn, HPDF_UINT num_param, HPDF_UINT phase)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_TEXT_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	const HPDF_UINT16 *pdash_ptn = dash_ptn;
	HPDF_PageAttr attr;
	HPDF_UINT i;

	static const HPDF_DashMode INIT_MODE = { {0, 0, 0, 0, 0, 0, 0, 0}, 0, 0 };

	if (ret != HPDF_OK)
		return ret;

	if ((num_param != 1 && (num_param / 2) * 2 != num_param) || num_param > 8)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_PARAM_COUNT, num_param);

	if (num_param == 0 && phase > 0)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, phase);

	if (!dash_ptn && num_param > 0)
		return HPDF_RaiseError(page->error, HPDF_INVALID_PARAMETER, phase);

	*pbuf++ = '[';

	for (i = 0; i < num_param; i++)
	{
#if 0
		if (*pdash_ptn == 0 || *pdash_ptn > HPDF_MAX_DASH_PATTERN)
			return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);
#endif

		pbuf = HPDF_IToA(pbuf, *pdash_ptn, eptr);
		*pbuf++ = ' ';
		pdash_ptn++;
	}

	*pbuf++ = ']';
	*pbuf++ = ' ';

	pbuf = HPDF_IToA(pbuf, phase, eptr);
	HPDF_StrCpy(pbuf, " d\012", eptr);

	attr = (HPDF_PageAttr) page->attr;

	if ((ret = HPDF_Stream_WriteStr(attr->stream, buf)) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->dash_mode = INIT_MODE;
	attr->gstate->dash_mode.num_ptn = num_param;
	attr->gstate->dash_mode.phase = phase;

	pdash_ptn = dash_ptn;
	for (i = 0; i < num_param; i++)
	{
		attr->gstate->dash_mode.ptn[i] = *pdash_ptn;
		pdash_ptn++;
	}

	return ret;
}
#endif


/*
 intent ri
 Set the color rendering intent in the graphics state
 --not implemented yet
 */

#if defined(PAGEOP_ALL) || defined(PAGE_SETFLAT)
/*
 flatness i
 Set the flatness tolerance in the graphics state
 */
HPDF_STATUS HPDF_Page_SetFlat(HPDF_Page page, HPDF_REAL flatness)
{
	HPDF_PageAttr attr;
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_TEXT_OBJECT);

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (flatness > 100 || flatness < 0)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	if (HPDF_Stream_WriteReal(attr->stream, flatness) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " i\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->flatness = flatness;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETEXTGSTATE)
/*
 dictName gs
 Set the specified parameters in the graphics state
 */
HPDF_STATUS HPDF_Page_SetExtGState(HPDF_Page page, HPDF_ExtGState ext_gstate)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION);
	HPDF_PageAttr attr;
	const char *local_name;

	if (ret != HPDF_OK)
		return ret;

	if (!HPDF_ExtGState_Validate(ext_gstate))
		return HPDF_RaiseError(page->error, HPDF_INVALID_OBJECT, 0);

	if (page->mmgr != ext_gstate->mmgr)
		return HPDF_RaiseError(page->error, HPDF_INVALID_EXT_GSTATE, 0);

	attr = (HPDF_PageAttr) page->attr;
	local_name = HPDF_Page_GetExtGStateName(page, ext_gstate);

	if (!local_name)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteEscapeName(attr->stream, local_name) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " gs\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	/* change objct class to read only. */
	ext_gstate->header.obj_class = HPDF_OSUBCLASS_EXT_GSTATE_R | HPDF_OCLASS_DICT;

	return ret;
}
#endif


/*--- Special graphic state operator --------------------------------------*/

#if defined(PAGEOP_ALL) || defined(PAGE_GSAVE)
/*
 q
 Save the current graphics state on the graphics state stack
 */
HPDF_STATUS HPDF_Page_GSave(HPDF_Page page)
{
	HPDF_GState new_gstate;
	HPDF_PageAttr attr;
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION);

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	new_gstate = HPDF_GState_New(page->mmgr, attr->gstate);
	if (!new_gstate)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, "q\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate = new_gstate;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_GRESTORE)
/*
 Q
 Restore the graphics state by removing the most recently saved
 state from the stack and making it the current state
 */
HPDF_STATUS HPDF_Page_GRestore(HPDF_Page page)
{
	HPDF_GState new_gstate;
	HPDF_PageAttr attr;
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION);

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (!attr->gstate->prev)
		return HPDF_RaiseError(page->error, HPDF_PAGE_CANNOT_RESTORE_GSTATE, 0);

	new_gstate = HPDF_GState_Free(page->mmgr, attr->gstate);

	attr->gstate = new_gstate;

	if (HPDF_Stream_WriteStr(attr->stream, "Q\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_CONCAT)
/*
 a b c d e f cm
 Modify the current transformation matrix (CTM) by concatenating
 the specified matrix
 */
HPDF_STATUS HPDF_Page_Concat(HPDF_Page page, HPDF_REAL a, HPDF_REAL b, HPDF_REAL c, HPDF_REAL d, HPDF_REAL x, HPDF_REAL y)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;
	HPDF_TransMatrix tm;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, a, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, b, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, c, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, d, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	HPDF_StrCpy(pbuf, " cm\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	tm = attr->gstate->trans_matrix;

	attr->gstate->trans_matrix.a = tm.a * a + tm.b * c;
	attr->gstate->trans_matrix.b = tm.a * b + tm.b * d;
	attr->gstate->trans_matrix.c = tm.c * a + tm.d * c;
	attr->gstate->trans_matrix.d = tm.c * b + tm.d * d;
	attr->gstate->trans_matrix.x = tm.x + x * tm.a + y * tm.c;
	attr->gstate->trans_matrix.y = tm.y + x * tm.b + y * tm.d;

	return ret;
}
#endif

/*--- Path construction operator ------------------------------------------*/

#if defined(PAGEOP_ALL) || defined(PAGE_MOVETO)
/*
 x y m
 Begin a new subpath by moving the current point to
 coordinates (x, y), omitting any connecting line segment. If
 the previous path construction operator in the current path
 was also m, the new m overrides it; no vestige of the
 previous m operation remains in the path.
 */
HPDF_STATUS HPDF_Page_MoveTo(HPDF_Page page, HPDF_REAL x, HPDF_REAL y)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, x, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	HPDF_StrCpy(pbuf, " m\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->cur_pos.x = x;
	attr->cur_pos.y = y;
	attr->str_pos = attr->cur_pos;
	attr->gmode = HPDF_GMODE_PATH_OBJECT;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_LINETO)
/*
 x y l
 Append a straight line segment from the current point to the
 point (x, y). The new current point shall be (x, y).
 */
HPDF_STATUS HPDF_Page_LineTo(HPDF_Page page, HPDF_REAL x, HPDF_REAL y)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, x, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	HPDF_StrCpy(pbuf, " l\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->cur_pos.x = x;
	attr->cur_pos.y = y;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_CURVETO)
/*
 x1 y1 x2 y2 x3 y3 c
 Append a cubic Bezier curve to the current path. The curve
 shall extend from the current point to the point (x3, y3), using
 (x1, y1 ) and (x2, y2 ) as the Bezier control points.
 The new current point shall be (x3, y3).
 */
HPDF_STATUS HPDF_Page_CurveTo(HPDF_Page page, HPDF_REAL x1, HPDF_REAL y1, HPDF_REAL x2, HPDF_REAL y2, HPDF_REAL x3, HPDF_REAL y3)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, x1, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y1, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x2, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y2, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x3, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y3, eptr);
	HPDF_StrCpy(pbuf, " c\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->cur_pos.x = x3;
	attr->cur_pos.y = y3;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_CURVETO2)
/*
 x2 y2 x3 y3 v
 Append a cubic Bezier curve to the current path. The curve
 shall extend from the current point to the point (x3, y3), using
 the current point and (x2, y2) as the Bezier control points.
 The new current point shall be (x3, y3).
 */
HPDF_STATUS HPDF_Page_CurveTo2(HPDF_Page page, HPDF_REAL x2, HPDF_REAL y2, HPDF_REAL x3, HPDF_REAL y3)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, x2, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y2, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x3, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y3, eptr);
	HPDF_StrCpy(pbuf, " v\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->cur_pos.x = x3;
	attr->cur_pos.y = y3;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_CURVETO3)
/*
 x1 y1 x3 y3 y
 Append a cubic Bezier curve to the current path. The curve
 shall extend from the current point to the point (x3, y3), using
 (x1, y1) and (x3, y3) as the Bezier control points.
 The new current point shall be (x3, y3 ).
 */
HPDF_STATUS HPDF_Page_CurveTo3(HPDF_Page page, HPDF_REAL x1, HPDF_REAL y1, HPDF_REAL x3, HPDF_REAL y3)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, x1, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y1, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x3, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y3, eptr);
	HPDF_StrCpy(pbuf, " y\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->cur_pos.x = x3;
	attr->cur_pos.y = y3;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_CLOSEPATH)
/*
 h
 Close the current subpath by appending a straight line
 segment from the current point to the starting point of the
 subpath. If the current subpath is already closed, h shall do
 nothing.
 */
HPDF_STATUS HPDF_Page_ClosePath(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "h\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->cur_pos = attr->str_pos;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_RECTANGLE)
/*
 x y width height re
 Append a rectangle to the current path as a complete
 subpath, with lower-left corner (x, y) and dimensions width
 and height in user space.
 */
HPDF_STATUS HPDF_Page_Rectangle(HPDF_Page page, HPDF_REAL x, HPDF_REAL y, HPDF_REAL width, HPDF_REAL height)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, x, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, width, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, height, eptr);
	HPDF_StrCpy(pbuf, " re\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->cur_pos.x = x;
	attr->cur_pos.y = y;
	attr->str_pos = attr->cur_pos;
	attr->gmode = HPDF_GMODE_PATH_OBJECT;

	return ret;
}
#endif


/*--- Path painting operator ---------------------------------------------*/

#if defined(PAGEOP_ALL) || defined(PAGE_STROKE)
/*
 S
 Stroke the path.
 */
HPDF_STATUS HPDF_Page_Stroke(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_CLIPPING_PATH);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "S\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_PAGE_DESCRIPTION;
	attr->cur_pos.x = attr->cur_pos.y = 0;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_CLOSEPATHSTROKE)
/*
 s
 Close and stroke the path. This operator shall have the same effect as the
 sequence h S.
 */
HPDF_STATUS HPDF_Page_ClosePathStroke(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_CLIPPING_PATH);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "s\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_PAGE_DESCRIPTION;
	attr->cur_pos.x = attr->cur_pos.y = 0;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_FILL)
/*
 f
 Fill the path, using the nonzero winding number rule to determine the region
 to fill. Any subpaths that are open shall be implicitly closed before being filled.
 */
HPDF_STATUS HPDF_Page_Fill(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_CLIPPING_PATH);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "f\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_PAGE_DESCRIPTION;
	attr->cur_pos.x = attr->cur_pos.y = 0;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_EOFILL)
/*
 f*
 Fill the path, using the even-odd rule to determine the region to fill.
 */
HPDF_STATUS HPDF_Page_Eofill(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_CLIPPING_PATH);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "f*\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_PAGE_DESCRIPTION;
	attr->cur_pos.x = attr->cur_pos.y = 0;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_FILLSTROKE)
/*
 B
 Fill and then stroke the path, using the nonzero winding number rule to
 determine the region to fill. This operator shall produce the same result as
 constructing two identical path objects, painting the first with f and the
 second with S.
 */
HPDF_STATUS HPDF_Page_FillStroke(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_CLIPPING_PATH);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "B\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_PAGE_DESCRIPTION;
	attr->cur_pos.x = attr->cur_pos.y = 0;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_EOFILLSTROKE)
/*
 B*
 Fill and then stroke the path, using the even-odd rule to determine the region
 to fill. This operator shall produce the same result as B, except that the path
 is filled as if with f* instead of f.
 */
HPDF_STATUS HPDF_Page_EofillStroke(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_CLIPPING_PATH);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "B*\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_PAGE_DESCRIPTION;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_CLOSEPATHFILLSTROKE)
/*
 b
 Fill and then stroke the path, using the even-odd rule to determine the region
 to fill. This operator shall produce the same result as B, except that the path
 is filled as if with f* instead of f.
 */
HPDF_STATUS HPDF_Page_ClosePathFillStroke(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_CLIPPING_PATH);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "b\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_PAGE_DESCRIPTION;
	attr->cur_pos.x = attr->cur_pos.y = 0;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_CLOSEPATHEOFILLSTROKE)
/*
 b*
 Close, fill, and then stroke the path, using the even-odd rule to determine the
 region to fill. This operator shall have the same effect as the sequence h B*.
 */
HPDF_STATUS HPDF_Page_ClosePathEofillStroke(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_CLIPPING_PATH);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "b*\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_PAGE_DESCRIPTION;
	attr->cur_pos.x = attr->cur_pos.y = 0;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_ENDPATH)
/*
 n
 End the path object without filling or stroking it. This operator shall be a path-
 painting no-op, used primarily for the side effect of changing the current
 clipping path.
 */
HPDF_STATUS HPDF_Page_EndPath(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_CLIPPING_PATH);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "n\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_PAGE_DESCRIPTION;
	attr->cur_pos.x = attr->cur_pos.y = 0;

	return ret;
}
#endif


/*--- Clipping paths operator --------------------------------------------*/

#if defined(PAGEOP_ALL) || defined(PAGE_CLIP)
/*
 W
 Modify the current clipping path by intersecting it with the current path, using
 the nonzero winding number rule to determine which regions lie inside the
 clipping path.
 */
HPDF_STATUS HPDF_Page_Clip(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "W\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_CLIPPING_PATH;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_EOCLIP)
/*
 W*
 Modify the current clipping path by intersecting it with the current path, using
 the even-odd rule to determine which regions lie inside the clipping path.
 */
HPDF_STATUS HPDF_Page_Eoclip(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PATH_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "W*\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_CLIPPING_PATH;

	return ret;
}
#endif


/*--- Text object operator -----------------------------------------------*/

#if defined(PAGEOP_ALL) || defined(PAGE_BEGINTEXT)
/*
 BT
 Begin a text object, initializing the text matrix, Tm, and the text line matrix,
 Tlm, to the identity matrix. Text objects shall not be nested; a second BT shall
 not appear before an ET.
 */
HPDF_STATUS HPDF_Page_BeginText(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "BT\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gmode = HPDF_GMODE_TEXT_OBJECT;
	attr->text_pos.x = attr->text_pos.y = 0;
	attr->text_matrix.a = 1;
	attr->text_matrix.b = 0;
	attr->text_matrix.c = 0;
	attr->text_matrix.d = 1;
	attr->text_matrix.x = 0;
	attr->text_matrix.y = 0;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_ENDTEXT)
/*
 ET
 End a text object, discarding the text matrix.
 */
HPDF_STATUS HPDF_Page_EndText(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "ET\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->text_pos.x = attr->text_pos.y = 0;
	attr->gmode = HPDF_GMODE_PAGE_DESCRIPTION;

	return ret;
}
#endif

/*--- Text state ---------------------------------------------------------*/

#if defined(PAGEOP_ALL) || defined(PAGE_SETCHARSPACE)
/*
 Tc
 Set character spacing
 */
HPDF_STATUS HPDF_Page_SetCharSpace(HPDF_Page page, HPDF_REAL value)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (value < HPDF_MIN_CHARSPACE || value > HPDF_MAX_CHARSPACE)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	if (HPDF_Stream_WriteReal(attr->stream, value) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " Tc\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->char_space = value;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETWORDSPACE)
/*
 Tw
 Set word spacing
 */
HPDF_STATUS HPDF_Page_SetWordSpace(HPDF_Page page, HPDF_REAL value)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (value < HPDF_MIN_WORDSPACE || value > HPDF_MAX_WORDSPACE)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	if (HPDF_Stream_WriteReal(attr->stream, value) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " Tw\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->word_space = value;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETHORIZONTALSCALING)
/*
 Tz
 Set horizontal text scaling
 */
HPDF_STATUS HPDF_Page_SetHorizontalScaling(HPDF_Page page, HPDF_REAL value)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (value < HPDF_MIN_HORIZONTALSCALING || value > HPDF_MAX_HORIZONTALSCALING)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	if (HPDF_Stream_WriteReal(attr->stream, value) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " Tz\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->h_scaling = value;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETTEXTLEADING)
/*
 TL
 Set text leading
 */
HPDF_STATUS HPDF_Page_SetTextLeading(HPDF_Page page, HPDF_REAL value)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteReal(attr->stream, value) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " TL\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->text_leading = value;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETFONTANDSIZE)
/*
 Tf
 Set text font and size
 */
HPDF_STATUS HPDF_Page_SetFontAndSize(HPDF_Page page, HPDF_Font font, HPDF_REAL size)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_TEXT_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	const char *local_name;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	if (!HPDF_Font_Validate(font))
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_FONT, 0);

	if (size <= 0 || size > HPDF_MAX_FONTSIZE)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_FONT_SIZE, size);

	if (page->mmgr != font->mmgr)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_FONT, 0);

	attr = (HPDF_PageAttr) page->attr;
	local_name = HPDF_Page_GetLocalFontName(page, font);

	if (!local_name)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_FONT, 0);

	if (HPDF_Stream_WriteEscapeName(attr->stream, local_name) != HPDF_OK)
		return HPDF_CheckError(page->error);

	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, size, eptr);
	HPDF_StrCpy(pbuf, " Tf\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->font = font;
	attr->gstate->font_size = size;
	attr->gstate->writing_mode = ((HPDF_FontAttr) font->attr)->writing_mode;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETTEXTRENDERINGMODE)
/*
 Tr
 Set text rendering mode
 */
HPDF_STATUS HPDF_Page_SetTextRenderingMode(HPDF_Page page, HPDF_TextRenderingMode mode)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	if (mode >= HPDF_RENDERING_MODE_EOF)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, mode);

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteInt(attr->stream, (HPDF_INT) mode) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " Tr\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->rendering_mode = mode;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETTEXTRISE)
/*
 Ts
 Set text rise
 */
HPDF_STATUS HPDF_Page_SetTextRise(HPDF_Page page, HPDF_REAL value)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteReal(attr->stream, value) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " Ts\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->text_rise = value;

	return ret;
}
#endif

/*--- Text positioning ---------------------------------------------------*/

#if defined(PAGEOP_ALL) || defined(PAGE_MOVETEXTPOS)
/*
 tx ty Td
 Move to the start of the next line, offset from the start of the current line by
 (tx, ty). tx and ty shall denote numbers expressed in unscaled text space
 units.
 */
HPDF_STATUS HPDF_Page_MoveTextPos(HPDF_Page page, HPDF_REAL x, HPDF_REAL y)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, x, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	HPDF_StrCpy(pbuf, " Td\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->text_matrix.x += x * attr->text_matrix.a + y * attr->text_matrix.c;
	attr->text_matrix.y += y * attr->text_matrix.d + x * attr->text_matrix.b;
	attr->text_pos.x = attr->text_matrix.x;
	attr->text_pos.y = attr->text_matrix.y;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_MOVETEXTPOS2)
/*
 tx ty TD
 Move to the start of the next line, offset from the start of the current line by
 (tx, ty). As a side effect, this operator shall set the leading parameter in
 the text state.
 */
HPDF_STATUS HPDF_Page_MoveTextPos2(HPDF_Page page, HPDF_REAL x, HPDF_REAL y)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, x, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	HPDF_StrCpy(pbuf, " TD\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->text_matrix.x += x * attr->text_matrix.a + y * attr->text_matrix.c;
	attr->text_matrix.y += y * attr->text_matrix.d + x * attr->text_matrix.b;
	attr->text_pos.x = attr->text_matrix.x;
	attr->text_pos.y = attr->text_matrix.y;
	attr->gstate->text_leading = -y;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETTEXTMATRIX)
/*
 a b c d e f Tm
 Set the text matrix, Tm, and the text line matrix, Tlm.
 */
HPDF_STATUS HPDF_Page_SetTextMatrix(HPDF_Page page, HPDF_REAL a, HPDF_REAL b, HPDF_REAL c, HPDF_REAL d, HPDF_REAL x, HPDF_REAL y)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if ((a == 0 || d == 0) && (b == 0 || c == 0))
		return HPDF_RaiseError(page->error, HPDF_INVALID_PARAMETER, 0);

	pbuf = HPDF_FToA(pbuf, a, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, b, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, c, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, d, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	HPDF_StrCpy(pbuf, " Tm\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->text_matrix.a = a;
	attr->text_matrix.b = b;
	attr->text_matrix.c = c;
	attr->text_matrix.d = d;
	attr->text_matrix.x = x;
	attr->text_matrix.y = y;
	attr->text_pos.x = attr->text_matrix.x;
	attr->text_pos.y = attr->text_matrix.y;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_MOVETONEXTLINE)
/*
 T*
 Move to the start of the next line.
 */
HPDF_STATUS HPDF_Page_MoveToNextLine(HPDF_Page page)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (HPDF_Stream_WriteStr(attr->stream, "T*\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	/* calculate the reference point of text */
	attr->text_matrix.x -= attr->gstate->text_leading * attr->text_matrix.c;
	attr->text_matrix.y -= attr->gstate->text_leading * attr->text_matrix.d;

	attr->text_pos.x = attr->text_matrix.x;
	attr->text_pos.y = attr->text_matrix.y;

	return ret;
}
#endif

/*--- Text showing -------------------------------------------------------*/

#if defined(PAGEOP_ALL) || defined(PAGE_INTERNALWRITETEXT)
HPDF_STATUS HPDF_Page_InternalWriteText(HPDF_PageAttr attr, HPDF_MMgr mmgr, const char *text)
{
	HPDF_FontAttr font_attr = (HPDF_FontAttr) attr->gstate->font->attr;
	HPDF_STATUS ret;

	if (font_attr->type == HPDF_FONT_TYPE0_TT || font_attr->type == HPDF_FONT_TYPE0_CID)
	{
		HPDF_Encoder encoder;
		HPDF_UINT len;

		if ((ret = HPDF_Stream_WriteStr(attr->stream, "<")) != HPDF_OK)
			return ret;

		encoder = font_attr->encoder;
		len = HPDF_StrLen(text, HPDF_LIMIT_MAX_STRING_LEN);

		if (encoder->encode_text_fn == NULL)
		{
			if ((ret = HPDF_Stream_WriteBinary(attr->stream, (const HPDF_BYTE *) text, len, NULL)) != HPDF_OK)
				return ret;
		} else
		{
			char *encoded;
			HPDF_UINT length;

			encoded = (encoder->encode_text_fn) (encoder, mmgr, text, len, &length);

			ret = HPDF_Stream_WriteBinary(attr->stream, (HPDF_BYTE *) encoded, length, NULL);

			HPDF_DirectFree(mmgr, encoded);

			if (ret != HPDF_OK)
				return ret;
		}

		return HPDF_Stream_WriteStr(attr->stream, ">");
	}

	return HPDF_Stream_WriteEscapeText(attr->stream, text);
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SHOWTEXT)
/*
 string Tj
 Show a text string.
 */
HPDF_STATUS HPDF_Page_ShowText(HPDF_Page page, const char *text)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;
	HPDF_REAL tw;

	if (ret != HPDF_OK || text == NULL || text[0] == 0)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	/* no font exists */
	if (!attr->gstate->font)
		return HPDF_RaiseError(page->error, HPDF_PAGE_FONT_NOT_FOUND, 0);

	tw = HPDF_Page_TextWidth(page, text);
	if (!tw)
		return ret;

	if (HPDF_Page_InternalWriteText(attr, page->mmgr, text) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " Tj\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	/* calculate the reference point of text */
	if (attr->gstate->writing_mode == HPDF_WMODE_HORIZONTAL)
	{
		attr->text_pos.x += tw * attr->text_matrix.a;
		attr->text_pos.y += tw * attr->text_matrix.b;
	} else
	{
		attr->text_pos.x -= tw * attr->text_matrix.b;
		attr->text_pos.y -= tw * attr->text_matrix.a;
	}

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SHOWTEXTNEXTLINE)
/*
 string TJ
 string '
 Move to the next line and show a text string.
 */
HPDF_STATUS HPDF_Page_ShowTextNextLine(HPDF_Page page, const char *text)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;
	HPDF_REAL tw;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	/* no font exists */
	if (!attr->gstate->font)
		return HPDF_RaiseError(page->error, HPDF_PAGE_FONT_NOT_FOUND, 0);

	if (text == NULL || text[0] == 0)
		return HPDF_Page_MoveToNextLine(page);

	if (HPDF_Page_InternalWriteText(attr, page->mmgr, text) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " '\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	tw = HPDF_Page_TextWidth(page, text);

	/* calculate the reference point of text */
	attr->text_matrix.x -= attr->gstate->text_leading * attr->text_matrix.c;
	attr->text_matrix.y -= attr->gstate->text_leading * attr->text_matrix.d;

	attr->text_pos.x = attr->text_matrix.x;
	attr->text_pos.y = attr->text_matrix.y;

	if (attr->gstate->writing_mode == HPDF_WMODE_HORIZONTAL)
	{
		attr->text_pos.x += tw * attr->text_matrix.a;
		attr->text_pos.y += tw * attr->text_matrix.b;
	} else
	{
		attr->text_pos.x -= tw * attr->text_matrix.b;
		attr->text_pos.y -= tw * attr->text_matrix.a;
	}

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SHOWTEXTNEXTLINEEX)
/*
 aw ac string "
 Move to the next line and show a text string, using aw as the word spacing
 and ac as the character spacing (setting the corresponding parameters in
 the text state). aw and ac shall be numbers expressed in unscaled text
 space units.
 */
HPDF_STATUS HPDF_Page_ShowTextNextLineEx(HPDF_Page page, HPDF_REAL word_space, HPDF_REAL char_space, const char *text)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;
	HPDF_REAL tw;
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;

	if (ret != HPDF_OK)
		return ret;

	if (word_space < HPDF_MIN_WORDSPACE || word_space > HPDF_MAX_WORDSPACE)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	if (char_space < HPDF_MIN_CHARSPACE || char_space > HPDF_MAX_CHARSPACE)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	attr = (HPDF_PageAttr) page->attr;

	/* no font exists */
	if (!attr->gstate->font)
		return HPDF_RaiseError(page->error, HPDF_PAGE_FONT_NOT_FOUND, 0);

	if (text == NULL || text[0] == 0)
		return HPDF_Page_MoveToNextLine(page);

	pbuf = HPDF_FToA(pbuf, word_space, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, char_space, eptr);
	*pbuf = ' ';

	if (HPDF_Page_InternalWriteText(attr, page->mmgr, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Page_InternalWriteText(attr, page->mmgr, text) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " \"\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->word_space = word_space;
	attr->gstate->char_space = char_space;

	tw = HPDF_Page_TextWidth(page, text);

	/* calculate the reference point of text */
	attr->text_matrix.x += attr->gstate->text_leading * attr->text_matrix.b;
	attr->text_matrix.y -= attr->gstate->text_leading * attr->text_matrix.a;

	attr->text_pos.x = attr->text_matrix.x;
	attr->text_pos.y = attr->text_matrix.y;

	if (attr->gstate->writing_mode == HPDF_WMODE_HORIZONTAL)
	{
		attr->text_pos.x += tw * attr->text_matrix.a;
		attr->text_pos.y += tw * attr->text_matrix.b;
	} else
	{
		attr->text_pos.x -= tw * attr->text_matrix.b;
		attr->text_pos.y -= tw * attr->text_matrix.a;
	}

	return ret;
}
#endif


/*--- Color showing ------------------------------------------------------*/

#if defined(PAGEOP_ALL) || defined(PAGE_SETCOLORSPACESTROKE)
/*
 name CS
 Set the current color space to use for stroking operations.
 */
HPDF_STATUS HPDF_Page_SetColorspaceStroke(HPDF_Page page, HPDF_PatternColorspace colorspace)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;
	const char *local_name;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;
	local_name = HPDF_Page_GetLocalColorspaceName(page, colorspace);

	if (!local_name)
		return HPDF_RaiseError(page->error, HPDF_INVALID_OBJECT, 0);

	if (HPDF_Stream_WriteEscapeName(attr->stream, local_name) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " CS\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->cs_stroke = HPDF_CS_PATTERN;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETCOLORSPACEFILL)
/*
 name cs
 Same as CS but used for nonstroking operations.
 */
HPDF_STATUS HPDF_Page_SetColorspaceFill(HPDF_Page page, HPDF_PatternColorspace colorspace)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;
	const char *local_name;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;
	local_name = HPDF_Page_GetLocalColorspaceName(page, colorspace);

	if (!local_name)
		return HPDF_RaiseError(page->error, HPDF_INVALID_OBJECT, 0);

	if (HPDF_Stream_WriteEscapeName(attr->stream, local_name) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " cs\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->cs_fill = HPDF_CS_PATTERN;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETPATTERNSTROKE)
/*
 SC
 Set the color to use for stroking operations in a device, CIE-
 based (other than ICCBased), or Indexed color space. The number of
 operands required and their interpretation depends on the current
 stroking color space.
 SCN
 Same as SC but also supports Pattern, Separation, DeviceN
 and ICCBased color spaces.
 */
HPDF_STATUS HPDF_Page_SetPatternStroke(HPDF_Page page, HPDF_Pattern pattern, HPDF_REAL r, HPDF_REAL g, HPDF_REAL b)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;
	const char *local_name;
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_Number obj;
	
	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;
	local_name = HPDF_Page_GetLocalPatternName(page, pattern);

	if (!local_name)
		return HPDF_RaiseError(page->error, HPDF_INVALID_OBJECT, 0);

	/*
	 * only for uncolored tiling patterns
	 */
	obj = (HPDF_Number) HPDF_Dict_GetItem(pattern, "PatternType", HPDF_OCLASS_NUMBER);
	if (obj && obj->value == HPDF_PATTERN_TYPE_TILED)
	{
		obj = (HPDF_Number) HPDF_Dict_GetItem(pattern, "PaintType", HPDF_OCLASS_NUMBER);
		if (obj && obj->value == HPDF_PAINT_TYPE_UNCOLORED)
		{
			pbuf = HPDF_FToA(pbuf, r, eptr);
			*pbuf++ = ' ';
			pbuf = HPDF_FToA(pbuf, g, eptr);
			*pbuf++ = ' ';
			pbuf = HPDF_FToA(pbuf, b, eptr);
			*pbuf++ = ' ';
		}
	}
	*pbuf++ = '/';
	pbuf = HPDF_StrCpy(pbuf, local_name, eptr);

	switch (attr->gstate->cs_fill)
	{
	case HPDF_CS_ICC_BASED:
	case HPDF_CS_SEPARATION:
	case HPDF_CS_DEVICE_N:
	case HPDF_CS_PATTERN:
		pbuf = HPDF_StrCpy(pbuf, " SCN\012", eptr);
		break;
	default:
		pbuf = HPDF_StrCpy(pbuf, " SC\012", eptr);
		break;
	}
	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETPATTERNFILL)
/*
 sc
 Same as SC but used for nonstroking operations.
 scn
 Same as SCN but used for nonstroking operations.
 */
HPDF_STATUS HPDF_Page_SetPatternFill(HPDF_Page page, HPDF_Pattern pattern, HPDF_REAL r, HPDF_REAL g, HPDF_REAL b)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;
	const char *local_name;
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_Number obj;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;
	local_name = HPDF_Page_GetLocalPatternName(page, pattern);

	if (!local_name)
		return HPDF_RaiseError(page->error, HPDF_INVALID_OBJECT, 0);

	/*
	 * only for uncolored tiling patterns
	 */
	obj = (HPDF_Number) HPDF_Dict_GetItem(pattern, "PatternType", HPDF_OCLASS_NUMBER);
	if (obj && obj->value == HPDF_PATTERN_TYPE_TILED)
	{
		obj = (HPDF_Number) HPDF_Dict_GetItem(pattern, "PaintType", HPDF_OCLASS_NUMBER);
		if (obj && obj->value == HPDF_PAINT_TYPE_UNCOLORED)
		{
			pbuf = HPDF_FToA(pbuf, r, eptr);
			*pbuf++ = ' ';
			pbuf = HPDF_FToA(pbuf, g, eptr);
			*pbuf++ = ' ';
			pbuf = HPDF_FToA(pbuf, b, eptr);
			*pbuf++ = ' ';
		}
	}
	*pbuf++ = '/';
	pbuf = HPDF_StrCpy(pbuf, local_name, eptr);

	switch (attr->gstate->cs_fill)
	{
	case HPDF_CS_ICC_BASED:
	case HPDF_CS_SEPARATION:
	case HPDF_CS_DEVICE_N:
	case HPDF_CS_PATTERN:
		pbuf = HPDF_StrCpy(pbuf, " scn\012", eptr);
		break;
	default:
		pbuf = HPDF_StrCpy(pbuf, " sc\012", eptr);
		break;
	}
	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETGRAYFILL)
/*
 gray g
 Same as G but used for nonstroking operations.
 */
HPDF_STATUS HPDF_Page_SetGrayFill(HPDF_Page page, HPDF_REAL gray)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (gray < 0 || gray > 1)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	if (HPDF_Stream_WriteReal(attr->stream, gray) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " g\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->gray_fill = gray;
	attr->gstate->cs_fill = HPDF_CS_DEVICE_GRAY;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETGRAYSTROKE)
/*
 gray G
 Set the stroking color space to DeviceGray (or the DefaultGray color
 space) and set the gray level to use for stroking operations.
 gray shall be a number between 0.0 (black) and 1.0 (white).
 */
HPDF_STATUS HPDF_Page_SetGrayStroke(HPDF_Page page, HPDF_REAL gray)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	if (gray < 0 || gray > 1)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	if (HPDF_Stream_WriteReal(attr->stream, gray) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " G\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->gray_stroke = gray;
	attr->gstate->cs_stroke = HPDF_CS_DEVICE_GRAY;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETRGBFILL)
/*
 r g b rg
 Same as RG but used for nonstroking operations.
 */
HPDF_STATUS HPDF_Page_SetRGBFill(HPDF_Page page, HPDF_REAL r, HPDF_REAL g, HPDF_REAL b)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_PAGE_DESCRIPTION);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	if (r < 0 || r > 1 || g < 0 || g > 1 || b < 0 || b > 1)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, r, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, g, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, b, eptr);
	HPDF_StrCpy(pbuf, " rg\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->rgb_fill.r = r;
	attr->gstate->rgb_fill.g = g;
	attr->gstate->rgb_fill.b = b;
	attr->gstate->cs_fill = HPDF_CS_DEVICE_RGB;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETRGBSTROKE)
/*
 r g b RG
 Set the stroking colour space to DeviceRGB (or the DefaultRGB colour
 space) and set the colour to use for stroking operations.
 Each operand shall be a number between 0.0 (minimum intensity)
 and 1.0 (maximum intensity).
 */
HPDF_STATUS HPDF_Page_SetRGBStroke(HPDF_Page page, HPDF_REAL r, HPDF_REAL g, HPDF_REAL b)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_PAGE_DESCRIPTION);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	if (r < 0 || r > 1 || g < 0 || g > 1 || b < 0 || b > 1)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, r, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, g, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, b, eptr);
	HPDF_StrCpy(pbuf, " RG\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->rgb_stroke.r = r;
	attr->gstate->rgb_stroke.g = g;
	attr->gstate->rgb_stroke.b = b;
	attr->gstate->cs_stroke = HPDF_CS_DEVICE_RGB;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETCMYKFILL)
/*
 c m y k k
 Same as K but used for nonstroking operations.
 */
HPDF_STATUS HPDF_Page_SetCMYKFill(HPDF_Page page, HPDF_REAL c, HPDF_REAL m, HPDF_REAL y, HPDF_REAL k)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_PAGE_DESCRIPTION);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	if (c < 0 || c > 1 || m < 0 || m > 1 || y < 0 || y > 1 || k < 0 || k > 1)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, c, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, m, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, k, eptr);
	HPDF_StrCpy(pbuf, " k\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->cmyk_fill.c = c;
	attr->gstate->cmyk_fill.m = m;
	attr->gstate->cmyk_fill.y = y;
	attr->gstate->cmyk_fill.k = k;
	attr->gstate->cs_fill = HPDF_CS_DEVICE_CMYK;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETCMYKSTROKE)
/*
 c m y k K
 Set the stroking colour space to DeviceCMYK (or the DefaultCMYK
 colour space) and set the colour to
 use for stroking operations. Each operand shall be a number between 0.0
 (zero concentration) and 1.0 (maximum concentration). The behaviour of
 this operator is affected by the overprint mode.
 */
HPDF_STATUS HPDF_Page_SetCMYKStroke(HPDF_Page page, HPDF_REAL c, HPDF_REAL m, HPDF_REAL y, HPDF_REAL k)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT | HPDF_GMODE_PATH_OBJECT | HPDF_GMODE_PAGE_DESCRIPTION);
	char buf[HPDF_TMP_BUF_SIZ];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	if (c < 0 || c > 1 || m < 0 || m > 1 || y < 0 || y > 1 || k < 0 || k > 1)
		return HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, c, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, m, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, k, eptr);
	HPDF_StrCpy(pbuf, " K\012", eptr);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->gstate->cmyk_stroke.c = c;
	attr->gstate->cmyk_stroke.m = m;
	attr->gstate->cmyk_stroke.y = y;
	attr->gstate->cmyk_stroke.k = k;
	attr->gstate->cs_stroke = HPDF_CS_DEVICE_CMYK;

	return ret;
}
#endif

/*--- Shading patterns ---------------------------------------------------*/

/*
 name sh
 Paint the shape and colour shading described by a shading
 dictionary, subject to the current clipping path. The current colour in the
 graphics state is neither used nor altered. The effect is different from that of
 painting a path using a shading pattern as the current colour.
 name is the name of a shading dictionary resource in the Shading
 subdictionary of the current resource dictionary.
 All coordinates in the shading dictionary are interpreted
 relative to the current user space. (By contrast, when a shading dictionary is
 used in a type 2 pattern, the coordinates are expressed in pattern space.) All
 colours are interpreted in the colour space identified by the shading
 dictionary?s ColorSpace entry (see Table 78). The Background entry, if
 present, is ignored.
 This operator should be applied only to bounded or geometrically defined
 shadings. If applied to an unbounded shading, it paints the shading?s
 gradient fill across the entire clipping region, which may be time-consuming.
 --not implemented yet
 */

/*--- In-line images -----------------------------------------------------*/

/*
 BI
 Begin an inline image object.
 --not implemented yet
 */
/*
 ID
 Begin the image data for an inline image object.
 --not implemented yet
 */
/*
 EI
 --not implemented yet
 End an inline image object.
 */

/*--- XObjects -----------------------------------------------------------*/

#if defined(PAGEOP_ALL) || defined(PAGE_EXECUTEXOBJECT)
/*
 name Do
 Paint the specified XObject. The operand name shall appear as a key in
 the XObject subdictionary of the current resource dictionary.
 The associated value shall be a stream whose
 Type entry, if present, is XObject. The effect of Do depends on the value
 of the XObject's Subtype entry, which may be Image, Form, or PS.
 */
HPDF_STATUS HPDF_Page_ExecuteXObject(HPDF_Page page, HPDF_XObject obj)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION);
	HPDF_PageAttr attr;
	const char *local_name;

	if (ret != HPDF_OK)
		return ret;

	if (!obj || obj->header.obj_class != (HPDF_OSUBCLASS_XOBJECT | HPDF_OCLASS_DICT))
		return HPDF_RaiseError(page->error, HPDF_INVALID_OBJECT, 0);

	if (page->mmgr != obj->mmgr)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_XOBJECT, 0);

	attr = (HPDF_PageAttr) page->attr;
	local_name = HPDF_Page_GetXObjectName(page, obj);

	if (!local_name)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_XOBJECT, 0);

	if (HPDF_Stream_WriteEscapeName(attr->stream, local_name) != HPDF_OK)
		return HPDF_CheckError(page->error);

	if (HPDF_Stream_WriteStr(attr->stream, " Do\012") != HPDF_OK)
		return HPDF_CheckError(page->error);

	return ret;
}
#endif

/*--- Marked content -----------------------------------------------------*/

/*
 tag BMC
 Begin a marked-content sequence terminated by a balancing EMC
 operator. tag shall be a name object indicating the role or significance of
 the sequence.
 --not implemented yet
 */

/*
 tag BDC
 Begin a marked-content sequence with an associated property list,
 terminated by a balancing EMC operator. tag shall be a name object
 indicating the role or significance of the sequence. properties shall be
 either an inline dictionary containing the property list or a name object
 associated with it in the Properties subdictionary of the current resource
 dictionary.
 --not implemented yet
 */

/*
 EMC
 End a marked-content sequence begun by a BMC or BDC operator.
 --not implemented yet
 */

/*
 tag MP
 Designate a marked-content point. tag shall be a name object indicating
 the role or significance of the point.
 --not implemented yet
 */

/*
 tag properties DP
 Designate a marked-content point with an associated property list. tag
 shall be a name object indicating the role or significance of the point.
 properties shall be either an inline dictionary containing the property list or
 a name object associated with it in the Properties subdictionary of the
 current resource dictionary.
 --not implemented yet
 */

/*--- Compatibility ------------------------------------------------------*/

/*
 BX
 Begin a compatibility section. Unrecognized operators (along with
 their operands) shall be ignored without error until the balancing EX operator
 is encountered.
 --not implemented yet
 */

/*
 EX
 End a compatibility section begun by a balancing BX operator.
 Ignore any unrecognized operands and operators from previous matching
 BX onward.
 --not implemented yet
 */

/*--- combined function --------------------------------------------------*/

#define KAPPA 0.5522847498F /* (4.0 * (sqrt(2.0) - 1.0) / 3.0) */

#if defined(PAGEOP_ALL) || defined(PAGE_CIRCLE)
HPDF_STATUS HPDF_Page_Circle(HPDF_Page page, HPDF_REAL x, HPDF_REAL y, HPDF_REAL ray)
{
	return HPDF_Page_Ellipse(page, x, y, ray, ray);
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_ELLIPSE)
static char *QuarterEllipseA(char *pbuf, char *eptr, HPDF_REAL x, HPDF_REAL y, HPDF_REAL xray, HPDF_REAL yray)
{
	pbuf = HPDF_FToA(pbuf, x - xray, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y + yray * KAPPA, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x - xray * KAPPA, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y + yray, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y + yray, eptr);
	return HPDF_StrCpy(pbuf, " c\012", eptr);
}


static char *QuarterEllipseB(char *pbuf, char *eptr, HPDF_REAL x, HPDF_REAL y, HPDF_REAL xray, HPDF_REAL yray)
{
	pbuf = HPDF_FToA(pbuf, x + xray * KAPPA, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y + yray, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x + xray, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y + yray * KAPPA, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x + xray, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	return HPDF_StrCpy(pbuf, " c\012", eptr);
}


static char *QuarterEllipseC(char *pbuf, char *eptr, HPDF_REAL x, HPDF_REAL y, HPDF_REAL xray, HPDF_REAL yray)
{
	pbuf = HPDF_FToA(pbuf, x + xray, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y - yray * KAPPA, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x + xray * KAPPA, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y - yray, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y - yray, eptr);
	return HPDF_StrCpy(pbuf, " c\012", eptr);
}


static char *QuarterEllipseD(char *pbuf, char *eptr, HPDF_REAL x, HPDF_REAL y, HPDF_REAL xray, HPDF_REAL yray)
{
	pbuf = HPDF_FToA(pbuf, x - xray * KAPPA, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y - yray, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x - xray, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y - yray * KAPPA, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, x - xray, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	return HPDF_StrCpy(pbuf, " c\012", eptr);
}


HPDF_STATUS HPDF_Page_Ellipse(HPDF_Page page, HPDF_REAL x, HPDF_REAL y, HPDF_REAL xray, HPDF_REAL yray)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT);
	char buf[HPDF_TMP_BUF_SIZ * 2];
	char *pbuf = buf;
	char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	pbuf = HPDF_FToA(pbuf, x - xray, eptr);
	*pbuf++ = ' ';
	pbuf = HPDF_FToA(pbuf, y, eptr);
	pbuf = HPDF_StrCpy(pbuf, " m\012", eptr);

	pbuf = QuarterEllipseA(pbuf, eptr, x, y, xray, yray);
	pbuf = QuarterEllipseB(pbuf, eptr, x, y, xray, yray);
	pbuf = QuarterEllipseC(pbuf, eptr, x, y, xray, yray);
	QuarterEllipseD(pbuf, eptr, x, y, xray, yray);

	if (HPDF_Stream_WriteStr(attr->stream, buf) != HPDF_OK)
		return HPDF_CheckError(page->error);

	attr->cur_pos.x = x - xray;
	attr->cur_pos.y = y;
	attr->str_pos = attr->cur_pos;
	attr->gmode = HPDF_GMODE_PATH_OBJECT;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_ARC)
HPDF_STATUS HPDF_Page_Arc(HPDF_Page page, HPDF_REAL x0, HPDF_REAL y0, HPDF_REAL xrad, HPDF_REAL yrad, HPDF_REAL beg_ang, HPDF_REAL end_ang)
{
	HPDF_STATUS ret;
	HPDF_REAL del_ang;
	HPDF_REAL angle, start;
	HPDF_REAL x, y;
	const HPDF_REAL PIE2 = 2.0f * 3.14159F;
	int i, n_steps;
	HPDF_PageAttr attr;
	
	ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_PATH_OBJECT);
	if (ret != HPDF_OK)
		return ret;
	attr = (HPDF_PageAttr) page->attr;

	beg_ang = (beg_ang / 360.0f) * PIE2;
	end_ang = (end_ang / 360.0f) * PIE2;

	del_ang = end_ang - beg_ang;
	if (del_ang < 0)
		del_ang += PIE2;
	if (del_ang > PIE2)
		HPDF_RaiseError(page->error, HPDF_PAGE_OUT_OF_RANGE, 0);

	while (beg_ang < 0 || end_ang < 0)
	{
		beg_ang += PIE2;
		end_ang += PIE2;
	}

	angle = beg_ang;
	start = angle;

	x = x0 + HPDF_COS(angle) * xrad;
	y = y0 + HPDF_SIN(angle) * yrad;
	
	if (attr->gmode == HPDF_GMODE_PATH_OBJECT)
		ret = HPDF_Page_LineTo(page, x, y);
	else
		ret = HPDF_Page_MoveTo(page, x, y);
	if (ret != HPDF_OK)
		return ret;

	if (xrad > yrad)
		n_steps = xrad;
	else
		n_steps = yrad;
	n_steps >>= 2;
	if (n_steps < 32)
		n_steps = 32;

	for (i = 1; i < n_steps; i++)
	{
		angle = (del_ang * i) / n_steps + start;
		
		x = x0 + HPDF_COS(angle) * xrad;
		y = y0 + HPDF_SIN(angle) * yrad;
		
		if ((ret = HPDF_Page_LineTo(page, x, y)) != HPDF_OK)
			return ret;
	}
		
	angle = end_ang;
	x = x0 + HPDF_COS(angle) * xrad;
	y = y0 + HPDF_SIN(angle) * yrad;
	if ((ret = HPDF_Page_LineTo(page, x, y)) != HPDF_OK)
		return ret;
	
	return HPDF_OK;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_DRAWIMAGE)
HPDF_STATUS HPDF_Page_DrawImage(HPDF_Page page, HPDF_Image image, HPDF_REAL x, HPDF_REAL y, HPDF_REAL width, HPDF_REAL height)
{
	HPDF_STATUS ret;

	if ((ret = HPDF_Page_GSave(page)) != HPDF_OK)
		return ret;

	if ((ret = HPDF_Page_Concat(page, width, 0, 0, height, x, y)) != HPDF_OK)
		return ret;

	if ((ret = HPDF_Page_ExecuteXObject(page, image)) != HPDF_OK)
		return ret;

	return HPDF_Page_GRestore(page);
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_TEXTPOS_ABSTOREL)
/*
 * Convert a user space text position from absolute to relative coordinates.
 * Absolute values are passed in xAbs and yAbs, relative values are returned
 * to xRel and yRel. The latter two must not be NULL.
 */
void HPDF_Page_TextPos_AbsToRel(const HPDF_TransMatrix *text_matrix, HPDF_REAL xAbs, HPDF_REAL yAbs, HPDF_REAL *xRel, HPDF_REAL *yRel)
{
	if (text_matrix->a == 0)
	{
		*xRel = (yAbs - text_matrix->y - (xAbs - text_matrix->x) * text_matrix->d / text_matrix->c) / text_matrix->b;
		*yRel = (xAbs - text_matrix->x) / text_matrix->c;
	} else
	{
		HPDF_REAL y = (yAbs - text_matrix->y - (xAbs - text_matrix->x) *
					   text_matrix->b / text_matrix->a) / (text_matrix->d - text_matrix->c * text_matrix->b / text_matrix->a);
		*xRel = (xAbs - text_matrix->x - y * text_matrix->c) / text_matrix->a;
		*yRel = y;
	}
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_TEXTOUT)
HPDF_STATUS HPDF_Page_TextOut(HPDF_Page page, HPDF_REAL xpos, HPDF_REAL ypos, const char *text)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT);
	HPDF_REAL x;
	HPDF_REAL y;
	HPDF_PageAttr attr;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;
	HPDF_Page_TextPos_AbsToRel(&attr->text_matrix, xpos, ypos, &x, &y);
	if ((ret = HPDF_Page_MoveTextPos(page, x, y)) != HPDF_OK)
		return ret;

	return HPDF_Page_ShowText(page, text);
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_TEXTRECT)
static HPDF_STATUS InternalShowTextNextLine(HPDF_Page page, const char *text, HPDF_UINT len)
{
	HPDF_STATUS ret;
	HPDF_PageAttr attr;
	HPDF_REAL tw;
	HPDF_FontAttr font_attr;

	attr = (HPDF_PageAttr) page->attr;
	font_attr = (HPDF_FontAttr) attr->gstate->font->attr;

	if (font_attr->type == HPDF_FONT_TYPE0_TT || font_attr->type == HPDF_FONT_TYPE0_CID)
	{
		HPDF_Encoder encoder = font_attr->encoder;

		if ((ret = HPDF_Stream_WriteStr(attr->stream, "<")) != HPDF_OK)
			return ret;

		if (encoder->encode_text_fn == NULL)
		{
			if ((ret = HPDF_Stream_WriteBinary(attr->stream, (const HPDF_BYTE *) text, len, NULL)) != HPDF_OK)
				return ret;
		} else
		{
			char *encoded;
			HPDF_UINT length;

			encoded = (encoder->encode_text_fn) (encoder, page->mmgr, text, len, &length);
			ret = HPDF_Stream_WriteBinary(attr->stream, (HPDF_BYTE *) encoded, length, NULL);
			HPDF_DirectFree(page->mmgr, encoded);

			if (ret != HPDF_OK)
				return ret;
		}

		if ((ret = HPDF_Stream_WriteStr(attr->stream, ">")) != HPDF_OK)
			return ret;
	} else if ((ret = HPDF_Stream_WriteEscapeText2(attr->stream, text, len)) != HPDF_OK)
	{
		return ret;
	}

	if ((ret = HPDF_Stream_WriteStr(attr->stream, " \'\012")) != HPDF_OK)
		return ret;

	tw = HPDF_Page_TextWidth(page, text);

	/* calculate the reference point of text */
	attr->text_matrix.x -= attr->gstate->text_leading * attr->text_matrix.c;
	attr->text_matrix.y -= attr->gstate->text_leading * attr->text_matrix.d;

	attr->text_pos.x = attr->text_matrix.x;
	attr->text_pos.y = attr->text_matrix.y;

	if (attr->gstate->writing_mode == HPDF_WMODE_HORIZONTAL)
	{
		attr->text_pos.x += tw * attr->text_matrix.a;
		attr->text_pos.y += tw * attr->text_matrix.b;
	} else
	{
		attr->text_pos.x -= tw * attr->text_matrix.b;
		attr->text_pos.y -= tw * attr->text_matrix.a;
	}

	return ret;
}


HPDF_STATUS HPDF_Page_TextRect(
	HPDF_Page page,
	HPDF_REAL left,
	HPDF_REAL top,
	HPDF_REAL right,
	HPDF_REAL bottom,
	const char *text,
	HPDF_TextAlignment align,
	HPDF_UINT *len)
{
	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;
	const char *ptr = text;
	HPDF_BOOL pos_initialized = HPDF_FALSE;
	HPDF_REAL save_char_space = 0;
	HPDF_BOOL is_insufficient_space = HPDF_FALSE;
	HPDF_UINT num_rest;
	HPDF_Box bbox;
	HPDF_BOOL char_space_changed = HPDF_FALSE;

	if (ret != HPDF_OK)
		return ret;

	attr = (HPDF_PageAttr) page->attr;

	/* no font exists */
	if (!attr->gstate->font)
	{
		return HPDF_RaiseError(page->error, HPDF_PAGE_FONT_NOT_FOUND, 0);
	}

	HPDF_Font_GetBBox(attr->gstate->font, &bbox);

	if (len)
		*len = 0;
	num_rest = HPDF_StrLen(text, HPDF_LIMIT_MAX_STRING_LEN + 1);

	if (num_rest > HPDF_LIMIT_MAX_STRING_LEN)
	{
		return HPDF_RaiseError(page->error, HPDF_STRING_OUT_OF_RANGE, 0);
	}
	if (num_rest == 0)
		return HPDF_OK;

	if (attr->gstate->text_leading == 0)
		HPDF_Page_SetTextLeading(page, (bbox.top - bbox.bottom) / 1000 * attr->gstate->font_size);

	top = top - bbox.top / 1000 * attr->gstate->font_size + attr->gstate->text_leading;
	bottom = bottom - bbox.bottom / 1000 * attr->gstate->font_size;

	if (align == HPDF_TALIGN_JUSTIFY)
	{
		save_char_space = attr->gstate->char_space;
		attr->gstate->char_space = 0;
	}

	for (;;)
	{
		HPDF_REAL x, y;
		HPDF_UINT line_len, tmp_len;
		HPDF_REAL rw;
		HPDF_BOOL LineBreak;

		attr->gstate->char_space = 0;
		line_len = tmp_len = HPDF_Page_MeasureText(page, ptr, right - left, HPDF_TRUE, &rw);
		if (line_len == 0)
		{
			is_insufficient_space = HPDF_TRUE;
			break;
		}

		if (len)
			*len += line_len;
		num_rest -= line_len;

		/* Shorten tmp_len by trailing whitespace and control characters. */
		LineBreak = HPDF_FALSE;
		while (tmp_len > 0 && HPDF_IS_WHITE_SPACE(ptr[tmp_len - 1]))
		{
			tmp_len--;
			if (ptr[tmp_len] == 0x0A || ptr[tmp_len] == 0x0D)
			{
				LineBreak = HPDF_TRUE;
			}
		}

		switch (align)
		{

		case HPDF_TALIGN_RIGHT:
			HPDF_Page_TextPos_AbsToRel(&attr->text_matrix, right - rw, top, &x, &y);
			if (!pos_initialized)
			{
				pos_initialized = HPDF_TRUE;
			} else
			{
				y = 0;
			}
			if ((ret = HPDF_Page_MoveTextPos(page, x, y)) != HPDF_OK)
				return ret;
			break;

		case HPDF_TALIGN_CENTER:
			HPDF_Page_TextPos_AbsToRel(&attr->text_matrix, left + (right - left - rw) / 2, top, &x, &y);
			if (!pos_initialized)
			{
				pos_initialized = HPDF_TRUE;
			} else
			{
				y = 0;
			}
			if ((ret = HPDF_Page_MoveTextPos(page, x, y)) != HPDF_OK)
				return ret;
			break;

		case HPDF_TALIGN_JUSTIFY:
			if (!pos_initialized)
			{
				pos_initialized = HPDF_TRUE;
				HPDF_Page_TextPos_AbsToRel(&attr->text_matrix, left, top, &x, &y);
				if ((ret = HPDF_Page_MoveTextPos(page, x, y)) != HPDF_OK)
					return ret;
			}

			/* Do not justify last line of paragraph or text. */
			if (LineBreak || num_rest <= 0)
			{
				if ((ret = HPDF_Page_SetCharSpace(page, save_char_space)) != HPDF_OK)
					return ret;
				char_space_changed = HPDF_FALSE;
			} else
			{
				HPDF_REAL x_adjust;
				HPDF_ParseText_Rec state;
				HPDF_UINT i = 0;
				HPDF_UINT num_char = 0;
				HPDF_Encoder encoder = ((HPDF_FontAttr) attr->gstate->font->attr)->encoder;
				const char *tmp_ptr = ptr;

				HPDF_Encoder_SetParseText(encoder, &state, (const HPDF_BYTE *) tmp_ptr, tmp_len);
				while (*tmp_ptr)
				{
					HPDF_ByteType btype = HPDF_Encoder_ByteType(encoder, &state);

					if (btype != HPDF_BYTE_TYPE_TRIAL)
						num_char++;
					i++;
					if (i >= tmp_len)
						break;
					tmp_ptr++;
				}

				x_adjust = num_char == 0 ? 0 : (right - left - rw) / (num_char - 1);
				if ((ret = HPDF_Page_SetCharSpace(page, x_adjust)) != HPDF_OK)
					return ret;
				char_space_changed = HPDF_TRUE;
			}
			break;

		default:
			if (!pos_initialized)
			{
				pos_initialized = HPDF_TRUE;
				HPDF_Page_TextPos_AbsToRel(&attr->text_matrix, left, top, &x, &y);
				if ((ret = HPDF_Page_MoveTextPos(page, x, y)) != HPDF_OK)
					return ret;
			}
		}

		if (InternalShowTextNextLine(page, ptr, tmp_len) != HPDF_OK)
			return HPDF_CheckError(page->error);

		if (num_rest <= 0)
			break;

		if (attr->text_pos.y - attr->gstate->text_leading < bottom)
		{
			is_insufficient_space = HPDF_TRUE;
			break;
		}

		ptr += line_len;
	}

	if (char_space_changed && save_char_space != attr->gstate->char_space)
	{
		if ((ret = HPDF_Page_SetCharSpace(page, save_char_space)) != HPDF_OK)
			return ret;
	}

	if (is_insufficient_space)
		return HPDF_PAGE_INSUFFICIENT_SPACE;
	return HPDF_OK;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_SETSLIDESHOW)
/*
 *  This function is contributed by Adrian Nelson (adenelson).
 */
HPDF_STATUS HPDF_Page_SetSlideShow(HPDF_Page page, HPDF_TransitionStyle type, HPDF_REAL disp_time, HPDF_REAL trans_time)
{
	HPDF_STATUS ret = HPDF_OK;
	HPDF_Dict dict;

	if (!HPDF_Page_Validate(page))
		return HPDF_INVALID_PAGE;

	if (disp_time < 0)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_DISPLAY_TIME, disp_time);

	if (trans_time < 0)
		return HPDF_RaiseError(page->error, HPDF_PAGE_INVALID_TRANSITION_TIME, trans_time);

	dict = HPDF_Dict_New(page->mmgr);

	if (!dict)
		return HPDF_Error_GetCode(page->error);

	if (HPDF_Dict_AddName(dict, "Type", "Trans") != HPDF_OK)
		goto Fail;

	if (HPDF_Dict_AddReal(dict, "D", trans_time) != HPDF_OK)
		goto Fail;

	switch (type)
	{
	case HPDF_TS_WIPE_RIGHT:
		ret |= HPDF_Dict_AddName(dict, "S", "Wipe");
		ret |= HPDF_Dict_AddNumber(dict, "Di", 0);
		break;
	case HPDF_TS_WIPE_UP:
		ret |= HPDF_Dict_AddName(dict, "S", "Wipe");
		ret |= HPDF_Dict_AddNumber(dict, "Di", 90);
		break;
	case HPDF_TS_WIPE_LEFT:
		ret |= HPDF_Dict_AddName(dict, "S", "Wipe");
		ret |= HPDF_Dict_AddNumber(dict, "Di", 180);
		break;
	case HPDF_TS_WIPE_DOWN:
		ret |= HPDF_Dict_AddName(dict, "S", "Wipe");
		ret |= HPDF_Dict_AddNumber(dict, "Di", 270);
		break;
	case HPDF_TS_BARN_DOORS_HORIZONTAL_OUT:
		ret |= HPDF_Dict_AddName(dict, "S", "Split");
		ret |= HPDF_Dict_AddName(dict, "Dm", "H");
		ret |= HPDF_Dict_AddName(dict, "M", "O");
		break;
	case HPDF_TS_BARN_DOORS_HORIZONTAL_IN:
		ret |= HPDF_Dict_AddName(dict, "S", "Split");
		ret |= HPDF_Dict_AddName(dict, "Dm", "H");
		ret |= HPDF_Dict_AddName(dict, "M", "I");
		break;
	case HPDF_TS_BARN_DOORS_VERTICAL_OUT:
		ret |= HPDF_Dict_AddName(dict, "S", "Split");
		ret |= HPDF_Dict_AddName(dict, "Dm", "V");
		ret |= HPDF_Dict_AddName(dict, "M", "O");
		break;
	case HPDF_TS_BARN_DOORS_VERTICAL_IN:
		ret |= HPDF_Dict_AddName(dict, "S", "Split");
		ret |= HPDF_Dict_AddName(dict, "Dm", "V");
		ret |= HPDF_Dict_AddName(dict, "M", "I");
		break;
	case HPDF_TS_BOX_OUT:
		ret |= HPDF_Dict_AddName(dict, "S", "Box");
		ret |= HPDF_Dict_AddName(dict, "M", "O");
		break;
	case HPDF_TS_BOX_IN:
		ret |= HPDF_Dict_AddName(dict, "S", "Box");
		ret |= HPDF_Dict_AddName(dict, "M", "I");
		break;
	case HPDF_TS_BLINDS_HORIZONTAL:
		ret |= HPDF_Dict_AddName(dict, "S", "Blinds");
		ret |= HPDF_Dict_AddName(dict, "Dm", "H");
		break;
	case HPDF_TS_BLINDS_VERTICAL:
		ret |= HPDF_Dict_AddName(dict, "S", "Blinds");
		ret |= HPDF_Dict_AddName(dict, "Dm", "V");
		break;
	case HPDF_TS_DISSOLVE:
		ret |= HPDF_Dict_AddName(dict, "S", "Dissolve");
		break;
	case HPDF_TS_GLITTER_RIGHT:
		ret |= HPDF_Dict_AddName(dict, "S", "Glitter");
		ret |= HPDF_Dict_AddNumber(dict, "Di", 0);
		break;
	case HPDF_TS_GLITTER_DOWN:
		ret |= HPDF_Dict_AddName(dict, "S", "Glitter");
		ret |= HPDF_Dict_AddNumber(dict, "Di", 270);
		break;
	case HPDF_TS_GLITTER_TOP_LEFT_TO_BOTTOM_RIGHT:
		ret |= HPDF_Dict_AddName(dict, "S", "Glitter");
		ret |= HPDF_Dict_AddNumber(dict, "Di", 315);
		break;
	case HPDF_TS_REPLACE:
		ret |= HPDF_Dict_AddName(dict, "S", "R");
		break;
	default:
		ret |= HPDF_SetError(page->error, HPDF_INVALID_PAGE_SLIDESHOW_TYPE, 0);
		break;
	}

	if (ret != HPDF_OK)
		goto Fail;

	if (HPDF_Dict_AddReal(page, "Dur", disp_time) != HPDF_OK)
		goto Fail;

	if ((ret = HPDF_Dict_Add(page, "Trans", dict)) != HPDF_OK)
		return ret;

	return HPDF_OK;

  Fail:
	HPDF_Dict_Free(dict);
	return HPDF_Error_GetCode(page->error);
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_NEW_CONTENT_STREAM)
/*
 *  This function is contributed by Finn Arildsen.
 */
HPDF_STATUS HPDF_Page_New_Content_Stream(HPDF_Page page, HPDF_Dict *new_stream)
{
	/* Call this function to start a new content stream on a page. The
	   handle is returned to new_stream.
	   new_stream can later be used on other pages as a shared content stream;
	   insert using HPDF_Page_Insert_Shared_Content_Stream */

	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_TEXT_OBJECT);
	HPDF_PageAttr attr;
	HPDF_UINT filter;
	HPDF_Array contents_array;

	attr = (HPDF_PageAttr) page->attr;
	filter = attr->contents->filter;

	/* check if there is already an array of contents */
	contents_array = (HPDF_Array) HPDF_Dict_GetItem(page, "Contents", HPDF_OCLASS_ARRAY);
	if (!contents_array)
	{
		HPDF_Error_Reset(page->error);
		/* no contents_array already -- create one
		   and replace current single contents item */
		contents_array = HPDF_Array_New(page->mmgr);
		if (!contents_array)
			return HPDF_Error_GetCode(page->error);
		ret |= HPDF_Array_Add(contents_array, attr->contents);
		ret |= HPDF_Dict_Add(page, "Contents", contents_array);
	}

	/* create new contents stream and add it to the page's contents array */
	attr->contents = HPDF_DictStream_New(page->mmgr, attr->xref);
	attr->contents->filter = filter;
	attr->stream = attr->contents->stream;

	if (!attr->contents)
		return HPDF_Error_GetCode(page->error);

	ret |= HPDF_Array_Add(contents_array, attr->contents);

	/* return the value of the new stream, so that 
	   the application can use it as a shared contents stream */
	if (ret == HPDF_OK && new_stream != NULL)
		*new_stream = attr->contents;

	return ret;
}
#endif


#if defined(PAGEOP_ALL) || defined(PAGE_INSERT_SHARED_CONTENT_STREAM)
/*
 *  This function is contributed by Finn Arildsen.
 */
HPDF_STATUS HPDF_Page_Insert_Shared_Content_Stream(HPDF_Page page, HPDF_Dict shared_stream)
{
	/* Call this function to insert a previously (with HPDF_New_Content_Stream) created content stream
	   as a shared content stream on this page */

	HPDF_STATUS ret = HPDF_Page_CheckState(page, HPDF_GMODE_PAGE_DESCRIPTION | HPDF_GMODE_TEXT_OBJECT);
	HPDF_Array contents_array;

	/* check if there is already an array of contents */
	contents_array = (HPDF_Array) HPDF_Dict_GetItem(page, "Contents", HPDF_OCLASS_ARRAY);
	if (!contents_array)
	{
		HPDF_PageAttr attr;

		HPDF_Error_Reset(page->error);
		/* no contents_array already -- create one
		   and replace current single contents item */
		contents_array = HPDF_Array_New(page->mmgr);
		if (!contents_array)
			return HPDF_Error_GetCode(page->error);
		attr = (HPDF_PageAttr) page->attr;
		ret |= HPDF_Array_Add(contents_array, attr->contents);
		ret |= HPDF_Dict_Add(page, "Contents", contents_array);
	}

	ret |= HPDF_Array_Add(contents_array, shared_stream);

	/* Continue with a new stream */
	ret |= HPDF_Page_New_Content_Stream(page, NULL);

	return ret;
}
#endif
