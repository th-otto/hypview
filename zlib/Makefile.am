noinst_LIBRARIES = libz.a

AM_CPPFLAGS =
AM_CPPFLAGS += -I$(top_srcdir)
AM_CPPFLAGS += -DZLIB_STATIC

libz_a_SOURCES = \
	zlib.h zconf.h \
	adler32.c \
	crc32.c crc32.h \
	deflate.c deflate.h \
	gzclose.c \
	gzguts.h \
	gzlib.c \
	gzread.c \
	gzwrite.c \
	infback.c \
	inffast.c inffast.h \
	inflate.c inflate.h \
	inftrees.c inftrees.h \
	trees.c trees.h \
	compress.c \
	uncompr.c \
	zutil.c zutil.h \
	\
	ioapi.c ioapi.h \
	zip.c zip.h \
	unzip.c unzip.h \
	iowin32.c iowin32.h \
	\
	$(empty)

EXTRA_DIST = makecrc.c
