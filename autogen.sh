#! /bin/sh
# Run this to generate all the initial makefiles, etc.
# This was lifted from the BasiliskII, and adapted slightly by
# Milan Jurik.

DIE=0

top_srcdir=`dirname $0`

PROG="hcp"

# Check for GNU make

test -z "$MAKE" && MAKE=make

# Check how echo works in this /bin/sh
case `echo -n` in
-n) _echo_n=   _echo_c='\c';;
*)  _echo_n=-n _echo_c=;;
esac

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
        echo
        echo "You must have autoconf installed to compile $PROG."
        echo "Download the appropriate package for your distribution,"
        echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/autoconf/"
        DIE=1
}

(aclocal --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "**Error**: Missing aclocal. The version of automake"
	echo "installed doesn't appear recent enough."
	echo "Get ftp://ftp.gnu.org/pub/gnu/automake/automake-1.15.1.tar.xz"
	echo "(or a newer version if it is available)"
	DIE=1
}

if test "$DIE" -eq 1; then
        exit 1
fi

(
cd "$top_srcdir" || exit 1

rm -rf autom4te.cache

ACLOCAL_FLAGS="-I m4"
aclocalinclude="$ACLOCAL_FLAGS"; \
(echo $_echo_n " + Running aclocal: $_echo_c"; \
    aclocal $aclocalinclude; \
 echo "done.") && \
(echo $_echo_n " + Running autoconf: $_echo_c"; \
    autoconf; \
 echo "done.") && \
(echo $_echo_n " + Running autoheader: $_echo_c"; \
    autoheader; \
 echo "done.") && \
(echo $_echo_n " + Running automake: $_echo_c"; \
    automake --add-missing --copy; \
 echo "done.") 

rm -rf autom4te.cache

) || exit 1


echo all done
