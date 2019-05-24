AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS =
AM_CPPFLAGS += -I$(top_srcdir)
AM_CPPFLAGS += -I$(srcdir)
AM_CPPFLAGS += -DZLIB_STATIC

noinst_LIBRARIES = libpng.a

libpng_a_SOURCES = \
	png.c \
	pngerror.c \
	pngget.c \
	pngmem.c \
	pngpread.c \
	pngread.c \
	pngrio.c \
	pngrtran.c \
	pngrutil.c \
	pngset.c \
	pngtrans.c \
	pngwio.c \
	pngwrite.c \
	pngwtran.c \
	pngwutil.c \
	intel/intel_init.c \
	intel/filter_sse2_intrinsics.c \
	$(empty)
