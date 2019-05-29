#include "gem_vdiP.h"
#include "mt_gemx.h"

/** see v_ftext_offset()
 *
 *  @param handle Device handle
 *  @param x x-coordinate
 *  @param y y-coordinate
 *  @param wstr string (16 bits per character)
 *  @param num string len
 *  @param offset offsets for the characters
 *
 *
 */

void
v_ftext_offset16n (short handle, short x, short y,
                  const vdi_wchar_t * wstr, short num, const short * offset)
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_ptsin[VDI_PTSINMAX*2+2];   
	const int32_t * src = (const int32_t*)offset;
	int32_t       * dst =       (int32_t*)vdi_ptsin;
	short     i;
	
	VDI_PARAMS(vdi_control, (short *)NO_CONST(wstr), vdi_ptsin, vdi_dummy, vdi_dummy);
	if(num>VDI_PTSINMAX) num=VDI_PTSINMAX;
	
	
	vdi_ptsin[0] = x;
	vdi_ptsin[1] = y;
	for (i = 0; i < num; i++) {
		*(++dst) = *(src++);
	}

	VDI_TRAP (vdi_params, handle, 241, (num +1),num);
}
