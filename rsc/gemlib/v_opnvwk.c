#include "gem_vdiP.h"

/** opens a virtual workstation on an already 
 *  opened physical device (the screen). The attributes for each (virtual) 
 *  workstation are maintained separately.
 *
 *  @param work_in same as v_opnwk()
 *  @param handle before the call, \p handle specifies the handle of the
 *         physical workstation (returned by graf_handle). \n
 *         After the call, \p handle will contain the handle of the virtual
 *         workstation
 *  @param work_out same as v_opnwk()
 *
 *  @since all VDI versions
 *
 */


void
v_opnvwk (short work_in[], short *handle, short work_out[])
{
	short vdi_control[VDI_CNTRLMAX]; 

	VDI_PARAMS(vdi_control, work_in, 0L, &work_out[0], &work_out[45]);
	
	VDI_TRAP (vdi_params, *handle, 100, 0,11);

	*handle = VDI_HANDLE;
}
