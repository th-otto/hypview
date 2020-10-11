include $(top_srcdir)/silent.am
include $(top_srcdir)/Makefile.common

noinst_LIBRARIES = libhpdf.a

libhpdf_a_SOURCES = \
	annotat.c \
	array.c \
	binary.c \
	boolean.c \
	catalog.c \
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
	3dmeasur.c \
	exdata.c \
	enc_utf8.c \
	t4.h \
	$(empty)

AM_CPPFLAGS = -I$(srcdir)/../include -I$(top_srcdir)/include
AM_CPPFLAGS += $(Z_CFLAGS)
if HAVE_PNG
AM_CPPFLAGS += $(PNG_CFLAGS)
endif
