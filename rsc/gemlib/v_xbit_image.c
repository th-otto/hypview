#include "gem_vdiP.h"

void v_xbit_image(short handle, const char *filename, short aspect, short x_scale, short y_scale, short h_align, short v_align, short rotation, short background, short foreground, short xy[])
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intin[VDI_INTINMAX];
	short n;
	
	VDI_PARAMS(vdi_control, vdi_intin, xy, vdi_dummy, vdi_dummy);
		
    vdi_intin[0] = aspect;
    vdi_intin[1] = x_scale;
    vdi_intin[2] = y_scale;
    vdi_intin[3] = h_align;
    vdi_intin[4] = v_align;
    vdi_intin[5] = rotation;
    vdi_intin[6] = background;
    vdi_intin[7] = foreground;
    
    n = 8 + vdi_str2array(filename, (vdi_wchar_t *)vdi_intin + 8);
    
	VDI_TRAP_ESC (vdi_params, handle, 5,101, 2,n);
}
