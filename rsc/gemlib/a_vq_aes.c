#include "gem_aesP.h"

short vq_aes(void)
{
	appl_init();
	return gl_ap_version == 0 ? -1 : gl_apid;
}
