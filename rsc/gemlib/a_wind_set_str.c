#include "gem_aesP.h"
#include "debug.h"

/** sets various window attributes.
 *
 *  @param WindowHandle specifies the window handle of the window to modify.
 *  @param What specifies the attribute to change
 *  @param str a string
 *  @param global_aes global AES array
 *
 *  see mt_wind_set() documentation for more details.
 *
 *  the \a str pointer (32 bits) is sent in place of the two first parameters
 *  \a w1 (most significant word of \a str) and \a w2 (less significant word
 *  of \a str) of mt_wind_set(). Parameters \a w3 and \a w4 of mt_wind_set() are
 *  undefined.
 */

short wind_set_str (short WindowHandle, short What, const char *str)
{
	AES_PARAMS(105,2 + 2 * N_PTRINTS,1,0,0);

	aes_intin[0]                  = WindowHandle;
	aes_intin[1]                  = What;

	aes_intin_ptr(2, char *) = (char *)NO_CONST(str);
	aes_intin_ptr(2 + N_PTRINTS, char *) = NULL;
	
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
		KINFO(("wind_set_str() called for type %d\n", What));
		return 0;
	}
	
	AES_TRAP(aes_params);

	return aes_intout[0];
}
