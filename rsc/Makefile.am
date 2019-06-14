AUTOMAKE_OPTIONS = subdir-objects

include $(top_srcdir)/silent.am
include $(top_srcdir)/Makefile.common

AM_CPPFLAGS = -I$(srcdir)/gemlib -I$(srcdir)/aes -I$(srcdir)/vdi -I$(srcdir)/include -I$(top_srcdir)/include $(GLIB_CFLAGS)
AM_CPPFLAGS += $(PNG_CFLAGS) $(Z_CFLAGS)
AM_CPPFLAGS += -DPRIVATE_AES -DPRIVATE_VDI

SUBDIRS = . src

noinst_LIBRARIES = libgem.a

libgem_a_SOURCES = \
	gemlib/gem_vdiP.h \
	gemlib/gem_aesP.h \
	gemlib/mt_gem.h \
	gemlib/gem.h \
	gemlib/aes.c \
	gemlib/v_bez.h \
	\
	gemlib/a_appl_exit.c \
	gemlib/a_appl_init.c \
	gemlib/a_vq_aes.c \
	\
	gemlib/a_form_alert.c \
	gemlib/a_form_center.c \
	gemlib/a_form_center_grect.c \
	gemlib/a_form_dial.c \
	gemlib/a_form_dial_grect.c \
	\
	gemlib/a_graf_handle.c \
	\
	gemlib/a_menu_register.c \
	gemlib/a_menu_bar.c \
	\
	gemlib/a_objc_draw.c \
	gemlib/a_objc_draw_grect.c \
	gemlib/a_objc_offset.c \
	gemlib/a_objc_find.c \
	\
	gemlib/a_wind_get.c \
	gemlib/a_wind_set.c \
	gemlib/a_wind_set_int.c \
	gemlib/a_wind_set_str.c \
	gemlib/a_wind_set_ptr.c \
	gemlib/a_wind_update.c \
	\
	gemlib/rc_copy.c \
	gemlib/rc_equal.c \
	gemlib/rc_intersect.c \
	gemlib/rc_union.c \
	gemlib/rc_array_to_grect.c \
	gemlib/rc_grect_to_array.c \
	gemlib/globals.c \
	\
	gemlib/vs_calibrate.c \
	gemlib/vs_color.c \
	gemlib/vsf_color.c \
	gemlib/vsf_interior.c \
	gemlib/vsf_perimeter.c \
	gemlib/vsf_xperimeter.c \
	gemlib/vsf_style.c \
	gemlib/vsf_udpat.c \
	gemlib/vsl_color.c \
	gemlib/vsl_ends.c \
	gemlib/vsl_type.c \
	gemlib/vsl_udsty.c \
	gemlib/vsl_width.c \
	gemlib/vsm_color.c \
	gemlib/vsm_height.c \
	gemlib/vsm_type.c \
	gemlib/vst_alignment.c \
	gemlib/vst_color.c \
	gemlib/vst_effects.c \
	gemlib/vst_error.c \
	gemlib/vst_font.c \
	gemlib/vst_height.c \
	gemlib/vst_point.c \
	gemlib/vst_rotation.c \
	gemlib/vst_scratch.c \
	gemlib/vswr_mode.c \
	\
	gemlib/v_opnwk.c \
	gemlib/v_opnvwk.c \
	gemlib/v_clsvwk.c \
	gemlib/v_clswk.c \
	gemlib/v_clrwk.c \
	gemlib/v_updwk.c \
	gemlib/v_flushcache.c \
	gemlib/v_loadcache.c \
	gemlib/v_savecache.c \
	gemlib/v_set_app_buff.c \
	gemlib/vs_clip.c \
	gemlib/vs_clip_pxy.c \
	gemlib/vs_clip_off.c \
	gemlib/vst_load_fonts.c \
	gemlib/vst_unload_fonts.c \
	\
	gemlib/v_alpha_text.c \
	gemlib/v_alpha_text16n.c \
	gemlib/v_bit_image.c \
	gemlib/v_xbit_image.c \
	gemlib/v_clear_disp_list.c \
	gemlib/v_copies.c \
	gemlib/v_curdown.c \
	gemlib/v_curhome.c \
	gemlib/v_curleft.c \
	gemlib/v_curright.c \
	gemlib/v_curtext.c \
	gemlib/v_curtext16n.c \
	gemlib/v_curup.c \
	gemlib/v_dspcur.c \
	gemlib/v_eeol.c \
	gemlib/v_eeos.c \
	gemlib/v_enter_cur.c \
	gemlib/v_escape2000.c \
	gemlib/v_exit_cur.c \
	gemlib/v_fontinit.c \
	gemlib/v_form_adv.c \
	gemlib/v_hardcopy.c \
	gemlib/v_hardcopy_ex.c \
	gemlib/v_meta_extents.c \
	gemlib/v_offset.c \
	gemlib/v_orient.c \
	gemlib/v_output_window.c \
	gemlib/v_page_size.c \
	gemlib/v_rmcur.c \
	gemlib/v_rvoff.c \
	gemlib/v_rvon.c \
	gemlib/v_sound.c \
	gemlib/v_trays.c \
	gemlib/v_ps_halftone.c \
	gemlib/v_write_meta.c \
	gemlib/vm_coords.c \
	gemlib/vm_filename.c \
	gemlib/vm_pagesize.c \
	gemlib/vq_calibrate.c \
	gemlib/vq_chcells.c \
	gemlib/vq_curaddress.c \
	gemlib/vq_page_name.c \
	gemlib/vq_scan.c \
	gemlib/vq_tabstatus.c \
	gemlib/vq_tdimensions.c \
	gemlib/vq_tray_names.c \
	gemlib/vqp_error.c \
	gemlib/vqp_filmname.c \
	gemlib/vs_curaddress.c \
	gemlib/vs_palette.c \
	gemlib/vs_mute.c \
	gemlib/vsc_expose.c \
	gemlib/vsp_film.c \
	gemlib/vqp_films.c \
	gemlib/vqp_state.c \
	gemlib/vsp_state.c \
	gemlib/vsp_save.c \
	gemlib/vsp_message.c \
	gemlib/vt_alignment.c \
	gemlib/vt_axis.c \
	gemlib/vt_origin.c \
	gemlib/vt_resolution.c \
	\
	gemlib/v_hide_c.c \
	gemlib/v_show_c.c \
	gemlib/vex_butv.c \
	gemlib/vex_curv.c \
	gemlib/vex_motv.c \
	gemlib/vex_wheelv.c \
	gemlib/vex_timv.c \
	gemlib/vq_key_s.c \
	gemlib/vq_mouse.c \
	gemlib/vrq_choice.c \
	gemlib/vrq_locator.c \
	gemlib/vrq_string.c \
	gemlib/vrq_string16.c \
	gemlib/vrq_valuator.c \
	gemlib/vsc_form.c \
	gemlib/vsin_mode.c \
	gemlib/vsm_choice.c \
	gemlib/vsm_locator.c \
	gemlib/vsm_string.c \
	gemlib/vsm_string16.c \
	gemlib/vsm_valuator.c \
	\
	gemlib/vq_calibrate.c \
	gemlib/vq_cellarray.c \
	gemlib/vq_color.c \
	gemlib/vq_extnd.c \
	gemlib/vqf_attributes.c \
	gemlib/vqin_mode.c \
	gemlib/vql_attributes.c \
	gemlib/vqm_attributes.c \
	gemlib/vqt_attributes.c \
	gemlib/vqt_cachesize.c \
	gemlib/vqt_extent.c \
	gemlib/vqt_extentn.c \
	gemlib/vqt_extent16.c \
	gemlib/vqt_extent16n.c \
	gemlib/vqt_fontinfo.c \
	gemlib/vqt_get_table.c \
	gemlib/vqt_name.c \
	gemlib/vqt_width.c \
	\
	gemlib/v_arc.c \
	gemlib/v_bar.c \
	gemlib/v_cellarray.c \
	gemlib/v_circle.c \
	gemlib/v_contourfill.c \
	gemlib/v_ellarc.c \
	gemlib/v_ellipse.c \
	gemlib/v_ellpie.c \
	gemlib/v_fillarea.c \
	gemlib/v_gtext.c \
	gemlib/v_gtextn.c \
	gemlib/v_gtext16.c \
	gemlib/v_gtext16n.c \
	gemlib/v_justified.c \
	gemlib/v_justified16n.c \
	gemlib/v_pieslice.c \
	gemlib/v_pline.c \
	gemlib/v_pmarker.c \
	gemlib/v_rbox.c \
	gemlib/v_rfbox.c \
	gemlib/vr_recfl.c \
	\
	gemlib/v_get_pixel.c \
	gemlib/vr_trnfm.c \
	gemlib/vro_cpyfm.c \
	gemlib/vrt_cpyfm.c \
	\
	gemlib/v_write_png.c \
	\
	gemlib/n_v_bez.c \
	gemlib/n_v_bez_fill.c \
	gemlib/n_v_bez_off.c \
	gemlib/n_v_bez_on.c \
	gemlib/n_v_bez_con.c \
	gemlib/n_v_bez_qual.c \
	gemlib/n_v_create_driver_info.c \
	gemlib/n_v_delete_driver_info.c \
	gemlib/n_v_opnprn.c \
	gemlib/n_v_read_default_settings.c \
	gemlib/n_v_write_default_settings.c \
	gemlib/n_v_clsbm.c \
	gemlib/n_v_color2nearest.c \
	gemlib/n_v_color2value.c \
	gemlib/n_v_create_ctab.c \
	gemlib/n_v_create_itab.c \
	gemlib/n_v_ctab_idx2value.c \
	gemlib/n_v_ctab_idx2vdi.c \
	gemlib/n_v_ctab_vdi2idx.c \
	gemlib/n_v_delete_ctab.c \
	gemlib/n_v_delete_itab.c \
	gemlib/n_v_ftext.c \
	gemlib/n_v_ftextn.c \
	gemlib/n_v_ftext16.c \
	gemlib/n_v_ftext16n.c \
	gemlib/n_v_ftext_offset.c \
	gemlib/n_v_ftext_offset16.c \
	gemlib/n_v_ftext_offset16n.c \
	gemlib/n_v_get_ctab_id.c \
	gemlib/n_v_get_outline.c \
	gemlib/n_v_getbitmap_info.c \
	gemlib/n_v_getoutline.c \
	gemlib/n_v_killoutline.c \
	gemlib/n_v_open_bm.c \
	gemlib/n_v_opnbm.c \
	gemlib/n_v_resize_bm.c \
	gemlib/n_v_setrgb.c \
	gemlib/n_v_value2color.c \
	gemlib/n_vq_ctab.c \
	gemlib/n_vq_ctab_entry.c \
	gemlib/n_vq_ctab_id.c \
	gemlib/n_vq_devinfo.c \
	gemlib/n_vq_dflt_ctab.c \
	gemlib/n_vq_ext_devinfo.c \
	gemlib/n_vq_hilite_color.c \
	gemlib/n_vq_margins.c \
	gemlib/n_vq_max_color.c \
	gemlib/n_vq_min_color.c \
	gemlib/n_vq_prn_scaling.c \
	gemlib/n_vq_px_format.c \
	gemlib/n_vq_scrninfo.c \
	gemlib/n_vq_weight_color.c \
	gemlib/n_vqf_bg_color.c \
	gemlib/n_vqf_fg_color.c \
	gemlib/n_vql_bg_color.c \
	gemlib/n_vql_fg_color.c \
	gemlib/n_vqm_bg_color.c \
	gemlib/n_vqm_fg_color.c \
	gemlib/n_vqr_bg_color.c \
	gemlib/n_vqr_fg_color.c \
	gemlib/n_vqt_advance.c \
	gemlib/n_vqt_advance32.c \
	gemlib/n_vqt_bg_color.c \
	gemlib/n_vqt_char_index.c \
	gemlib/n_vqt_devinfo.c \
	gemlib/n_vqt_ext_name.c \
	gemlib/n_vqt_f_extent.c \
	gemlib/n_vqt_f_extent16.c \
	gemlib/n_vqt_f_extent16n.c \
	gemlib/n_vqt_f_extentn.c \
	gemlib/n_vqt_fg_color.c \
	gemlib/n_vqt_fontheader.c \
	gemlib/n_vqt_name_and_id.c \
	gemlib/n_vqt_pairkern.c \
	gemlib/n_vqt_real_extent.c \
	gemlib/n_vqt_real_extentn.c \
	gemlib/n_vqt_real_extent16n.c \
	gemlib/n_vqt_trackkern.c \
	gemlib/n_vqt_xfntinfo.c \
	gemlib/n_vr_transfer_bits.c \
	gemlib/n_vs_ctab.c \
	gemlib/n_vs_ctab_entry.c \
	gemlib/n_vs_dflt_ctab.c \
	gemlib/n_vs_ctab_entry.c \
	gemlib/n_vs_dflt_ctab.c \
	gemlib/n_vs_document_info.c \
	gemlib/n_vs_hilite_color.c \
	gemlib/n_vs_max_color.c \
	gemlib/n_vs_min_color.c \
	gemlib/n_vs_weight_color.c \
	gemlib/n_vsf_bg_color.c \
	gemlib/n_vsf_fg_color.c \
	gemlib/n_vsl_bg_color.c \
	gemlib/n_vsl_fg_color.c \
	gemlib/n_vsm_bg_color.c \
	gemlib/n_vsm_fg_color.c \
	gemlib/n_vsr_bg_color.c \
	gemlib/n_vsr_fg_color.c \
	gemlib/n_vst_arbpt.c \
	gemlib/n_vst_arbpt32.c \
	gemlib/n_vst_bg_color.c \
	gemlib/n_vst_charmap.c \
	gemlib/n_vst_fg_color.c \
	gemlib/n_vst_kern.c \
	gemlib/n_vst_map_mode.c \
	gemlib/n_vst_name.c \
	gemlib/n_vst_setsize.c \
	gemlib/n_vst_setsize32.c \
	gemlib/n_vst_skew.c \
	gemlib/n_vst_track_offset.c \
	gemlib/n_vst_width.c \
	gemlib/n_vq_driver_info.c \
	gemlib/n_vq_bit_image.c \
	gemlib/n_vs_page_info.c \
	gemlib/n_vs_crop.c \
	gemlib/n_vq_image_type.c \
	gemlib/n_vs_save_disp_list.c \
	gemlib/n_vs_load_disp_list.c \
	\
	gemlib/vdi_array2str.c \
	gemlib/vdi_str2array.c \
	gemlib/vdi_wstrlen.c \
	\
	gemlib/vdi_dummy.c \
	gemlib/vdi_userdef.c gemlib/vdi_userdef.h \
	\
	gemlib/vq_gdos.c \
	gemlib/vq_vgdos.c \
	gemlib/vdi.c \
	\
	$(empty)

libgem_a_SOURCES += \
	aes/aes.c aes/aes.h \
	aes/aesdefs.h aes/crysbind.h aes/gemlib.h aes/gempd.h aes/gsxdefs.h aes/struct.h \
	\
	aes/gemaplib.c \
	aes/gembase.c \
	aes/gemctrl.c \
	aes/gemevlib.c \
	aes/gemfmalt.c \
	aes/gemfmlib.c \
	aes/gemfslib.c \
	aes/gemglobe.c \
	aes/gemgraf.c \
	aes/gemgrlib.c \
	aes/gemgsxif.c \
	aes/geminit.c \
	aes/geminput.c \
	aes/gemmnlib.c \
	aes/gemobed.c \
	aes/gempd.c \
	aes/gemobjop.c \
	aes/gemoblib.c \
	aes/gemrslib.c \
	aes/gemsclib.c \
	aes/gemshlib.c \
	aes/gemwmlib.c \
	aes/gemwrect.c \
	aes/aesutils.c \
	\
	aes/gemdos.c \
	\
	aes/gem_rsc.c aes/gem_rsc.rsc aes/gem_rsc.rso aes/gem_rsc.rsh aes/gem_rsc.h \
	\
	$(empty)

libgem_a_SOURCES += \
	vdi/vdi.c vdi/vdi.h \
	vdi/vdi_bm.c \
	vdi/maptab.c vdi/maptab.h \
	\
	vdi/vdidefs.h vdi/vdimaps.h vdi/pattern.h \
	\
	vdi/writepng.c vdi/writepng.h \
	$(empty)

EXTRA_DIST = \
	include \
	data
