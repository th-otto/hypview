include ../../../../silent.am
include ../../Makefile.common

AM_CPPFLAGS = $(GLIB_CFLAGS) \
	-DBINDIR="\"$(bindir)\""
AM_CPPFLAGS += -I$(hyp_srcdir)/include -I$(top_srcdir)/src

bin_PROGRAMS = 1st_conv

1st_conv_SOURCES = 1st_conv.c
if OS_WIN32
1st_conv_DEPENDENCIES = $(LIBRCINTL_DEP)
endif
1st_conv_LDADD = $(INTLLIBS)
