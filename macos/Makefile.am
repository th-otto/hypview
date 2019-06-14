AUTOMAKE_OPTIONS = -Wno-portability subdir-objects

include $(top_srcdir)/Makefile.common

if WITH_GUI_COCOA
bin_PROGRAMS = hypview
endif

AM_CPPFLAGS =
AM_CPPFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/rcintl
AM_CPPFLAGS += -I$(top_srcdir)/rsc/include
if WITH_PDF
AM_CPPFLAGS += -I$(top_srcdir)/hpdf/include
endif

hypview_SOURCES = \
	hv_defs.h w_draw.h \
	hv_main.m \
	hv_about.m \
	hv_asc.m \
	hv_autol.m \
	hv_bin.m \
	hv_block.m \
	hv_curs.m \
	hv_disp.m \
	hv_eref.m \
	hv_error.m \
	hv_file.m \
	hv_font.m \
	hv_fsel.m \
	hv_gfx.m \
	hv_help.m \
	hv_hfind.m \
	hv_hist.m \
	hv_info.m \
	hv_init.m \
	hv_mark.m \
	hv_misc.m \
	hv_nav.m \
	hv_popup.m \
	hv_prefs.m \
	hv_rcomp.m \
	hv_rsc.m \
	hv_selec.m \
	hv_srch.m \
	hv_tbar.m \
	hv_win.m \
	autoloc.m \
	hcp_opts.m \
	pixbuf.c pixbuf.h \
	pixdata.c pixdata.h \
	../hyp/outcomm.c \
	../hyp/outstg.c \
	../hyp/outhtml.c \
	../hyp/outxml.c \
	../hyp/outasc.c \
	../hyp/outdump.c \
	\
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
hypview_LDADD += $(INTLLIBS)
if OS_WIN32
hypview_DEPENDENCIES += $(LIBRCINTL_DEP)
endif
if OS_MACOS
hypview_LDFLAGS = -framework Cocoa -lobjc
endif

EXTRA_DIST = \
	Info.plist \
	hypview.xcodeproj/project.pbxproj \
	$(empty)
