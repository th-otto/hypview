#include "hypdefs.h"

/*
 * on windows, this is already included in librcintl.a
 */
#if !(defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__MSYS__))
#include "../../../rcintl/localenm.c"
#endif
