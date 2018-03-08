include ../Makefile.common

INCLUDES = -I$(hyp_srcdir)/include
AM_CPPFLAGS = $(GLIB_CFLAGS) \
	-DBINDIR="\"$(bindir)\""

noinst_PROGRAMS = help_rc

help_rc_SOURCES = help_rc.c help_rc.h
help_rc_LDADD = $(LIBINTL)
