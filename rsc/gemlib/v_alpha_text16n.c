#include "gem_vdiP.h"

/** 
 *
 *  @param handle Device handle
 *  @param wstr
 *  @param num string len  
 *
 *  @since 
 *
 *  @sa
 *
 *
 *
 */

void
v_alpha_text16n (short handle, const vdi_wchar_t *wstr, short num)
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intout[5];

	VDI_PARAMS(vdi_control, (short *)NO_CONST(wstr), 0L, vdi_intout, vdi_dummy);
	
	VDI_TRAP_ESC (vdi_params, handle, 5,25, 0,num);
}
