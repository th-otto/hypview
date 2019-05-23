AUTOMAKE_OPTIONS = subdir-objects

include ../Makefile.common

SUBDIRS = utils/call-acc

if WITH_GUI_GEM
bin_PROGRAMS = hypview
endif
EXEEXT = .app

LIBGEM = -lgem

AM_CPPFLAGS = -I$(hyp_srcdir)/include -I$(hyp_srcdir)/include/tos

hypview_SOURCES = \
	hypview.c hypview.h \
	lang/en/hypview.rsh lang/en/hypview.h \
	lang/de/hypview.rsh lang/de/hypview.h \
	lang/cs/hypview.rsh lang/cs/hypview.h \
	hv_defs.h \
	\
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
	hv_globl.c \
	hv_hfind.c \
	hv_hist.c \
	hv_info.c \
	hv_init.c \
	hv_mark.c \
	hv_nav.c \
	hv_popup.c \
	hv_selec.c \
	hv_srch.c \
	hv_tbar.c \
	hv_user.c \
	hv_win.c \
	hv_rsc.c \
	autoloc.c \
	\
	dl_apinf.c \
	dl_avcmd.c \
	dl_bubbl.c \
	dl_dhst.c \
	dl_dial.c \
	dl_drag.c \
	dl_event.c \
	dl_filsl.c \
	dl_fonsl.c \
	dl_help.c \
	dl_init.c \
	dl_items.c \
	dl_ledit.c \
	dl_mem.c \
	dl_menu.c \
	dl_msgs.c \
	dl_popup.c \
	dl_routs.c \
	dl_tools.c \
	dl_user.c \
	dl_win.c \
	\
	dirent.c \
	nf_debug.c \
	nf_name.c \
	nf_ops.c \
	nf_shutd.c \
	nf_stder.c \
	nf_vers.c \
	\
	dbgwin.c dbgwin.h \
	gscript.c \
	dragdrop/dragdrop.c \
	bubble/bgh.c

hypview_DEPENDENCIES = $(LIBPIC_DEP) $(LIBHYPDOC_DEP) $(LIBHYP_DEP)
hypview_LDADD = $(LIBPIC) $(LIBHYPDOC) $(LIBHYP) $(LIBPIC) $(LIBGEM) $(INTLLIBS)

CLEANFILES = hypview.app

clean-local:
	$(RM) *.tos *.ttp *.app *.gtp

distclean-local:
	$(RM) *.pdb

EXTRA_DIST = lang utils \
	lang/en/hypview.rsc lang/en/hypview.rso \
	lang/de/hypview.rsc lang/de/hypview.rso \
	lang/cs/hypview.rsc lang/cs/hypview.rso \
	hypview.prj hcp.prj stool.prj refcheck.prj reflink.prj hypfind.prj \
	help_rc.prj all.prj \
	bubble/history.txt bubble/test.bgh bubble/test.c \
	gif2bmp.prj picinfo.prj
