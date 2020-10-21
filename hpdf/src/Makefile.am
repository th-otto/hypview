include $(top_srcdir)/silent.am
include $(top_srcdir)/Makefile.common

noinst_LIBRARIES = libhpdf.a

libhpdf_a_SOURCES = \
	annotat.c \
	array.c \
	binary.c \
	boolean.c \
	destinat.c \
	dict.c \
	doc.c \
	doc_png.c \
	encoder.c \
	enc_cns.c \
	enc_cnt.c \
	enc_jp.c \
	enc_kr.c \
	encrypt.c \
	encryptd.c \
	error.c \
	extgstat.c \
	font.c \
	font_cid.c \
	fontdb14.c \
	fontdef.c \
	fontdcid.c \
	fontdcns.c \
	fontdcnt.c \
	fontdjp.c \
	fontdkr.c \
	fontdtt.c \
	fontdt1.c \
	font_tt.c \
	font_t1.c \
	gstate.c \
	image.c \
	imagecci.c \
	imagepng.c \
	info.c \
	list.c \
	mmgr.c \
	name.c \
	namedict.c \
	null.c \
	number.c \
	objects.c \
	outline.c \
	pagelab.c \
	pageop.c \
	pages.c \
	real.c \
	streams.c \
	string.c \
	u3d.c \
	utils.c \
	xref.c \
	pdfa.c \
	exdata.c \
	enc_utf8.c \
	t4.h \
	$(empty)

libhpdf_a_LIBADD = \
	catalog_new.o \
	catalog_getroot.o \
	catalog_getnames.o \
	catalog_setnames.o \
	catalog_pagelayout.o \
	catalog_pagemode.o \
	catalog_setopenaction.o \
	catalog_validate.o \
	catalog_addpagelabel.o \
	catalog_setviewerpreferences.o \
	catalog_getviewerpreferences.o \
	3dmeasur_addpoint.o \
	3dmeasur_new.o \
	3dmeasur_setcolor.o \
	3dmeasur_settextsize.o \
	3dmeasur_setname.o \
	3dmeasur_settextboxsize.o \
	3dmeasur_settext.o \
	3dmeasur_setprojectionannotation.o \
	3dmeasur_pd33dmeasure_new.o \
	$(empty)

catalog_new.o: catalog.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DCATALOG_NEW

catalog_getroot.o: catalog.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DCATALOG_GETROOT

catalog_getnames.o: catalog.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DCATALOG_GETNAMES

catalog_setnames.o: catalog.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DCATALOG_SETNAMES

catalog_pagelayout.o: catalog.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DCATALOG_PAGELAYOUT

catalog_pagemode.o: catalog.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DCATALOG_PAGEMODE

catalog_setopenaction.o: catalog.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DCATALOG_SETOPENACTION

catalog_validate.o: catalog.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DCATALOG_VALIDATE

catalog_addpagelabel.o: catalog.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DCATALOG_ADDPAGELABEL

catalog_setviewerpreferences.o: catalog.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DCATALOG_SETVIEWERPREFERENCES

catalog_getviewerpreferences.o: catalog.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DCATALOG_GETVIEWERPREFERENCES


3dmeasur_addpoint.o: 3dmeasur.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DMEASURE3D_ADDPOINT

3dmeasur_new.o: 3dmeasur.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DMEASURE3D_NEW

3dmeasur_setcolor.o: 3dmeasur.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DMEASURE3D_SETCOLOR

3dmeasur_settextsize.o: 3dmeasur.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DMEASURE3D_SETTEXTSIZE

3dmeasur_setname.o: 3dmeasur.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DMEASURE3D_SETNAME

3dmeasur_settextboxsize.o: 3dmeasur.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DMEASURE3D_SETTEXTBOXSIZE

3dmeasur_settext.o: 3dmeasur.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DMEASURE3D_SETTEXT

3dmeasur_setprojectionannotation.o: 3dmeasur.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DMEASURE3D_SETPROJECTIONANNOTATION

3dmeasur_pd33dmeasure_new.o: 3dmeasur.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DMEASURE3D_PD33DMEASURE_NEW



EXTRADIST = \
	catalog.c \
	3dmeasur.c \
	$(empty)

AM_CPPFLAGS = -I$(srcdir)/../include -I$(top_srcdir)/include
AM_CPPFLAGS += $(Z_CFLAGS)
if HAVE_PNG
AM_CPPFLAGS += $(PNG_CFLAGS)
endif
