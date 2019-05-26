#include "gem_vdiP.h"

/** This function report back, if vs_calibrate() exists and if calibration is 
 *  switched on.
 *
 *  @param handle Device handle
 *  @param flag calibration aus (0) or ein (1)
 *
 *  @return O if vs_calibrate() is not supported by the driver, any other value
 *          if vs_calibrate() is supported by the driver.
 *
 *  @since depends on the driver
 *
 */

short
vq_calibrate (short handle, short *flag)
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intout[1];

	VDI_PARAMS(vdi_control, 0L, 0L, vdi_intout, vdi_dummy );
	
	vdi_intout[0] = 0;
	VDI_TRAP_ESC (vdi_params, handle, 5,77, 0,0);
	
	*flag = vdi_intout[0];
	return VDI_N_INTOUT;
}
