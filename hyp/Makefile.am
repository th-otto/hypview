#
# Makefile for hypview (hyp)
#
AUTOMAKE_OPTIONS = subdir-objects

include ../Makefile.common

noinst_LIBRARIES = libhyp.a libhypdoc.a
bin_PROGRAMS = reflink refcheck hcp stool hypfind

if OS_TOS
bin_SCRIPTS = reflink.ttp refcheck.ttp hcp.ttp stool.tos hypfind.ttp
endif
if !CROSS_COMPILING
noinst_SCRIPTS = $(build_triplet)-hcp$(EXEEXT) \
		 $(build_triplet)-hypfind$(EXEEXT) \
		 $(build_triplet)-stool$(EXEEXT)
endif

EXTRA_DIST = gfx.s genmaptab.c


noinst_PROGRAMS = img2bmp bmp2img icn2img picinfo bmp2gif gif2bmp icn2ico

INCLUDES = -I$(hyp_srcdir)/include
AM_CPPFLAGS = $(GLIB_CFLAGS) \
	-DBINDIR="\"$(bindir)\""

libhyp_a_SOURCES = \
	casefold.h \
	cp_1252.h \
	cp_850.h \
	cp_atari.h \
	cp_mac.h \
	lh5int.h \
	sorttab.h \
	gunichar.h \
	\
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
	utl_prof.c \
	utl_dbg.c \
	xgetopt.c \
	str2imax.c \
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
	hyp_bin.c \
	localenm.c


UTIL_SRC = \
	utl_err.c \
	hyp_err.c \
	utl_ngui.c


reflink_SOURCES = \
	reflink.c \
	refcmds.c \
	$(UTIL_SRC)
reflink_DEPENDENCIES = libhyp.a
reflink_LDADD = $(LIBHYP) $(GLIB_LIBS) $(LIBINTL)

reflink.ttp: reflink
	$(LN_S) $?$(EXEEXT) $@

refcheck_SOURCES = \
	refcheck.c \
	refcmds.c \
	$(UTIL_SRC)

refcheck_DEPENDENCIES = libhyp.a
refcheck_LDADD = $(LIBHYP) $(GLIB_LIBS) $(LIBINTL)

refcheck.ttp: refcheck
	$(LN_S) $?$(EXEEXT) $@


hcp_SOURCES = \
	hcpmain.c \
	hcp_opts.c \
	hcp_comp.c \
	kwset.c \
	hobstack.c \
	refcmds.c \
	$(UTIL_SRC)

hcp_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
hcp_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(LIBINTL)

hcp.ttp: hcp
	$(LN_S) $?$(EXEEXT) $@

$(build_triplet)-hcp$(EXEEXT): hcp$(EXEEXT)
	cp -a $? $@

stool_SOURCES = \
	stool.c \
	$(UTIL_SRC)

stool_DEPENDENCIES = libhyp.a
stool_LDADD = $(LIBHYP) $(GLIB_LIBS) $(LIBINTL)

stool.tos: stool
	$(LN_S) $?$(EXEEXT) $@

$(build_triplet)-stool$(EXEEXT): stool$(EXEEXT)
	cp -a $? $@


hypfind_SOURCES = \
	hypfind.c \
	hcp_opts.c \
	$(UTIL_SRC)
hypfind_DEPENDENCIES = libhyp.a
hypfind_LDADD = $(LIBHYP) $(GLIB_LIBS) $(LIBINTL)

hypfind.ttp: hypfind
	$(LN_S) $?$(EXEEXT) $@

$(build_triplet)-hypfind$(EXEEXT): hypfind$(EXEEXT)
	cp -a $? $@


img2bmp_SOURCES = \
	img2bmp.c \
	$(UTIL_SRC)

img2bmp_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
img2bmp_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(LIBINTL)


bmp2img_SOURCES = \
	bmp2img.c \
	$(UTIL_SRC)

bmp2img_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
bmp2img_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(LIBINTL)


bmp2gif_SOURCES = \
	bmp2gif.c \
	$(UTIL_SRC)

bmp2gif_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
bmp2gif_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(LIBINTL)


gif2bmp_SOURCES = \
	gif2bmp.c \
	$(UTIL_SRC)

gif2bmp_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
gif2bmp_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(LIBINTL)


icn2img_SOURCES = \
	icn2img.c \
	$(UTIL_SRC)

icn2img_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
icn2img_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(LIBINTL)


icn2ico_SOURCES = \
	icn2ico.c \
	$(UTIL_SRC)

icn2ico_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
icn2ico_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(LIBINTL)


picinfo_SOURCES = \
	picinfo.c \
	$(UTIL_SRC)

picinfo_DEPENDENCIES = libhyp.a $(LIBPIC_DEP)
picinfo_LDADD = $(LIBHYP) $(LIBPIC) $(GLIB_LIBS) $(LIBINTL)


CLEANFILES = reflink.ttp refcheck.ttp hcp.ttp stool.tos hypfind.ttp
DISTCLEANFILES = $(build_triplet)-hcp$(EXEEXT)
