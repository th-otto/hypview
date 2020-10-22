include $(top_srcdir)/silent.am
include $(top_srcdir)/Makefile.common

noinst_LIBRARIES = libhpdf.a

libhpdf_a_SOURCES = \
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
	annotat_type_names.o \
	annotat_ending_names.o \
	annotat_new.o \
	annotat_setborderstyle.o \
	annotat_widgetannot_new.o \
	annotat_linkannot_new.o \
	annotat_actionannot_new.o \
	annotat_urilinkannot_new.o \
	annotat_gotolinkannot_new.o \
	annotat_gotorlinkannot_new.o \
	annotat_namedlinkannot_new.o \
	annotat_launchlinkannot_new.o \
	annotat_checksubtype.o \
	annotat_linkannot_setjavascript.o \
	annotat_linkannot_setborderstyle.o \
	annotat_linkannot_sethighlightmode.o \
	annotat_3dannot_new.o \
	annotat_markupannot_new.o \
	annotat_setrgbcolor.o \
	annotat_setcmykcolor.o \
	annotat_setgraycolor.o \
	annotat_setnocolor.o \
	annotat_text_seticon.o \
	annotat_text_setopened.o \
	annotat_popup_setopened.o \
	annotat_settitle.o \
	annotat_setsubject.o \
	annotat_setcreationdate.o \
	annotat_settransparency.o \
	annotat_setintent.o \
	annotat_setpopup.o \
	annotat_setinteriorrgb.o \
	annotat_setinteriorcmyk.o \
	annotat_setinteriorgray.o \
	annotat_setinteriortransparent.o \
	annotat_validate.o \
	annotat_set3dview.o \
	annotat_popupannot_new.o \
	annotat_stampannot_new.o \
	annotat_projectionannot_new.o \
	annotat_textmarkup_setquad.o \
	annotat_freetext_setlineending.o \
	annotat_markup_setrectdiff.o \
	annotat_freetext_setdefaultstyle.o \
	annotat_freetext_set3pointcalloutline.o \
	annotat_freetext_set2pointcalloutline.o \
	annotat_markup_setcloudeffect.o \
	annotat_line_setposition.o \
	annotat_line_setleader.o \
	annotat_line_setcaption.o \
	annotat_projection_setexdata.o \
	array_new.o \
	array_box_new.o \
	array_free.o \
	array_write.o \
	array_addnumber.o \
	array_addreal.o \
	array_addnull.o \
	array_addname.o \
	array_add.o \
	array_items.o \
	array_insert.o \
	array_getitem.o \
	array_clear.o \
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



annotat_type_names.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_TYPE_NAMES

annotat_ending_names.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_ENDING_NAMES

annotat_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_NEW

annotat_setborderstyle.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETBORDERSTYLE

annotat_widgetannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_WIDGETANNOT_NEW

annotat_linkannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_LINKANNOT_NEW

annotat_actionannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_ACTIONANNOT_NEW

annotat_urilinkannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_URILINKANNOT_NEW

annotat_gotolinkannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_GOTOLINKANNOT_NEW

annotat_gotorlinkannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_GOTORLINKANNOT_NEW

annotat_namedlinkannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_NAMEDLINKANNOT_NEW

annotat_launchlinkannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_LAUNCHLINKANNOT_NEW

annotat_checksubtype.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_CHECKSUBTYPE

annotat_linkannot_setjavascript.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_LINK_SETJAVASCRIPT

annotat_linkannot_setborderstyle.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_LINK_SETBORDERSTYLE

annotat_linkannot_sethighlightmode.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_LINK_SETHIGHLIGHT

annotat_3dannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_3DANNOT_NEW

annotat_markupannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_MARKUPANNOT_NEW

annotat_setrgbcolor.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETRGBCOLOR

annotat_setcmykcolor.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETCMYKCOLOR

annotat_setgraycolor.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETGRAYCOLOR

annotat_setnocolor.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETNOCOLOR

annotat_text_seticon.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_TEXT_SETICON

annotat_text_setopened.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_TEXT_SETOPENED

annotat_popup_setopened.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_POPUP_SETOPENED

annotat_settitle.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETITLE

annotat_setsubject.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETSUBJECT

annotat_setcreationdate.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETCREATIONDATE

annotat_settransparency.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETTRANSPARENCY

annotat_setintent.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETINTENT

annotat_setpopup.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETPOPUP

annotat_setinteriorrgb.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETINTERIORRGB

annotat_setinteriorcmyk.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETINTERIORCMYK

annotat_setinteriorgray.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETINTERIORGRAY

annotat_setinteriortransparent.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SETINTERIORTRANSPARENT

annotat_validate.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_VALIDATE

annotat_set3dview.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_SET3DVIEW

annotat_popupannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_POPUPANNOT_NEW

annotat_stampannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_STAMPANNOT_NEW

annotat_projectionannot_new.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_PROJECTIONANNOT_NEW

annotat_textmarkup_setquad.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_TEXT_SETQUAD

annotat_freetext_setlineending.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_TEXT_SETLINEENDING

annotat_markup_setrectdiff.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_MARKUP_SETRECTDIFF

annotat_freetext_setdefaultstyle.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_TEXT_SETDEFAULTSTYLE

annotat_freetext_set3pointcalloutline.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_TEXT_SET3POINTCALLOUTLINE

annotat_freetext_set2pointcalloutline.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT__TEXT_SET2POINTCALLOUTLINE

annotat_markup_setcloudeffect.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_MARKUP_SETCLOUDEFFECT

annotat_line_setposition.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_LINE_SETPOSITION

annotat_line_setleader.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_LINE_SETLEADER

annotat_line_setcaption.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_LINE_SETCAPTION

annotat_projection_setexdata.o: annotat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DANNOTAT_PROJECTION_SETEXDATA



array_new.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_NEW

array_box_new.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_BOX_NEW

array_free.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_FREE

array_write.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_WRITE

array_addnumber.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_ADDNUMBER

array_addreal.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_ADDREAL

array_addnull.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_ADDNULL

array_addname.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_ADDNAME

array_add.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_ADD

array_items.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_ITEMS

array_insert.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_INSERT

array_getitem.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_GETITEM

array_clear.o: array.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DARRAY_CLEAR



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
	annotat.c \
	array.c \
	catalog.c \
	3dmeasur.c \
	$(empty)

AM_CPPFLAGS = -I$(srcdir)/../include -I$(top_srcdir)/include
AM_CPPFLAGS += $(Z_CFLAGS)
if HAVE_PNG
AM_CPPFLAGS += $(PNG_CFLAGS)
endif
