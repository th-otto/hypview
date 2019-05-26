#include "gem_vdiP.h"
#include "mt_gemx.h"

/** This function exists only for compatibility reasons. It switches off the 
 *  special treatment of v_bez()/v_bez_fill().
 *
 *  @param handle Device handle
 *
 *  @since since NVDI 2.10
 *
 *
 *
 */

void
v_bez_off (short handle)
{
	v_bez_con(handle, 0);
}
