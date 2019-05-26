#include "gem_vdiP.h"

/** 
 *
 *  @param handle Device handle
 *
 *  @since all VDI versions
 *
 *  @sa
 *
 *
 *
 */

void
vsp_save (short handle)
{
	short vdi_control[VDI_CNTRLMAX]; 
	
	VDI_PARAMS(vdi_control, vdi_dummy, 0L, vdi_dummy, vdi_dummy );
	
	VDI_TRAP_ESC (vdi_params, handle, 5,94, 0,0);
}
