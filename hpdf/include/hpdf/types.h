/*
 * << Haru Free PDF Library >> -- hpdf_types.h
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

#ifndef _HPDF_TYPES_H
#define _HPDF_TYPES_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/*----- type definition ------------------------------------------------------*/


/*  native OS integer types */
#if defined(__PUREC__) || defined(__MSHORT__)
typedef signed long HPDF_INT;
typedef unsigned long HPDF_UINT;
#else
typedef signed int HPDF_INT;
typedef unsigned int HPDF_UINT;
#endif


/*  32bit integer types
 */
#if defined(__PUREC__) || defined(__MSHORT__)
typedef signed long HPDF_INT32;
typedef unsigned long HPDF_UINT32;
#else
typedef signed int HPDF_INT32;
typedef unsigned int HPDF_UINT32;
#endif


/*  16bit integer types
 */
typedef signed short HPDF_INT16;
typedef unsigned short HPDF_UINT16;


/*  8bit integer types
 */
typedef signed char HPDF_INT8;
typedef unsigned char HPDF_UINT8;


/*  8bit binary types
 */
typedef unsigned char HPDF_BYTE;


/*  float type (32bit IEEE754)
 */
typedef float HPDF_REAL;


/*  double type (64bit IEEE754)
 */
typedef double HPDF_DOUBLE;


/*  boolean type (0: False, !0: True)
 */
typedef signed int HPDF_BOOL;


/*  error-no type (32bit unsigned integer)
 */
typedef unsigned long HPDF_STATUS;


/*  charactor-code type (16bit)
 */
typedef HPDF_UINT16 HPDF_CID;
typedef HPDF_UINT16 HPDF_UNICODE;


/*  HPDF_Point struct
 */
typedef struct _HPDF_Point
{
	HPDF_REAL x;
	HPDF_REAL y;
} HPDF_Point;

typedef struct _HPDF_Rect
{
	HPDF_REAL left;
	HPDF_REAL bottom;
	HPDF_REAL right;
	HPDF_REAL top;
} HPDF_Rect;

/*  HPDF_Point3D struct
*/
typedef struct _HPDF_Point3D
{
	HPDF_REAL x;
	HPDF_REAL y;
	HPDF_REAL z;
} HPDF_Point3D;

typedef struct _HPDF_Rect HPDF_Box;

/* HPDF_Date struct
 */
typedef struct _HPDF_Date
{
	HPDF_INT year;
	HPDF_INT month;
	HPDF_INT day;
	HPDF_INT hour;
	HPDF_INT minutes;
	HPDF_INT seconds;
	char ind;
	HPDF_INT off_hour;
	HPDF_INT off_minutes;
} HPDF_Date;


typedef enum _HPDF_InfoType
{
	/* date-time type parameters */
	HPDF_INFO_CREATION_DATE,
	HPDF_INFO_MOD_DATE,

	/* string type parameters */
	HPDF_INFO_AUTHOR,
	HPDF_INFO_CREATOR,
	HPDF_INFO_PRODUCER,
	HPDF_INFO_TITLE,
	HPDF_INFO_SUBJECT,
	HPDF_INFO_KEYWORDS,
	HPDF_INFO_TRAPPED,
	HPDF_INFO_GTS_PDFX,
	HPDF_INFO_EOF
} HPDF_InfoType;

/* PDF-A Types */

typedef enum _HPDF_PDFA_TYPE
{
	HPDF_PDFA_1A,
	HPDF_PDFA_1B
} HPDF_PDFAType;


typedef enum _HPDF_PdfVer
{
	HPDF_VER_12,
	HPDF_VER_13,
	HPDF_VER_14,
	HPDF_VER_15,
	HPDF_VER_16,
	HPDF_VER_17,
	HPDF_VER_EOF
} HPDF_PDFVer;

typedef enum _HPDF_EncryptMode
{
	HPDF_ENCRYPT_R2 = 2,
	HPDF_ENCRYPT_R3 = 3
} HPDF_EncryptMode;


typedef void (*HPDF_Error_Handler) (HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data);

typedef void *(* HPDF_Alloc_Func) (HPDF_UINT size);


typedef void (*HPDF_Free_Func) (void *aptr);


/*---------------------------------------------------------------------------*/
/*------ text width struct --------------------------------------------------*/

typedef struct _HPDF_TextWidth
{
	HPDF_UINT numchars;

	/* don't use this value (it may be change in the feature).
	   use numspace as alternated. */
	HPDF_UINT numwords;

	HPDF_UINT width;
	HPDF_UINT numspace;
} HPDF_TextWidth;


/*---------------------------------------------------------------------------*/
/*------ dash mode ----------------------------------------------------------*/

typedef struct _HPDF_DashMode
{
	HPDF_UINT16 ptn[8];
	HPDF_UINT num_ptn;
	HPDF_UINT phase;
} HPDF_DashMode;


/*---------------------------------------------------------------------------*/
/*----- HPDF_TransMatrix struct ---------------------------------------------*/

typedef struct _HPDF_TransMatrix
{
	HPDF_REAL a;
	HPDF_REAL b;
	HPDF_REAL c;
	HPDF_REAL d;
	HPDF_REAL x;
	HPDF_REAL y;
} HPDF_TransMatrix;

/*---------------------------------------------------------------------------*/
/*----- HPDF_3DMatrix struct ------------------------------------------------*/

typedef struct _HPDF_3DMatrix
{
	HPDF_REAL a;
	HPDF_REAL b;
	HPDF_REAL c;
	HPDF_REAL d;
	HPDF_REAL e;
	HPDF_REAL f;
	HPDF_REAL g;
	HPDF_REAL h;
	HPDF_REAL i;
	HPDF_REAL tx;
	HPDF_REAL ty;
	HPDF_REAL tz;
} HPDF_3DMatrix;

/*---------------------------------------------------------------------------*/

typedef enum _HPDF_ColorSpace
{
	HPDF_CS_DEVICE_GRAY,
	HPDF_CS_DEVICE_RGB,
	HPDF_CS_DEVICE_CMYK,
	HPDF_CS_CAL_GRAY,
	HPDF_CS_CAL_RGB,
	HPDF_CS_LAB,
	HPDF_CS_ICC_BASED,
	HPDF_CS_SEPARATION,
	HPDF_CS_DEVICE_N,
	HPDF_CS_INDEXED,
	HPDF_CS_PATTERN,
	HPDF_CS_EOF
} HPDF_ColorSpace;

extern const char *const HPDF_COLORSPACE_NAMES[HPDF_CS_EOF];

/*---------------------------------------------------------------------------*/
/*----- HPDF_RGBColor struct ------------------------------------------------*/

typedef struct _HPDF_RGBColor
{
	HPDF_REAL r;
	HPDF_REAL g;
	HPDF_REAL b;
} HPDF_RGBColor;

/*---------------------------------------------------------------------------*/
/*----- HPDF_CMYKColor struct -----------------------------------------------*/

typedef struct _HPDF_CMYKColor
{
	HPDF_REAL c;
	HPDF_REAL m;
	HPDF_REAL y;
	HPDF_REAL k;
} HPDF_CMYKColor;

/*---------------------------------------------------------------------------*/
/*------ The line cap style -------------------------------------------------*/

typedef enum _HPDF_LineCap
{
	HPDF_BUTT_END,
	HPDF_ROUND_END,
	HPDF_PROJECTING_SQUARE_END,
	HPDF_LINECAP_EOF
} HPDF_LineCap;

/*----------------------------------------------------------------------------*/
/*------ The line join style -------------------------------------------------*/

typedef enum _HPDF_LineJoin
{
	HPDF_MITER_JOIN,
	HPDF_ROUND_JOIN,
	HPDF_BEVEL_JOIN,
	HPDF_LINEJOIN_EOF
} HPDF_LineJoin;

/*----------------------------------------------------------------------------*/
/*------ The text rendering mode ---------------------------------------------*/

typedef enum _HPDF_TextRenderingMode
{
	HPDF_FILL,
	HPDF_STROKE,
	HPDF_FILL_THEN_STROKE,
	HPDF_INVISIBLE,
	HPDF_FILL_CLIPPING,
	HPDF_STROKE_CLIPPING,
	HPDF_FILL_STROKE_CLIPPING,
	HPDF_CLIPPING,
	HPDF_RENDERING_MODE_EOF
} HPDF_TextRenderingMode;


typedef enum _HPDF_WritingMode
{
	HPDF_WMODE_HORIZONTAL,
	HPDF_WMODE_VERTICAL,
	HPDF_WMODE_EOF
} HPDF_WritingMode;


typedef enum _HPDF_PageLayout
{
	HPDF_PAGE_LAYOUT_SINGLE,
	HPDF_PAGE_LAYOUT_ONE_COLUMN,
	HPDF_PAGE_LAYOUT_TWO_COLUMN_LEFT,
	HPDF_PAGE_LAYOUT_TWO_COLUMN_RIGHT,
	HPDF_PAGE_LAYOUT_TWO_PAGE_LEFT,
	HPDF_PAGE_LAYOUT_TWO_PAGE_RIGHT,
	HPDF_PAGE_LAYOUT_EOF
} HPDF_PageLayout;


typedef enum _HPDF_PageMode
{
	HPDF_PAGE_MODE_USE_NONE,
	HPDF_PAGE_MODE_USE_OUTLINE,
	HPDF_PAGE_MODE_USE_THUMBS,
	HPDF_PAGE_MODE_FULL_SCREEN,
#if 0
	HPDF_PAGE_MODE_USE_OC,
	HPDF_PAGE_MODE_USE_ATTACHMENTS,
#endif
	HPDF_PAGE_MODE_EOF
} HPDF_PageMode;


typedef enum _HPDF_PageNumStyle
{
	HPDF_PAGE_NUM_STYLE_DECIMAL,
	HPDF_PAGE_NUM_STYLE_UPPER_ROMAN,
	HPDF_PAGE_NUM_STYLE_LOWER_ROMAN,
	HPDF_PAGE_NUM_STYLE_UPPER_LETTERS,
	HPDF_PAGE_NUM_STYLE_LOWER_LETTERS,
	HPDF_PAGE_NUM_STYLE_EOF
} HPDF_PageNumStyle;


typedef enum _HPDF_DestinationType
{
	HPDF_XYZ,
	HPDF_FIT,
	HPDF_FIT_H,
	HPDF_FIT_V,
	HPDF_FIT_R,
	HPDF_FIT_B,
	HPDF_FIT_BH,
	HPDF_FIT_BV,
	HPDF_DST_EOF
} HPDF_DestinationType;


typedef enum _HPDF_AnnotType
{
	HPDF_ANNOT_TEXT,
	HPDF_ANNOT_LINK,
	HPDF_ANNOT_FREE_TEXT,
	HPDF_ANNOT_LINE,
	HPDF_ANNOT_SQUARE,
	HPDF_ANNOT_CIRCLE,
	HPDF_ANNOT_POLYGON,
	HPDF_ANNOT_POLYLINE,
	HPDF_ANNOT_HIGHLIGHT,
	HPDF_ANNOT_UNDERLINE,
	HPDF_ANNOT_SQUIGGLY,
	HPDF_ANNOT_STRIKE_OUT,
	HPDF_ANNOT_STAMP,
	HPDF_ANNOT_CARET,
	HPDF_ANNOT_INK,
	HPDF_ANNOT_POPUP,
	HPDF_ANNOT_FILE_ATTACHMENT,
	HPDF_ANNOT_SOUND,
	HPDF_ANNOT_MOVIE,
	HPDF_ANNOT_WIDGET,
	HPDF_ANNOT_SCREEN,
	HPDF_ANNOT_PRINTERMARK,
	HPDF_ANNOT_TRAPNET,
	HPDF_ANNOT_WATERMARK,
	HPDF_ANNOT_3D,
	HPDF_ANNOT_REDACT,
	HPDF_ANNOT_PROJECTION,
} HPDF_AnnotType;


typedef enum _HPDF_AnnotFlags
{
	HPDF_ANNOT_INVISIBLE,
	HPDF_ANNOT_HIDDEN,
	HPDF_ANNOT_PRINT,
	HPDF_ANNOT_NOZOOM,
	HPDF_ANNOT_NOROTATE,
	HPDF_ANNOT_NOVIEW,
	HPDF_ANNOT_READONLY,
	HPDF_ANNOT_LOCKED,
	HPDF_ANNOT_TOGGLENOVIEW,
	HPDF_ANNOT_LOCKEDCONTENTS
} HPDF_AnnotFlags;


typedef enum _HPDF_AnnotHighlightMode
{
	HPDF_ANNOT_NO_HIGHLIGHT,
	HPDF_ANNOT_INVERT_BOX,
	HPDF_ANNOT_INVERT_BORDER,
	HPDF_ANNOT_DOWN_APPEARANCE,
	HPDF_ANNOT_HIGHLIGHT_MODE_EOF
} HPDF_AnnotHighlightMode;


typedef enum _HPDF_AnnotIcon
{
	HPDF_ANNOT_ICON_COMMENT,
	HPDF_ANNOT_ICON_KEY,
	HPDF_ANNOT_ICON_NOTE,
	HPDF_ANNOT_ICON_HELP,
	HPDF_ANNOT_ICON_NEW_PARAGRAPH,
	HPDF_ANNOT_ICON_PARAGRAPH,
	HPDF_ANNOT_ICON_INSERT,
	HPDF_ANNOT_ICON_EOF
} HPDF_AnnotIcon;

typedef enum _HPDF_AnnotIntent
{
	HPDF_ANNOT_INTENT_FREETEXTCALLOUT,
	HPDF_ANNOT_INTENT_FREETEXTTYPEWRITER,
	HPDF_ANNOT_INTENT_LINEARROW,
	HPDF_ANNOT_INTENT_LINEDIMENSION,
	HPDF_ANNOT_INTENT_POLYGONCLOUD,
	HPDF_ANNOT_INTENT_POLYLINEDIMENSION,
	HPDF_ANNOT_INTENT_POLYGONDIMENSION
} HPDF_AnnotIntent;

typedef enum _HPDF_LineAnnotEndingStyle
{
	HPDF_LINE_ANNOT_NONE,
	HPDF_LINE_ANNOT_SQUARE,
	HPDF_LINE_ANNOT_CIRCLE,
	HPDF_LINE_ANNOT_DIAMOND,
	HPDF_LINE_ANNOT_OPENARROW,
	HPDF_LINE_ANNOT_CLOSEDARROW,
	HPDF_LINE_ANNOT_BUTT,
	HPDF_LINE_ANNOT_ROPENARROW,
	HPDF_LINE_ANNOT_RCLOSEDARROW,
	HPDF_LINE_ANNOT_SLASH
} HPDF_LineAnnotEndingStyle;

typedef enum _HPDF_LineAnnotCapPosition
{
	HPDF_LINE_ANNOT_CAP_INLINE,
	HPDF_LINE_ANNOT_CAP_TOP
} HPDF_LineAnnotCapPosition;

typedef enum _HPDF_StampAnnotName
{
	HPDF_STAMP_ANNOT_APPROVED,
	HPDF_STAMP_ANNOT_EXPERIMENTAL,
	HPDF_STAMP_ANNOT_NOTAPPROVED,
	HPDF_STAMP_ANNOT_ASIS,
	HPDF_STAMP_ANNOT_EXPIRED,
	HPDF_STAMP_ANNOT_NOTFORPUBLICRELEASE,
	HPDF_STAMP_ANNOT_CONFIDENTIAL,
	HPDF_STAMP_ANNOT_FINAL,
	HPDF_STAMP_ANNOT_SOLD,
	HPDF_STAMP_ANNOT_DEPARTMENTAL,
	HPDF_STAMP_ANNOT_FORCOMMENT,
	HPDF_STAMP_ANNOT_TOPSECRET,
	HPDF_STAMP_ANNOT_DRAFT,
	HPDF_STAMP_ANNOT_FORPUBLICRELEASE
} HPDF_StampAnnotName;

/*----------------------------------------------------------------------------*/
/*------ border stype --------------------------------------------------------*/

typedef enum _HPDF_BSSubtype
{
	HPDF_BS_SOLID,
	HPDF_BS_DASHED,
	HPDF_BS_BEVELED,
	HPDF_BS_INSET,
	HPDF_BS_UNDERLINED
} HPDF_BSSubtype;


/*----- blend modes ----------------------------------------------------------*/

typedef enum _HPDF_BlendMode
{
	HPDF_BM_NORMAL,
	HPDF_BM_MULTIPLY,
	HPDF_BM_SCREEN,
	HPDF_BM_OVERLAY,
	HPDF_BM_DARKEN,
	HPDF_BM_LIGHTEN,
	HPDF_BM_COLOR_DODGE,
	HPDF_BM_COLOR_BUM,
	HPDF_BM_HARD_LIGHT,
	HPDF_BM_SOFT_LIGHT,
	HPDF_BM_DIFFERENCE,
	HPDF_BM_EXCLUSHON,
	HPDF_BM_EOF
} HPDF_BlendMode;

/*----- slide show -----------------------------------------------------------*/

typedef enum _HPDF_TransitionStyle
{
	HPDF_TS_WIPE_RIGHT,
	HPDF_TS_WIPE_UP,
	HPDF_TS_WIPE_LEFT,
	HPDF_TS_WIPE_DOWN,
	HPDF_TS_BARN_DOORS_HORIZONTAL_OUT,
	HPDF_TS_BARN_DOORS_HORIZONTAL_IN,
	HPDF_TS_BARN_DOORS_VERTICAL_OUT,
	HPDF_TS_BARN_DOORS_VERTICAL_IN,
	HPDF_TS_BOX_OUT,
	HPDF_TS_BOX_IN,
	HPDF_TS_BLINDS_HORIZONTAL,
	HPDF_TS_BLINDS_VERTICAL,
	HPDF_TS_DISSOLVE,
	HPDF_TS_GLITTER_RIGHT,
	HPDF_TS_GLITTER_DOWN,
	HPDF_TS_GLITTER_TOP_LEFT_TO_BOTTOM_RIGHT,
	HPDF_TS_REPLACE,
	HPDF_TS_EOF
} HPDF_TransitionStyle;

/*----------------------------------------------------------------------------*/

typedef enum _HPDF_PageSizes
{
	HPDF_PAGE_SIZE_LETTER,
	HPDF_PAGE_SIZE_LEGAL,
	HPDF_PAGE_SIZE_A3,
	HPDF_PAGE_SIZE_A4,
	HPDF_PAGE_SIZE_A5,
	HPDF_PAGE_SIZE_B4,
	HPDF_PAGE_SIZE_B5,
	HPDF_PAGE_SIZE_EXECUTIVE,
	HPDF_PAGE_SIZE_US4x6,
	HPDF_PAGE_SIZE_US4x8,
	HPDF_PAGE_SIZE_US5x7,
	HPDF_PAGE_SIZE_COMM10,
	HPDF_PAGE_SIZE_HALF_LETTER,
	HPDF_PAGE_SIZE_DOUBLE_LETTER,
	HPDF_PAGE_SIZE_BROADSHEET,
	HPDF_PAGE_SIZE_EOF
} HPDF_PageSizes;


typedef enum _HPDF_PageDirection
{
	HPDF_PAGE_PORTRAIT,
	HPDF_PAGE_LANDSCAPE
} HPDF_PageDirection;


typedef enum _HPDF_EncoderType
{
	HPDF_ENCODER_TYPE_SINGLE_BYTE,
	HPDF_ENCODER_TYPE_DOUBLE_BYTE,
	HPDF_ENCODER_TYPE_UNINITIALIZED,
	HPDF_ENCODER_UNKNOWN
} HPDF_EncoderType;


typedef enum _HPDF_ByteType
{
	HPDF_BYTE_TYPE_SINGLE,
	HPDF_BYTE_TYPE_LEAD,
	HPDF_BYTE_TYPE_TRIAL,
	HPDF_BYTE_TYPE_UNKNOWN
} HPDF_ByteType;


typedef enum _HPDF_TextAlignment
{
	HPDF_TALIGN_LEFT,
	HPDF_TALIGN_RIGHT,
	HPDF_TALIGN_CENTER,
	HPDF_TALIGN_JUSTIFY
} HPDF_TextAlignment;

/*----------------------------------------------------------------------------*/

/* Name Dictionary values -- see PDF reference section 7.7.4 */
typedef enum _HPDF_NameDictKey
{
	HPDF_NAME_EMBEDDED_FILES,
	HPDF_NAME_DESTS,              /* named destinations */
	HPDF_NAME_AP,                 /* annotation appearance */
	HPDF_NAME_JAVASCRIPT,
	HPDF_NAME_PAGES,
	HPDF_NAME_TEMPLATES,
	HPDF_NAME_IDS,
	HPDF_NAME_URLS,
	HPDF_NAME_ALTERNATEPRESENTATIONS,
	HPDF_NAME_RENDITIONS,
	HPDF_NAME_EOF
} HPDF_NameDictKey;

/*----------------------------------------------------------------------------*/

typedef enum {
	HPDF_PATTERN_TYPE_TILED = 1,
	HPDF_PATTERN_TYPE_SHADING = 2,
} HPDF_PATTERN_TYPE;

typedef enum {
	HPDF_PAINT_TYPE_COLORED = 1,
	HPDF_PAINT_TYPE_UNCOLORED = 2,
} HPDF_PAINT_TYPE;

typedef enum {
	HPDF_TILING_TYPE_CONSTANT = 1,
	HPDF_TILING_TYPE_NO_DISTORTION = 2,
	HPDF_TILING_TYPE_DISTORTION = 3,
} HPDF_TILING_TYPE;

#ifdef __cplusplus
}
#endif

#endif /* _HPDF_TYPES_H */
