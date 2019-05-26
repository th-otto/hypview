#include "gem_aesP.h"

short aes(AESPB *pb)
{
	_AES_TRAP(pb);
	
	return pb->intout[0];
}
