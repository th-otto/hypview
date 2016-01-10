/* GDK - The GIMP Drawing Kit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 * Copyright (C) 2005, 2006, 2007, 2009 GNOME Foundation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


/*
 * File auto-generated from script gdkkeysyms-update.pl
 * using the input file
 * http://gitweb.freedesktop.org/?p=xorg/proto/x11proto.git;a=blob_plain;f=keysymdef.h
 * and
 * http://gitweb.freedesktop.org/?p=xorg/proto/x11proto.git;a=blob_plain;f=XF86keysym.h
 */

/*
 * Compatibility wrapper for gdkkeysyms.h.
 * In GTK3, keysyms changed to have a KEY_ prefix.  This is a compatibility header
 * your application can include to map the new names to the old names,
 * in case your you are still compiling with GTK2.
 * This assumes that your application has already been ported to use the new names.
 */

#ifndef __GDK_KEYSYMS_WRAPPER_H__
#define __GDK_KEYSYMS_WRAPPER_H__


#include <gdk/gdkkeysyms.h>

#ifndef GDK_KEY_BackSpace
#define GDK_KEY_BackSpace 0xff08
#endif
#ifndef GDK_KEY_Cyrillic_HA
#define GDK_KEY_Cyrillic_HA 0x6e8
#endif
#ifndef GDK_KEY_Sacute
#define GDK_KEY_Sacute 0x1a6
#endif
#ifndef GDK_KEY_braille_dots_25678
#define GDK_KEY_braille_dots_25678 0x10028f2
#endif
#ifndef GDK_KEY_braille_dots_1478
#define GDK_KEY_braille_dots_1478 0x10028c9
#endif
#ifndef GDK_KEY_3270_Attn
#define GDK_KEY_3270_Attn 0xfd0e
#endif
#ifndef GDK_KEY_onesuperior
#define GDK_KEY_onesuperior 0x0b9
#endif
#ifndef GDK_KEY_upleftcorner
#define GDK_KEY_upleftcorner 0x9ec
#endif
#ifndef GDK_KEY_acircumflextilde
#define GDK_KEY_acircumflextilde 0x1001eab
#endif
#ifndef GDK_KEY_6
#define GDK_KEY_6 0x036
#endif
#ifndef GDK_KEY_Omacron
#define GDK_KEY_Omacron 0x3d2
#endif
#ifndef GDK_KEY_braille_dots_58
#define GDK_KEY_braille_dots_58 0x1002890
#endif
#ifndef GDK_KEY_Arabic_5
#define GDK_KEY_Arabic_5 0x1000665
#endif
#ifndef GDK_KEY_Greek_GAMMA
#define GDK_KEY_Greek_GAMMA 0x7c3
#endif
#ifndef GDK_KEY_Armenian_to
#define GDK_KEY_Armenian_to 0x1000569
#endif
#ifndef GDK_KEY_Greek_OMICRON
#define GDK_KEY_Greek_OMICRON 0x7cf
#endif
#ifndef GDK_KEY_Hangul_AraeA
#define GDK_KEY_Hangul_AraeA 0xef6
#endif
#ifndef GDK_KEY_Select
#define GDK_KEY_Select 0xff60
#endif
#ifndef GDK_KEY_Cyrillic_el
#define GDK_KEY_Cyrillic_el 0x6cc
#endif
#ifndef GDK_KEY_LaunchA
#define GDK_KEY_LaunchA 0x1008ff4a
#endif
#ifndef GDK_KEY_Mail
#define GDK_KEY_Mail 0x1008ff19
#endif
#ifndef GDK_KEY_degree
#define GDK_KEY_degree 0x0b0
#endif
#ifndef GDK_KEY_braille_dots_123568
#define GDK_KEY_braille_dots_123568 0x10028b7
#endif
#ifndef GDK_KEY_3270_Copy
#define GDK_KEY_3270_Copy 0xfd15
#endif
#ifndef GDK_KEY_Redo
#define GDK_KEY_Redo 0xff66
#endif
#ifndef GDK_KEY_braille_dots_12357
#define GDK_KEY_braille_dots_12357 0x1002857
#endif
#ifndef GDK_KEY_braille_dots_468
#define GDK_KEY_braille_dots_468 0x10028a8
#endif
#ifndef GDK_KEY_dead_belowtilde
#define GDK_KEY_dead_belowtilde 0xfe6a
#endif
#ifndef GDK_KEY_braille_dots_256
#define GDK_KEY_braille_dots_256 0x1002832
#endif
#ifndef GDK_KEY_nabla
#define GDK_KEY_nabla 0x8c5
#endif
#ifndef GDK_KEY_Otilde
#define GDK_KEY_Otilde 0x0d5
#endif
#ifndef GDK_KEY_L5
#define GDK_KEY_L5 0xffcc
#endif
#ifndef GDK_KEY_braille_dots_2578
#define GDK_KEY_braille_dots_2578 0x10028d2
#endif
#ifndef GDK_KEY_Hangul_J_RieulSios
#define GDK_KEY_Hangul_J_RieulSios 0xedf
#endif
#ifndef GDK_KEY_Hangul_Hieuh
#define GDK_KEY_Hangul_Hieuh 0xebe
#endif
#ifndef GDK_KEY_Pointer_Drag_Dflt
#define GDK_KEY_Pointer_Drag_Dflt 0xfef4
#endif
#ifndef GDK_KEY_Prev_Virtual_Screen
#define GDK_KEY_Prev_Virtual_Screen 0xfed1
#endif
#ifndef GDK_KEY_HomePage
#define GDK_KEY_HomePage 0x1008ff18
#endif
#ifndef GDK_KEY_AudioMedia
#define GDK_KEY_AudioMedia 0x1008ff32
#endif
#ifndef GDK_KEY_scaron
#define GDK_KEY_scaron 0x1b9
#endif
#ifndef GDK_KEY_F5
#define GDK_KEY_F5 0xffc2
#endif
#ifndef GDK_KEY_Cyrillic_shorti
#define GDK_KEY_Cyrillic_shorti 0x6ca
#endif
#ifndef GDK_KEY_Forward
#define GDK_KEY_Forward 0x1008ff27
#endif
#ifndef GDK_KEY_braille_dots_4678
#define GDK_KEY_braille_dots_4678 0x10028e8
#endif
#ifndef GDK_KEY_leftpointer
#define GDK_KEY_leftpointer 0xaea
#endif
#ifndef GDK_KEY_sacute
#define GDK_KEY_sacute 0x1b6
#endif
#ifndef GDK_KEY_Dcaron
#define GDK_KEY_Dcaron 0x1cf
#endif
#ifndef GDK_KEY_Hangul_J_Ieung
#define GDK_KEY_Hangul_J_Ieung 0xee8
#endif
#ifndef GDK_KEY_Massyo
#define GDK_KEY_Massyo 0xff2c
#endif
#ifndef GDK_KEY_Thai_popla
#define GDK_KEY_Thai_popla 0xdbb
#endif
#ifndef GDK_KEY_Last_Virtual_Screen
#define GDK_KEY_Last_Virtual_Screen 0xfed4
#endif
#ifndef GDK_KEY_History
#define GDK_KEY_History 0x1008ff37
#endif
#ifndef GDK_KEY_Ocircumflexacute
#define GDK_KEY_Ocircumflexacute 0x1001ed0
#endif
#ifndef GDK_KEY_horizconnector
#define GDK_KEY_horizconnector 0x8a3
#endif
#ifndef GDK_KEY_braille_dots_8
#define GDK_KEY_braille_dots_8 0x1002880
#endif
#ifndef GDK_KEY_gcedilla
#define GDK_KEY_gcedilla 0x3bb
#endif
#ifndef GDK_KEY_DOS
#define GDK_KEY_DOS 0x1008ff5a
#endif
#ifndef GDK_KEY_Georgian_tar
#define GDK_KEY_Georgian_tar 0x10010e2
#endif
#ifndef GDK_KEY_Hangul_Rieul
#define GDK_KEY_Hangul_Rieul 0xea9
#endif
#ifndef GDK_KEY_kana_YO
#define GDK_KEY_kana_YO 0x4d6
#endif
#ifndef GDK_KEY_at
#define GDK_KEY_at 0x040
#endif
#ifndef GDK_KEY_3270_Setup
#define GDK_KEY_3270_Setup 0xfd17
#endif
#ifndef GDK_KEY_botrightsqbracket
#define GDK_KEY_botrightsqbracket 0x8aa
#endif
#ifndef GDK_KEY_Hangul_J_RieulPieub
#define GDK_KEY_Hangul_J_RieulPieub 0xede
#endif
#ifndef GDK_KEY_Cancel
#define GDK_KEY_Cancel 0xff69
#endif
#ifndef GDK_KEY_Copy
#define GDK_KEY_Copy 0x1008ff57
#endif
#ifndef GDK_KEY_B
#define GDK_KEY_B 0x042
#endif
#ifndef GDK_KEY_braille_dots_148
#define GDK_KEY_braille_dots_148 0x1002889
#endif
#ifndef GDK_KEY_breve
#define GDK_KEY_breve 0x1a2
#endif
#ifndef GDK_KEY_P
#define GDK_KEY_P 0x050
#endif
#ifndef GDK_KEY_Arabic_0
#define GDK_KEY_Arabic_0 0x1000660
#endif
#ifndef GDK_KEY_hebrew_resh
#define GDK_KEY_hebrew_resh 0xcf8
#endif
#ifndef GDK_KEY_hebrew_samech
#define GDK_KEY_hebrew_samech 0xcf1
#endif
#ifndef GDK_KEY_kcedilla
#define GDK_KEY_kcedilla 0x3f3
#endif
#ifndef GDK_KEY_zacute
#define GDK_KEY_zacute 0x1bc
#endif
#ifndef GDK_KEY_fivesuperior
#define GDK_KEY_fivesuperior 0x1002075
#endif
#ifndef GDK_KEY_Switch_VT_12
#define GDK_KEY_Switch_VT_12 0x1008fe0c
#endif
#ifndef GDK_KEY_braille_dots_258
#define GDK_KEY_braille_dots_258 0x1002892
#endif
#ifndef GDK_KEY_braille_dots_13456
#define GDK_KEY_braille_dots_13456 0x100283d
#endif
#ifndef GDK_KEY_Thai_khokhai
#define GDK_KEY_Thai_khokhai 0xda2
#endif
#ifndef GDK_KEY_AudioStop
#define GDK_KEY_AudioStop 0x1008ff15
#endif
#ifndef GDK_KEY_Hangul_NieunJieuj
#define GDK_KEY_Hangul_NieunJieuj 0xea5
#endif
#ifndef GDK_KEY_masculine
#define GDK_KEY_masculine 0x0ba
#endif
#ifndef GDK_KEY_Armenian_SHA
#define GDK_KEY_Armenian_SHA 0x1000547
#endif
#ifndef GDK_KEY_singlelowquotemark
#define GDK_KEY_singlelowquotemark 0xafd
#endif
#ifndef GDK_KEY_kana_I
#define GDK_KEY_kana_I 0x4b2
#endif
#ifndef GDK_KEY_Switch_VT_9
#define GDK_KEY_Switch_VT_9 0x1008fe09
#endif
#ifndef GDK_KEY_Armenian_SE
#define GDK_KEY_Armenian_SE 0x100054d
#endif
#ifndef GDK_KEY_Armenian_ben
#define GDK_KEY_Armenian_ben 0x1000562
#endif
#ifndef GDK_KEY_stricteq
#define GDK_KEY_stricteq 0x1002263
#endif
#ifndef GDK_KEY_ybelowdot
#define GDK_KEY_ybelowdot 0x1001ef5
#endif
#ifndef GDK_KEY_numbersign
#define GDK_KEY_numbersign 0x023
#endif
#ifndef GDK_KEY_Greek_iota
#define GDK_KEY_Greek_iota 0x7e9
#endif
#ifndef GDK_KEY_Control_L
#define GDK_KEY_Control_L 0xffe3
#endif
#ifndef GDK_KEY_Hangul_RieulPieub
#define GDK_KEY_Hangul_RieulPieub 0xeac
#endif
#ifndef GDK_KEY_Greek_OMEGAaccent
#define GDK_KEY_Greek_OMEGAaccent 0x7ab
#endif
#ifndef GDK_KEY_dead_u
#define GDK_KEY_dead_u 0xfe88
#endif
#ifndef GDK_KEY_Abelowdot
#define GDK_KEY_Abelowdot 0x1001ea0
#endif
#ifndef GDK_KEY_Open
#define GDK_KEY_Open 0x1008ff6b
#endif
#ifndef GDK_KEY_braille_dots_34567
#define GDK_KEY_braille_dots_34567 0x100287c
#endif
#ifndef GDK_KEY_script_switch
#define GDK_KEY_script_switch 0xff7e
#endif
#ifndef GDK_KEY_Macedonia_kje
#define GDK_KEY_Macedonia_kje 0x6ac
#endif
#ifndef GDK_KEY_iogonek
#define GDK_KEY_iogonek 0x3e7
#endif
#ifndef GDK_KEY_3270_Reset
#define GDK_KEY_3270_Reset 0xfd08
#endif
#ifndef GDK_KEY_Escape
#define GDK_KEY_Escape 0xff1b
#endif
#ifndef GDK_KEY_hebrew_samekh
#define GDK_KEY_hebrew_samekh 0xcf1
#endif
#ifndef GDK_KEY_ISO_Last_Group
#define GDK_KEY_ISO_Last_Group 0xfe0e
#endif
#ifndef GDK_KEY_Cyrillic_u_macron
#define GDK_KEY_Cyrillic_u_macron 0x10004ef
#endif
#ifndef GDK_KEY_Thai_maitri
#define GDK_KEY_Thai_maitri 0xdea
#endif
#ifndef GDK_KEY_Arabic_jeem
#define GDK_KEY_Arabic_jeem 0x5cc
#endif
#ifndef GDK_KEY_Cyrillic_ya
#define GDK_KEY_Cyrillic_ya 0x6d1
#endif
#ifndef GDK_KEY_braille_dots_12348
#define GDK_KEY_braille_dots_12348 0x100288f
#endif
#ifndef GDK_KEY_Thai_khokhon
#define GDK_KEY_Thai_khokhon 0xda5
#endif
#ifndef GDK_KEY_Cyrillic_u_straight_bar
#define GDK_KEY_Cyrillic_u_straight_bar 0x10004b1
#endif
#ifndef GDK_KEY_KP_Page_Up
#define GDK_KEY_KP_Page_Up 0xff9a
#endif
#ifndef GDK_KEY_braille_dots_13
#define GDK_KEY_braille_dots_13 0x1002805
#endif
#ifndef GDK_KEY_Ohorntilde
#define GDK_KEY_Ohorntilde 0x1001ee0
#endif
#ifndef GDK_KEY_leftopentriangle
#define GDK_KEY_leftopentriangle 0xacc
#endif
#ifndef GDK_KEY_Ukrainian_YI
#define GDK_KEY_Ukrainian_YI 0x6b7
#endif
#ifndef GDK_KEY_Thai_chochang
#define GDK_KEY_Thai_chochang 0xdaa
#endif
#ifndef GDK_KEY_lessthanequal
#define GDK_KEY_lessthanequal 0x8bc
#endif
#ifndef GDK_KEY_Greek_chi
#define GDK_KEY_Greek_chi 0x7f7
#endif
#ifndef GDK_KEY_Cyrillic_EN_descender
#define GDK_KEY_Cyrillic_EN_descender 0x10004a2
#endif
#ifndef GDK_KEY_hebrew_kaph
#define GDK_KEY_hebrew_kaph 0xceb
#endif
#ifndef GDK_KEY_Armenian_tso
#define GDK_KEY_Armenian_tso 0x1000581
#endif
#ifndef GDK_KEY_3270_AltCursor
#define GDK_KEY_3270_AltCursor 0xfd10
#endif
#ifndef GDK_KEY_BackForward
#define GDK_KEY_BackForward 0x1008ff3f
#endif
#ifndef GDK_KEY_braille_dots_346
#define GDK_KEY_braille_dots_346 0x100282c
#endif
#ifndef GDK_KEY_crossinglines
#define GDK_KEY_crossinglines 0x9ee
#endif
#ifndef GDK_KEY_Thai_chochoe
#define GDK_KEY_Thai_chochoe 0xdac
#endif
#ifndef GDK_KEY_Armenian_tsa
#define GDK_KEY_Armenian_tsa 0x100056e
#endif
#ifndef GDK_KEY_Launch7
#define GDK_KEY_Launch7 0x1008ff47
#endif
#ifndef GDK_KEY_checkerboard
#define GDK_KEY_checkerboard 0x9e1
#endif
#ifndef GDK_KEY_function
#define GDK_KEY_function 0x8f6
#endif
#ifndef GDK_KEY_Go
#define GDK_KEY_Go 0x1008ff5f
#endif
#ifndef GDK_KEY_abovedot
#define GDK_KEY_abovedot 0x1ff
#endif
#ifndef GDK_KEY_Subtitle
#define GDK_KEY_Subtitle 0x1008ff9a
#endif
#ifndef GDK_KEY_leftmiddlecurlybrace
#define GDK_KEY_leftmiddlecurlybrace 0x8af
#endif
#ifndef GDK_KEY_twofifths
#define GDK_KEY_twofifths 0xab3
#endif
#ifndef GDK_KEY_3270_PA1
#define GDK_KEY_3270_PA1 0xfd0a
#endif
#ifndef GDK_KEY_braille_dots_14578
#define GDK_KEY_braille_dots_14578 0x10028d9
#endif
#ifndef GDK_KEY_exclamdown
#define GDK_KEY_exclamdown 0x0a1
#endif
#ifndef GDK_KEY_kana_NE
#define GDK_KEY_kana_NE 0x4c8
#endif
#ifndef GDK_KEY_ISO_Fast_Cursor_Up
#define GDK_KEY_ISO_Fast_Cursor_Up 0xfe2e
#endif
#ifndef GDK_KEY_Hangul_E
#define GDK_KEY_Hangul_E 0xec4
#endif
#ifndef GDK_KEY_plus
#define GDK_KEY_plus 0x02b
#endif
#ifndef GDK_KEY_braille_dots_12467
#define GDK_KEY_braille_dots_12467 0x100286b
#endif
#ifndef GDK_KEY_braille_dots_1234567
#define GDK_KEY_braille_dots_1234567 0x100287f
#endif
#ifndef GDK_KEY_Armenian_VO
#define GDK_KEY_Armenian_VO 0x1000548
#endif
#ifndef GDK_KEY_oacute
#define GDK_KEY_oacute 0x0f3
#endif
#ifndef GDK_KEY_LightBulb
#define GDK_KEY_LightBulb 0x1008ff35
#endif
#ifndef GDK_KEY_KP_Decimal
#define GDK_KEY_KP_Decimal 0xffae
#endif
#ifndef GDK_KEY_Terminal
#define GDK_KEY_Terminal 0x1008ff80
#endif
#ifndef GDK_KEY_etilde
#define GDK_KEY_etilde 0x1001ebd
#endif
#ifndef GDK_KEY_ISO_Center_Object
#define GDK_KEY_ISO_Center_Object 0xfe33
#endif
#ifndef GDK_KEY_kana_RA
#define GDK_KEY_kana_RA 0x4d7
#endif
#ifndef GDK_KEY_Arabic_khah
#define GDK_KEY_Arabic_khah 0x5ce
#endif
#ifndef GDK_KEY_Pointer_DblClick5
#define GDK_KEY_Pointer_DblClick5 0xfef3
#endif
#ifndef GDK_KEY_F34
#define GDK_KEY_F34 0xffdf
#endif
#ifndef GDK_KEY_Greek_alpha
#define GDK_KEY_Greek_alpha 0x7e1
#endif
#ifndef GDK_KEY_braille_dots_134578
#define GDK_KEY_braille_dots_134578 0x10028dd
#endif
#ifndef GDK_KEY_hebrew_chet
#define GDK_KEY_hebrew_chet 0xce7
#endif
#ifndef GDK_KEY_KP_9
#define GDK_KEY_KP_9 0xffb9
#endif
#ifndef GDK_KEY_braille_dots_1457
#define GDK_KEY_braille_dots_1457 0x1002859
#endif
#ifndef GDK_KEY_doubleacute
#define GDK_KEY_doubleacute 0x1bd
#endif
#ifndef GDK_KEY_Greek_iotaaccent
#define GDK_KEY_Greek_iotaaccent 0x7b4
#endif
#ifndef GDK_KEY_hebrew_mem
#define GDK_KEY_hebrew_mem 0xcee
#endif
#ifndef GDK_KEY_Cyrillic_YU
#define GDK_KEY_Cyrillic_YU 0x6e0
#endif
#ifndef GDK_KEY_F29
#define GDK_KEY_F29 0xffda
#endif
#ifndef GDK_KEY_ISO_Next_Group_Lock
#define GDK_KEY_ISO_Next_Group_Lock 0xfe09
#endif
#ifndef GDK_KEY_Ohornbelowdot
#define GDK_KEY_Ohornbelowdot 0x1001ee2
#endif
#ifndef GDK_KEY_PowerDown
#define GDK_KEY_PowerDown 0x1008ff21
#endif
#ifndef GDK_KEY_kana_a
#define GDK_KEY_kana_a 0x4a7
#endif
#ifndef GDK_KEY_braille_dots_2456
#define GDK_KEY_braille_dots_2456 0x100283a
#endif
#ifndef GDK_KEY_Arabic_hamza_below
#define GDK_KEY_Arabic_hamza_below 0x1000655
#endif
#ifndef GDK_KEY_ocircumflexhook
#define GDK_KEY_ocircumflexhook 0x1001ed5
#endif
#ifndef GDK_KEY_Acircumflexbelowdot
#define GDK_KEY_Acircumflexbelowdot 0x1001eac
#endif
#ifndef GDK_KEY_brokenbar
#define GDK_KEY_brokenbar 0x0a6
#endif
#ifndef GDK_KEY_ubreve
#define GDK_KEY_ubreve 0x2fd
#endif
#ifndef GDK_KEY_Ntilde
#define GDK_KEY_Ntilde 0x0d1
#endif
#ifndef GDK_KEY_Armenian_hyphen
#define GDK_KEY_Armenian_hyphen 0x100058a
#endif
#ifndef GDK_KEY_PesetaSign
#define GDK_KEY_PesetaSign 0x10020a7
#endif
#ifndef GDK_KEY_Ohornhook
#define GDK_KEY_Ohornhook 0x1001ede
#endif
#ifndef GDK_KEY_KbdBrightnessDown
#define GDK_KEY_KbdBrightnessDown 0x1008ff06
#endif
#ifndef GDK_KEY_E
#define GDK_KEY_E 0x045
#endif
#ifndef GDK_KEY_emspace
#define GDK_KEY_emspace 0xaa1
#endif
#ifndef GDK_KEY_Cyrillic_ka
#define GDK_KEY_Cyrillic_ka 0x6cb
#endif
#ifndef GDK_KEY_emptyset
#define GDK_KEY_emptyset 0x1002205
#endif
#ifndef GDK_KEY_Georgian_he
#define GDK_KEY_Georgian_he 0x10010f1
#endif
#ifndef GDK_KEY_fivesixths
#define GDK_KEY_fivesixths 0xab7
#endif
#ifndef GDK_KEY_ISO_Level3_Lock
#define GDK_KEY_ISO_Level3_Lock 0xfe05
#endif
#ifndef GDK_KEY_Hangul_RieulPhieuf
#define GDK_KEY_Hangul_RieulPhieuf 0xeaf
#endif
#ifndef GDK_KEY_Thai_oang
#define GDK_KEY_Thai_oang 0xdcd
#endif
#ifndef GDK_KEY_adiaeresis
#define GDK_KEY_adiaeresis 0x0e4
#endif
#ifndef GDK_KEY_ISO_Group_Latch
#define GDK_KEY_ISO_Group_Latch 0xfe06
#endif
#ifndef GDK_KEY_AudioForward
#define GDK_KEY_AudioForward 0x1008ff97
#endif
#ifndef GDK_KEY_Delete
#define GDK_KEY_Delete 0xffff
#endif
#ifndef GDK_KEY_ohornbelowdot
#define GDK_KEY_ohornbelowdot 0x1001ee3
#endif
#ifndef GDK_KEY_braille_dots_358
#define GDK_KEY_braille_dots_358 0x1002894
#endif
#ifndef GDK_KEY_Lbelowdot
#define GDK_KEY_Lbelowdot 0x1001e36
#endif
#ifndef GDK_KEY_braille_dots_23457
#define GDK_KEY_braille_dots_23457 0x100285e
#endif
#ifndef GDK_KEY_upshoe
#define GDK_KEY_upshoe 0xbc3
#endif
#ifndef GDK_KEY_Georgian_on
#define GDK_KEY_Georgian_on 0x10010dd
#endif
#ifndef GDK_KEY_kana_SU
#define GDK_KEY_kana_SU 0x4bd
#endif
#ifndef GDK_KEY_Gabovedot
#define GDK_KEY_Gabovedot 0x2d5
#endif
#ifndef GDK_KEY_braille_dots_136
#define GDK_KEY_braille_dots_136 0x1002825
#endif
#ifndef GDK_KEY_3270_KeyClick
#define GDK_KEY_3270_KeyClick 0xfd11
#endif
#ifndef GDK_KEY_R13
#define GDK_KEY_R13 0xffde
#endif
#ifndef GDK_KEY_ubelowdot
#define GDK_KEY_ubelowdot 0x1001ee5
#endif
#ifndef GDK_KEY_Armenian_KHE
#define GDK_KEY_Armenian_KHE 0x100053d
#endif
#ifndef GDK_KEY_hebrew_aleph
#define GDK_KEY_hebrew_aleph 0xce0
#endif
#ifndef GDK_KEY_braille_dots_1
#define GDK_KEY_braille_dots_1 0x1002801
#endif
#ifndef GDK_KEY_Zabovedot
#define GDK_KEY_Zabovedot 0x1af
#endif
#ifndef GDK_KEY_Cabovedot
#define GDK_KEY_Cabovedot 0x2c5
#endif
#ifndef GDK_KEY_ocircumflexbelowdot
#define GDK_KEY_ocircumflexbelowdot 0x1001ed9
#endif
#ifndef GDK_KEY_Utilde
#define GDK_KEY_Utilde 0x3dd
#endif
#ifndef GDK_KEY_ISO_Prev_Group_Lock
#define GDK_KEY_ISO_Prev_Group_Lock 0xfe0b
#endif
#ifndef GDK_KEY_Cyrillic_ES
#define GDK_KEY_Cyrillic_ES 0x6f3
#endif
#ifndef GDK_KEY_onehalf
#define GDK_KEY_onehalf 0x0bd
#endif
#ifndef GDK_KEY_braille_dots_2348
#define GDK_KEY_braille_dots_2348 0x100288e
#endif
#ifndef GDK_KEY_braille_dots_23478
#define GDK_KEY_braille_dots_23478 0x10028ce
#endif
#ifndef GDK_KEY_Arabic_fathatan
#define GDK_KEY_Arabic_fathatan 0x5eb
#endif
#ifndef GDK_KEY_X
#define GDK_KEY_X 0x058
#endif
#ifndef GDK_KEY_ZoomIn
#define GDK_KEY_ZoomIn 0x1008ff8b
#endif
#ifndef GDK_KEY_Hangul_YU
#define GDK_KEY_Hangul_YU 0xed0
#endif
#ifndef GDK_KEY_Serbian_tshe
#define GDK_KEY_Serbian_tshe 0x6ab
#endif
#ifndef GDK_KEY_ycircumflex
#define GDK_KEY_ycircumflex 0x1000177
#endif
#ifndef GDK_KEY_ccedilla
#define GDK_KEY_ccedilla 0x0e7
#endif
#ifndef GDK_KEY_figdash
#define GDK_KEY_figdash 0xabb
#endif
#ifndef GDK_KEY_Hangul_RieulMieum
#define GDK_KEY_Hangul_RieulMieum 0xeab
#endif
#ifndef GDK_KEY_Greek_PSI
#define GDK_KEY_Greek_PSI 0x7d8
#endif
#ifndef GDK_KEY_otilde
#define GDK_KEY_otilde 0x0f5
#endif
#ifndef GDK_KEY_Cyrillic_CHE
#define GDK_KEY_Cyrillic_CHE 0x6fe
#endif
#ifndef GDK_KEY_Arabic_lam
#define GDK_KEY_Arabic_lam 0x5e4
#endif
#ifndef GDK_KEY_abelowdot
#define GDK_KEY_abelowdot 0x1001ea1
#endif
#ifndef GDK_KEY_Menu
#define GDK_KEY_Menu 0xff67
#endif
#ifndef GDK_KEY_braille_dots_27
#define GDK_KEY_braille_dots_27 0x1002842
#endif
#ifndef GDK_KEY_Ncaron
#define GDK_KEY_Ncaron 0x1d2
#endif
#ifndef GDK_KEY_circle
#define GDK_KEY_circle 0xbcf
#endif
#ifndef GDK_KEY_Thai_lekchet
#define GDK_KEY_Thai_lekchet 0xdf7
#endif
#ifndef GDK_KEY_prolongedsound
#define GDK_KEY_prolongedsound 0x4b0
#endif
#ifndef GDK_KEY_ISO_Continuous_Underline
#define GDK_KEY_ISO_Continuous_Underline 0xfe30
#endif
#ifndef GDK_KEY_ebelowdot
#define GDK_KEY_ebelowdot 0x1001eb9
#endif
#ifndef GDK_KEY_Greek_psi
#define GDK_KEY_Greek_psi 0x7f8
#endif
#ifndef GDK_KEY_Switch_VT_3
#define GDK_KEY_Switch_VT_3 0x1008fe03
#endif
#ifndef GDK_KEY_Xabovedot
#define GDK_KEY_Xabovedot 0x1001e8a
#endif
#ifndef GDK_KEY_braille_dots_2568
#define GDK_KEY_braille_dots_2568 0x10028b2
#endif
#ifndef GDK_KEY_Serbian_DZE
#define GDK_KEY_Serbian_DZE 0x6bf
#endif
#ifndef GDK_KEY_s
#define GDK_KEY_s 0x073
#endif
#ifndef GDK_KEY_cr
#define GDK_KEY_cr 0x9e4
#endif
#ifndef GDK_KEY_Hangul_J_Cieuc
#define GDK_KEY_Hangul_J_Cieuc 0xeea
#endif
#ifndef GDK_KEY_KP_0
#define GDK_KEY_KP_0 0xffb0
#endif
#ifndef GDK_KEY_braille_dots_124567
#define GDK_KEY_braille_dots_124567 0x100287b
#endif
#ifndef GDK_KEY_Ukrainian_i
#define GDK_KEY_Ukrainian_i 0x6a6
#endif
#ifndef GDK_KEY_kana_KI
#define GDK_KEY_kana_KI 0x4b7
#endif
#ifndef GDK_KEY_Yellow
#define GDK_KEY_Yellow 0x1008ffa5
#endif
#ifndef GDK_KEY_ISO_Release_Margin_Left
#define GDK_KEY_ISO_Release_Margin_Left 0xfe29
#endif
#ifndef GDK_KEY_Hangul_J_PieubSios
#define GDK_KEY_Hangul_J_PieubSios 0xee5
#endif
#ifndef GDK_KEY_aacute
#define GDK_KEY_aacute 0x0e1
#endif
#ifndef GDK_KEY_Cyrillic_A
#define GDK_KEY_Cyrillic_A 0x6e1
#endif
#ifndef GDK_KEY_Search
#define GDK_KEY_Search 0x1008ff1b
#endif
#ifndef GDK_KEY_kana_SHI
#define GDK_KEY_kana_SHI 0x4bc
#endif
#ifndef GDK_KEY_braille_dots_367
#define GDK_KEY_braille_dots_367 0x1002864
#endif
#ifndef GDK_KEY_SlowKeys_Enable
#define GDK_KEY_SlowKeys_Enable 0xfe73
#endif
#ifndef GDK_KEY_3270_Quit
#define GDK_KEY_3270_Quit 0xfd09
#endif
#ifndef GDK_KEY_AudioPlay
#define GDK_KEY_AudioPlay 0x1008ff14
#endif
#ifndef GDK_KEY_braille_dots_13578
#define GDK_KEY_braille_dots_13578 0x10028d5
#endif
#ifndef GDK_KEY_braille_dots_357
#define GDK_KEY_braille_dots_357 0x1002854
#endif
#ifndef GDK_KEY_sabovedot
#define GDK_KEY_sabovedot 0x1001e61
#endif
#ifndef GDK_KEY_AE
#define GDK_KEY_AE 0x0c6
#endif
#ifndef GDK_KEY_hebrew_teth
#define GDK_KEY_hebrew_teth 0xce8
#endif
#ifndef GDK_KEY_Thai_yoyak
#define GDK_KEY_Thai_yoyak 0xdc2
#endif
#ifndef GDK_KEY_braille_dots_123478
#define GDK_KEY_braille_dots_123478 0x10028cf
#endif
#ifndef GDK_KEY_kana_tu
#define GDK_KEY_kana_tu 0x4af
#endif
#ifndef GDK_KEY_Eogonek
#define GDK_KEY_Eogonek 0x1ca
#endif
#ifndef GDK_KEY_braille_dots_246
#define GDK_KEY_braille_dots_246 0x100282a
#endif
#ifndef GDK_KEY_iTouch
#define GDK_KEY_iTouch 0x1008ff60
#endif
#ifndef GDK_KEY_dead_stroke
#define GDK_KEY_dead_stroke 0xfe63
#endif
#ifndef GDK_KEY_MailForward
#define GDK_KEY_MailForward 0x1008ff90
#endif
#ifndef GDK_KEY_F1
#define GDK_KEY_F1 0xffbe
#endif
#ifndef GDK_KEY_ccaron
#define GDK_KEY_ccaron 0x1e8
#endif
#ifndef GDK_KEY_AddFavorite
#define GDK_KEY_AddFavorite 0x1008ff39
#endif
#ifndef GDK_KEY_Arabic_ra
#define GDK_KEY_Arabic_ra 0x5d1
#endif
#ifndef GDK_KEY_lstroke
#define GDK_KEY_lstroke 0x1b3
#endif
#ifndef GDK_KEY_Scroll_Lock
#define GDK_KEY_Scroll_Lock 0xff14
#endif
#ifndef GDK_KEY_registered
#define GDK_KEY_registered 0x0ae
#endif
#ifndef GDK_KEY_Armenian_ayb
#define GDK_KEY_Armenian_ayb 0x1000561
#endif
#ifndef GDK_KEY_Cyrillic_EL
#define GDK_KEY_Cyrillic_EL 0x6ec
#endif
#ifndef GDK_KEY_KP_F4
#define GDK_KEY_KP_F4 0xff94
#endif
#ifndef GDK_KEY_trademark
#define GDK_KEY_trademark 0xac9
#endif
#ifndef GDK_KEY_dead_I
#define GDK_KEY_dead_I 0xfe85
#endif
#ifndef GDK_KEY_Hangul_Jeonja
#define GDK_KEY_Hangul_Jeonja 0xff38
#endif
#ifndef GDK_KEY_Ukranian_i
#define GDK_KEY_Ukranian_i 0x6a6
#endif
#ifndef GDK_KEY_Documents
#define GDK_KEY_Documents 0x1008ff5b
#endif
#ifndef GDK_KEY_Thai_khokhwai
#define GDK_KEY_Thai_khokhwai 0xda4
#endif
#ifndef GDK_KEY_kana_fullstop
#define GDK_KEY_kana_fullstop 0x4a1
#endif
#ifndef GDK_KEY_Hangul_J_RieulHieuh
#define GDK_KEY_Hangul_J_RieulHieuh 0xee2
#endif
#ifndef GDK_KEY_Armenian_separation_mark
#define GDK_KEY_Armenian_separation_mark 0x100055d
#endif
#ifndef GDK_KEY_Thai_nonen
#define GDK_KEY_Thai_nonen 0xdb3
#endif
#ifndef GDK_KEY_Lcedilla
#define GDK_KEY_Lcedilla 0x3a6
#endif
#ifndef GDK_KEY_Georgian_san
#define GDK_KEY_Georgian_san 0x10010e1
#endif
#ifndef GDK_KEY_Arabic_tcheh
#define GDK_KEY_Arabic_tcheh 0x1000686
#endif
#ifndef GDK_KEY_threequarters
#define GDK_KEY_threequarters 0x0be
#endif
#ifndef GDK_KEY_Pointer_DfltBtnPrev
#define GDK_KEY_Pointer_DfltBtnPrev 0xfefc
#endif
#ifndef GDK_KEY_braille_dots_3567
#define GDK_KEY_braille_dots_3567 0x1002874
#endif
#ifndef GDK_KEY_Bluetooth
#define GDK_KEY_Bluetooth 0x1008ff94
#endif
#ifndef GDK_KEY_Print
#define GDK_KEY_Print 0xff61
#endif
#ifndef GDK_KEY_Armenian_khe
#define GDK_KEY_Armenian_khe 0x100056d
#endif
#ifndef GDK_KEY_braille_dots_124
#define GDK_KEY_braille_dots_124 0x100280b
#endif
#ifndef GDK_KEY_braille_dots_23
#define GDK_KEY_braille_dots_23 0x1002806
#endif
#ifndef GDK_KEY_braille_dots_48
#define GDK_KEY_braille_dots_48 0x1002888
#endif
#ifndef GDK_KEY_F6
#define GDK_KEY_F6 0xffc3
#endif
#ifndef GDK_KEY_Thai_sosala
#define GDK_KEY_Thai_sosala 0xdc8
#endif
#ifndef GDK_KEY_braille_dots_347
#define GDK_KEY_braille_dots_347 0x100284c
#endif
#ifndef GDK_KEY_r
#define GDK_KEY_r 0x072
#endif
#ifndef GDK_KEY_KP_3
#define GDK_KEY_KP_3 0xffb3
#endif
#ifndef GDK_KEY_uhook
#define GDK_KEY_uhook 0x1001ee7
#endif
#ifndef GDK_KEY_acircumflexbelowdot
#define GDK_KEY_acircumflexbelowdot 0x1001ead
#endif
#ifndef GDK_KEY_UWB
#define GDK_KEY_UWB 0x1008ff96
#endif
#ifndef GDK_KEY_Hangul_WAE
#define GDK_KEY_Hangul_WAE 0xec9
#endif
#ifndef GDK_KEY_endash
#define GDK_KEY_endash 0xaaa
#endif
#ifndef GDK_KEY_idotless
#define GDK_KEY_idotless 0x2b9
#endif
#ifndef GDK_KEY_icircumflex
#define GDK_KEY_icircumflex 0x0ee
#endif
#ifndef GDK_KEY_Shift_L
#define GDK_KEY_Shift_L 0xffe1
#endif
#ifndef GDK_KEY_kana_ya
#define GDK_KEY_kana_ya 0x4ac
#endif
#ifndef GDK_KEY_braille_dots_125678
#define GDK_KEY_braille_dots_125678 0x10028f3
#endif
#ifndef GDK_KEY_jot
#define GDK_KEY_jot 0xbca
#endif
#ifndef GDK_KEY_Cyrillic_be
#define GDK_KEY_Cyrillic_be 0x6c2
#endif
#ifndef GDK_KEY_KP_Delete
#define GDK_KEY_KP_Delete 0xff9f
#endif
#ifndef GDK_KEY_Hyper_L
#define GDK_KEY_Hyper_L 0xffed
#endif
#ifndef GDK_KEY_0
#define GDK_KEY_0 0x030
#endif
#ifndef GDK_KEY_Georgian_phar
#define GDK_KEY_Georgian_phar 0x10010e4
#endif
#ifndef GDK_KEY_ohook
#define GDK_KEY_ohook 0x1001ecf
#endif
#ifndef GDK_KEY_dead_dasia
#define GDK_KEY_dead_dasia 0xfe65
#endif
#ifndef GDK_KEY_MonBrightnessUp
#define GDK_KEY_MonBrightnessUp 0x1008ff02
#endif
#ifndef GDK_KEY_dead_caron
#define GDK_KEY_dead_caron 0xfe5a
#endif
#ifndef GDK_KEY_dead_iota
#define GDK_KEY_dead_iota 0xfe5d
#endif
#ifndef GDK_KEY_braille_dots_5678
#define GDK_KEY_braille_dots_5678 0x10028f0
#endif
#ifndef GDK_KEY_braille_dots_4578
#define GDK_KEY_braille_dots_4578 0x10028d8
#endif
#ifndef GDK_KEY_Armenian_ken
#define GDK_KEY_Armenian_ken 0x100056f
#endif
#ifndef GDK_KEY_braille_dots_78
#define GDK_KEY_braille_dots_78 0x10028c0
#endif
#ifndef GDK_KEY_braille_dots_156
#define GDK_KEY_braille_dots_156 0x1002831
#endif
#ifndef GDK_KEY_Cyrillic_schwa
#define GDK_KEY_Cyrillic_schwa 0x10004d9
#endif
#ifndef GDK_KEY_Pointer_Drag4
#define GDK_KEY_Pointer_Drag4 0xfef8
#endif
#ifndef GDK_KEY_KP_Home
#define GDK_KEY_KP_Home 0xff95
#endif
#ifndef GDK_KEY_onefifth
#define GDK_KEY_onefifth 0xab2
#endif
#ifndef GDK_KEY_rcaron
#define GDK_KEY_rcaron 0x1f8
#endif
#ifndef GDK_KEY_Armenian_TYUN
#define GDK_KEY_Armenian_TYUN 0x100054f
#endif
#ifndef GDK_KEY_Thai_maitaikhu
#define GDK_KEY_Thai_maitaikhu 0xde7
#endif
#ifndef GDK_KEY_braille_dots_2367
#define GDK_KEY_braille_dots_2367 0x1002866
#endif
#ifndef GDK_KEY_emopencircle
#define GDK_KEY_emopencircle 0xace
#endif
#ifndef GDK_KEY_kana_E
#define GDK_KEY_kana_E 0x4b4
#endif
#ifndef GDK_KEY_Cyrillic_che_vertstroke
#define GDK_KEY_Cyrillic_che_vertstroke 0x10004b9
#endif
#ifndef GDK_KEY_kana_MU
#define GDK_KEY_kana_MU 0x4d1
#endif
#ifndef GDK_KEY_kana_O
#define GDK_KEY_kana_O 0x4b5
#endif
#ifndef GDK_KEY_Pointer_Drag3
#define GDK_KEY_Pointer_Drag3 0xfef7
#endif
#ifndef GDK_KEY_ocircumflextilde
#define GDK_KEY_ocircumflextilde 0x1001ed7
#endif
#ifndef GDK_KEY_Thai_honokhuk
#define GDK_KEY_Thai_honokhuk 0xdce
#endif
#ifndef GDK_KEY_Tcedilla
#define GDK_KEY_Tcedilla 0x1de
#endif
#ifndef GDK_KEY_Sleep
#define GDK_KEY_Sleep 0x1008ff2f
#endif
#ifndef GDK_KEY_Cyrillic_pe
#define GDK_KEY_Cyrillic_pe 0x6d0
#endif
#ifndef GDK_KEY_ISO_Release_Margin_Right
#define GDK_KEY_ISO_Release_Margin_Right 0xfe2a
#endif
#ifndef GDK_KEY_braille_dots_578
#define GDK_KEY_braille_dots_578 0x10028d0
#endif
#ifndef GDK_KEY_logicalor
#define GDK_KEY_logicalor 0x8df
#endif
#ifndef GDK_KEY_Undo
#define GDK_KEY_Undo 0xff65
#endif
#ifndef GDK_KEY_Uhorntilde
#define GDK_KEY_Uhorntilde 0x1001eee
#endif
#ifndef GDK_KEY_section
#define GDK_KEY_section 0x0a7
#endif
#ifndef GDK_KEY_eabovedot
#define GDK_KEY_eabovedot 0x3ec
#endif
#ifndef GDK_KEY_Launch3
#define GDK_KEY_Launch3 0x1008ff43
#endif
#ifndef GDK_KEY_D
#define GDK_KEY_D 0x044
#endif
#ifndef GDK_KEY_Armenian_TSO
#define GDK_KEY_Armenian_TSO 0x1000551
#endif
#ifndef GDK_KEY_Armenian_re
#define GDK_KEY_Armenian_re 0x1000580
#endif
#ifndef GDK_KEY_Cyrillic_TSE
#define GDK_KEY_Cyrillic_TSE 0x6e3
#endif
#ifndef GDK_KEY_kana_TE
#define GDK_KEY_kana_TE 0x4c3
#endif
#ifndef GDK_KEY_Hangul_J_SsangKiyeog
#define GDK_KEY_Hangul_J_SsangKiyeog 0xed5
#endif
#ifndef GDK_KEY_Scaron
#define GDK_KEY_Scaron 0x1a9
#endif
#ifndef GDK_KEY_HotLinks
#define GDK_KEY_HotLinks 0x1008ff3a
#endif
#ifndef GDK_KEY_Zenkaku_Hankaku
#define GDK_KEY_Zenkaku_Hankaku 0xff2a
#endif
#ifndef GDK_KEY_ISO_Level5_Lock
#define GDK_KEY_ISO_Level5_Lock 0xfe13
#endif
#ifndef GDK_KEY_dead_small_schwa
#define GDK_KEY_dead_small_schwa 0xfe8a
#endif
#ifndef GDK_KEY_mu
#define GDK_KEY_mu 0x0b5
#endif
#ifndef GDK_KEY_kana_HO
#define GDK_KEY_kana_HO 0x4ce
#endif
#ifndef GDK_KEY_openrectbullet
#define GDK_KEY_openrectbullet 0xae2
#endif
#ifndef GDK_KEY_Arabic_tah
#define GDK_KEY_Arabic_tah 0x5d7
#endif
#ifndef GDK_KEY_Caps_Lock
#define GDK_KEY_Caps_Lock 0xffe5
#endif
#ifndef GDK_KEY_Acircumflexgrave
#define GDK_KEY_Acircumflexgrave 0x1001ea6
#endif
#ifndef GDK_KEY_Serbian_nje
#define GDK_KEY_Serbian_nje 0x6aa
#endif
#ifndef GDK_KEY_Wacute
#define GDK_KEY_Wacute 0x1001e82
#endif
#ifndef GDK_KEY_Hangul_KiyeogSios
#define GDK_KEY_Hangul_KiyeogSios 0xea3
#endif
#ifndef GDK_KEY_T
#define GDK_KEY_T 0x054
#endif
#ifndef GDK_KEY_Book
#define GDK_KEY_Book 0x1008ff52
#endif
#ifndef GDK_KEY_braille_dots_1357
#define GDK_KEY_braille_dots_1357 0x1002855
#endif
#ifndef GDK_KEY_Cyrillic_ze
#define GDK_KEY_Cyrillic_ze 0x6da
#endif
#ifndef GDK_KEY_braille_dots_1368
#define GDK_KEY_braille_dots_1368 0x10028a5
#endif
#ifndef GDK_KEY_Green
#define GDK_KEY_Green 0x1008ffa4
#endif
#ifndef GDK_KEY_braille_dots_1356
#define GDK_KEY_braille_dots_1356 0x1002835
#endif
#ifndef GDK_KEY_Ocircumflex
#define GDK_KEY_Ocircumflex 0x0d4
#endif
#ifndef GDK_KEY_Armenian_O
#define GDK_KEY_Armenian_O 0x1000555
#endif
#ifndef GDK_KEY_Hangul_A
#define GDK_KEY_Hangul_A 0xebf
#endif
#ifndef GDK_KEY_Terminate_Server
#define GDK_KEY_Terminate_Server 0xfed5
#endif
#ifndef GDK_KEY_Greek_OMICRONaccent
#define GDK_KEY_Greek_OMICRONaccent 0x7a7
#endif
#ifndef GDK_KEY_Cyrillic_te
#define GDK_KEY_Cyrillic_te 0x6d4
#endif
#ifndef GDK_KEY_filledlefttribullet
#define GDK_KEY_filledlefttribullet 0xadc
#endif
#ifndef GDK_KEY_Arabic_theh
#define GDK_KEY_Arabic_theh 0x5cb
#endif
#ifndef GDK_KEY_Arabic_hamzaonyeh
#define GDK_KEY_Arabic_hamzaonyeh 0x5c6
#endif
#ifndef GDK_KEY_F32
#define GDK_KEY_F32 0xffdd
#endif
#ifndef GDK_KEY_ModeLock
#define GDK_KEY_ModeLock 0x1008ff01
#endif
#ifndef GDK_KEY_braille_dots_4567
#define GDK_KEY_braille_dots_4567 0x1002878
#endif
#ifndef GDK_KEY_3270_FieldMark
#define GDK_KEY_3270_FieldMark 0xfd02
#endif
#ifndef GDK_KEY_AudioRandomPlay
#define GDK_KEY_AudioRandomPlay 0x1008ff99
#endif
#ifndef GDK_KEY_Iogonek
#define GDK_KEY_Iogonek 0x3c7
#endif
#ifndef GDK_KEY_BounceKeys_Enable
#define GDK_KEY_BounceKeys_Enable 0xfe74
#endif
#ifndef GDK_KEY_Switch_VT_4
#define GDK_KEY_Switch_VT_4 0x1008fe04
#endif
#ifndef GDK_KEY_downtack
#define GDK_KEY_downtack 0xbc2
#endif
#ifndef GDK_KEY_RockerUp
#define GDK_KEY_RockerUp 0x1008ff23
#endif
#ifndef GDK_KEY_braille_dots_147
#define GDK_KEY_braille_dots_147 0x1002849
#endif
#ifndef GDK_KEY_Cyrillic_ZHE_descender
#define GDK_KEY_Cyrillic_ZHE_descender 0x1000496
#endif
#ifndef GDK_KEY_Cyrillic_tse
#define GDK_KEY_Cyrillic_tse 0x6c3
#endif
#ifndef GDK_KEY_Arabic_ghain
#define GDK_KEY_Arabic_ghain 0x5da
#endif
#ifndef GDK_KEY_braille_dots_13467
#define GDK_KEY_braille_dots_13467 0x100286d
#endif
#ifndef GDK_KEY_Ecircumflex
#define GDK_KEY_Ecircumflex 0x0ca
#endif
#ifndef GDK_KEY_Armenian_DA
#define GDK_KEY_Armenian_DA 0x1000534
#endif
#ifndef GDK_KEY_Uhook
#define GDK_KEY_Uhook 0x1001ee6
#endif
#ifndef GDK_KEY_Execute
#define GDK_KEY_Execute 0xff62
#endif
#ifndef GDK_KEY_cedilla
#define GDK_KEY_cedilla 0x0b8
#endif
#ifndef GDK_KEY_hairspace
#define GDK_KEY_hairspace 0xaa8
#endif
#ifndef GDK_KEY_hstroke
#define GDK_KEY_hstroke 0x2b1
#endif
#ifndef GDK_KEY_dead_currency
#define GDK_KEY_dead_currency 0xfe6f
#endif
#ifndef GDK_KEY_ScrollUp
#define GDK_KEY_ScrollUp 0x1008ff78
#endif
#ifndef GDK_KEY_Farsi_1
#define GDK_KEY_Farsi_1 0x10006f1
#endif
#ifndef GDK_KEY_Armenian_DZA
#define GDK_KEY_Armenian_DZA 0x1000541
#endif
#ifndef GDK_KEY_Greek_UPSILON
#define GDK_KEY_Greek_UPSILON 0x7d5
#endif
#ifndef GDK_KEY_3270_Test
#define GDK_KEY_3270_Test 0xfd0d
#endif
#ifndef GDK_KEY_hebrew_yod
#define GDK_KEY_hebrew_yod 0xce9
#endif
#ifndef GDK_KEY_aring
#define GDK_KEY_aring 0x0e5
#endif
#ifndef GDK_KEY_omacron
#define GDK_KEY_omacron 0x3f2
#endif
#ifndef GDK_KEY_Eabovedot
#define GDK_KEY_Eabovedot 0x3cc
#endif
#ifndef GDK_KEY_acircumflexhook
#define GDK_KEY_acircumflexhook 0x1001ea9
#endif
#ifndef GDK_KEY_ocaron
#define GDK_KEY_ocaron 0x10001d2
#endif
#ifndef GDK_KEY_Cyrillic_yeru
#define GDK_KEY_Cyrillic_yeru 0x6d9
#endif
#ifndef GDK_KEY_Georgian_jhan
#define GDK_KEY_Georgian_jhan 0x10010ef
#endif
#ifndef GDK_KEY_9
#define GDK_KEY_9 0x039
#endif
#ifndef GDK_KEY_Arabic_thal
#define GDK_KEY_Arabic_thal 0x5d0
#endif
#ifndef GDK_KEY_Pointer_DblClick1
#define GDK_KEY_Pointer_DblClick1 0xfeef
#endif
#ifndef GDK_KEY_F25
#define GDK_KEY_F25 0xffd6
#endif
#ifndef GDK_KEY_abreveacute
#define GDK_KEY_abreveacute 0x1001eaf
#endif
#ifndef GDK_KEY_braille_dots_236
#define GDK_KEY_braille_dots_236 0x1002826
#endif
#ifndef GDK_KEY_Odoubleacute
#define GDK_KEY_Odoubleacute 0x1d5
#endif
#ifndef GDK_KEY_ISO_First_Group_Lock
#define GDK_KEY_ISO_First_Group_Lock 0xfe0d
#endif
#ifndef GDK_KEY_f
#define GDK_KEY_f 0x066
#endif
#ifndef GDK_KEY_Serbian_JE
#define GDK_KEY_Serbian_JE 0x6b8
#endif
#ifndef GDK_KEY_division
#define GDK_KEY_division 0x0f7
#endif
#ifndef GDK_KEY_i
#define GDK_KEY_i 0x069
#endif
#ifndef GDK_KEY_Arabic_waw
#define GDK_KEY_Arabic_waw 0x5e8
#endif
#ifndef GDK_KEY_topleftsqbracket
#define GDK_KEY_topleftsqbracket 0x8a7
#endif
#ifndef GDK_KEY_question
#define GDK_KEY_question 0x03f
#endif
#ifndef GDK_KEY_Hangul_AE
#define GDK_KEY_Hangul_AE 0xec0
#endif
#ifndef GDK_KEY_Thai_lakkhangyao
#define GDK_KEY_Thai_lakkhangyao 0xde5
#endif
#ifndef GDK_KEY_o
#define GDK_KEY_o 0x06f
#endif
#ifndef GDK_KEY_Armenian_RE
#define GDK_KEY_Armenian_RE 0x1000550
#endif
#ifndef GDK_KEY_CD
#define GDK_KEY_CD 0x1008ff53
#endif
#ifndef GDK_KEY_KP_Space
#define GDK_KEY_KP_Space 0xff80
#endif
#ifndef GDK_KEY_3270_Enter
#define GDK_KEY_3270_Enter 0xfd1e
#endif
#ifndef GDK_KEY_botleftsummation
#define GDK_KEY_botleftsummation 0x8b2
#endif
#ifndef GDK_KEY_Cyrillic_SCHWA
#define GDK_KEY_Cyrillic_SCHWA 0x10004d8
#endif
#ifndef GDK_KEY_SingleCandidate
#define GDK_KEY_SingleCandidate 0xff3c
#endif
#ifndef GDK_KEY_doublelowquotemark
#define GDK_KEY_doublelowquotemark 0xafe
#endif
#ifndef GDK_KEY_kana_o
#define GDK_KEY_kana_o 0x4ab
#endif
#ifndef GDK_KEY_Stop
#define GDK_KEY_Stop 0x1008ff28
#endif
#ifndef GDK_KEY_grave
#define GDK_KEY_grave 0x060
#endif
#ifndef GDK_KEY_kana_RU
#define GDK_KEY_kana_RU 0x4d9
#endif
#ifndef GDK_KEY_Greek_THETA
#define GDK_KEY_Greek_THETA 0x7c8
#endif
#ifndef GDK_KEY_Armenian_full_stop
#define GDK_KEY_Armenian_full_stop 0x1000589
#endif
#ifndef GDK_KEY_braille_dots_12356
#define GDK_KEY_braille_dots_12356 0x1002837
#endif
#ifndef GDK_KEY_iacute
#define GDK_KEY_iacute 0x0ed
#endif
#ifndef GDK_KEY_Standby
#define GDK_KEY_Standby 0x1008ff10
#endif
#ifndef GDK_KEY_braille_dots_678
#define GDK_KEY_braille_dots_678 0x10028e0
#endif
#ifndef GDK_KEY_F12
#define GDK_KEY_F12 0xffc9
#endif
#ifndef GDK_KEY_Cyrillic_u_straight
#define GDK_KEY_Cyrillic_u_straight 0x10004af
#endif
#ifndef GDK_KEY_KP_Up
#define GDK_KEY_KP_Up 0xff97
#endif
#ifndef GDK_KEY_KP_Down
#define GDK_KEY_KP_Down 0xff99
#endif
#ifndef GDK_KEY_Cyrillic_sha
#define GDK_KEY_Cyrillic_sha 0x6db
#endif
#ifndef GDK_KEY_braille_dot_8
#define GDK_KEY_braille_dot_8 0xfff8
#endif
#ifndef GDK_KEY_Cyrillic_SHORTI
#define GDK_KEY_Cyrillic_SHORTI 0x6ea
#endif
#ifndef GDK_KEY_underbar
#define GDK_KEY_underbar 0xbc6
#endif
#ifndef GDK_KEY_filledrectbullet
#define GDK_KEY_filledrectbullet 0xadb
#endif
#ifndef GDK_KEY_Cyrillic_ka_vertstroke
#define GDK_KEY_Cyrillic_ka_vertstroke 0x100049d
#endif
#ifndef GDK_KEY_kana_KE
#define GDK_KEY_kana_KE 0x4b9
#endif
#ifndef GDK_KEY_braille_dots_1346
#define GDK_KEY_braille_dots_1346 0x100282d
#endif
#ifndef GDK_KEY_Thai_maitho
#define GDK_KEY_Thai_maitho 0xde9
#endif
#ifndef GDK_KEY_ygrave
#define GDK_KEY_ygrave 0x1001ef3
#endif
#ifndef GDK_KEY_Pointer_UpRight
#define GDK_KEY_Pointer_UpRight 0xfee5
#endif
#ifndef GDK_KEY_Ibreve
#define GDK_KEY_Ibreve 0x100012c
#endif
#ifndef GDK_KEY_braille_dots_3458
#define GDK_KEY_braille_dots_3458 0x100289c
#endif
#ifndef GDK_KEY_Hangul_SsangKiyeog
#define GDK_KEY_Hangul_SsangKiyeog 0xea2
#endif
#ifndef GDK_KEY_Armenian_ZHE
#define GDK_KEY_Armenian_ZHE 0x100053a
#endif
#ifndef GDK_KEY_Armenian_yentamna
#define GDK_KEY_Armenian_yentamna 0x100058a
#endif
#ifndef GDK_KEY_bar
#define GDK_KEY_bar 0x07c
#endif
#ifndef GDK_KEY_Thai_loling
#define GDK_KEY_Thai_loling 0xdc5
#endif
#ifndef GDK_KEY_kana_NA
#define GDK_KEY_kana_NA 0x4c5
#endif
#ifndef GDK_KEY_quad
#define GDK_KEY_quad 0xbcc
#endif
#ifndef GDK_KEY_braille_dots_1235
#define GDK_KEY_braille_dots_1235 0x1002817
#endif
#ifndef GDK_KEY_Thai_ngongu
#define GDK_KEY_Thai_ngongu 0xda7
#endif
#ifndef GDK_KEY_Thai_saraam
#define GDK_KEY_Thai_saraam 0xdd3
#endif
#ifndef GDK_KEY_KP_4
#define GDK_KEY_KP_4 0xffb4
#endif
#ifndef GDK_KEY_I
#define GDK_KEY_I 0x049
#endif
#ifndef GDK_KEY_hebrew_beth
#define GDK_KEY_hebrew_beth 0xce1
#endif
#ifndef GDK_KEY_kana_WO
#define GDK_KEY_kana_WO 0x4a6
#endif
#ifndef GDK_KEY_braille_dots_5
#define GDK_KEY_braille_dots_5 0x1002810
#endif
#ifndef GDK_KEY_R4
#define GDK_KEY_R4 0xffd5
#endif
#ifndef GDK_KEY_Pointer_Button1
#define GDK_KEY_Pointer_Button1 0xfee9
#endif
#ifndef GDK_KEY_Ubelowdot
#define GDK_KEY_Ubelowdot 0x1001ee4
#endif
#ifndef GDK_KEY_udoubleacute
#define GDK_KEY_udoubleacute 0x1fb
#endif
#ifndef GDK_KEY_braille_dots_12458
#define GDK_KEY_braille_dots_12458 0x100289b
#endif
#ifndef GDK_KEY_ISO_Group_Shift
#define GDK_KEY_ISO_Group_Shift 0xff7e
#endif
#ifndef GDK_KEY_Armenian_VYUN
#define GDK_KEY_Armenian_VYUN 0x1000552
#endif
#ifndef GDK_KEY_L7
#define GDK_KEY_L7 0xffce
#endif
#ifndef GDK_KEY_braille_dots_2345678
#define GDK_KEY_braille_dots_2345678 0x10028fe
#endif
#ifndef GDK_KEY_Hangul_SunkyeongeumPieub
#define GDK_KEY_Hangul_SunkyeongeumPieub 0xef1
#endif
#ifndef GDK_KEY_A
#define GDK_KEY_A 0x041
#endif
#ifndef GDK_KEY_dead_belowmacron
#define GDK_KEY_dead_belowmacron 0xfe68
#endif
#ifndef GDK_KEY_dead_belowdiaeresis
#define GDK_KEY_dead_belowdiaeresis 0xfe6c
#endif
#ifndef GDK_KEY_Thai_sarauu
#define GDK_KEY_Thai_sarauu 0xdd9
#endif
#ifndef GDK_KEY_Thai_fofa
#define GDK_KEY_Thai_fofa 0xdbd
#endif
#ifndef GDK_KEY_braille_dot_6
#define GDK_KEY_braille_dot_6 0xfff6
#endif
#ifndef GDK_KEY_LaunchD
#define GDK_KEY_LaunchD 0x1008ff4d
#endif
#ifndef GDK_KEY_Control_R
#define GDK_KEY_Control_R 0xffe4
#endif
#ifndef GDK_KEY_Abrevebelowdot
#define GDK_KEY_Abrevebelowdot 0x1001eb6
#endif
#ifndef GDK_KEY_AudioPrev
#define GDK_KEY_AudioPrev 0x1008ff16
#endif
#ifndef GDK_KEY_braille_dots_35
#define GDK_KEY_braille_dots_35 0x1002814
#endif
#ifndef GDK_KEY_Paste
#define GDK_KEY_Paste 0x1008ff6d
#endif
#ifndef GDK_KEY_ampersand
#define GDK_KEY_ampersand 0x026
#endif
#ifndef GDK_KEY_Cyrillic_em
#define GDK_KEY_Cyrillic_em 0x6cd
#endif
#ifndef GDK_KEY_Jcircumflex
#define GDK_KEY_Jcircumflex 0x2ac
#endif
#ifndef GDK_KEY_kana_HU
#define GDK_KEY_kana_HU 0x4cc
#endif
#ifndef GDK_KEY_ibelowdot
#define GDK_KEY_ibelowdot 0x1001ecb
#endif
#ifndef GDK_KEY_braille_dots_3578
#define GDK_KEY_braille_dots_3578 0x10028d4
#endif
#ifndef GDK_KEY_Armenian_e
#define GDK_KEY_Armenian_e 0x1000567
#endif
#ifndef GDK_KEY_Hstroke
#define GDK_KEY_Hstroke 0x2a1
#endif
#ifndef GDK_KEY_Finance
#define GDK_KEY_Finance 0x1008ff3c
#endif
#ifndef GDK_KEY_Armenian_PE
#define GDK_KEY_Armenian_PE 0x100054a
#endif
#ifndef GDK_KEY_Greek_IOTAdiaeresis
#define GDK_KEY_Greek_IOTAdiaeresis 0x7a5
#endif
#ifndef GDK_KEY_Odiaeresis
#define GDK_KEY_Odiaeresis 0x0d6
#endif
#ifndef GDK_KEY_Thai_maihanakat_maitho
#define GDK_KEY_Thai_maihanakat_maitho 0xdde
#endif
#ifndef GDK_KEY_Farsi_yeh
#define GDK_KEY_Farsi_yeh 0x10006cc
#endif
#ifndef GDK_KEY_Armenian_fe
#define GDK_KEY_Armenian_fe 0x1000586
#endif
#ifndef GDK_KEY_ugrave
#define GDK_KEY_ugrave 0x0f9
#endif
#ifndef GDK_KEY_Arabic_hamza_above
#define GDK_KEY_Arabic_hamza_above 0x1000654
#endif
#ifndef GDK_KEY_Greek_tau
#define GDK_KEY_Greek_tau 0x7f4
#endif
#ifndef GDK_KEY_View
#define GDK_KEY_View 0x1008ffa1
#endif
#ifndef GDK_KEY_Xfer
#define GDK_KEY_Xfer 0x1008ff8a
#endif
#ifndef GDK_KEY_Armenian_E
#define GDK_KEY_Armenian_E 0x1000537
#endif
#ifndef GDK_KEY_dead_hook
#define GDK_KEY_dead_hook 0xfe61
#endif
#ifndef GDK_KEY_User2KB
#define GDK_KEY_User2KB 0x1008ff86
#endif
#ifndef GDK_KEY_dead_breve
#define GDK_KEY_dead_breve 0xfe55
#endif
#ifndef GDK_KEY_braille_dots_13468
#define GDK_KEY_braille_dots_13468 0x10028ad
#endif
#ifndef GDK_KEY_Armenian_nu
#define GDK_KEY_Armenian_nu 0x1000576
#endif
#ifndef GDK_KEY_Ediaeresis
#define GDK_KEY_Ediaeresis 0x0cb
#endif
#ifndef GDK_KEY_Cyrillic_zhe_descender
#define GDK_KEY_Cyrillic_zhe_descender 0x1000497
#endif
#ifndef GDK_KEY_Num_Lock
#define GDK_KEY_Num_Lock 0xff7f
#endif
#ifndef GDK_KEY_braille_dots_14
#define GDK_KEY_braille_dots_14 0x1002809
#endif
#ifndef GDK_KEY_z
#define GDK_KEY_z 0x07a
#endif
#ifndef GDK_KEY_checkmark
#define GDK_KEY_checkmark 0xaf3
#endif
#ifndef GDK_KEY_kana_KU
#define GDK_KEY_kana_KU 0x4b8
#endif
#ifndef GDK_KEY_sevensuperior
#define GDK_KEY_sevensuperior 0x1002077
#endif
#ifndef GDK_KEY_Arabic_hamza
#define GDK_KEY_Arabic_hamza 0x5c1
#endif
#ifndef GDK_KEY_AudioCycleTrack
#define GDK_KEY_AudioCycleTrack 0x1008ff9b
#endif
#ifndef GDK_KEY_3
#define GDK_KEY_3 0x033
#endif
#ifndef GDK_KEY_Ukranian_I
#define GDK_KEY_Ukranian_I 0x6b6
#endif
#ifndef GDK_KEY_Eacute
#define GDK_KEY_Eacute 0x0c9
#endif
#ifndef GDK_KEY_Thai_saraii
#define GDK_KEY_Thai_saraii 0xdd5
#endif
#ifndef GDK_KEY_EuroSign
#define GDK_KEY_EuroSign 0x20ac
#endif
#ifndef GDK_KEY_Launch2
#define GDK_KEY_Launch2 0x1008ff42
#endif
#ifndef GDK_KEY_Hangul_J_RieulTieut
#define GDK_KEY_Hangul_J_RieulTieut 0xee0
#endif
#ifndef GDK_KEY_macron
#define GDK_KEY_macron 0x0af
#endif
#ifndef GDK_KEY_Arabic_dad
#define GDK_KEY_Arabic_dad 0x5d6
#endif
#ifndef GDK_KEY_Zen_Koho
#define GDK_KEY_Zen_Koho 0xff3d
#endif
#ifndef GDK_KEY_Hangul_WI
#define GDK_KEY_Hangul_WI 0xecf
#endif
#ifndef GDK_KEY_R5
#define GDK_KEY_R5 0xffd6
#endif
#ifndef GDK_KEY_Hangul_Phieuf
#define GDK_KEY_Hangul_Phieuf 0xebd
#endif
#ifndef GDK_KEY_F18
#define GDK_KEY_F18 0xffcf
#endif
#ifndef GDK_KEY_Armenian_cha
#define GDK_KEY_Armenian_cha 0x1000579
#endif
#ifndef GDK_KEY_Hangul_Jamo
#define GDK_KEY_Hangul_Jamo 0xff35
#endif
#ifndef GDK_KEY_Option
#define GDK_KEY_Option 0x1008ff6c
#endif
#ifndef GDK_KEY_braille_dots_2468
#define GDK_KEY_braille_dots_2468 0x10028aa
#endif
#ifndef GDK_KEY_ScrollClick
#define GDK_KEY_ScrollClick 0x1008ff7a
#endif
#ifndef GDK_KEY_braille_dots_458
#define GDK_KEY_braille_dots_458 0x1002898
#endif
#ifndef GDK_KEY_Ohornacute
#define GDK_KEY_Ohornacute 0x1001eda
#endif
#ifndef GDK_KEY_braille_dots_125
#define GDK_KEY_braille_dots_125 0x1002813
#endif
#ifndef GDK_KEY_Armenian_ke
#define GDK_KEY_Armenian_ke 0x1000584
#endif
#ifndef GDK_KEY_F35
#define GDK_KEY_F35 0xffe0
#endif
#ifndef GDK_KEY_KP_Multiply
#define GDK_KEY_KP_Multiply 0xffaa
#endif
#ifndef GDK_KEY_KP_5
#define GDK_KEY_KP_5 0xffb5
#endif
#ifndef GDK_KEY_Ucircumflex
#define GDK_KEY_Ucircumflex 0x0db
#endif
#ifndef GDK_KEY_L4
#define GDK_KEY_L4 0xffcb
#endif
#ifndef GDK_KEY_KP_Equal
#define GDK_KEY_KP_Equal 0xffbd
#endif
#ifndef GDK_KEY_ISO_Lock
#define GDK_KEY_ISO_Lock 0xfe01
#endif
#ifndef GDK_KEY_braille_dots_24578
#define GDK_KEY_braille_dots_24578 0x10028da
#endif
#ifndef GDK_KEY_Ukranian_JE
#define GDK_KEY_Ukranian_JE 0x6b4
#endif
#ifndef GDK_KEY_Prev_VMode
#define GDK_KEY_Prev_VMode 0x1008fe23
#endif
#ifndef GDK_KEY_Cyrillic_en
#define GDK_KEY_Cyrillic_en 0x6ce
#endif
#ifndef GDK_KEY_kana_MA
#define GDK_KEY_kana_MA 0x4cf
#endif
#ifndef GDK_KEY_Cyrillic_nje
#define GDK_KEY_Cyrillic_nje 0x6aa
#endif
#ifndef GDK_KEY_signaturemark
#define GDK_KEY_signaturemark 0xaca
#endif
#ifndef GDK_KEY_uhornhook
#define GDK_KEY_uhornhook 0x1001eed
#endif
#ifndef GDK_KEY_Pointer_Button_Dflt
#define GDK_KEY_Pointer_Button_Dflt 0xfee8
#endif
#ifndef GDK_KEY_hebrew_doublelowline
#define GDK_KEY_hebrew_doublelowline 0xcdf
#endif
#ifndef GDK_KEY_hebrew_finalzadi
#define GDK_KEY_hebrew_finalzadi 0xcf5
#endif
#ifndef GDK_KEY_KP_Subtract
#define GDK_KEY_KP_Subtract 0xffad
#endif
#ifndef GDK_KEY_kana_A
#define GDK_KEY_kana_A 0x4b1
#endif
#ifndef GDK_KEY_Arabic_keheh
#define GDK_KEY_Arabic_keheh 0x10006a9
#endif
#ifndef GDK_KEY_Egrave
#define GDK_KEY_Egrave 0x0c8
#endif
#ifndef GDK_KEY_Greek_ZETA
#define GDK_KEY_Greek_ZETA 0x7c6
#endif
#ifndef GDK_KEY_diamond
#define GDK_KEY_diamond 0xaed
#endif
#ifndef GDK_KEY_twosuperior
#define GDK_KEY_twosuperior 0x0b2
#endif
#ifndef GDK_KEY_Georgian_ghan
#define GDK_KEY_Georgian_ghan 0x10010e6
#endif
#ifndef GDK_KEY_rightsinglequotemark
#define GDK_KEY_rightsinglequotemark 0xad1
#endif
#ifndef GDK_KEY_Hangul_J_Tieut
#define GDK_KEY_Hangul_J_Tieut 0xeec
#endif
#ifndef GDK_KEY_KP_Insert
#define GDK_KEY_KP_Insert 0xff9e
#endif
#ifndef GDK_KEY_Lacute
#define GDK_KEY_Lacute 0x1c5
#endif
#ifndef GDK_KEY_Thai_nikhahit
#define GDK_KEY_Thai_nikhahit 0xded
#endif
#ifndef GDK_KEY_Thai_thanthakhat
#define GDK_KEY_Thai_thanthakhat 0xdec
#endif
#ifndef GDK_KEY_F13
#define GDK_KEY_F13 0xffca
#endif
#ifndef GDK_KEY_Ecircumflexacute
#define GDK_KEY_Ecircumflexacute 0x1001ebe
#endif
#ifndef GDK_KEY_Armenian_AT
#define GDK_KEY_Armenian_AT 0x1000538
#endif
#ifndef GDK_KEY_braille_dots_14567
#define GDK_KEY_braille_dots_14567 0x1002879
#endif
#ifndef GDK_KEY_hebrew_finalzade
#define GDK_KEY_hebrew_finalzade 0xcf5
#endif
#ifndef GDK_KEY_Kcedilla
#define GDK_KEY_Kcedilla 0x3d3
#endif
#ifndef GDK_KEY_Cyrillic_I
#define GDK_KEY_Cyrillic_I 0x6e9
#endif
#ifndef GDK_KEY_braille_dots_157
#define GDK_KEY_braille_dots_157 0x1002851
#endif
#ifndef GDK_KEY_F2
#define GDK_KEY_F2 0xffbf
#endif
#ifndef GDK_KEY_Greek_UPSILONdieresis
#define GDK_KEY_Greek_UPSILONdieresis 0x7a9
#endif
#ifndef GDK_KEY_braille_dots_128
#define GDK_KEY_braille_dots_128 0x1002883
#endif
#ifndef GDK_KEY_braille_dots_2368
#define GDK_KEY_braille_dots_2368 0x10028a6
#endif
#ifndef GDK_KEY_wcircumflex
#define GDK_KEY_wcircumflex 0x1000175
#endif
#ifndef GDK_KEY_Georgian_qar
#define GDK_KEY_Georgian_qar 0x10010e7
#endif
#ifndef GDK_KEY_Cyrillic_SOFTSIGN
#define GDK_KEY_Cyrillic_SOFTSIGN 0x6f8
#endif
#ifndef GDK_KEY_nl
#define GDK_KEY_nl 0x9e8
#endif
#ifndef GDK_KEY_Hangul_J_Khieuq
#define GDK_KEY_Hangul_J_Khieuq 0xeeb
#endif
#ifndef GDK_KEY_imacron
#define GDK_KEY_imacron 0x3ef
#endif
#ifndef GDK_KEY_braille_dots_15678
#define GDK_KEY_braille_dots_15678 0x10028f1
#endif
#ifndef GDK_KEY_Super_R
#define GDK_KEY_Super_R 0xffec
#endif
#ifndef GDK_KEY_braille_dots_1238
#define GDK_KEY_braille_dots_1238 0x1002887
#endif
#ifndef GDK_KEY_Ocircumflextilde
#define GDK_KEY_Ocircumflextilde 0x1001ed6
#endif
#ifndef GDK_KEY_emopenrectangle
#define GDK_KEY_emopenrectangle 0xacf
#endif
#ifndef GDK_KEY_Hangul_EU
#define GDK_KEY_Hangul_EU 0xed1
#endif
#ifndef GDK_KEY_radical
#define GDK_KEY_radical 0x8d6
#endif
#ifndef GDK_KEY_Armenian_at
#define GDK_KEY_Armenian_at 0x1000568
#endif
#ifndef GDK_KEY_partialderivative
#define GDK_KEY_partialderivative 0x8ef
#endif
#ifndef GDK_KEY_filledrighttribullet
#define GDK_KEY_filledrighttribullet 0xadd
#endif
#ifndef GDK_KEY_braille_dots_2347
#define GDK_KEY_braille_dots_2347 0x100284e
#endif
#ifndef GDK_KEY_Thai_saraaa
#define GDK_KEY_Thai_saraaa 0xdd2
#endif
#ifndef GDK_KEY_braille_dots_47
#define GDK_KEY_braille_dots_47 0x1002848
#endif
#ifndef GDK_KEY_ohorngrave
#define GDK_KEY_ohorngrave 0x1001edd
#endif
#ifndef GDK_KEY_abrevetilde
#define GDK_KEY_abrevetilde 0x1001eb5
#endif
#ifndef GDK_KEY_Cyrillic_u
#define GDK_KEY_Cyrillic_u 0x6d5
#endif
#ifndef GDK_KEY_Hangul_NieunHieuh
#define GDK_KEY_Hangul_NieunHieuh 0xea6
#endif
#ifndef GDK_KEY_Fabovedot
#define GDK_KEY_Fabovedot 0x1001e1e
#endif
#ifndef GDK_KEY_containsas
#define GDK_KEY_containsas 0x100220b
#endif
#ifndef GDK_KEY_Uhorn
#define GDK_KEY_Uhorn 0x10001af
#endif
#ifndef GDK_KEY_Armenian_CHA
#define GDK_KEY_Armenian_CHA 0x1000549
#endif
#ifndef GDK_KEY_Cyrillic_U_straight
#define GDK_KEY_Cyrillic_U_straight 0x10004ae
#endif
#ifndef GDK_KEY_Cyrillic_CHE_descender
#define GDK_KEY_Cyrillic_CHE_descender 0x10004b6
#endif
#ifndef GDK_KEY_Hangul_SsangJieuj
#define GDK_KEY_Hangul_SsangJieuj 0xeb9
#endif
#ifndef GDK_KEY_kana_tsu
#define GDK_KEY_kana_tsu 0x4af
#endif
#ifndef GDK_KEY_dead_grave
#define GDK_KEY_dead_grave 0xfe50
#endif
#ifndef GDK_KEY_R12
#define GDK_KEY_R12 0xffdd
#endif
#ifndef GDK_KEY_braille_dots_123467
#define GDK_KEY_braille_dots_123467 0x100286f
#endif
#ifndef GDK_KEY_Greek_delta
#define GDK_KEY_Greek_delta 0x7e4
#endif
#ifndef GDK_KEY_Hangul_End
#define GDK_KEY_Hangul_End 0xff33
#endif
#ifndef GDK_KEY_Arabic_hamzaunderalef
#define GDK_KEY_Arabic_hamzaunderalef 0x5c5
#endif
#ifndef GDK_KEY_Switch_VT_8
#define GDK_KEY_Switch_VT_8 0x1008fe08
#endif
#ifndef GDK_KEY_Rcaron
#define GDK_KEY_Rcaron 0x1d8
#endif
#ifndef GDK_KEY_KP_End
#define GDK_KEY_KP_End 0xff9c
#endif
#ifndef GDK_KEY_braille_dots_26
#define GDK_KEY_braille_dots_26 0x1002822
#endif
#ifndef GDK_KEY_Hangul_J_Mieum
#define GDK_KEY_Hangul_J_Mieum 0xee3
#endif
#ifndef GDK_KEY_Obelowdot
#define GDK_KEY_Obelowdot 0x1001ecc
#endif
#ifndef GDK_KEY_5
#define GDK_KEY_5 0x035
#endif
#ifndef GDK_KEY_lcedilla
#define GDK_KEY_lcedilla 0x3b6
#endif
#ifndef GDK_KEY_ordfeminine
#define GDK_KEY_ordfeminine 0x0aa
#endif
#ifndef GDK_KEY_O
#define GDK_KEY_O 0x04f
#endif
#ifndef GDK_KEY_Farsi_6
#define GDK_KEY_Farsi_6 0x10006f6
#endif
#ifndef GDK_KEY_braille_dots_368
#define GDK_KEY_braille_dots_368 0x10028a4
#endif
#ifndef GDK_KEY_fivesubscript
#define GDK_KEY_fivesubscript 0x1002085
#endif
#ifndef GDK_KEY_Pointer_Button5
#define GDK_KEY_Pointer_Button5 0xfeed
#endif
#ifndef GDK_KEY_Hangul_J_RieulKiyeog
#define GDK_KEY_Hangul_J_RieulKiyeog 0xedc
#endif
#ifndef GDK_KEY_KP_1
#define GDK_KEY_KP_1 0xffb1
#endif
#ifndef GDK_KEY_emdash
#define GDK_KEY_emdash 0xaa9
#endif
#ifndef GDK_KEY_eth
#define GDK_KEY_eth 0x0f0
#endif
#ifndef GDK_KEY_Switch_VT_2
#define GDK_KEY_Switch_VT_2 0x1008fe02
#endif
#ifndef GDK_KEY_braille_dots_23678
#define GDK_KEY_braille_dots_23678 0x10028e6
#endif
#ifndef GDK_KEY_botleftparens
#define GDK_KEY_botleftparens 0x8ac
#endif
#ifndef GDK_KEY_fourfifths
#define GDK_KEY_fourfifths 0xab5
#endif
#ifndef GDK_KEY_Greek_rho
#define GDK_KEY_Greek_rho 0x7f1
#endif
#ifndef GDK_KEY_SplitScreen
#define GDK_KEY_SplitScreen 0x1008ff7d
#endif
#ifndef GDK_KEY_Super_L
#define GDK_KEY_Super_L 0xffeb
#endif
#ifndef GDK_KEY_Cyrillic_SHHA
#define GDK_KEY_Cyrillic_SHHA 0x10004ba
#endif
#ifndef GDK_KEY_Acircumflextilde
#define GDK_KEY_Acircumflextilde 0x1001eaa
#endif
#ifndef GDK_KEY_Hangul_YeorinHieuh
#define GDK_KEY_Hangul_YeorinHieuh 0xef5
#endif
#ifndef GDK_KEY_ohornacute
#define GDK_KEY_ohornacute 0x1001edb
#endif
#ifndef GDK_KEY_3270_EraseEOF
#define GDK_KEY_3270_EraseEOF 0xfd06
#endif
#ifndef GDK_KEY_hebrew_qoph
#define GDK_KEY_hebrew_qoph 0xcf7
#endif
#ifndef GDK_KEY_Thai_totao
#define GDK_KEY_Thai_totao 0xdb5
#endif
#ifndef GDK_KEY_Byelorussian_shortu
#define GDK_KEY_Byelorussian_shortu 0x6ae
#endif
#ifndef GDK_KEY_3270_Duplicate
#define GDK_KEY_3270_Duplicate 0xfd01
#endif
#ifndef GDK_KEY_gcaron
#define GDK_KEY_gcaron 0x10001e7
#endif
#ifndef GDK_KEY_Hangul_J_Jieuj
#define GDK_KEY_Hangul_J_Jieuj 0xee9
#endif
#ifndef GDK_KEY_Mabovedot
#define GDK_KEY_Mabovedot 0x1001e40
#endif
#ifndef GDK_KEY_kana_CHI
#define GDK_KEY_kana_CHI 0x4c1
#endif
#ifndef GDK_KEY_Agrave
#define GDK_KEY_Agrave 0x0c0
#endif
#ifndef GDK_KEY_gabovedot
#define GDK_KEY_gabovedot 0x2f5
#endif
#ifndef GDK_KEY_braille_dot_3
#define GDK_KEY_braille_dot_3 0xfff3
#endif
#ifndef GDK_KEY_3270_PA2
#define GDK_KEY_3270_PA2 0xfd0b
#endif
#ifndef GDK_KEY_maltesecross
#define GDK_KEY_maltesecross 0xaf0
#endif
#ifndef GDK_KEY_braille_dots_24678
#define GDK_KEY_braille_dots_24678 0x10028ea
#endif
#ifndef GDK_KEY_Ukranian_je
#define GDK_KEY_Ukranian_je 0x6a4
#endif
#ifndef GDK_KEY_Ukrainian_IE
#define GDK_KEY_Ukrainian_IE 0x6b4
#endif
#ifndef GDK_KEY_Georgian_hie
#define GDK_KEY_Georgian_hie 0x10010f2
#endif
#ifndef GDK_KEY_Armenian_MEN
#define GDK_KEY_Armenian_MEN 0x1000544
#endif
#ifndef GDK_KEY_Georgian_par
#define GDK_KEY_Georgian_par 0x10010de
#endif
#ifndef GDK_KEY_LogOff
#define GDK_KEY_LogOff 0x1008ff61
#endif
#ifndef GDK_KEY_Favorites
#define GDK_KEY_Favorites 0x1008ff30
#endif
#ifndef GDK_KEY_ydiaeresis
#define GDK_KEY_ydiaeresis 0x0ff
#endif
#ifndef GDK_KEY_Armenian_ZA
#define GDK_KEY_Armenian_ZA 0x1000536
#endif
#ifndef GDK_KEY_Hangul_AraeAE
#define GDK_KEY_Hangul_AraeAE 0xef7
#endif
#ifndef GDK_KEY_Cyrillic_SHA
#define GDK_KEY_Cyrillic_SHA 0x6fb
#endif
#ifndef GDK_KEY_braille_dot_10
#define GDK_KEY_braille_dot_10 0xfffa
#endif
#ifndef GDK_KEY_braille_dots_23456
#define GDK_KEY_braille_dots_23456 0x100283e
#endif
#ifndef GDK_KEY_ISO_Prev_Group
#define GDK_KEY_ISO_Prev_Group 0xfe0a
#endif
#ifndef GDK_KEY_VendorHome
#define GDK_KEY_VendorHome 0x1008ff34
#endif
#ifndef GDK_KEY_ellipsis
#define GDK_KEY_ellipsis 0xaae
#endif
#ifndef GDK_KEY_KP_Add
#define GDK_KEY_KP_Add 0xffab
#endif
#ifndef GDK_KEY_Pointer_DblClick2
#define GDK_KEY_Pointer_DblClick2 0xfef0
#endif
#ifndef GDK_KEY_hebrew_lamed
#define GDK_KEY_hebrew_lamed 0xcec
#endif
#ifndef GDK_KEY_kana_NO
#define GDK_KEY_kana_NO 0x4c9
#endif
#ifndef GDK_KEY_dead_abovecomma
#define GDK_KEY_dead_abovecomma 0xfe64
#endif
#ifndef GDK_KEY_UserPB
#define GDK_KEY_UserPB 0x1008ff84
#endif
#ifndef GDK_KEY_Racute
#define GDK_KEY_Racute 0x1c0
#endif
#ifndef GDK_KEY_braille_dots_3478
#define GDK_KEY_braille_dots_3478 0x10028cc
#endif
#ifndef GDK_KEY_braille_dots_12578
#define GDK_KEY_braille_dots_12578 0x10028d3
#endif
#ifndef GDK_KEY_R6
#define GDK_KEY_R6 0xffd7
#endif
#ifndef GDK_KEY_caron
#define GDK_KEY_caron 0x1b7
#endif
#ifndef GDK_KEY_braille_dots_2457
#define GDK_KEY_braille_dots_2457 0x100285a
#endif
#ifndef GDK_KEY_Georgian_shin
#define GDK_KEY_Georgian_shin 0x10010e8
#endif
#ifndef GDK_KEY_F17
#define GDK_KEY_F17 0xffce
#endif
#ifndef GDK_KEY_braille_dots_12368
#define GDK_KEY_braille_dots_12368 0x10028a7
#endif
#ifndef GDK_KEY_Georgian_gan
#define GDK_KEY_Georgian_gan 0x10010d2
#endif
#ifndef GDK_KEY_leftanglebracket
#define GDK_KEY_leftanglebracket 0xabc
#endif
#ifndef GDK_KEY_Messenger
#define GDK_KEY_Messenger 0x1008ff8e
#endif
#ifndef GDK_KEY_Hangul_Jieuj
#define GDK_KEY_Hangul_Jieuj 0xeb8
#endif
#ifndef GDK_KEY_Armenian_ra
#define GDK_KEY_Armenian_ra 0x100057c
#endif
#ifndef GDK_KEY_Thai_saraaimaimalai
#define GDK_KEY_Thai_saraaimaimalai 0xde4
#endif
#ifndef GDK_KEY_union
#define GDK_KEY_union 0x8dd
#endif
#ifndef GDK_KEY_braille_dots_234568
#define GDK_KEY_braille_dots_234568 0x10028be
#endif
#ifndef GDK_KEY_braceright
#define GDK_KEY_braceright 0x07d
#endif
#ifndef GDK_KEY_cent
#define GDK_KEY_cent 0x0a2
#endif
#ifndef GDK_KEY_Arabic_alefmaksura
#define GDK_KEY_Arabic_alefmaksura 0x5e9
#endif
#ifndef GDK_KEY_Thai_sarai
#define GDK_KEY_Thai_sarai 0xdd4
#endif
#ifndef GDK_KEY_KP_Prior
#define GDK_KEY_KP_Prior 0xff9a
#endif
#ifndef GDK_KEY_braille_dots_1234568
#define GDK_KEY_braille_dots_1234568 0x10028bf
#endif
#ifndef GDK_KEY_Aring
#define GDK_KEY_Aring 0x0c5
#endif
#ifndef GDK_KEY_scedilla
#define GDK_KEY_scedilla 0x1ba
#endif
#ifndef GDK_KEY_Ccaron
#define GDK_KEY_Ccaron 0x1c8
#endif
#ifndef GDK_KEY_Hangul_SsangPieub
#define GDK_KEY_Hangul_SsangPieub 0xeb3
#endif
#ifndef GDK_KEY_ISO_Emphasize
#define GDK_KEY_ISO_Emphasize 0xfe32
#endif
#ifndef GDK_KEY_hebrew_taf
#define GDK_KEY_hebrew_taf 0xcfa
#endif
#ifndef GDK_KEY_uptack
#define GDK_KEY_uptack 0xbce
#endif
#ifndef GDK_KEY_e
#define GDK_KEY_e 0x065
#endif
#ifndef GDK_KEY_leftdoublequotemark
#define GDK_KEY_leftdoublequotemark 0xad2
#endif
#ifndef GDK_KEY_braille_dots_1248
#define GDK_KEY_braille_dots_1248 0x100288b
#endif
#ifndef GDK_KEY_Thai_thothung
#define GDK_KEY_Thai_thothung 0xdb6
#endif
#ifndef GDK_KEY_ssharp
#define GDK_KEY_ssharp 0x0df
#endif
#ifndef GDK_KEY_Travel
#define GDK_KEY_Travel 0x1008ff82
#endif
#ifndef GDK_KEY_braille_dots_2358
#define GDK_KEY_braille_dots_2358 0x1002896
#endif
#ifndef GDK_KEY_ecaron
#define GDK_KEY_ecaron 0x1ec
#endif
#ifndef GDK_KEY_filledtribulletup
#define GDK_KEY_filledtribulletup 0xae8
#endif
#ifndef GDK_KEY_3270_PrintScreen
#define GDK_KEY_3270_PrintScreen 0xfd1d
#endif
#ifndef GDK_KEY_Hangul_YI
#define GDK_KEY_Hangul_YI 0xed2
#endif
#ifndef GDK_KEY_Insert
#define GDK_KEY_Insert 0xff63
#endif
#ifndef GDK_KEY_L1
#define GDK_KEY_L1 0xffc8
#endif
#ifndef GDK_KEY_Thai_paiyannoi
#define GDK_KEY_Thai_paiyannoi 0xdcf
#endif
#ifndef GDK_KEY_itilde
#define GDK_KEY_itilde 0x3b5
#endif
#ifndef GDK_KEY_musicalsharp
#define GDK_KEY_musicalsharp 0xaf5
#endif
#ifndef GDK_KEY_Ugrave
#define GDK_KEY_Ugrave 0x0d9
#endif
#ifndef GDK_KEY_Hangul_J_Hieuh
#define GDK_KEY_Hangul_J_Hieuh 0xeee
#endif
#ifndef GDK_KEY_ISO_Level5_Shift
#define GDK_KEY_ISO_Level5_Shift 0xfe11
#endif
#ifndef GDK_KEY_numerosign
#define GDK_KEY_numerosign 0x6b0
#endif
#ifndef GDK_KEY_seconds
#define GDK_KEY_seconds 0xad7
#endif
#ifndef GDK_KEY_Multi_key
#define GDK_KEY_Multi_key 0xff20
#endif
#ifndef GDK_KEY_enopencircbullet
#define GDK_KEY_enopencircbullet 0xae0
#endif
#ifndef GDK_KEY_Cyrillic_ha
#define GDK_KEY_Cyrillic_ha 0x6c8
#endif
#ifndef GDK_KEY_braille_dots_123458
#define GDK_KEY_braille_dots_123458 0x100289f
#endif
#ifndef GDK_KEY_Thai_lu
#define GDK_KEY_Thai_lu 0xdc6
#endif
#ifndef GDK_KEY_hebrew_gimel
#define GDK_KEY_hebrew_gimel 0xce2
#endif
#ifndef GDK_KEY_overbar
#define GDK_KEY_overbar 0xbc0
#endif
#ifndef GDK_KEY_ncaron
#define GDK_KEY_ncaron 0x1f2
#endif
#ifndef GDK_KEY_tabovedot
#define GDK_KEY_tabovedot 0x1001e6b
#endif
#ifndef GDK_KEY_q
#define GDK_KEY_q 0x071
#endif
#ifndef GDK_KEY_horizlinescan7
#define GDK_KEY_horizlinescan7 0x9f2
#endif
#ifndef GDK_KEY_Armenian_GHAT
#define GDK_KEY_Armenian_GHAT 0x1000542
#endif
#ifndef GDK_KEY_braille_dots_23568
#define GDK_KEY_braille_dots_23568 0x10028b6
#endif
#ifndef GDK_KEY_greaterthanequal
#define GDK_KEY_greaterthanequal 0x8be
#endif
#ifndef GDK_KEY_rightmiddlecurlybrace
#define GDK_KEY_rightmiddlecurlybrace 0x8b0
#endif
#ifndef GDK_KEY_kana_conjunctive
#define GDK_KEY_kana_conjunctive 0x4a5
#endif
#ifndef GDK_KEY_multiply
#define GDK_KEY_multiply 0x0d7
#endif
#ifndef GDK_KEY_Greek_PHI
#define GDK_KEY_Greek_PHI 0x7d6
#endif
#ifndef GDK_KEY_braille_dots_1247
#define GDK_KEY_braille_dots_1247 0x100284b
#endif
#ifndef GDK_KEY_latincross
#define GDK_KEY_latincross 0xad9
#endif
#ifndef GDK_KEY_kana_N
#define GDK_KEY_kana_N 0x4dd
#endif
#ifndef GDK_KEY_Greek_upsilondieresis
#define GDK_KEY_Greek_upsilondieresis 0x7b9
#endif
#ifndef GDK_KEY_CycleAngle
#define GDK_KEY_CycleAngle 0x1008ff9c
#endif
#ifndef GDK_KEY_infinity
#define GDK_KEY_infinity 0x8c2
#endif
#ifndef GDK_KEY_kana_yo
#define GDK_KEY_kana_yo 0x4ae
#endif
#ifndef GDK_KEY_Romaji
#define GDK_KEY_Romaji 0xff24
#endif
#ifndef GDK_KEY_yhook
#define GDK_KEY_yhook 0x1001ef7
#endif
#ifndef GDK_KEY_quotedbl
#define GDK_KEY_quotedbl 0x022
#endif
#ifndef GDK_KEY_xabovedot
#define GDK_KEY_xabovedot 0x1001e8b
#endif
#ifndef GDK_KEY_kana_NI
#define GDK_KEY_kana_NI 0x4c6
#endif
#ifndef GDK_KEY_3270_EraseInput
#define GDK_KEY_3270_EraseInput 0xfd07
#endif
#ifndef GDK_KEY_RockerDown
#define GDK_KEY_RockerDown 0x1008ff24
#endif
#ifndef GDK_KEY_Pointer_Drag2
#define GDK_KEY_Pointer_Drag2 0xfef6
#endif
#ifndef GDK_KEY_filledtribulletdown
#define GDK_KEY_filledtribulletdown 0xae9
#endif
#ifndef GDK_KEY_Zstroke
#define GDK_KEY_Zstroke 0x10001b5
#endif
#ifndef GDK_KEY_digitspace
#define GDK_KEY_digitspace 0xaa5
#endif
#ifndef GDK_KEY_ISO_First_Group
#define GDK_KEY_ISO_First_Group 0xfe0c
#endif
#ifndef GDK_KEY_Hangul_J_PanSios
#define GDK_KEY_Hangul_J_PanSios 0xef8
#endif
#ifndef GDK_KEY_Thai_leknung
#define GDK_KEY_Thai_leknung 0xdf1
#endif
#ifndef GDK_KEY_Hangul_YEO
#define GDK_KEY_Hangul_YEO 0xec5
#endif
#ifndef GDK_KEY_Thai_baht
#define GDK_KEY_Thai_baht 0xddf
#endif
#ifndef GDK_KEY_ogonek
#define GDK_KEY_ogonek 0x1b2
#endif
#ifndef GDK_KEY_3270_Record
#define GDK_KEY_3270_Record 0xfd18
#endif
#ifndef GDK_KEY_hebrew_kuf
#define GDK_KEY_hebrew_kuf 0xcf7
#endif
#ifndef GDK_KEY_Arabic_yeh_baree
#define GDK_KEY_Arabic_yeh_baree 0x10006d2
#endif
#ifndef GDK_KEY_Thai_fofan
#define GDK_KEY_Thai_fofan 0xdbf
#endif
#ifndef GDK_KEY_includedin
#define GDK_KEY_includedin 0x8da
#endif
#ifndef GDK_KEY_R1
#define GDK_KEY_R1 0xffd2
#endif
#ifndef GDK_KEY_Thai_maiek
#define GDK_KEY_Thai_maiek 0xde8
#endif
#ifndef GDK_KEY_Alt_L
#define GDK_KEY_Alt_L 0xffe9
#endif
#ifndef GDK_KEY_Armenian_LYUN
#define GDK_KEY_Armenian_LYUN 0x100053c
#endif
#ifndef GDK_KEY_odoubleacute
#define GDK_KEY_odoubleacute 0x1f5
#endif
#ifndef GDK_KEY_Armenian_men
#define GDK_KEY_Armenian_men 0x1000574
#endif
#ifndef GDK_KEY_ecircumflexhook
#define GDK_KEY_ecircumflexhook 0x1001ec3
#endif
#ifndef GDK_KEY_Dstroke
#define GDK_KEY_Dstroke 0x1d0
#endif
#ifndef GDK_KEY_Thai_phosamphao
#define GDK_KEY_Thai_phosamphao 0xdc0
#endif
#ifndef GDK_KEY_Hangul_Nieun
#define GDK_KEY_Hangul_Nieun 0xea4
#endif
#ifndef GDK_KEY_braille_blank
#define GDK_KEY_braille_blank 0x1002800
#endif
#ifndef GDK_KEY_Hangul_Hanja
#define GDK_KEY_Hangul_Hanja 0xff34
#endif
#ifndef GDK_KEY_ohorn
#define GDK_KEY_ohorn 0x10001a1
#endif
#ifndef GDK_KEY_Armenian_TCHE
#define GDK_KEY_Armenian_TCHE 0x1000543
#endif
#ifndef GDK_KEY_Scircumflex
#define GDK_KEY_Scircumflex 0x2de
#endif
#ifndef GDK_KEY_Cyrillic_U
#define GDK_KEY_Cyrillic_U 0x6f5
#endif
#ifndef GDK_KEY_Hangul_KkogjiDalrinIeung
#define GDK_KEY_Hangul_KkogjiDalrinIeung 0xef3
#endif
#ifndef GDK_KEY_botleftsqbracket
#define GDK_KEY_botleftsqbracket 0x8a8
#endif
#ifndef GDK_KEY_Georgian_jil
#define GDK_KEY_Georgian_jil 0x10010eb
#endif
#ifndef GDK_KEY_braille_dots_13457
#define GDK_KEY_braille_dots_13457 0x100285d
#endif
#ifndef GDK_KEY_colon
#define GDK_KEY_colon 0x03a
#endif
#ifndef GDK_KEY_botvertsummationconnector
#define GDK_KEY_botvertsummationconnector 0x8b4
#endif
#ifndef GDK_KEY_hcircumflex
#define GDK_KEY_hcircumflex 0x2b6
#endif
#ifndef GDK_KEY_braille_dots_13478
#define GDK_KEY_braille_dots_13478 0x10028cd
#endif
#ifndef GDK_KEY_Hangul
#define GDK_KEY_Hangul 0xff31
#endif
#ifndef GDK_KEY_F31
#define GDK_KEY_F31 0xffdc
#endif
#ifndef GDK_KEY_Arabic_9
#define GDK_KEY_Arabic_9 0x1000669
#endif
#ifndef GDK_KEY_Arabic_3
#define GDK_KEY_Arabic_3 0x1000663
#endif
#ifndef GDK_KEY_Georgian_fi
#define GDK_KEY_Georgian_fi 0x10010f6
#endif
#ifndef GDK_KEY_hebrew_finalkaph
#define GDK_KEY_hebrew_finalkaph 0xcea
#endif
#ifndef GDK_KEY_Greek_KAPPA
#define GDK_KEY_Greek_KAPPA 0x7ca
#endif
#ifndef GDK_KEY_Hangul_J_KiyeogSios
#define GDK_KEY_Hangul_J_KiyeogSios 0xed6
#endif
#ifndef GDK_KEY_exclam
#define GDK_KEY_exclam 0x021
#endif
#ifndef GDK_KEY_Obarred
#define GDK_KEY_Obarred 0x100019f
#endif
#ifndef GDK_KEY_Thai_thothahan
#define GDK_KEY_Thai_thothahan 0xdb7
#endif
#ifndef GDK_KEY_F23
#define GDK_KEY_F23 0xffd4
#endif
#ifndef GDK_KEY_rightshoe
#define GDK_KEY_rightshoe 0xbd8
#endif
#ifndef GDK_KEY_Tslash
#define GDK_KEY_Tslash 0x3ac
#endif
#ifndef GDK_KEY_dollar
#define GDK_KEY_dollar 0x024
#endif
#ifndef GDK_KEY_Tools
#define GDK_KEY_Tools 0x1008ff81
#endif
#ifndef GDK_KEY_greater
#define GDK_KEY_greater 0x03e
#endif
#ifndef GDK_KEY_braille_dots_123567
#define GDK_KEY_braille_dots_123567 0x1002877
#endif
#ifndef GDK_KEY_Armenian_verjaket
#define GDK_KEY_Armenian_verjaket 0x1000589
#endif
#ifndef GDK_KEY_threesuperior
#define GDK_KEY_threesuperior 0x0b3
#endif
#ifndef GDK_KEY_Battery
#define GDK_KEY_Battery 0x1008ff93
#endif
#ifndef GDK_KEY_dead_belowbreve
#define GDK_KEY_dead_belowbreve 0xfe6b
#endif
#ifndef GDK_KEY_Arabic_question_mark
#define GDK_KEY_Arabic_question_mark 0x5bf
#endif
#ifndef GDK_KEY_Etilde
#define GDK_KEY_Etilde 0x1001ebc
#endif
#ifndef GDK_KEY_Arabic_beh
#define GDK_KEY_Arabic_beh 0x5c8
#endif
#ifndef GDK_KEY_Music
#define GDK_KEY_Music 0x1008ff92
#endif
#ifndef GDK_KEY_Hangul_J_Nieun
#define GDK_KEY_Hangul_J_Nieun 0xed7
#endif
#ifndef GDK_KEY_Arabic_farsi_yeh
#define GDK_KEY_Arabic_farsi_yeh 0x10006cc
#endif
#ifndef GDK_KEY_Cyrillic_lje
#define GDK_KEY_Cyrillic_lje 0x6a9
#endif
#ifndef GDK_KEY_Cyrillic_NJE
#define GDK_KEY_Cyrillic_NJE 0x6ba
#endif
#ifndef GDK_KEY_braille_dots_3457
#define GDK_KEY_braille_dots_3457 0x100285c
#endif
#ifndef GDK_KEY_LaunchE
#define GDK_KEY_LaunchE 0x1008ff4e
#endif
#ifndef GDK_KEY_braille_dots_68
#define GDK_KEY_braille_dots_68 0x10028a0
#endif
#ifndef GDK_KEY_yen
#define GDK_KEY_yen 0x0a5
#endif
#ifndef GDK_KEY_Thai_phophung
#define GDK_KEY_Thai_phophung 0xdbc
#endif
#ifndef GDK_KEY_L10
#define GDK_KEY_L10 0xffd1
#endif
#ifndef GDK_KEY_horizlinescan3
#define GDK_KEY_horizlinescan3 0x9f0
#endif
#ifndef GDK_KEY_braille_dots_167
#define GDK_KEY_braille_dots_167 0x1002861
#endif
#ifndef GDK_KEY_Red
#define GDK_KEY_Red 0x1008ffa3
#endif
#ifndef GDK_KEY_braille_dots_247
#define GDK_KEY_braille_dots_247 0x100284a
#endif
#ifndef GDK_KEY_partdifferential
#define GDK_KEY_partdifferential 0x1002202
#endif
#ifndef GDK_KEY_Hangul_SsangSios
#define GDK_KEY_Hangul_SsangSios 0xeb6
#endif
#ifndef GDK_KEY_FrameBack
#define GDK_KEY_FrameBack 0x1008ff9d
#endif
#ifndef GDK_KEY_ClearGrab
#define GDK_KEY_ClearGrab 0x1008fe21
#endif
#ifndef GDK_KEY_ISO_Partial_Line_Down
#define GDK_KEY_ISO_Partial_Line_Down 0xfe24
#endif
#ifndef GDK_KEY_Georgian_zen
#define GDK_KEY_Georgian_zen 0x10010d6
#endif
#ifndef GDK_KEY_ContrastAdjust
#define GDK_KEY_ContrastAdjust 0x1008ff22
#endif
#ifndef GDK_KEY_Aacute
#define GDK_KEY_Aacute 0x0c1
#endif
#ifndef GDK_KEY_Arabic_heh_goal
#define GDK_KEY_Arabic_heh_goal 0x10006c1
#endif
#ifndef GDK_KEY_upstile
#define GDK_KEY_upstile 0xbd3
#endif
#ifndef GDK_KEY_lcaron
#define GDK_KEY_lcaron 0x1b5
#endif
#ifndef GDK_KEY_Arabic_comma
#define GDK_KEY_Arabic_comma 0x5ac
#endif
#ifndef GDK_KEY_Wdiaeresis
#define GDK_KEY_Wdiaeresis 0x1001e84
#endif
#ifndef GDK_KEY_vertconnector
#define GDK_KEY_vertconnector 0x8a6
#endif
#ifndef GDK_KEY_Z
#define GDK_KEY_Z 0x05a
#endif
#ifndef GDK_KEY_Arabic_damma
#define GDK_KEY_Arabic_damma 0x5ef
#endif
#ifndef GDK_KEY_WindowClear
#define GDK_KEY_WindowClear 0x1008ff55
#endif
#ifndef GDK_KEY_Arabic_ddal
#define GDK_KEY_Arabic_ddal 0x1000688
#endif
#ifndef GDK_KEY_Arabic_veh
#define GDK_KEY_Arabic_veh 0x10006a4
#endif
#ifndef GDK_KEY_Cyrillic_shcha
#define GDK_KEY_Cyrillic_shcha 0x6dd
#endif
#ifndef GDK_KEY_club
#define GDK_KEY_club 0xaec
#endif
#ifndef GDK_KEY_Arabic_dal
#define GDK_KEY_Arabic_dal 0x5cf
#endif
#ifndef GDK_KEY_KP_Left
#define GDK_KEY_KP_Left 0xff96
#endif
#ifndef GDK_KEY_Arabic_rreh
#define GDK_KEY_Arabic_rreh 0x1000691
#endif
#ifndef GDK_KEY_Greek_MU
#define GDK_KEY_Greek_MU 0x7cc
#endif
#ifndef GDK_KEY_Armenian_lyun
#define GDK_KEY_Armenian_lyun 0x100056c
#endif
#ifndef GDK_KEY_Thorn
#define GDK_KEY_Thorn 0x0de
#endif
#ifndef GDK_KEY_Thai_maihanakat
#define GDK_KEY_Thai_maihanakat 0xdd1
#endif
#ifndef GDK_KEY_Armenian_pyur
#define GDK_KEY_Armenian_pyur 0x1000583
#endif
#ifndef GDK_KEY_Acircumflexacute
#define GDK_KEY_Acircumflexacute 0x1001ea4
#endif
#ifndef GDK_KEY_Armenian_PYUR
#define GDK_KEY_Armenian_PYUR 0x1000553
#endif
#ifndef GDK_KEY_braille_dots_567
#define GDK_KEY_braille_dots_567 0x1002870
#endif
#ifndef GDK_KEY_AudioRaiseVolume
#define GDK_KEY_AudioRaiseVolume 0x1008ff13
#endif
#ifndef GDK_KEY_KP_Divide
#define GDK_KEY_KP_Divide 0xffaf
#endif
#ifndef GDK_KEY_openstar
#define GDK_KEY_openstar 0xae5
#endif
#ifndef GDK_KEY_phonographcopyright
#define GDK_KEY_phonographcopyright 0xafb
#endif
#ifndef GDK_KEY_braille_dots_1345678
#define GDK_KEY_braille_dots_1345678 0x10028fd
#endif
#ifndef GDK_KEY_Ocaron
#define GDK_KEY_Ocaron 0x10001d1
#endif
#ifndef GDK_KEY_Thai_hohip
#define GDK_KEY_Thai_hohip 0xdcb
#endif
#ifndef GDK_KEY_ISO_Set_Margin_Left
#define GDK_KEY_ISO_Set_Margin_Left 0xfe27
#endif
#ifndef GDK_KEY_MySites
#define GDK_KEY_MySites 0x1008ff67
#endif
#ifndef GDK_KEY_Hangul_Dikeud
#define GDK_KEY_Hangul_Dikeud 0xea7
#endif
#ifndef GDK_KEY_periodcentered
#define GDK_KEY_periodcentered 0x0b7
#endif
#ifndef GDK_KEY_odiaeresis
#define GDK_KEY_odiaeresis 0x0f6
#endif
#ifndef GDK_KEY_agrave
#define GDK_KEY_agrave 0x0e0
#endif
#ifndef GDK_KEY_H
#define GDK_KEY_H 0x048
#endif
#ifndef GDK_KEY_kana_YU
#define GDK_KEY_kana_YU 0x4d5
#endif
#ifndef GDK_KEY_n
#define GDK_KEY_n 0x06e
#endif
#ifndef GDK_KEY_Muhenkan
#define GDK_KEY_Muhenkan 0xff22
#endif
#ifndef GDK_KEY_Armenian_exclam
#define GDK_KEY_Armenian_exclam 0x100055c
#endif
#ifndef GDK_KEY_kana_RO
#define GDK_KEY_kana_RO 0x4db
#endif
#ifndef GDK_KEY_Hangul_PieubSios
#define GDK_KEY_Hangul_PieubSios 0xeb4
#endif
#ifndef GDK_KEY_Pointer_DblClick_Dflt
#define GDK_KEY_Pointer_DblClick_Dflt 0xfeee
#endif
#ifndef GDK_KEY_opentribulletdown
#define GDK_KEY_opentribulletdown 0xae4
#endif
#ifndef GDK_KEY_Arabic_jeh
#define GDK_KEY_Arabic_jeh 0x1000698
#endif
#ifndef GDK_KEY_Armenian_amanak
#define GDK_KEY_Armenian_amanak 0x100055c
#endif
#ifndef GDK_KEY_Hangul_J_YeorinHieuh
#define GDK_KEY_Hangul_J_YeorinHieuh 0xefa
#endif
#ifndef GDK_KEY_Gcedilla
#define GDK_KEY_Gcedilla 0x3ab
#endif
#ifndef GDK_KEY_Georgian_ban
#define GDK_KEY_Georgian_ban 0x10010d1
#endif
#ifndef GDK_KEY_Greek_horizbar
#define GDK_KEY_Greek_horizbar 0x7af
#endif
#ifndef GDK_KEY_L8
#define GDK_KEY_L8 0xffcf
#endif
#ifndef GDK_KEY_uring
#define GDK_KEY_uring 0x1f9
#endif
#ifndef GDK_KEY_Cyrillic_a
#define GDK_KEY_Cyrillic_a 0x6c1
#endif
#ifndef GDK_KEY_Armenian_AYB
#define GDK_KEY_Armenian_AYB 0x1000531
#endif
#ifndef GDK_KEY_dead_i
#define GDK_KEY_dead_i 0xfe84
#endif
#ifndef GDK_KEY_w
#define GDK_KEY_w 0x077
#endif
#ifndef GDK_KEY_Cacute
#define GDK_KEY_Cacute 0x1c6
#endif
#ifndef GDK_KEY_h
#define GDK_KEY_h 0x068
#endif
#ifndef GDK_KEY_semicolon
#define GDK_KEY_semicolon 0x03b
#endif
#ifndef GDK_KEY_braille_dots_1347
#define GDK_KEY_braille_dots_1347 0x100284d
#endif
#ifndef GDK_KEY_Georgian_un
#define GDK_KEY_Georgian_un 0x10010e3
#endif
#ifndef GDK_KEY_hebrew_finalmem
#define GDK_KEY_hebrew_finalmem 0xced
#endif
#ifndef GDK_KEY_braille_dots_36
#define GDK_KEY_braille_dots_36 0x1002824
#endif
#ifndef GDK_KEY_Hangul_Start
#define GDK_KEY_Hangul_Start 0xff32
#endif
#ifndef GDK_KEY_seveneighths
#define GDK_KEY_seveneighths 0xac6
#endif
#ifndef GDK_KEY_kana_RE
#define GDK_KEY_kana_RE 0x4da
#endif
#ifndef GDK_KEY_Arabic_yeh
#define GDK_KEY_Arabic_yeh 0x5ea
#endif
#ifndef GDK_KEY_braille_dots_134
#define GDK_KEY_braille_dots_134 0x100280d
#endif
#ifndef GDK_KEY_braille_dots_1257
#define GDK_KEY_braille_dots_1257 0x1002853
#endif
#ifndef GDK_KEY_udiaeresis
#define GDK_KEY_udiaeresis 0x0fc
#endif
#ifndef GDK_KEY_braille_dots_237
#define GDK_KEY_braille_dots_237 0x1002846
#endif
#ifndef GDK_KEY_Kanji_Bangou
#define GDK_KEY_Kanji_Bangou 0xff37
#endif
#ifndef GDK_KEY_Ebelowdot
#define GDK_KEY_Ebelowdot 0x1001eb8
#endif
#ifndef GDK_KEY_braille_dots_378
#define GDK_KEY_braille_dots_378 0x10028c4
#endif
#ifndef GDK_KEY_braille_dots_2678
#define GDK_KEY_braille_dots_2678 0x10028e2
#endif
#ifndef GDK_KEY_dead_perispomeni
#define GDK_KEY_dead_perispomeni 0xfe53
#endif
#ifndef GDK_KEY_Umacron
#define GDK_KEY_Umacron 0x3de
#endif
#ifndef GDK_KEY_3270_CursorSelect
#define GDK_KEY_3270_CursorSelect 0xfd1c
#endif
#ifndef GDK_KEY_onesubscript
#define GDK_KEY_onesubscript 0x1002081
#endif
#ifndef GDK_KEY_Ibelowdot
#define GDK_KEY_Ibelowdot 0x1001eca
#endif
#ifndef GDK_KEY_OE
#define GDK_KEY_OE 0x13bc
#endif
#ifndef GDK_KEY_Thai_lekhok
#define GDK_KEY_Thai_lekhok 0xdf6
#endif
#ifndef GDK_KEY_braille_dot_7
#define GDK_KEY_braille_dot_7 0xfff7
#endif
#ifndef GDK_KEY_braille_dots_1578
#define GDK_KEY_braille_dots_1578 0x10028d1
#endif
#ifndef GDK_KEY_logicaland
#define GDK_KEY_logicaland 0x8de
#endif
#ifndef GDK_KEY_topt
#define GDK_KEY_topt 0x9f7
#endif
#ifndef GDK_KEY_Hangul_Kiyeog
#define GDK_KEY_Hangul_Kiyeog 0xea1
#endif
#ifndef GDK_KEY_Gcircumflex
#define GDK_KEY_Gcircumflex 0x2d8
#endif
#ifndef GDK_KEY_hebrew_waw
#define GDK_KEY_hebrew_waw 0xce5
#endif
#ifndef GDK_KEY_ff
#define GDK_KEY_ff 0x9e3
#endif
#ifndef GDK_KEY_Cyrillic_GHE_bar
#define GDK_KEY_Cyrillic_GHE_bar 0x1000492
#endif
#ifndef GDK_KEY_Greek_ALPHAaccent
#define GDK_KEY_Greek_ALPHAaccent 0x7a1
#endif
#ifndef GDK_KEY_idiaeresis
#define GDK_KEY_idiaeresis 0x0ef
#endif
#ifndef GDK_KEY_Cyrillic_U_straight_bar
#define GDK_KEY_Cyrillic_U_straight_bar 0x10004b0
#endif
#ifndef GDK_KEY_braille_dots_1358
#define GDK_KEY_braille_dots_1358 0x1002895
#endif
#ifndef GDK_KEY_braille_dots_1367
#define GDK_KEY_braille_dots_1367 0x1002865
#endif
#ifndef GDK_KEY_Armenian_JE
#define GDK_KEY_Armenian_JE 0x100054b
#endif
#ifndef GDK_KEY_Ubreve
#define GDK_KEY_Ubreve 0x2dd
#endif
#ifndef GDK_KEY_Hangul_J_Rieul
#define GDK_KEY_Hangul_J_Rieul 0xedb
#endif
#ifndef GDK_KEY_Acircumflex
#define GDK_KEY_Acircumflex 0x0c2
#endif
#ifndef GDK_KEY_ISO_Left_Tab
#define GDK_KEY_ISO_Left_Tab 0xfe20
#endif
#ifndef GDK_KEY_Greek_omicronaccent
#define GDK_KEY_Greek_omicronaccent 0x7b7
#endif
#ifndef GDK_KEY_Arabic_hamzaonwaw
#define GDK_KEY_Arabic_hamzaonwaw 0x5c4
#endif
#ifndef GDK_KEY_braille_dots_57
#define GDK_KEY_braille_dots_57 0x1002850
#endif
#ifndef GDK_KEY_wdiaeresis
#define GDK_KEY_wdiaeresis 0x1001e85
#endif
#ifndef GDK_KEY_Greek_OMEGA
#define GDK_KEY_Greek_OMEGA 0x7d9
#endif
#ifndef GDK_KEY_similarequal
#define GDK_KEY_similarequal 0x8c9
#endif
#ifndef GDK_KEY_Greek_IOTA
#define GDK_KEY_Greek_IOTA 0x7c9
#endif
#ifndef GDK_KEY_Farsi_7
#define GDK_KEY_Farsi_7 0x10006f7
#endif
#ifndef GDK_KEY_braille_dots_146
#define GDK_KEY_braille_dots_146 0x1002829
#endif
#ifndef GDK_KEY_Arabic_meem
#define GDK_KEY_Arabic_meem 0x5e5
#endif
#ifndef GDK_KEY_lowrightcorner
#define GDK_KEY_lowrightcorner 0x9ea
#endif
#ifndef GDK_KEY_RotateWindows
#define GDK_KEY_RotateWindows 0x1008ff74
#endif
#ifndef GDK_KEY_Thai_phophan
#define GDK_KEY_Thai_phophan 0xdbe
#endif
#ifndef GDK_KEY_braille_dots_46
#define GDK_KEY_braille_dots_46 0x1002828
#endif
#ifndef GDK_KEY_New
#define GDK_KEY_New 0x1008ff68
#endif
#ifndef GDK_KEY_Georgian_nar
#define GDK_KEY_Georgian_nar 0x10010dc
#endif
#ifndef GDK_KEY_kana_FU
#define GDK_KEY_kana_FU 0x4cc
#endif
#ifndef GDK_KEY_uacute
#define GDK_KEY_uacute 0x0fa
#endif
#ifndef GDK_KEY_rightpointer
#define GDK_KEY_rightpointer 0xaeb
#endif
#ifndef GDK_KEY_F26
#define GDK_KEY_F26 0xffd7
#endif
#ifndef GDK_KEY_Blue
#define GDK_KEY_Blue 0x1008ffa6
#endif
#ifndef GDK_KEY_ohornhook
#define GDK_KEY_ohornhook 0x1001edf
#endif
#ifndef GDK_KEY_Hibernate
#define GDK_KEY_Hibernate 0x1008ffa8
#endif
#ifndef GDK_KEY_Igrave
#define GDK_KEY_Igrave 0x0cc
#endif
#ifndef GDK_KEY_Arabic_sad
#define GDK_KEY_Arabic_sad 0x5d5
#endif
#ifndef GDK_KEY_Armenian_TO
#define GDK_KEY_Armenian_TO 0x1000539
#endif
#ifndef GDK_KEY_dead_doubleacute
#define GDK_KEY_dead_doubleacute 0xfe59
#endif
#ifndef GDK_KEY_SelectButton
#define GDK_KEY_SelectButton 0x1008ffa0
#endif
#ifndef GDK_KEY_Hangul_J_Kiyeog
#define GDK_KEY_Hangul_J_Kiyeog 0xed4
#endif
#ifndef GDK_KEY_W
#define GDK_KEY_W 0x057
#endif
#ifndef GDK_KEY_elementof
#define GDK_KEY_elementof 0x1002208
#endif
#ifndef GDK_KEY_KP_F3
#define GDK_KEY_KP_F3 0xff93
#endif
#ifndef GDK_KEY_Farsi_2
#define GDK_KEY_Farsi_2 0x10006f2
#endif
#ifndef GDK_KEY_Arabic_kaf
#define GDK_KEY_Arabic_kaf 0x5e3
#endif
#ifndef GDK_KEY_fabovedot
#define GDK_KEY_fabovedot 0x1001e1f
#endif
#ifndef GDK_KEY_News
#define GDK_KEY_News 0x1008ff69
#endif
#ifndef GDK_KEY_cuberoot
#define GDK_KEY_cuberoot 0x100221b
#endif
#ifndef GDK_KEY_enspace
#define GDK_KEY_enspace 0xaa2
#endif
#ifndef GDK_KEY_botrightsummation
#define GDK_KEY_botrightsummation 0x8b6
#endif
#ifndef GDK_KEY_S
#define GDK_KEY_S 0x053
#endif
#ifndef GDK_KEY_kana_u
#define GDK_KEY_kana_u 0x4a9
#endif
#ifndef GDK_KEY_braille_dots_124578
#define GDK_KEY_braille_dots_124578 0x10028db
#endif
#ifndef GDK_KEY_Hangul_Cieuc
#define GDK_KEY_Hangul_Cieuc 0xeba
#endif
#ifndef GDK_KEY_ninesuperior
#define GDK_KEY_ninesuperior 0x1002079
#endif
#ifndef GDK_KEY_Uhornbelowdot
#define GDK_KEY_Uhornbelowdot 0x1001ef0
#endif
#ifndef GDK_KEY_LiraSign
#define GDK_KEY_LiraSign 0x10020a4
#endif
#ifndef GDK_KEY_hebrew_het
#define GDK_KEY_hebrew_het 0xce7
#endif
#ifndef GDK_KEY_Switch_VT_7
#define GDK_KEY_Switch_VT_7 0x1008fe07
#endif
#ifndef GDK_KEY_Greek_mu
#define GDK_KEY_Greek_mu 0x7ec
#endif
#ifndef GDK_KEY_braille_dots_245
#define GDK_KEY_braille_dots_245 0x100281a
#endif
#ifndef GDK_KEY_em3space
#define GDK_KEY_em3space 0xaa3
#endif
#ifndef GDK_KEY_dead_belowcomma
#define GDK_KEY_dead_belowcomma 0xfe6e
#endif
#ifndef GDK_KEY_atilde
#define GDK_KEY_atilde 0x0e3
#endif
#ifndef GDK_KEY_Ccircumflex
#define GDK_KEY_Ccircumflex 0x2c6
#endif
#ifndef GDK_KEY_dead_voiced_sound
#define GDK_KEY_dead_voiced_sound 0xfe5e
#endif
#ifndef GDK_KEY_kana_WA
#define GDK_KEY_kana_WA 0x4dc
#endif
#ifndef GDK_KEY_Find
#define GDK_KEY_Find 0xff68
#endif
#ifndef GDK_KEY_Eth
#define GDK_KEY_Eth 0x0d0
#endif
#ifndef GDK_KEY_downarrow
#define GDK_KEY_downarrow 0x8fe
#endif
#ifndef GDK_KEY_Hangul_EO
#define GDK_KEY_Hangul_EO 0xec3
#endif
#ifndef GDK_KEY_Ungrab
#define GDK_KEY_Ungrab 0x1008fe20
#endif
#ifndef GDK_KEY_Touroku
#define GDK_KEY_Touroku 0xff2b
#endif
#ifndef GDK_KEY_KP_F2
#define GDK_KEY_KP_F2 0xff92
#endif
#ifndef GDK_KEY_Farsi_3
#define GDK_KEY_Farsi_3 0x10006f3
#endif
#ifndef GDK_KEY_braille_dots_134678
#define GDK_KEY_braille_dots_134678 0x10028ed
#endif
#ifndef GDK_KEY_dead_tilde
#define GDK_KEY_dead_tilde 0xfe53
#endif
#ifndef GDK_KEY_Cyrillic_i
#define GDK_KEY_Cyrillic_i 0x6c9
#endif
#ifndef GDK_KEY_Acircumflexhook
#define GDK_KEY_Acircumflexhook 0x1001ea8
#endif
#ifndef GDK_KEY_WheelButton
#define GDK_KEY_WheelButton 0x1008ff88
#endif
#ifndef GDK_KEY_oe
#define GDK_KEY_oe 0x13bd
#endif
#ifndef GDK_KEY_braille_dots_14568
#define GDK_KEY_braille_dots_14568 0x10028b9
#endif
#ifndef GDK_KEY_kana_yu
#define GDK_KEY_kana_yu 0x4ad
#endif
#ifndef GDK_KEY_telephone
#define GDK_KEY_telephone 0xaf9
#endif
#ifndef GDK_KEY_kana_KO
#define GDK_KEY_kana_KO 0x4ba
#endif
#ifndef GDK_KEY_braille_dots_2357
#define GDK_KEY_braille_dots_2357 0x1002856
#endif
#ifndef GDK_KEY_topintegral
#define GDK_KEY_topintegral 0x8a4
#endif
#ifndef GDK_KEY_Arabic_tteh
#define GDK_KEY_Arabic_tteh 0x1000679
#endif
#ifndef GDK_KEY_Pointer_Drag1
#define GDK_KEY_Pointer_Drag1 0xfef5
#endif
#ifndef GDK_KEY_Hangul_PostHanja
#define GDK_KEY_Hangul_PostHanja 0xff3b
#endif
#ifndef GDK_KEY_Pointer_DblClick3
#define GDK_KEY_Pointer_DblClick3 0xfef1
#endif
#ifndef GDK_KEY_G
#define GDK_KEY_G 0x047
#endif
#ifndef GDK_KEY_Arabic_noon
#define GDK_KEY_Arabic_noon 0x5e6
#endif
#ifndef GDK_KEY_Greek_PI
#define GDK_KEY_Greek_PI 0x7d0
#endif
#ifndef GDK_KEY_braille_dots_4568
#define GDK_KEY_braille_dots_4568 0x10028b8
#endif
#ifndef GDK_KEY_braille_dots_38
#define GDK_KEY_braille_dots_38 0x1002884
#endif
#ifndef GDK_KEY_braille_dots_1234
#define GDK_KEY_braille_dots_1234 0x100280f
#endif
#ifndef GDK_KEY_Eisu_Shift
#define GDK_KEY_Eisu_Shift 0xff2f
#endif
#ifndef GDK_KEY_braille_dots_12358
#define GDK_KEY_braille_dots_12358 0x1002897
#endif
#ifndef GDK_KEY_topleftparens
#define GDK_KEY_topleftparens 0x8ab
#endif
#ifndef GDK_KEY_braille_dots_356
#define GDK_KEY_braille_dots_356 0x1002834
#endif
#ifndef GDK_KEY_Hiragana_Katakana
#define GDK_KEY_Hiragana_Katakana 0xff27
#endif
#ifndef GDK_KEY_Thai_khorakhang
#define GDK_KEY_Thai_khorakhang 0xda6
#endif
#ifndef GDK_KEY_Ukrainian_GHE_WITH_UPTURN
#define GDK_KEY_Ukrainian_GHE_WITH_UPTURN 0x6bd
#endif
#ifndef GDK_KEY_MillSign
#define GDK_KEY_MillSign 0x10020a5
#endif
#ifndef GDK_KEY_toprightsqbracket
#define GDK_KEY_toprightsqbracket 0x8a9
#endif
#ifndef GDK_KEY_ecircumflextilde
#define GDK_KEY_ecircumflextilde 0x1001ec5
#endif
#ifndef GDK_KEY_squareroot
#define GDK_KEY_squareroot 0x100221a
#endif
#ifndef GDK_KEY_Hankaku
#define GDK_KEY_Hankaku 0xff29
#endif
#ifndef GDK_KEY_braille_dots_12367
#define GDK_KEY_braille_dots_12367 0x1002867
#endif
#ifndef GDK_KEY_Armenian_HO
#define GDK_KEY_Armenian_HO 0x1000540
#endif
#ifndef GDK_KEY_dead_diaeresis
#define GDK_KEY_dead_diaeresis 0xfe57
#endif
#ifndef GDK_KEY_overline
#define GDK_KEY_overline 0x47e
#endif
#ifndef GDK_KEY_braille_dot_9
#define GDK_KEY_braille_dot_9 0xfff9
#endif
#ifndef GDK_KEY_Uhornhook
#define GDK_KEY_Uhornhook 0x1001eec
#endif
#ifndef GDK_KEY_ninesubscript
#define GDK_KEY_ninesubscript 0x1002089
#endif
#ifndef GDK_KEY_nacute
#define GDK_KEY_nacute 0x1f1
#endif
#ifndef GDK_KEY_notequal
#define GDK_KEY_notequal 0x8bd
#endif
#ifndef GDK_KEY_braille_dots_34678
#define GDK_KEY_braille_dots_34678 0x10028ec
#endif
#ifndef GDK_KEY_Hangul_SsangDikeud
#define GDK_KEY_Hangul_SsangDikeud 0xea8
#endif
#ifndef GDK_KEY_braille_dots_12478
#define GDK_KEY_braille_dots_12478 0x10028cb
#endif
#ifndef GDK_KEY_L6
#define GDK_KEY_L6 0xffcd
#endif
#ifndef GDK_KEY_Cyrillic_je
#define GDK_KEY_Cyrillic_je 0x6a8
#endif
#ifndef GDK_KEY_Ohorn
#define GDK_KEY_Ohorn 0x10001a0
#endif
#ifndef GDK_KEY_ColonSign
#define GDK_KEY_ColonSign 0x10020a1
#endif
#ifndef GDK_KEY_eightsuperior
#define GDK_KEY_eightsuperior 0x1002078
#endif
#ifndef GDK_KEY_approxeq
#define GDK_KEY_approxeq 0x1002248
#endif
#ifndef GDK_KEY_lacute
#define GDK_KEY_lacute 0x1e5
#endif
#ifndef GDK_KEY_ZoomOut
#define GDK_KEY_ZoomOut 0x1008ff8c
#endif
#ifndef GDK_KEY_Cyrillic_ha_descender
#define GDK_KEY_Cyrillic_ha_descender 0x10004b3
#endif
#ifndef GDK_KEY_telephonerecorder
#define GDK_KEY_telephonerecorder 0xafa
#endif
#ifndef GDK_KEY_Tcaron
#define GDK_KEY_Tcaron 0x1ab
#endif
#ifndef GDK_KEY_Mode_switch
#define GDK_KEY_Mode_switch 0xff7e
#endif
#ifndef GDK_KEY_Macedonia_GJE
#define GDK_KEY_Macedonia_GJE 0x6b2
#endif
#ifndef GDK_KEY_Abrevegrave
#define GDK_KEY_Abrevegrave 0x1001eb0
#endif
#ifndef GDK_KEY_Cyrillic_BE
#define GDK_KEY_Cyrillic_BE 0x6e2
#endif
#ifndef GDK_KEY_Cyrillic_ghe
#define GDK_KEY_Cyrillic_ghe 0x6c7
#endif
#ifndef GDK_KEY_braille_dots_1258
#define GDK_KEY_braille_dots_1258 0x1002893
#endif
#ifndef GDK_KEY_Switch_VT_6
#define GDK_KEY_Switch_VT_6 0x1008fe06
#endif
#ifndef GDK_KEY_Armenian_shesht
#define GDK_KEY_Armenian_shesht 0x100055b
#endif
#ifndef GDK_KEY_Cyrillic_ve
#define GDK_KEY_Cyrillic_ve 0x6d7
#endif
#ifndef GDK_KEY_R11
#define GDK_KEY_R11 0xffdc
#endif
#ifndef GDK_KEY_parenright
#define GDK_KEY_parenright 0x029
#endif
#ifndef GDK_KEY_Arabic_feh
#define GDK_KEY_Arabic_feh 0x5e1
#endif
#ifndef GDK_KEY_Send
#define GDK_KEY_Send 0x1008ff7b
#endif
#ifndef GDK_KEY_Greek_accentdieresis
#define GDK_KEY_Greek_accentdieresis 0x7ae
#endif
#ifndef GDK_KEY_FFrancSign
#define GDK_KEY_FFrancSign 0x10020a3
#endif
#ifndef GDK_KEY_Abreve
#define GDK_KEY_Abreve 0x1c3
#endif
#ifndef GDK_KEY_AudioRecord
#define GDK_KEY_AudioRecord 0x1008ff1c
#endif
#ifndef GDK_KEY_Thai_thothong
#define GDK_KEY_Thai_thothong 0xdb8
#endif
#ifndef GDK_KEY_braille_dots_2458
#define GDK_KEY_braille_dots_2458 0x100289a
#endif
#ifndef GDK_KEY_kana_SE
#define GDK_KEY_kana_SE 0x4be
#endif
#ifndef GDK_KEY_Hangul_MultipleCandidate
#define GDK_KEY_Hangul_MultipleCandidate 0xff3d
#endif
#ifndef GDK_KEY_Greek_sigma
#define GDK_KEY_Greek_sigma 0x7f2
#endif
#ifndef GDK_KEY_R10
#define GDK_KEY_R10 0xffdb
#endif
#ifndef GDK_KEY_kana_middledot
#define GDK_KEY_kana_middledot 0x4a5
#endif
#ifndef GDK_KEY_Greek_epsilon
#define GDK_KEY_Greek_epsilon 0x7e5
#endif
#ifndef GDK_KEY_braille_dots_234
#define GDK_KEY_braille_dots_234 0x100280e
#endif
#ifndef GDK_KEY_Hangul_Tieut
#define GDK_KEY_Hangul_Tieut 0xebc
#endif
#ifndef GDK_KEY_braille_dot_5
#define GDK_KEY_braille_dot_5 0xfff5
#endif
#ifndef GDK_KEY_m
#define GDK_KEY_m 0x06d
#endif
#ifndef GDK_KEY_braille_dots_138
#define GDK_KEY_braille_dots_138 0x1002885
#endif
#ifndef GDK_KEY_ISO_Fast_Cursor_Left
#define GDK_KEY_ISO_Fast_Cursor_Left 0xfe2c
#endif
#ifndef GDK_KEY_V
#define GDK_KEY_V 0x056
#endif
#ifndef GDK_KEY_3270_Right2
#define GDK_KEY_3270_Right2 0xfd03
#endif
#ifndef GDK_KEY_Greek_IOTAaccent
#define GDK_KEY_Greek_IOTAaccent 0x7a4
#endif
#ifndef GDK_KEY_Greek_omega
#define GDK_KEY_Greek_omega 0x7f9
#endif
#ifndef GDK_KEY_Georgian_in
#define GDK_KEY_Georgian_in 0x10010d8
#endif
#ifndef GDK_KEY_d
#define GDK_KEY_d 0x064
#endif
#ifndef GDK_KEY_ecircumflex
#define GDK_KEY_ecircumflex 0x0ea
#endif
#ifndef GDK_KEY_acute
#define GDK_KEY_acute 0x0b4
#endif
#ifndef GDK_KEY_Pabovedot
#define GDK_KEY_Pabovedot 0x1001e56
#endif
#ifndef GDK_KEY_threesubscript
#define GDK_KEY_threesubscript 0x1002083
#endif
#ifndef GDK_KEY_PreviousCandidate
#define GDK_KEY_PreviousCandidate 0xff3e
#endif
#ifndef GDK_KEY_marker
#define GDK_KEY_marker 0xabf
#endif
#ifndef GDK_KEY_Ohorngrave
#define GDK_KEY_Ohorngrave 0x1001edc
#endif
#ifndef GDK_KEY_Cyrillic_O
#define GDK_KEY_Cyrillic_O 0x6ef
#endif
#ifndef GDK_KEY_Hangul_J_RieulPhieuf
#define GDK_KEY_Hangul_J_RieulPhieuf 0xee1
#endif
#ifndef GDK_KEY_ISO_Fast_Cursor_Down
#define GDK_KEY_ISO_Fast_Cursor_Down 0xfe2f
#endif
#ifndef GDK_KEY_Time
#define GDK_KEY_Time 0x1008ff9f
#endif
#ifndef GDK_KEY_Pointer_Button4
#define GDK_KEY_Pointer_Button4 0xfeec
#endif
#ifndef GDK_KEY_ocircumflex
#define GDK_KEY_ocircumflex 0x0f4
#endif
#ifndef GDK_KEY_Greek_finalsmallsigma
#define GDK_KEY_Greek_finalsmallsigma 0x7f3
#endif
#ifndef GDK_KEY_Market
#define GDK_KEY_Market 0x1008ff62
#endif
#ifndef GDK_KEY_KP_Separator
#define GDK_KEY_KP_Separator 0xffac
#endif
#ifndef GDK_KEY_Serbian_LJE
#define GDK_KEY_Serbian_LJE 0x6b9
#endif
#ifndef GDK_KEY_c
#define GDK_KEY_c 0x063
#endif
#ifndef GDK_KEY_femalesymbol
#define GDK_KEY_femalesymbol 0xaf8
#endif
#ifndef GDK_KEY_kana_HE
#define GDK_KEY_kana_HE 0x4cd
#endif
#ifndef GDK_KEY_ISO_Level3_Shift
#define GDK_KEY_ISO_Level3_Shift 0xfe03
#endif
#ifndef GDK_KEY_NairaSign
#define GDK_KEY_NairaSign 0x10020a6
#endif
#ifndef GDK_KEY_Cyrillic_shha
#define GDK_KEY_Cyrillic_shha 0x10004bb
#endif
#ifndef GDK_KEY_Video
#define GDK_KEY_Video 0x1008ff87
#endif
#ifndef GDK_KEY_ntilde
#define GDK_KEY_ntilde 0x0f1
#endif
#ifndef GDK_KEY_Greek_alphaaccent
#define GDK_KEY_Greek_alphaaccent 0x7b1
#endif
#ifndef GDK_KEY_enfilledsqbullet
#define GDK_KEY_enfilledsqbullet 0xae7
#endif
#ifndef GDK_KEY_Scedilla
#define GDK_KEY_Scedilla 0x1aa
#endif
#ifndef GDK_KEY_Memo
#define GDK_KEY_Memo 0x1008ff1e
#endif
#ifndef GDK_KEY_Hangul_Khieuq
#define GDK_KEY_Hangul_Khieuq 0xebb
#endif
#ifndef GDK_KEY_braille_dots_3456
#define GDK_KEY_braille_dots_3456 0x100283c
#endif
#ifndef GDK_KEY_Ecaron
#define GDK_KEY_Ecaron 0x1cc
#endif
#ifndef GDK_KEY_Georgian_hoe
#define GDK_KEY_Georgian_hoe 0x10010f5
#endif
#ifndef GDK_KEY_8
#define GDK_KEY_8 0x038
#endif
#ifndef GDK_KEY_Armenian_vo
#define GDK_KEY_Armenian_vo 0x1000578
#endif
#ifndef GDK_KEY_braille_dots_2467
#define GDK_KEY_braille_dots_2467 0x100286a
#endif
#ifndef GDK_KEY_F14
#define GDK_KEY_F14 0xffcb
#endif
#ifndef GDK_KEY_abrevebelowdot
#define GDK_KEY_abrevebelowdot 0x1001eb7
#endif
#ifndef GDK_KEY_Pointer_UpLeft
#define GDK_KEY_Pointer_UpLeft 0xfee4
#endif
#ifndef GDK_KEY_guillemotleft
#define GDK_KEY_guillemotleft 0x0ab
#endif
#ifndef GDK_KEY_kana_MI
#define GDK_KEY_kana_MI 0x4d0
#endif
#ifndef GDK_KEY_Thai_chochan
#define GDK_KEY_Thai_chochan 0xda8
#endif
#ifndef GDK_KEY_Greek_etaaccent
#define GDK_KEY_Greek_etaaccent 0x7b3
#endif
#ifndef GDK_KEY_Armenian_vyun
#define GDK_KEY_Armenian_vyun 0x1000582
#endif
#ifndef GDK_KEY_quoteright
#define GDK_KEY_quoteright 0x027
#endif
#ifndef GDK_KEY_ISO_Level2_Latch
#define GDK_KEY_ISO_Level2_Latch 0xfe02
#endif
#ifndef GDK_KEY_Clear
#define GDK_KEY_Clear 0xff0b
#endif
#ifndef GDK_KEY_threeeighths
#define GDK_KEY_threeeighths 0xac4
#endif
#ifndef GDK_KEY_thorn
#define GDK_KEY_thorn 0x0fe
#endif
#ifndef GDK_KEY_R2
#define GDK_KEY_R2 0xffd3
#endif
#ifndef GDK_KEY_leftradical
#define GDK_KEY_leftradical 0x8a1
#endif
#ifndef GDK_KEY_variation
#define GDK_KEY_variation 0x8c1
#endif
#ifndef GDK_KEY_braille_dots_2567
#define GDK_KEY_braille_dots_2567 0x1002872
#endif
#ifndef GDK_KEY_righttack
#define GDK_KEY_righttack 0xbfc
#endif
#ifndef GDK_KEY_Greek_lambda
#define GDK_KEY_Greek_lambda 0x7eb
#endif
#ifndef GDK_KEY_dead_e
#define GDK_KEY_dead_e 0xfe82
#endif
#ifndef GDK_KEY_asterisk
#define GDK_KEY_asterisk 0x02a
#endif
#ifndef GDK_KEY_Arabic_alef
#define GDK_KEY_Arabic_alef 0x5c7
#endif
#ifndef GDK_KEY_LaunchC
#define GDK_KEY_LaunchC 0x1008ff4c
#endif
#ifndef GDK_KEY_Serbian_dje
#define GDK_KEY_Serbian_dje 0x6a1
#endif
#ifndef GDK_KEY_braille_dots_1267
#define GDK_KEY_braille_dots_1267 0x1002863
#endif
#ifndef GDK_KEY_ScrollDown
#define GDK_KEY_ScrollDown 0x1008ff79
#endif
#ifndef GDK_KEY_Greek_theta
#define GDK_KEY_Greek_theta 0x7e8
#endif
#ifndef GDK_KEY_Armenian_accent
#define GDK_KEY_Armenian_accent 0x100055b
#endif
#ifndef GDK_KEY_Launch0
#define GDK_KEY_Launch0 0x1008ff40
#endif
#ifndef GDK_KEY_F19
#define GDK_KEY_F19 0xffd0
#endif
#ifndef GDK_KEY_Emacron
#define GDK_KEY_Emacron 0x3aa
#endif
#ifndef GDK_KEY_Macedonia_DSE
#define GDK_KEY_Macedonia_DSE 0x6b5
#endif
#ifndef GDK_KEY_Alt_R
#define GDK_KEY_Alt_R 0xffea
#endif
#ifndef GDK_KEY_fourthroot
#define GDK_KEY_fourthroot 0x100221c
#endif
#ifndef GDK_KEY_backslash
#define GDK_KEY_backslash 0x05c
#endif
#ifndef GDK_KEY_kana_TU
#define GDK_KEY_kana_TU 0x4c2
#endif
#ifndef GDK_KEY_Armenian_hi
#define GDK_KEY_Armenian_hi 0x1000575
#endif
#ifndef GDK_KEY_wgrave
#define GDK_KEY_wgrave 0x1001e81
#endif
#ifndef GDK_KEY_3270_Left2
#define GDK_KEY_3270_Left2 0xfd04
#endif
#ifndef GDK_KEY_Greek_TAU
#define GDK_KEY_Greek_TAU 0x7d4
#endif
#ifndef GDK_KEY_Ybelowdot
#define GDK_KEY_Ybelowdot 0x1001ef4
#endif
#ifndef GDK_KEY_Georgian_zhar
#define GDK_KEY_Georgian_zhar 0x10010df
#endif
#ifndef GDK_KEY_Word
#define GDK_KEY_Word 0x1008ff89
#endif
#ifndef GDK_KEY_F3
#define GDK_KEY_F3 0xffc0
#endif
#ifndef GDK_KEY_dstroke
#define GDK_KEY_dstroke 0x1f0
#endif
#ifndef GDK_KEY_Greek_zeta
#define GDK_KEY_Greek_zeta 0x7e6
#endif
#ifndef GDK_KEY_Macedonia_dse
#define GDK_KEY_Macedonia_dse 0x6a5
#endif
#ifndef GDK_KEY_Hangul_Codeinput
#define GDK_KEY_Hangul_Codeinput 0xff37
#endif
#ifndef GDK_KEY_kana_NU
#define GDK_KEY_kana_NU 0x4c7
#endif
#ifndef GDK_KEY_Thai_kokai
#define GDK_KEY_Thai_kokai 0xda1
#endif
#ifndef GDK_KEY_dintegral
#define GDK_KEY_dintegral 0x100222c
#endif
#ifndef GDK_KEY_EcuSign
#define GDK_KEY_EcuSign 0x10020a0
#endif
#ifndef GDK_KEY_Hangul_Banja
#define GDK_KEY_Hangul_Banja 0xff39
#endif
#ifndef GDK_KEY_abrevegrave
#define GDK_KEY_abrevegrave 0x1001eb1
#endif
#ifndef GDK_KEY_Display
#define GDK_KEY_Display 0x1008ff59
#endif
#ifndef GDK_KEY_percent
#define GDK_KEY_percent 0x025
#endif
#ifndef GDK_KEY_Thai_phinthu
#define GDK_KEY_Thai_phinthu 0xdda
#endif
#ifndef GDK_KEY_oneeighth
#define GDK_KEY_oneeighth 0xac3
#endif
#ifndef GDK_KEY_ISO_Next_Group
#define GDK_KEY_ISO_Next_Group 0xfe08
#endif
#ifndef GDK_KEY_Georgian_don
#define GDK_KEY_Georgian_don 0x10010d3
#endif
#ifndef GDK_KEY_ocircumflexgrave
#define GDK_KEY_ocircumflexgrave 0x1001ed3
#endif
#ifndef GDK_KEY_voicedsound
#define GDK_KEY_voicedsound 0x4de
#endif
#ifndef GDK_KEY_dead_abovering
#define GDK_KEY_dead_abovering 0xfe58
#endif
#ifndef GDK_KEY_RupeeSign
#define GDK_KEY_RupeeSign 0x10020a8
#endif
#ifndef GDK_KEY_kana_openingbracket
#define GDK_KEY_kana_openingbracket 0x4a2
#endif
#ifndef GDK_KEY_Armenian_TSA
#define GDK_KEY_Armenian_TSA 0x100053e
#endif
#ifndef GDK_KEY_braille_dots_126
#define GDK_KEY_braille_dots_126 0x1002823
#endif
#ifndef GDK_KEY_braille_dots_45678
#define GDK_KEY_braille_dots_45678 0x10028f8
#endif
#ifndef GDK_KEY_equal
#define GDK_KEY_equal 0x03d
#endif
#ifndef GDK_KEY_Cyrillic_DE
#define GDK_KEY_Cyrillic_DE 0x6e4
#endif
#ifndef GDK_KEY_semivoicedsound
#define GDK_KEY_semivoicedsound 0x4df
#endif
#ifndef GDK_KEY_Abrevehook
#define GDK_KEY_Abrevehook 0x1001eb2
#endif
#ifndef GDK_KEY_Ygrave
#define GDK_KEY_Ygrave 0x1001ef2
#endif
#ifndef GDK_KEY_notidentical
#define GDK_KEY_notidentical 0x1002262
#endif
#ifndef GDK_KEY_gbreve
#define GDK_KEY_gbreve 0x2bb
#endif
#ifndef GDK_KEY_Cyrillic_de
#define GDK_KEY_Cyrillic_de 0x6c4
#endif
#ifndef GDK_KEY_Uring
#define GDK_KEY_Uring 0x1d9
#endif
#ifndef GDK_KEY_lowleftcorner
#define GDK_KEY_lowleftcorner 0x9ed
#endif
#ifndef GDK_KEY_twothirds
#define GDK_KEY_twothirds 0xab1
#endif
#ifndef GDK_KEY_braille_dots_12568
#define GDK_KEY_braille_dots_12568 0x10028b3
#endif
#ifndef GDK_KEY_uparrow
#define GDK_KEY_uparrow 0x8fc
#endif
#ifndef GDK_KEY_braille_dots_345678
#define GDK_KEY_braille_dots_345678 0x10028fc
#endif
#ifndef GDK_KEY_onequarter
#define GDK_KEY_onequarter 0x0bc
#endif
#ifndef GDK_KEY_Uogonek
#define GDK_KEY_Uogonek 0x3d9
#endif
#ifndef GDK_KEY_AudioRepeat
#define GDK_KEY_AudioRepeat 0x1008ff98
#endif
#ifndef GDK_KEY_hebrew_daleth
#define GDK_KEY_hebrew_daleth 0xce3
#endif
#ifndef GDK_KEY_KP_Next
#define GDK_KEY_KP_Next 0xff9b
#endif
#ifndef GDK_KEY_braille_dots_158
#define GDK_KEY_braille_dots_158 0x1002891
#endif
#ifndef GDK_KEY_Greek_XI
#define GDK_KEY_Greek_XI 0x7ce
#endif
#ifndef GDK_KEY_minutes
#define GDK_KEY_minutes 0xad6
#endif
#ifndef GDK_KEY_emfilledrect
#define GDK_KEY_emfilledrect 0xadf
#endif
#ifndef GDK_KEY_sixsuperior
#define GDK_KEY_sixsuperior 0x1002076
#endif
#ifndef GDK_KEY_Game
#define GDK_KEY_Game 0x1008ff5e
#endif
#ifndef GDK_KEY_N
#define GDK_KEY_N 0x04e
#endif
#ifndef GDK_KEY_Cyrillic_ka_descender
#define GDK_KEY_Cyrillic_ka_descender 0x100049b
#endif
#ifndef GDK_KEY_braille_dots_12345
#define GDK_KEY_braille_dots_12345 0x100281f
#endif
#ifndef GDK_KEY_Cyrillic_KA_vertstroke
#define GDK_KEY_Cyrillic_KA_vertstroke 0x100049c
#endif
#ifndef GDK_KEY_Macedonia_gje
#define GDK_KEY_Macedonia_gje 0x6a2
#endif
#ifndef GDK_KEY_Iacute
#define GDK_KEY_Iacute 0x0cd
#endif
#ifndef GDK_KEY_braille_dots_135678
#define GDK_KEY_braille_dots_135678 0x10028f5
#endif
#ifndef GDK_KEY_Hiragana
#define GDK_KEY_Hiragana 0xff25
#endif
#ifndef GDK_KEY_braille_dots_2
#define GDK_KEY_braille_dots_2 0x1002802
#endif
#ifndef GDK_KEY_Ukrainian_I
#define GDK_KEY_Ukrainian_I 0x6b6
#endif
#ifndef GDK_KEY_Cyrillic_e
#define GDK_KEY_Cyrillic_e 0x6dc
#endif
#ifndef GDK_KEY_braille_dots_16
#define GDK_KEY_braille_dots_16 0x1002821
#endif
#ifndef GDK_KEY_dead_acute
#define GDK_KEY_dead_acute 0xfe51
#endif
#ifndef GDK_KEY_ucircumflex
#define GDK_KEY_ucircumflex 0x0fb
#endif
#ifndef GDK_KEY_onesixth
#define GDK_KEY_onesixth 0xab6
#endif
#ifndef GDK_KEY_Arabic_7
#define GDK_KEY_Arabic_7 0x1000667
#endif
#ifndef GDK_KEY_braille_dots_13458
#define GDK_KEY_braille_dots_13458 0x100289d
#endif
#ifndef GDK_KEY_Eject
#define GDK_KEY_Eject 0x1008ff2c
#endif
#ifndef GDK_KEY_ograve
#define GDK_KEY_ograve 0x0f2
#endif
#ifndef GDK_KEY_ETH
#define GDK_KEY_ETH 0x0d0
#endif
#ifndef GDK_KEY_Shop
#define GDK_KEY_Shop 0x1008ff36
#endif
#ifndef GDK_KEY_VoidSymbol
#define GDK_KEY_VoidSymbol 0xffffff
#endif
#ifndef GDK_KEY_Hangul_YE
#define GDK_KEY_Hangul_YE 0xec6
#endif
#ifndef GDK_KEY_braille_dots_35678
#define GDK_KEY_braille_dots_35678 0x10028f4
#endif
#ifndef GDK_KEY_Serbian_TSHE
#define GDK_KEY_Serbian_TSHE 0x6bb
#endif
#ifndef GDK_KEY_Thai_leksam
#define GDK_KEY_Thai_leksam 0xdf3
#endif
#ifndef GDK_KEY_Away
#define GDK_KEY_Away 0x1008ff8d
#endif
#ifndef GDK_KEY_Rcedilla
#define GDK_KEY_Rcedilla 0x3a3
#endif
#ifndef GDK_KEY_WakeUp
#define GDK_KEY_WakeUp 0x1008ff2b
#endif
#ifndef GDK_KEY_F7
#define GDK_KEY_F7 0xffc4
#endif
#ifndef GDK_KEY_Reload
#define GDK_KEY_Reload 0x1008ff73
#endif
#ifndef GDK_KEY_Arabic_noon_ghunna
#define GDK_KEY_Arabic_noon_ghunna 0x10006ba
#endif
#ifndef GDK_KEY_braille_dots_12346
#define GDK_KEY_braille_dots_12346 0x100282f
#endif
#ifndef GDK_KEY_Switch_VT_10
#define GDK_KEY_Switch_VT_10 0x1008fe0a
#endif
#ifndef GDK_KEY_braille_dots_24567
#define GDK_KEY_braille_dots_24567 0x100287a
#endif
#ifndef GDK_KEY_Gbreve
#define GDK_KEY_Gbreve 0x2ab
#endif
#ifndef GDK_KEY_F30
#define GDK_KEY_F30 0xffdb
#endif
#ifndef GDK_KEY_Serbian_je
#define GDK_KEY_Serbian_je 0x6a8
#endif
#ifndef GDK_KEY_F21
#define GDK_KEY_F21 0xffd2
#endif
#ifndef GDK_KEY_Greek_EPSILON
#define GDK_KEY_Greek_EPSILON 0x7c5
#endif
#ifndef GDK_KEY_ISO_Level3_Latch
#define GDK_KEY_ISO_Level3_Latch 0xfe04
#endif
#ifndef GDK_KEY_Thai_nonu
#define GDK_KEY_Thai_nonu 0xdb9
#endif
#ifndef GDK_KEY_Thai_sorusi
#define GDK_KEY_Thai_sorusi 0xdc9
#endif
#ifndef GDK_KEY_THORN
#define GDK_KEY_THORN 0x0de
#endif
#ifndef GDK_KEY_Cyrillic_O_bar
#define GDK_KEY_Cyrillic_O_bar 0x10004e8
#endif
#ifndef GDK_KEY_Greek_lamda
#define GDK_KEY_Greek_lamda 0x7eb
#endif
#ifndef GDK_KEY_Armenian_dza
#define GDK_KEY_Armenian_dza 0x1000571
#endif
#ifndef GDK_KEY_tintegral
#define GDK_KEY_tintegral 0x100222d
#endif
#ifndef GDK_KEY_Zacute
#define GDK_KEY_Zacute 0x1ac
#endif
#ifndef GDK_KEY_dead_ogonek
#define GDK_KEY_dead_ogonek 0xfe5c
#endif
#ifndef GDK_KEY_Greek_upsilonaccent
#define GDK_KEY_Greek_upsilonaccent 0x7b8
#endif
#ifndef GDK_KEY_Launch5
#define GDK_KEY_Launch5 0x1008ff45
#endif
#ifndef GDK_KEY_Armenian_da
#define GDK_KEY_Armenian_da 0x1000564
#endif
#ifndef GDK_KEY_braille_dots_1467
#define GDK_KEY_braille_dots_1467 0x1002869
#endif
#ifndef GDK_KEY_Hangul_PreHanja
#define GDK_KEY_Hangul_PreHanja 0xff3a
#endif
#ifndef GDK_KEY_Greek_CHI
#define GDK_KEY_Greek_CHI 0x7d7
#endif
#ifndef GDK_KEY_braille_dot_2
#define GDK_KEY_braille_dot_2 0xfff2
#endif
#ifndef GDK_KEY_braille_dots_12456
#define GDK_KEY_braille_dots_12456 0x100283b
#endif
#ifndef GDK_KEY_Launch6
#define GDK_KEY_Launch6 0x1008ff46
#endif
#ifndef GDK_KEY_punctspace
#define GDK_KEY_punctspace 0xaa6
#endif
#ifndef GDK_KEY_Ukranian_yi
#define GDK_KEY_Ukranian_yi 0x6a7
#endif
#ifndef GDK_KEY_Arabic_8
#define GDK_KEY_Arabic_8 0x1000668
#endif
#ifndef GDK_KEY_thinspace
#define GDK_KEY_thinspace 0xaa7
#endif
#ifndef GDK_KEY_Armenian_ho
#define GDK_KEY_Armenian_ho 0x1000570
#endif
#ifndef GDK_KEY_uhornbelowdot
#define GDK_KEY_uhornbelowdot 0x1001ef1
#endif
#ifndef GDK_KEY_R14
#define GDK_KEY_R14 0xffdf
#endif
#ifndef GDK_KEY_WLAN
#define GDK_KEY_WLAN 0x1008ff95
#endif
#ifndef GDK_KEY_includes
#define GDK_KEY_includes 0x8db
#endif
#ifndef GDK_KEY_braille_dots_3467
#define GDK_KEY_braille_dots_3467 0x100286c
#endif
#ifndef GDK_KEY_Thai_maichattawa
#define GDK_KEY_Thai_maichattawa 0xdeb
#endif
#ifndef GDK_KEY_Farsi_8
#define GDK_KEY_Farsi_8 0x10006f8
#endif
#ifndef GDK_KEY_Arabic_fatha
#define GDK_KEY_Arabic_fatha 0x5ee
#endif
#ifndef GDK_KEY_braille_dots_267
#define GDK_KEY_braille_dots_267 0x1002862
#endif
#ifndef GDK_KEY_Armenian_FE
#define GDK_KEY_Armenian_FE 0x1000556
#endif
#ifndef GDK_KEY_Tab
#define GDK_KEY_Tab 0xff09
#endif
#ifndef GDK_KEY_Arabic_2
#define GDK_KEY_Arabic_2 0x1000662
#endif
#ifndef GDK_KEY_wacute
#define GDK_KEY_wacute 0x1001e83
#endif
#ifndef GDK_KEY_FrameForward
#define GDK_KEY_FrameForward 0x1008ff9e
#endif
#ifndef GDK_KEY_Dabovedot
#define GDK_KEY_Dabovedot 0x1001e0a
#endif
#ifndef GDK_KEY_parenleft
#define GDK_KEY_parenleft 0x028
#endif
#ifndef GDK_KEY_braille_dots_2478
#define GDK_KEY_braille_dots_2478 0x10028ca
#endif
#ifndef GDK_KEY_Greek_xi
#define GDK_KEY_Greek_xi 0x7ee
#endif
#ifndef GDK_KEY_minus
#define GDK_KEY_minus 0x02d
#endif
#ifndef GDK_KEY_braille_dots_235678
#define GDK_KEY_braille_dots_235678 0x10028f6
#endif
#ifndef GDK_KEY_Arabic_peh
#define GDK_KEY_Arabic_peh 0x100067e
#endif
#ifndef GDK_KEY_braille_dots_123468
#define GDK_KEY_braille_dots_123468 0x10028af
#endif
#ifndef GDK_KEY_hexagram
#define GDK_KEY_hexagram 0xada
#endif
#ifndef GDK_KEY_soliddiamond
#define GDK_KEY_soliddiamond 0x9e0
#endif
#ifndef GDK_KEY_Georgian_khar
#define GDK_KEY_Georgian_khar 0x10010e5
#endif
#ifndef GDK_KEY_Thai_leksun
#define GDK_KEY_Thai_leksun 0xdf0
#endif
#ifndef GDK_KEY_KP_Begin
#define GDK_KEY_KP_Begin 0xff9d
#endif
#ifndef GDK_KEY_Cyrillic_ie
#define GDK_KEY_Cyrillic_ie 0x6c5
#endif
#ifndef GDK_KEY_Hangul_J_NieunJieuj
#define GDK_KEY_Hangul_J_NieunJieuj 0xed8
#endif
#ifndef GDK_KEY_integral
#define GDK_KEY_integral 0x8bf
#endif
#ifndef GDK_KEY_braille_dots_1378
#define GDK_KEY_braille_dots_1378 0x10028c5
#endif
#ifndef GDK_KEY_RotationPB
#define GDK_KEY_RotationPB 0x1008ff75
#endif
#ifndef GDK_KEY_Thai_bobaimai
#define GDK_KEY_Thai_bobaimai 0xdba
#endif
#ifndef GDK_KEY_Armenian_HI
#define GDK_KEY_Armenian_HI 0x1000545
#endif
#ifndef GDK_KEY_Hangul_U
#define GDK_KEY_Hangul_U 0xecc
#endif
#ifndef GDK_KEY_3270_BackTab
#define GDK_KEY_3270_BackTab 0xfd05
#endif
#ifndef GDK_KEY_signifblank
#define GDK_KEY_signifblank 0xaac
#endif
#ifndef GDK_KEY_RockerEnter
#define GDK_KEY_RockerEnter 0x1008ff25
#endif
#ifndef GDK_KEY_Next_Virtual_Screen
#define GDK_KEY_Next_Virtual_Screen 0xfed2
#endif
#ifndef GDK_KEY_notelementof
#define GDK_KEY_notelementof 0x1002209
#endif
#ifndef GDK_KEY_Armenian_vev
#define GDK_KEY_Armenian_vev 0x100057e
#endif
#ifndef GDK_KEY_Thai_topatak
#define GDK_KEY_Thai_topatak 0xdaf
#endif
#ifndef GDK_KEY_braille_dots_45
#define GDK_KEY_braille_dots_45 0x1002818
#endif
#ifndef GDK_KEY_Itilde
#define GDK_KEY_Itilde 0x3a5
#endif
#ifndef GDK_KEY_dead_macron
#define GDK_KEY_dead_macron 0xfe54
#endif
#ifndef GDK_KEY_braille_dots_145
#define GDK_KEY_braille_dots_145 0x1002819
#endif
#ifndef GDK_KEY_braille_dots_17
#define GDK_KEY_braille_dots_17 0x1002841
#endif
#ifndef GDK_KEY_currency
#define GDK_KEY_currency 0x0a4
#endif
#ifndef GDK_KEY_hyphen
#define GDK_KEY_hyphen 0x0ad
#endif
#ifndef GDK_KEY_Ohook
#define GDK_KEY_Ohook 0x1001ece
#endif
#ifndef GDK_KEY_Suspend
#define GDK_KEY_Suspend 0x1008ffa7
#endif
#ifndef GDK_KEY_Henkan_Mode
#define GDK_KEY_Henkan_Mode 0xff23
#endif
#ifndef GDK_KEY_therefore
#define GDK_KEY_therefore 0x8c0
#endif
#ifndef GDK_KEY_Greek_ALPHA
#define GDK_KEY_Greek_ALPHA 0x7c1
#endif
#ifndef GDK_KEY_ecircumflexbelowdot
#define GDK_KEY_ecircumflexbelowdot 0x1001ec7
#endif
#ifndef GDK_KEY_Hangul_RieulYeorinHieuh
#define GDK_KEY_Hangul_RieulYeorinHieuh 0xeef
#endif
#ifndef GDK_KEY_ooblique
#define GDK_KEY_ooblique 0x0f8
#endif
#ifndef GDK_KEY_kana_ME
#define GDK_KEY_kana_ME 0x4d2
#endif
#ifndef GDK_KEY_ecircumflexacute
#define GDK_KEY_ecircumflexacute 0x1001ebf
#endif
#ifndef GDK_KEY_Thai_sarae
#define GDK_KEY_Thai_sarae 0xde0
#endif
#ifndef GDK_KEY_Udoubleacute
#define GDK_KEY_Udoubleacute 0x1db
#endif
#ifndef GDK_KEY_Georgian_cil
#define GDK_KEY_Georgian_cil 0x10010ec
#endif
#ifndef GDK_KEY_R
#define GDK_KEY_R 0x052
#endif
#ifndef GDK_KEY_M
#define GDK_KEY_M 0x04d
#endif
#ifndef GDK_KEY_braille_dots_1234678
#define GDK_KEY_braille_dots_1234678 0x10028ef
#endif
#ifndef GDK_KEY_AudibleBell_Enable
#define GDK_KEY_AudibleBell_Enable 0xfe7a
#endif
#ifndef GDK_KEY_Thai_thophuthao
#define GDK_KEY_Thai_thophuthao 0xdb2
#endif
#ifndef GDK_KEY_Pointer_Left
#define GDK_KEY_Pointer_Left 0xfee0
#endif
#ifndef GDK_KEY_leftsinglequotemark
#define GDK_KEY_leftsinglequotemark 0xad0
#endif
#ifndef GDK_KEY_hebrew_tet
#define GDK_KEY_hebrew_tet 0xce8
#endif
#ifndef GDK_KEY_Georgian_en
#define GDK_KEY_Georgian_en 0x10010d4
#endif
#ifndef GDK_KEY_p
#define GDK_KEY_p 0x070
#endif
#ifndef GDK_KEY_dead_cedilla
#define GDK_KEY_dead_cedilla 0xfe5b
#endif
#ifndef GDK_KEY_Thai_lekpaet
#define GDK_KEY_Thai_lekpaet 0xdf8
#endif
#ifndef GDK_KEY_Greek_ETA
#define GDK_KEY_Greek_ETA 0x7c7
#endif
#ifndef GDK_KEY_braille_dots_13568
#define GDK_KEY_braille_dots_13568 0x10028b5
#endif
#ifndef GDK_KEY_ScreenSaver
#define GDK_KEY_ScreenSaver 0x1008ff2d
#endif
#ifndef GDK_KEY_braille_dots_34578
#define GDK_KEY_braille_dots_34578 0x10028dc
#endif
#ifndef GDK_KEY_Abrevetilde
#define GDK_KEY_Abrevetilde 0x1001eb4
#endif
#ifndef GDK_KEY_braille_dots_238
#define GDK_KEY_braille_dots_238 0x1002886
#endif
#ifndef GDK_KEY_NewSheqelSign
#define GDK_KEY_NewSheqelSign 0x10020aa
#endif
#ifndef GDK_KEY_asciicircum
#define GDK_KEY_asciicircum 0x05e
#endif
#ifndef GDK_KEY_braille_dots_1237
#define GDK_KEY_braille_dots_1237 0x1002847
#endif
#ifndef GDK_KEY_Pointer_DblClick4
#define GDK_KEY_Pointer_DblClick4 0xfef2
#endif
#ifndef GDK_KEY_eng
#define GDK_KEY_eng 0x3bf
#endif
#ifndef GDK_KEY_Hangul_J_Pieub
#define GDK_KEY_Hangul_J_Pieub 0xee4
#endif
#ifndef GDK_KEY_braille_dots_1468
#define GDK_KEY_braille_dots_1468 0x10028a9
#endif
#ifndef GDK_KEY_3270_Ident
#define GDK_KEY_3270_Ident 0xfd13
#endif
#ifndef GDK_KEY_uhorngrave
#define GDK_KEY_uhorngrave 0x1001eeb
#endif
#ifndef GDK_KEY_braille_dots_23468
#define GDK_KEY_braille_dots_23468 0x10028ae
#endif
#ifndef GDK_KEY_braille_dots_1268
#define GDK_KEY_braille_dots_1268 0x10028a3
#endif
#ifndef GDK_KEY_Arabic_fullstop
#define GDK_KEY_Arabic_fullstop 0x10006d4
#endif
#ifndef GDK_KEY_R7
#define GDK_KEY_R7 0xffd8
#endif
#ifndef GDK_KEY_Greek_upsilon
#define GDK_KEY_Greek_upsilon 0x7f5
#endif
#ifndef GDK_KEY_zerosuperior
#define GDK_KEY_zerosuperior 0x1002070
#endif
#ifndef GDK_KEY_L9
#define GDK_KEY_L9 0xffd0
#endif
#ifndef GDK_KEY_Greek_LAMBDA
#define GDK_KEY_Greek_LAMBDA 0x7cb
#endif
#ifndef GDK_KEY_Hangul_J_SsangSios
#define GDK_KEY_Hangul_J_SsangSios 0xee7
#endif
#ifndef GDK_KEY_Ocircumflexbelowdot
#define GDK_KEY_Ocircumflexbelowdot 0x1001ed8
#endif
#ifndef GDK_KEY_Hangul_Sios
#define GDK_KEY_Hangul_Sios 0xeb5
#endif
#ifndef GDK_KEY_Arabic_zah
#define GDK_KEY_Arabic_zah 0x5d8
#endif
#ifndef GDK_KEY_Imacron
#define GDK_KEY_Imacron 0x3cf
#endif
#ifndef GDK_KEY_Hangul_J_KkogjiDalrinIeung
#define GDK_KEY_Hangul_J_KkogjiDalrinIeung 0xef9
#endif
#ifndef GDK_KEY_Armenian_ini
#define GDK_KEY_Armenian_ini 0x100056b
#endif
#ifndef GDK_KEY_braille_dots_135
#define GDK_KEY_braille_dots_135 0x1002815
#endif
#ifndef GDK_KEY_3270_PA3
#define GDK_KEY_3270_PA3 0xfd0c
#endif
#ifndef GDK_KEY_Cyrillic_IO
#define GDK_KEY_Cyrillic_IO 0x6b3
#endif
#ifndef GDK_KEY_F10
#define GDK_KEY_F10 0xffc7
#endif
#ifndef GDK_KEY_Armenian_pe
#define GDK_KEY_Armenian_pe 0x100057a
#endif
#ifndef GDK_KEY_Hebrew_switch
#define GDK_KEY_Hebrew_switch 0xff7e
#endif
#ifndef GDK_KEY_braille_dots_1567
#define GDK_KEY_braille_dots_1567 0x1002871
#endif
#ifndef GDK_KEY_Cyrillic_IE
#define GDK_KEY_Cyrillic_IE 0x6e5
#endif
#ifndef GDK_KEY_kana_comma
#define GDK_KEY_kana_comma 0x4a4
#endif
#ifndef GDK_KEY_Hangul_Mieum
#define GDK_KEY_Hangul_Mieum 0xeb1
#endif
#ifndef GDK_KEY_Hangul_SingleCandidate
#define GDK_KEY_Hangul_SingleCandidate 0xff3c
#endif
#ifndef GDK_KEY_KP_6
#define GDK_KEY_KP_6 0xffb6
#endif
#ifndef GDK_KEY_F22
#define GDK_KEY_F22 0xffd3
#endif
#ifndef GDK_KEY_Pointer_Button3
#define GDK_KEY_Pointer_Button3 0xfeeb
#endif
#ifndef GDK_KEY_rcedilla
#define GDK_KEY_rcedilla 0x3b3
#endif
#ifndef GDK_KEY_braille_dots_1246
#define GDK_KEY_braille_dots_1246 0x100282b
#endif
#ifndef GDK_KEY_4
#define GDK_KEY_4 0x034
#endif
#ifndef GDK_KEY_Greek_kappa
#define GDK_KEY_Greek_kappa 0x7ea
#endif
#ifndef GDK_KEY_Hangul_OE
#define GDK_KEY_Hangul_OE 0xeca
#endif
#ifndef GDK_KEY_Thai_maiyamok
#define GDK_KEY_Thai_maiyamok 0xde6
#endif
#ifndef GDK_KEY_Cyrillic_ef
#define GDK_KEY_Cyrillic_ef 0x6c6
#endif
#ifndef GDK_KEY_Cyrillic_er
#define GDK_KEY_Cyrillic_er 0x6d2
#endif
#ifndef GDK_KEY_k
#define GDK_KEY_k 0x06b
#endif
#ifndef GDK_KEY_Arabic_madda_above
#define GDK_KEY_Arabic_madda_above 0x1000653
#endif
#ifndef GDK_KEY_braille_dots_1348
#define GDK_KEY_braille_dots_1348 0x100288d
#endif
#ifndef GDK_KEY_v
#define GDK_KEY_v 0x076
#endif
#ifndef GDK_KEY_K
#define GDK_KEY_K 0x04b
#endif
#ifndef GDK_KEY_Hyper_R
#define GDK_KEY_Hyper_R 0xffee
#endif
#ifndef GDK_KEY_implies
#define GDK_KEY_implies 0x8ce
#endif
#ifndef GDK_KEY_foursubscript
#define GDK_KEY_foursubscript 0x1002084
#endif
#ifndef GDK_KEY_braille_dots_25
#define GDK_KEY_braille_dots_25 0x1002812
#endif
#ifndef GDK_KEY_Armenian_za
#define GDK_KEY_Armenian_za 0x1000566
#endif
#ifndef GDK_KEY_R15
#define GDK_KEY_R15 0xffe0
#endif
#ifndef GDK_KEY_intersection
#define GDK_KEY_intersection 0x8dc
#endif
#ifndef GDK_KEY_Greek_BETA
#define GDK_KEY_Greek_BETA 0x7c2
#endif
#ifndef GDK_KEY_BrightnessAdjust
#define GDK_KEY_BrightnessAdjust 0x1008ff3b
#endif
#ifndef GDK_KEY_R8
#define GDK_KEY_R8 0xffd9
#endif
#ifndef GDK_KEY_tcaron
#define GDK_KEY_tcaron 0x1bb
#endif
#ifndef GDK_KEY_Arabic_superscript_alef
#define GDK_KEY_Arabic_superscript_alef 0x1000670
#endif
#ifndef GDK_KEY_enopensquarebullet
#define GDK_KEY_enopensquarebullet 0xae1
#endif
#ifndef GDK_KEY_Close
#define GDK_KEY_Close 0x1008ff56
#endif
#ifndef GDK_KEY_Hcircumflex
#define GDK_KEY_Hcircumflex 0x2a6
#endif
#ifndef GDK_KEY_ISO_Last_Group_Lock
#define GDK_KEY_ISO_Last_Group_Lock 0xfe0f
#endif
#ifndef GDK_KEY_KbdBrightnessUp
#define GDK_KEY_KbdBrightnessUp 0x1008ff05
#endif
#ifndef GDK_KEY_3270_Play
#define GDK_KEY_3270_Play 0xfd16
#endif
#ifndef GDK_KEY_hebrew_finalpe
#define GDK_KEY_hebrew_finalpe 0xcf3
#endif
#ifndef GDK_KEY_braille_dots_457
#define GDK_KEY_braille_dots_457 0x1002858
#endif
#ifndef GDK_KEY_F8
#define GDK_KEY_F8 0xffc5
#endif
#ifndef GDK_KEY_Eisu_toggle
#define GDK_KEY_Eisu_toggle 0xff30
#endif
#ifndef GDK_KEY_Georgian_kan
#define GDK_KEY_Georgian_kan 0x10010d9
#endif
#ifndef GDK_KEY_dead_belowcircumflex
#define GDK_KEY_dead_belowcircumflex 0xfe69
#endif
#ifndef GDK_KEY_Arabic_zain
#define GDK_KEY_Arabic_zain 0x5d2
#endif
#ifndef GDK_KEY_Greek_IOTAdieresis
#define GDK_KEY_Greek_IOTAdieresis 0x7a5
#endif
#ifndef GDK_KEY_Cut
#define GDK_KEY_Cut 0x1008ff58
#endif
#ifndef GDK_KEY_Hangul_YA
#define GDK_KEY_Hangul_YA 0xec1
#endif
#ifndef GDK_KEY_Armenian_tyun
#define GDK_KEY_Armenian_tyun 0x100057f
#endif
#ifndef GDK_KEY_Georgian_char
#define GDK_KEY_Georgian_char 0x10010ed
#endif
#ifndef GDK_KEY_threefifths
#define GDK_KEY_threefifths 0xab4
#endif
#ifndef GDK_KEY_Meeting
#define GDK_KEY_Meeting 0x1008ff63
#endif
#ifndef GDK_KEY_space
#define GDK_KEY_space 0x020
#endif
#ifndef GDK_KEY_zcaron
#define GDK_KEY_zcaron 0x1be
#endif
#ifndef GDK_KEY_Cyrillic_E
#define GDK_KEY_Cyrillic_E 0x6fc
#endif
#ifndef GDK_KEY_Thai_ru
#define GDK_KEY_Thai_ru 0xdc4
#endif
#ifndef GDK_KEY_braille_dots_67
#define GDK_KEY_braille_dots_67 0x1002860
#endif
#ifndef GDK_KEY_WWW
#define GDK_KEY_WWW 0x1008ff2e
#endif
#ifndef GDK_KEY_downstile
#define GDK_KEY_downstile 0xbc4
#endif
#ifndef GDK_KEY_Hangul_J_NieunHieuh
#define GDK_KEY_Hangul_J_NieunHieuh 0xed9
#endif
#ifndef GDK_KEY_bott
#define GDK_KEY_bott 0x9f6
#endif
#ifndef GDK_KEY_Thai_khokhuat
#define GDK_KEY_Thai_khokhuat 0xda3
#endif
#ifndef GDK_KEY_Cyrillic_HA_descender
#define GDK_KEY_Cyrillic_HA_descender 0x10004b2
#endif
#ifndef GDK_KEY_Greek_gamma
#define GDK_KEY_Greek_gamma 0x7e3
#endif
#ifndef GDK_KEY_obelowdot
#define GDK_KEY_obelowdot 0x1001ecd
#endif
#ifndef GDK_KEY_cabovedot
#define GDK_KEY_cabovedot 0x2e5
#endif
#ifndef GDK_KEY_ccircumflex
#define GDK_KEY_ccircumflex 0x2e6
#endif
#ifndef GDK_KEY_Greek_pi
#define GDK_KEY_Greek_pi 0x7f0
#endif
#ifndef GDK_KEY_Thai_sosua
#define GDK_KEY_Thai_sosua 0xdca
#endif
#ifndef GDK_KEY_tcedilla
#define GDK_KEY_tcedilla 0x1fe
#endif
#ifndef GDK_KEY_hebrew_shin
#define GDK_KEY_hebrew_shin 0xcf9
#endif
#ifndef GDK_KEY_braille_dots_1278
#define GDK_KEY_braille_dots_1278 0x10028c3
#endif
#ifndef GDK_KEY_Armenian_tche
#define GDK_KEY_Armenian_tche 0x1000573
#endif
#ifndef GDK_KEY_sevensubscript
#define GDK_KEY_sevensubscript 0x1002087
#endif
#ifndef GDK_KEY_Wgrave
#define GDK_KEY_Wgrave 0x1001e80
#endif
#ifndef GDK_KEY_Greek_upsilonaccentdieresis
#define GDK_KEY_Greek_upsilonaccentdieresis 0x7ba
#endif
#ifndef GDK_KEY_Pointer_Right
#define GDK_KEY_Pointer_Right 0xfee1
#endif
#ifndef GDK_KEY_doubledagger
#define GDK_KEY_doubledagger 0xaf2
#endif
#ifndef GDK_KEY_braille_dots_248
#define GDK_KEY_braille_dots_248 0x100288a
#endif
#ifndef GDK_KEY_Cyrillic_i_macron
#define GDK_KEY_Cyrillic_i_macron 0x10004e3
#endif
#ifndef GDK_KEY_braille_dots_56
#define GDK_KEY_braille_dots_56 0x1002830
#endif
#ifndef GDK_KEY_MenuPB
#define GDK_KEY_MenuPB 0x1008ff66
#endif
#ifndef GDK_KEY_dead_belowring
#define GDK_KEY_dead_belowring 0xfe67
#endif
#ifndef GDK_KEY_Georgian_man
#define GDK_KEY_Georgian_man 0x10010db
#endif
#ifndef GDK_KEY_braceleft
#define GDK_KEY_braceleft 0x07b
#endif
#ifndef GDK_KEY_Arabic_ha
#define GDK_KEY_Arabic_ha 0x5e7
#endif
#ifndef GDK_KEY_Ycircumflex
#define GDK_KEY_Ycircumflex 0x1000176
#endif
#ifndef GDK_KEY_dead_a
#define GDK_KEY_dead_a 0xfe80
#endif
#ifndef GDK_KEY_Tabovedot
#define GDK_KEY_Tabovedot 0x1001e6a
#endif
#ifndef GDK_KEY_Cyrillic_EN
#define GDK_KEY_Cyrillic_EN 0x6ee
#endif
#ifndef GDK_KEY_Cyrillic_DZHE
#define GDK_KEY_Cyrillic_DZHE 0x6bf
#endif
#ifndef GDK_KEY_Greek_iotadieresis
#define GDK_KEY_Greek_iotadieresis 0x7b5
#endif
#ifndef GDK_KEY_ISO_Enter
#define GDK_KEY_ISO_Enter 0xfe34
#endif
#ifndef GDK_KEY_TouchpadToggle
#define GDK_KEY_TouchpadToggle 0x1008ffa9
#endif
#ifndef GDK_KEY_braille_dots_234567
#define GDK_KEY_braille_dots_234567 0x100287e
#endif
#ifndef GDK_KEY_apostrophe
#define GDK_KEY_apostrophe 0x027
#endif
#ifndef GDK_KEY_Cyrillic_softsign
#define GDK_KEY_Cyrillic_softsign 0x6d8
#endif
#ifndef GDK_KEY_braille_dots_345
#define GDK_KEY_braille_dots_345 0x100281c
#endif
#ifndef GDK_KEY_Icircumflex
#define GDK_KEY_Icircumflex 0x0ce
#endif
#ifndef GDK_KEY_braille_dots_168
#define GDK_KEY_braille_dots_168 0x10028a1
#endif
#ifndef GDK_KEY_OpenURL
#define GDK_KEY_OpenURL 0x1008ff38
#endif
#ifndef GDK_KEY_Sabovedot
#define GDK_KEY_Sabovedot 0x1001e60
#endif
#ifndef GDK_KEY_plusminus
#define GDK_KEY_plusminus 0x0b1
#endif
#ifndef GDK_KEY_CruzeiroSign
#define GDK_KEY_CruzeiroSign 0x10020a2
#endif
#ifndef GDK_KEY_braille_dots_127
#define GDK_KEY_braille_dots_127 0x1002843
#endif
#ifndef GDK_KEY_Phone
#define GDK_KEY_Phone 0x1008ff6e
#endif
#ifndef GDK_KEY_Thai_moma
#define GDK_KEY_Thai_moma 0xdc1
#endif
#ifndef GDK_KEY_topleftradical
#define GDK_KEY_topleftradical 0x8a2
#endif
#ifndef GDK_KEY_Thai_leksong
#define GDK_KEY_Thai_leksong 0xdf2
#endif
#ifndef GDK_KEY_Ecircumflextilde
#define GDK_KEY_Ecircumflextilde 0x1001ec4
#endif
#ifndef GDK_KEY_braille_dots_123457
#define GDK_KEY_braille_dots_123457 0x100285f
#endif
#ifndef GDK_KEY_ISO_Set_Margin_Right
#define GDK_KEY_ISO_Set_Margin_Right 0xfe28
#endif
#ifndef GDK_KEY_bracketleft
#define GDK_KEY_bracketleft 0x05b
#endif
#ifndef GDK_KEY_ISO_Fast_Cursor_Right
#define GDK_KEY_ISO_Fast_Cursor_Right 0xfe2d
#endif
#ifndef GDK_KEY_Cyrillic_ghe_bar
#define GDK_KEY_Cyrillic_ghe_bar 0x1000493
#endif
#ifndef GDK_KEY_Cyrillic_o
#define GDK_KEY_Cyrillic_o 0x6cf
#endif
#ifndef GDK_KEY_babovedot
#define GDK_KEY_babovedot 0x1001e03
#endif
#ifndef GDK_KEY_Thai_thonangmontho
#define GDK_KEY_Thai_thonangmontho 0xdb1
#endif
#ifndef GDK_KEY_ISO_Partial_Space_Left
#define GDK_KEY_ISO_Partial_Space_Left 0xfe25
#endif
#ifndef GDK_KEY_Arabic_dammatan
#define GDK_KEY_Arabic_dammatan 0x5ec
#endif
#ifndef GDK_KEY_kana_U
#define GDK_KEY_kana_U 0x4b3
#endif
#ifndef GDK_KEY_braille_dots_478
#define GDK_KEY_braille_dots_478 0x10028c8
#endif
#ifndef GDK_KEY_diaeresis
#define GDK_KEY_diaeresis 0x0a8
#endif
#ifndef GDK_KEY_Greek_omicron
#define GDK_KEY_Greek_omicron 0x7ef
#endif
#ifndef GDK_KEY_Amacron
#define GDK_KEY_Amacron 0x3c0
#endif
#ifndef GDK_KEY_MouseKeys_Accel_Enable
#define GDK_KEY_MouseKeys_Accel_Enable 0xfe77
#endif
#ifndef GDK_KEY_comma
#define GDK_KEY_comma 0x02c
#endif
#ifndef GDK_KEY_kana_TSU
#define GDK_KEY_kana_TSU 0x4c2
#endif
#ifndef GDK_KEY_kra
#define GDK_KEY_kra 0x3a2
#endif
#ifndef GDK_KEY_Serbian_dze
#define GDK_KEY_Serbian_dze 0x6af
#endif
#ifndef GDK_KEY_TaskPane
#define GDK_KEY_TaskPane 0x1008ff7f
#endif
#ifndef GDK_KEY_braille_dots_278
#define GDK_KEY_braille_dots_278 0x10028c2
#endif
#ifndef GDK_KEY_Aogonek
#define GDK_KEY_Aogonek 0x1a1
#endif
#ifndef GDK_KEY_braille_dots_1235678
#define GDK_KEY_braille_dots_1235678 0x10028f7
#endif
#ifndef GDK_KEY_obarred
#define GDK_KEY_obarred 0x1000275
#endif
#ifndef GDK_KEY_Thai_leksi
#define GDK_KEY_Thai_leksi 0xdf4
#endif
#ifndef GDK_KEY_braille_dots_1678
#define GDK_KEY_braille_dots_1678 0x10028e1
#endif
#ifndef GDK_KEY_Ukrainian_yi
#define GDK_KEY_Ukrainian_yi 0x6a7
#endif
#ifndef GDK_KEY_Greek_DELTA
#define GDK_KEY_Greek_DELTA 0x7c4
#endif
#ifndef GDK_KEY_Ecircumflexhook
#define GDK_KEY_Ecircumflexhook 0x1001ec2
#endif
#ifndef GDK_KEY_braille_dots_6
#define GDK_KEY_braille_dots_6 0x1002820
#endif
#ifndef GDK_KEY_MultipleCandidate
#define GDK_KEY_MultipleCandidate 0xff3d
#endif
#ifndef GDK_KEY_dead_circumflex
#define GDK_KEY_dead_circumflex 0xfe52
#endif
#ifndef GDK_KEY_Launch1
#define GDK_KEY_Launch1 0x1008ff41
#endif
#ifndef GDK_KEY_y
#define GDK_KEY_y 0x079
#endif
#ifndef GDK_KEY_Greek_nu
#define GDK_KEY_Greek_nu 0x7ed
#endif
#ifndef GDK_KEY_racute
#define GDK_KEY_racute 0x1e0
#endif
#ifndef GDK_KEY_Explorer
#define GDK_KEY_Explorer 0x1008ff5d
#endif
#ifndef GDK_KEY_Greek_beta
#define GDK_KEY_Greek_beta 0x7e2
#endif
#ifndef GDK_KEY_braille_dots_456
#define GDK_KEY_braille_dots_456 0x1002838
#endif
#ifndef GDK_KEY_ApplicationLeft
#define GDK_KEY_ApplicationLeft 0x1008ff50
#endif
#ifndef GDK_KEY_Arabic_switch
#define GDK_KEY_Arabic_switch 0xff7e
#endif
#ifndef GDK_KEY_Cyrillic_ER
#define GDK_KEY_Cyrillic_ER 0x6f2
#endif
#ifndef GDK_KEY_less
#define GDK_KEY_less 0x03c
#endif
#ifndef GDK_KEY_2
#define GDK_KEY_2 0x032
#endif
#ifndef GDK_KEY_copyright
#define GDK_KEY_copyright 0x0a9
#endif
#ifndef GDK_KEY_Ydiaeresis
#define GDK_KEY_Ydiaeresis 0x13be
#endif
#ifndef GDK_KEY_dead_U
#define GDK_KEY_dead_U 0xfe89
#endif
#ifndef GDK_KEY_acircumflexacute
#define GDK_KEY_acircumflexacute 0x1001ea5
#endif
#ifndef GDK_KEY_eogonek
#define GDK_KEY_eogonek 0x1ea
#endif
#ifndef GDK_KEY_rightcaret
#define GDK_KEY_rightcaret 0xba6
#endif
#ifndef GDK_KEY_Greek_LAMDA
#define GDK_KEY_Greek_LAMDA 0x7cb
#endif
#ifndef GDK_KEY_Start
#define GDK_KEY_Start 0x1008ff1a
#endif
#ifndef GDK_KEY_topleftsummation
#define GDK_KEY_topleftsummation 0x8b1
#endif
#ifndef GDK_KEY_Help
#define GDK_KEY_Help 0xff6a
#endif
#ifndef GDK_KEY_braille_dots_15
#define GDK_KEY_braille_dots_15 0x1002811
#endif
#ifndef GDK_KEY_Arabic_hamzaonalef
#define GDK_KEY_Arabic_hamzaonalef 0x5c3
#endif
#ifndef GDK_KEY_braille_dots_1256
#define GDK_KEY_braille_dots_1256 0x1002833
#endif
#ifndef GDK_KEY_tslash
#define GDK_KEY_tslash 0x3bc
#endif
#ifndef GDK_KEY_kana_SO
#define GDK_KEY_kana_SO 0x4bf
#endif
#ifndef GDK_KEY_Cyrillic_CHE_vertstroke
#define GDK_KEY_Cyrillic_CHE_vertstroke 0x10004b8
#endif
#ifndef GDK_KEY_Back
#define GDK_KEY_Back 0x1008ff26
#endif
#ifndef GDK_KEY_Hangul_WEO
#define GDK_KEY_Hangul_WEO 0xecd
#endif
#ifndef GDK_KEY_zabovedot
#define GDK_KEY_zabovedot 0x1bf
#endif
#ifndef GDK_KEY_Arabic_qaf
#define GDK_KEY_Arabic_qaf 0x5e2
#endif
#ifndef GDK_KEY_KP_Page_Down
#define GDK_KEY_KP_Page_Down 0xff9b
#endif
#ifndef GDK_KEY_zstroke
#define GDK_KEY_zstroke 0x10001b6
#endif
#ifndef GDK_KEY_Arabic_heh_doachashmee
#define GDK_KEY_Arabic_heh_doachashmee 0x10006be
#endif
#ifndef GDK_KEY_approximate
#define GDK_KEY_approximate 0x8c8
#endif
#ifndef GDK_KEY_l
#define GDK_KEY_l 0x06c
#endif
#ifndef GDK_KEY_em4space
#define GDK_KEY_em4space 0xaa4
#endif
#ifndef GDK_KEY_dead_abovereversedcomma
#define GDK_KEY_dead_abovereversedcomma 0xfe65
#endif
#ifndef GDK_KEY_Mae_Koho
#define GDK_KEY_Mae_Koho 0xff3e
#endif
#ifndef GDK_KEY_Kana_Lock
#define GDK_KEY_Kana_Lock 0xff2d
#endif
#ifndef GDK_KEY_Armenian_sha
#define GDK_KEY_Armenian_sha 0x1000577
#endif
#ifndef GDK_KEY_Hangul_J_RieulMieum
#define GDK_KEY_Hangul_J_RieulMieum 0xedd
#endif
#ifndef GDK_KEY_g
#define GDK_KEY_g 0x067
#endif
#ifndef GDK_KEY_Cyrillic_yu
#define GDK_KEY_Cyrillic_yu 0x6c0
#endif
#ifndef GDK_KEY_sterling
#define GDK_KEY_sterling 0x0a3
#endif
#ifndef GDK_KEY_topvertsummationconnector
#define GDK_KEY_topvertsummationconnector 0x8b3
#endif
#ifndef GDK_KEY_Launch9
#define GDK_KEY_Launch9 0x1008ff49
#endif
#ifndef GDK_KEY_abreve
#define GDK_KEY_abreve 0x1e3
#endif
#ifndef GDK_KEY_Armenian_VEV
#define GDK_KEY_Armenian_VEV 0x100054e
#endif
#ifndef GDK_KEY_Armenian_apostrophe
#define GDK_KEY_Armenian_apostrophe 0x100055a
#endif
#ifndef GDK_KEY_Q
#define GDK_KEY_Q 0x051
#endif
#ifndef GDK_KEY_Thai_yoying
#define GDK_KEY_Thai_yoying 0xdad
#endif
#ifndef GDK_KEY_uogonek
#define GDK_KEY_uogonek 0x3f9
#endif
#ifndef GDK_KEY_nobreakspace
#define GDK_KEY_nobreakspace 0x0a0
#endif
#ifndef GDK_KEY_Armenian_o
#define GDK_KEY_Armenian_o 0x1000585
#endif
#ifndef GDK_KEY_slash
#define GDK_KEY_slash 0x02f
#endif
#ifndef GDK_KEY_paragraph
#define GDK_KEY_paragraph 0x0b6
#endif
#ifndef GDK_KEY_dead_invertedbreve
#define GDK_KEY_dead_invertedbreve 0xfe6d
#endif
#ifndef GDK_KEY_hebrew_he
#define GDK_KEY_hebrew_he 0xce4
#endif
#ifndef GDK_KEY_braille_dots_235
#define GDK_KEY_braille_dots_235 0x1002816
#endif
#ifndef GDK_KEY_leftshoe
#define GDK_KEY_leftshoe 0xbda
#endif
#ifndef GDK_KEY_F
#define GDK_KEY_F 0x046
#endif
#ifndef GDK_KEY_Armenian_RA
#define GDK_KEY_Armenian_RA 0x100054c
#endif
#ifndef GDK_KEY_braille_dots_123678
#define GDK_KEY_braille_dots_123678 0x10028e7
#endif
#ifndef GDK_KEY_b
#define GDK_KEY_b 0x062
#endif
#ifndef GDK_KEY_Farsi_9
#define GDK_KEY_Farsi_9 0x10006f9
#endif
#ifndef GDK_KEY_Armenian_YECH
#define GDK_KEY_Armenian_YECH 0x1000535
#endif
#ifndef GDK_KEY_MouseKeys_Enable
#define GDK_KEY_MouseKeys_Enable 0xfe76
#endif
#ifndef GDK_KEY_Cyrillic_hardsign
#define GDK_KEY_Cyrillic_hardsign 0x6df
#endif
#ifndef GDK_KEY_Meta_R
#define GDK_KEY_Meta_R 0xffe8
#endif
#ifndef GDK_KEY_7
#define GDK_KEY_7 0x037
#endif
#ifndef GDK_KEY_Arabic_heh
#define GDK_KEY_Arabic_heh 0x5e7
#endif
#ifndef GDK_KEY_Support
#define GDK_KEY_Support 0x1008ff7e
#endif
#ifndef GDK_KEY_braille_dots_12378
#define GDK_KEY_braille_dots_12378 0x10028c7
#endif
#ifndef GDK_KEY_3270_Jump
#define GDK_KEY_3270_Jump 0xfd12
#endif
#ifndef GDK_KEY_Pointer_EnableKeys
#define GDK_KEY_Pointer_EnableKeys 0xfef9
#endif
#ifndef GDK_KEY_Cyrillic_TE
#define GDK_KEY_Cyrillic_TE 0x6f4
#endif
#ifndef GDK_KEY_Thai_saraue
#define GDK_KEY_Thai_saraue 0xdd6
#endif
#ifndef GDK_KEY_Ecircumflexgrave
#define GDK_KEY_Ecircumflexgrave 0x1001ec0
#endif
#ifndef GDK_KEY_Arabic_maddaonalef
#define GDK_KEY_Arabic_maddaonalef 0x5c2
#endif
#ifndef GDK_KEY_Meta_L
#define GDK_KEY_Meta_L 0xffe7
#endif
#ifndef GDK_KEY_jcircumflex
#define GDK_KEY_jcircumflex 0x2bc
#endif
#ifndef GDK_KEY_Ograve
#define GDK_KEY_Ograve 0x0d2
#endif
#ifndef GDK_KEY_Arabic_sheen
#define GDK_KEY_Arabic_sheen 0x5d4
#endif
#ifndef GDK_KEY_Shift_Lock
#define GDK_KEY_Shift_Lock 0xffe6
#endif
#ifndef GDK_KEY_Pointer_DfltBtnNext
#define GDK_KEY_Pointer_DfltBtnNext 0xfefb
#endif
#ifndef GDK_KEY_gcircumflex
#define GDK_KEY_gcircumflex 0x2f8
#endif
#ifndef GDK_KEY_KP_7
#define GDK_KEY_KP_7 0xffb7
#endif
#ifndef GDK_KEY_kana_TO
#define GDK_KEY_kana_TO 0x4c4
#endif
#ifndef GDK_KEY_Nacute
#define GDK_KEY_Nacute 0x1d1
#endif
#ifndef GDK_KEY_Thai_sarao
#define GDK_KEY_Thai_sarao 0xde2
#endif
#ifndef GDK_KEY_AudioMute
#define GDK_KEY_AudioMute 0x1008ff12
#endif
#ifndef GDK_KEY_Farsi_4
#define GDK_KEY_Farsi_4 0x10006f4
#endif
#ifndef GDK_KEY_underscore
#define GDK_KEY_underscore 0x05f
#endif
#ifndef GDK_KEY_3270_DeleteWord
#define GDK_KEY_3270_DeleteWord 0xfd1a
#endif
#ifndef GDK_KEY_Ytilde
#define GDK_KEY_Ytilde 0x1001ef8
#endif
#ifndef GDK_KEY_Thai_wowaen
#define GDK_KEY_Thai_wowaen 0xdc7
#endif
#ifndef GDK_KEY_Arabic_shadda
#define GDK_KEY_Arabic_shadda 0x5f1
#endif
#ifndef GDK_KEY_Reply
#define GDK_KEY_Reply 0x1008ff72
#endif
#ifndef GDK_KEY_Katakana
#define GDK_KEY_Katakana 0xff26
#endif
#ifndef GDK_KEY_Thai_saraaimaimuan
#define GDK_KEY_Thai_saraaimaimuan 0xde3
#endif
#ifndef GDK_KEY_Arabic_teh
#define GDK_KEY_Arabic_teh 0x5ca
#endif
#ifndef GDK_KEY_because
#define GDK_KEY_because 0x1002235
#endif
#ifndef GDK_KEY_braille_dots_12468
#define GDK_KEY_braille_dots_12468 0x10028ab
#endif
#ifndef GDK_KEY_WebCam
#define GDK_KEY_WebCam 0x1008ff8f
#endif
#ifndef GDK_KEY_rightdoublequotemark
#define GDK_KEY_rightdoublequotemark 0xad3
#endif
#ifndef GDK_KEY_Serbian_NJE
#define GDK_KEY_Serbian_NJE 0x6ba
#endif
#ifndef GDK_KEY_hebrew_zade
#define GDK_KEY_hebrew_zade 0xcf6
#endif
#ifndef GDK_KEY_braille_dots_2356
#define GDK_KEY_braille_dots_2356 0x1002836
#endif
#ifndef GDK_KEY_blank
#define GDK_KEY_blank 0x9df
#endif
#ifndef GDK_KEY_zerosubscript
#define GDK_KEY_zerosubscript 0x1002080
#endif
#ifndef GDK_KEY_braille_dots_28
#define GDK_KEY_braille_dots_28 0x1002882
#endif
#ifndef GDK_KEY_decimalpoint
#define GDK_KEY_decimalpoint 0xabd
#endif
#ifndef GDK_KEY_F15
#define GDK_KEY_F15 0xffcc
#endif
#ifndef GDK_KEY_Cyrillic_che_descender
#define GDK_KEY_Cyrillic_che_descender 0x10004b7
#endif
#ifndef GDK_KEY_ae
#define GDK_KEY_ae 0x0e6
#endif
#ifndef GDK_KEY_F28
#define GDK_KEY_F28 0xffd9
#endif
#ifndef GDK_KEY_Cyrillic_en_descender
#define GDK_KEY_Cyrillic_en_descender 0x10004a3
#endif
#ifndef GDK_KEY_Hangul_Romaja
#define GDK_KEY_Hangul_Romaja 0xff36
#endif
#ifndef GDK_KEY_F27
#define GDK_KEY_F27 0xffd8
#endif
#ifndef GDK_KEY_ocircumflexacute
#define GDK_KEY_ocircumflexacute 0x1001ed1
#endif
#ifndef GDK_KEY_Armenian_yech
#define GDK_KEY_Armenian_yech 0x1000565
#endif
#ifndef GDK_KEY_Hangul_SunkyeongeumPhieuf
#define GDK_KEY_Hangul_SunkyeongeumPhieuf 0xef4
#endif
#ifndef GDK_KEY_Arabic_gaf
#define GDK_KEY_Arabic_gaf 0x10006af
#endif
#ifndef GDK_KEY_braille_dots_7
#define GDK_KEY_braille_dots_7 0x1002840
#endif
#ifndef GDK_KEY_abrevehook
#define GDK_KEY_abrevehook 0x1001eb3
#endif
#ifndef GDK_KEY_braille_dots_134568
#define GDK_KEY_braille_dots_134568 0x10028bd
#endif
#ifndef GDK_KEY_3270_CursorBlink
#define GDK_KEY_3270_CursorBlink 0xfd0f
#endif
#ifndef GDK_KEY_KbdLightOnOff
#define GDK_KEY_KbdLightOnOff 0x1008ff04
#endif
#ifndef GDK_KEY_Lcaron
#define GDK_KEY_Lcaron 0x1a5
#endif
#ifndef GDK_KEY_asciitilde
#define GDK_KEY_asciitilde 0x07e
#endif
#ifndef GDK_KEY_ENG
#define GDK_KEY_ENG 0x3bd
#endif
#ifndef GDK_KEY_ISO_Release_Both_Margins
#define GDK_KEY_ISO_Release_Both_Margins 0xfe2b
#endif
#ifndef GDK_KEY_Cyrillic_che
#define GDK_KEY_Cyrillic_che 0x6de
#endif
#ifndef GDK_KEY_KP_F1
#define GDK_KEY_KP_F1 0xff91
#endif
#ifndef GDK_KEY_Hangul_J_Sios
#define GDK_KEY_Hangul_J_Sios 0xee6
#endif
#ifndef GDK_KEY_RotationKB
#define GDK_KEY_RotationKB 0x1008ff76
#endif
#ifndef GDK_KEY_toprightparens
#define GDK_KEY_toprightparens 0x8ad
#endif
#ifndef GDK_KEY_Ecircumflexbelowdot
#define GDK_KEY_Ecircumflexbelowdot 0x1001ec6
#endif
#ifndef GDK_KEY_uhornacute
#define GDK_KEY_uhornacute 0x1001ee9
#endif
#ifndef GDK_KEY_3270_Rule
#define GDK_KEY_3270_Rule 0xfd14
#endif
#ifndef GDK_KEY_dabovedot
#define GDK_KEY_dabovedot 0x1001e0b
#endif
#ifndef GDK_KEY_Gcaron
#define GDK_KEY_Gcaron 0x10001e6
#endif
#ifndef GDK_KEY_Ocircumflexhook
#define GDK_KEY_Ocircumflexhook 0x1001ed4
#endif
#ifndef GDK_KEY_braille_dots_467
#define GDK_KEY_braille_dots_467 0x1002868
#endif
#ifndef GDK_KEY_Ihook
#define GDK_KEY_Ihook 0x1001ec8
#endif
#ifndef GDK_KEY_LaunchF
#define GDK_KEY_LaunchF 0x1008ff4f
#endif
#ifndef GDK_KEY_emfilledcircle
#define GDK_KEY_emfilledcircle 0xade
#endif
#ifndef GDK_KEY_Hangul_WE
#define GDK_KEY_Hangul_WE 0xece
#endif
#ifndef GDK_KEY_Arabic_ain
#define GDK_KEY_Arabic_ain 0x5d9
#endif
#ifndef GDK_KEY_braille_dots_37
#define GDK_KEY_braille_dots_37 0x1002844
#endif
#ifndef GDK_KEY_Arabic_6
#define GDK_KEY_Arabic_6 0x1000666
#endif
#ifndef GDK_KEY_Cyrillic_o_bar
#define GDK_KEY_Cyrillic_o_bar 0x10004e9
#endif
#ifndef GDK_KEY_Hangul_Ieung
#define GDK_KEY_Hangul_Ieung 0xeb7
#endif
#ifndef GDK_KEY_hebrew_zain
#define GDK_KEY_hebrew_zain 0xce6
#endif
#ifndef GDK_KEY_Greek_epsilonaccent
#define GDK_KEY_Greek_epsilonaccent 0x7b2
#endif
#ifndef GDK_KEY_rightarrow
#define GDK_KEY_rightarrow 0x8fd
#endif
#ifndef GDK_KEY_Hangul_RieulSios
#define GDK_KEY_Hangul_RieulSios 0xead
#endif
#ifndef GDK_KEY_Uhorngrave
#define GDK_KEY_Uhorngrave 0x1001eea
#endif
#ifndef GDK_KEY_Thai_thothan
#define GDK_KEY_Thai_thothan 0xdb0
#endif
#ifndef GDK_KEY_KP_Right
#define GDK_KEY_KP_Right 0xff98
#endif
#ifndef GDK_KEY_Pause
#define GDK_KEY_Pause 0xff13
#endif
#ifndef GDK_KEY_braille_dots_34568
#define GDK_KEY_braille_dots_34568 0x10028bc
#endif
#ifndef GDK_KEY_Y
#define GDK_KEY_Y 0x059
#endif
#ifndef GDK_KEY_braille_dot_4
#define GDK_KEY_braille_dot_4 0xfff4
#endif
#ifndef GDK_KEY_Greek_UPSILONaccent
#define GDK_KEY_Greek_UPSILONaccent 0x7a8
#endif
#ifndef GDK_KEY_careof
#define GDK_KEY_careof 0xab8
#endif
#ifndef GDK_KEY_kana_RI
#define GDK_KEY_kana_RI 0x4d8
#endif
#ifndef GDK_KEY_Arabic_kasratan
#define GDK_KEY_Arabic_kasratan 0x5ed
#endif
#ifndef GDK_KEY_notsign
#define GDK_KEY_notsign 0x0ac
#endif
#ifndef GDK_KEY_Cyrillic_ZE
#define GDK_KEY_Cyrillic_ZE 0x6fa
#endif
#ifndef GDK_KEY_braille_dots_3
#define GDK_KEY_braille_dots_3 0x1002804
#endif
#ifndef GDK_KEY_mabovedot
#define GDK_KEY_mabovedot 0x1001e41
#endif
#ifndef GDK_KEY_enfilledcircbullet
#define GDK_KEY_enfilledcircbullet 0xae6
#endif
#ifndef GDK_KEY_kana_switch
#define GDK_KEY_kana_switch 0xff7e
#endif
#ifndef GDK_KEY_ApplicationRight
#define GDK_KEY_ApplicationRight 0x1008ff51
#endif
#ifndef GDK_KEY_Sys_Req
#define GDK_KEY_Sys_Req 0xff15
#endif
#ifndef GDK_KEY_fiveeighths
#define GDK_KEY_fiveeighths 0xac5
#endif
#ifndef GDK_KEY_PowerOff
#define GDK_KEY_PowerOff 0x1008ff2a
#endif
#ifndef GDK_KEY_Hangul_PanSios
#define GDK_KEY_Hangul_PanSios 0xef2
#endif
#ifndef GDK_KEY_Arabic_4
#define GDK_KEY_Arabic_4 0x1000664
#endif
#ifndef GDK_KEY_Next_VMode
#define GDK_KEY_Next_VMode 0x1008fe22
#endif
#ifndef GDK_KEY_SCHWA
#define GDK_KEY_SCHWA 0x100018f
#endif
#ifndef GDK_KEY_Pointer_DownLeft
#define GDK_KEY_Pointer_DownLeft 0xfee6
#endif
#ifndef GDK_KEY_ISO_Group_Lock
#define GDK_KEY_ISO_Group_Lock 0xfe07
#endif
#ifndef GDK_KEY_hebrew_zayin
#define GDK_KEY_hebrew_zayin 0xce6
#endif
#ifndef GDK_KEY_braille_dots_568
#define GDK_KEY_braille_dots_568 0x10028b0
#endif
#ifndef GDK_KEY_Overlay1_Enable
#define GDK_KEY_Overlay1_Enable 0xfe78
#endif
#ifndef GDK_KEY_Thai_saraa
#define GDK_KEY_Thai_saraa 0xdd0
#endif
#ifndef GDK_KEY_Henkan
#define GDK_KEY_Henkan 0xff23
#endif
#ifndef GDK_KEY_braille_dots_12347
#define GDK_KEY_braille_dots_12347 0x100284f
#endif
#ifndef GDK_KEY_kana_SA
#define GDK_KEY_kana_SA 0x4bb
#endif
#ifndef GDK_KEY_acircumflex
#define GDK_KEY_acircumflex 0x0e2
#endif
#ifndef GDK_KEY_cacute
#define GDK_KEY_cacute 0x1e6
#endif
#ifndef GDK_KEY_dead_psili
#define GDK_KEY_dead_psili 0xfe64
#endif
#ifndef GDK_KEY_vt
#define GDK_KEY_vt 0x9e9
#endif
#ifndef GDK_KEY_Wcircumflex
#define GDK_KEY_Wcircumflex 0x1000174
#endif
#ifndef GDK_KEY_braille_dots_178
#define GDK_KEY_braille_dots_178 0x10028c1
#endif
#ifndef GDK_KEY_Hangul_RieulKiyeog
#define GDK_KEY_Hangul_RieulKiyeog 0xeaa
#endif
#ifndef GDK_KEY_WonSign
#define GDK_KEY_WonSign 0x10020a9
#endif
#ifndef GDK_KEY_Overlay2_Enable
#define GDK_KEY_Overlay2_Enable 0xfe79
#endif
#ifndef GDK_KEY_Greek_iotaaccentdieresis
#define GDK_KEY_Greek_iotaaccentdieresis 0x7b6
#endif
#ifndef GDK_KEY_Babovedot
#define GDK_KEY_Babovedot 0x1001e02
#endif
#ifndef GDK_KEY_Hangul_J_Phieuf
#define GDK_KEY_Hangul_J_Phieuf 0xeed
#endif
#ifndef GDK_KEY_kana_closingbracket
#define GDK_KEY_kana_closingbracket 0x4a3
#endif
#ifndef GDK_KEY_Greek_phi
#define GDK_KEY_Greek_phi 0x7f6
#endif
#ifndef GDK_KEY_Pointer_Down
#define GDK_KEY_Pointer_Down 0xfee3
#endif
#ifndef GDK_KEY_Launch8
#define GDK_KEY_Launch8 0x1008ff48
#endif
#ifndef GDK_KEY_Cyrillic_VE
#define GDK_KEY_Cyrillic_VE 0x6f7
#endif
#ifndef GDK_KEY_Save
#define GDK_KEY_Save 0x1008ff77
#endif
#ifndef GDK_KEY_Arabic_seen
#define GDK_KEY_Arabic_seen 0x5d3
#endif
#ifndef GDK_KEY_F20
#define GDK_KEY_F20 0xffd1
#endif
#ifndef GDK_KEY_Hangul_YO
#define GDK_KEY_Hangul_YO 0xecb
#endif
#ifndef GDK_KEY_Switch_VT_11
#define GDK_KEY_Switch_VT_11 0x1008fe0b
#endif
#ifndef GDK_KEY_dead_semivoiced_sound
#define GDK_KEY_dead_semivoiced_sound 0xfe5f
#endif
#ifndef GDK_KEY_Hangul_O
#define GDK_KEY_Hangul_O 0xec7
#endif
#ifndef GDK_KEY_acircumflexgrave
#define GDK_KEY_acircumflexgrave 0x1001ea7
#endif
#ifndef GDK_KEY_Arabic_sukun
#define GDK_KEY_Arabic_sukun 0x5f2
#endif
#ifndef GDK_KEY_braille_dots_1234578
#define GDK_KEY_braille_dots_1234578 0x10028df
#endif
#ifndef GDK_KEY_R3
#define GDK_KEY_R3 0xffd4
#endif
#ifndef GDK_KEY_kana_KA
#define GDK_KEY_kana_KA 0x4b6
#endif
#ifndef GDK_KEY_eightsubscript
#define GDK_KEY_eightsubscript 0x1002088
#endif
#ifndef GDK_KEY_Refresh
#define GDK_KEY_Refresh 0x1008ff29
#endif
#ifndef GDK_KEY_Arabic_hah
#define GDK_KEY_Arabic_hah 0x5cd
#endif
#ifndef GDK_KEY_KP_8
#define GDK_KEY_KP_8 0xffb8
#endif
#ifndef GDK_KEY_Pointer_Accelerate
#define GDK_KEY_Pointer_Accelerate 0xfefa
#endif
#ifndef GDK_KEY_horizlinescan9
#define GDK_KEY_horizlinescan9 0x9f3
#endif
#ifndef GDK_KEY_Georgian_hae
#define GDK_KEY_Georgian_hae 0x10010f0
#endif
#ifndef GDK_KEY_dead_abovedot
#define GDK_KEY_dead_abovedot 0xfe56
#endif
#ifndef GDK_KEY_onethird
#define GDK_KEY_onethird 0xab0
#endif
#ifndef GDK_KEY_Thai_dochada
#define GDK_KEY_Thai_dochada 0xdae
#endif
#ifndef GDK_KEY_Korean_Won
#define GDK_KEY_Korean_Won 0xeff
#endif
#ifndef GDK_KEY_trademarkincircle
#define GDK_KEY_trademarkincircle 0xacb
#endif
#ifndef GDK_KEY_hebrew_ayin
#define GDK_KEY_hebrew_ayin 0xcf2
#endif
#ifndef GDK_KEY_Uacute
#define GDK_KEY_Uacute 0x0da
#endif
#ifndef GDK_KEY_downshoe
#define GDK_KEY_downshoe 0xbd6
#endif
#ifndef GDK_KEY_braille_dots_12
#define GDK_KEY_braille_dots_12 0x1002803
#endif
#ifndef GDK_KEY_Armenian_but
#define GDK_KEY_Armenian_but 0x100055d
#endif
#ifndef GDK_KEY_Ehook
#define GDK_KEY_Ehook 0x1001eba
#endif
#ifndef GDK_KEY_Break
#define GDK_KEY_Break 0xff6b
#endif
#ifndef GDK_KEY_hebrew_pe
#define GDK_KEY_hebrew_pe 0xcf4
#endif
#ifndef GDK_KEY_braille_dots_1458
#define GDK_KEY_braille_dots_1458 0x1002899
#endif
#ifndef GDK_KEY_MyComputer
#define GDK_KEY_MyComputer 0x1008ff33
#endif
#ifndef GDK_KEY_Cyrillic_YA
#define GDK_KEY_Cyrillic_YA 0x6f1
#endif
#ifndef GDK_KEY_Abreveacute
#define GDK_KEY_Abreveacute 0x1001eae
#endif
#ifndef GDK_KEY_Armenian_je
#define GDK_KEY_Armenian_je 0x100057b
#endif
#ifndef GDK_KEY_t
#define GDK_KEY_t 0x074
#endif
#ifndef GDK_KEY_3270_ChangeScreen
#define GDK_KEY_3270_ChangeScreen 0xfd19
#endif
#ifndef GDK_KEY_braille_dots_3678
#define GDK_KEY_braille_dots_3678 0x10028e4
#endif
#ifndef GDK_KEY_period
#define GDK_KEY_period 0x02e
#endif
#ifndef GDK_KEY_Armenian_INI
#define GDK_KEY_Armenian_INI 0x100053b
#endif
#ifndef GDK_KEY_bracketright
#define GDK_KEY_bracketright 0x05d
#endif
#ifndef GDK_KEY_scircumflex
#define GDK_KEY_scircumflex 0x2fe
#endif
#ifndef GDK_KEY_leftt
#define GDK_KEY_leftt 0x9f4
#endif
#ifndef GDK_KEY_Cyrillic_HARDSIGN
#define GDK_KEY_Cyrillic_HARDSIGN 0x6ff
#endif
#ifndef GDK_KEY_RepeatKeys_Enable
#define GDK_KEY_RepeatKeys_Enable 0xfe72
#endif
#ifndef GDK_KEY_dead_E
#define GDK_KEY_dead_E 0xfe83
#endif
#ifndef GDK_KEY_KP_Enter
#define GDK_KEY_KP_Enter 0xff8d
#endif
#ifndef GDK_KEY_Cyrillic_LJE
#define GDK_KEY_Cyrillic_LJE 0x6b9
#endif
#ifndef GDK_KEY_Cyrillic_I_macron
#define GDK_KEY_Cyrillic_I_macron 0x10004e2
#endif
#ifndef GDK_KEY_braille_dots_2345
#define GDK_KEY_braille_dots_2345 0x100281e
#endif
#ifndef GDK_KEY_Cyrillic_KA
#define GDK_KEY_Cyrillic_KA 0x6eb
#endif
#ifndef GDK_KEY_yacute
#define GDK_KEY_yacute 0x0fd
#endif
#ifndef GDK_KEY_hebrew_nun
#define GDK_KEY_hebrew_nun 0xcf0
#endif
#ifndef GDK_KEY_DongSign
#define GDK_KEY_DongSign 0x10020ab
#endif
#ifndef GDK_KEY_braille_dots_137
#define GDK_KEY_braille_dots_137 0x1002845
#endif
#ifndef GDK_KEY_Zenkaku
#define GDK_KEY_Zenkaku 0xff28
#endif
#ifndef GDK_KEY_ytilde
#define GDK_KEY_ytilde 0x1001ef9
#endif
#ifndef GDK_KEY_dcaron
#define GDK_KEY_dcaron 0x1ef
#endif
#ifndef GDK_KEY_braille_dots_123
#define GDK_KEY_braille_dots_123 0x1002807
#endif
#ifndef GDK_KEY_Cyrillic_YERU
#define GDK_KEY_Cyrillic_YERU 0x6f9
#endif
#ifndef GDK_KEY_Codeinput
#define GDK_KEY_Codeinput 0xff37
#endif
#ifndef GDK_KEY_Cyrillic_U_macron
#define GDK_KEY_Cyrillic_U_macron 0x10004ee
#endif
#ifndef GDK_KEY_lbelowdot
#define GDK_KEY_lbelowdot 0x1001e37
#endif
#ifndef GDK_KEY_Linefeed
#define GDK_KEY_Linefeed 0xff0a
#endif
#ifndef GDK_KEY_Armenian_ligature_ew
#define GDK_KEY_Armenian_ligature_ew 0x1000587
#endif
#ifndef GDK_KEY_F11
#define GDK_KEY_F11 0xffc8
#endif
#ifndef GDK_KEY_hebrew_bet
#define GDK_KEY_hebrew_bet 0xce1
#endif
#ifndef GDK_KEY_ISO_Level5_Latch
#define GDK_KEY_ISO_Level5_Latch 0xfe12
#endif
#ifndef GDK_KEY_leftarrow
#define GDK_KEY_leftarrow 0x8fb
#endif
#ifndef GDK_KEY_Pictures
#define GDK_KEY_Pictures 0x1008ff91
#endif
#ifndef GDK_KEY_braille_dots_23578
#define GDK_KEY_braille_dots_23578 0x10028d6
#endif
#ifndef GDK_KEY_hebrew_taw
#define GDK_KEY_hebrew_taw 0xcfa
#endif
#ifndef GDK_KEY_L
#define GDK_KEY_L 0x04c
#endif
#ifndef GDK_KEY_eacute
#define GDK_KEY_eacute 0x0e9
#endif
#ifndef GDK_KEY_Yhook
#define GDK_KEY_Yhook 0x1001ef6
#endif
#ifndef GDK_KEY_First_Virtual_Screen
#define GDK_KEY_First_Virtual_Screen 0xfed0
#endif
#ifndef GDK_KEY_braille_dots_1236
#define GDK_KEY_braille_dots_1236 0x1002827
#endif
#ifndef GDK_KEY_Kanji
#define GDK_KEY_Kanji 0xff21
#endif
#ifndef GDK_KEY_Ooblique
#define GDK_KEY_Ooblique 0x0d8
#endif
#ifndef GDK_KEY_TopMenu
#define GDK_KEY_TopMenu 0x1008ffa2
#endif
#ifndef GDK_KEY_j
#define GDK_KEY_j 0x06a
#endif
#ifndef GDK_KEY_malesymbol
#define GDK_KEY_malesymbol 0xaf7
#endif
#ifndef GDK_KEY_toprightsummation
#define GDK_KEY_toprightsummation 0x8b5
#endif
#ifndef GDK_KEY_dagger
#define GDK_KEY_dagger 0xaf1
#endif
#ifndef GDK_KEY_Georgian_an
#define GDK_KEY_Georgian_an 0x10010d0
#endif
#ifndef GDK_KEY_Thai_sarau
#define GDK_KEY_Thai_sarau 0xdd8
#endif
#ifndef GDK_KEY_Thai_soso
#define GDK_KEY_Thai_soso 0xdab
#endif
#ifndef GDK_KEY_Iabovedot
#define GDK_KEY_Iabovedot 0x2a9
#endif
#ifndef GDK_KEY_ohorntilde
#define GDK_KEY_ohorntilde 0x1001ee1
#endif
#ifndef GDK_KEY_rightanglebracket
#define GDK_KEY_rightanglebracket 0xabe
#endif
#ifndef GDK_KEY_Thai_lochula
#define GDK_KEY_Thai_lochula 0xdcc
#endif
#ifndef GDK_KEY_braille_dots_13567
#define GDK_KEY_braille_dots_13567 0x1002875
#endif
#ifndef GDK_KEY_pabovedot
#define GDK_KEY_pabovedot 0x1001e57
#endif
#ifndef GDK_KEY_Georgian_tan
#define GDK_KEY_Georgian_tan 0x10010d7
#endif
#ifndef GDK_KEY_notapproxeq
#define GDK_KEY_notapproxeq 0x1002247
#endif
#ifndef GDK_KEY_AudioPause
#define GDK_KEY_AudioPause 0x1008ff31
#endif
#ifndef GDK_KEY_kappa
#define GDK_KEY_kappa 0x3a2
#endif
#ifndef GDK_KEY_rightmiddlesummation
#define GDK_KEY_rightmiddlesummation 0x8b7
#endif
#ifndef GDK_KEY_ecircumflexgrave
#define GDK_KEY_ecircumflexgrave 0x1001ec1
#endif
#ifndef GDK_KEY_Hangul_Special
#define GDK_KEY_Hangul_Special 0xff3f
#endif
#ifndef GDK_KEY_Pointer_Button2
#define GDK_KEY_Pointer_Button2 0xfeea
#endif
#ifndef GDK_KEY_Hangul_J_Dikeud
#define GDK_KEY_Hangul_J_Dikeud 0xeda
#endif
#ifndef GDK_KEY_Greek_RHO
#define GDK_KEY_Greek_RHO 0x7d1
#endif
#ifndef GDK_KEY_Ahook
#define GDK_KEY_Ahook 0x1001ea2
#endif
#ifndef GDK_KEY_Return
#define GDK_KEY_Return 0xff0d
#endif
#ifndef GDK_KEY_Hangul_RieulHieuh
#define GDK_KEY_Hangul_RieulHieuh 0xeb0
#endif
#ifndef GDK_KEY_Serbian_lje
#define GDK_KEY_Serbian_lje 0x6a9
#endif
#ifndef GDK_KEY_ISO_Discontinuous_Underline
#define GDK_KEY_ISO_Discontinuous_Underline 0xfe31
#endif
#ifndef GDK_KEY_ncedilla
#define GDK_KEY_ncedilla 0x3f1
#endif
#ifndef GDK_KEY_emacron
#define GDK_KEY_emacron 0x3ba
#endif
#ifndef GDK_KEY_Spell
#define GDK_KEY_Spell 0x1008ff7c
#endif
#ifndef GDK_KEY_Greek_omegaaccent
#define GDK_KEY_Greek_omegaaccent 0x7bb
#endif
#ifndef GDK_KEY_Ukrainian_ie
#define GDK_KEY_Ukrainian_ie 0x6a4
#endif
#ifndef GDK_KEY_leftcaret
#define GDK_KEY_leftcaret 0xba3
#endif
#ifndef GDK_KEY_Armenian_ghat
#define GDK_KEY_Armenian_ghat 0x1000572
#endif
#ifndef GDK_KEY_braille_dots_12567
#define GDK_KEY_braille_dots_12567 0x1002873
#endif
#ifndef GDK_KEY_opentribulletup
#define GDK_KEY_opentribulletup 0xae3
#endif
#ifndef GDK_KEY_egrave
#define GDK_KEY_egrave 0x0e8
#endif
#ifndef GDK_KEY_twosubscript
#define GDK_KEY_twosubscript 0x1002082
#endif
#ifndef GDK_KEY_u
#define GDK_KEY_u 0x075
#endif
#ifndef GDK_KEY_braille_dots_34
#define GDK_KEY_braille_dots_34 0x100280c
#endif
#ifndef GDK_KEY_Cyrillic_io
#define GDK_KEY_Cyrillic_io 0x6a3
#endif
#ifndef GDK_KEY_J
#define GDK_KEY_J 0x04a
#endif
#ifndef GDK_KEY_a
#define GDK_KEY_a 0x061
#endif
#ifndef GDK_KEY_Georgian_we
#define GDK_KEY_Georgian_we 0x10010f3
#endif
#ifndef GDK_KEY_Arabic_tehmarbuta
#define GDK_KEY_Arabic_tehmarbuta 0x5c9
#endif
#ifndef GDK_KEY_Armenian_gim
#define GDK_KEY_Armenian_gim 0x1000563
#endif
#ifndef GDK_KEY_braille_dots_12678
#define GDK_KEY_braille_dots_12678 0x10028e3
#endif
#ifndef GDK_KEY_AccessX_Feedback_Enable
#define GDK_KEY_AccessX_Feedback_Enable 0xfe71
#endif
#ifndef GDK_KEY_aogonek
#define GDK_KEY_aogonek 0x1b1
#endif
#ifndef GDK_KEY_Greek_NU
#define GDK_KEY_Greek_NU 0x7cd
#endif
#ifndef GDK_KEY_lf
#define GDK_KEY_lf 0x9e5
#endif
#ifndef GDK_KEY_ehook
#define GDK_KEY_ehook 0x1001ebb
#endif
#ifndef GDK_KEY_braille_dots_14678
#define GDK_KEY_braille_dots_14678 0x10028e9
#endif
#ifndef GDK_KEY_braille_dots_234578
#define GDK_KEY_braille_dots_234578 0x10028de
#endif
#ifndef GDK_KEY_Cyrillic_es
#define GDK_KEY_Cyrillic_es 0x6d3
#endif
#ifndef GDK_KEY_F16
#define GDK_KEY_F16 0xffcd
#endif
#ifndef GDK_KEY_downcaret
#define GDK_KEY_downcaret 0xba8
#endif
#ifndef GDK_KEY_Armenian_NU
#define GDK_KEY_Armenian_NU 0x1000546
#endif
#ifndef GDK_KEY_ediaeresis
#define GDK_KEY_ediaeresis 0x0eb
#endif
#ifndef GDK_KEY_doubbaselinedot
#define GDK_KEY_doubbaselinedot 0xaaf
#endif
#ifndef GDK_KEY_questiondown
#define GDK_KEY_questiondown 0x0bf
#endif
#ifndef GDK_KEY_braille_dots_24568
#define GDK_KEY_braille_dots_24568 0x10028ba
#endif
#ifndef GDK_KEY_Thai_choching
#define GDK_KEY_Thai_choching 0xda9
#endif
#ifndef GDK_KEY_braille_dots_268
#define GDK_KEY_braille_dots_268 0x10028a2
#endif
#ifndef GDK_KEY_R9
#define GDK_KEY_R9 0xffda
#endif
#ifndef GDK_KEY_Ccedilla
#define GDK_KEY_Ccedilla 0x0c7
#endif
#ifndef GDK_KEY_dead_capital_schwa
#define GDK_KEY_dead_capital_schwa 0xfe8b
#endif
#ifndef GDK_KEY_caret
#define GDK_KEY_caret 0xafc
#endif
#ifndef GDK_KEY_Hangul_YAE
#define GDK_KEY_Hangul_YAE 0xec2
#endif
#ifndef GDK_KEY_ifonlyif
#define GDK_KEY_ifonlyif 0x8cd
#endif
#ifndef GDK_KEY_Hangul_switch
#define GDK_KEY_Hangul_switch 0xff7e
#endif
#ifndef GDK_KEY_braille_dots_4
#define GDK_KEY_braille_dots_4 0x1002808
#endif
#ifndef GDK_KEY_Kana_Shift
#define GDK_KEY_Kana_Shift 0xff2e
#endif
#ifndef GDK_KEY_Hangul_SunkyeongeumMieum
#define GDK_KEY_Hangul_SunkyeongeumMieum 0xef0
#endif
#ifndef GDK_KEY_braille_dots_3468
#define GDK_KEY_braille_dots_3468 0x10028ac
#endif
#ifndef GDK_KEY_uprightcorner
#define GDK_KEY_uprightcorner 0x9eb
#endif
#ifndef GDK_KEY_F9
#define GDK_KEY_F9 0xffc6
#endif
#ifndef GDK_KEY_botrightparens
#define GDK_KEY_botrightparens 0x8ae
#endif
#ifndef GDK_KEY_braille_dot_1
#define GDK_KEY_braille_dot_1 0xfff1
#endif
#ifndef GDK_KEY_braille_dots_18
#define GDK_KEY_braille_dots_18 0x1002881
#endif
#ifndef GDK_KEY_braille_dots_1568
#define GDK_KEY_braille_dots_1568 0x10028b1
#endif
#ifndef GDK_KEY_braille_dots_2346
#define GDK_KEY_braille_dots_2346 0x100282e
#endif
#ifndef GDK_KEY_Arabic_tatweel
#define GDK_KEY_Arabic_tatweel 0x5e0
#endif
#ifndef GDK_KEY_Pointer_Up
#define GDK_KEY_Pointer_Up 0xfee2
#endif
#ifndef GDK_KEY_ISO_Partial_Space_Right
#define GDK_KEY_ISO_Partial_Space_Right 0xfe26
#endif
#ifndef GDK_KEY_Switch_VT_5
#define GDK_KEY_Switch_VT_5 0x1008fe05
#endif
#ifndef GDK_KEY_upcaret
#define GDK_KEY_upcaret 0xba9
#endif
#ifndef GDK_KEY_dead_doublegrave
#define GDK_KEY_dead_doublegrave 0xfe66
#endif
#ifndef GDK_KEY_braille_dots_124568
#define GDK_KEY_braille_dots_124568 0x10028bb
#endif
#ifndef GDK_KEY_dead_belowdot
#define GDK_KEY_dead_belowdot 0xfe60
#endif
#ifndef GDK_KEY_Arabic_kasra
#define GDK_KEY_Arabic_kasra 0x5f0
#endif
#ifndef GDK_KEY_Armenian_question
#define GDK_KEY_Armenian_question 0x100055e
#endif
#ifndef GDK_KEY_ISO_Move_Line_Down
#define GDK_KEY_ISO_Move_Line_Down 0xfe22
#endif
#ifndef GDK_KEY_Shift_R
#define GDK_KEY_Shift_R 0xffe2
#endif
#ifndef GDK_KEY_Launch4
#define GDK_KEY_Launch4 0x1008ff44
#endif
#ifndef GDK_KEY_ISO_Move_Line_Up
#define GDK_KEY_ISO_Move_Line_Up 0xfe21
#endif
#ifndef GDK_KEY_utilde
#define GDK_KEY_utilde 0x3fd
#endif
#ifndef GDK_KEY_kana_HA
#define GDK_KEY_kana_HA 0x4ca
#endif
#ifndef GDK_KEY_foursuperior
#define GDK_KEY_foursuperior 0x1002074
#endif
#ifndef GDK_KEY_Serbian_DJE
#define GDK_KEY_Serbian_DJE 0x6b1
#endif
#ifndef GDK_KEY_braille_dots_145678
#define GDK_KEY_braille_dots_145678 0x10028f9
#endif
#ifndef GDK_KEY_Armenian_zhe
#define GDK_KEY_Armenian_zhe 0x100056a
#endif
#ifndef GDK_KEY_braille_dots_1456
#define GDK_KEY_braille_dots_1456 0x1002839
#endif
#ifndef GDK_KEY_braille_dots_23458
#define GDK_KEY_braille_dots_23458 0x100289e
#endif
#ifndef GDK_KEY_kana_TI
#define GDK_KEY_kana_TI 0x4c1
#endif
#ifndef GDK_KEY_braille_dots_13678
#define GDK_KEY_braille_dots_13678 0x10028e5
#endif
#ifndef GDK_KEY_braille_dots_124678
#define GDK_KEY_braille_dots_124678 0x10028eb
#endif
#ifndef GDK_KEY_botintegral
#define GDK_KEY_botintegral 0x8a5
#endif
#ifndef GDK_KEY_braille_dots_23567
#define GDK_KEY_braille_dots_23567 0x1002876
#endif
#ifndef GDK_KEY_kana_TA
#define GDK_KEY_kana_TA 0x4c0
#endif
#ifndef GDK_KEY_Zcaron
#define GDK_KEY_Zcaron 0x1ae
#endif
#ifndef GDK_KEY_F24
#define GDK_KEY_F24 0xffd5
#endif
#ifndef GDK_KEY_KP_2
#define GDK_KEY_KP_2 0xffb2
#endif
#ifndef GDK_KEY_Thai_rorua
#define GDK_KEY_Thai_rorua 0xdc3
#endif
#ifndef GDK_KEY_U
#define GDK_KEY_U 0x055
#endif
#ifndef GDK_KEY_braille_dots_3568
#define GDK_KEY_braille_dots_3568 0x10028b4
#endif
#ifndef GDK_KEY_Thai_lekkao
#define GDK_KEY_Thai_lekkao 0xdf9
#endif
#ifndef GDK_KEY_kana_i
#define GDK_KEY_kana_i 0x4a8
#endif
#ifndef GDK_KEY_L2
#define GDK_KEY_L2 0xffc9
#endif
#ifndef GDK_KEY_Calculator
#define GDK_KEY_Calculator 0x1008ff1d
#endif
#ifndef GDK_KEY_AccessX_Enable
#define GDK_KEY_AccessX_Enable 0xfe70
#endif
#ifndef GDK_KEY_Pointer_DownRight
#define GDK_KEY_Pointer_DownRight 0xfee7
#endif
#ifndef GDK_KEY_Armenian_se
#define GDK_KEY_Armenian_se 0x100057d
#endif
#ifndef GDK_KEY_Farsi_5
#define GDK_KEY_Farsi_5 0x10006f5
#endif
#ifndef GDK_KEY_sixsubscript
#define GDK_KEY_sixsubscript 0x1002086
#endif
#ifndef GDK_KEY_ToDoList
#define GDK_KEY_ToDoList 0x1008ff1f
#endif
#ifndef GDK_KEY_quoteleft
#define GDK_KEY_quoteleft 0x060
#endif
#ifndef GDK_KEY_Georgian_chin
#define GDK_KEY_Georgian_chin 0x10010e9
#endif
#ifndef GDK_KEY_braille_dots_1245
#define GDK_KEY_braille_dots_1245 0x100281b
#endif
#ifndef GDK_KEY_horizlinescan5
#define GDK_KEY_horizlinescan5 0x9f1
#endif
#ifndef GDK_KEY_Ukrainian_ghe_with_upturn
#define GDK_KEY_Ukrainian_ghe_with_upturn 0x6ad
#endif
#ifndef GDK_KEY_Farsi_0
#define GDK_KEY_Farsi_0 0x10006f0
#endif
#ifndef GDK_KEY_Adiaeresis
#define GDK_KEY_Adiaeresis 0x0c4
#endif
#ifndef GDK_KEY_guillemotright
#define GDK_KEY_guillemotright 0x0bb
#endif
#ifndef GDK_KEY_Cyrillic_KA_descender
#define GDK_KEY_Cyrillic_KA_descender 0x100049a
#endif
#ifndef GDK_KEY_kana_YA
#define GDK_KEY_kana_YA 0x4d4
#endif
#ifndef GDK_KEY_braille_dots_123456
#define GDK_KEY_braille_dots_123456 0x100283f
#endif
#ifndef GDK_KEY_1
#define GDK_KEY_1 0x031
#endif
#ifndef GDK_KEY_Georgian_las
#define GDK_KEY_Georgian_las 0x10010da
#endif
#ifndef GDK_KEY_hebrew_dalet
#define GDK_KEY_hebrew_dalet 0xce3
#endif
#ifndef GDK_KEY_Ocircumflexgrave
#define GDK_KEY_Ocircumflexgrave 0x1001ed2
#endif
#ifndef GDK_KEY_Cyrillic_PE
#define GDK_KEY_Cyrillic_PE 0x6f0
#endif
#ifndef GDK_KEY_C
#define GDK_KEY_C 0x043
#endif
#ifndef GDK_KEY_Cyrillic_ZHE
#define GDK_KEY_Cyrillic_ZHE 0x6f6
#endif
#ifndef GDK_KEY_Greek_switch
#define GDK_KEY_Greek_switch 0xff7e
#endif
#ifndef GDK_KEY_dead_A
#define GDK_KEY_dead_A 0xfe81
#endif
#ifndef GDK_KEY_heart
#define GDK_KEY_heart 0xaee
#endif
#ifndef GDK_KEY_OfficeHome
#define GDK_KEY_OfficeHome 0x1008ff6a
#endif
#ifndef GDK_KEY_hebrew_gimmel
#define GDK_KEY_hebrew_gimmel 0xce2
#endif
#ifndef GDK_KEY_hebrew_finalnun
#define GDK_KEY_hebrew_finalnun 0xcef
#endif
#ifndef GDK_KEY_Hangul_PreviousCandidate
#define GDK_KEY_Hangul_PreviousCandidate 0xff3e
#endif
#ifndef GDK_KEY_Byelorussian_SHORTU
#define GDK_KEY_Byelorussian_SHORTU 0x6be
#endif
#ifndef GDK_KEY_hebrew_zadi
#define GDK_KEY_hebrew_zadi 0xcf6
#endif
#ifndef GDK_KEY_Pointer_Drag5
#define GDK_KEY_Pointer_Drag5 0xfefd
#endif
#ifndef GDK_KEY_umacron
#define GDK_KEY_umacron 0x3fe
#endif
#ifndef GDK_KEY_Oslash
#define GDK_KEY_Oslash 0x0d8
#endif
#ifndef GDK_KEY_schwa
#define GDK_KEY_schwa 0x1000259
#endif
#ifndef GDK_KEY_Switch_VT_1
#define GDK_KEY_Switch_VT_1 0x1008fe01
#endif
#ifndef GDK_KEY_Cyrillic_EF
#define GDK_KEY_Cyrillic_EF 0x6e6
#endif
#ifndef GDK_KEY_ballotcross
#define GDK_KEY_ballotcross 0xaf4
#endif
#ifndef GDK_KEY_Atilde
#define GDK_KEY_Atilde 0x0c3
#endif
#ifndef GDK_KEY_Arabic_semicolon
#define GDK_KEY_Arabic_semicolon 0x5bb
#endif
#ifndef GDK_KEY_3270_ExSelect
#define GDK_KEY_3270_ExSelect 0xfd1b
#endif
#ifndef GDK_KEY_x
#define GDK_KEY_x 0x078
#endif
#ifndef GDK_KEY_braille_dots_234678
#define GDK_KEY_braille_dots_234678 0x10028ee
#endif
#ifndef GDK_KEY_braille_dots_12345678
#define GDK_KEY_braille_dots_12345678 0x10028ff
#endif
#ifndef GDK_KEY_dead_O
#define GDK_KEY_dead_O 0xfe87
#endif
#ifndef GDK_KEY_Macedonia_KJE
#define GDK_KEY_Macedonia_KJE 0x6bc
#endif
#ifndef GDK_KEY_kana_e
#define GDK_KEY_kana_e 0x4aa
#endif
#ifndef GDK_KEY_braille_dots_245678
#define GDK_KEY_braille_dots_245678 0x10028fa
#endif
#ifndef GDK_KEY_Cyrillic_SHCHA
#define GDK_KEY_Cyrillic_SHCHA 0x6fd
#endif
#ifndef GDK_KEY_AudioLowerVolume
#define GDK_KEY_AudioLowerVolume 0x1008ff11
#endif
#ifndef GDK_KEY_AudioRewind
#define GDK_KEY_AudioRewind 0x1008ff3e
#endif
#ifndef GDK_KEY_Community
#define GDK_KEY_Community 0x1008ff3d
#endif
#ifndef GDK_KEY_Thai_dodek
#define GDK_KEY_Thai_dodek 0xdb4
#endif
#ifndef GDK_KEY_uhorn
#define GDK_KEY_uhorn 0x10001b0
#endif
#ifndef GDK_KEY_dead_o
#define GDK_KEY_dead_o 0xfe86
#endif
#ifndef GDK_KEY_Oacute
#define GDK_KEY_Oacute 0x0d3
#endif
#ifndef GDK_KEY_Thai_lekha
#define GDK_KEY_Thai_lekha 0xdf5
#endif
#ifndef GDK_KEY_Greek_ETAaccent
#define GDK_KEY_Greek_ETAaccent 0x7a3
#endif
#ifndef GDK_KEY_prescription
#define GDK_KEY_prescription 0xad4
#endif
#ifndef GDK_KEY_uhorntilde
#define GDK_KEY_uhorntilde 0x1001eef
#endif
#ifndef GDK_KEY_Yacute
#define GDK_KEY_Yacute 0x0dd
#endif
#ifndef GDK_KEY_ahook
#define GDK_KEY_ahook 0x1001ea3
#endif
#ifndef GDK_KEY_musicalflat
#define GDK_KEY_musicalflat 0xaf6
#endif
#ifndef GDK_KEY_StickyKeys_Enable
#define GDK_KEY_StickyKeys_Enable 0xfe75
#endif
#ifndef GDK_KEY_Thai_saraae
#define GDK_KEY_Thai_saraae 0xde1
#endif
#ifndef GDK_KEY_Arabic_1
#define GDK_KEY_Arabic_1 0x1000661
#endif
#ifndef GDK_KEY_AudioNext
#define GDK_KEY_AudioNext 0x1008ff17
#endif
#ifndef GDK_KEY_Hangul_Pieub
#define GDK_KEY_Hangul_Pieub 0xeb2
#endif
#ifndef GDK_KEY_Georgian_rae
#define GDK_KEY_Georgian_rae 0x10010e0
#endif
#ifndef GDK_KEY_braille_dots_12457
#define GDK_KEY_braille_dots_12457 0x100285b
#endif
#ifndef GDK_KEY_amacron
#define GDK_KEY_amacron 0x3e0
#endif
#ifndef GDK_KEY_ibreve
#define GDK_KEY_ibreve 0x100012d
#endif
#ifndef GDK_KEY_KP_Tab
#define GDK_KEY_KP_Tab 0xff89
#endif
#ifndef GDK_KEY_Idiaeresis
#define GDK_KEY_Idiaeresis 0x0cf
#endif
#ifndef GDK_KEY_cursor
#define GDK_KEY_cursor 0xaff
#endif
#ifndef GDK_KEY_Armenian_GIM
#define GDK_KEY_Armenian_GIM 0x1000533
#endif
#ifndef GDK_KEY_Calendar
#define GDK_KEY_Calendar 0x1008ff20
#endif
#ifndef GDK_KEY_braille_dots_257
#define GDK_KEY_braille_dots_257 0x1002852
#endif
#ifndef GDK_KEY_braille_dots_134567
#define GDK_KEY_braille_dots_134567 0x100287d
#endif
#ifndef GDK_KEY_Cyrillic_dzhe
#define GDK_KEY_Cyrillic_dzhe 0x6af
#endif
#ifndef GDK_KEY_F4
#define GDK_KEY_F4 0xffc1
#endif
#ifndef GDK_KEY_rightt
#define GDK_KEY_rightt 0x9f5
#endif
#ifndef GDK_KEY_Excel
#define GDK_KEY_Excel 0x1008ff5c
#endif
#ifndef GDK_KEY_dead_horn
#define GDK_KEY_dead_horn 0xfe62
#endif
#ifndef GDK_KEY_Cyrillic_GHE
#define GDK_KEY_Cyrillic_GHE 0x6e7
#endif
#ifndef GDK_KEY_Ukranian_YI
#define GDK_KEY_Ukranian_YI 0x6b7
#endif
#ifndef GDK_KEY_Armenian_paruyk
#define GDK_KEY_Armenian_paruyk 0x100055e
#endif
#ifndef GDK_KEY_ihook
#define GDK_KEY_ihook 0x1001ec9
#endif
#ifndef GDK_KEY_F33
#define GDK_KEY_F33 0xffde
#endif
#ifndef GDK_KEY_Hangul_I
#define GDK_KEY_Hangul_I 0xed3
#endif
#ifndef GDK_KEY_LaunchB
#define GDK_KEY_LaunchB 0x1008ff4b
#endif
#ifndef GDK_KEY_horizlinescan1
#define GDK_KEY_horizlinescan1 0x9ef
#endif
#ifndef GDK_KEY_Arabic_percent
#define GDK_KEY_Arabic_percent 0x100066a
#endif
#ifndef GDK_KEY_MenuKB
#define GDK_KEY_MenuKB 0x1008ff65
#endif
#ifndef GDK_KEY_kana_HI
#define GDK_KEY_kana_HI 0x4cb
#endif
#ifndef GDK_KEY_Ncedilla
#define GDK_KEY_Ncedilla 0x3d1
#endif
#ifndef GDK_KEY_Cyrillic_JE
#define GDK_KEY_Cyrillic_JE 0x6b8
#endif
#ifndef GDK_KEY_Greek_eta
#define GDK_KEY_Greek_eta 0x7e7
#endif
#ifndef GDK_KEY_rightopentriangle
#define GDK_KEY_rightopentriangle 0xacd
#endif
#ifndef GDK_KEY_Georgian_har
#define GDK_KEY_Georgian_har 0x10010f4
#endif
#ifndef GDK_KEY_Hangul_WA
#define GDK_KEY_Hangul_WA 0xec8
#endif
#ifndef GDK_KEY_ISO_Partial_Line_Up
#define GDK_KEY_ISO_Partial_Line_Up 0xfe23
#endif
#ifndef GDK_KEY_braille_dots_23467
#define GDK_KEY_braille_dots_23467 0x100286e
#endif
#ifndef GDK_KEY_Armenian_BEN
#define GDK_KEY_Armenian_BEN 0x1000532
#endif
#ifndef GDK_KEY_Cyrillic_zhe
#define GDK_KEY_Cyrillic_zhe 0x6d6
#endif
#ifndef GDK_KEY_ht
#define GDK_KEY_ht 0x9e2
#endif
#ifndef GDK_KEY_Georgian_vin
#define GDK_KEY_Georgian_vin 0x10010d5
#endif
#ifndef GDK_KEY_braille_dots_123578
#define GDK_KEY_braille_dots_123578 0x10028d7
#endif
#ifndef GDK_KEY_oslash
#define GDK_KEY_oslash 0x0f8
#endif
#ifndef GDK_KEY_Georgian_can
#define GDK_KEY_Georgian_can 0x10010ea
#endif
#ifndef GDK_KEY_Uhornacute
#define GDK_KEY_Uhornacute 0x1001ee8
#endif
#ifndef GDK_KEY_braille_dots_2378
#define GDK_KEY_braille_dots_2378 0x10028c6
#endif
#ifndef GDK_KEY_vertbar
#define GDK_KEY_vertbar 0x9f8
#endif
#ifndef GDK_KEY_kana_MO
#define GDK_KEY_kana_MO 0x4d3
#endif
#ifndef GDK_KEY_Thai_sarauee
#define GDK_KEY_Thai_sarauee 0xdd7
#endif
#ifndef GDK_KEY_Armenian_KE
#define GDK_KEY_Armenian_KE 0x1000554
#endif
#ifndef GDK_KEY_Udiaeresis
#define GDK_KEY_Udiaeresis 0x0dc
#endif
#ifndef GDK_KEY_braille_dots_1245678
#define GDK_KEY_braille_dots_1245678 0x10028fb
#endif
#ifndef GDK_KEY_braille_dots_1345
#define GDK_KEY_braille_dots_1345 0x100281d
#endif
#ifndef GDK_KEY_User1KB
#define GDK_KEY_User1KB 0x1008ff85
#endif
#ifndef GDK_KEY_igrave
#define GDK_KEY_igrave 0x0ec
#endif
#ifndef GDK_KEY_Greek_EPSILONaccent
#define GDK_KEY_Greek_EPSILONaccent 0x7a2
#endif
#ifndef GDK_KEY_Armenian_KEN
#define GDK_KEY_Armenian_KEN 0x100053f
#endif
#ifndef GDK_KEY_Greek_SIGMA
#define GDK_KEY_Greek_SIGMA 0x7d2
#endif
#ifndef GDK_KEY_Cyrillic_EM
#define GDK_KEY_Cyrillic_EM 0x6ed
#endif
#ifndef GDK_KEY_Lstroke
#define GDK_KEY_Lstroke 0x1a3
#endif
#ifndef GDK_KEY_identical
#define GDK_KEY_identical 0x8cf
#endif
#ifndef GDK_KEY_lefttack
#define GDK_KEY_lefttack 0xbdc
#endif
#ifndef GDK_KEY_Georgian_xan
#define GDK_KEY_Georgian_xan 0x10010ee
#endif
#ifndef GDK_KEY_braille_dots_348
#define GDK_KEY_braille_dots_348 0x100288c
#endif
#ifndef GDK_KEY_Hangul_RieulTieut
#define GDK_KEY_Hangul_RieulTieut 0xeae
#endif
#ifndef GDK_KEY_braille_dots_24
#define GDK_KEY_braille_dots_24 0x100280a
#endif
#ifndef GDK_KEY_L3
#define GDK_KEY_L3 0xffca
#endif
#ifndef GDK_KEY_MonBrightnessDown
#define GDK_KEY_MonBrightnessDown 0x1008ff03
#endif

#ifndef GDK_KEY_Home
#define GDK_KEY_Home 0xff50
#endif
#ifndef GDK_KEY_Left
#define GDK_KEY_Left 0xff51
#endif
#ifndef GDK_KEY_Up
#define GDK_KEY_Up 0xff52
#endif
#ifndef GDK_KEY_Right
#define GDK_KEY_Right 0xff53
#endif
#ifndef GDK_KEY_Down
#define GDK_KEY_Down 0xff54
#endif
#ifndef GDK_KEY_Page_Up
#define GDK_KEY_Page_Up 0xff55
#endif
#ifndef GDK_KEY_Page_Down
#define GDK_KEY_Page_Down 0xff56
#endif
#ifndef GDK_KEY_End
#define GDK_KEY_End 0xff57
#endif
#ifndef GDK_KEY_Begin
#define GDK_KEY_Begin 0xff58
#endif

#ifndef GDK_KEY_Prior
#define GDK_KEY_Prior 0xff55
#endif
#ifndef GDK_KEY_Next
#define GDK_KEY_Next 0xff56
#endif

#endif /* __GDK_KEYSYMS_WRAPPER_H__ */
