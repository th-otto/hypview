include ../Makefile.common

if WITH_GUI_GTK
bin_PROGRAMS = hypview
endif

INCLUDES = -I$(hyp_srcdir)/include

AM_CPPFLAGS =	$(GTK_CFLAGS) \
		-DBINDIR="\"$(bindir)\""
	

hypview_SOURCES = \
	hv_main.c \
	hv_help.c \
	hv_globl.c \
	hv_disp.c \
	hv_click.c \
	hv_curs.c \
	hv_misc.c \
	hv_fixme.c

hypview_DEPENDENCIES = $(LIBHYP_DEP)
hypview_LDADD = $(LIBHYP) $(GTK_LIBS) $(LIBINTL)

