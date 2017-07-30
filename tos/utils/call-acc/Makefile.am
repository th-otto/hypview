include ../../../../../silent.am
include ../../../Makefile.common

if WITH_GUI_GEM
bin_PROGRAMS = call-acc
endif
EXEEXT = .gtp

LIBGEM = -lgem

AM_CPPFLAGS = -I$(hyp_srcdir)/include -I$(hyp_srcdir)/include/tos -I$(srcdir)/../..

call_acc_SOURCES = \
	call-acc.c

call_acc_LDADD = $(LIBGEM)

distclean-local:
	$(RM) *.pdb

EXTRA_DIST = call-acc.prj
