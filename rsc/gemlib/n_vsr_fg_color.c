#include "gem_vdiP.h"
#include "mt_gemx.h"

/** 
 *
 *  @param handle Device handle
 *  @param color_space color space
 *  @param fg_color COLOR_ENTRY of the background color
 *
 *  @return -1=wrong subfunction number, 0=function not present, 1=all ok
 *
 *  @since NVDI 5 ?
 *
 *
 *
 */

short
vsr_fg_color (short handle, int32_t color_space, COLOR_ENTRY * fg_color)
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intin[6];   
	short vdi_intout[1]; 

	VDI_PARAMS(vdi_control, vdi_intin, 0L, vdi_intout, vdi_dummy);
	
	vdi_intin_long(0) = color_space;
	*(COLOR_ENTRY*)&vdi_intin[2] = *fg_color;

	VDI_TRAP_ESC (vdi_params, handle, 200,4, 0,6);

	return vdi_intout[0];
}
