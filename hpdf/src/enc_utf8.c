/*
 * << Haru Free PDF Library >> -- hpdf_encoder_utf.c
 *
 * URL: http://libharu.org
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 * Copyright (c) 2007-2008 Antony Dovgal <tony@daylessday.org>
 * Copyright (c) 2010      Sergey Konovalov <webmaster@crynet.ru>
 * Copyright (c) 2011      Koen Deforche <koen@emweb.be>
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
#include "hpdf/encoder.h"
#include "hpdf.h"
#include <string.h>

typedef struct _UTF8_EncoderAttr_Rec *UTF8_EncoderAttr;
typedef struct _UTF8_EncoderAttr_Rec
{
	HPDF_BYTE current_byte;
	HPDF_BYTE end_byte;
	HPDF_BYTE utf8_bytes[8];
} UTF8_EncoderAttr_Rec;

static const HPDF_CidRange_Rec UTF8_NOTDEF_RANGE = { 0x0000, 0x001F, 1 };
static const HPDF_CidRange_Rec UTF8_SPACE_RANGE = { 0x0000, 0xFFFF, 0 };

static const HPDF_CidRange_Rec UTF8_CID_RANGE[] = {
	{0x0000, 0xFFFF, 0x0},
	{0xFFFF, 0xFFFF, 0x0}
};



/*--------------------------------------------------------------------------*/


/*
 * This function is taken from hpdf_encoder_utf8.c, originally submitted
 * to libharu by 'Mirco'
 */
static HPDF_ByteType UTF8_Encoder_ByteType_Func(HPDF_Encoder encoder, HPDF_ParseText_Rec *state)
{
	/* This function is supposed to increment state->index */
	/* Not logical ! (look at function HPDF_String_Write in hpdf_string.c) */

	/*
	 * When HPDF_BYTE_TYPE_SINGLE is returned, the current byte is the
	 *   CODE argument in call ToUnicode_Func
	 * When HPDF_BYTE_TYPE_LEAD is returned, the current byte (msb) and the
	 *   next byte (lsb) is the CODE arguement in call ToUnicode_Func
	 * When HPDF_BYTE_TYPE_TRIAL is returned, the current byte is ignored
	 */

	HPDF_CMapEncoderAttr encoder_attr;
	HPDF_BYTE byte;
	UTF8_EncoderAttr utf8_attr;

	encoder_attr = (HPDF_CMapEncoderAttr) encoder->attr;
	utf8_attr = (UTF8_EncoderAttr) ((void *) encoder_attr->cid_map[0]);

	if (state->index == 0)
	{
		/* First byte, initialize. */
		utf8_attr->current_byte = 0;
	}

	byte = state->text[state->index];
	state->index++;

	if (utf8_attr->current_byte == 0)
	{
		utf8_attr->utf8_bytes[0] = byte;
		utf8_attr->current_byte = 1;

		if (!(byte & 0x80))
		{
			utf8_attr->current_byte = 0;
			utf8_attr->end_byte = 0;
			return HPDF_BYTE_TYPE_SINGLE;
		}

		if ((byte & 0xf8) == 0xf0)
			utf8_attr->end_byte = 3;
		else if ((byte & 0xf0) == 0xe0)
			utf8_attr->end_byte = 2;
		else if ((byte & 0xe0) == 0xc0)
			utf8_attr->end_byte = 1;
		else
			utf8_attr->current_byte = 0;	/* ERROR, skip this byte */
	} else
	{
		utf8_attr->utf8_bytes[utf8_attr->current_byte] = byte;
		if (utf8_attr->current_byte == utf8_attr->end_byte)
		{
			utf8_attr->current_byte = 0;
			return HPDF_BYTE_TYPE_SINGLE;
		}

		utf8_attr->current_byte++;
	}

	return HPDF_BYTE_TYPE_TRIAL;
}


/*
 * This function is taken from hpdf_encoder_utf8.c, originally submitted
 * to libharu by 'Mirco'
 */
static HPDF_UNICODE UTF8_Encoder_ToUnicode_Func(HPDF_Encoder encoder, HPDF_UINT16 code)
{
	/* Supposed to convert CODE to unicode. */
	/* This function is allways called after ByteType_Func. */
	/* ByteType_Func recognizes the utf-8 bytes belonging to one character. */

	HPDF_CMapEncoderAttr encoder_attr;
	UTF8_EncoderAttr utf8_attr;
	HPDF_UINT val;

	HPDF_UNUSED(code);
	encoder_attr = (HPDF_CMapEncoderAttr) encoder->attr;
	utf8_attr = (UTF8_EncoderAttr) ((void *) encoder_attr->cid_map[0]);

	switch (utf8_attr->end_byte)
	{
	case 3:
		val = (((HPDF_UINT)utf8_attr->utf8_bytes[0] & 0x7) << 18) |
			(((HPDF_UINT)utf8_attr->utf8_bytes[1] & 0x3f) << 12) |
			(((HPDF_UINT)utf8_attr->utf8_bytes[2] & 0x3f) << 6) |
			(((HPDF_UINT)utf8_attr->utf8_bytes[3] & 0x3f));
		break;
	case 2:
		val = (((HPDF_UINT)utf8_attr->utf8_bytes[0] & 0xf) << 12) |
			(((HPDF_UINT)utf8_attr->utf8_bytes[1] & 0x3f) << 6) |
			(((HPDF_UINT)utf8_attr->utf8_bytes[2] & 0x3f));
		break;
	case 1:
		val = (((HPDF_UINT)utf8_attr->utf8_bytes[0] & 0x1f) << 6) |
			(((HPDF_UINT)utf8_attr->utf8_bytes[1] & 0x3f));
		break;
	case 0:
		val = utf8_attr->utf8_bytes[0];
		break;
	default:
		val = 32;						/* Unknown character */
		break;
	}

	if (val > 65535L)					/* Convert everything outside UCS-2 to space */
		val = 32;

	return val;
}


static char *UTF8_Encoder_EncodeText_Func(HPDF_Encoder encoder, HPDF_MMgr mmgr, const char *text, HPDF_UINT len, HPDF_UINT *length)
{
	char *result = (char *) HPDF_DirectAlloc(mmgr, len * 2);
	char *c = result;
	HPDF_ParseText_Rec parse_state;
	HPDF_UINT i;

	if (result == NULL)
		return NULL;
	HPDF_Encoder_SetParseText(encoder, &parse_state, (const HPDF_BYTE *) text, len);

	for (i = 0; i < len; i++)
	{
		HPDF_UNICODE tmp_unicode;
		HPDF_ByteType btype = HPDF_Encoder_ByteType(encoder, &parse_state);

		if (btype != HPDF_BYTE_TYPE_TRIAL)
		{
			tmp_unicode = HPDF_Encoder_ToUnicode(encoder, 0);

			HPDF_UInt16Swap(&tmp_unicode);
			memcpy(c, &tmp_unicode, 2);
			c += 2;
		}
	}

	*length = (HPDF_UINT)(c - result);

	return result;
}


static HPDF_STATUS UTF8_Init(HPDF_Encoder encoder)
{
	HPDF_CMapEncoderAttr attr;
	HPDF_STATUS ret;

	if ((ret = HPDF_CMapEncoder_InitAttr(encoder)) != HPDF_OK)
		return ret;

	/*
	 * We override these two
	 */
	encoder->byte_type_fn = UTF8_Encoder_ByteType_Func;
	encoder->to_unicode_fn = UTF8_Encoder_ToUnicode_Func;
	encoder->encode_text_fn = UTF8_Encoder_EncodeText_Func;

	attr = (HPDF_CMapEncoderAttr) encoder->attr;

	if (HPDF_CMapEncoder_AddCMap(encoder, UTF8_CID_RANGE) != HPDF_OK)
		return encoder->error->error_no;

	if (HPDF_CMapEncoder_AddCodeSpaceRange(encoder, &UTF8_SPACE_RANGE) != HPDF_OK)
		return encoder->error->error_no;

	if (HPDF_CMapEncoder_AddNotDefRange(encoder, &UTF8_NOTDEF_RANGE) != HPDF_OK)
		return encoder->error->error_no;

	attr->is_lead_byte_fn = NULL;
	attr->is_trial_byte_fn = NULL;

	strcpy(attr->registry, "Adobe");
	strcpy(attr->ordering, "Identity-H");
	attr->suppliment = 0;
	attr->writing_mode = HPDF_WMODE_HORIZONTAL;

	/* Not sure about this
	   attr->uid_offset = 0;
	   attr->xuid[0] = 0;
	   attr->xuid[1] = 0;
	   attr->xuid[2] = 0;
	 */

	encoder->type = HPDF_ENCODER_TYPE_DOUBLE_BYTE;

	return HPDF_OK;
}

/*--------------------------------------------------------------------------*/

HPDF_STATUS HPDF_UseUTF8Encodings(HPDF_Doc pdf)
{
	HPDF_Encoder encoder;
	HPDF_STATUS ret;

	if (!HPDF_HasDoc(pdf))
		return HPDF_INVALID_DOCUMENT;

	encoder = HPDF_CMapEncoder_New(pdf->mmgr, "UTF-8", UTF8_Init);

	if ((ret = HPDF_Doc_RegisterEncoder(pdf, encoder)) != HPDF_OK)
		return ret;

	return HPDF_OK;
}
