#include "gem_vdiP.h"
#include "mt_gemx.h"

short
v_hardcopy_ex (short handle, short *pxyarray, int32_t px_format, int32_t rowstride, void *buffer)
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intin[4]; 

	VDI_PARAMS(vdi_control, vdi_intin, pxyarray, (short*)buffer, vdi_dummy);

	vdi_intin_long(0) = px_format;
	vdi_intin_long(2) = rowstride;
	
	VDI_TRAP_ESC (vdi_params, handle, 5,17, 2,4);

	return VDI_N_INTOUT;
}
