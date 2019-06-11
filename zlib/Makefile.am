noinst_LIBRARIES = libz.a

AM_CPPFLAGS =
AM_CPPFLAGS += -I$(top_srcdir)/include
AM_CPPFLAGS += -DZLIB_STATIC

libz_a_SOURCES = \
	zlib.h zconf.h \
	adler32.c \
	crc32.c crc32.h \
	deflate.c deflate.h \
	infback.c \
	inffast.c inffast.h inffixed.h \
	inflate.c inflate.h \
	inftrees.c inftrees.h \
	trees.c trees.h \
	zutil.c zutil.h \
	dbgmem.h \
	compress.c \
	uncompr.c \
	gzclose.c gzguts.h \
	gzlib.c \
	gzread.c \
	gzwrite.c \
	\
	ioapi.c ioapi.h crypt.h \
	zip.c zip.h \
	unzip.c unzip.h \
	iowin32.c iowin32.h \
	\
	$(empty)

EXTRA_DIST = makecrc.c
