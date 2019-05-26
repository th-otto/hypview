#include "gem_aesP.h"

short wind_set_int (short WindowHandle, short What, short i)
{
	AES_PARAMS(105,6,1,0,0);

	aes_intin[0]                  = WindowHandle;
	aes_intin[1]                  = What;
	aes_intin[2]                  = i;
	aes_intin[3]                  = 0;
	aes_intin[4]                  = 0;
	aes_intin[5]                  = 0;
	
	AES_TRAP(aes_params);

	return aes_intout[0];
}
