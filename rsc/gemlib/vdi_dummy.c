#include "gem_vdiP.h"

#if USE_VDI_DUMMY
#ifndef max
#define max(x,y)    (((x)>(y))?(x):(y))
#endif
short vdi_dummy[max(max(max(max(VDI_CNTRLMAX, VDI_INTINMAX), VDI_INTOUTMAX), VDI_PTSOUTMAX), VDI_PTSINMAX)];
#endif
