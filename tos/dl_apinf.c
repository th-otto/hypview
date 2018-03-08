#include "hv_defs.h"

#if 0

_WORD appl_xgetinfo(_WORD type, _WORD *out1, _WORD *out2, _WORD *out3, _WORD *out4)
{
    static _WORD has_appl_getinfo = -1;
    _WORD ret;
    
    /* check for appl_getinfo() being present */
    if (has_appl_getinfo < 0)
    {
        _WORD vers, dummy;
        
        has_appl_getinfo = 0;
        /* AES 4.0? */
        if (gl_ap_version >= 0x400)
             has_appl_getinfo = 1;
        else
        if (appl_find( "?AGI\0\0\0\0") >= 0)
            has_appl_getinfo = 2;
        else
        /* WiNX >= 2.2 ? */
        if (wind_get(0, WF_WINX, &vers, &dummy, &dummy, &dummy) == WF_WINX &&
        	(vers & 0xfff) >= 0x220)
            has_appl_getinfo = 3;
    }

    /* no appl_getinfo? return error code */
	if (!has_appl_getinfo || (ret = appl_getinfo(type, out1, out2, out3, out4)) == 0)
	{
	    if (out1 != NULL)
	    	*out1 = 0;
	    if (out2 != NULL)
		    *out2 = 0;
		if (out3 != NULL)
		    *out3 = 0;
		if (out4 != NULL)
		    *out4 = 0;
		return 0;
	}
    return ret;
}

#endif
