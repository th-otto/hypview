#include "gem_vdiP.h"

/** 
 *
 *  @param src input string (standard null-terminated C-string)
 *  @param des output string (VDI format, each char occupied 16 bits)
 *
 *  @return the len of the string
 *
 */

short
vdi_str2array (const char *src, vdi_wchar_t *des)
{
	short len = 0;
	const unsigned char *c  = (const unsigned char *) src;

	while (*c)
	{
		*(des++) = *(c++);
		len++;
	}
	return len;
}

/** 
 *
 *  @param src input string (standard null-terminated C-string)
 *  @param des output string (VDI format, each char occupied 16 bits)
 *  @param nmax maximum of char to translate in the output string
 *
 *  @return the len of the string
 *
 */

short
vdi_str2arrayn (const char *src, vdi_wchar_t *des, short nmax)
{
	short i = 0;
	const unsigned char *c  = (const unsigned char *) src;

	while (i < nmax && *c)
	{
		*(des++) = *(c++);
		i++;
	}
	return i;
}
