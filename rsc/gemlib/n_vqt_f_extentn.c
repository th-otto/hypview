#include "gem_vdiP.h"
#include "mt_gemx.h"

void
vqt_f_extentn (short handle, const char *str, short num, short extent[])
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intin[VDI_INTINMAX];   
	register short n = vdi_str2arrayn (str, (vdi_wchar_t *)vdi_intin, num);

	VDI_PARAMS(vdi_control, vdi_intin, 0L, vdi_dummy, extent);

	VDI_TRAP (vdi_params, handle, 240, 0,n);
}
