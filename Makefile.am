AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = rcintl potools pic icons po hyp
if HAVE_PNG
SUBDIRS += zlib libpng
endif
if WITH_PDF
SUBDIRS += hpdf
endif
SUBDIRS += gtk win32 tos tools doc www

EXTRA_DIST = \
	include defaults skins \
	intltool-merge.in intltool-extract.in intltool-update.in \
	COPYING \
	langid/model.c \
	$(empty)

DISTCLEANFILES = intltool-merge intltool-extract intltool-update
