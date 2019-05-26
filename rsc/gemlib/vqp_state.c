#include "gem_vdiP.h"

void vqp_state (short handle, short *port, short *film, short *lightness, short *interlace, short *planes, short *indexes )
{
	short vdi_control[VDI_CNTRLMAX]; 
	short vdi_intout[VDI_INTOUTMAX]; 
	int i;
	
	VDI_PARAMS(vdi_control, vdi_dummy, 0L, vdi_intout, vdi_dummy );
		
	VDI_TRAP_ESC (vdi_params, handle, 5,92, 0,0);
	
	*port = vdi_intout[0];
	*film = vdi_intout[0];
	*lightness = vdi_intout[2];
	*interlace = vdi_intout[3];
	*planes = vdi_intout[4];
	
	for (i = 0; i < 16; i++)
		indexes[i] = vdi_intout[5 + i];
}
