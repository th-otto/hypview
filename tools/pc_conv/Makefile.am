include ../../../../silent.am
include ../../Makefile.common

INCLUDES = -I$(hyp_srcdir)/include -I$(top_srcdir)/src
AM_CPPFLAGS = $(GLIB_CFLAGS) \
	-DBINDIR="\"$(bindir)\""

bin_PROGRAMS = pc_conv

pc_conv_SOURCES = pc_conv.c
