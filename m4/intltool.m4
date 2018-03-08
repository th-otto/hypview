dnl AC_PROG_INTLTOOL([MINIMUM-VERSION])
# serial 1 AC_PROG_INTLTOOL
AC_DEFUN([AC_PROG_INTLTOOL],
[

if test -n "$1"; then
    AC_MSG_CHECKING(for intltool >= $1)

    INTLTOOL_REQUIRED_VERSION_AS_INT=`echo $1 | awk -F. '{ printf "%d", $[1] * 100 + $[2]; }'`
    INTLTOOL_APPLIED_VERSION=`awk -F\" '/\\$VERSION / { printf $[2]; }'  < intltool-update.in`
    changequote({{,}})
    INTLTOOL_APPLIED_VERSION_AS_INT=`awk -F\" '/\\$VERSION / { split(${{2}}, VERSION, "."); printf "%d\n", VERSION[1] * 100 + VERSION[2];}' < intltool-update.in`
    changequote([,])

    if test "$INTLTOOL_APPLIED_VERSION_AS_INT" -ge "$INTLTOOL_REQUIRED_VERSION_AS_INT"; then
	AC_MSG_RESULT([$INTLTOOL_APPLIED_VERSION found])
    else
	AC_MSG_RESULT([$INTLTOOL_APPLIED_VERSION found. Your intltool is too old.  You need intltool $1 or later.])
	exit 1
    fi
fi

  INTLTOOL_DESKTOP_RULE='%.desktop:   %.desktop.in   $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< [$]@ -d -u -c $(top_builddir)/po/.intltool-merge-cache'
INTLTOOL_DIRECTORY_RULE='%.directory: %.directory.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< [$]@ -d -u -c $(top_builddir)/po/.intltool-merge-cache'
     INTLTOOL_KEYS_RULE='%.keys:      %.keys.in      $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< [$]@ -k -u -c $(top_builddir)/po/.intltool-merge-cache'
      INTLTOOL_OAF_RULE='%.oaf:       %.oaf.in       $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< [$]@ -o -p'
     INTLTOOL_PONG_RULE='%.pong:      %.pong.in      $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< [$]@ -x -u -c $(top_builddir)/po/.intltool-merge-cache'
   INTLTOOL_SERVER_RULE='%.server:    %.server.in    $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< [$]@ -o -u -c $(top_builddir)/po/.intltool-merge-cache'
    INTLTOOL_SHEET_RULE='%.sheet:     %.sheet.in     $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< [$]@ -x -u -c $(top_builddir)/po/.intltool-merge-cache'
INTLTOOL_SOUNDLIST_RULE='%.soundlist: %.soundlist.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< [$]@ -d -u -c $(top_builddir)/po/.intltool-merge-cache'
       INTLTOOL_UI_RULE='%.ui:        %.ui.in        $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< [$]@ -x -u -c $(top_builddir)/po/.intltool-merge-cache'
      INTLTOOL_XML_RULE='%.xml:       %.xml.in       $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< [$]@ -x -u -c $(top_builddir)/po/.intltool-merge-cache'
    INTLTOOL_CAVES_RULE='%.caves:     %.caves.in     $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< [$]@ -d -u -c $(top_builddir)/po/.intltool-merge-cache'

AC_SUBST(INTLTOOL_DESKTOP_RULE)
AC_SUBST(INTLTOOL_DIRECTORY_RULE)
AC_SUBST(INTLTOOL_KEYS_RULE)
AC_SUBST(INTLTOOL_OAF_RULE)
AC_SUBST(INTLTOOL_PONG_RULE)
AC_SUBST(INTLTOOL_SERVER_RULE)
AC_SUBST(INTLTOOL_SHEET_RULE)
AC_SUBST(INTLTOOL_SOUNDLIST_RULE)
AC_SUBST(INTLTOOL_UI_RULE)
AC_SUBST(INTLTOOL_XML_RULE)
AC_SUBST(INTLTOOL_CAVES_RULE)

# Use the tools built into the package, not the ones that are installed.

INTLTOOL_EXTRACT='$(top_builddir)/intltool-extract'
INTLTOOL_MERGE='$(top_builddir)/intltool-merge'
INTLTOOL_UPDATE='$(top_builddir)/intltool-update'

AC_SUBST(INTLTOOL_EXTRACT)
AC_SUBST(INTLTOOL_MERGE)
AC_SUBST(INTLTOOL_UPDATE)

AC_PATH_PROG(INTLTOOL_PERL, perl)
if test -z "$INTLTOOL_PERL"; then
   AC_MSG_ERROR([perl not found; required for intltool])
fi
if test -z "`$INTLTOOL_PERL -v | fgrep '5.' 2> /dev/null`"; then
   AC_MSG_ERROR([perl 5.x required for intltool])
fi

# Remove file type tags (using []) from po/POTFILES.

ifdef([AC_DIVERSION_ICMDS],[
  AC_DIVERT_PUSH(AC_DIVERSION_ICMDS)
      changequote(,)
      mv -f po/POTFILES po/POTFILES.tmp
      sed -e 's/\[.*\] *//' < po/POTFILES.tmp > po/POTFILES
      rm -f po/POTFILES.tmp
      changequote([,])
  AC_DIVERT_POP()
],[
  ifdef([AC_CONFIG_COMMANDS_PRE],[
    AC_CONFIG_COMMANDS_PRE([
        changequote(,)
        mv -f po/POTFILES po/POTFILES.tmp
        sed -e 's/\[.*\] *//' < po/POTFILES.tmp > po/POTFILES
        rm -f po/POTFILES.tmp
        changequote([,])
    ])
  ])
])

# Manually sed perl in so people don't have to put the intltool scripts in AC_OUTPUT.

AC_CONFIG_COMMANDS([intltool],[

sed -e "s:@INTLTOOL_PERL@:${INTLTOOL_PERL}:;" < ${srcdir}/intltool-extract.in > intltool-extract.out
if cmp -s intltool-extract intltool-extract.out 2>/dev/null; then
  rm -f intltool-extract.out
else
  mv -f intltool-extract.out intltool-extract
fi
chmod ugo+x intltool-extract
chmod u+w intltool-extract

sed -e "s:@INTLTOOL_PERL@:${INTLTOOL_PERL}:;" < ${srcdir}/intltool-merge.in > intltool-merge.out
if cmp -s intltool-merge intltool-merge.out 2>/dev/null; then
  rm -f intltool-merge.out
else
  mv -f intltool-merge.out intltool-merge
fi
chmod ugo+x intltool-merge
chmod u+w intltool-merge

sed -e "s:@INTLTOOL_PERL@:${INTLTOOL_PERL}:;" < ${srcdir}/intltool-update.in > intltool-update.out
if cmp -s intltool-update intltool-update.out 2>/dev/null; then
  rm -f intltool-update.out
else
  mv -f intltool-update.out intltool-update
fi
chmod ugo+x intltool-update
chmod u+w intltool-update

], INTLTOOL_PERL=${INTLTOOL_PERL})

])


