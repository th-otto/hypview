include ../../../../silent.am
include ../../Makefile.common

AM_CPPFLAGS = $(GLIB_CFLAGS) \
	-DBINDIR="\"$(bindir)\""
AM_CPPFLAGS += -I$(hyp_srcdir)/include -I$(top_srcdir)/src

noinst_PROGRAMS = help_rc

help_rc_SOURCES = help_rc.c help_rc.h malloc.c
