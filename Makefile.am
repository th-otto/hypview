AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = include rcintl potools pic icons po
if NEED_ZLIB
SUBDIRS += zlib
endif
if HAVE_PNG
SUBDIRS += libpng
endif
if WITH_PDF
SUBDIRS += hpdf
endif
if NEED_GEMLIB
SUBDIRS += rsc
endif
SUBDIRS += hyp gtk win32 tos macos tools doc www

EXTRA_DIST = \
	defaults skins \
	intltool-merge.in intltool-extract.in intltool-update.in \
	COPYING \
	langid/model.c \
	$(empty)

DISTCLEANFILES = intltool-merge intltool-extract intltool-update
