include ../Makefile.common

noinst_LIBRARIES = libpic.a

AM_CPPFLAGS = $(GLIB_CFLAGS) \
	-DBINDIR="\"$(bindir)\""
AM_CPPFLAGS += -I$(hyp_srcdir)/include -I$(top_srcdir)/src

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
	pictools.c \
	piccolor.c
