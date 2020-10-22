include $(top_srcdir)/silent.am
include $(top_srcdir)/Makefile.common

noinst_LIBRARIES = libhpdf.a

libhpdf_a_SOURCES = \
	binary.c \
	boolean.c \
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
	destinat_new.o \
	destinat_validate.o \
	destinat_setxyz.o \
	destinat_setfit.o \
	destinat_setfith.o \
	destinat_setfitv.o \
	destinat_setfitr.o \
	destinat_setfitb.o \
	destinat_setfitbh.o \
	destinat_setfitbv.o \
	page_setlinewidth.o \
	page_setlinecap.o \
	page_setlinejoin.o \
	page_setmiterlimit.o \
	page_setdash.o \
	page_setflat.o \
	page_setextgstate.o \
	page_gsave.o \
	page_grestore.o \
	page_concat.o \
	page_moveto.o \
	page_lineto.o \
	page_curveto.o \
	page_curveto2.o \
	page_curveto3.o \
	page_closepath.o \
	page_rectangle.o \
	page_stroke.o \
	page_closepathstroke.o \
	page_fill.o \
	page_eofill.o \
	page_fillstroke.o \
	page_eofillstroke.o \
	page_closepathfillstroke.o \
	page_closepatheofillstroke.o \
	page_endpath.o \
	page_clip.o \
	page_eoclip.o \
	page_begintext.o \
	page_endtext.o \
	page_setcharspace.o \
	page_setwordspace.o \
	page_sethorizontalscaling.o \
	page_settextleading.o \
	page_setfontandsize.o \
	page_settextrenderingmode.o \
	page_settextrise.o \
	page_movetextpos.o \
	page_movetextpos2.o \
	page_settextmatrix.o \
	page_movetonextline.o \
	page_internalwritetext.o \
	page_showtext.o \
	page_showtextnextline.o \
	page_showtextnextlineex.o \
	page_setgrayfill.o \
	page_setgraystroke.o \
	page_setrgbfill.o \
	page_setrgbstroke.o \
	page_setcmykfill.o \
	page_setcmykstroke.o \
	page_executexobject.o \
	page_circle.o \
	page_llipse.o \
	page_arc.o \
	page_drawimage.o \
	page_textpos_abstorel.o \
	page_textout.o \
	page_textrect.o \
	page_setslideshow.o \
	page_new_content_stream.o \
	page_insert_shared_content_stream.o \
	pages_sizes.o \
	pages_new.o \
	pages_addkids.o \
	pages_insertbefore.o \
	pages_validate.o \
	page_new.o \
	page_checkstate.o \
	page_getinheritableitem.o \
	page_getlocalfontname.o \
	page_getmediabox.o \
	page_createxobjectfromimage.o \
	page_createxobjectaswhiterect.o \
	page_getxobjectname.o \
	page_getextgstatename.o \
	page_addannotation.o \
	page_textsize.o \
	page_measuretext.o \
	page_getwidth.o \
	page_getheight.o \
	page_getcurrentfont.o \
	page_getcurrentfontsize.o \
	page_gettransmatrix.o \
	page_getlinewidth.o \
	page_getlinecap.o \
	page_getlinejoin.o \
	page_getmiterlimit.o \
	page_getdash.o \
	page_getflat.o \
	page_getwordspae.o \
	page_getcharspae.o \
	page_gethorizontalscaling.o \
	page_gettextleading.o \
	page_gettextrenderingmode.o \
	page_gettextrise.o \
	page_getrgbfill.o \
	page_getrgbstroke.o \
	page_getcmykfill.o \
	page_getcmykstroke.o \
	page_getgrayfill.o \
	page_getgraystroke.o \
	page_getstrokingcolorspace.o \
	page_getfillingcolorspace.o \
	page_gettextmatrix.o \
	page_getgstatedepth.o \
	page_getgmode.o \
	page_getcurrentpos.o \
	page_getcurrenttextpos.o \
	page_setboxvalue.o \
	page_setrotate.o \
	page_setzoom.o \
	page_setwidth.o \
	page_setheight.o \
	page_setsize.o \
	page_validate.o \
	page_createdestination.o \
	page_create3dannot.o \
	page_createtextannot.o \
	page_createfreetextannot.o \
	page_createlineannot.o \
	page_createwidgetannot.o \
	page_createwidgetannot_whiteonlywhileprint.o \
	page_createlinkannot.o \
	page_creategotoannot.o \
	page_creategotorannot.o \
	page_createnamedannot.o \
	page_createlaunchannot.o \
	page_createurilinkannot.o \
	page_createcircleannot.o \
	page_createsquareannot.o \
	page_create3dview.o \
	page_createtextmarkupannot.o \
	page_createhighlightannot.o \
	page_createsquigglyannot.o \
	page_createunderlineannot.o \
	page_createstrikeoutannot.o \
	page_createpopupannot.o \
	page_createstampannot.o \
	page_createprojectionannot.o \
	page_create3dc3dmeasure.o \
	page_createpd33dmeasure.o \
	page_create3dannotexdata.o \
	page_setfilter.o \
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



destinat_new.o: destinat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DDESTINAT_NEW

destinat_validate.o: destinat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DDESTINAT_VALIDATE

destinat_setxyz.o: destinat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DDESTINAT_SETXYZ

destinat_setfit.o: destinat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DDESTINAT_SETFIT

destinat_setfith.o: destinat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DDESTINAT_SETFITH

destinat_setfitv.o: destinat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DDESTINAT_SETFITV

destinat_setfitr.o: destinat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DDESTINAT_SETFITR

destinat_setfitb.o: destinat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DDESTINAT_SETFITB

destinat_setfitbh.o: destinat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DDESTINAT_SETFITBH

destinat_setfitbv.o: destinat.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DDESTINAT_SETFITBV




pages_sizes.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGES_SIZES

pages_new.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGES_NEW

pages_addkids.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGES_ADDKIDS

pages_insertbefore.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGES_INSERTBEFORE

pages_validate.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGES_VALIDATE

page_new.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_NEW

page_checkstate.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CHECKSTATE

page_getinheritableitem.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETINHERITABLEITEM

page_getlocalfontname.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETLOCALFONTNAME

page_getmediabox.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETMEDIABOX

page_createxobjectfromimage.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEXOBJECTFROMIMAGE

page_createxobjectaswhiterect.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEXOBJECTASWHITERECT

page_getxobjectname.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETXOBJECTNAME

page_getextgstatename.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETEXTGSTATENAME

page_addannotation.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_ADDANNOTATION

page_textsize.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_TEXTWIDTH

page_measuretext.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_MEASURETEXT

page_getwidth.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETWIDTH

page_getheight.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETHEIGHT

page_getcurrentfont.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETCURRENTFONT

page_getcurrentfontsize.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETCURRENTFONTSIZE

page_gettransmatrix.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETTRANSMATRIX

page_getlinewidth.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETLINEWIDTH

page_getlinecap.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETLINECAP

page_getlinejoin.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETLINEJOIN

page_getmiterlimit.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETMITERLIMIT

page_getdash.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETDASH

page_getflat.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETFLAT

page_getwordspae.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETWORDSPACE

page_getcharspae.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETCHARSPACE

page_gethorizontalscaling.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETHORIZONTALSCALING

page_gettextleading.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETTEXTLEADING

page_gettextrenderingmode.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETTEXTRENDERINGMODE

page_gettextrise.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETTEXTRISE

page_getrgbfill.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETRGBFILL

page_getrgbstroke.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETRGBSTROKE

page_getcmykfill.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETCMYKFILL

page_getcmykstroke.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETCMYKSTROKE

page_getgrayfill.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETGRAYFILL

page_getgraystroke.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETGRAYSTROKE

page_getstrokingcolorspace.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETSTROKINGCOLORSPACE

page_getfillingcolorspace.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETFILLINGCOLORSPACE

page_gettextmatrix.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETTEXTMATRIX

page_getgstatedepth.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETGSTATEDEPTH

page_getgmode.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETGMODE

page_getcurrentpos.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETCURRENTPOS

page_getcurrenttextpos.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GETCURRENTTEXTPOS

page_setboxvalue.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETBOXVALUE

page_setrotate.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETROTATE

page_setzoom.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETZOOM

page_setwidth.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETWIDTH

page_setheight.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETHEIGHT

page_setsize.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETSIZE

page_validate.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_VALIDATE

page_createdestination.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEDESTINATION

page_create3dannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATE3DANNOT

page_createtextannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATETEXTANNOT

page_createfreetextannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEFREETEXTANNOT

page_createlineannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATELINEANNOT

page_createwidgetannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEWIDGETANNOT

page_createwidgetannot_whiteonlywhileprint.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEWIDGETANNOT_WHITEONLYWHILEPRINT

page_createlinkannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATELINKANNOT

page_creategotoannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEGOTOANNOT

page_creategotorannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEGOTORANNOT

page_createnamedannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATENAMEDANNOT

page_createlaunchannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATELAUNCHANNOT

page_createurilinkannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEURILINKANNOT

page_createcircleannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATECIRCLEANNOT

page_createsquareannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATESQUAREANNOT

page_create3dview.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATE3DVIEW

page_createtextmarkupannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATETEXTMARKUPANNOT

page_createhighlightannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEHIGHLIGHTANNOT

page_createsquigglyannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATESQUIGGLYANNOT

page_createunderlineannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEUNDERLINEANNOT

page_createstrikeoutannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATESTRIKEOUTANNOT

page_createpopupannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEPOPUPANNOT

page_createstampannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATESTAMPANNOT

page_createprojectionannot.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEPROJECTIONANNOT

page_create3dc3dmeasure.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATE3DC3DMEASURE

page_createpd33dmeasure.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATEPD33DMEASURE

page_create3dannotexdata.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CREATE3DANNOTEXDATA

page_setfilter.o: pages.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETFILTER




page_setlinewidth.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETLINEWIDTH

page_setlinecap.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETLINECAP

page_setlinejoin.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETLINEJOIN

page_setmiterlimit.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETMITERLIMIT

page_setdash.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETDASH

page_setflat.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETFLAT

page_setextgstate.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETEXTGSTATE

page_gsave.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GSAVE

page_grestore.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_GRESTORE

page_concat.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CONCAT

page_moveto.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_MOVETO

page_lineto.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_LINETO

page_curveto.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CURVETO

page_curveto2.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CURVETO2

page_curveto3.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CURVETO3

page_closepath.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CLOSEPATH

page_rectangle.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_RECTANGLE

page_stroke.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_STROKE

page_closepathstroke.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CLOSEPATHSTROKE

page_fill.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_FILL

page_eofill.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_EOFILL

page_fillstroke.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_FILLSTROKE

page_eofillstroke.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_EOFILLSTROKE

page_closepathfillstroke.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CLOSEPATHFILLSTROKE

page_closepatheofillstroke.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CLOSEPATHEOFILLSTROKE

page_endpath.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_ENDPATH

page_clip.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CLIP

page_eoclip.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_EOCLIP

page_begintext.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_BEGINTEXT

page_endtext.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_ENDTEXT

page_setcharspace.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETCHARSPACE

page_setwordspace.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETWORDSPACE

page_sethorizontalscaling.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETHORIZONTALSCALING

page_settextleading.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETTEXTLEADING

page_setfontandsize.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETFONTANDSIZE

page_settextrenderingmode.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETTEXTRENDERINGMODE

page_settextrise.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETTEXTRISE

page_movetextpos.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_MOVETEXTPOS

page_movetextpos2.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_MOVETEXTPOS2

page_settextmatrix.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETTEXTMATRIX

page_movetonextline.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_MOVETONEXTLINE

page_internalwritetext.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_INTERNALWRITETEXT

page_showtext.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SHOWTEXT

page_showtextnextline.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SHOWTEXTNEXTLINE

page_showtextnextlineex.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SHOWTEXTNEXTLINEEX

page_setgrayfill.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETGRAYFILL

page_setgraystroke.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETGRAYSTROKE

page_setrgbfill.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETRGBFILL

page_setrgbstroke.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETRGBSTROKE

page_setcmykfill.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETCMYKFILL

page_setcmykstroke.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETCMYKSTROKE

page_executexobject.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_EXECUTEXOBJECT

page_circle.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_CIRCLE

page_llipse.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_ELLIPSE

page_arc.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_ARC

page_drawimage.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_DRAWIMAGE

page_textpos_abstorel.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_TEXTPOS_ABSTOREL

page_textout.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_TEXTOUT

page_textrect.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_TEXTRECT

page_setslideshow.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_SETSLIDESHOW

page_new_content_stream.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_NEW_CONTENT_STREAM

page_insert_shared_content_stream.o: pageop.c
	$(AM_V_CC)$(COMPILE) -c -o $@ $< -DPAGE_INSERT_SHARED_CONTENT_STREAM




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
	destinat.c \
	pages.c \
	pageop.c \
	3dmeasur.c \
	$(empty)

AM_CPPFLAGS = -I$(srcdir)/../include -I$(top_srcdir)/include
AM_CPPFLAGS += $(Z_CFLAGS)
if HAVE_PNG
AM_CPPFLAGS += $(PNG_CFLAGS)
endif
