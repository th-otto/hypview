include $(top_srcdir)/silent.am
include $(top_srcdir)/Makefile.common

AM_CPPFLAGS = -I$(srcdir) -I$(srcdir)/../include $(GLIB_CFLAGS)

noinst_LIBRARIES = libaes.a

libaes_a_SOURCES = \
	aes.c aes.h \
	aesdefs.h crysbind.h gemlib.h gempd.h gsxdefs.h struct.h \
	\
	gemaplib.c \
	gembase.c \
	gemctrl.c \
	gemevlib.c \
	gemfmalt.c \
	gemfmlib.c \
	gemfslib.c \
	gemglobe.c \
	gemgraf.c \
	gemgrlib.c \
	gemgsxif.c \
	geminit.c \
	geminput.c \
	gemmnlib.c \
	gemobed.c \
	gempd.c \
	gemobjop.c \
	gemoblib.c \
	gemrslib.c \
	gemsclib.c \
	gemshlib.c \
	gemwmlib.c \
	gemwrect.c \
	aesutils.c \
	\
	gemdos.c \
	\
	gem_rsc.c gem_rsc.rsc gem_rsc.rso gem_rsc.rsh gem_rsc.h \
	\
	$(empty)
