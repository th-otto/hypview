#include "gem_aesP.h"
#include "debug.h"

short wind_set_ptr (short WindowHandle, short What, void *p1, void *p2)
{
	AES_PARAMS(105,2 + 2 * N_PTRINTS,1,0,0);

	aes_intin[0]                  = WindowHandle;
	aes_intin[1]                  = What;

	switch (What)
	{
	case WF_INFO:
	case WF_NAME:
	case WF_NEWDESK:
	case WF_TOOLBAR:
	case WF_USER_POINTER:
	case WF_WIND_ATTACH:
		break;
	default:
		KINFO(("wind_set_ptr() called for type %d\n", What));
		return 0;
	}
	
	aes_intin_ptr(2, void *) = NO_CONST(p1);
	aes_intin_ptr(2 + N_PTRINTS, void *) = NO_CONST(p2);
	
	AES_TRAP(aes_params);

	return aes_intout[0];
}
