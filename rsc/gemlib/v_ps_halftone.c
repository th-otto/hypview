#include "gem_vdiP.h"

void v_ps_halftone      (VdiHdl handle , short index, short angle, short frequency )
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intin[3];

	VDI_PARAMS(vdi_control, vdi_intin, 0L, vdi_dummy, vdi_dummy );
	
	vdi_intin[0] = index;
	vdi_intin[1] = angle;
	vdi_intin[2] = frequency;
	
	VDI_TRAP_ESC (vdi_params, handle, 5,32, 0,3);
}
