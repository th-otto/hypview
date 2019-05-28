#include "gem_vdiP.h"
#include "mt_gemx.h"

short
v_hardcopy_ex (short handle, short *pxyarray, int32_t rowstride, void *buffer)
{
	short vdi_control[VDI_CNTRLMAX]; 

	VDI_PARAMS(vdi_control, (short*)&rowstride, pxyarray, (short*)buffer, vdi_dummy);
	
	VDI_TRAP_ESC (vdi_params, handle, 5,17, 2,2);

	return VDI_N_INTOUT;
}
