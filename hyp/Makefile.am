#
# Makefile for hypview (hyp)
#
AUTOMAKE_OPTIONS = subdir-objects -Wno-portability

include ../Makefile.common

noinst_LIBRARIES = libhyp.a libhypdoc.a
bin_PROGRAMS = reflink refcheck hcp stool hypfind

noinst_PROGRAMS = 
noinst_SCRIPTS = 

if HAVE_CURL
noinst_PROGRAMS += hypview.cgi
endif
SRVDIR = /srv/www/htdocs/hypview


if OS_TOS
bin_SCRIPTS = reflink.ttp refcheck.ttp hcp.ttp stool.tos hypfind.ttp
endif
if !CROSS_COMPILING
noinst_SCRIPTS += $(build_triplet)-hcp$(EXEEXT) \
		  $(build_triplet)-hypfind$(EXEEXT) \
		  $(build_triplet)-stool$(EXEEXT)
endif

EXTRA_DIST = gfx.s genmaptab.c

noinst_PROGRAMS += img2bmp bmp2img icn2img picinfo bmp2gif gif2bmp icn2ico dumpicon

INCLUDES = -I$(hyp_srcdir)/include
AM_CPPFLAGS = $(GLIB_CFLAGS) $(CURL_CFLAGS) \
	-DBINDIR="\"$(bindir)\""

libhyp_a_SOURCES = \
	casefold.h \
	cp_1252.h \
	cp_1250.h \
	cp_latin1.h \
	cp_850.h \
	cp_atari.h \
	cp_mac.h \
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
	tool.c \
	hyp_file.c \
	hyp_err.c \
	hyp_cset.c \
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
	$(empty)

EXTRA_DIST += tos/cookie.c
if OS_TOS
#libhyp_a_SOURCES += tos/cookie.c
libhyp_a_SOURCES += tos/mkdir.c
libhyp_a_SOURCES += tos/spawn.c
libhyp_a_SOURCES += tos/malloc.c
INCLUDES +=  -I$(hyp_srcdir)/include/tos
endif

if OS_UNIX
libhyp_a_SOURCES += unix/spawn.c unix/malloc.c
endif

if OS_WIN32
libhyp_a_SOURCES += win32.c win32/spawn.c unix/malloc.c
endif


libhypdoc_a_SOURCES = \
	hypdoc.c hypdoc.h \
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
	outasc.h outstg.h outdump.h outhtml.h htmljs.h \
	$(UTIL_SRC)

EVAL_MOFILES = cat $(srcdir)/../po/LINGUAS | sed -e '/^\#/d' -e 's@^\(.*\)@ ../po/\1@CATOBJEXT@ @'
MOFILES := $(shell $(EVAL_MOFILES))

hcp_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
hcp_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)
if OS_WIN32
hcp_SOURCES += hcp_rc.rc
hcp_DEPENDENCIES += hcp_rc.res $(LIBRCINTL_DEP)
hcp_LDADD += hcp_rc.res
hcp_rc.res: $(srcdir)/hcp_rc.rc $(srcdir)/../po/linguas.rc $(MOFILES)
	$(AM_V_RC)$(WINDRES) $(INCLUDES) --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $<
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
	outasc.h outstg.h outdump.h outhtml.h htmljs.h \
	$(UTIL_SRC)

hypview_cgi_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
hypview_cgi_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(CURL_LIBS) $(INTLLIBS)
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
	@rm -f hypview.cgi$(EXEEXT)
	$(AM_V_CCLD)$(hypview_cgi_LINK) $(hypview_cgi_OBJECTS) $(hypview_cgi_LDADD) $(LIBS); \
	if test -d "$(SRVDIR)"; then \
	   echo "installing $@ in $(SRVDIR)"; \
	   cp -a $@ "$(SRVDIR)/hypview.cgi"; \
	fi
	
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
	$(AM_V_RC)$(WINDRES) $(INCLUDES) --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $<
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
refcheck_SOURCES += refcheck.rc
refcheck_DEPENDENCIES += refcheck_rc.res $(LIBRCINTL_DEP)
refcheck_LDADD += refcheck_rc.res
refcheck_rc.res: $(srcdir)/refcheck_rc.rc $(srcdir)/../po/linguas.rc $(MOFILES)
	$(AM_V_RC)$(WINDRES) $(INCLUDES) --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $<
endif

refcheck.ttp: refcheck
	$(LN_S) $?$(EXEEXT) $@


stool_SOURCES = \
	stool.c \
	$(UTIL_SRC)

stool_DEPENDENCIES = libhyp.a
stool_LDADD = $(LIBHYP) $(GLIB_LIBS) $(INTLLIBS)
if OS_WIN32
stool_SOURCES += stool.rc
stool_DEPENDENCIES += stool_rc.res $(LIBRCINTL_DEP)
stool_LDADD += stool_rc.res
stool_rc.res: $(srcdir)/stool_rc.rc $(srcdir)/../po/linguas.rc $(MOFILES)
	$(AM_V_RC)$(WINDRES) $(INCLUDES) --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $<
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
	$(UTIL_SRC)
hypfind_DEPENDENCIES = libhyp.a
hypfind_LDADD = $(LIBHYP) $(GLIB_LIBS) $(INTLLIBS)
if OS_WIN32
hypfind_SOURCES += hypfind.rc
hypfind_DEPENDENCIES += hypfind_rc.res $(LIBRCINTL_DEP)
hypfind_LDADD += hypfind_rc.res
hypfind_rc.res: $(srcdir)/hypfind_rc.rc $(srcdir)/../po/linguas.rc $(MOFILES)
	$(AM_V_RC)$(WINDRES) $(INCLUDES) --target=$(RC_TARGET) --input-format rc --output-format coff -o $@ $<
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


bmp2img_SOURCES = \
	bmp2img.c \
	$(UTIL_SRC)

bmp2img_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
bmp2img_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)


bmp2gif_SOURCES = \
	bmp2gif.c \
	$(UTIL_SRC)

bmp2gif_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
bmp2gif_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)


gif2bmp_SOURCES = \
	gif2bmp.c \
	$(UTIL_SRC)

gif2bmp_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
gif2bmp_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)


icn2img_SOURCES = \
	icn2img.c \
	$(UTIL_SRC)

icn2img_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
icn2img_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)


icn2ico_SOURCES = \
	icn2ico.c \
	$(UTIL_SRC)

icn2ico_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
icn2ico_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)


picinfo_SOURCES = \
	picinfo.c \
	$(UTIL_SRC)

picinfo_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
picinfo_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(INTLLIBS)


dumpicon_SOURCES = \
	dumpicon.c

CLEANFILES = *.ttp *.tos *.res
DISTCLEANFILES = $(build_triplet)-hcp$(EXEEXT)
