include ../Makefile.common

if WITH_GUI_GTK
bin_PROGRAMS = hypview
endif

INCLUDES = -I$(hyp_srcdir)/include

AM_CPPFLAGS =	$(GTK_CFLAGS) \
		-DBINDIR="\"$(bindir)\""
	

hypview_SOURCES = \
	hv_main.c \
	hv_gtk.h \
	gtk_comp.h \
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
	hv_srch.c \
	hv_tbar.c \
	hv_win.c \
	autoloc.c \
	$(empty)

hypview_DEPENDENCIES = $(LIBHYP_DEP) $(LIBHYPDOC_DEP)
hypview_LDADD = $(LIBPIC) $(LIBHYPDOC) $(LIBHYP) $(GTK_LIBS) $(LIBINTL)

icons: $(srcdir)/../icons/hypview.h

GEN_PIXBUF_CSOURCE = gen-pixbuf-csource

$(srcdir)/../icons/hypview.h: $(srcdir)/../icons/hypview.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=hypview_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }
