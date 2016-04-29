dnl AM_PATH_CURL([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for libcurl, and define CURL_CFLAGS and CURL_LIBS
dnl
AC_DEFUN([AM_PATH_CURL],
[dnl 
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(curl,
   [AC_HELP_STRING(--with-curl=PFX, [Prefix where libcurl is installed (optional)])],
   [curl_prefix="$withval"],
   [curl_prefix=""])
AC_ARG_WITH(curl-libraries,
   [AC_HELP_STRING(--with-curl-libraries=DIR, [Directory where libcurl library is installed (optional)])],
   [curl_libraries="$withval"],
   [curl_libraries=""])
AC_ARG_WITH(curl-includes,
   [AC_HELP_STRING(--with-curl-includes=DIR, [Directory where libcurl header files are installed (optional)])],
   [curl_includes="$withval"],
   [curl_includes=""])

CURL_CFLAGS=""
CURL_LIBS=""
if test "x$curl_prefix" != "xno" ; then

  if test "x$curl_libraries" != "x" ; then
    CURL_LIBS="-L$curl_libraries"
  elif test "x$curl_prefix" != "x" ; then
    CURL_LIBS="-L$curl_prefix/lib"
  elif test "x$prefix" != "xNONE" -a "$prefix" != /usr -a "$prefix" != /mingw ; then
    CURL_LIBS="-L$prefix/lib"
  fi

  CURL_LIBS="$CURL_LIBS -lcurl"

  if test "x$curl_includes" != "x" ; then
    CURL_CFLAGS="-I$curl_includes"
  elif test "x$curl_prefix" != "x" ; then
    CURL_CFLAGS="-I$curl_prefix/include"
  elif test "x$prefix" != "xNONE" -a "$prefix" != /usr -a "$prefix" != /mingw; then
    CURL_CFLAGS="-I$prefix/include"
  fi

  no_curl=""

  ac_save_CFLAGS="$CFLAGS" 
  ac_save_LIBS="$LIBS"
  CFLAGS="$CFLAGS $CURL_CFLAGS"
  LIBS="$LIBS $CURL_LIBS"
  AC_CHECK_HEADERS(curl/curl.h,,no_curl=yes)
  AC_CHECK_LIB([curl], curl_easy_init, , no_curl=yes)
  CFLAGS="$ac_save_CFLAGS"
  LIBS="$ac_save_LIBS"
  AC_MSG_CHECKING(for libcurl)

  if test "x$no_curl" = "x" ; then
     AC_MSG_RESULT(yes)
     ifelse([$1], , :, [$1])     
     AC_DEFINE(HAVE_CURL, 1, [Define if you have libcurl.])
  else
     CURL_CFLAGS=""
     CURL_LIBS=""
     AC_MSG_RESULT(no)
     ifelse([$2], , :, [$2])
  fi
fi
  CURL_LIBDIR=`$pkg_config --variable=libdir libcurl`
  AC_SUBST(CURL_CFLAGS)
  AC_SUBST(CURL_LIBS)
  AC_SUBST(CURL_LIBDIR)
  rm -f conf.curltest
])
