#include "gem_vdiP.h"

short vqp_error (short handle)
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intout[1]; 

	VDI_PARAMS(vdi_control, vdi_dummy, 0L, vdi_intout, vdi_dummy );
		
	VDI_TRAP_ESC (vdi_params, handle, 5,96, 0,0);
	
	return vdi_intout[0];
}
