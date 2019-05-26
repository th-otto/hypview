#include "gem_vdiP.h"
#include "mt_gemx.h"

/** 
 *
 *  @param handle Device handle
 *  @param src_bm 
 *  @param dst_bm 
 *  @param src_rect 
 *  @param dst_rect 
 *  @param mode 
 *
 *  @since NVDI 5 ?
 *
 *
 *
 */

void vr_transfer_bits (short handle, GCBITMAP * src_bm, GCBITMAP * dst_bm, const RECT16 *src_rect, const RECT16 *dst_rect, short mode)
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intin[4];   
	short vdi_ptsin[8];   

	VDI_PARAMS(vdi_control, vdi_intin, vdi_ptsin, vdi_dummy, vdi_dummy);
	
	vdi_intin[0] = mode;
	vdi_intin[1] = 0;
	vdi_intin[2] = 0;
	vdi_intin[3] = 0;

	*(RECT16 *)(vdi_ptsin +0) = *src_rect;
	*(RECT16 *)(vdi_ptsin +4) = *dst_rect;

	vdi_control_ptr(0, GCBITMAP *)  = src_bm;
	vdi_control_ptr(1, GCBITMAP *)  = dst_bm;
	vdi_control_ptr(2, void *) = NULL;

	VDI_TRAP (vdi_params, handle, 170, 4,4);
}
