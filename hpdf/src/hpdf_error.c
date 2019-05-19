/*
 * << Haru Free PDF Library >> -- hpdf_error.c
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

#include "hpdf_conf.h"
#include "hpdf_utils.h"
#include "hpdf_error.h"
#include "hpdf_consts.h"
#include "hpdf.h"
#include <string.h>

#define _(x) x


void HPDF_Error_Init(HPDF_Error error, void *user_data)
{
	memset(error, 0, sizeof(HPDF_Error_Rec));

	error->user_data = user_data;
}


HPDF_STATUS HPDF_Error_GetCode(HPDF_Error error)
{
	return error->error_no;
}


HPDF_STATUS HPDF_Error_GetDetailCode(HPDF_Error error)
{
	return error->detail_no;
}


void HPDF_CopyError(HPDF_Error dst, HPDF_Error src)
{
	dst->error_no = src->error_no;
	dst->detail_no = src->detail_no;
	dst->error_fn = src->error_fn;
	dst->user_data = src->user_data;
}


HPDF_STATUS HPDF_SetError(HPDF_Error error, HPDF_STATUS error_no, HPDF_STATUS detail_no)
{
	HPDF_PTRACE((" HPDF_SetError: error_no=0x%04X detail_no=0x%04X\n", (unsigned int) error_no, (unsigned int) detail_no));

	error->error_no = error_no;
	error->detail_no = detail_no;

	return error_no;
}


HPDF_STATUS HPDF_CheckError(HPDF_Error error)
{
	HPDF_PTRACE((" HPDF_CheckError: error_no=0x%04X detail_no=0x%04X\n",
				 (unsigned int) error->error_no, (unsigned int) error->detail_no));

	if (error->error_no != HPDF_OK && error->error_fn)
		error->error_fn(error->error_no, error->detail_no, error->user_data);

	return error->error_no;
}


HPDF_STATUS HPDF_RaiseError(HPDF_Error error, HPDF_STATUS error_no, HPDF_STATUS detail_no)
{
	HPDF_SetError(error, error_no, detail_no);

	return HPDF_CheckError(error);
}


void HPDF_Error_Reset(HPDF_Error error)
{
	error->error_no = HPDF_NOERROR;
	error->detail_no = HPDF_NOERROR;
}


const char *HPDF_ErrorStr(HPDF_STATUS error_no)
{
	switch (error_no)
	{
		case HPDF_NOERROR: return _("no error");
		case HPDF_ARRAY_COUNT_ERR: return _("Array count error");
		case HPDF_ARRAY_ITEM_NOT_FOUND: return _("Array item not found");
		case HPDF_ARRAY_ITEM_UNEXPECTED_TYPE: return _("Unexpected array item type");
		case HPDF_BINARY_LENGTH_ERR: return _("Data length > HPDF_LIMIT_MAX_STRING_LEN");
		case HPDF_CANNOT_GET_PALETTE: return _("Cannot get palette data from PNG image");
		case 0x1006: return _("reserved");
		case HPDF_DICT_COUNT_ERR: return _("Dictionary elements > HPDF_LIMIT_MAX_DICT_ELEMENT");
		case HPDF_DICT_ITEM_NOT_FOUND: return _("Dictionary item not found");
		case HPDF_DICT_ITEM_UNEXPECTED_TYPE: return _("Unexpected dictionary item type");
		case HPDF_DICT_STREAM_LENGTH_NOT_FOUND: return _("Stream length not found");
		case HPDF_DOC_ENCRYPTDICT_NOT_FOUND: return _("HPDF_SetEncryptMode() or HPDF_SetPermission() called before password set");
		case HPDF_DOC_INVALID_OBJECT: return _("Invalid object");
		case 0x100d: return _("reserved");
		case HPDF_DUPLICATE_REGISTRATION: return _("Tried to re-register a registered font");
		case HPDF_EXCEED_JWW_CODE_NUM_LIMIT: return _("Cannot register a character to the Japanese word wrap characters list.");
		case 0x1010: return _("reserved");
		case HPDF_ENCRYPT_INVALID_PASSWORD: return _("Invalid encryption password");
		case 0x1012: return _("reserved");
		case HPDF_ERR_UNKNOWN_CLASS: return _("Unknown class");
		case HPDF_EXCEED_GSTATE_LIMIT: return _("Stack depth > HPDF_LIMIT_MAX_GSTATE");
		case HPDF_FAILD_TO_ALLOC_MEM: return _("Memory allocation failed");
		case HPDF_FILE_IO_ERROR: return _("File processing failed");
		case HPDF_FILE_OPEN_ERROR: return _("Cannot open a file");
		case 0x1018: return _("reserved");
		case HPDF_FONT_EXISTS: return _("Tried to load a font that has been registered");
		case HPDF_FONT_INVALID_WIDTHS_TABLE: return _("Font-file format is invalid");
		case HPDF_INVALID_AFM_HEADER: return _("Cannot recognize header of afm file");
		case HPDF_INVALID_ANNOTATION: return _("Specified annotation handle is invalid");
		case 0x101d: return _("reserved");
		case HPDF_INVALID_BIT_PER_COMPONENT: return _("Bit-per-component of a image which was set as mask-image is invalid");
		case HPDF_INVALID_CHAR_MATRICS_DATA: return _("Cannot recognize char-matrics-data of afm file");
		case HPDF_INVALID_COLOR_SPACE: return _("Invalid color_space parameter");
		case HPDF_INVALID_COMPRESSION_MODE: return _("Invalid value set when invoking HPDF_SetCommpressionMode()");
		case HPDF_INVALID_DATE_TIME: return _("An invalid date-time value was set");
		case HPDF_INVALID_DESTINATION: return _("HPDF_INVALID_DESTINATION");
		case 0x1024: return _("reserved");
		case HPDF_INVALID_DOCUMENT: return _("An invalid document handle was set");
		case HPDF_INVALID_DOCUMENT_STATE: return _("Function invalid in the present state was invoked");
		case HPDF_INVALID_ENCODER: return _("An invalid encoder handle was set");
		case HPDF_INVALID_ENCODER_TYPE: return _("Combination between font and encoder is wrong");
		case 0x1029: return _("reserved");
		case 0x102a: return _("reserved");
		case HPDF_INVALID_ENCODING_NAME: return _("An Invalid encoding name is specified");
		case HPDF_INVALID_ENCRYPT_KEY_LEN: return _("Encryption key length is invalid");
		case HPDF_INVALID_FONTDEF_DATA: return _("An invalid font handle was set");
		case HPDF_INVALID_FONTDEF_TYPE: return _("Invalid font definition type");
		case HPDF_INVALID_FONT_NAME: return _("Font with the specified name is not found");
		case HPDF_INVALID_IMAGE: return _("Unsupported image format");
		case HPDF_INVALID_JPEG_DATA: return _("Invalie Jpeg data");
		case HPDF_INVALID_N_DATA: return _("Cannot read a postscript-name from an afm file");
		case HPDF_INVALID_OBJECT: return _("An invalid object is set");
		case HPDF_INVALID_OBJ_ID: return _("An invalid object id is set");
		case HPDF_INVALID_OPERATION: return _("Invoked HPDF_Image_SetColorMask() against the image-object which was set a mask-image");
		case HPDF_INVALID_OUTLINE: return _("An invalid outline-handle was specified");
		case HPDF_INVALID_PAGE: return _("An invalid page-handle was specified");
		case HPDF_INVALID_PAGES: return _("An invalid pages-handle was specified");
		case HPDF_INVALID_PARAMETER: return _("An invalid value is set");
		case 0x103a: return _("reserved");
		case HPDF_INVALID_PNG_IMAGE: return _("Invalid PNG image format");
		case HPDF_INVALID_STREAM: return _("An invalid stream handle was specified");
		case HPDF_MISSING_FILE_NAME_ENTRY: return _("\"_FILE_NAME\" entry for delayed loading is missing");
		case 0x103e: return _("reserved");
		case HPDF_INVALID_TTC_FILE: return _("Invalid .TTC file format");
		case HPDF_INVALID_TTC_INDEX: return _("Index parameter > number of included fonts");
		case HPDF_INVALID_WX_DATA: return _("Cannot read a width-data from an afm file");
		case HPDF_ITEM_NOT_FOUND: return _("Item not found");
		case HPDF_LIBPNG_ERROR: return _("Error returned from PNGLIB while loading image");
		case HPDF_NAME_INVALID_VALUE: return _("Invalid name was used");
		case HPDF_NAME_OUT_OF_RANGE: return _("Name out of range");
		case 0x1046: return _("reserved");
		case 0x1047: return _("reserved");
		case HPDF_PAGE_INVALID_PARAM_COUNT: return _("Invalid page param count");
		case HPDF_PAGES_MISSING_KIDS_ENTRY: return _("Pages is missing kids entry");
		case HPDF_PAGE_CANNOT_FIND_OBJECT: return _("Cannot find page object");
		case HPDF_PAGE_CANNOT_GET_ROOT_PAGES: return _("Cannot get root pages");
		case HPDF_PAGE_CANNOT_RESTORE_GSTATE: return _("There are no graphics-states to be restored");
		case HPDF_PAGE_CANNOT_SET_PARENT: return _("Cannot set page parent");
		case HPDF_PAGE_FONT_NOT_FOUND: return _("The current font is not set");
		case HPDF_PAGE_INVALID_FONT: return _("An invalid font-handle was specified");
		case HPDF_PAGE_INVALID_FONT_SIZE: return _("An invalid font-size was set");
		case HPDF_PAGE_INVALID_GMODE: return _("Invalid page graphics mode");
		case HPDF_PAGE_INVALID_INDEX: return _("Invaid page index");
		case HPDF_PAGE_INVALID_ROTATE_VALUE: return _("Specified value is not multiple of 90");
		case HPDF_PAGE_INVALID_SIZE: return _("An invalid page-size was set");
		case HPDF_PAGE_INVALID_XOBJECT: return _("An invalid image-handle was set");
		case HPDF_PAGE_OUT_OF_RANGE: return _("The specified value is out of range");
		case HPDF_REAL_OUT_OF_RANGE: return _("The specified value is out of range");
		case HPDF_STREAM_EOF: return _("Unexpected EOF marker was detected");
		case HPDF_STREAM_READLN_CONTINUE: return _("Continue reading line");
		case 0x105a: return _("reserved");
		case HPDF_STRING_OUT_OF_RANGE: return _("The length of the text is too long");
		case HPDF_THIS_FUNC_WAS_SKIPPED: return _("Function not executed because of other errors");
		case HPDF_TTF_CANNOT_EMBEDDING_FONT: return _("Font cannot be embedded. license restriction");
		case HPDF_TTF_INVALID_CMAP: return _("Unsupported ttf format (cannot find unicode cmap)");
		case HPDF_TTF_INVALID_FOMAT: return _("Unsupported ttf format");
		case HPDF_TTF_MISSING_TABLE: return _("Unsupported ttf format (cannot find a necessary table)");
		case HPDF_UNSUPPORTED_FONT_TYPE: return _("Unsupported font type");
		case HPDF_UNSUPPORTED_FUNC: return _("Library not configured to use PNGLIB");
		case HPDF_UNSUPPORTED_JPEG_FORMAT: return _("Unsupported JPEG format");
		case HPDF_UNSUPPORTED_TYPE1_FONT: return _("Failed to parse .PFB file");
		case HPDF_XREF_COUNT_ERR: return _("XRef count error");
		case HPDF_ZLIB_ERROR: return _("Error while executing ZLIB function");
		case HPDF_INVALID_PAGE_INDEX: return _("An invalid page index was passed");
		case HPDF_INVALID_URI: return _("An invalid URI was set");
		case HPDF_PAGE_LAYOUT_OUT_OF_RANGE: return _("An invalid page-layout was set");
		case 0x106a: return _("reserved");
		case 0x106b: return _("reserved");
		case 0x106c: return _("reserved");
		case 0x106d: return _("reserved");
		case 0x106e: return _("reserved");
		case 0x106f: return _("reserved");
		case HPDF_PAGE_MODE_OUT_OF_RANGE: return _("An invalid page-mode was set");
		case HPDF_PAGE_NUM_STYLE_OUT_OF_RANGE: return _("An invalid page-num-style was set");
		case HPDF_ANNOT_INVALID_ICON: return _("An invalid icon was set");
		case HPDF_ANNOT_INVALID_BORDER_STYLE: return _("An invalid border-style was set");
		case HPDF_PAGE_INVALID_DIRECTION: return _("An invalid page-direction was set");
		case HPDF_INVALID_FONT: return _("An invalid font-handle was specified");
		case HPDF_PAGE_INSUFFICIENT_SPACE: return _("Insufficient space on page");
		case HPDF_PAGE_INVALID_DISPLAY_TIME: return _("Invalid display time");
		case HPDF_PAGE_INVALID_TRANSITION_TIME: return _("Invalid page transition time");
		case HPDF_INVALID_PAGE_SLIDESHOW_TYPE: return _("Invalid psgae slideshow type");
		case HPDF_EXT_GSTATE_OUT_OF_RANGE: return _("ExtGState out of range");
		case HPDF_INVALID_EXT_GSTATE: return _("Invalid ExtGState");
		case HPDF_EXT_GSTATE_READ_ONLY: return _("ExtGStateis read-only");
		case HPDF_INVALID_U3D_DATA: return _("Invalid U3D data");
		case 0x1084: return _("reserved");
		case HPDF_INVALID_ICC_COMPONENT_NUM: return _("Invalid ICC component number");
	}
	return _("unknown error");
}
