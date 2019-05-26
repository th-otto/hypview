#include "gem_aesP.h"

short appl_init(void)
{
	appl_init();
	return gl_ap_version == 0 ? -1 : gl_apid;
}
