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
#ifdef HAVE_STAT_H
# include <stat.h>
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

#ifdef __cplusplus
extern "C" {
#endif

int rpl_stat(const char *path, struct stat *st);
int rpl_lstat(const char *path, struct stat *st);
int rpl_fstat(int handle, struct stat *st);

int hyp_utf8_stat(const char *path, struct stat *st);
int hyp_utf8_lstat(const char *path, struct stat *st);

#ifdef __cplusplus
}
#endif

#ifdef __LINUX_GLIBC_WRAP_H

#if __GLIBC_PREREQ(2, 33)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * avoid references to stat/lstat/fstat, which is only available in glibc >= 2.33
 */

extern int __fxstat(int __ver, int __fildes, struct stat *__stat_buf)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3)));
extern int __xstat(int __ver, const char *__filename,
      struct stat *__stat_buf) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 3)));
extern int __lxstat(int __ver, const char *__filename,
       struct stat *__stat_buf) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 3)));
extern int __fxstatat (int __ver, int __fildes, const char *__filename,
         struct stat *__stat_buf, int __flag)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (3, 4)));

#define stat(__path, __statbuf) __xstat(1, __path, __statbuf)
#define lstat(__path, __statbuf) __lxstat(1, __path, __statbuf)
#define fstat(__fd, __statbuf) __fxstat(1, __fd, __statbuf)
#define fstatat(__fd, __filename, __statbuf, __flag) __fxstatat(1, __fd, __filename, __statbuf, __flag)

#ifdef __cplusplus
}
#endif

#endif /* __GLIBC_PREREQ */

#endif /* __LINUX_GLIBC_WRAP_H */

#endif /* __HYP_STAT__H__ */
