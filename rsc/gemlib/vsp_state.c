#include "gem_vdiP.h"

void vsp_state (short handle, short port, short film, short lightness, short interlace, short planes, short *indexes )
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intout[VDI_INTOUTMAX];
	short vdi_intin[21];
	int i;
	
	VDI_PARAMS(vdi_control, vdi_intin, 0L, vdi_intout, vdi_dummy );
		
	vdi_intin[0] = port;
	vdi_intin[1] = film;
	vdi_intin[2] = lightness;
	vdi_intin[3] = interlace;
	vdi_intin[4] = planes;
	
	for (i = 0; i < 16; i++)
		vdi_intin[5 + i] = indexes[i];
	VDI_TRAP_ESC (vdi_params, handle, 5,93, 0,21);
}
