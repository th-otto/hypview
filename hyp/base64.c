#define __BASE64_IMPLEMENTATION__
#include "hypdefs.h"
#include "base64.h"

typedef struct _TempBucket
{
	unsigned char nData[4];
	size_t nSize;
} TempBucket;
#define Clear(r) do { memset(r.nData, 0, sizeof(r.nData)); r.nSize = 0; } while (0)


struct _base64 {
	unsigned char *m_pDBuffer;
	unsigned char *m_pEBuffer;
	size_t m_nDBufLen;
	size_t m_nEBufLen;
	size_t m_nDDataLen;
	size_t m_nEDataLen;
	
	unsigned char m_DecodeTable[256];
};

static unsigned char const Base64Digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

Base64 *Base64_New(void)
{
	Base64 *b;
	int	i;
	
	b = g_new(Base64, 1);
	if (b != NULL)
	{
		b->m_pDBuffer = NULL;
		b->m_pEBuffer = NULL;
		b->m_nDBufLen = 0;
		b->m_nEBufLen = 0;
		b->m_nDDataLen = 0;
		b->m_nEDataLen = 0;

		/* Initialize Decoding table. */
		for (i = 0; i < 256; i++)
			b->m_DecodeTable[i] = 254;
	
		for (i = 0; i < 64; i++)
		{
			b->m_DecodeTable[Base64Digits[i]]	= (char) i;
			b->m_DecodeTable[Base64Digits[i]|0x80] = (char) i;
		}
	
		b->m_DecodeTable['='] = 255;
		b->m_DecodeTable['='|0x80] = 255;
	}
	return b;
}

/*** ---------------------------------------------------------------------- ***/

void Base64_Delete(Base64 *b)
{
	if (b != NULL)
	{
		if (b->m_pDBuffer != NULL)
			g_free(b->m_pDBuffer);
		if (b->m_pEBuffer != NULL)
			g_free(b->m_pEBuffer);
		g_free(b);
	}
}

/*** ---------------------------------------------------------------------- ***/

void *Base64_DecodedMessage(Base64 *b)
{
	if (b != NULL)
		return (void *)b->m_pDBuffer;
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

char *Base64_EncodedMessage(Base64 *b)
{ 
	if (b != NULL)
		return (char *)b->m_pEBuffer;
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

size_t Base64_DecodedMessageSize(Base64 *b)
{ 
	if (b != NULL)
		return b->m_nDDataLen;
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

size_t Base64_EncodedMessageSize(Base64 *b)
{ 
	if (b != NULL)
		return b->m_nEDataLen;
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL Base64_AllocEncode(Base64 *b, size_t size)
{
	if (b != NULL && size > 0)
	{
		if (b->m_nEBufLen < size)
		{
			if (b->m_pEBuffer != NULL)
				g_free(b->m_pEBuffer);
	
			b->m_nEBufLen = size;
			b->m_pEBuffer = g_new(unsigned char, size);
			if (b->m_pEBuffer == NULL)
				return FALSE;
		}
	
		memset(b->m_pEBuffer, 0, b->m_nEBufLen);
		b->m_nEDataLen = 0;
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL Base64_AllocDecode(Base64 *b, size_t size)
{
	if (b != NULL && size > 0)
	{
		if (b->m_nDBufLen < size)
		{
			if (b->m_pDBuffer != NULL)
				g_free(b->m_pDBuffer);
	
			b->m_nDBufLen = size;
			b->m_pDBuffer = g_new(unsigned char, size);
			if (b->m_pDBuffer == NULL)
				return FALSE;
		}
	
		memset(b->m_pDBuffer, 0, b->m_nDBufLen);
		b->m_nDDataLen = 0;
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL _IsBadMimeChar(unsigned char nData)
{
	switch (nData)
	{
	case '\r':
	case '\n':
	case '\t':
	case ' ' :
	case '\b':
	case '\a':
	case '\f':
	case '\v':
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL Base64_SetEncodeBuffer(Base64 *b, const unsigned char *pBuffer, size_t nBufLen)
{
	size_t i;

	if (pBuffer != NULL && Base64_AllocEncode(b, nBufLen))
	{
		i = 0;
		while (i < nBufLen)
		{
			if (!_IsBadMimeChar(pBuffer[i]))
			{
				b->m_pEBuffer[b->m_nEDataLen] = pBuffer[i];
				b->m_nEDataLen++;
			}
			i++;
		}
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL Base64_SetDecodeBuffer(Base64 *b, const unsigned char *pBuffer, size_t nBufLen)
{
	if (pBuffer != NULL && Base64_AllocDecode(b, nBufLen))
	{
		memcpy(b->m_pDBuffer, pBuffer, nBufLen);
		b->m_nDDataLen = nBufLen;
		return TRUE;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static void EncodeRaw(TempBucket *Data, const TempBucket *Decode)
{
	unsigned char nTemp;

	Data->nData[0] = Decode->nData[0];
	Data->nData[0] >>= 2;
	
	Data->nData[1] = Decode->nData[0];
	Data->nData[1] <<= 4;
	nTemp = Decode->nData[1];
	nTemp >>= 4;
	Data->nData[1] |= nTemp;
	Data->nData[1] &= 0x3F;

	Data->nData[2] = Decode->nData[1];
	Data->nData[2] <<= 2;

	nTemp = Decode->nData[2];
	nTemp >>= 6;

	Data->nData[2] |= nTemp;
	Data->nData[2] &= 0x3F;

	Data->nData[3] = Decode->nData[2];
	Data->nData[3] &= 0x3F;
}

/*** ---------------------------------------------------------------------- ***/

static void EncodeToBuffer(const TempBucket *Decode, unsigned char *pBuffer)
{
	TempBucket Data;
	int i;
	
	EncodeRaw(&Data, Decode);

	for (i = 0; i < 4; i++)
		pBuffer[i] = Base64Digits[Data.nData[i]];

	if (Decode->nSize == 1)
	{
		pBuffer[2] = '=';
		pBuffer[3] = '=';
	} else if (Decode->nSize == 2)
	{
		pBuffer[3] = '=';
	}
}

/*** ---------------------------------------------------------------------- ***/

_BOOL Base64_Encode(Base64 *b, const void *pBuffer, size_t nBufLen)
{
	TempBucket Raw;
	size_t nIndex;
	
	if (Base64_SetDecodeBuffer(b, (const unsigned char *)pBuffer, nBufLen))
	{
		if (Base64_AllocEncode(b, nBufLen * 2))
		{
			nIndex	= 0;
			while ((nIndex + 3) <= nBufLen)
			{
				Clear(Raw);
				memcpy(Raw.nData, b->m_pDBuffer + nIndex, 3);
				Raw.nSize = 3;
				EncodeToBuffer(&Raw, b->m_pEBuffer + b->m_nEDataLen);
				nIndex += 3;
				b->m_nEDataLen += 4;
			}
		
			if (nBufLen > nIndex)
			{
				Clear(Raw);
				Raw.nSize = nBufLen - nIndex;
				memcpy(Raw.nData, b->m_pDBuffer + nIndex, nBufLen - nIndex);
				EncodeToBuffer(&Raw, b->m_pEBuffer + b->m_nEDataLen);
				b->m_nEDataLen += 4;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static void DecodeRaw(TempBucket *Data, const TempBucket *Decode)
{
	unsigned char nTemp;

	Data->nData[0] = Decode->nData[0];
	Data->nData[0] <<= 2;

	nTemp = Decode->nData[1];
	nTemp >>= 4;
	nTemp &= 0x03;
	Data->nData[0] |= nTemp;

	Data->nData[1] = Decode->nData[1];
	Data->nData[1] <<= 4;

	nTemp = Decode->nData[2];
	nTemp >>= 2;
	nTemp &= 0x0F;
	Data->nData[1] |= nTemp;

	Data->nData[2] = Decode->nData[2];
	Data->nData[2] <<= 6;
	nTemp = Decode->nData[3];
	nTemp &= 0x3F;
	Data->nData[2] |= nTemp;
}

/*** ---------------------------------------------------------------------- ***/

static size_t DecodeToBuffer(const TempBucket *Decode, unsigned char *pBuffer)
{
	TempBucket Data;
	size_t nCount;
	int i;
	
	DecodeRaw(&Data, Decode);

	nCount = 0;
	for (i = 0; i < 3; i++)
	{
		pBuffer[i] = Data.nData[i];
		if (pBuffer[i] != 255)
			nCount++;
	}

	return nCount;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL Base64_Decode(Base64 *b, const char *pBuffer, size_t dwBufLen)
{
	TempBucket Raw;
	size_t nIndex;

	if (Base64_SetEncodeBuffer(b, (const unsigned char *)pBuffer, dwBufLen))
	{
		if (Base64_AllocDecode(b, dwBufLen))
		{
			nIndex = 0;
			while ((nIndex + 4) <= b->m_nEDataLen)
			{
				Clear(Raw);
				Raw.nData[0] = b->m_DecodeTable[b->m_pEBuffer[nIndex]];
				Raw.nData[1] = b->m_DecodeTable[b->m_pEBuffer[nIndex + 1]];
				Raw.nData[2] = b->m_DecodeTable[b->m_pEBuffer[nIndex + 2]];
				Raw.nData[3] = b->m_DecodeTable[b->m_pEBuffer[nIndex + 3]];
		
				if (Raw.nData[2] == 255)
					Raw.nData[2] = 0;
				if (Raw.nData[3] == 255)
					Raw.nData[3] = 0;
				
				Raw.nSize = 4;
				DecodeToBuffer(&Raw, b->m_pDBuffer + b->m_nDDataLen);
				nIndex += 4;
				b->m_nDDataLen += 3;
			}
			
			/*
			 * If nIndex < m_nEDataLen, then we got a decode message without padding.
			 * We may want to throw some kind of warning here, but we are still required
			 * to handle the decoding as if it was properly padded.
			 */
			if (nIndex < b->m_nEDataLen)
			{
				size_t i;
				
				Clear(Raw);
				for (i = nIndex; i < b->m_nEDataLen; i++)
				{
					Raw.nData[i - nIndex] = b->m_DecodeTable[b->m_pEBuffer[i]];
					Raw.nSize++;
					if (Raw.nData[i - nIndex] == 255)
						Raw.nData[i - nIndex] = 0;
				}
		
				DecodeToBuffer(&Raw, b->m_pDBuffer + b->m_nDDataLen);
				b->m_nDDataLen += (b->m_nEDataLen - nIndex);
			}
			return TRUE;
		}
	}
	return FALSE;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#ifdef MAIN

int main(int argc, char **argv)
{
	Base64 *base64;
	char *argp;
	_BOOL encode;
	
	encode = TRUE;
	while (--argc)
	{
		argp = *++argv;
		if (*argp == '-')
		{
			while (*++argp)
			{
				switch (*argp)
				{
				case 'd':
					encode = FALSE;
					break;
				case 'e':
					encode = TRUE;
					break;
				default:
					return 1;
				}
			}
		} else
		{
			base64 = Base64_New();
			if (encode)
			{
				Base64_Encode(base64, argp, strlen(argp));
				printf("%s: ", argp);
				fwrite(Base64_EncodedMessage(base64), 1, Base64_EncodedMessageSize(base64), stdout);
				printf("\n");
			} else
			{
				Base64_Decode(base64, argp, strlen(argp));
				printf("%s: ", argp);
				fwrite(Base64_DecodedMessage(base64), 1, Base64_DecodedMessageSize(base64), stdout);
				printf("\n");
			}
			Base64_Delete(base64);
		}
	}
	return 0;
}

#endif /* MAIN */

