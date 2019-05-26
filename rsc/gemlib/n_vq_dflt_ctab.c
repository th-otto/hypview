#include "gem_vdiP.h"
#include "mt_gemx.h"

/** 
 *
 *  @param handle Device handle
 *  @param ctab_length 
 *  @param ctab 
 *
 *  @return 
 *
 *  @since NVDI 5 ?
 *
 *
 *
 */

short
vq_dflt_ctab (short handle, int32_t ctab_length, COLOR_TAB *ctab)
{
	short vdi_control[VDI_CNTRLMAX]; 

	VDI_PARAMS(vdi_control, (short*)&ctab_length, 0L, (short*)ctab, vdi_dummy);
	
	VDI_TRAP_ESC (vdi_params, handle, 206,7, 0,2);

	return VDI_N_INTOUT ? 1 : 0;
}
