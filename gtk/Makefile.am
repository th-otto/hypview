AUTOMAKE_OPTIONS = -Wno-portability subdir-objects

include $(top_srcdir)/Makefile.common

if WITH_GUI_GTK
bin_PROGRAMS = hypview
noinst_PROGRAMS = gfilesel
if !CROSS_COMPILING
noinst_SCRIPTS = $(build_triplet)-hypview$(EXEEXT)
endif
endif

AM_CPPFLAGS =	$(GTK_CFLAGS) \
		-DBINDIR="\"$(bindir)\"" \
		-DPKGDATADIR="\"$(pkgdatadir)\""

AM_CPPFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/rcintl
AM_CPPFLAGS += -I$(top_srcdir)/rsc/include
if WITH_PDF
AM_CPPFLAGS += -I$(top_srcdir)/hpdf/include
endif

applicationdir = $(datadir)/applications

application_DATA = hypview.desktop

mimeinfodir = $(datadir)/mime-info

mimeinfo_DATA = hypview.keys hypview.mime

PO_DIR = $(builddir)/../po
PO_SRCDIR = $(srcdir)/../po

EVAL_POFILES = cat $(PO_SRCDIR)/LINGUAS | sed -e '/^\#/d' -e 's@^\(.*\)@	$(PO_DIR)/\1.po @'
POFILES := $(shell $(EVAL_POFILES))

EVAL_MOFILES = cat $(PO_SRCDIR)/LINGUAS | sed -e '/^\#/d' -e 's@^\(.*\)@ ../po/\1@CATOBJEXT@ @'
MOFILES := $(shell $(EVAL_MOFILES))

hypview_SOURCES = \
	hv_main.c \
	hv_gtk.h \
	gtk_comp.h \
	gdkkeysyms.h \
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
	hv_rcomp.c \
	hv_rsc.c \
	hv_srch.c \
	hv_tbar.c \
	hv_win.c \
	hv_tree.c \
	autoloc.c \
	hcp_opts.c \
	../hyp/outcomm.c \
	../hyp/outstg.c \
	../hyp/outhtml.c \
	../hyp/outxml.c \
	../hyp/outasc.c \
	../hyp/outdump.c \
	$(empty)

if WITH_PDF
hypview_SOURCES += ../hyp/pdf.c
endif


hypview_DEPENDENCIES = $(LIBPIC_DEP) $(LIBHYP_DEP) $(LIBHYPDOC_DEP)
hypview_LDADD =
if WITH_PDF
hypview_DEPENDENCIES += $(LIBHPDF_DEP)
hypview_LDADD += $(LIBHPDF)
endif
hypview_LDADD += $(LIBPIC) $(LIBHYPDOC) $(LIBHYP)
hypview_DEPENDENCIES += $(GEMLIB_DEPS)
hypview_LDADD += $(GEMLIBS)
if HAVE_PNG
hypview_DEPENDENCIES += $(LIBPNG_DEP)
hypview_LDADD += $(LIBPNG)
endif
if NEED_ZLIB
hypview_DEPENDENCIES += $(LIBZ_DEP)
hypview_LDADD += $(LIBZ)
endif
hypview_LDADD += $(GTK_LIBS) $(INTLLIBS)
hypview_LDFLAGS = -Wl,-rpath,$(GTK_LIBDIR)
if OS_WIN32
hypview_DEPENDENCIES += $(LIBRCINTL_DEP)
endif

$(build_triplet)-hypview$(EXEEXT): hypview$(EXEEXT)
	cp -a $? $@

gfilesel_SOURCES = gfilesel.c
gfilesel_LDADD = $(GTK_LIBS)
gfilesel_LDFLAGS = -Wl,-rpath,$(GTK_LIBDIR)

icons: $(srcdir)/../icons/hypview.h

GEN_PIXBUF_CSOURCE = gen-pixbuf-csource

$(srcdir)/../icons/hypview.h: $(srcdir)/../icons/hypview.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=hypview_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(PO_DIR)/.intltool-merge-cache: $(srcdir)/hypview.desktop.in $(INTLTOOL_MERGE) $(POFILES)
	$(AM_V_GEN)
	$(AM_V_at)LC_ALL=C $(INTLTOOL_MERGE) -q -d -u -c $(PO_DIR)/.intltool-merge-cache $(PO_DIR) $(srcdir)/hypview.desktop.in hypview.desktop.$$$$ && $(RM) hypview.desktop.$$$$

hypview.desktop: $(srcdir)/hypview.desktop.in $(INTLTOOL_MERGE) $(PO_DIR)/.intltool-merge-cache
	$(AM_V_GEN)
	$(AM_V_at)LC_ALL=C $(INTLTOOL_MERGE) -q -d -u -c $(PO_DIR)/.intltool-merge-cache $(PO_DIR) $< $@.$$$$ && mv $@.$$$$ $@ || $(RM) $@.$$$$

EXTRA_DIST = hypview.desktop hypview.desktop.in $(mimeinfo_DATA)

CLEANFILES = hypview.desktop $(PO_DIR)/.intltool-merge-cache
DISTCLEANFILES = $(build_triplet)-hypview$(EXEEXT)
