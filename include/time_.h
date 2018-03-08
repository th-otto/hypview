#ifndef __TIME__H__
#define __TIME__H__

#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#include <sys/time.h>
#else
#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifndef HAVE_LOCALTIME_R
#define localtime_r(timer, tp) ((*(tp) = *localtime(timer)))
#endif
#ifndef HAVE_GMTIME_R
#define gmtime_r(timer, tp) ((*(tp) = *gmtime(timer)))
#endif
#ifndef HAVE_CTIME_R
#define ctime_r(time, buf) (strcpy(buf, ctime(time)))
#endif

#endif /* __TIME__H__ */
