AUTOMAKE_OPTIONS = -Wno-portability subdir-objects

include $(top_srcdir)/Makefile.common

if WITH_GUI_WIN32
bin_PROGRAMS = hypview
endif

AM_CPPFLAGS =	-DBINDIR="\"$(bindir)\"" \
		-DPKGDATADIR="\"$(pkgdatadir)\""
AM_CPPFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/rcintl
AM_CPPFLAGS += -I$(top_srcdir)/rsc/include
if WITH_PDF
AM_CPPFLAGS += -I$(top_srcdir)/hpdf/include
endif

	
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
	hv_rsc.c \
	hv_selec.c \
	hv_srch.c \
	hv_tbar.c \
	hv_win.c \
	autoloc.c \
	hv_rcomp.c \
	hcp_opts.c \
	w_draw.c w_draw.h \
	../hyp/outcomm.c \
	../hyp/outstg.c \
	../hyp/outhtml.c \
	../hyp/outxml.c \
	../hyp/outasc.c \
	../hyp/outdump.c \
	\
	windebug.c windebug.h \
	$(empty)

if WITH_PDF
hypview_SOURCES += ../hyp/pdf.c
endif

hypview_DEPENDENCIES = resource.res $(LIBPIC_DEP) $(LIBHYP_DEP) $(LIBHYPDOC_DEP) $(LIBRCINTL_DEP)
hypview_LDADD = resource.res
if WITH_PDF
hypview_DEPENDENCIES += $(LIBHPDF_DEP)
hypview_LDADD += $(LIBHPDF)
endif
hypview_LDADD += $(LIBPIC) $(LIBHYPDOC) $(LIBHYP)
hypview_DEPENDENCIES += $(GEMLIB_DEPS)
hypview_LDADD += $(GEMLIBS) $(GEMLIBS)
if HAVE_PNG
hypview_DEPENDENCIES += $(LIBPNG_DEP)
hypview_LDADD += $(LIBPNG)
endif
if NEED_ZLIB
hypview_DEPENDENCIES += $(LIBZ_DEP)
hypview_LDADD += $(LIBZ)
endif
hypview_LDADD += $(INTLLIBS) -lshell32 -lgdi32 -lcomdlg32
hypview_LDFLAGS = -mwindows

EVAL_MOFILES = cat $(srcdir)/../po/LINGUAS | sed -e '/^\#/d' -e 's@^\(.*\)@ ../po/\1@CATOBJEXT@ @'
MOFILES := $(shell $(EVAL_MOFILES))

resource_files = \
	$(srcdir)/resource.rc \
	$(srcdir)/resource.rh \
	$(srcdir)/resource_en.rc \
	$(srcdir)/resource_de.rc \
	$(srcdir)/hypview.exe.manifest \
	$(srcdir)/res/mail.bmp \
	$(srcdir)/res/weblink.bmp \
	$(empty)

resource_icons = \
	$(srcdir)/../icons/hypview.ico \
	$(srcdir)/../icons/back.ico \
	$(srcdir)/../icons/help.ico \
	$(srcdir)/../icons/home.ico \
	$(srcdir)/../icons/index.ico \
	$(srcdir)/../icons/info.ico \
	$(srcdir)/../icons/catalog.ico \
	$(srcdir)/../icons/load.ico \
	$(srcdir)/../icons/bookmark.ico \
	$(srcdir)/../icons/menu.ico \
	$(srcdir)/../icons/history.ico \
	$(srcdir)/../icons/next.ico \
	$(srcdir)/../icons/nextphys.ico \
	$(srcdir)/../icons/previous.ico \
	$(srcdir)/../icons/prevphys.ico \
	$(srcdir)/../icons/first.ico \
	$(srcdir)/../icons/last.ico \
	$(srcdir)/../icons/referenc.ico \
	$(srcdir)/../icons/save.ico \
	$(srcdir)/../icons/remarker.ico \
	$(empty)
	
resource.res: $(resource_files) $(resource_icons) $(srcdir)/../po/linguas.rc $(MOFILES)
	$(AM_V_RC)$(WINDRES) $(AM_CPPFLAGS) -I../po --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $(srcdir)/resource.rc

EXTRA_DIST = $(resource_files)

CLEANFILES = *.res
