#ifndef __HYPDEFS_H__
#define __HYPDEFS_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 * define __TOS__ if using any compiler for AtariST
 */
#if (defined(atarist) || defined(__atarist)) && !defined(__TOS__)
#  define __TOS__
#endif
#if defined(__TOS__) && !defined(WITH_GUI_GEM)
#  define WITH_GUI_GEM 1
#endif
#if defined(__TOS__) && !defined(HAVE_CONFIG_H)
# define HAVE_UNISTD_H 1
# define HAVE_GETUID 1
# define HAVE_GETEUID 1
# define HAVE_GETGID 1
# define HAVE_GETEGID 1
# define HAVE_GETPID 1
# define HAVE_LSTAT 1
# define HAVE_WCHAR_H 1
# define HAVE_MKDIR 1
#endif

/*
 * define __WIN32__ if using any compiler for Windoze
 */
#include "windows_.h"

#if defined(__MACOSX__) && !defined(__MACOS__)
#  define __MACOS__
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "string_.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "time_.h"
#include "stat_.h"
#include "portab.h"
#include "hypmem.h"
#include "hypintl.h"
#if defined(HAVE_WCHAR_H)
#include <wchar.h>
#elif defined(__WCHAR_TYPE__)
typedef __WCHAR_TYPE__ wchar_t;
#else
typedef unsigned short wchar_t;
#endif

#ifdef __TOS__
#include "tos/gem.h"
#else
#include "tos/nogem.h"
#endif


#ifndef ENAMETOOLONG
#  define ENAMETOOLONG EINVAL
#endif
#ifndef EFAULT
#  define EFAULT EINVAL
#endif


#define	LINE_BUF		512


#include "hyp.h"

#if defined(__WIN32__) || defined(__TOS__)
#define filename_cmp strcasecmp
#define filename_ncmp strncasecmp
#else
#define filename_cmp strcmp
#define filename_ncmp strncmp
#endif

#ifdef __TOS__
extern BASEPAGE *SetActPD (BASEPAGE *newpd);
extern BASEPAGE *_base;
extern int acc_memsave;
#define SavePD() \
	BASEPAGE *old = NULL; \
	if (acc_memsave) \
		old = SetActPD(_base)
#define RestorePD() \
	if (acc_memsave) \
		SetActPD(old)
#endif


extern char const gl_program_name[];
extern char const gl_program_version[];
extern char const gl_compile_date[12];


/*
 *		Global.c
 */
extern _WORD font_cw, font_ch;

/*
 *		File.c
 */
#ifdef __TOS__
char GetBootDrive(void);
#endif


/*
 *		Error.c
 */
void FileError(const char *path, const char *str);
void FileErrorNr(const char *path, int err_no);
void FileErrorErrno(const char *path);
void FileExecError(const char *path);


#endif /* __HYPDEFS_H__ */
