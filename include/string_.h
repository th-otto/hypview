#include <string.h>

#if defined(__PUREC__) && defined(_PUREC_SOURCE)
#  define strcasecmp stricmp
#  define strncasecmp strnicmp
#endif

