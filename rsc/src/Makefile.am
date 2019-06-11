AUTOMAKE_OPTIONS = subdir-objects

include $(top_srcdir)/silent.am
include $(top_srcdir)/Makefile.common

AM_CPPFLAGS = -I$(srcdir) -I$(srcdir)/../include -I$(top_srcdir)/include
AM_CPPFLAGS += $(GLIB_CFLAGS)
AM_CPPFLAGS += $(PNG_CFLAGS)
AM_CPPFLAGS += $(Z_CFLAGS)

noinst_PROGRAMS = rscview
noinst_LIBRARIES = librsc.a

librsc_a_SOURCES = \
	fileio.h rso.h \
	rsrcload.c \
	object.c \
	rsc.c \
	rsc_lang.c rsc_lang.h langtab.h \
	fileio.c \
	bghio.c \
	debug.c \
	win32.c \
	$(empty)

rscview_SOURCES = \
	rscview.c \
	glib.c \
	fio_err.c \
	$(empty)

$(top_builddir)/rsc/src/librsc.a:: librsc.a

rscview_LDADD = $(GEMLIBS) $(LIBPNG) $(LIBZ) $(GLIB_LIBS)
rscview_DEPENDENCIES = $(GEMLIB_DEPS)
