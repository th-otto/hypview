#include "gem_vdiP.h"

short vqp_films (short handle, char * name)
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intout[VDI_INTOUTMAX]; 
	short vdi_intin[1]; 

	VDI_PARAMS(vdi_control, vdi_intin, 0L, vdi_intout, vdi_dummy );

	vdi_intin[0] = 0;
		
	VDI_TRAP_ESC (vdi_params, handle, 5,91, 0,0);
	
	vdi_array2str( vdi_intout, name, VDI_N_INTOUT);
	
	return VDI_N_INTOUT;
}
