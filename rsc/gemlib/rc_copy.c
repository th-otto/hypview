#include "gem_aesP.h"

/** copy a GRECT structure
 *
 *  @param src
 *  @param dst
 *
 *  @return always 1.
 *
 */
 
void
rc_copy (const GRECT * src, GRECT * dst)
{
	*dst = *src;
}
