include ../Makefile.common

if WITH_GUI_WIN32
bin_PROGRAMS = hypview
endif

INCLUDES = -I$(hyp_srcdir)/include

AM_CPPFLAGS =	-DBINDIR="\"$(bindir)\"" \
		-DPKGDATADIR="\"$(pkgdatadir)\""
	
PO_DIR = $(builddir)/../po
PO_SRCDIR = $(srcdir)/../po

EVAL_POFILES = cat $(PO_SRCDIR)/LINGUAS | sed -e '/^\#/d' -e 's@^\(.*\)@ $(PO_DIR)/\1.po @'
POFILES := $(shell $(EVAL_POFILES))

#WINDRES = windres
WINDRES = orc --compiler none

hypview_SOURCES = \
	hv_main.c \
	hv_defs.h \
	\
	hv_about.c \
	hv_asc.c \
	hv_autol.c \
	hv_bin.c \
	hv_block.c \
	hv_click.c \
	hv_curs.c \
	hv_disp.c \
	hv_eref.c \
	hv_error.c \
	hv_file.c \
	hv_font.c \
	hv_fsel.c \
	hv_gfx.c \
	hv_globl.c \
	hv_help.c \
	hv_hfind.c \
	hv_hist.c \
	hv_info.c \
	hv_init.c \
	hv_mark.c \
	hv_misc.c \
	hv_nav.c \
	hv_popup.c \
	hv_prefs.c \
	hv_srch.c \
	hv_tbar.c \
	hv_win.c \
	autoloc.c \
	$(empty)

hypview_DEPENDENCIES = resource.res $(LIBHYP_DEP) $(LIBHYPDOC_DEP)
hypview_LDADD = resource.res $(LIBPIC) $(LIBHYPDOC) $(LIBHYP) $(LIBINTL) -lshell32 -lgdi32 -lcomdlg32
hypview_LDFLAGS = -mconsole

resource.res: $(srcdir)/resource.rc $(srcdir)/resource.rh
	$(WINDRES) $(INCLUDES) --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $(srcdir)/resource.rc

EXTRA_DIST = resource.rc resource.rh windows.rh

CLEANFILES = *.res
