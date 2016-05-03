include ../Makefile.common

noinst_LIBRARIES = libpic.a

INCLUDES = -I$(hyp_srcdir)/include
AM_CPPFLAGS = $(GLIB_CFLAGS) \
	-DBINDIR="\"$(bindir)\""

libpic_a_SOURCES = \
	bmp.c \
	degas.c \
	gif.c \
	ico.c \
	iff.c \
	imagic.c \
	img.c \
	picconv.c \
	stad.c \
	icn.c \
	png.c \
	pictools.c
