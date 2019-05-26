#include "gem_vdiP.h"
#include "mt_gemx.h"

void v_killoutline (short handle, fsm_component_t *component)
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intin[N_PTRINTS];

	VDI_PARAMS(vdi_control, vdi_intin, 0L, vdi_dummy, vdi_dummy);
	
	vdi_intin_ptr(0, fsm_component_t *) = component;
	
	VDI_TRAP (vdi_params, handle, 242, 0, N_PTRINTS);
}
