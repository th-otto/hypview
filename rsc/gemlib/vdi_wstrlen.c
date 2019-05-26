#include "gem_vdiP.h"

/** return the length of a w-string
 *
 *  @param wstr a string with 16 bits per character, null-terminated.
 *
 *  @return the length of the string
 *
 *
 */

short
vdi_wstrlen (const vdi_wchar_t *wstr)
{
	register short len = 0;
	
	while (*wstr++)
		len++;
	
	return len;
}
