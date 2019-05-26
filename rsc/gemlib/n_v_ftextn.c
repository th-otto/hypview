#include "gem_vdiP.h"
#include "mt_gemx.h"

void
v_ftextn (short handle, short x, short y, const char *str, short num)
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intin[VDI_INTINMAX];   
	short vdi_ptsin[2];   
	register short n = vdi_str2arrayn (str, (vdi_wchar_t *)vdi_intin, num);

	VDI_PARAMS(vdi_control, vdi_intin, vdi_ptsin, vdi_dummy, vdi_dummy);
	
	vdi_ptsin[0] = x;
	vdi_ptsin[1] = y;

	VDI_TRAP (vdi_params, handle, 241, 1,n);
}
