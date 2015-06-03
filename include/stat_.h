#ifndef __HYP_STAT__H__
#define __HYP_STAT__H__

#if defined(__PUREC__)
#define HAVE_SYS_STAT_H
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if defined(HAVE_SYS_STAT_H)
#include <sys/stat.h>
#endif

#ifndef O_RDONLY
#  ifdef _O_RDONLY
#    define O_RDONLY _O_RDONLY
#  endif
#endif

#ifndef O_BINARY
#  ifdef _O_BINARY
#    define O_BINARY _O_BINARY
#  endif
#endif

#ifndef O_BINARY
#  define O_BINARY 0
#endif

#ifndef O_EXCL
#  ifdef _O_EXCL
#    define O_EXCL _O_EXCL
#  endif
#endif

#ifndef O_EXCL
#  define O_EXCL 0
#endif


#ifndef S_ISDIR
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

/*
 * almost all systems need a replacement for stat
 */

/*
 * dont rely on #define stat() rpl_stat(),
 * it might already have been redefined by system headers.
 * use rpl_stat() directly in your sources.
 */

int rpl_stat(const char *path, struct stat *st);
int rpl_lstat(const char *path, struct stat *st);
int rpl_fstat(int handle, struct stat *st);

int hyp_utf8_stat(const char *path, struct stat *st);
int hyp_utf8_lstat(const char *path, struct stat *st);

#endif /* __HYP_STAT__H__ */
