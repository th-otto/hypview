#
# Makefile for hypview (hyp)
#
AUTOMAKE_OPTIONS = subdir-objects -Wno-portability

include $(top_srcdir)/Makefile.common

noinst_LIBRARIES = libhyp.a libhypdoc.a
bin_PROGRAMS = reflink refcheck hcp stool hypfind hypinfo

noinst_PROGRAMS = pdfid
noinst_SCRIPTS = 

if HAVE_CURL
# currently has problems linking the gem libraries; FIXME
if OS_UNIX
noinst_PROGRAMS += hypview.cgi
endif
endif
SRVDIR = /srv/www/htdocs/hypview


if !CROSS_COMPILING
noinst_SCRIPTS += $(build_triplet)-hcp$(EXEEXT) \
		  $(build_triplet)-hypfind$(EXEEXT) \
		  $(build_triplet)-stool$(EXEEXT)
endif

EXTRA_DIST = gfx.s genmaptab.c

noinst_PROGRAMS += img2bmp bmp2img icn2img picinfo bmp2gif gif2bmp icn2ico ico2icn dumpicon

AM_CPPFLAGS = $(GLIB_CFLAGS) $(CURL_CFLAGS) \
	-DBINDIR="\"$(bindir)\""
AM_CPPFLAGS += -I$(hyp_srcdir)/include -I$(top_srcdir)
if WITH_PDF
AM_CPPFLAGS += -I$(srcdir)/../hpdf/include
endif
if HAVE_PNG
AM_CPPFLAGS += $(PNG_CFLAGS)
endif
if NEED_ZLIB
AM_CPPFLAGS += $(Z_CFLAGS)
endif

LANGID_MODEL = $(srcdir)/ldpy.model

libhyp_a_SOURCES = \
	casefold.h \
	cp_1250.h \
	cp_1251.h \
	cp_1252.h \
	cp_1253.h \
	cp_1254.h \
	cp_1255.h \
	cp_1256.h \
	cp_1257.h \
	cp_1258.h \
	cp_st_ru.h \
	cp_latin1.h \
	cp_850.h \
	cp_atari.h \
	cp_mac.h \
	cp_binary.h \
	lh5int.h \
	sorttab.h \
	gunichar.h \
	\
	base64.c \
	cache.c \
	ext_refs.c \
	hyp.c \
	lh5d.c \
	lh5c.c \
	lh5crc.c \
	hyp_load.c \
	prepare.c \
	ref.c \
	hyp_save.c \
	hyp_tree.c \
	tool.c \
	hyp_file.c \
	hyp_err.c \
	hyp_cset.c \
	csetname.c \
	hyp_intl.c \
	hyp_glib.c \
	hypgfx.c \
	rpl_stat.c \
	profile.c \
	hyp_cfg.c \
	utl_dbg.c \
	xgetopt.c \
	str2imax.c \
	localenm.c \
	win32.c \
	$(empty)

EXTRA_DIST += tos/cookie.c
if OS_TOS
#libhyp_a_SOURCES += tos/cookie.c
libhyp_a_SOURCES += tos/mkdir.c
libhyp_a_SOURCES += tos/spawn.c
libhyp_a_SOURCES += tos/malloc.c
libhyp_a_SOURCES += tos/hypmain.c
AM_CPPFLAGS +=  -I$(hyp_srcdir)/include/tos
endif

if OS_UNIX
libhyp_a_SOURCES += unix/spawn.c unix/malloc.c
endif

if OS_MACOS
libhyp_a_SOURCES += unix/spawn.c unix/malloc.c
endif

if OS_WIN32
libhyp_a_SOURCES += win32/spawn.c win32/hypmain.c unix/malloc.c
endif


libhypdoc_a_SOURCES = \
	hypdoc.c \
	block.c \
	hyp_asc.c \
	hyp_bin.c


UTIL_SRC = \
	utl_err.c \
	hyp_err.c \
	utl_ngui.c


hcp_SOURCES = \
	hcpmain.c \
	hcp_opts.c \
	hcp_comp.c \
	kwset.c \
	hobstack.c \
	refcmds.c \
	outcomm.c outcomm.h \
	outstg.c outstg.h \
	outhtml.c outhtml.h htmljs.h \
	outxml.c outxml.h \
	outasc.c outasc.h \
	outdump.c outdump.h \
	pdf.c pdf.h \
	$(UTIL_SRC)

htmljs.h: $(top_srcdir)/www/_hypview.js
	$(AM_V_GEN)( \
	  echo 'static char const html_javascript_code[] ='; \
	  sed -e 's@/\*.*\*/@@' -e '/^[ ]*$$/d' -e 's/"/\\"/g' -e 's/^/	"/' -e 's/$$/\\n"/' $<; \
	  echo ';' \
	) > $@

iback_png.h: $(top_srcdir)/www/images/iback.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const nav_back_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

iprev_png.h: $(top_srcdir)/www/images/iprev.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const nav_prev_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

itoc_png.h: $(top_srcdir)/www/images/itoc.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const nav_toc_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

inext_png.h: $(top_srcdir)/www/images/inext.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const nav_next_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

ixref_png.h: $(top_srcdir)/www/images/ixref.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const nav_xref_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

iload_png.h: $(top_srcdir)/www/images/iload.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const nav_load_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

iindex_png.h: $(top_srcdir)/www/images/iindex.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const nav_index_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

ihelp_png.h: $(top_srcdir)/www/images/ihelp.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const nav_help_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

iinfo_png.h: $(top_srcdir)/www/images/iinfo.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const nav_info_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

itreeview_png.h: $(top_srcdir)/www/images/itreeview.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const nav_treeview_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

tv_blank_png.h: $(top_srcdir)/www/images/tv_blank.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const tv_blank_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

tv_intersec_png.h: $(top_srcdir)/www/images/tv_intersec.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const tv_intersec_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

tv_collapsed_png.h: $(top_srcdir)/www/images/tv_collapsed.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const tv_collapsed_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

tv_collapsed_end_png.h: $(top_srcdir)/www/images/tv_collapsed_end.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const tv_collapsed_end_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

tv_expanded_png.h: $(top_srcdir)/www/images/tv_expanded.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const tv_expanded_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

tv_expanded_end_png.h: $(top_srcdir)/www/images/tv_expanded_end.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const tv_expanded_end_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

tv_nointersec_png.h: $(top_srcdir)/www/images/tv_nointersec.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const tv_nointersec_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

tv_end_png.h: $(top_srcdir)/www/images/tv_end.png
	$(AM_V_GEN)( \
	  echo 'static unsigned char const tv_end_data[] = {'; \
	  xxd -i < $<; \
	  echo '};' \
	) > $@

NAV_IMAGES = \
	iback_png.h \
	iprev_png.h \
	itoc_png.h \
	inext_png.h \
	ixref_png.h \
	iload_png.h \
	iindex_png.h \
	ihelp_png.h \
	iinfo_png.h \
	itreeview_png.h \
	tv_blank_png.h \
	tv_intersec_png.h \
	tv_collapsed_png.h \
	tv_collapsed_end_png.h \
	tv_expanded_png.h \
	tv_expanded_end_png.h \
	tv_nointersec_png.h \
	tv_end_png.h \
	$(empty)

icons: $(NAV_IMAGES)

outhtml.o: $(NAV_IMAGES)

EVAL_MOFILES = cat $(srcdir)/../po/LINGUAS | sed -e '/^\#/d' -e 's@^\(.*\)@ ../po/\1@CATOBJEXT@ @'
MOFILES := $(shell $(EVAL_MOFILES))

hcp_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
hcp_LDADD =
hcp_LDADD += $(LIBHYP) $(LIBPIC)
if WITH_PDF
hcp_DEPENDENCIES += $(LIBHPDF_DEP)
hcp_LDADD += $(LIBHPDF)
endif
if HAVE_PNG
hcp_DEPENDENCIES += $(LIBPNG_DEP)
hcp_LDADD += $(LIBPNG)
endif
if NEED_ZLIB
hcp_DEPENDENCIES += $(LIBZ_DEP)
hcp_LDADD += $(LIBZ)
endif
hcp_LDADD += $(GLIB_LIBS) $(INTLLIBS)
if OS_WIN32
hcp_SOURCES += hcp_rc.rc
hcp_DEPENDENCIES += hcp_rc.res $(LIBRCINTL_DEP)
hcp_LDADD += hcp_rc.res
hcp_rc.res: $(srcdir)/hcp_rc.rc $(srcdir)/../po/linguas.rc $(MOFILES)
	$(AM_V_RC)$(WINDRES) $(AM_CPPFLAGS) --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $<
endif
if WITH_GUI_GTK
hcp_LDFLAGS = -Wl,-rpath,$(GTK_LIBDIR)
endif

hcp.ttp: hcp
	$(LN_S) $?$(EXEEXT) $@

$(build_triplet)-hcp$(EXEEXT): hcp$(EXEEXT)
	cp -a $? $@


hypview_cgi_SOURCES = \
	cgi.c \
	hcp_opts.c \
	hcp_comp.c \
	kwset.c \
	hobstack.c \
	refcmds.c \
	cgirsc.c cgirsc.h \
	outcomm.c outcomm.h \
	outstg.c outstg.h \
	outhtml.c outhtml.h htmljs.h \
	outxml.c outxml.h \
	pdf.c pdf.h \
	bm.c bm.h \
	cgic.c cgic.h \
	$(UTIL_SRC)

hypview_cgi_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
hypview_cgi_LDADD =
if WITH_PDF
hypview_cgi_DEPENDENCIES += $(LIBHPDF_DEP)
hypview_cgi_LDADD += $(LIBHPDF)
endif
hypview_cgi_LDADD += $(LIBHYP) $(LIBPIC)
hypview_cgi_DEPENDENCIES += $(GEMLIB_DEPS)
hypview_cgi_LDADD += $(GEMLIBS)
if HAVE_PNG
hypview_cgi_DEPENDENCIES += $(LIBPNG_DEP)
hypview_cgi_LDADD += $(LIBPNG)
endif
if NEED_ZLIB
hypview_cgi_DEPENDENCIES += $(LIBZ_DEP)
hypview_cgi_LDADD += $(LIBZ)
endif
hypview_cgi_LDADD += $(GLIB_LIBS) $(CURL_LIBS) $(INTLLIBS)
if OS_WIN32
hypview_cgi_SOURCES += hcp_rc.rc
hypview_cgi_DEPENDENCIES += hcp_rc.res $(LIBRCINTL_DEP)
hypview_cgi_LDADD += hcp_rc.res
endif
hypview_cgi_LDFLAGS = -Wl,-rpath,$(CURL_LIBDIR)
if WITH_GUI_GTK
hypview_cgi_LDFLAGS += -Wl,-rpath,$(GTK_LIBDIR)
endif
hypview.cgi$(EXEEXT): $(hypview_cgi_OBJECTS) $(hypview_cgi_DEPENDENCIES) $(EXTRA_hypview_cgi_DEPENDENCIES) 
	@rm -f $@
	$(AM_V_CCLD)$(hypview_cgi_LINK) $(hypview_cgi_OBJECTS) $(hypview_cgi_LDADD) $(LIBS)
if !CROSS_COMPILING
	$(AM_V_at)if test -d "$(SRVDIR)"; then \
	   echo "installing $@ in $(SRVDIR)"; \
	   cp $@ "$(SRVDIR)/$@"; \
	fi
endif

hypinfo_SOURCES = \
	hypinfo.c \
	hcp_opts.c \
	refcmds.c \
	llangid.c llangid.h \
	sparse.c sparse.h \
	outcomm.c outcomm.h \
	outasc.c outasc.h \
	model.c model.h \
	$(UTIL_SRC)
hypinfo_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
hypinfo_LDADD = $(LIBHYP) $(LIBPIC)
hypinfo_LDADD += $(GLIB_LIBS) $(INTLLIBS)
if HAVE_PNG
hypinfo_DEPENDENCIES += $(LIBPNG_DEP)
hypinfo_LDADD += $(LIBPNG)
endif
if NEED_ZLIB
hypinfo_DEPENDENCIES += $(LIBZ_DEP)
hypinfo_LDADD += $(LIBZ)
endif
if WITH_GUI_GTK
hypinfo_LDFLAGS = -Wl,-rpath,$(GTK_LIBDIR)
endif
hypinfo$(EXEEXT): $(hypinfo_OBJECTS) $(hypinfo_DEPENDENCIES) $(EXTRA_hypinfo_DEPENDENCIES) 
	@rm -f hypinfo$(EXEEXT)
	$(AM_V_CCLD)$(hypinfo_LINK) $(hypinfo_OBJECTS) $(hypinfo_LDADD) $(LIBS)
if !CROSS_COMPILING
	$(AM_V_at)if test -d "$(SRVDIR)"; then \
	   echo "installing $@ in $(SRVDIR)"; \
	   cp $@ "$(SRVDIR)/hypinfo"; \
	fi
endif
hypinfo.ttp: hypinfo
	$(LN_S) $?$(EXEEXT) $@



reflink_SOURCES = \
	reflink.c \
	refcmds.c \
	$(UTIL_SRC)
reflink_DEPENDENCIES = libhyp.a
reflink_LDADD = $(LIBHYP) $(GLIB_LIBS) $(INTLLIBS)
if OS_WIN32
reflink_SOURCES += reflink_rc.rc
reflink_DEPENDENCIES += reflink_rc.res $(LIBRCINTL_DEP)
reflink_LDADD += reflink_rc.res
reflink_rc.res: $(srcdir)/reflink_rc.rc $(srcdir)/../po/linguas.rc $(MOFILES)
	$(AM_V_RC)$(WINDRES) $(AM_CPPFLAGS) --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $<
endif

reflink.ttp: reflink
	$(LN_S) $?$(EXEEXT) $@


refcheck_SOURCES = \
	refcheck.c \
	refcmds.c \
	$(UTIL_SRC)

refcheck_DEPENDENCIES = libhyp.a
refcheck_LDADD = $(LIBHYP) $(GLIB_LIBS) $(INTLLIBS)
if OS_WIN32
refcheck_SOURCES += refcheck_rc.rc
refcheck_DEPENDENCIES += refcheck_rc.res $(LIBRCINTL_DEP)
refcheck_LDADD += refcheck_rc.res
refcheck_rc.res: $(srcdir)/refcheck_rc.rc $(srcdir)/../po/linguas.rc $(MOFILES)
	$(AM_V_RC)$(WINDRES) $(AM_CPPFLAGS) --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $<
endif

refcheck.ttp: refcheck
	$(LN_S) $?$(EXEEXT) $@


stool_SOURCES = \
	stool.c \
	$(UTIL_SRC)

stool_DEPENDENCIES = libhyp.a
stool_LDADD = $(LIBHYP) $(GLIB_LIBS) $(INTLLIBS)
if OS_WIN32
stool_SOURCES += stool_rc.rc
stool_DEPENDENCIES += stool_rc.res $(LIBRCINTL_DEP)
stool_LDADD += stool_rc.res
stool_rc.res: $(srcdir)/stool_rc.rc $(srcdir)/../po/linguas.rc $(MOFILES)
	$(AM_V_RC)$(WINDRES) $(AM_CPPFLAGS) --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $<
endif
if WITH_GUI_GTK
stool_LDFLAGS = -Wl,-rpath,$(GTK_LIBDIR)
endif

stool.tos: stool
	$(LN_S) $?$(EXEEXT) $@

$(build_triplet)-stool$(EXEEXT): stool$(EXEEXT)
	cp -a $? $@


hypfind_SOURCES = \
	hypfind.c \
	hcp_opts.c \
	bm.c bm.h \
	$(UTIL_SRC)
hypfind_DEPENDENCIES = libhyp.a
hypfind_LDADD = $(LIBHYP) $(GLIB_LIBS) $(INTLLIBS)
if OS_WIN32
hypfind_SOURCES += hypfind_rc.rc
hypfind_DEPENDENCIES += hypfind_rc.res $(LIBRCINTL_DEP)
hypfind_LDADD += hypfind_rc.res
hypfind_rc.res: $(srcdir)/hypfind_rc.rc $(srcdir)/../po/linguas.rc $(MOFILES)
	$(AM_V_RC)$(WINDRES) $(AM_CPPFLAGS) --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $<
endif
if WITH_GUI_GTK
hypfind_LDFLAGS = -Wl,-rpath,$(GTK_LIBDIR)
endif

hypfind.ttp: hypfind
	$(LN_S) $?$(EXEEXT) $@

$(build_triplet)-hypfind$(EXEEXT): hypfind$(EXEEXT)
	cp -a $? $@


img2bmp_SOURCES = \
	img2bmp.c \
	$(UTIL_SRC)
img2bmp_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
img2bmp_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)
if HAVE_PNG
img2bmp_DEPENDENCIES += $(LIBPNG_DEP) $(LIBZ_DEP)
img2bmp_LDADD += $(LIBPNG) $(LIBZ)
endif


bmp2img_SOURCES = \
	bmp2img.c \
	$(UTIL_SRC)
bmp2img_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
bmp2img_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)
if HAVE_PNG
bmp2img_DEPENDENCIES += $(LIBPNG_DEP) $(LIBZ_DEP)
bmp2img_LDADD += $(LIBPNG) $(LIBZ)
endif


bmp2gif_SOURCES = \
	bmp2gif.c \
	$(UTIL_SRC)
bmp2gif_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
bmp2gif_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)
if HAVE_PNG
bmp2gif_DEPENDENCIES += $(LIBPNG_DEP) $(LIBZ_DEP)
bmp2gif_LDADD += $(LIBPNG) $(LIBZ)
endif


gif2bmp_SOURCES = \
	gif2bmp.c \
	$(UTIL_SRC)
gif2bmp_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
gif2bmp_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)
if HAVE_PNG
gif2bmp_DEPENDENCIES += $(LIBPNG_DEP) $(LIBZ_DEP)
gif2bmp_LDADD += $(LIBPNG) $(LIBZ)
endif


icn2img_SOURCES = \
	icn2img.c \
	$(UTIL_SRC)
icn2img_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
icn2img_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)
if HAVE_PNG
icn2img_DEPENDENCIES += $(LIBPNG_DEP) $(LIBZ_DEP)
icn2img_LDADD += $(LIBPNG) $(LIBZ)
endif


icn2ico_SOURCES = \
	icn2ico.c \
	$(UTIL_SRC)
icn2ico_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
icn2ico_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)
if HAVE_PNG
icn2ico_DEPENDENCIES += $(LIBPNG_DEP) $(LIBZ_DEP)
icn2ico_LDADD += $(LIBPNG) $(LIBZ)
endif


ico2icn_SOURCES = \
	ico2icn.c \
	$(UTIL_SRC)
ico2icn_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
ico2icn_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)
if HAVE_PNG
ico2icn_DEPENDENCIES += $(LIBPNG_DEP) $(LIBZ_DEP)
ico2icn_LDADD += $(LIBPNG) $(LIBZ)
endif



picinfo_SOURCES = \
	picinfo.c \
	$(UTIL_SRC)

picinfo_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
picinfo_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)
if HAVE_PNG
picinfo_DEPENDENCIES += $(LIBPNG_DEP) $(LIBZ_DEP)
picinfo_LDADD += $(LIBPNG) $(LIBZ)
endif
if WITH_GUI_GTK
picinfo_LDFLAGS = -Wl,-rpath,$(GTK_LIBDIR)
endif


dumpicon_SOURCES = \
	dumpicon.c


pdfid_SOURCES = pdfid.c


liblangid.o sparseset.o: liblangid.h model.h sparseset.h
model.o: model.h

model.h: $(LANGID_MODEL) $(srcdir)/ldpy2ldc.py
	python $(srcdir)/ldpy2ldc.py --header $< -o $@

EXTRA_DIST += ldpy2ldc.py $(LANGID_MODEL)

if OS_TOS
install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	mv reflink reflink.ttp && \
	mv refcheck refcheck.ttp && \
	mv hcp hcp.ttp && \
	mv stool stool.tos && \
	mv hypfind hypfind.ttp && \
	mv hypinfo hypinfo.ttp
EXTTTP = .ttp
EXTTOS = .tos
endif

CLEANFILES = *.ttp *.tos *.res $(NAV_IMAGES)
DISTCLEANFILES = $(build_triplet)-hcp$(EXEEXT) $(build_triplet)-stool$(EXEEXT) $(build_triplet)-hypfind$(EXEEXT) $(build_triplet)-hypinfo$(EXEEXT)
