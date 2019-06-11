include $(top_srcdir)/silent.am
include $(top_srcdir)/Makefile.common

AM_CPPFLAGS = -I$(srcdir) -I$(srcdir)/../include -I$(top_srcdir)/include $(GLIB_CFLAGS)
AM_CPPFLAGS += $(PNG_CFLAGS) $(Z_CFLAGS)

noinst_LIBRARIES = libvdi.a

libvdi_a_SOURCES = \
	vdi.c vdi.h \
	vdi_bm.c \
	maptab.c maptab.h \
	\
	vdidefs.h vdimaps.h pattern.h \
	\
	writepng.c writepng.h \
	$(empty)
