/*
 * << Haru Free PDF Library >> -- hpdf_streams.c
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
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "hpdf/conf.h"
#include <errno.h>
#include "hpdf/conf.h"
#include "hpdf/utils.h"
#include "hpdf/consts.h"
#include "hpdf/utils.h"
#include "hpdf/streams.h"
#include <string.h>

#ifndef LIBHPDF_HAVE_NOZLIB
#include <zlib.h>
#endif


/*
 *  HPDF_Stream_Read
 *
 *  stream : Pointer to a HPDF_Stream object.
 *  ptr : Pointer to a buffer to copy read data.
 *  size : Pointer to a variable which indecates buffer size.
 *
 *  HPDF_Stream_Read returns HPDF_OK when success. On failer, it returns
 *  error-code returned by reading function of this stream.
 *
 */
HPDF_STATUS HPDF_Stream_Read(HPDF_Stream stream, void *ptr, HPDF_UINT *size)
{
	if (!(stream->read_fn))
		return HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);

	/*
	   if (HPDF_Error_GetCode(stream->error) != HPDF_NOERROR)
	   return HPDF_THIS_FUNC_WAS_SKIPPED;
	 */

	return stream->read_fn(stream, ptr, size);
}


/*
 *  HPDF_Stream_ReadLn
 *
 *  stream : Pointer to a HPDF_Stream object.
 *  s : Pointer to a buffer to copy read data.
 *  size : buffer-size of s.
 *
 *  Read from stream until the buffer is exhausted or line-feed charactor is
 *  read.
 *
 */
HPDF_STATUS HPDF_Stream_ReadLn(HPDF_Stream stream, char *s, HPDF_UINT *size)
{
	char buf[HPDF_STREAM_BUF_SIZ];
	HPDF_UINT r_size = *size;
	HPDF_UINT read_size = HPDF_STREAM_BUF_SIZ;

	if (!stream)
		return HPDF_INVALID_PARAMETER;

	if (!s || *size == 0)
		return HPDF_SetError(stream->error, HPDF_INVALID_PARAMETER, 0);

	if (!stream->seek_fn || !stream->read_fn)
		return HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);

	if (r_size < HPDF_STREAM_BUF_SIZ)
		read_size = r_size;

	*size = 0;

	while (r_size > 1)
	{
		char *pbuf = buf;
		HPDF_STATUS ret = HPDF_Stream_Read(stream, buf, &read_size);

		if (ret != HPDF_OK && read_size == 0)
			return ret;

		r_size -= read_size;

		while (read_size > 0)
		{
			if (*pbuf == 0x0A || *pbuf == 0x0D)
			{
				*s = 0;
				read_size--;

				/* handling CR-LF marker */
				if (*pbuf == 0x0D || read_size > 1)
				{
					pbuf++;

					if (*pbuf == 0x0A)
						read_size--;
				}

				if (read_size > 0)
					return HPDF_Stream_Seek(stream, 0 - read_size, HPDF_SEEK_CUR);
				else
					return HPDF_OK;
			}

			*s++ = *pbuf++;
			read_size--;
			(*size)++;
		}

		if (r_size < HPDF_STREAM_BUF_SIZ)
			read_size = r_size;
		else
			read_size = HPDF_STREAM_BUF_SIZ;

		if (ret == HPDF_STREAM_EOF)
			return HPDF_STREAM_EOF;
	}

	*s = 0;

	return HPDF_STREAM_READLN_CONTINUE;
}


/*
 * HPDF_Stream_Write
 *
 *  stream : Pointer to a HPDF_Stream object.
 *  ptr : Pointer to a buffer to write.
 *  siz : The size of buffer to write.
 *
 *  HPDF_Stream_Write returns HPDF_OK when success. On failer, it returns
 *  error-code returned by writing function of this stream.
 *
 */
HPDF_STATUS HPDF_Stream_Write(HPDF_Stream stream, const void *ptr, HPDF_UINT size)
{
	HPDF_STATUS ret;

	if (!(stream->write_fn))
		return HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);

	/*
	   if (HPDF_Error_GetCode(stream->error) != HPDF_NOERROR)
	   return HPDF_THIS_FUNC_WAS_SKIPPED;
	 */

	ret = stream->write_fn(stream, ptr, size);

	if (ret != HPDF_OK)
		return ret;

	stream->size += size;

	return HPDF_OK;
}


HPDF_STATUS HPDF_Stream_WriteChar(HPDF_Stream stream, char value)
{
	return HPDF_Stream_Write(stream, &value, sizeof(char));
}


HPDF_STATUS HPDF_Stream_WriteStr(HPDF_Stream stream, const char *value)
{
	HPDF_UINT len = HPDF_StrLen(value, -1);

	return HPDF_Stream_Write(stream, value, len);
}


HPDF_STATUS HPDF_Stream_WriteUChar(HPDF_Stream stream, HPDF_BYTE value)
{
	return HPDF_Stream_Write(stream, &value, sizeof(HPDF_BYTE));
}


HPDF_STATUS HPDF_Stream_WriteInt(HPDF_Stream stream, HPDF_INT value)
{
	char buf[HPDF_INT_LEN + 1];

	char *p = HPDF_IToA(buf, value, buf + HPDF_INT_LEN);

	return HPDF_Stream_Write(stream, buf, (HPDF_UINT) (p - buf));
}


HPDF_STATUS HPDF_Stream_WriteUInt(HPDF_Stream stream, HPDF_UINT value)
{
	return HPDF_Stream_WriteInt(stream, value);
}


HPDF_STATUS HPDF_Stream_WriteReal(HPDF_Stream stream, HPDF_REAL value)
{
	char buf[HPDF_REAL_LEN + 1];

	char *p = HPDF_FToA(buf, value, buf + HPDF_REAL_LEN);

	return HPDF_Stream_Write(stream, buf, (HPDF_UINT) (p - buf));
}


void HPDF_Stream_Free(HPDF_Stream stream)
{
	if (!stream)
		return;

	if (stream->free_fn)
		stream->free_fn(stream);

	stream->sig_bytes = 0;

	HPDF_FreeMem(stream->mmgr, stream);
}


HPDF_STATUS HPDF_Stream_Seek(HPDF_Stream stream, HPDF_INT pos, HPDF_WhenceMode mode)
{
	if (!(stream->seek_fn))
		return HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);

	if (HPDF_Error_GetCode(stream->error) != 0)
		return HPDF_THIS_FUNC_WAS_SKIPPED;

	return stream->seek_fn(stream, pos, mode);
}


HPDF_INT32 HPDF_Stream_Tell(HPDF_Stream stream)
{
	if (!(stream->tell_fn))
	{
		HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);
		return -1;
	}
	if (HPDF_Error_GetCode(stream->error) != 0)
		return -1;

	return stream->tell_fn(stream);
}


HPDF_UINT32 HPDF_Stream_Size(HPDF_Stream stream)
{
	if (stream->write_fn)
		return stream->size;

	if (!(stream->size_fn))
	{
		HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);
		return 0;
	}

	if (HPDF_Error_GetCode(stream->error) != 0)
		return 0;

	return stream->size_fn(stream);
}


HPDF_STATUS HPDF_Stream_WriteEscapeName(HPDF_Stream stream, const char *value)
{
	char tmp_char[HPDF_LIMIT_MAX_NAME_LEN * 3 + 2];
	HPDF_UINT len;
	HPDF_UINT i;
	const HPDF_BYTE *pos1;
	char *pos2;

	len = HPDF_StrLen(value, HPDF_LIMIT_MAX_NAME_LEN);
	pos1 = (const HPDF_BYTE *) value;
	pos2 = tmp_char;

	*pos2++ = '/';
	for (i = 0; i < len; i++)
	{
		HPDF_BYTE c = *pos1++;

		if (HPDF_NEEDS_ESCAPE(c))
		{
			*pos2++ = '#';
			*pos2 = c >> 4;
			if (*pos2 <= 9)
				*pos2 += 0x30;
			else
				*pos2 += 0x41 - 10;
			pos2++;

			*pos2 = c & 0x0f;
			if (*pos2 <= 9)
				*pos2 += 0x30;
			else
				*pos2 += 0x41 - 10;
			pos2++;
		} else
		{
			*pos2++ = c;
		}
	}
	*pos2 = 0;

	return HPDF_Stream_Write(stream, tmp_char, HPDF_StrLen(tmp_char, -1));
}


HPDF_STATUS HPDF_Stream_WriteEscapeText2(HPDF_Stream stream, const char *text, HPDF_UINT len)
{
	HPDF_BYTE buf[HPDF_TEXT_DEFAULT_LEN];
	HPDF_UINT idx = 0;
	HPDF_UINT i;
	const char *p = text;
	HPDF_STATUS ret;

	/* The following block is commented out because it violates "PDF Spec 7.3.4.2 Literal Strings". 
	 * It states that the two matching parentheses must still be present to represent an empty 
	 * string of zero length. 
	 */
	/*
	   if (!len)
	   return HPDF_OK;
	 */

	buf[idx++] = '(';

	for (i = 0; i < len; i++)
	{
		HPDF_BYTE c = *p++;

		if (HPDF_NEEDS_ESCAPE(c))
		{
			buf[idx++] = '\\';

			buf[idx] = (c >> 6);
			buf[idx] += 0x30;
			idx++;
			buf[idx] = ((c & 0x38) >> 3);
			buf[idx] += 0x30;
			idx++;
			buf[idx] = (c & 0x07);
			buf[idx] += 0x30;
			idx++;
		} else
		{
			buf[idx++] = c;
		}

		if (idx > HPDF_TEXT_DEFAULT_LEN - 4)
		{
			ret = HPDF_Stream_Write(stream, buf, idx);
			if (ret != HPDF_OK)
				return ret;
			idx = 0;
		}
	}
	buf[idx++] = ')';

	ret = HPDF_Stream_Write(stream, buf, idx);

	return ret;
}


HPDF_STATUS HPDF_Stream_WriteEscapeText(HPDF_Stream stream, const char *text)
{
	HPDF_UINT len;

	len = text == NULL ? 0 : HPDF_StrLen(text, HPDF_LIMIT_MAX_STRING_LEN);

	return HPDF_Stream_WriteEscapeText2(stream, text, len);
}


HPDF_STATUS HPDF_Stream_WriteBinary(HPDF_Stream stream, const void *data, HPDF_UINT len, HPDF_Encrypt e)
{
	char buf[HPDF_TEXT_DEFAULT_LEN];
	HPDF_BYTE ebuf[HPDF_TEXT_DEFAULT_LEN];
	HPDF_BYTE *pbuf = NULL;
	HPDF_BOOL flg = HPDF_FALSE;
	HPDF_UINT idx = 0;
	HPDF_UINT i;
	const HPDF_BYTE *p;
	HPDF_STATUS ret = HPDF_OK;

	if (e)
	{
		if (len <= HPDF_TEXT_DEFAULT_LEN)
		{
			pbuf = ebuf;
		} else
		{
			pbuf = (HPDF_BYTE *) HPDF_GetMem(stream->mmgr, len);
			flg = HPDF_TRUE;
		}

		HPDF_Encrypt_CryptBuf(e, data, pbuf, len);
		p = pbuf;
	} else
	{
		p = (const HPDF_BYTE *)data;
	}

	for (i = 0; i < len; i++, p++)
	{
		HPDF_BYTE c = *p >> 4;

		if (c <= 9)
			c += 0x30;
		else
			c += 0x41 - 10;
		buf[idx++] = c;

		c = *p & 0x0f;
		if (c <= 9)
			c += 0x30;
		else
			c += 0x41 - 10;
		buf[idx++] = c;

		if (idx > HPDF_TEXT_DEFAULT_LEN - 2)
		{
			ret = HPDF_Stream_Write(stream, buf, idx);
			if (ret != HPDF_OK)
			{
				if (flg)
					HPDF_FreeMem(stream->mmgr, pbuf);
				return ret;
			}
			idx = 0;
		}
	}

	if (idx > 0)
	{
		ret = HPDF_Stream_Write(stream, buf, idx);
	}

	if (flg)
		HPDF_FreeMem(stream->mmgr, pbuf);

	return ret;
}


static HPDF_STATUS HPDF_Stream_WriteToStreamWithDeflate(HPDF_Stream src, HPDF_Stream dst, HPDF_Encrypt e)
{
#ifndef LIBHPDF_HAVE_NOZLIB

#define DEFLATE_BUF_SIZ  ((HPDF_INT)(HPDF_STREAM_BUF_SIZ * 1.1) + 13)

	HPDF_STATUS ret;
	HPDF_BOOL flg;

	z_stream strm;
	Bytef inbuf[HPDF_STREAM_BUF_SIZ];
	Bytef otbuf[DEFLATE_BUF_SIZ];
	HPDF_BYTE ebuf[DEFLATE_BUF_SIZ];

	/* initialize input stream */
	ret = HPDF_Stream_Seek(src, 0, HPDF_SEEK_SET);
	if (ret != HPDF_OK)
		return ret;

	/* initialize decompression stream. */
	memset(&strm, 0, sizeof(z_stream));
	strm.next_out = otbuf;
	strm.avail_out = (uInt)DEFLATE_BUF_SIZ;

	ret = deflateInit_(&strm, Z_DEFAULT_COMPRESSION, ZLIB_VERSION, (int)sizeof(z_stream));
	if (ret != Z_OK)
		return HPDF_SetError(src->error, HPDF_ZLIB_ERROR, ret);

	strm.next_in = inbuf;
	strm.avail_in = 0;

	flg = HPDF_FALSE;
	for (;;)
	{
		HPDF_UINT size = HPDF_STREAM_BUF_SIZ;

		ret = HPDF_Stream_Read(src, inbuf, &size);

		strm.next_in = inbuf;
		strm.avail_in = (uInt)size;

		if (ret != HPDF_OK)
		{
			if (ret == HPDF_STREAM_EOF)
			{
				flg = HPDF_TRUE;
				if (size == 0)
					break;
			} else
			{
				deflateEnd(&strm);
				return ret;
			}
		}

		while (strm.avail_in > 0)
		{
			ret = deflate(&strm, Z_NO_FLUSH);
			if (ret != Z_OK && ret != Z_STREAM_END)
			{
				deflateEnd(&strm);
				return HPDF_SetError(src->error, HPDF_ZLIB_ERROR, ret);
			}

			if (strm.avail_out == 0)
			{
				if (e)
				{
					HPDF_Encrypt_CryptBuf(e, otbuf, ebuf, DEFLATE_BUF_SIZ);
					ret = HPDF_Stream_Write(dst, ebuf, DEFLATE_BUF_SIZ);
				} else
				{
					ret = HPDF_Stream_Write(dst, otbuf, DEFLATE_BUF_SIZ);
				}

				if (ret != HPDF_OK)
				{
					deflateEnd(&strm);
					return HPDF_SetError(src->error, HPDF_ZLIB_ERROR, ret);
				}

				strm.next_out = otbuf;
				strm.avail_out = (uInt)DEFLATE_BUF_SIZ;
			}
		}

		if (flg)
			break;
	}

	flg = HPDF_FALSE;
	for (;;)
	{
		ret = deflate(&strm, Z_FINISH);
		if (ret != Z_OK && ret != Z_STREAM_END)
		{
			deflateEnd(&strm);
			return HPDF_SetError(src->error, HPDF_ZLIB_ERROR, ret);
		}

		if (ret == Z_STREAM_END)
			flg = HPDF_TRUE;

		if (strm.avail_out < DEFLATE_BUF_SIZ)
		{
			HPDF_UINT osize = DEFLATE_BUF_SIZ - strm.avail_out;

			if (e)
			{
				HPDF_Encrypt_CryptBuf(e, otbuf, ebuf, osize);
				ret = HPDF_Stream_Write(dst, ebuf, osize);
			} else
			{
				ret = HPDF_Stream_Write(dst, otbuf, osize);
			}

			if (ret != HPDF_OK)
			{
				deflateEnd(&strm);
				return HPDF_SetError(src->error, HPDF_ZLIB_ERROR, ret);
			}

			strm.next_out = otbuf;
			strm.avail_out = (uInt)DEFLATE_BUF_SIZ;
		}

		if (flg)
			break;
	}

	deflateEnd(&strm);
	return HPDF_OK;
#else
	HPDF_UNUSED(e);
	HPDF_UNUSED(dst);
	HPDF_UNUSED(src);
	return HPDF_UNSUPPORTED_FUNC;
#endif /* LIBHPDF_HAVE_NOZLIB */
}


HPDF_STATUS HPDF_Stream_WriteToStream(HPDF_Stream src, HPDF_Stream dst, HPDF_UINT filter, HPDF_Encrypt e)
{
	HPDF_STATUS ret;
	HPDF_BYTE buf[HPDF_STREAM_BUF_SIZ];
	HPDF_BYTE ebuf[HPDF_STREAM_BUF_SIZ];
	HPDF_BOOL flg;

	HPDF_UNUSED(filter);

	if (!dst || !(dst->write_fn))
	{
		HPDF_SetError(src->error, HPDF_INVALID_OBJECT, 0);
		return HPDF_INVALID_OBJECT;
	}

	if (HPDF_Error_GetCode(src->error) != HPDF_NOERROR || HPDF_Error_GetCode(dst->error) != HPDF_NOERROR)
		return HPDF_THIS_FUNC_WAS_SKIPPED;

	/* initialize input stream */
	if (HPDF_Stream_Size(src) == 0)
		return HPDF_OK;

#ifndef LIBHPDF_HAVE_NOZLIB
	if (filter & HPDF_STREAM_FILTER_FLATE_DECODE)
		return HPDF_Stream_WriteToStreamWithDeflate(src, dst, e);
#endif

	ret = HPDF_Stream_Seek(src, 0, HPDF_SEEK_SET);
	if (ret != HPDF_OK)
		return ret;

	flg = HPDF_FALSE;
	for (;;)
	{
		HPDF_UINT size = HPDF_STREAM_BUF_SIZ;

		ret = HPDF_Stream_Read(src, buf, &size);

		if (ret != HPDF_OK)
		{
			if (ret == HPDF_STREAM_EOF)
			{
				flg = HPDF_TRUE;
				if (size == 0)
					break;
			} else
			{
				return ret;
			}
		}

		if (e)
		{
			HPDF_Encrypt_CryptBuf(e, buf, ebuf, size);
			ret = HPDF_Stream_Write(dst, ebuf, size);
		} else
		{
			ret = HPDF_Stream_Write(dst, buf, size);
		}

		if (ret != HPDF_OK)
			return ret;

		if (flg)
			break;
	}

	return HPDF_OK;
}


/*
 *  HPDF_FileReader_ReadFunc
 *
 *  Reading data function for HPDF_FileReader.
 *
 */
static HPDF_STATUS HPDF_FileReader_ReadFunc(HPDF_Stream stream, void *ptr, HPDF_UINT *siz)
{
	HPDF_FILEP fp = (HPDF_FILEP) stream->attr;
	size_t rsiz;

	memset(ptr, 0, *siz);
	rsiz = HPDF_FREAD(ptr, 1, *siz, fp);

	if (rsiz != *siz)
	{
		if (HPDF_FEOF(fp))
		{
			*siz = (HPDF_UINT)rsiz;

			return HPDF_STREAM_EOF;
		}

		return HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR, errno);
	}

	return HPDF_OK;
}


/*
 *  HPDF_FileReader_SeekFunc
 *
 *  Seeking data function for HPDF_FileReader.
 *
 *  stream : Pointer to a HPDF_Stream object.
 *  pos : New position of stream object.
 *  HPDF_whence_mode : Seeking mode describing below.
 *                     HPDF_SEEK_SET : Absolute file position
 *                     HPDF_SEEK_CUR : Relative to the current file position
 *                     HPDF_SEEK_END : Relative to the current end of file.
 *
 *  HPDF_FileReader_seek_fn returns HPDF_OK when successful. On failer
 *  the result is HPDF_FILE_IO_ERROR and HPDF_Error_GetCode2() returns the
 *  error which returned by file seeking function of platform.
 *
 */
static HPDF_STATUS HPDF_FileReader_SeekFunc(HPDF_Stream stream, HPDF_INT pos, HPDF_WhenceMode mode)
{
	HPDF_FILEP fp = (HPDF_FILEP) stream->attr;
	int whence;

	switch (mode)
	{
	case HPDF_SEEK_CUR:
		whence = SEEK_CUR;
		break;
	case HPDF_SEEK_END:
		whence = SEEK_END;
		break;
	default:
		whence = SEEK_SET;
		break;
	}

	if (HPDF_FSEEK(fp, pos, whence) != 0)
	{
		return HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR, errno);
	}

	return HPDF_OK;
}


static HPDF_INT32 HPDF_FileStream_TellFunc(HPDF_Stream stream)
{
	long ret;
	HPDF_FILEP fp = (HPDF_FILEP) stream->attr;

	if ((ret = HPDF_FTELL(fp)) < 0)
	{
		HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR, errno);
		return -1;
	}

	return (HPDF_INT32)ret;
}


static HPDF_UINT32 HPDF_FileStream_SizeFunc(HPDF_Stream stream)
{
	long size;
	long ptr;
	HPDF_FILEP fp = (HPDF_FILEP) stream->attr;

	/* save current file-pointer */
	if ((ptr = HPDF_FTELL(fp)) < 0)
	{
		HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR, errno);
		return 0;
	}

	/* move file-pointer to the end of the file */
	if (HPDF_FSEEK(fp, 0, SEEK_END) < 0)
	{
		HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR, errno);
		return 0;
	}

	/* get the pointer of the end of the file */
	if ((size = HPDF_FTELL(fp)) < 0)
	{
		HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR, errno);
		return 0;
	}

	/* restore current file-pointer */
	if (HPDF_FSEEK(fp, ptr, SEEK_SET) < 0)
	{
		HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR, errno);
		return 0;
	}

	return (HPDF_UINT32)size;
}


static void HPDF_FileStream_FreeFunc(HPDF_Stream stream)
{
	HPDF_FILEP fp;

	fp = (HPDF_FILEP) stream->attr;

	if (fp)
		HPDF_FCLOSE(fp);

	stream->attr = NULL;
}



HPDF_Stream HPDF_FileReader_New(HPDF_MMgr mmgr, const char *fname)
{
	HPDF_Stream stream;
	HPDF_FILEP fp = HPDF_FOPEN(fname, "rb");

	if (!fp)
	{
		HPDF_SetError(mmgr->error, HPDF_FILE_OPEN_ERROR, errno);
		return NULL;
	}

	stream = (HPDF_Stream) HPDF_GetMem(mmgr, sizeof(*stream));

	if (stream)
	{
		memset(stream, 0, sizeof(*stream));
		stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
		stream->type = HPDF_STREAM_FILE;
		stream->error = mmgr->error;
		stream->mmgr = mmgr;
		stream->read_fn = HPDF_FileReader_ReadFunc;
		stream->seek_fn = HPDF_FileReader_SeekFunc;
		stream->tell_fn = HPDF_FileStream_TellFunc;
		stream->size_fn = HPDF_FileStream_SizeFunc;
		stream->free_fn = HPDF_FileStream_FreeFunc;
		stream->attr = fp;
	}

	return stream;
}


static HPDF_STATUS HPDF_FileWriter_WriteFunc(HPDF_Stream stream, const void *ptr, HPDF_UINT siz)
{
	HPDF_FILEP fp;
	size_t ret;

	fp = (HPDF_FILEP) stream->attr;
	ret = HPDF_FWRITE(ptr, 1, siz, fp);

	if (ret != siz)
	{
		return HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR, errno);
	}

	return HPDF_OK;
}


HPDF_Stream HPDF_FileWriter_New(HPDF_MMgr mmgr, const char *fname)
{
	HPDF_Stream stream;
	HPDF_FILEP fp = HPDF_FOPEN(fname, "wb");

	if (!fp)
	{
		HPDF_SetError(mmgr->error, HPDF_FILE_OPEN_ERROR, errno);
		return NULL;
	}

	stream = (HPDF_Stream) HPDF_GetMem(mmgr, sizeof(*stream));

	if (stream)
	{
		memset(stream, 0, sizeof(*stream));
		stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
		stream->error = mmgr->error;
		stream->mmgr = mmgr;
		stream->write_fn = HPDF_FileWriter_WriteFunc;
		stream->free_fn = HPDF_FileStream_FreeFunc;
		stream->tell_fn = HPDF_FileStream_TellFunc;
		stream->attr = fp;
		stream->type = HPDF_STREAM_FILE;
	}

	return stream;
}


HPDF_Stream HPDF_FileWriter_Newfp(HPDF_MMgr mmgr, FILE *fp)
{
	HPDF_Stream stream;

	if (!fp)
	{
		HPDF_SetError(mmgr->error, HPDF_INVALID_PARAMETER, 0);
		return NULL;
	}

	stream = (HPDF_Stream) HPDF_GetMem(mmgr, sizeof(*stream));

	if (stream)
	{
		memset(stream, 0, sizeof(*stream));
		stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
		stream->error = mmgr->error;
		stream->mmgr = mmgr;
		stream->write_fn = HPDF_FileWriter_WriteFunc;
		stream->free_fn = HPDF_FileStream_FreeFunc;
		stream->tell_fn = HPDF_FileStream_TellFunc;
		stream->attr = fp;
		stream->type = HPDF_STREAM_FILE;
	}

	return stream;
}


static HPDF_STATUS HPDF_MemStream_InWrite(HPDF_Stream stream, const void **ptr, HPDF_UINT *count)
{
	HPDF_MemStreamAttr attr = (HPDF_MemStreamAttr) stream->attr;
	HPDF_UINT rsize = attr->buf_siz - attr->w_pos;

	if (*count <= 0)
		return HPDF_OK;

	if (rsize >= *count)
	{
		memcpy(attr->w_ptr, *ptr, *count);
		attr->w_ptr += *count;
		attr->w_pos += *count;
		*count = 0;
	} else
	{
		if (rsize > 0)
		{
			memcpy(attr->w_ptr, *ptr, rsize);
			*ptr = (const char *)*ptr + rsize;
			*count -= rsize;
		}
		attr->w_ptr = (HPDF_BYTE *) HPDF_GetMem(stream->mmgr, attr->buf_siz);

		if (attr->w_ptr == NULL)
			return HPDF_Error_GetCode(stream->error);

		if (HPDF_List_Add(attr->buf, attr->w_ptr) != HPDF_OK)
		{
			HPDF_FreeMem(stream->mmgr, attr->w_ptr);
			attr->w_ptr = NULL;

			return HPDF_Error_GetCode(stream->error);
		}
		attr->w_pos = 0;
	}
	return HPDF_OK;
}


static HPDF_STATUS HPDF_MemStream_WriteFunc(HPDF_Stream stream, const void *ptr, HPDF_UINT siz)
{
	HPDF_UINT wsiz = siz;

	if (HPDF_Error_GetCode(stream->error) != 0)
		return HPDF_THIS_FUNC_WAS_SKIPPED;

	while (wsiz > 0)
	{
		HPDF_STATUS ret = HPDF_MemStream_InWrite(stream, &ptr, &wsiz);

		if (ret != HPDF_OK)
			return ret;
	}

	return HPDF_OK;
}


static HPDF_INT32 HPDF_MemStream_TellFunc(HPDF_Stream stream)
{
	HPDF_INT32 ret;
	HPDF_MemStreamAttr attr = (HPDF_MemStreamAttr) stream->attr;

	ret = attr->r_ptr_idx * attr->buf_siz;
	ret += attr->r_pos;

	return ret;
}


static HPDF_UINT32 HPDF_MemStream_SizeFunc(HPDF_Stream stream)
{
	return stream->size;
}


static HPDF_STATUS HPDF_MemStream_SeekFunc(HPDF_Stream stream, HPDF_INT pos, HPDF_WhenceMode mode)
{
	HPDF_MemStreamAttr attr = (HPDF_MemStreamAttr) stream->attr;

	if (mode == HPDF_SEEK_CUR)
	{
		pos += (attr->r_ptr_idx * attr->buf_siz);
		pos += attr->r_pos;
	} else if (mode == HPDF_SEEK_END)
	{
		pos = stream->size - pos;
	}

	if ((HPDF_UINT) pos > stream->size)
	{
		return HPDF_SetError(stream->error, HPDF_STREAM_EOF, 0);
	}

	if (stream->size == 0)
	{
		return HPDF_OK;
	}

	attr->r_ptr_idx = pos / attr->buf_siz;
	attr->r_pos = pos % attr->buf_siz;
	attr->r_ptr = (HPDF_BYTE *) HPDF_List_ItemAt(attr->buf, attr->r_ptr_idx);
	if (attr->r_ptr == NULL)
	{
		HPDF_SetError(stream->error, HPDF_INVALID_OBJECT, 0);
		return HPDF_INVALID_OBJECT;
	} else
	{
		attr->r_ptr += attr->r_pos;
	}

	return HPDF_OK;
}


HPDF_BYTE *HPDF_MemStream_GetBufPtr(HPDF_Stream stream, HPDF_UINT index, HPDF_UINT *length)
{
	HPDF_BYTE *ret;
	HPDF_MemStreamAttr attr;

	if (stream->type != HPDF_STREAM_MEMORY)
	{
		HPDF_SetError(stream->error, HPDF_INVALID_OBJECT, 0);
		return NULL;
	}

	attr = (HPDF_MemStreamAttr) stream->attr;

	ret = (HPDF_BYTE *) HPDF_List_ItemAt(attr->buf, index);
	if (ret == NULL)
	{
		HPDF_SetError(stream->error, HPDF_INVALID_PARAMETER, 0);
		*length = 0;
		return NULL;
	}

	*length = (attr->buf->count - 1 == index) ? attr->w_pos : attr->buf_siz;
	return ret;
}


void HPDF_MemStream_FreeData(HPDF_Stream stream)
{
	HPDF_MemStreamAttr attr;
	HPDF_UINT i;

	if (!stream || stream->type != HPDF_STREAM_MEMORY)
		return;

	attr = (HPDF_MemStreamAttr) stream->attr;

	for (i = 0; i < attr->buf->count; i++)
		HPDF_FreeMem(stream->mmgr, HPDF_List_ItemAt(attr->buf, i));

	HPDF_List_Clear(attr->buf);

	stream->size = 0;
	attr->w_pos = attr->buf_siz;
	attr->w_ptr = NULL;
	attr->r_ptr_idx = 0;
	attr->r_pos = 0;
}


static void HPDF_MemStream_FreeFunc(HPDF_Stream stream)
{
	HPDF_MemStreamAttr attr;

	attr = (HPDF_MemStreamAttr) stream->attr;
	HPDF_MemStream_FreeData(stream);
	HPDF_List_Free(attr->buf);
	HPDF_FreeMem(stream->mmgr, attr);
	stream->attr = NULL;
}


HPDF_UINT HPDF_MemStream_GetBufSize(HPDF_Stream stream)
{
	HPDF_MemStreamAttr attr;

	if (!stream || stream->type != HPDF_STREAM_MEMORY)
		return 0;

	attr = (HPDF_MemStreamAttr) stream->attr;
	return attr->buf_siz;
}


HPDF_UINT HPDF_MemStream_GetBufCount(HPDF_Stream stream)
{
	HPDF_MemStreamAttr attr;

	if (!stream || stream->type != HPDF_STREAM_MEMORY)
		return 0;

	attr = (HPDF_MemStreamAttr) stream->attr;
	return attr->buf->count;
}


static HPDF_STATUS HPDF_MemStream_ReadFunc(HPDF_Stream stream, void *buf, HPDF_UINT *size)
{
	HPDF_MemStreamAttr attr = (HPDF_MemStreamAttr) stream->attr;
	HPDF_UINT buf_size;
	HPDF_UINT rlen = *size;

	*size = 0;

	while (rlen > 0)
	{
		HPDF_UINT tmp_len;

		if (attr->buf->count == 0)
			return HPDF_STREAM_EOF;

		if (attr->buf->count - 1 > attr->r_ptr_idx)
			tmp_len = attr->buf_siz - attr->r_pos;
		else if (attr->buf->count - 1 == attr->r_ptr_idx)
			tmp_len = attr->w_pos - attr->r_pos;
		else
			return HPDF_STREAM_EOF;

		if (!attr->r_ptr)
			attr->r_ptr = (HPDF_BYTE *) HPDF_List_ItemAt(attr->buf, attr->r_ptr_idx);

		if (tmp_len >= rlen)
		{
			memcpy(buf, attr->r_ptr, rlen);
			attr->r_pos += rlen;
			*size += rlen;
			attr->r_ptr += rlen;
			return HPDF_OK;
		} else
		{
			memcpy(buf, attr->r_ptr, tmp_len);
			buf = (char *)buf + tmp_len;
			rlen -= tmp_len;
			*size += tmp_len;

			if (attr->r_ptr_idx == attr->buf->count - 1)
			{
				attr->r_ptr += tmp_len;
				attr->r_pos += tmp_len;
				return HPDF_STREAM_EOF;
			}

			attr->r_ptr_idx++;
			attr->r_pos = 0;
			attr->r_ptr = HPDF_MemStream_GetBufPtr(stream, attr->r_ptr_idx, &buf_size);
		}
	}

	return HPDF_OK;
}


HPDF_STATUS HPDF_MemStream_Rewrite(HPDF_Stream stream, HPDF_BYTE *buf, HPDF_UINT size)
{
	HPDF_MemStreamAttr attr = (HPDF_MemStreamAttr) stream->attr;
	HPDF_UINT buf_size;
	HPDF_UINT rlen = size;

	while (rlen > 0)
	{
		HPDF_UINT tmp_len;

		if (attr->buf->count <= attr->r_ptr_idx)
		{
			HPDF_STATUS ret = HPDF_MemStream_WriteFunc(stream, buf, rlen);

			attr->r_ptr_idx = attr->buf->count;
			attr->r_pos = attr->w_pos;
			attr->r_ptr = attr->w_ptr;
			return ret;
		} else if (attr->buf->count == attr->r_ptr_idx)
		{
			tmp_len = attr->w_pos - attr->r_pos;
		} else
		{
			tmp_len = attr->buf_siz - attr->r_pos;
		}

		if (tmp_len >= rlen)
		{
			memcpy(attr->r_ptr, buf, rlen);
			attr->r_pos += rlen;
			attr->r_ptr += rlen;
			return HPDF_OK;
		} else
		{
			memcpy(attr->r_ptr, buf, tmp_len);
			buf += tmp_len;
			rlen -= tmp_len;
			attr->r_ptr_idx++;

			if (attr->buf->count > attr->r_ptr_idx)
			{
				attr->r_pos = 0;
				attr->r_ptr = HPDF_MemStream_GetBufPtr(stream, attr->r_ptr_idx, &buf_size);
			}
		}
	}
	return HPDF_OK;
}


HPDF_Stream HPDF_MemStream_New(HPDF_MMgr mmgr, HPDF_UINT buf_siz)
{
	HPDF_Stream stream;

	/* Create new HPDF_Stream object. */
	stream = (HPDF_Stream) HPDF_GetMem(mmgr, sizeof(*stream));

	if (stream)
	{
		/* Create attribute struct. */
		HPDF_MemStreamAttr attr = (HPDF_MemStreamAttr) HPDF_GetMem(mmgr, sizeof(*attr));

		if (!attr)
		{
			HPDF_FreeMem(mmgr, stream);
			return NULL;
		}

		memset(stream, 0, sizeof(*stream));
		memset(attr, 0, sizeof(*attr));

		attr->buf = HPDF_List_New(mmgr, HPDF_DEF_ITEMS_PER_BLOCK);
		if (!attr->buf)
		{
			HPDF_FreeMem(mmgr, stream);
			HPDF_FreeMem(mmgr, attr);
			return NULL;
		}

		stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
		stream->type = HPDF_STREAM_MEMORY;
		stream->error = mmgr->error;
		stream->mmgr = mmgr;
		stream->attr = attr;
		attr->buf_siz = (buf_siz > 0) ? buf_siz : HPDF_STREAM_BUF_SIZ;
		attr->w_pos = attr->buf_siz;

		stream->write_fn = HPDF_MemStream_WriteFunc;
		stream->read_fn = HPDF_MemStream_ReadFunc;
		stream->seek_fn = HPDF_MemStream_SeekFunc;
		stream->tell_fn = HPDF_MemStream_TellFunc;
		stream->size_fn = HPDF_MemStream_SizeFunc;
		stream->free_fn = HPDF_MemStream_FreeFunc;
	}

	return stream;
}


/*
 *  HPDF_CallbackReader_new
 *
 *  Constractor for HPDF_CallbackReader.
 *
 *  mmgr : Pointer to a HPDF_MMgr object.
 *  read_fn : Pointer to a user function for reading data.
 *  seek_fn : Pointer to a user function for seeking data.
 *  data : Pointer to a data which defined by user.
 *
 *  return: If success, It returns pointer to new HPDF_Stream object,
 *          otherwise, it returns NULL.
 *
 */
HPDF_Stream HPDF_CallbackReader_New(
	HPDF_MMgr mmgr,
	HPDF_Stream_Read_Func read_fn,
	HPDF_Stream_Seek_Func seek_fn,
	HPDF_Stream_Tell_Func tell_fn,
	HPDF_Stream_Size_Func size_fn,
	void *data)
{
	HPDF_Stream stream;

	stream = (HPDF_Stream) HPDF_GetMem(mmgr, sizeof(*stream));

	if (stream)
	{
		stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
		stream->type = HPDF_STREAM_CALLBACK;
		stream->mmgr = mmgr;
		stream->error = mmgr->error;
		stream->size = 0;
		stream->read_fn = read_fn;
		stream->seek_fn = seek_fn;
		stream->free_fn = 0;
		stream->tell_fn = tell_fn;
		stream->size_fn = size_fn;
		stream->attr = data;
	}

	return stream;
}


/*
 *  HPDF_CallbackWriter_new
 *
 *  Constractor for HPDF_CallbackWriter.
 *
 *  mmgr : Pointer to a HPDF_MMgr object.
 *  read_fn : Pointer to a user function for writing data.
 *  data : Pointer to a data which defined by user.
 *
 *  return: If success, It returns pointer to new HPDF_Stream object,
 *          otherwise, it returns NULL.
 *
 */
HPDF_Stream HPDF_CallbackWriter_New(HPDF_MMgr mmgr, HPDF_Stream_Write_Func write_fn, void *data)
{
	HPDF_Stream stream;

	stream = (HPDF_Stream) HPDF_GetMem(mmgr, sizeof(*stream));

	if (stream)
	{
		memset(stream, 0, sizeof(*stream));
		stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
		stream->type = HPDF_STREAM_CALLBACK;
		stream->mmgr = mmgr;
		stream->error = mmgr->error;
		stream->size = 0;
		stream->write_fn = write_fn;
		stream->attr = data;
	}

	return stream;
}


HPDF_STATUS HPDF_Stream_Validate(HPDF_Stream stream)
{
	if (!stream || stream->sig_bytes != HPDF_STREAM_SIG_BYTES)
		return HPDF_FALSE;
	else
		return HPDF_TRUE;
}
