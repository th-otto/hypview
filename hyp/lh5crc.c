#include "hypdefs.h"
#include "hypdebug.h"
#include <limits.h>
#include "lh5int.h"

static unsigned short crctable[UCHAR_MAX + 1];


#define UPDATE_CRC(c) \
	crc = crctable[(crc ^ (c)) & 0xFF] ^ (crc >> CHAR_BIT)

#define CRCPOLY  0xA001					/* ANSI CRC-16 */
						 /* CCITT: 0x8408 */

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

void lh5_make_crctable(void)
{
	unsigned int i, j, r;

	static int crc_ready = 0;

	if (crc_ready == 0)
	{
		for (i = 0; i <= UCHAR_MAX; i++)
		{
			r = i;
			for (j = 0; j < CHAR_BIT; j++)
			{
				if (r & 1)
					r = (r >> 1) ^ CRCPOLY;
				else
					r >>= 1;
			}
			crctable[i] = r;
		}
		crc_ready = 1;
	}
}

/* ------------------------------------------------------------------------- */

unsigned short lh5_update_crc(const unsigned char *p, unsigned long n, unsigned short crc)
{
	if (n != 0)
	{
		do
		{
			UPDATE_CRC(*p++);
		} while (--n != 0);
	}
	return crc;
}
