include ../Makefile.common

if WITH_GUI_GEM
noinst_PROGRAMS = hypview
noinst_SCRIPTS = hypview.app
endif

LIBGEM = -lgem

INCLUDES = -I$(hyp_srcdir)/include -I$(hyp_srcdir)/include/tos

hypview_SOURCES = \
	hypview.c hypview.h hypview.rsh \
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
	dragdrop/dragdrop.c \
	bubble/bgh.c

hypview_DEPENDENCIES = $(LIBPIC_DEP) $(LIBHYPDOC_DEP) $(LIBHYP_DEP)
hypview_LDADD = $(LIBPIC) $(LIBHYPDOC) $(LIBHYP) $(LIBGEM) $(LIBINTL)

hypview.app: hypview$(EXEEXT)
	cp -a $? $@

CLEANFILES = hypview.app

install-exec-local: $(noinst_SCRIPTS)
	$(MKINSTALLDIRS) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) hypview.app $(DESTDIR)$(bindir)

uninstall-local:
	$(RM) $(DESTDIR)$(bindir)/hypview.app

clean-local:
	$(RM) *.tos *.ttp *.app *.gtp

distclean-local:
	$(RM) *.pdb

EXTRA_DIST = lang utils \
	hypview.rsc hypview.rso hypview.bgh \
	hypview.prj hcp.prj stool.prj refcheck.prj reflink.prj hypfind.prj \
	help_rc.prj all.prj dragdrop/libdgdp.prj \
	bubble/history.txt bubble/libbgh.prj bubble/test.bgh bubble/test.c \
	gif2bmp.prj picinfo.prj
