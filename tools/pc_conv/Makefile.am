include $(top_srcdir)/silent.am
include $(top_srcdir)/Makefile.common

AM_CPPFLAGS = $(GLIB_CFLAGS) \
	-DBINDIR="\"$(bindir)\""
AM_CPPFLAGS += -I$(hyp_srcdir)/include -I$(top_srcdir)

bin_PROGRAMS = pc_conv

pc_conv_SOURCES = pc_conv.c
if OS_WIN32
pc_conv_DEPENDENCIES = $(LIBRCINTL_DEP)
endif
pc_conv_LDADD = $(INTLLIBS)
