/*********************************************************************
* LH5 decoder
*********************************************************************/

#include "hypdefs.h"
#include "hypdebug.h"
#include <limits.h>
#include "lh5int.h"


/* Parameterblock */
typedef struct {
	const unsigned char *packedMem;
	unsigned long packedLen;
	unsigned char *unpackedMem;
	unsigned long unpackedLen;

	unsigned short bitbuf;
	unsigned short subbitbuf;
	short bitcount;
	short blocksize;

	short dec_j;						/* remaining bytes to copy */
	unsigned int dec_i;

	unsigned short left[2 * NC - 1];
	unsigned short right[2 * NC - 1];

	unsigned char c_len[NC];
	unsigned char pt_len[NPT];

	unsigned short c_table[4096];
	unsigned short pt_table[256];

	unsigned char buffer[DICSIZ];
} lh5_decoder;

#define BITBUFSIZ	(CHAR_BIT * (short)sizeof(lh5->bitbuf))



static void fillbuf(lh5_decoder *lh5, short n)			/* Shift bitbuf n bits left, read n bits */
{
	lh5->bitbuf <<= n;
	while (n > lh5->bitcount)
	{
		lh5->bitbuf |= lh5->subbitbuf << (n -= lh5->bitcount);
		if (lh5->packedLen != 0)
		{
			lh5->packedLen--;
			lh5->subbitbuf = *lh5->packedMem++;
		} else
		{
			lh5->subbitbuf = 0;
		}
		lh5->bitcount = CHAR_BIT;
	}
	lh5->bitbuf |= lh5->subbitbuf >> (lh5->bitcount -= n);
}


static unsigned short getbits(lh5_decoder *lh5, short n)
{
	unsigned short x;

	if (n == 0)
		x = 0;
	else
		x = lh5->bitbuf >> (BITBUFSIZ - n);
	fillbuf(lh5, n);
	return x;
}


static void make_table(lh5_decoder *lh5, short nchar, unsigned char bitlen[], short tablebits, unsigned short table[])
{
	unsigned short count[17], weight[17], start[18], *p;
	unsigned short i, k, len, ch, jutbits, avail, nextcode, mask;

	for (i = 1; i <= 16; i++)
		count[i] = 0;
	for (i = 0; i < nchar; i++)
		count[bitlen[i]]++;

	start[1] = 0;
	for (i = 1; i <= 16; i++)
		start[i + 1] = start[i] + (count[i] << (16 - i));
#if 0
	if (start[17] != (unsigned short) (1U << 16))
		error("Bad table");
#endif

	jutbits = 16 - tablebits;
	for (i = 1; i <= tablebits; i++)
	{
		start[i] >>= jutbits;
		weight[i] = 1U << (tablebits - i);
	}
	while (i <= 16)
	{
		weight[i] = 1U << (16 - i);
		i++;
	}

	i = start[tablebits + 1] >> jutbits;
	if (i != (unsigned short) (1UL << 16))
	{
		k = 1U << tablebits;
		while (i != k)
			table[i++] = 0;
	}

	avail = nchar;
	mask = 1U << (15 - tablebits);
	for (ch = 0; ch < nchar; ch++)
	{
		if ((len = bitlen[ch]) == 0)
			continue;
		nextcode = start[len] + weight[len];
		if (len <= tablebits)
		{
			for (i = start[len]; i < nextcode; i++)
				table[i] = ch;
		} else
		{
			k = start[len];
			p = &table[k >> jutbits];
			i = len - tablebits;
			while (i != 0)
			{
				if (*p == 0)
				{
					lh5->right[avail] = lh5->left[avail] = 0;
					*p = avail++;
				}
				if (k & mask)
					p = &lh5->right[*p];
				else
					p = &lh5->left[*p];
				k <<= 1;
				i--;
			}
			*p = ch;
		}
		start[len] = nextcode;
	}
}


static void read_pt_len(lh5_decoder *lh5, short nn, short nbit, short i_special)
{
	short i, c, n;
	unsigned short mask;

	n = getbits(lh5, nbit);
	if (n == 0)
	{
		c = getbits(lh5, nbit);
		for (i = 0; i < nn; i++)
			lh5->pt_len[i] = 0;
		for (i = 0; i < 256; i++)
			lh5->pt_table[i] = c;
	} else
	{
		i = 0;
		while (i < n)
		{
			c = lh5->bitbuf >> (BITBUFSIZ - 3);
			if (c == 7)
			{
				mask = 1U << (BITBUFSIZ - 1 - 3);
				while (mask & lh5->bitbuf)
				{
					mask >>= 1;
					c++;
				}
			}
			fillbuf(lh5, (c < 7) ? 3 : c - 3);
			lh5->pt_len[i++] = c;
			if (i == i_special)
			{
				c = getbits(lh5, 2);
				while (--c >= 0)
					lh5->pt_len[i++] = 0;
			}
		}
		while (i < nn)
			lh5->pt_len[i++] = 0;
		make_table(lh5, nn, lh5->pt_len, 8, lh5->pt_table);
	}
}


static void read_c_len(lh5_decoder *lh5)
{
	short i, c, n;
	unsigned short mask;

	n = getbits(lh5, CBIT);
	if (n == 0)
	{
		c = getbits(lh5, CBIT);
		for (i = 0; i < NC; i++)
			lh5->c_len[i] = 0;
		for (i = 0; i < 4096; i++)
			lh5->c_table[i] = c;
	} else
	{
		i = 0;
		while (i < n)
		{
			c = lh5->pt_table[lh5->bitbuf >> (BITBUFSIZ - 8)];
			if (c >= NT)
			{
				mask = 1U << (BITBUFSIZ - 1 - 8);
				do
				{
					if (lh5->bitbuf & mask)
						c = lh5->right[c];
					else
						c = lh5->left[c];
					mask >>= 1;
				} while (c >= NT);
			}
			fillbuf(lh5, lh5->pt_len[c]);
			if (c <= 2)
			{
				if (c == 0)
					c = 1;
				else if (c == 1)
					c = getbits(lh5, 4) + 3;
				else
					c = getbits(lh5, CBIT) + 20;
				while (--c >= 0)
					lh5->c_len[i++] = 0;
			} else
			{
				lh5->c_len[i++] = c - 2;
			}
		}
		while (i < NC)
			lh5->c_len[i++] = 0;
		make_table(lh5, NC, lh5->c_len, 12, lh5->c_table);
	}
}


static unsigned short decode_c(lh5_decoder *lh5)
{
	unsigned short j, mask;

	if (lh5->blocksize == 0)
	{
		lh5->blocksize = getbits(lh5, 16);
		read_pt_len(lh5, NT, TBIT, 3);
		read_c_len(lh5);
		read_pt_len(lh5, NP, PBIT, -1);
	}
	lh5->blocksize--;
	j = lh5->c_table[lh5->bitbuf >> (BITBUFSIZ - 12)];
	if (j >= NC)
	{
		mask = 1U << (BITBUFSIZ - 1 - 12);
		do
		{
			if (lh5->bitbuf & mask)
				j = lh5->right[j];
			else
				j = lh5->left[j];
			mask >>= 1;
		} while (j >= NC);
	}
	fillbuf(lh5, lh5->c_len[j]);
	return j;
}


static unsigned short decode_p(lh5_decoder *lh5)
{
	unsigned short j, mask;

	j = lh5->pt_table[lh5->bitbuf >> (BITBUFSIZ - 8)];
	if (j >= NP)
	{
		mask = 1U << (BITBUFSIZ - 1 - 8);
		do
		{
			if (lh5->bitbuf & mask)
				j = lh5->right[j];
			else
				j = lh5->left[j];
			mask >>= 1;
		} while (j >= NP);
	}
	fillbuf(lh5, lh5->pt_len[j]);
	if (j != 0)
		j = (1U << (j - 1)) + getbits(lh5, j - 1);
	return j;
}


static void init_getbits(lh5_decoder *lh5)
{
	lh5->bitbuf = 0;
	lh5->subbitbuf = 0;
	lh5->bitcount = 0;
	fillbuf(lh5, BITBUFSIZ);
}


static void decode_start(lh5_decoder *lh5)
{
	init_getbits(lh5);
	lh5->blocksize = 0;
	lh5->dec_j = 0;
}


/* The calling function must keep the number of
   bytes to be processed.  This function decodes
   either 'count' bytes or 'DICSIZ' bytes, whichever
   is smaller.
   Call decode_start() once for each new file
   before calling this function. */
static void decode5(lh5_decoder *lh5, unsigned int count)
{
	unsigned short c;
	unsigned int r;

	r = 0;
	while (--lh5->dec_j >= 0)
	{
		lh5->buffer[r] = lh5->buffer[lh5->dec_i];
		lh5->dec_i = (lh5->dec_i + 1) & (DICSIZ - 1);
		if (++r == count)
			return;
	}
	for (;;)
	{
		c = decode_c(lh5);

		if (c <= UCHAR_MAX)
		{
			lh5->buffer[r] = c;
			if (++r == count)
				return;
		} else
		{
			lh5->dec_j = c - (UCHAR_MAX + 1 - THRESHOLD);
			lh5->dec_i = (r - decode_p(lh5) - 1) & (DICSIZ - 1);
			while (--lh5->dec_j >= 0)
			{
				lh5->buffer[r] = lh5->buffer[lh5->dec_i];
				lh5->dec_i = (lh5->dec_i + 1) & (DICSIZ - 1);
				if (++r == count)
					return;
			}
		}
	}
}


gboolean lh5_decode(unsigned char *unpackedMem, unsigned long unpackedLen, const unsigned char *packedMem, unsigned long packedLen)
{
	unsigned int n;
	lh5_decoder *lh5;
	unsigned short crc;
	
	lh5_make_crctable();
	lh5 = g_new0(lh5_decoder, 1);
	if (lh5 == NULL)
		return FALSE;
	lh5->unpackedMem = unpackedMem;
	lh5->unpackedLen = unpackedLen;
	lh5->packedMem = packedMem;
	lh5->packedLen = packedLen;
	decode_start(lh5);
	crc = INIT_CRC;
	while (lh5->unpackedLen != 0)
	{
		n = (unsigned int) ((lh5->unpackedLen > DICSIZ) ? DICSIZ : lh5->unpackedLen);
		decode5(lh5, n);
		crc = lh5_update_crc(lh5->buffer, n, crc);
		lh5->unpackedLen -= n;
		memcpy(lh5->unpackedMem, lh5->buffer, n);
		lh5->unpackedMem += n;
	}
	g_free(lh5);
	return TRUE;
}
