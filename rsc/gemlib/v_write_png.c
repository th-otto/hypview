#include "gem_vdiP.h"

short v_write_png(short handle, const char *filename)
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intin[VDI_INTINMAX];   
	short vdi_intout[1];
	short n = vdi_str2array(filename, (vdi_wchar_t *)vdi_intin);

	VDI_PARAMS(vdi_control, vdi_intin, NULL, vdi_intout, vdi_dummy);

	VDI_TRAP_ESC(vdi_params, handle, 96, 1, 0, n);
	return vdi_intout[0];
}
