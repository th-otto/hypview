/* Determine name of the currently selected locale.
   Copyright (C) 1995-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Ulrich Drepper <drepper@gnu.org>, 1995.  */
/* Native Windows code written by Tor Lillqvist <tml@iki.fi>.  */
/* Mac OS X code written by Bruno Haible <bruno@clisp.org>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "windows_.h"

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>
#include <string.h>

/* Specification.  */
#include "localename.h"

#ifdef HAVE_USELOCALE
/* Mac OS X 10.5 defines the locale_t type in <xlocale.h>.  */
# if defined __APPLE__ && defined __MACH__
#  include <xlocale.h>
# endif
# include <langinfo.h>
#endif

#if defined(HAVE_CFLOCALECOPYCURRENT) || defined(HAVE_CFPREFERENCESCOPYAPPVALUE)
# include <CoreFoundation/CFString.h>
# if defined(HAVE_CFLOCALECOPYCURRENT)
#  include <CoreFoundation/CFLocale.h>
# elif HAVE_CFPREFERENCESCOPYAPPVALUE
#  include <CoreFoundation/CFPreferences.h>
# endif
#endif

#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__ || defined __MSYS__
# define WINDOWS_NATIVE
#endif

#if defined WINDOWS_NATIVE
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
/* List of language codes, sorted by value:
   0x01 LANG_ARABIC
   0x02 LANG_BULGARIAN
   0x03 LANG_CATALAN
   0x04 LANG_CHINESE
   0x05 LANG_CZECH
   0x06 LANG_DANISH
   0x07 LANG_GERMAN
   0x08 LANG_GREEK
   0x09 LANG_ENGLISH
   0x0a LANG_SPANISH
   0x0b LANG_FINNISH
   0x0c LANG_FRENCH
   0x0d LANG_HEBREW
   0x0e LANG_HUNGARIAN
   0x0f LANG_ICELANDIC
   0x10 LANG_ITALIAN
   0x11 LANG_JAPANESE
   0x12 LANG_KOREAN
   0x13 LANG_DUTCH
   0x14 LANG_NORWEGIAN
   0x15 LANG_POLISH
   0x16 LANG_PORTUGUESE
   0x17 LANG_ROMANSH
   0x18 LANG_ROMANIAN
   0x19 LANG_RUSSIAN
   0x1a LANG_CROATIAN == LANG_SERBIAN
   0x1b LANG_SLOVAK
   0x1c LANG_ALBANIAN
   0x1d LANG_SWEDISH
   0x1e LANG_THAI
   0x1f LANG_TURKISH
   0x20 LANG_URDU
   0x21 LANG_INDONESIAN
   0x22 LANG_UKRAINIAN
   0x23 LANG_BELARUSIAN
   0x24 LANG_SLOVENIAN
   0x25 LANG_ESTONIAN
   0x26 LANG_LATVIAN
   0x27 LANG_LITHUANIAN
   0x28 LANG_TAJIK
   0x29 LANG_FARSI
   0x2a LANG_VIETNAMESE
   0x2b LANG_ARMENIAN
   0x2c LANG_AZERI
   0x2d LANG_BASQUE
   0x2e LANG_SORBIAN
   0x2f LANG_MACEDONIAN
   0x30 LANG_SUTU
   0x31 LANG_TSONGA
   0x32 LANG_TSWANA
   0x33 LANG_VENDA
   0x34 LANG_XHOSA
   0x35 LANG_ZULU
   0x36 LANG_AFRIKAANS
   0x37 LANG_GEORGIAN
   0x38 LANG_FAEROESE
   0x39 LANG_HINDI
   0x3a LANG_MALTESE
   0x3b LANG_SAMI
   0x3c LANG_GAELIC
   0x3d LANG_YIDDISH
   0x3e LANG_MALAY
   0x3f LANG_KAZAK
   0x40 LANG_KYRGYZ
   0x41 LANG_SWAHILI
   0x42 LANG_TURKMEN
   0x43 LANG_UZBEK
   0x44 LANG_TATAR
   0x45 LANG_BENGALI
   0x46 LANG_PUNJABI
   0x47 LANG_GUJARATI
   0x48 LANG_ORIYA
   0x49 LANG_TAMIL
   0x4a LANG_TELUGU
   0x4b LANG_KANNADA
   0x4c LANG_MALAYALAM
   0x4d LANG_ASSAMESE
   0x4e LANG_MARATHI
   0x4f LANG_SANSKRIT
   0x50 LANG_MONGOLIAN
   0x51 LANG_TIBETAN
   0x52 LANG_WELSH
   0x53 LANG_CAMBODIAN
   0x54 LANG_LAO
   0x55 LANG_BURMESE
   0x56 LANG_GALICIAN
   0x57 LANG_KONKANI
   0x58 LANG_MANIPURI
   0x59 LANG_SINDHI
   0x5a LANG_SYRIAC
   0x5b LANG_SINHALESE
   0x5c LANG_CHEROKEE
   0x5d LANG_INUKTITUT
   0x5e LANG_AMHARIC
   0x5f LANG_TAMAZIGHT
   0x60 LANG_KASHMIRI
   0x61 LANG_NEPALI
   0x62 LANG_FRISIAN
   0x63 LANG_PASHTO
   0x64 LANG_TAGALOG
   0x65 LANG_DIVEHI
   0x66 LANG_EDO
   0x67 LANG_FULFULDE
   0x68 LANG_HAUSA
   0x69 LANG_IBIBIO
   0x6a LANG_YORUBA
   0x6d LANG_BASHKIR
   0x6e LANG_LUXEMBOURGISH
   0x6f LANG_GREENLANDIC
   0x70 LANG_IGBO
   0x71 LANG_KANURI
   0x72 LANG_OROMO
   0x73 LANG_TIGRINYA
   0x74 LANG_GUARANI
   0x75 LANG_HAWAIIAN
   0x76 LANG_LATIN
   0x77 LANG_SOMALI
   0x78 LANG_YI
   0x79 LANG_PAPIAMENTU
   0x7a LANG_MAPUDUNGUN
   0x7c LANG_MOHAWK
   0x7e LANG_BRETON
   0x82 LANG_OCCITAN
   0x83 LANG_CORSICAN
   0x84 LANG_ALSATIAN
   0x85 LANG_YAKUT
   0x86 LANG_KICHE
   0x87 LANG_KINYARWANDA
   0x88 LANG_WOLOF
   0x8c LANG_DARI
   0x91 LANG_SCOTTISH_GAELIC
*/
/* Mingw headers don't have latest language and sublanguage codes.  */
#include "../win32/windows.rh"
# ifndef LANG_AFRIKAANS
# define LANG_AFRIKAANS 0x36
# endif
# ifndef LANG_ALBANIAN
# define LANG_ALBANIAN 0x1c
# endif
# ifndef LANG_ALSATIAN
# define LANG_ALSATIAN 0x84
# endif
# ifndef LANG_AMHARIC
# define LANG_AMHARIC 0x5e
# endif
# ifndef LANG_ARABIC
# define LANG_ARABIC 0x01
# endif
# ifndef LANG_ARMENIAN
# define LANG_ARMENIAN 0x2b
# endif
# ifndef LANG_ASSAMESE
# define LANG_ASSAMESE 0x4d
# endif
# ifndef LANG_AZERI
# define LANG_AZERI 0x2c
# endif
# ifndef LANG_BASHKIR
# define LANG_BASHKIR 0x6d
# endif
# ifndef LANG_BASQUE
# define LANG_BASQUE 0x2d
# endif
# ifndef LANG_BELARUSIAN
# define LANG_BELARUSIAN 0x23
# endif
# ifndef LANG_BENGALI
# define LANG_BENGALI 0x45
# endif
# ifndef LANG_BRETON
# define LANG_BRETON 0x7e
# endif
# ifndef LANG_BURMESE
# define LANG_BURMESE 0x55
# endif
# ifndef LANG_CAMBODIAN
# define LANG_CAMBODIAN 0x53
# endif
# ifndef LANG_CATALAN
# define LANG_CATALAN 0x03
# endif
# ifndef LANG_CHEROKEE
# define LANG_CHEROKEE 0x5c
# endif
# ifndef LANG_CORSICAN
# define LANG_CORSICAN 0x83
# endif
# ifndef LANG_DARI
# define LANG_DARI 0x8c
# endif
# ifndef LANG_DIVEHI
# define LANG_DIVEHI 0x65
# endif
# ifndef LANG_EDO
# define LANG_EDO 0x66
# endif
# ifndef LANG_ESTONIAN
# define LANG_ESTONIAN 0x25
# endif
# ifndef LANG_FAEROESE
# define LANG_FAEROESE 0x38
# endif
# ifndef LANG_FARSI
# define LANG_FARSI 0x29
# endif
# ifndef LANG_FRISIAN
# define LANG_FRISIAN 0x62
# endif
# ifndef LANG_FULFULDE
# define LANG_FULFULDE 0x67
# endif
# ifndef LANG_GAELIC
# define LANG_GAELIC 0x3c
# endif
# ifndef LANG_GALICIAN
# define LANG_GALICIAN 0x56
# endif
# ifndef LANG_GEORGIAN
# define LANG_GEORGIAN 0x37
# endif
# ifndef LANG_GREENLANDIC
# define LANG_GREENLANDIC 0x6f
# endif
# ifndef LANG_GUARANI
# define LANG_GUARANI 0x74
# endif
# ifndef LANG_GUJARATI
# define LANG_GUJARATI 0x47
# endif
# ifndef LANG_HAUSA
# define LANG_HAUSA 0x68
# endif
# ifndef LANG_HAWAIIAN
# define LANG_HAWAIIAN 0x75
# endif
# ifndef LANG_HEBREW
# define LANG_HEBREW 0x0d
# endif
# ifndef LANG_HINDI
# define LANG_HINDI 0x39
# endif
# ifndef LANG_IBIBIO
# define LANG_IBIBIO 0x69
# endif
# ifndef LANG_IGBO
# define LANG_IGBO 0x70
# endif
# ifndef LANG_INDONESIAN
# define LANG_INDONESIAN 0x21
# endif
# ifndef LANG_INUKTITUT
# define LANG_INUKTITUT 0x5d
# endif
# ifndef LANG_KANNADA
# define LANG_KANNADA 0x4b
# endif
# ifndef LANG_KANURI
# define LANG_KANURI 0x71
# endif
# ifndef LANG_KASHMIRI
# define LANG_KASHMIRI 0x60
# endif
# ifndef LANG_KAZAK
# define LANG_KAZAK 0x3f
# endif
# ifndef LANG_KICHE
# define LANG_KICHE 0x86
# endif
# ifndef LANG_KINYARWANDA
# define LANG_KINYARWANDA 0x87
# endif
# ifndef LANG_KONKANI
# define LANG_KONKANI 0x57
# endif
# ifndef LANG_KYRGYZ
# define LANG_KYRGYZ 0x40
# endif
# ifndef LANG_LAO
# define LANG_LAO 0x54
# endif
# ifndef LANG_LATIN
# define LANG_LATIN 0x76
# endif
# ifndef LANG_LATVIAN
# define LANG_LATVIAN 0x26
# endif
# ifndef LANG_LITHUANIAN
# define LANG_LITHUANIAN 0x27
# endif
# ifndef LANG_LUXEMBOURGISH
# define LANG_LUXEMBOURGISH 0x6e
# endif
# ifndef LANG_MACEDONIAN
# define LANG_MACEDONIAN 0x2f
# endif
# ifndef LANG_MALAY
# define LANG_MALAY 0x3e
# endif
# ifndef LANG_MALAYALAM
# define LANG_MALAYALAM 0x4c
# endif
# ifndef LANG_MALTESE
# define LANG_MALTESE 0x3a
# endif
# ifndef LANG_MANIPURI
# define LANG_MANIPURI 0x58
# endif
# ifndef LANG_MAORI
# define LANG_MAORI 0x81
# endif
# ifndef LANG_MAPUDUNGUN
# define LANG_MAPUDUNGUN 0x7a
# endif
# ifndef LANG_MARATHI
# define LANG_MARATHI 0x4e
# endif
# ifndef LANG_MOHAWK
# define LANG_MOHAWK 0x7c
# endif
# ifndef LANG_MONGOLIAN
# define LANG_MONGOLIAN 0x50
# endif
# ifndef LANG_NEPALI
# define LANG_NEPALI 0x61
# endif
# ifndef LANG_OCCITAN
# define LANG_OCCITAN 0x82
# endif
# ifndef LANG_ORIYA
# define LANG_ORIYA 0x48
# endif
# ifndef LANG_OROMO
# define LANG_OROMO 0x72
# endif
# ifndef LANG_PAPIAMENTU
# define LANG_PAPIAMENTU 0x79
# endif
# ifndef LANG_PASHTO
# define LANG_PASHTO 0x63
# endif
# ifndef LANG_PUNJABI
# define LANG_PUNJABI 0x46
# endif
# ifndef LANG_QUECHUA
# define LANG_QUECHUA 0x6b
# endif
# ifndef LANG_ROMANSH
# define LANG_ROMANSH 0x17
# endif
# ifndef LANG_SAMI
# define LANG_SAMI 0x3b
# endif
# ifndef LANG_SANSKRIT
# define LANG_SANSKRIT 0x4f
# endif
# ifndef LANG_SCOTTISH_GAELIC
# define LANG_SCOTTISH_GAELIC 0x91
# endif
# ifndef LANG_SERBIAN
# define LANG_SERBIAN 0x1a
# endif
#ifndef LANG_BURMESE
#  define LANG_BURMESE 0x55
#endif
# ifndef LANG_SINDHI
# define LANG_SINDHI 0x59
# endif
# ifndef LANG_SINHALESE
# define LANG_SINHALESE 0x5b
# endif
# ifndef LANG_SLOVAK
# define LANG_SLOVAK 0x1b
# endif
# ifndef LANG_SOMALI
# define LANG_SOMALI 0x77
# endif
# ifndef LANG_SORBIAN
# define LANG_SORBIAN 0x2e
# endif
# ifndef LANG_SOTHO
# define LANG_SOTHO 0x6c
# endif
# ifndef LANG_SUTU
# define LANG_SUTU 0x30
# endif
# ifndef LANG_SWAHILI
# define LANG_SWAHILI 0x41
# endif
# ifndef LANG_SYRIAC
# define LANG_SYRIAC 0x5a
# endif
# ifndef LANG_TAGALOG
# define LANG_TAGALOG 0x64
# endif
# ifndef LANG_TAJIK
# define LANG_TAJIK 0x28
# endif
# ifndef LANG_TAMAZIGHT
# define LANG_TAMAZIGHT 0x5f
# endif
# ifndef LANG_TAMIL
# define LANG_TAMIL 0x49
# endif
# ifndef LANG_TATAR
# define LANG_TATAR 0x44
# endif
# ifndef LANG_TELUGU
# define LANG_TELUGU 0x4a
# endif
# ifndef LANG_THAI
# define LANG_THAI 0x1e
# endif
# ifndef LANG_TIBETAN
# define LANG_TIBETAN 0x51
# endif
# ifndef LANG_TIGRINYA
# define LANG_TIGRINYA 0x73
# endif
# ifndef LANG_TSONGA
# define LANG_TSONGA 0x31
# endif
# ifndef LANG_TSWANA
# define LANG_TSWANA 0x32
# endif
# ifndef LANG_TURKMEN
# define LANG_TURKMEN 0x42
# endif
# ifndef LANG_UIGHUR
# define LANG_UIGHUR 0x80
# endif
# ifndef LANG_UKRAINIAN
# define LANG_UKRAINIAN 0x22
# endif
# ifndef LANG_URDU
# define LANG_URDU 0x20
# endif
# ifndef LANG_UZBEK
# define LANG_UZBEK 0x43
# endif
# ifndef LANG_VENDA
# define LANG_VENDA 0x33
# endif
# ifndef LANG_VIETNAMESE
# define LANG_VIETNAMESE 0x2a
# endif
# ifndef LANG_WELSH
# define LANG_WELSH 0x52
# endif
# ifndef LANG_WOLOF
# define LANG_WOLOF 0x88
# endif
# ifndef LANG_XHOSA
# define LANG_XHOSA 0x34
# endif
# ifndef LANG_YAKUT
# define LANG_YAKUT 0x85
# endif
# ifndef LANG_YI
# define LANG_YI 0x78
# endif
# ifndef LANG_YIDDISH
# define LANG_YIDDISH 0x3d
# endif
# ifndef LANG_YORUBA
# define LANG_YORUBA 0x6a
# endif
# ifndef LANG_ZULU
# define LANG_ZULU 0x35
# endif
#ifndef LANG_DZONGKHA
#  define LANG_DZONGKHA                 LANG_TIBETAN
#endif
# ifndef SUBLANG_AFRIKAANS_SOUTH_AFRICA
# define SUBLANG_AFRIKAANS_SOUTH_AFRICA 0x01
# endif
# ifndef SUBLANG_ALBANIAN_ALBANIA
# define SUBLANG_ALBANIAN_ALBANIA 0x01
# endif
# ifndef SUBLANG_ALSATIAN_FRANCE
# define SUBLANG_ALSATIAN_FRANCE 0x01
# endif
# ifndef SUBLANG_AMHARIC_ETHIOPIA
# define SUBLANG_AMHARIC_ETHIOPIA 0x01
# endif
# ifndef SUBLANG_ARABIC_SAUDI_ARABIA
# define SUBLANG_ARABIC_SAUDI_ARABIA 0x01
# endif
# ifndef SUBLANG_ARABIC_IRAQ
# define SUBLANG_ARABIC_IRAQ 0x02
# endif
# ifndef SUBLANG_ARABIC_EGYPT
# define SUBLANG_ARABIC_EGYPT 0x03
# endif
# ifndef SUBLANG_ARABIC_LIBYA
# define SUBLANG_ARABIC_LIBYA 0x04
# endif
# ifndef SUBLANG_ARABIC_ALGERIA
# define SUBLANG_ARABIC_ALGERIA 0x05
# endif
# ifndef SUBLANG_ARABIC_MOROCCO
# define SUBLANG_ARABIC_MOROCCO 0x06
# endif
# ifndef SUBLANG_ARABIC_TUNISIA
# define SUBLANG_ARABIC_TUNISIA 0x07
# endif
# ifndef SUBLANG_ARABIC_OMAN
# define SUBLANG_ARABIC_OMAN 0x08
# endif
# ifndef SUBLANG_ARABIC_YEMEN
# define SUBLANG_ARABIC_YEMEN 0x09
# endif
# ifndef SUBLANG_ARABIC_SYRIA
# define SUBLANG_ARABIC_SYRIA 0x0a
# endif
# ifndef SUBLANG_ARABIC_JORDAN
# define SUBLANG_ARABIC_JORDAN 0x0b
# endif
# ifndef SUBLANG_ARABIC_LEBANON
# define SUBLANG_ARABIC_LEBANON 0x0c
# endif
# ifndef SUBLANG_ARABIC_KUWAIT
# define SUBLANG_ARABIC_KUWAIT 0x0d
# endif
# ifndef SUBLANG_ARABIC_UAE
# define SUBLANG_ARABIC_UAE 0x0e
# endif
# ifndef SUBLANG_ARABIC_BAHRAIN
# define SUBLANG_ARABIC_BAHRAIN 0x0f
# endif
# ifndef SUBLANG_ARABIC_QATAR
# define SUBLANG_ARABIC_QATAR 0x10
# endif
# ifndef SUBLANG_ARMENIAN_ARMENIA
# define SUBLANG_ARMENIAN_ARMENIA 0x01
# endif
# ifndef SUBLANG_ASSAMESE_INDIA
# define SUBLANG_ASSAMESE_INDIA 0x01
# endif
# ifndef SUBLANG_AZERI_LATIN
# define SUBLANG_AZERI_LATIN 0x01
# endif
# ifndef SUBLANG_AZERI_CYRILLIC
# define SUBLANG_AZERI_CYRILLIC 0x02
# endif
# ifndef SUBLANG_BASHKIR_RUSSIA
# define SUBLANG_BASHKIR_RUSSIA 0x01
# endif
# ifndef SUBLANG_BASQUE_BASQUE
# define SUBLANG_BASQUE_BASQUE 0x01
# endif
# ifndef SUBLANG_BELARUSIAN_BELARUS
# define SUBLANG_BELARUSIAN_BELARUS 0x01
# endif
# ifndef SUBLANG_BENGALI_INDIA
# define SUBLANG_BENGALI_INDIA 0x01
# endif
# ifndef SUBLANG_BENGALI_BANGLADESH
# define SUBLANG_BENGALI_BANGLADESH 0x02
# endif
# ifndef SUBLANG_BOSNIAN_BOSNIA_HERZEGOVINA_LATIN
# define SUBLANG_BOSNIAN_BOSNIA_HERZEGOVINA_LATIN 0x05
# endif
# ifndef SUBLANG_BOSNIAN_BOSNIA_HERZEGOVINA_CYRILLIC
# define SUBLANG_BOSNIAN_BOSNIA_HERZEGOVINA_CYRILLIC 0x08
# endif
# ifndef SUBLANG_BRETON_FRANCE
# define SUBLANG_BRETON_FRANCE 0x01
# endif
# ifndef SUBLANG_BULGARIAN_BULGARIA
# define SUBLANG_BULGARIAN_BULGARIA 0x01
# endif
# ifndef SUBLANG_CAMBODIAN_CAMBODIA
# define SUBLANG_CAMBODIAN_CAMBODIA 0x01
# endif
# ifndef SUBLANG_CATALAN_SPAIN
# define SUBLANG_CATALAN_SPAIN 0x01
# endif
# ifndef SUBLANG_CORSICAN_FRANCE
# define SUBLANG_CORSICAN_FRANCE 0x01
# endif
# ifndef SUBLANG_CROATIAN_CROATIA
# define SUBLANG_CROATIAN_CROATIA 0x01
# endif
# ifndef SUBLANG_CROATIAN_BOSNIA_HERZEGOVINA_LATIN
# define SUBLANG_CROATIAN_BOSNIA_HERZEGOVINA_LATIN 0x04
# endif
# ifndef SUBLANG_CHINESE_MACAU
# define SUBLANG_CHINESE_MACAU 0x05
# endif
# ifndef SUBLANG_CZECH_CZECH_REPUBLIC
# define SUBLANG_CZECH_CZECH_REPUBLIC 0x01
# endif
# ifndef SUBLANG_DANISH_DENMARK
# define SUBLANG_DANISH_DENMARK 0x01
# endif
# ifndef SUBLANG_DARI_AFGHANISTAN
# define SUBLANG_DARI_AFGHANISTAN 0x01
# endif
# ifndef SUBLANG_DIVEHI_MALDIVES
# define SUBLANG_DIVEHI_MALDIVES 0x01
# endif
# ifndef SUBLANG_DUTCH_SURINAM
# define SUBLANG_DUTCH_SURINAM 0x03
# endif
# ifndef SUBLANG_ENGLISH_SOUTH_AFRICA
# define SUBLANG_ENGLISH_SOUTH_AFRICA 0x07
# endif
# ifndef SUBLANG_ENGLISH_JAMAICA
# define SUBLANG_ENGLISH_JAMAICA 0x08
# endif
# ifndef SUBLANG_ENGLISH_CARIBBEAN
# define SUBLANG_ENGLISH_CARIBBEAN 0x09
# endif
# ifndef SUBLANG_ENGLISH_BELIZE
# define SUBLANG_ENGLISH_BELIZE 0x0a
# endif
# ifndef SUBLANG_ENGLISH_TRINIDAD
# define SUBLANG_ENGLISH_TRINIDAD 0x0b
# endif
# ifndef SUBLANG_ENGLISH_ZIMBABWE
# define SUBLANG_ENGLISH_ZIMBABWE 0x0c
# endif
# ifndef SUBLANG_ENGLISH_PHILIPPINES
# define SUBLANG_ENGLISH_PHILIPPINES 0x0d
# endif
# ifndef SUBLANG_ENGLISH_INDONESIA
# define SUBLANG_ENGLISH_INDONESIA 0x0e
# endif
# ifndef SUBLANG_ENGLISH_HONGKONG
# define SUBLANG_ENGLISH_HONGKONG 0x0f
# endif
# ifndef SUBLANG_ENGLISH_INDIA
# define SUBLANG_ENGLISH_INDIA 0x10
# endif
# ifndef SUBLANG_ENGLISH_MALAYSIA
# define SUBLANG_ENGLISH_MALAYSIA 0x11
# endif
# ifndef SUBLANG_ENGLISH_SINGAPORE
# define SUBLANG_ENGLISH_SINGAPORE 0x12
# endif
# ifndef SUBLANG_ESTONIAN_ESTONIA
# define SUBLANG_ESTONIAN_ESTONIA 0x01
# endif
# ifndef SUBLANG_FAEROESE_FAROE_ISLANDS
# define SUBLANG_FAEROESE_FAROE_ISLANDS 0x01
# endif
# ifndef SUBLANG_FARSI_IRAN
# define SUBLANG_FARSI_IRAN 0x01
# endif
# ifndef SUBLANG_FINNISH_FINLAND
# define SUBLANG_FINNISH_FINLAND 0x01
# endif
# ifndef SUBLANG_FRENCH_LUXEMBOURG
# define SUBLANG_FRENCH_LUXEMBOURG 0x05
# endif
# ifndef SUBLANG_FRENCH_MONACO
# define SUBLANG_FRENCH_MONACO 0x06
# endif
# ifndef SUBLANG_FRENCH_WESTINDIES
# define SUBLANG_FRENCH_WESTINDIES 0x07
# endif
# ifndef SUBLANG_FRENCH_REUNION
# define SUBLANG_FRENCH_REUNION 0x08
# endif
# ifndef SUBLANG_FRENCH_CONGO
# define SUBLANG_FRENCH_CONGO 0x09
# endif
# ifndef SUBLANG_FRENCH_SENEGAL
# define SUBLANG_FRENCH_SENEGAL 0x0a
# endif
# ifndef SUBLANG_FRENCH_CAMEROON
# define SUBLANG_FRENCH_CAMEROON 0x0b
# endif
# ifndef SUBLANG_FRENCH_COTEDIVOIRE
# define SUBLANG_FRENCH_COTEDIVOIRE 0x0c
# endif
# ifndef SUBLANG_FRENCH_MALI
# define SUBLANG_FRENCH_MALI 0x0d
# endif
# ifndef SUBLANG_FRENCH_MOROCCO
# define SUBLANG_FRENCH_MOROCCO 0x0e
# endif
# ifndef SUBLANG_FRENCH_HAITI
# define SUBLANG_FRENCH_HAITI 0x0f
# endif
# ifndef SUBLANG_FRISIAN_NETHERLANDS
# define SUBLANG_FRISIAN_NETHERLANDS 0x01
# endif
# ifndef SUBLANG_GALICIAN_SPAIN
# define SUBLANG_GALICIAN_SPAIN 0x01
# endif
# ifndef SUBLANG_GEORGIAN_GEORGIA
# define SUBLANG_GEORGIAN_GEORGIA 0x01
# endif
# ifndef SUBLANG_GERMAN_LUXEMBOURG
# define SUBLANG_GERMAN_LUXEMBOURG 0x04
# endif
# ifndef SUBLANG_GERMAN_LIECHTENSTEIN
# define SUBLANG_GERMAN_LIECHTENSTEIN 0x05
# endif
# ifndef SUBLANG_GREEK_GREECE
# define SUBLANG_GREEK_GREECE 0x01
# endif
# ifndef SUBLANG_GREENLANDIC_GREENLAND
# define SUBLANG_GREENLANDIC_GREENLAND 0x01
# endif
# ifndef SUBLANG_GUJARATI_INDIA
# define SUBLANG_GUJARATI_INDIA 0x01
# endif
# ifndef SUBLANG_HAUSA_NIGERIA_LATIN
# define SUBLANG_HAUSA_NIGERIA_LATIN 0x01
# endif
# ifndef SUBLANG_HEBREW_ISRAEL
# define SUBLANG_HEBREW_ISRAEL 0x01
# endif
# ifndef SUBLANG_HINDI_INDIA
# define SUBLANG_HINDI_INDIA 0x01
# endif
# ifndef SUBLANG_HUNGARIAN_HUNGARY
# define SUBLANG_HUNGARIAN_HUNGARY 0x01
# endif
# ifndef SUBLANG_ICELANDIC_ICELAND
# define SUBLANG_ICELANDIC_ICELAND 0x01
# endif
# ifndef SUBLANG_IGBO_NIGERIA
# define SUBLANG_IGBO_NIGERIA 0x01
# endif
# ifndef SUBLANG_INDONESIAN_INDONESIA
# define SUBLANG_INDONESIAN_INDONESIA 0x01
# endif
# ifndef SUBLANG_INUKTITUT_CANADA
# define SUBLANG_INUKTITUT_CANADA 0x01
# endif
# undef SUBLANG_INUKTITUT_CANADA_LATIN
# define SUBLANG_INUKTITUT_CANADA_LATIN 0x02
# undef SUBLANG_IRISH_IRELAND
# define SUBLANG_IRISH_IRELAND 0x02
# ifndef SUBLANG_JAPANESE_JAPAN
# define SUBLANG_JAPANESE_JAPAN 0x01
# endif
# ifndef SUBLANG_KANNADA_INDIA
# define SUBLANG_KANNADA_INDIA 0x01
# endif
# ifndef SUBLANG_KASHMIRI_INDIA
# define SUBLANG_KASHMIRI_INDIA 0x02
# endif
# ifndef SUBLANG_KAZAK_KAZAKHSTAN
# define SUBLANG_KAZAK_KAZAKHSTAN 0x01
# endif
# ifndef SUBLANG_KICHE_GUATEMALA
# define SUBLANG_KICHE_GUATEMALA 0x01
# endif
# ifndef SUBLANG_KINYARWANDA_RWANDA
# define SUBLANG_KINYARWANDA_RWANDA 0x01
# endif
# ifndef SUBLANG_KONKANI_INDIA
# define SUBLANG_KONKANI_INDIA 0x01
# endif
# ifndef SUBLANG_KYRGYZ_KYRGYZSTAN
# define SUBLANG_KYRGYZ_KYRGYZSTAN 0x01
# endif
# ifndef SUBLANG_LAO_LAOS
# define SUBLANG_LAO_LAOS 0x01
# endif
# ifndef SUBLANG_LATVIAN_LATVIA
# define SUBLANG_LATVIAN_LATVIA 0x01
# endif
# ifndef SUBLANG_LITHUANIAN_LITHUANIA
# define SUBLANG_LITHUANIAN_LITHUANIA 0x01
# endif
# undef SUBLANG_LOWER_SORBIAN_GERMANY
# define SUBLANG_LOWER_SORBIAN_GERMANY 0x02
# ifndef SUBLANG_LUXEMBOURGISH_LUXEMBOURG
# define SUBLANG_LUXEMBOURGISH_LUXEMBOURG 0x01
# endif
# ifndef SUBLANG_MACEDONIAN_MACEDONIA
# define SUBLANG_MACEDONIAN_MACEDONIA 0x01
# endif
# ifndef SUBLANG_MALAY_MALAYSIA
# define SUBLANG_MALAY_MALAYSIA 0x01
# endif
# ifndef SUBLANG_MALAY_BRUNEI_DARUSSALAM
# define SUBLANG_MALAY_BRUNEI_DARUSSALAM 0x02
# endif
# ifndef SUBLANG_MALAYALAM_INDIA
# define SUBLANG_MALAYALAM_INDIA 0x01
# endif
# ifndef SUBLANG_MALTESE_MALTA
# define SUBLANG_MALTESE_MALTA 0x01
# endif
# ifndef SUBLANG_MAORI_NEW_ZEALAND
# define SUBLANG_MAORI_NEW_ZEALAND 0x01
# endif
# ifndef SUBLANG_MAPUDUNGUN_CHILE
# define SUBLANG_MAPUDUNGUN_CHILE 0x01
# endif
# ifndef SUBLANG_MARATHI_INDIA
# define SUBLANG_MARATHI_INDIA 0x01
# endif
# ifndef SUBLANG_MOHAWK_CANADA
# define SUBLANG_MOHAWK_CANADA 0x01
# endif
# ifndef SUBLANG_MONGOLIAN_CYRILLIC_MONGOLIA
# define SUBLANG_MONGOLIAN_CYRILLIC_MONGOLIA 0x01
# endif
# ifndef SUBLANG_MONGOLIAN_PRC
# define SUBLANG_MONGOLIAN_PRC 0x02
# endif
# ifndef SUBLANG_NEPALI_NEPAL
# define SUBLANG_NEPALI_NEPAL 0x01
# endif
# ifndef SUBLANG_NEPALI_INDIA
# define SUBLANG_NEPALI_INDIA 0x02
# endif
# ifndef SUBLANG_OCCITAN_FRANCE
# define SUBLANG_OCCITAN_FRANCE 0x01
# endif
# ifndef SUBLANG_ORIYA_INDIA
# define SUBLANG_ORIYA_INDIA 0x01
# endif
# ifndef SUBLANG_PASHTO_AFGHANISTAN
# define SUBLANG_PASHTO_AFGHANISTAN 0x01
# endif
# ifndef SUBLANG_POLISH_POLAND
# define SUBLANG_POLISH_POLAND 0x01
# endif
# ifndef SUBLANG_PUNJABI_INDIA
# define SUBLANG_PUNJABI_INDIA 0x01
# endif
# ifndef SUBLANG_PUNJABI_PAKISTAN
# define SUBLANG_PUNJABI_PAKISTAN 0x02
# endif
# ifndef SUBLANG_QUECHUA_BOLIVIA
# define SUBLANG_QUECHUA_BOLIVIA 0x01
# endif
# ifndef SUBLANG_QUECHUA_ECUADOR
# define SUBLANG_QUECHUA_ECUADOR 0x02
# endif
# ifndef SUBLANG_QUECHUA_PERU
# define SUBLANG_QUECHUA_PERU 0x03
# endif
# ifndef SUBLANG_ROMANIAN_ROMANIA
# define SUBLANG_ROMANIAN_ROMANIA 0x01
# endif
# ifndef SUBLANG_ROMANIAN_MOLDOVA
# define SUBLANG_ROMANIAN_MOLDOVA 0x02
# endif
# ifndef SUBLANG_ROMANSH_SWITZERLAND
# define SUBLANG_ROMANSH_SWITZERLAND 0x01
# endif
# ifndef SUBLANG_RUSSIAN_RUSSIA
# define SUBLANG_RUSSIAN_RUSSIA 0x01
# endif
# ifndef SUBLANG_RUSSIAN_MOLDAVIA
# define SUBLANG_RUSSIAN_MOLDAVIA 0x02
# endif
# ifndef SUBLANG_SAMI_NORTHERN_NORWAY
# define SUBLANG_SAMI_NORTHERN_NORWAY 0x01
# endif
# ifndef SUBLANG_SAMI_NORTHERN_SWEDEN
# define SUBLANG_SAMI_NORTHERN_SWEDEN 0x02
# endif
# ifndef SUBLANG_SAMI_NORTHERN_FINLAND
# define SUBLANG_SAMI_NORTHERN_FINLAND 0x03
# endif
# ifndef SUBLANG_SAMI_LULE_NORWAY
# define SUBLANG_SAMI_LULE_NORWAY 0x04
# endif
# ifndef SUBLANG_SAMI_LULE_SWEDEN
# define SUBLANG_SAMI_LULE_SWEDEN 0x05
# endif
# ifndef SUBLANG_SAMI_SOUTHERN_NORWAY
# define SUBLANG_SAMI_SOUTHERN_NORWAY 0x06
# endif
# ifndef SUBLANG_SAMI_SOUTHERN_SWEDEN
# define SUBLANG_SAMI_SOUTHERN_SWEDEN 0x07
# endif
# undef SUBLANG_SAMI_SKOLT_FINLAND
# define SUBLANG_SAMI_SKOLT_FINLAND 0x08
# undef SUBLANG_SAMI_INARI_FINLAND
# define SUBLANG_SAMI_INARI_FINLAND 0x09
# ifndef SUBLANG_SANSKRIT_INDIA
# define SUBLANG_SANSKRIT_INDIA 0x01
# endif
# ifndef SUBLANG_SERBIAN_LATIN
# define SUBLANG_SERBIAN_LATIN 0x02
# endif
# ifndef SUBLANG_SERBIAN_CYRILLIC
# define SUBLANG_SERBIAN_CYRILLIC 0x03
# endif
#ifndef SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_LATIN
#  define SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_LATIN 0x06 /* Serbian (Bosnia and Herzegovina - Latin) */
#endif
#ifndef SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_CYRILLIC
#  define SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_CYRILLIC 0x07 /* Serbian (Bosnia and Herzegovina - Cyrillic) */
#endif
#ifndef SUBLANG_SERBIAN_SERBIA_LATIN
#  define SUBLANG_SERBIAN_SERBIA_LATIN    0x09    /* Serbian (Serbia - Latin) */
#endif
#ifndef SUBLANG_SERBIAN_SERBIA_CYRILLIC
#  define SUBLANG_SERBIAN_SERBIA_CYRILLIC 0x0a    /* Serbian (Serbia - Cyrillic) */
#endif
#ifndef SUBLANG_SERBIAN_MONTENEGRO_LATIN
#  define SUBLANG_SERBIAN_MONTENEGRO_LATIN 0x0b   /* Serbian (Montenegro - Latn) */
#endif
#ifndef SUBLANG_SERBIAN_MONTENEGRO_CYRILLIC
#  define SUBLANG_SERBIAN_MONTENEGRO_CYRILLIC 0x0c /* Serbian (Montenegro - Cyrillic) */
#endif
# ifndef SUBLANG_SINDHI_INDIA
# define SUBLANG_SINDHI_INDIA 0x01
# endif
# undef SUBLANG_SINDHI_PAKISTAN
# define SUBLANG_SINDHI_PAKISTAN 0x02
# ifndef SUBLANG_SINDHI_AFGHANISTAN
# define SUBLANG_SINDHI_AFGHANISTAN 0x02
# endif
# ifndef SUBLANG_SINHALESE_SRI_LANKA
# define SUBLANG_SINHALESE_SRI_LANKA 0x01
# endif
# ifndef SUBLANG_SLOVAK_SLOVAKIA
# define SUBLANG_SLOVAK_SLOVAKIA 0x01
# endif
# ifndef SUBLANG_SLOVENIAN_SLOVENIA
# define SUBLANG_SLOVENIAN_SLOVENIA 0x01
# endif
# ifndef SUBLANG_SOTHO_SOUTH_AFRICA
# define SUBLANG_SOTHO_SOUTH_AFRICA 0x01
# endif
# ifndef SUBLANG_SPANISH_GUATEMALA
# define SUBLANG_SPANISH_GUATEMALA 0x04
# endif
# ifndef SUBLANG_SPANISH_COSTA_RICA
# define SUBLANG_SPANISH_COSTA_RICA 0x05
# endif
# ifndef SUBLANG_SPANISH_PANAMA
# define SUBLANG_SPANISH_PANAMA 0x06
# endif
# ifndef SUBLANG_SPANISH_DOMINICAN_REPUBLIC
# define SUBLANG_SPANISH_DOMINICAN_REPUBLIC 0x07
# endif
# ifndef SUBLANG_SPANISH_VENEZUELA
# define SUBLANG_SPANISH_VENEZUELA 0x08
# endif
# ifndef SUBLANG_SPANISH_COLOMBIA
# define SUBLANG_SPANISH_COLOMBIA 0x09
# endif
# ifndef SUBLANG_SPANISH_PERU
# define SUBLANG_SPANISH_PERU 0x0a
# endif
# ifndef SUBLANG_SPANISH_ARGENTINA
# define SUBLANG_SPANISH_ARGENTINA 0x0b
# endif
# ifndef SUBLANG_SPANISH_ECUADOR
# define SUBLANG_SPANISH_ECUADOR 0x0c
# endif
# ifndef SUBLANG_SPANISH_CHILE
# define SUBLANG_SPANISH_CHILE 0x0d
# endif
# ifndef SUBLANG_SPANISH_URUGUAY
# define SUBLANG_SPANISH_URUGUAY 0x0e
# endif
# ifndef SUBLANG_SPANISH_PARAGUAY
# define SUBLANG_SPANISH_PARAGUAY 0x0f
# endif
# ifndef SUBLANG_SPANISH_BOLIVIA
# define SUBLANG_SPANISH_BOLIVIA 0x10
# endif
# ifndef SUBLANG_SPANISH_EL_SALVADOR
# define SUBLANG_SPANISH_EL_SALVADOR 0x11
# endif
# ifndef SUBLANG_SPANISH_HONDURAS
# define SUBLANG_SPANISH_HONDURAS 0x12
# endif
# ifndef SUBLANG_SPANISH_NICARAGUA
# define SUBLANG_SPANISH_NICARAGUA 0x13
# endif
# ifndef SUBLANG_SPANISH_PUERTO_RICO
# define SUBLANG_SPANISH_PUERTO_RICO 0x14
# endif
# ifndef SUBLANG_SPANISH_US
# define SUBLANG_SPANISH_US 0x15
# endif
# ifndef SUBLANG_SWAHILI_KENYA
# define SUBLANG_SWAHILI_KENYA 0x01
# endif
# ifndef SUBLANG_SWEDISH_SWEDEN
# define SUBLANG_SWEDISH_SWEDEN 0x01
# endif
# ifndef SUBLANG_SWEDISH_FINLAND
# define SUBLANG_SWEDISH_FINLAND 0x02
# endif
# ifndef SUBLANG_SYRIAC_SYRIA
# define SUBLANG_SYRIAC_SYRIA 0x01
# endif
# ifndef SUBLANG_TAGALOG_PHILIPPINES
# define SUBLANG_TAGALOG_PHILIPPINES 0x01
# endif
# ifndef SUBLANG_TAJIK_TAJIKISTAN
# define SUBLANG_TAJIK_TAJIKISTAN 0x01
# endif
# ifndef SUBLANG_TAMAZIGHT_ARABIC
# define SUBLANG_TAMAZIGHT_ARABIC 0x01
# endif
# ifndef SUBLANG_TAMAZIGHT_ALGERIA_LATIN
# define SUBLANG_TAMAZIGHT_ALGERIA_LATIN 0x02
# endif
# ifndef SUBLANG_TAMIL_INDIA
# define SUBLANG_TAMIL_INDIA 0x01
# endif
# ifndef SUBLANG_TATAR_RUSSIA
# define SUBLANG_TATAR_RUSSIA 0x01
# endif
# ifndef SUBLANG_TELUGU_INDIA
# define SUBLANG_TELUGU_INDIA 0x01
# endif
# ifndef SUBLANG_THAI_THAILAND
# define SUBLANG_THAI_THAILAND 0x01
# endif
# ifndef SUBLANG_TIBETAN_PRC
# define SUBLANG_TIBETAN_PRC 0x01
# endif
# undef SUBLANG_TIBETAN_BHUTAN
# define SUBLANG_TIBETAN_BHUTAN 0x02
# ifndef SUBLANG_TIGRINYA_ETHIOPIA
# define SUBLANG_TIGRINYA_ETHIOPIA 0x01
# endif
# ifndef SUBLANG_TIGRINYA_ERITREA
# define SUBLANG_TIGRINYA_ERITREA 0x02
# endif
# ifndef SUBLANG_TSWANA_SOUTH_AFRICA
# define SUBLANG_TSWANA_SOUTH_AFRICA 0x01
# endif
# ifndef SUBLANG_TURKISH_TURKEY
# define SUBLANG_TURKISH_TURKEY 0x01
# endif
# ifndef SUBLANG_TURKMEN_TURKMENISTAN
# define SUBLANG_TURKMEN_TURKMENISTAN 0x01
# endif
# ifndef SUBLANG_UIGHUR_PRC
# define SUBLANG_UIGHUR_PRC 0x01
# endif
# ifndef SUBLANG_UKRAINIAN_UKRAINE
# define SUBLANG_UKRAINIAN_UKRAINE 0x01
# endif
# ifndef SUBLANG_UPPER_SORBIAN_GERMANY
# define SUBLANG_UPPER_SORBIAN_GERMANY 0x01
# endif
# ifndef SUBLANG_URDU_PAKISTAN
# define SUBLANG_URDU_PAKISTAN 0x01
# endif
# ifndef SUBLANG_URDU_INDIA
# define SUBLANG_URDU_INDIA 0x02
# endif
# ifndef SUBLANG_UZBEK_LATIN
# define SUBLANG_UZBEK_LATIN 0x01
# endif
# ifndef SUBLANG_UZBEK_CYRILLIC
# define SUBLANG_UZBEK_CYRILLIC 0x02
# endif
# ifndef SUBLANG_VIETNAMESE_VIETNAM
# define SUBLANG_VIETNAMESE_VIETNAM 0x01
# endif
# ifndef SUBLANG_WELSH_UNITED_KINGDOM
# define SUBLANG_WELSH_UNITED_KINGDOM 0x01
# endif
# ifndef SUBLANG_WOLOF_SENEGAL
# define SUBLANG_WOLOF_SENEGAL 0x01
# endif
# ifndef SUBLANG_XHOSA_SOUTH_AFRICA
# define SUBLANG_XHOSA_SOUTH_AFRICA 0x01
# endif
# ifndef SUBLANG_YAKUT_RUSSIA
# define SUBLANG_YAKUT_RUSSIA 0x01
# endif
# ifndef SUBLANG_YI_PRC
# define SUBLANG_YI_PRC 0x01
# endif
# ifndef SUBLANG_YORUBA_NIGERIA
# define SUBLANG_YORUBA_NIGERIA 0x01
# endif
# ifndef SUBLANG_ZULU_SOUTH_AFRICA
# define SUBLANG_ZULU_SOUTH_AFRICA 0x01
# endif
/* GetLocaleInfoA operations.  */
# ifndef LOCALE_SNAME
# define LOCALE_SNAME 0x5c
# endif
#endif


#if defined(HAVE_CFLOCALECOPYCURRENT) || defined(HAVE_CFPREFERENCESCOPYAPPVALUE)
/* Mac OS X 10.2 or newer */

/* Canonicalize a Mac OS X locale name to a Unix locale name.
   NAME is a sufficiently large buffer.
   On input, it contains the Mac OS X locale name.
   On output, it contains the Unix locale name.  */
static void gl_locale_name_canonicalize (char *name)
{
  /* This conversion is based on a posting by
     Deborah GoldSmith <goldsmit@apple.com> on 2005-03-08,
     http://lists.apple.com/archives/carbon-dev/2005/Mar/msg00293.html */

  /* Convert legacy (NeXTstep inherited) English names to Unix (ISO 639 and
     ISO 3166) names.  Prior to Mac OS X 10.3, there is no API for doing this.
     Therefore we do it ourselves, using a table based on the results of the
     Mac OS X 10.3.8 function
     CFLocaleCreateCanonicalLocaleIdentifierFromString().  */
  typedef struct { const char legacy[21+1]; const char unixy[5+1]; }
          legacy_entry;
  static const legacy_entry legacy_table[] = {
    { "Afrikaans",             "af" },
    { "Albanian",              "sq" },
    { "Amharic",               "am" },
    { "Arabic",                "ar" },
    { "Armenian",              "hy" },
    { "Assamese",              "as" },
    { "Aymara",                "ay" },
    { "Azerbaijani",           "az" },
    { "Basque",                "eu" },
    { "Belarusian",            "be" },
    { "Belorussian",           "be" },
    { "Bengali",               "bn" },
    { "Brazilian Portugese",   "pt_BR" },
    { "Brazilian Portuguese",  "pt_BR" },
    { "Breton",                "br" },
    { "Bulgarian",             "bg" },
    { "Burmese",               "my" },
    { "Byelorussian",          "be" },
    { "Catalan",               "ca" },
    { "Chewa",                 "ny" },
    { "Chichewa",              "ny" },
    { "Chinese",               "zh" },
    { "Chinese, Simplified",   "zh_CN" },
    { "Chinese, Traditional",  "zh_TW" },
    { "Chinese, Tradtional",   "zh_TW" },
    { "Croatian",              "hr" },
    { "Czech",                 "cs" },
    { "Danish",                "da" },
    { "Dutch",                 "nl" },
    { "Dzongkha",              "dz" },
    { "English",               "en" },
    { "Esperanto",             "eo" },
    { "Estonian",              "et" },
    { "Faroese",               "fo" },
    { "Farsi",                 "fa" },
    { "Finnish",               "fi" },
    { "Flemish",               "nl_BE" },
    { "French",                "fr" },
    { "Galician",              "gl" },
    { "Gallegan",              "gl" },
    { "Georgian",              "ka" },
    { "German",                "de" },
    { "Greek",                 "el" },
    { "Greenlandic",           "kl" },
    { "Guarani",               "gn" },
    { "Gujarati",              "gu" },
    { "Hawaiian",              "haw" }, /* Yes, "haw", not "cpe".  */
    { "Hebrew",                "he" },
    { "Hindi",                 "hi" },
    { "Hungarian",             "hu" },
    { "Icelandic",             "is" },
    { "Indonesian",            "id" },
    { "Inuktitut",             "iu" },
    { "Irish",                 "ga" },
    { "Italian",               "it" },
    { "Japanese",              "ja" },
    { "Javanese",              "jv" },
    { "Kalaallisut",           "kl" },
    { "Kannada",               "kn" },
    { "Kashmiri",              "ks" },
    { "Kazakh",                "kk" },
    { "Khmer",                 "km" },
    { "Kinyarwanda",           "rw" },
    { "Kirghiz",               "ky" },
    { "Korean",                "ko" },
    { "Kurdish",               "ku" },
    { "Latin",                 "la" },
    { "Latvian",               "lv" },
    { "Lithuanian",            "lt" },
    { "Macedonian",            "mk" },
    { "Malagasy",              "mg" },
    { "Malay",                 "ms" },
    { "Malayalam",             "ml" },
    { "Maltese",               "mt" },
    { "Manx",                  "gv" },
    { "Marathi",               "mr" },
    { "Moldavian",             "mo" },
    { "Mongolian",             "mn" },
    { "Nepali",                "ne" },
    { "Norwegian",             "nb" }, /* Yes, "nb", not the obsolete "no".  */
    { "Nyanja",                "ny" },
    { "Nynorsk",               "nn" },
    { "Oriya",                 "or" },
    { "Oromo",                 "om" },
    { "Panjabi",               "pa" },
    { "Pashto",                "ps" },
    { "Persian",               "fa" },
    { "Polish",                "pl" },
    { "Portuguese",            "pt" },
    { "Portuguese, Brazilian", "pt_BR" },
    { "Punjabi",               "pa" },
    { "Pushto",                "ps" },
    { "Quechua",               "qu" },
    { "Romanian",              "ro" },
    { "Ruanda",                "rw" },
    { "Rundi",                 "rn" },
    { "Russian",               "ru" },
    { "Sami",                  "se_NO" }, /* Not just "se".  */
    { "Sanskrit",              "sa" },
    { "Scottish",              "gd" },
    { "Serbian",               "sr" },
    { "Simplified Chinese",    "zh_CN" },
    { "Sindhi",                "sd" },
    { "Sinhalese",             "si" },
    { "Slovak",                "sk" },
    { "Slovenian",             "sl" },
    { "Somali",                "so" },
    { "Spanish",               "es" },
    { "Sundanese",             "su" },
    { "Swahili",               "sw" },
    { "Swedish",               "sv" },
    { "Tagalog",               "tl" },
    { "Tajik",                 "tg" },
    { "Tajiki",                "tg" },
    { "Tamil",                 "ta" },
    { "Tatar",                 "tt" },
    { "Telugu",                "te" },
    { "Thai",                  "th" },
    { "Tibetan",               "bo" },
    { "Tigrinya",              "ti" },
    { "Tongan",                "to" },
    { "Traditional Chinese",   "zh_TW" },
    { "Turkish",               "tr" },
    { "Turkmen",               "tk" },
    { "Uighur",                "ug" },
    { "Ukrainian",             "uk" },
    { "Urdu",                  "ur" },
    { "Uzbek",                 "uz" },
    { "Vietnamese",            "vi" },
    { "Welsh",                 "cy" },
    { "Yiddish",               "yi" }
  };

  /* Convert new-style locale names with language tags (ISO 639 and ISO 15924)
     to Unix (ISO 639 and ISO 3166) names.  */
  typedef struct { const char langtag[7+1]; const char unixy[12+1]; }
          langtag_entry;
  static const langtag_entry langtag_table[] = {
    /* Mac OS X has "az-Arab", "az-Cyrl", "az-Latn".
       The default script for az on Unix is Latin.  */
    { "az-Latn", "az" },
    /* Mac OS X has "ga-dots".  Does not yet exist on Unix.  */
    { "ga-dots", "ga" },
    /* Mac OS X has "kk-Cyrl".  Does not yet exist on Unix.  */
    /* Mac OS X has "mn-Cyrl", "mn-Mong".
       The default script for mn on Unix is Cyrillic.  */
    { "mn-Cyrl", "mn" },
    /* Mac OS X has "ms-Arab", "ms-Latn".
       The default script for ms on Unix is Latin.  */
    { "ms-Latn", "ms" },
    /* Mac OS X has "tg-Cyrl".
       The default script for tg on Unix is Cyrillic.  */
    { "tg-Cyrl", "tg" },
    /* Mac OS X has "tk-Cyrl".  Does not yet exist on Unix.  */
    /* Mac OS X has "tt-Cyrl".
       The default script for tt on Unix is Cyrillic.  */
    { "tt-Cyrl", "tt" },
    /* Mac OS X has "zh-Hans", "zh-Hant".
       Country codes are used to distinguish these on Unix.  */
    { "zh-Hans", "zh_CN" },
    { "zh-Hant", "zh_TW" }
  };

  /* Convert script names (ISO 15924) to Unix conventions.
     See http://www.unicode.org/iso15924/iso15924-codes.html  */
  typedef struct { const char script[4+1]; const char unixy[9+1]; }
          script_entry;
  static const script_entry script_table[] = {
    { "Arab", "arabic" },
    { "Cyrl", "cyrillic" },
    { "Mong", "mongolian" }
  };

  /* Step 1: Convert using legacy_table.  */
  if (name[0] >= 'A' && name[0] <= 'Z')
    {
      unsigned int i1, i2;
      i1 = 0;
      i2 = sizeof (legacy_table) / sizeof (legacy_entry);
      while (i2 - i1 > 1)
        {
          /* At this point we know that if name occurs in legacy_table,
             its index must be >= i1 and < i2.  */
          unsigned int i = (i1 + i2) >> 1;
          const legacy_entry *p = &legacy_table[i];
          if (strcmp (name, p->legacy) < 0)
            i2 = i;
          else
            i1 = i;
        }
      if (strcmp (name, legacy_table[i1].legacy) == 0)
        {
          strcpy (name, legacy_table[i1].unixy);
          return;
        }
    }

  /* Step 2: Convert using langtag_table and script_table.  */
  if (strlen (name) == 7 && name[2] == '-')
    {
      unsigned int i1, i2;
      i1 = 0;
      i2 = sizeof (langtag_table) / sizeof (langtag_entry);
      while (i2 - i1 > 1)
        {
          /* At this point we know that if name occurs in langtag_table,
             its index must be >= i1 and < i2.  */
          unsigned int i = (i1 + i2) >> 1;
          const langtag_entry *p = &langtag_table[i];
          if (strcmp (name, p->langtag) < 0)
            i2 = i;
          else
            i1 = i;
        }
      if (strcmp (name, langtag_table[i1].langtag) == 0)
        {
          strcpy (name, langtag_table[i1].unixy);
          return;
        }

      i1 = 0;
      i2 = sizeof (script_table) / sizeof (script_entry);
      while (i2 - i1 > 1)
        {
          /* At this point we know that if (name + 3) occurs in script_table,
             its index must be >= i1 and < i2.  */
          unsigned int i = (i1 + i2) >> 1;
          const script_entry *p = &script_table[i];
          if (strcmp (name + 3, p->script) < 0)
            i2 = i;
          else
            i1 = i;
        }
      if (strcmp (name + 3, script_table[i1].script) == 0)
        {
          name[2] = '@';
          strcpy (name + 3, script_table[i1].unixy);
          return;
        }
    }

  /* Step 3: Convert new-style dash to Unix underscore. */
  {
    char *p;
    for (p = name; *p != '\0'; p++)
      if (*p == '-')
        *p = '_';
  }
}

#endif


#if defined WINDOWS_NATIVE

/* Canonicalize a Windows native locale name to a Unix locale name.
   NAME is a sufficiently large buffer.
   On input, it contains the Windows locale name.
   On output, it contains the Unix locale name.  */
static void gl_locale_name_canonicalize (char *name)
{
  /* FIXME: This is probably incomplete: it does not handle "zh-Hans" and
     "zh-Hant".  */
  char *p;

  for (p = name; *p != '\0'; p++)
    if (*p == '-')
      {
        *p = '_';
        p++;
        for (; *p != '\0'; p++)
          {
            if (*p >= 'a' && *p <= 'z')
              *p += 'A' - 'a';
            if (*p == '-')
              {
                *p = '\0';
                return;
              }
          }
        return;
      }
}

/* Dispatch on language.
   See also http://www.unicode.org/unicode/onlinedat/languages.html .
   For details about languages, see http://www.ethnologue.com/ .
 */
static struct _sublang_table {
	LANGID id;
	unsigned short namelen;
	char po_name[8];
} const sublang_table[] = {
#define LN(x) (unsigned short)sizeof(x) - 1, x
	{ MAKELANGID(LANG_AFRIKAANS, SUBLANG_AFRIKAANS_SOUTH_AFRICA), LN("af_ZA") },
	{ MAKELANGID(LANG_AFRIKAANS, SUBLANG_NEUTRAL), LN("af") },
	{ MAKELANGID(LANG_ALBANIAN, SUBLANG_ALBANIAN_ALBANIA), LN("sq_AL") },
	{ MAKELANGID(LANG_ALBANIAN, SUBLANG_NEUTRAL), LN("sq") },
	{ MAKELANGID(LANG_ALSATIAN, SUBLANG_ALSATIAN_FRANCE), LN("al_FR") },
	{ MAKELANGID(LANG_ALSATIAN, SUBLANG_NEUTRAL), LN("al") },
	{ MAKELANGID(LANG_AMHARIC, SUBLANG_AMHARIC_ETHIOPIA), LN("am_ET") },
	{ MAKELANGID(LANG_AMHARIC, SUBLANG_NEUTRAL), LN("am") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_SAUDI_ARABIA), LN("ar_SA") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_IRAQ), LN("ar_IQ") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_EGYPT), LN("ar_EG") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_LIBYA), LN("ar_LY") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_ALGERIA), LN("ar_DZ") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_MOROCCO), LN("ar_MA") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_TUNISIA), LN("ar_TN") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_OMAN), LN("ar_OM") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_YEMEN), LN("ar_YE") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_SYRIA), LN("ar_SY") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_JORDAN), LN("ar_JO") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_LEBANON), LN("ar_LB") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_KUWAIT), LN("ar_KW") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_UAE), LN("ar_AE") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_BAHRAIN), LN("ar_BH") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_ARABIC_QATAR), LN("ar_QA") },
	{ MAKELANGID(LANG_ARABIC, SUBLANG_NEUTRAL), LN("ar") },
	{ MAKELANGID(LANG_ARMENIAN, SUBLANG_ARMENIAN_ARMENIA), LN("hy_AM") },
	{ MAKELANGID(LANG_ARMENIAN, SUBLANG_NEUTRAL), LN("hy") },
	{ MAKELANGID(LANG_ASSAMESE, SUBLANG_ASSAMESE_INDIA), LN("as_IN") },
	{ MAKELANGID(LANG_ASSAMESE, SUBLANG_NEUTRAL), LN("as") },
	{ MAKELANGID(LANG_AZERI, SUBLANG_AZERI_LATIN), LN("az_AZ") },
	{ MAKELANGID(LANG_AZERI, SUBLANG_AZERI_CYRILLIC), LN("az_AZ") },
	{ MAKELANGID(LANG_AZERI, 0x1d), LN("az") },
	{ MAKELANGID(LANG_AZERI, 0x1e), LN("az") },
	{ MAKELANGID(LANG_AZERI, SUBLANG_NEUTRAL), LN("az") },
	{ MAKELANGID(LANG_BASHKIR, SUBLANG_BASHKIR_RUSSIA), LN("ba_RU") },
	{ MAKELANGID(LANG_BASHKIR, SUBLANG_NEUTRAL), LN("ba") },
	{ MAKELANGID(LANG_BASQUE, SUBLANG_BASQUE_BASQUE), LN("eu_ES") },
	{ MAKELANGID(LANG_BASQUE, SUBLANG_NEUTRAL), LN("eu") },
	{ MAKELANGID(LANG_BELARUSIAN, SUBLANG_BELARUSIAN_BELARUS), LN("be_BY") },
	{ MAKELANGID(LANG_BELARUSIAN, SUBLANG_NEUTRAL), LN("be") },
	{ MAKELANGID(LANG_BENGALI, SUBLANG_BENGALI_INDIA), LN("bn_IN") },
	{ MAKELANGID(LANG_BENGALI, SUBLANG_BENGALI_BANGLADESH), LN("bn_BD") },
	{ MAKELANGID(LANG_BENGALI, SUBLANG_NEUTRAL), LN("bn") },
	{ MAKELANGID(LANG_BRETON, SUBLANG_BRETON_FRANCE), LN("br_FR") },
	{ MAKELANGID(LANG_BRETON, SUBLANG_NEUTRAL), LN("br") },
	{ MAKELANGID(LANG_BOSNIAN, SUBLANG_NEUTRAL), LN("bs") },
	{ MAKELANGID(LANG_BULGARIAN, SUBLANG_BULGARIAN_BULGARIA), LN("bg_BG") },
	{ MAKELANGID(LANG_BULGARIAN, SUBLANG_NEUTRAL), LN("bg") },
	{ MAKELANGID(LANG_BURMESE, SUBLANG_BURMESE_BURMA), LN("my_MM") },
	{ MAKELANGID(LANG_BURMESE, SUBLANG_NEUTRAL), LN("my") },
	{ MAKELANGID(LANG_CATALAN, SUBLANG_CATALAN_CATALAN), LN("ca_ES") },
	{ MAKELANGID(LANG_CATALAN, SUBLANG_VALENCIAN_VALENCIA), LN("ca_ES") },
	{ MAKELANGID(LANG_CATALAN, SUBLANG_NEUTRAL), LN("ca") },
	{ MAKELANGID(LANG_CHEROKEE, SUBLANG_CHEROKEE_CHEROKEE), LN("chr_US") },
	{ MAKELANGID(LANG_CHEROKEE, SUBLANG_NEUTRAL), LN("chr") },
	{ MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL), LN("zh_TW") },
	{ MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), LN("zh_CN") },
	{ MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG), LN("zh_HK") },
	{ MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE), LN("zh_SG") },
	{ MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_MACAU), LN("zh_MO") },
	{ MAKELANGID(LANG_CHINESE, 0x1f), LN("zh_TW") },
	{ MAKELANGID(LANG_CHINESE, SUBLANG_NEUTRAL), LN("zh") },
	{ MAKELANGID(LANG_CORSICAN, SUBLANG_CORSICAN_FRANCE), LN("co_FR") },
	{ MAKELANGID(LANG_CORSICAN, SUBLANG_NEUTRAL), LN("co") },
	{ MAKELANGID(LANG_CZECH, SUBLANG_CZECH_CZECH_REPUBLIC), LN("cs_CZ") },
	{ MAKELANGID(LANG_CZECH, SUBLANG_NEUTRAL), LN("cs") },
	{ MAKELANGID(LANG_DANISH, SUBLANG_DANISH_DENMARK), LN("da_DK") },
	{ MAKELANGID(LANG_DANISH, SUBLANG_NEUTRAL), LN("ds") },
	{ MAKELANGID(LANG_DARI, SUBLANG_DARI_AFGHANISTAN), LN("prs_AF") },
	{ MAKELANGID(LANG_DARI, SUBLANG_NEUTRAL), LN("prs") },
	{ MAKELANGID(LANG_DIVEHI, SUBLANG_DIVEHI_MALDIVES), LN("dv_MV") },
	{ MAKELANGID(LANG_DIVEHI, SUBLANG_NEUTRAL), LN("dv") },
	{ MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH), LN("nl_NL") },
	{ MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH_BELGIAN), LN("nl_BE") },
	{ MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH_SURINAM), LN("nl_SR") },
	{ MAKELANGID(LANG_DUTCH, SUBLANG_NEUTRAL), LN("nl") },
	{ MAKELANGID(LANG_DZONGKHA, SUBLANG_DZONGKHA_BHUTAN), LN("dz_BT") },
	{ MAKELANGID(LANG_DZONGKHA, SUBLANG_NEUTRAL), LN("dz") },
	{ MAKELANGID(LANG_EDO,  SUBLANG_EDO_NIGERIA), LN("bin_NG") },
	{ MAKELANGID(LANG_EDO, SUBLANG_NEUTRAL), LN("bin") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), LN("en_US") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_UK), LN("en_GB") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_AUS), LN("en_AU") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_CAN), LN("en_CA") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_NZ), LN("en_NZ") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_EIRE), LN("en_IE") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_IRELAND), LN("en_IE") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_SOUTH_AFRICA), LN("en_ZA") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_JAMAICA), LN("en_JM") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_CARIBBEAN), LN("en_GD") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_BELIZE), LN("en_BZ") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_TRINIDAD), LN("en_TT") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_ZIMBABWE), LN("en_ZW") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_PHILIPPINES), LN("en_PH") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_INDONESIA), LN("en_ID") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_HONGKONG), LN("en_HK") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_INDIA), LN("en_IN") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_MALAYSIA), LN("en_MY") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_SINGAPORE), LN("en_SG") },
	{ MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), LN("en") },
	{ MAKELANGID(LANG_ESTONIAN, SUBLANG_ESTONIAN_ESTONIA), LN("et_EE") },
	{ MAKELANGID(LANG_ESTONIAN, SUBLANG_NEUTRAL), LN("et") },
	{ MAKELANGID(LANG_FAEROESE, SUBLANG_FAEROESE_FAROE_ISLANDS), LN("fo_FO") },
	{ MAKELANGID(LANG_FAEROESE, SUBLANG_NEUTRAL), LN("fo") },
	{ MAKELANGID(LANG_FARSI, SUBLANG_FARSI_IRAN), LN("fa_IR") },
	{ MAKELANGID(LANG_FARSI, SUBLANG_NEUTRAL), LN("fa") },
	{ MAKELANGID(LANG_FILIPINO, SUBLANG_FILIPINO_PHILIPPINES), LN("fil_PH") },
	{ MAKELANGID(LANG_FILIPINO, SUBLANG_NEUTRAL), LN("fil") },
	{ MAKELANGID(LANG_FINNISH, SUBLANG_FINNISH_FINLAND), LN("fi_FI") },
	{ MAKELANGID(LANG_FINNISH, SUBLANG_NEUTRAL), LN("fi") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH), LN("fr_FR") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_BELGIAN), LN("fr_BE") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_CANADIAN), LN("fr_CA") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_SWISS), LN("fr_CH") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_LUXEMBOURG), LN("fr_LU") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_MONACO), LN("fr_MC") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_WESTINDIES), LN("fr_CB") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_REUNION), LN("fr_RE") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_CONGO), LN("fr_CG") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_SENEGAL), LN("fr_SN") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_CAMEROON), LN("fr_CM") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_COTEDIVOIRE), LN("fr_CI") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_MALI), LN("fr_ML") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_MOROCCO), LN("fr_MA") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH_HAITI), LN("fr_HT") },
	{ MAKELANGID(LANG_FRENCH, SUBLANG_NEUTRAL), LN("fr") },
	{ MAKELANGID(LANG_FRISIAN, SUBLANG_FRISIAN_NETHERLANDS), LN("fy_NL") },
	{ MAKELANGID(LANG_FRISIAN, SUBLANG_NEUTRAL), LN("fy") },
	{ MAKELANGID(LANG_FULAH, SUBLANG_FULAH_NIGERIA), LN("ff_NG") },
	{ MAKELANGID(LANG_FULAH, SUBLANG_FULAH_SENEGAL), LN("ff_SN") },
	{ MAKELANGID(LANG_FULAH, SUBLANG_NEUTRAL), LN("ff") },
	{ MAKELANGID(LANG_GALICIAN, SUBLANG_GALICIAN_GALICIAN), LN("gl_ES") },
	{ MAKELANGID(LANG_GALICIAN, SUBLANG_NEUTRAL), LN("gl") },
	{ MAKELANGID(LANG_GEORGIAN, SUBLANG_GEORGIAN_GEORGIA), LN("ka_GE") },
	{ MAKELANGID(LANG_GEORGIAN, SUBLANG_NEUTRAL), LN("ka") },
	{ MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN), LN("de_DE") },
	{ MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN_SWISS), LN("de_CH") },
	{ MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN_AUSTRIAN), LN("de_AT") },
	{ MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN_LUXEMBOURG), LN("de_LU") },
	{ MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN_LIECHTENSTEIN), LN("de_LI") },
	{ MAKELANGID(LANG_GERMAN, SUBLANG_NEUTRAL), LN("de") },
	{ MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE), LN("el_GR") },
	{ MAKELANGID(LANG_GREEK, SUBLANG_NEUTRAL), LN("el") },
	{ MAKELANGID(LANG_GREENLANDIC, SUBLANG_GREENLANDIC_GREENLAND), LN("kl_GL") },
	{ MAKELANGID(LANG_GREENLANDIC, SUBLANG_NEUTRAL), LN("kl") },
	{ MAKELANGID(LANG_GUARANI, SUBLANG_GUARANI_PARAGUAY), LN("gn_PY") },
	{ MAKELANGID(LANG_GUARANI, SUBLANG_NEUTRAL), LN("gn") },
	{ MAKELANGID(LANG_GUJARATI, SUBLANG_GUJARATI_INDIA), LN("gu_IN") },
	{ MAKELANGID(LANG_GUJARATI, SUBLANG_NEUTRAL), LN("gu") },
	{ MAKELANGID(LANG_HAUSA, SUBLANG_HAUSA_NIGERIA_LATIN), LN("ha_NG") },
	{ MAKELANGID(LANG_HAUSA, 0x1f), LN("ha") },
	{ MAKELANGID(LANG_HAUSA, SUBLANG_NEUTRAL), LN("ha") },
	{ MAKELANGID(LANG_HAWAIIAN, SUBLANG_HAWAIIAN_US), LN("haw_US") },
	{ MAKELANGID(LANG_HAWAIIAN, SUBLANG_DEFAULT), LN("cpe_US") },
	{ MAKELANGID(LANG_HAWAIIAN, SUBLANG_NEUTRAL), LN("haw") },
	{ MAKELANGID(LANG_HEBREW, SUBLANG_HEBREW_ISRAEL), LN("he_IL") },
	{ MAKELANGID(LANG_HEBREW, SUBLANG_NEUTRAL), LN("he") },
	{ MAKELANGID(LANG_HINDI, SUBLANG_HINDI_INDIA), LN("hi_IN") },
	{ MAKELANGID(LANG_HINDI, SUBLANG_NEUTRAL), LN("hi") },
	{ MAKELANGID(LANG_HUNGARIAN, SUBLANG_HUNGARIAN_HUNGARY), LN("hu_HU") },
	{ MAKELANGID(LANG_HUNGARIAN, SUBLANG_NEUTRAL), LN("hu") },
	{ MAKELANGID(LANG_IBIBIO, SUBLANG_IBIBIO_NIGERIA), LN("ibb_NG") },
	{ MAKELANGID(LANG_IBIBIO, SUBLANG_NEUTRAL), LN("ibb") },
	{ MAKELANGID(LANG_ICELANDIC, SUBLANG_ICELANDIC_ICELAND), LN("is_IS") },
	{ MAKELANGID(LANG_ICELANDIC, SUBLANG_NEUTRAL), LN("is") },
	{ MAKELANGID(LANG_IGBO, SUBLANG_IGBO_NIGERIA), LN("ig_NG") },
	{ MAKELANGID(LANG_IGBO, SUBLANG_NEUTRAL), LN("ig") },
	{ MAKELANGID(LANG_INDONESIAN, SUBLANG_INDONESIAN_INDONESIA), LN("id_ID") },
	{ MAKELANGID(LANG_INDONESIAN, SUBLANG_NEUTRAL), LN("id") },
	{ MAKELANGID(LANG_INUKTITUT, SUBLANG_INUKTITUT_CANADA), LN("iu_CA") },
	{ MAKELANGID(LANG_INUKTITUT, SUBLANG_INUKTITUT_CANADA_LATIN), LN("iu_CA") },
	{ MAKELANGID(LANG_INUKTITUT, 0x1e), LN("iu") },
	{ MAKELANGID(LANG_INUKTITUT, 0x1f), LN("iu") },
	{ MAKELANGID(LANG_INUKTITUT, SUBLANG_NEUTRAL), LN("iu") },
	{ MAKELANGID(LANG_IRISH, SUBLANG_GAELIC), LN("gd_DB") },
	{ MAKELANGID(LANG_IRISH, SUBLANG_IRISH_IRELAND), LN("ga_IE") },
	{ MAKELANGID(LANG_IRISH, SUBLANG_NEUTRAL), LN("ga") },
	{ MAKELANGID(LANG_ITALIAN, SUBLANG_ITALIAN), LN("it_IT") },
	{ MAKELANGID(LANG_ITALIAN, SUBLANG_ITALIAN_SWISS), LN("it_CH") },
	{ MAKELANGID(LANG_ITALIAN, SUBLANG_NEUTRAL), LN("it") },
	{ MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN), LN("ja_JP") },
	{ MAKELANGID(LANG_JAPANESE, SUBLANG_NEUTRAL), LN("ja") },
	{ MAKELANGID(LANG_KANURI, SUBLANG_KANURI_NIGERIA), LN("kr_NG") },
	{ MAKELANGID(LANG_KANURI, SUBLANG_NEUTRAL), LN("kr") },
	{ MAKELANGID(LANG_KANNADA, SUBLANG_KANNADA_INDIA), LN("kn_IN") },
	{ MAKELANGID(LANG_KANNADA, SUBLANG_NEUTRAL), LN("kn") },
	{ MAKELANGID(LANG_KASHMIRI, SUBLANG_KASHMIRI_PAKISTAN), LN("ks_PK") },
	{ MAKELANGID(LANG_KASHMIRI, SUBLANG_KASHMIRI_SASIA), LN("ks_IN") },
	{ MAKELANGID(LANG_KASHMIRI, SUBLANG_NEUTRAL), LN("ks") },
	{ MAKELANGID(LANG_KAZAK, SUBLANG_KAZAK_KAZAKHSTAN), LN("kk_KZ") },
	{ MAKELANGID(LANG_KAZAK, SUBLANG_NEUTRAL), LN("kk") },
	{ MAKELANGID(LANG_KHMER, SUBLANG_KHMER_CAMBODIA), LN("km_KH") },
	{ MAKELANGID(LANG_KHMER, SUBLANG_NEUTRAL), LN("km") },
	{ MAKELANGID(LANG_KICHE, SUBLANG_KICHE_GUATEMALA), LN("quc_GT") },
	{ MAKELANGID(LANG_KICHE, SUBLANG_NEUTRAL), LN("quc") },
	{ MAKELANGID(LANG_KINYARWANDA, SUBLANG_KINYARWANDA_RWANDA), LN("rw_RW") },
	{ MAKELANGID(LANG_KINYARWANDA, SUBLANG_NEUTRAL), LN("rw") },
	{ MAKELANGID(LANG_KONKANI, SUBLANG_KONKANI_INDIA), LN("kok_IN") },
	{ MAKELANGID(LANG_KONKANI, SUBLANG_NEUTRAL), LN("kok") },
	{ MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN), LN("ko_KR") },
	{ MAKELANGID(LANG_KOREAN, SUBLANG_NEUTRAL), LN("ko") },
	{ MAKELANGID(LANG_CENTRAL_KURDISH, SUBLANG_CENTRAL_KURDISH_IRAQ), LN("ku_IQ") },
	{ MAKELANGID(LANG_CENTRAL_KURDISH, SUBLANG_NEUTRAL), LN("ku") },
	{ MAKELANGID(LANG_KYRGYZ, SUBLANG_KYRGYZ_KYRGYZSTAN), LN("ky_KG") },
	{ MAKELANGID(LANG_KYRGYZ, SUBLANG_NEUTRAL), LN("ky") },
	{ MAKELANGID(LANG_LAO, SUBLANG_LAO_LAO), LN("lo_LA") },
	{ MAKELANGID(LANG_LAO, SUBLANG_NEUTRAL), LN("lo") },
	{ MAKELANGID(LANG_LATIN, SUBLANG_LATIN_VATICAN), LN("la_VA") },
	{ MAKELANGID(LANG_LATIN, SUBLANG_NEUTRAL), LN("la") },
	{ MAKELANGID(LANG_LATVIAN, SUBLANG_LATVIAN_LATVIA), LN("lv_LV") },
	{ MAKELANGID(LANG_LATVIAN, SUBLANG_NEUTRAL), LN("lv") },
	{ MAKELANGID(LANG_LITHUANIAN, SUBLANG_LITHUANIAN), LN("lt_LT") },
	{ MAKELANGID(LANG_LITHUANIAN, SUBLANG_LITHUANIAN_CLASSIC), LN("lt_LT") },
	{ MAKELANGID(LANG_LITHUANIAN, SUBLANG_NEUTRAL), LN("lt") },
	{ MAKELANGID(LANG_LUXEMBOURGISH, SUBLANG_LUXEMBOURGISH_LUXEMBOURG), LN("lb_LU") },
	{ MAKELANGID(LANG_LUXEMBOURGISH, SUBLANG_NEUTRAL), LN("lb") },
	{ MAKELANGID(LANG_MACEDONIAN, SUBLANG_MACEDONIAN_MACEDONIA), LN("mk_MK") },
	{ MAKELANGID(LANG_MACEDONIAN, SUBLANG_NEUTRAL), LN("mk") },
	{ MAKELANGID(LANG_MALAGASY, SUBLANG_MALAGASY_MADAGASCAR), LN("mg_MG") },
	{ MAKELANGID(LANG_MALAGASY, SUBLANG_NEUTRAL), LN("mg") },
	{ MAKELANGID(LANG_MALAY, SUBLANG_MALAY_MALAYSIA), LN("ms_MY") },
	{ MAKELANGID(LANG_MALAY, SUBLANG_MALAY_BRUNEI_DARUSSALAM), LN("ms_BN") },
	{ MAKELANGID(LANG_MALAY, SUBLANG_NEUTRAL), LN("ms") },
	{ MAKELANGID(LANG_MALAYALAM, SUBLANG_MALAYALAM_INDIA), LN("ml_IN") },
	{ MAKELANGID(LANG_MALAYALAM, SUBLANG_NEUTRAL), LN("ml") },
	{ MAKELANGID(LANG_MALTESE, SUBLANG_MALTESE_MALTA), LN("mt_MT") },
	{ MAKELANGID(LANG_MALTESE, SUBLANG_NEUTRAL), LN("mt") },
	{ MAKELANGID(LANG_MANIPURI, SUBLANG_MANIPURI_MANIPUR), LN("mni_IN") },
	{ MAKELANGID(LANG_MANIPURI, SUBLANG_NEUTRAL), LN("mni") },
	{ MAKELANGID(LANG_MAORI, SUBLANG_MAORI_NEW_ZEALAND), LN("mi_NZ") },
	{ MAKELANGID(LANG_MAORI, SUBLANG_NEUTRAL), LN("mi") },
	{ MAKELANGID(LANG_MAPUDUNGUN, SUBLANG_MAPUDUNGUN_CHILE), LN("arn_CL") },
	{ MAKELANGID(LANG_MAPUDUNGUN, SUBLANG_NEUTRAL), LN("arn") },
	{ MAKELANGID(LANG_MARATHI, SUBLANG_MARATHI_INDIA), LN("mr_IN") },
	{ MAKELANGID(LANG_MARATHI, SUBLANG_NEUTRAL), LN("mr") },
	{ MAKELANGID(LANG_MOHAWK, SUBLANG_MOHAWK_MOHAWK), LN("moh_CA") },
	{ MAKELANGID(LANG_MOHAWK, SUBLANG_NEUTRAL), LN("moh") },
	{ MAKELANGID(LANG_MONGOLIAN, SUBLANG_MONGOLIAN_CYRILLIC_MONGOLIA), LN("mn_MN") },
	{ MAKELANGID(LANG_MONGOLIAN, SUBLANG_MONGOLIAN_PRC), LN("mn_CN") },
	{ MAKELANGID(LANG_MONGOLIAN, 0x1e), LN("mn_MN") },
	{ MAKELANGID(LANG_MONGOLIAN, 0x1f), LN("mn_CN") },
	{ MAKELANGID(LANG_MONGOLIAN, SUBLANG_NEUTRAL), LN("mn") },
	{ MAKELANGID(LANG_NEPALI, SUBLANG_NEPALI_NEPAL), LN("ne_NP") },
	{ MAKELANGID(LANG_NEPALI, SUBLANG_NEPALI_INDIA), LN("ne_IN") },
	{ MAKELANGID(LANG_NEPALI, SUBLANG_NEUTRAL), LN("ne") },
	{ MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL), LN("nb_NO") },
	{ MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_NYNORSK), LN("nn_NO") },
	{ MAKELANGID(LANG_NORWEGIAN, 0x1e), LN("nn") },
	{ MAKELANGID(LANG_NORWEGIAN, 0x1f), LN("nb") },
	{ MAKELANGID(LANG_NORWEGIAN, SUBLANG_NEUTRAL), LN("nb") },
	{ MAKELANGID(LANG_OCCITAN, SUBLANG_OCCITAN_FRANCE), LN("oc_FR") },
	{ MAKELANGID(LANG_OCCITAN, SUBLANG_NEUTRAL), LN("oc") },
	{ MAKELANGID(LANG_ORIYA, SUBLANG_ORIYA_INDIA), LN("or_IN") },
	{ MAKELANGID(LANG_ODIA, SUBLANG_NEUTRAL), LN("or") },
	{ MAKELANGID(LANG_OROMO, SUBLANG_OROMO_ETHIOPIA), LN("om_ET") },
	{ MAKELANGID(LANG_OROMO, SUBLANG_NEUTRAL), LN("om") },
	{ MAKELANGID(LANG_PAPIAMENTO, SUBLANG_PAPIAMENTO_NETHERLANDS_ANTILLES), LN("pap_AN") },
	{ MAKELANGID(LANG_PAPIAMENTO, SUBLANG_NEUTRAL), LN("pap") },
	{ MAKELANGID(LANG_PASHTO, SUBLANG_PASHTO_AFGHANISTAN), LN("ps_AF") },
	{ MAKELANGID(LANG_PASHTO, SUBLANG_NEUTRAL), LN("ps") },
	/* Note: LANG_PERSIAN == LANG_FARSI */
	{ MAKELANGID(LANG_PERSIAN, SUBLANG_PERSIAN_IRAN), LN("fa_IR") },
	{ MAKELANGID(LANG_PERSIAN, SUBLANG_NEUTRAL), LN("fa") },
	{ MAKELANGID(LANG_POLISH, SUBLANG_POLISH_POLAND), LN("pl_PL") },
	{ MAKELANGID(LANG_POLISH, SUBLANG_NEUTRAL), LN("pl") },
	{ MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE), LN("pt_PT") },
	{ MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN), LN("pt_BR") },
	{ MAKELANGID(LANG_PORTUGUESE, SUBLANG_NEUTRAL), LN("pt") },
	{ MAKELANGID(LANG_PUNJABI, SUBLANG_PUNJABI_INDIA), LN("pa_IN") },
	{ MAKELANGID(LANG_PUNJABI, SUBLANG_PUNJABI_PAKISTAN), LN("pa_PK") },
	{ MAKELANGID(LANG_PUNJABI, SUBLANG_NEUTRAL), LN("pa") },
	{ MAKELANGID(LANG_QUECHUA, SUBLANG_QUECHUA_BOLIVIA), LN("qu_BO") },
	{ MAKELANGID(LANG_QUECHUA, SUBLANG_QUECHUA_ECUADOR), LN("qu_EC") },
	{ MAKELANGID(LANG_QUECHUA, SUBLANG_QUECHUA_PERU), LN("qu_PE") },
	{ MAKELANGID(LANG_QUECHUA, SUBLANG_NEUTRAL), LN("qu") },
	{ MAKELANGID(LANG_ROMANIAN,  SUBLANG_ROMANIAN_ROMANIA), LN("ro_RO") },
	{ MAKELANGID(LANG_ROMANIAN,  SUBLANG_ROMANIAN_MOLDAVIA), LN("ro_MD") },
	{ MAKELANGID(LANG_ROMANIAN, SUBLANG_NEUTRAL), LN("ro") },
	{ MAKELANGID(LANG_ROMANSH, SUBLANG_ROMANSH_SWITZERLAND), LN("rm_CH") },
	{ MAKELANGID(LANG_ROMANSH, SUBLANG_NEUTRAL), LN("rm") },
	{ MAKELANGID(LANG_RUSSIAN, SUBLANG_RUSSIAN_RUSSIA), LN("ru_RU") },
	{ MAKELANGID(LANG_RUSSIAN, SUBLANG_RUSSIAN_MOLDAVIA), LN("ru_MD") },
	{ MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL), LN("ru") },
	{ MAKELANGID(LANG_SAMI, SUBLANG_SAMI_NORTHERN_NORWAY), LN("se_NO") },
 	{ MAKELANGID(LANG_SAMI, SUBLANG_SAMI_NORTHERN_SWEDEN), LN("se_SE") },
 	{ MAKELANGID(LANG_SAMI, SUBLANG_SAMI_NORTHERN_FINLAND), LN("se_FI") },
 	{ MAKELANGID(LANG_SAMI, SUBLANG_SAMI_LULE_NORWAY), LN("smj_NO") },
 	{ MAKELANGID(LANG_SAMI, SUBLANG_SAMI_LULE_SWEDEN), LN("smj_SE") },
 	{ MAKELANGID(LANG_SAMI, SUBLANG_SAMI_SOUTHERN_NORWAY), LN("sma_NO") },
 	{ MAKELANGID(LANG_SAMI, SUBLANG_SAMI_SOUTHERN_SWEDEN), LN("sma_SE") },
 	{ MAKELANGID(LANG_SAMI, SUBLANG_SAMI_SKOLT_FINLAND), LN("sms_FI") },
 	{ MAKELANGID(LANG_SAMI, SUBLANG_SAMI_INARI_FINLAND), LN("smn_FI") },
 	{ MAKELANGID(LANG_SAMI, 0x1c), LN("smn") },
 	{ MAKELANGID(LANG_SAMI, 0x1d), LN("sms") },
 	{ MAKELANGID(LANG_SAMI, 0x1e), LN("sma") },
 	{ MAKELANGID(LANG_SAMI, 0x1f), LN("smj") },
	{ MAKELANGID(LANG_SAMI, SUBLANG_NEUTRAL), LN("se") },
	{ MAKELANGID(LANG_SANSKRIT, SUBLANG_SANSKRIT_INDIA), LN("sa_IN") },
	{ MAKELANGID(LANG_SANSKRIT, SUBLANG_NEUTRAL), LN("sa") },
	{ MAKELANGID(LANG_SCOTTISH_GAELIC, SUBLANG_SCOTTISH_GAELIC), LN("gd_GB") },
	{ MAKELANGID(LANG_SCOTTISH_GAELIC, SUBLANG_NEUTRAL), LN("gd") },
 	/* Note: LANG_BOSNIAN == LANG_SERBIAN == LANG_CROATIAN */
	{ MAKELANGID(LANG_BOSNIAN, SUBLANG_BOSNIAN_BOSNIA_HERZEGOVINA_LATIN), LN("bs_BA") },
	{ MAKELANGID(LANG_BOSNIAN, SUBLANG_BOSNIAN_BOSNIA_HERZEGOVINA_CYRILLIC), LN("bs_BA") },
	{ MAKELANGID(LANG_CROATIAN, SUBLANG_CROATIAN_CROATIA), LN("hr_HR") },
	{ MAKELANGID(LANG_CROATIAN, SUBLANG_CROATIAN_BOSNIA_HERZEGOVINA_LATIN), LN("hr_BA") },
	{ MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_CROATIA), LN("sr_RS") },
	{ MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_LATIN), LN("sr_RS") },
	{ MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_CYRILLIC), LN("sr_RS") },
	{ MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_LATIN), LN("sr_BA") },
	{ MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_CYRILLIC), LN("sr_BA") },
	{ MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_SERBIA_LATIN), LN("sr_RS") },
	{ MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_SERBIA_CYRILLIC), LN("sr_RS") },
	{ MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_MONTENEGRO_LATIN), LN("sr_ME") },
	{ MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_MONTENEGRO_CYRILLIC), LN("sr_ME") },
	{ MAKELANGID(LANG_BOSNIAN, 0x19), LN("bs") },
	{ MAKELANGID(LANG_BOSNIAN, 0x1a), LN("bs") },
	{ MAKELANGID(LANG_SERBIAN, 0x1b), LN("sr") },
	{ MAKELANGID(LANG_SERBIAN, 0x1c), LN("sr") },
	{ MAKELANGID(LANG_BOSNIAN, 0x1e), LN("bs") },
	{ MAKELANGID(LANG_SERBIAN, 0x1f), LN("sr_RS") },
	{ MAKELANGID(LANG_SERBIAN, SUBLANG_NEUTRAL), LN("sr") },
	{ MAKELANGID(LANG_SINDHI, SUBLANG_SINDHI_INDIA), LN("sd_IN") },
 	{ MAKELANGID(LANG_SINDHI, SUBLANG_SINDHI_PAKISTAN), LN("sd_PK") },
 	{ MAKELANGID(LANG_SINDHI, SUBLANG_SINDHI_AFGHANISTAN), LN("sd_AF") },
	{ MAKELANGID(LANG_SINDHI, SUBLANG_NEUTRAL), LN("sd") },
	{ MAKELANGID(LANG_SINHALESE, SUBLANG_SINHALESE_SRI_LANKA), LN("si_LK") },
	{ MAKELANGID(LANG_SINHALESE, SUBLANG_NEUTRAL), LN("si") },
	{ MAKELANGID(LANG_SLOVAK, SUBLANG_SLOVAK_SLOVAKIA), LN("sk_SK") },
	{ MAKELANGID(LANG_SLOVAK, SUBLANG_NEUTRAL), LN("sk") },
	{ MAKELANGID(LANG_SLOVENIAN, SUBLANG_SLOVENIAN_SLOVENIA), LN("sl_SI") },
	{ MAKELANGID(LANG_SLOVENIAN, SUBLANG_NEUTRAL), LN("sl") },
	{ MAKELANGID(LANG_SOMALI, SUBLANG_SOMALI_SOMALIA), LN("so_SO") },
	{ MAKELANGID(LANG_SOMALI, SUBLANG_NEUTRAL), LN("so") },
	{ MAKELANGID(LANG_SOTHO, SUBLANG_SOTHO_NORTHERN_SOUTH_AFRICA), LN("nso_ZA") },
	{ MAKELANGID(LANG_SOTHO, SUBLANG_NEUTRAL), LN("nso") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH), LN("es_ES") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_MEXICAN), LN("es_MX") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_MODERN), LN("es_ES") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_GUATEMALA), LN("es_GT") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_COSTA_RICA), LN("es_CR") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_PANAMA), LN("es_PA") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_DOMINICAN_REPUBLIC), LN("es_DO") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_VENEZUELA), LN("es_VE") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_COLOMBIA), LN("es_CO") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_PERU), LN("es_PE") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_ARGENTINA), LN("es_AR") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_ECUADOR), LN("es_EC") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_CHILE), LN("es_CL") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_URUGUAY), LN("es_UY") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_PARAGUAY), LN("es_PY") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_BOLIVIA), LN("es_BO") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_EL_SALVADOR), LN("es_SV") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_HONDURAS), LN("es_HN") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_NICARAGUA), LN("es_NI") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_PUERTO_RICO), LN("es_PR") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_US), LN("es_US") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_LATIN_AMERICA), LN("es_LA") },
	{ MAKELANGID(LANG_SPANISH, SUBLANG_NEUTRAL), LN("es") },
	{ MAKELANGID(LANG_SUTU, SUBLANG_SUTU_SOUTH_AFRICA), LN("sx_ZA") },
	{ MAKELANGID(LANG_SUTU, SUBLANG_NEUTRAL), LN("sx") },
	{ MAKELANGID(LANG_SWAHILI, SUBLANG_SWAHILI_KENYA), LN("sw_KE") },
	{ MAKELANGID(LANG_SWAHILI, SUBLANG_NEUTRAL), LN("sw") },
	{ MAKELANGID(LANG_SWEDISH, SUBLANG_SWEDISH), LN("sv_SE") },
	{ MAKELANGID(LANG_SWEDISH, SUBLANG_SWEDISH_FINLAND), LN("sv_FI") },
	{ MAKELANGID(LANG_SWEDISH, SUBLANG_NEUTRAL), LN("sv") },
	{ MAKELANGID(LANG_SYRIAC, SUBLANG_SYRIAC_SYRIA), LN("syr_SY") },
	{ MAKELANGID(LANG_SYRIAC, SUBLANG_NEUTRAL), LN("syr") },
	{ MAKELANGID(LANG_TAJIK, SUBLANG_TAJIK_TAJIKISTAN), LN("tg_TJ") },
	{ MAKELANGID(LANG_TAJIK, 0x1f), LN("tg") },
	{ MAKELANGID(LANG_TAJIK, SUBLANG_NEUTRAL), LN("tg") },
	{ MAKELANGID(LANG_TAMAZIGHT, SUBLANG_TAMAZIGHT_MOROCCO), LN("tzm_MA") },
	{ MAKELANGID(LANG_TAMAZIGHT, SUBLANG_TAMAZIGHT_MOROCCO_TIFINAGH), LN("tzm_MA") },
	{ MAKELANGID(LANG_TAMAZIGHT, SUBLANG_TAMAZIGHT_ALGERIA_LATIN), LN("tzm_DZ") },
	{ MAKELANGID(LANG_TAMAZIGHT, 0x1f), LN("ber") },
	{ MAKELANGID(LANG_TAMAZIGHT, SUBLANG_NEUTRAL), LN("tzm") },
	{ MAKELANGID(LANG_TAMIL, SUBLANG_TAMIL_INDIA), LN("ta_IN") },
	{ MAKELANGID(LANG_TAMIL, SUBLANG_TAMIL_SRI_LANKA), LN("ta_LK") },
	{ MAKELANGID(LANG_TAMIL, SUBLANG_NEUTRAL), LN("ta") },
	{ MAKELANGID(LANG_TATAR, SUBLANG_TATAR_RUSSIA), LN("tt_RU") },
	{ MAKELANGID(LANG_TATAR, SUBLANG_NEUTRAL), LN("tt") },
	{ MAKELANGID(LANG_TELUGU, SUBLANG_TELUGU_INDIA), LN("te_IN") },
	{ MAKELANGID(LANG_TELUGU, SUBLANG_NEUTRAL), LN("te") },
	{ MAKELANGID(LANG_THAI, SUBLANG_THAI_THAILAND), LN("th_TH") },
	{ MAKELANGID(LANG_THAI, SUBLANG_NEUTRAL), LN("th") },
	{ MAKELANGID(LANG_TIBETAN, SUBLANG_TIBETAN_PRC), LN("bo_CN") },
	{ MAKELANGID(LANG_TIBETAN, SUBLANG_TIBETAN_BHUTAN), LN("bo_BT") },
	{ MAKELANGID(LANG_TIBETAN, SUBLANG_NEUTRAL), LN("bo") },
	{ MAKELANGID(LANG_TIGRIGNA, SUBLANG_TIGRIGNA_ETHIOPIA), LN("ti_ET") },
	{ MAKELANGID(LANG_TIGRIGNA, SUBLANG_TIGRIGNA_ERITREA), LN("ti_ER") },
	{ MAKELANGID(LANG_TIGRIGNA, SUBLANG_NEUTRAL), LN("ti") },
	{ MAKELANGID(LANG_TSONGA, SUBLANG_TSONGA_SOUTH_AFRICA), LN("ts_ZA") },
	{ MAKELANGID(LANG_TSONGA, SUBLANG_NEUTRAL), LN("ts") },
	{ MAKELANGID(LANG_TSWANA, SUBLANG_TSWANA_SOUTH_AFRICA), LN("tn_ZA") },
	{ MAKELANGID(LANG_TSWANA, SUBLANG_TSWANA_BOTSWANA), LN("tn_BW") },
	{ MAKELANGID(LANG_TSWANA, SUBLANG_NEUTRAL), LN("tn") },
	{ MAKELANGID(LANG_TURKISH, SUBLANG_TURKISH_TURKEY), LN("tr_TR") },
	{ MAKELANGID(LANG_TURKISH, SUBLANG_NEUTRAL), LN("tr") },
	{ MAKELANGID(LANG_TURKMEN, SUBLANG_TURKMEN_TURKMENISTAN), LN("tk_TM") },
	{ MAKELANGID(LANG_TURKMEN, SUBLANG_NEUTRAL), LN("tk") },
	{ MAKELANGID(LANG_UIGHUR, SUBLANG_UIGHUR_PRC), LN("ug_CN") },
	{ MAKELANGID(LANG_UIGHUR, SUBLANG_NEUTRAL), LN("ug") },
	{ MAKELANGID(LANG_UKRAINIAN, SUBLANG_UKRAINIAN_UKRAINE), LN("uk_UA") },
	{ MAKELANGID(LANG_UKRAINIAN, SUBLANG_NEUTRAL), LN("uk") },
	{ MAKELANGID(LANG_SORBIAN, SUBLANG_UPPER_SORBIAN_GERMANY), LN("hsb_DE") },
	{ MAKELANGID(LANG_SORBIAN, SUBLANG_LOWER_SORBIAN_GERMANY), LN("dsb_DE") },
	{ MAKELANGID(LANG_SORBIAN, 0x1e), LN("hsb") },
	{ MAKELANGID(LANG_SORBIAN, 0x1f), LN("dsb") },
	{ MAKELANGID(LANG_SORBIAN, SUBLANG_NEUTRAL), LN("hsb") },
	{ MAKELANGID(LANG_URDU, SUBLANG_URDU_PAKISTAN), LN("ur_PK") },
	{ MAKELANGID(LANG_URDU, SUBLANG_URDU_INDIA), LN("ur_IN") },
	{ MAKELANGID(LANG_URDU, SUBLANG_NEUTRAL), LN("ur") },
	{ MAKELANGID(LANG_UZBEK, SUBLANG_UZBEK_LATIN), LN("uz_UZ") },
	{ MAKELANGID(LANG_UZBEK, SUBLANG_UZBEK_CYRILLIC), LN("uz_UZ") },
	{ MAKELANGID(LANG_UZBEK, 0x1e), LN("uz") },
	{ MAKELANGID(LANG_UZBEK, 0x1f), LN("uz") },
	{ MAKELANGID(LANG_UZBEK, SUBLANG_NEUTRAL), LN("uz") },
	{ MAKELANGID(LANG_VENDA, SUBLANG_VENDA_SOUTH_AFRICA), LN("ve_ZA") },
	{ MAKELANGID(LANG_VENDA, SUBLANG_NEUTRAL), LN("ve") },
	{ MAKELANGID(LANG_VIETNAMESE, SUBLANG_VIETNAMESE_VIETNAM), LN("vi_VN") },
	{ MAKELANGID(LANG_VIETNAMESE, SUBLANG_NEUTRAL), LN("vi") },
	{ MAKELANGID(LANG_WELSH, SUBLANG_WELSH_UNITED_KINGDOM), LN("cy_GB") },
	{ MAKELANGID(LANG_WELSH, SUBLANG_NEUTRAL), LN("cy") },
	{ MAKELANGID(LANG_WOLOF, SUBLANG_WOLOF_SENEGAL), LN("wo_SN") },
	{ MAKELANGID(LANG_WOLOF, SUBLANG_NEUTRAL), LN("wo") },
	{ MAKELANGID(LANG_XHOSA, SUBLANG_XHOSA_SOUTH_AFRICA), LN("xh_ZA") },
	{ MAKELANGID(LANG_XHOSA, SUBLANG_NEUTRAL), LN("xh") },
	{ MAKELANGID(LANG_YAKUT, SUBLANG_YAKUT_RUSSIA), LN("sah_RU") },
	{ MAKELANGID(LANG_YAKUT, SUBLANG_NEUTRAL), LN("sah") },
	{ MAKELANGID(LANG_YI, SUBLANG_YI_PRC), LN("ii_CN") },
	{ MAKELANGID(LANG_YI, SUBLANG_NEUTRAL), LN("ii") },
	{ MAKELANGID(LANG_YIDDISH, SUBLANG_YIDDISH_ISRAEL), LN("yi_IL") },
	{ MAKELANGID(LANG_YIDDISH, SUBLANG_NEUTRAL), LN("yi") },
	{ MAKELANGID(LANG_YORUBA, SUBLANG_YORUBA_NIGERIA), LN("yo_NG") },
	{ MAKELANGID(LANG_YORUBA, SUBLANG_NEUTRAL), LN("yo") },
	{ MAKELANGID(LANG_ZULU, SUBLANG_ZULU_SOUTH_AFRICA), LN("zu_ZA") },
	{ MAKELANGID(LANG_ZULU, SUBLANG_NEUTRAL), LN("zu") },

	/* wine only */
	{ MAKELANGID(LANG_WALLOON, SUBLANG_WALLOON_BELGIUM), LN("wa_BE") },
	{ MAKELANGID(LANG_WALLOON, SUBLANG_NEUTRAL), LN("wa") },
	{ MAKELANGID(LANG_CORNISH, SUBLANG_CORNISH_UK), LN("kw_GB") },
	{ MAKELANGID(LANG_CORNISH, SUBLANG_NEUTRAL), LN("kw") },
	{ MAKELANGID(LANG_ESPERANTO, SUBLANG_DEFAULT), LN("eo") },
	{ MAKELANGID(LANG_ESPERANTO, SUBLANG_NEUTRAL), LN("eo") },
	{ MAKELANGID(LANG_GAELIC, SUBLANG_GAELIC), LN("ga_IE") },
	{ MAKELANGID(LANG_GAELIC, SUBLANG_GAELIC_SCOTTISH), LN("gd_GB") },
	{ MAKELANGID(LANG_GAELIC, SUBLANG_GAELIC_MANX), LN("gv_GB") },
	{ MAKELANGID(LANG_GAELIC, SUBLANG_NEUTRAL), LN("ga") },

	/* other languages not covered by windows */
	{ MAKELANGID(LANG_AFAR, SUBLANG_NEUTRAL), LN("aa") },
	{ MAKELANGID(LANG_ABKHAZIAN, SUBLANG_NEUTRAL), LN("ab") },
	{ MAKELANGID(LANG_AVESTAN, SUBLANG_NEUTRAL), LN("ae") },
	{ MAKELANGID(LANG_AKAN, SUBLANG_NEUTRAL), LN("ak") },
	{ MAKELANGID(LANG_ARAGONESE, SUBLANG_NEUTRAL), LN("an") },
	{ MAKELANGID(LANG_AVARIC, SUBLANG_NEUTRAL), LN("av") },
	{ MAKELANGID(LANG_AYMARA, SUBLANG_NEUTRAL), LN("ay") },
	{ MAKELANGID(LANG_BIHARI, SUBLANG_NEUTRAL), LN("bh") },
	{ MAKELANGID(LANG_BISLAMA, SUBLANG_NEUTRAL), LN("bi") },
	{ MAKELANGID(LANG_BAMBARA, SUBLANG_NEUTRAL), LN("bm") },
	{ MAKELANGID(LANG_CHECHEN, SUBLANG_NEUTRAL), LN("ce") },
	{ MAKELANGID(LANG_CHAMORRO, SUBLANG_NEUTRAL), LN("ch") },
 	{ MAKELANGID(LANG_CREE, SUBLANG_NEUTRAL), LN("cr") },
	{ MAKELANGID(LANG_CHURCH_SLAVONIC, SUBLANG_NEUTRAL), LN("cu") },
	{ MAKELANGID(LANG_CHUVASH, SUBLANG_NEUTRAL), LN("cv") },
	{ MAKELANGID(LANG_EWE, SUBLANG_NEUTRAL), LN("ee") },
	{ MAKELANGID(LANG_FIJIAN, SUBLANG_NEUTRAL), LN("fj") },
	{ MAKELANGID(LANG_HIRI_MOTU, SUBLANG_NEUTRAL), LN("ho") },
	{ MAKELANGID(LANG_HAITIAN, SUBLANG_NEUTRAL), LN("ht") },
	{ MAKELANGID(LANG_HERERO, SUBLANG_NEUTRAL), LN("hz") },
	{ MAKELANGID(LANG_INTERLINGUA, SUBLANG_NEUTRAL), LN("ia") },
	{ MAKELANGID(LANG_INTERLINGUE, SUBLANG_NEUTRAL), LN("ie") },
	{ MAKELANGID(LANG_INUPIAQ, SUBLANG_NEUTRAL), LN("ik") },
	{ MAKELANGID(LANG_IDO, SUBLANG_NEUTRAL), LN("io") },
	{ MAKELANGID(LANG_JAVANESE, SUBLANG_NEUTRAL), LN("jv") },
	{ MAKELANGID(LANG_KONGO, SUBLANG_NEUTRAL), LN("kg") },
	{ MAKELANGID(LANG_GIKUYU, SUBLANG_NEUTRAL), LN("ki") },
	{ MAKELANGID(LANG_KWANYAMA, SUBLANG_NEUTRAL), LN("kj") },
	{ MAKELANGID(LANG_KOMI, SUBLANG_NEUTRAL), LN("kv") },
	{ MAKELANGID(LANG_GANDA, SUBLANG_NEUTRAL), LN("lg") },
	{ MAKELANGID(LANG_LIMBURGISH, SUBLANG_NEUTRAL), LN("li") },
	{ MAKELANGID(LANG_LINGALA, SUBLANG_NEUTRAL), LN("ln") },
	{ MAKELANGID(LANG_LUBA_KATANGA, SUBLANG_NEUTRAL), LN("lu") },
	{ MAKELANGID(LANG_MARSHALLESE, SUBLANG_NEUTRAL), LN("mh") },
	{ MAKELANGID(LANG_MOLDOVAN, SUBLANG_NEUTRAL), LN("mo") },
	{ MAKELANGID(LANG_NAURUAN, SUBLANG_NEUTRAL), LN("na") },
	{ MAKELANGID(LANG_NDEBELE_NORTH, SUBLANG_NEUTRAL), LN("nd") },
	{ MAKELANGID(LANG_NDEBELE_SOUTH, SUBLANG_NEUTRAL), LN("nr") },
	{ MAKELANGID(LANG_NDONGA, SUBLANG_NEUTRAL), LN("ng") },
	{ MAKELANGID(LANG_NAVAJO, SUBLANG_NEUTRAL), LN("nv") },
	{ MAKELANGID(LANG_CHEWA, SUBLANG_NEUTRAL), LN("ny") },
	{ MAKELANGID(LANG_OJIBWA, SUBLANG_NEUTRAL), LN("oj") },
	{ MAKELANGID(LANG_OSSETIC, SUBLANG_NEUTRAL), LN("os") },
	{ MAKELANGID(LANG_PALI, SUBLANG_NEUTRAL), LN("pI") },
	{ MAKELANGID(LANG_KIRUNDI, SUBLANG_NEUTRAL), LN("rn") },
	{ MAKELANGID(LANG_SARDINIAN, SUBLANG_NEUTRAL), LN("sc") },
	{ MAKELANGID(LANG_SANGO, SUBLANG_NEUTRAL), LN("sg") },
	{ MAKELANGID(LANG_SAMOAN, SUBLANG_NEUTRAL), LN("sm") },
	{ MAKELANGID(LANG_SHONA, SUBLANG_NEUTRAL), LN("sn") },
	{ MAKELANGID(LANG_SWAZI, SUBLANG_NEUTRAL), LN("ss") },
	{ MAKELANGID(LANG_SUNDANESE, SUBLANG_NEUTRAL), LN("su") },
	{ MAKELANGID(LANG_TAGALOG, SUBLANG_NEUTRAL), LN("tl") },
	{ MAKELANGID(LANG_TONGAN, SUBLANG_NEUTRAL), LN("to") },
	{ MAKELANGID(LANG_TWI, SUBLANG_NEUTRAL), LN("tw") },
	{ MAKELANGID(LANG_TAHITIAN, SUBLANG_NEUTRAL), LN("ty") },
	{ MAKELANGID(LANG_VOLAPUK, SUBLANG_NEUTRAL), LN("vo") },
	{ MAKELANGID(LANG_ZHUANG, SUBLANG_NEUTRAL), LN("za") }
#undef LN
};

static const char *gl_locale_name_from_win32_LANGID (LANGID langid)
{
	int i;
  
  /* Activate the new code only when the GETTEXT_MUI environment variable is
     set, for the time being, since the new code is not well tested.  */
  if (getenv ("GETTEXT_MUI") != NULL)
    {
      static char namebuf[256];

      /* Query the system's notion of locale name.
         On Windows95/98/ME, GetLocaleInfoA returns some incorrect results.
         But we don't need to support systems that are so old.  */
      if (GetLocaleInfoA (MAKELCID (langid, SORT_DEFAULT), LOCALE_SNAME,
                          namebuf, sizeof (namebuf) - 1))
        {
          /* Convert it to a Unix locale name.  */
          gl_locale_name_canonicalize (namebuf);
          return namebuf;
        }
    }
  /* Internet Explorer has an LCID to RFC3066 name mapping stored in
     HKEY_CLASSES_ROOT\Mime\Database\Rfc1766.  But we better don't use that
     since IE's i18n subsystem is known to be inconsistent with the native
     Windows base (e.g. they have different character conversion facilities
     that produce different results).  */
  /* Use our own table.  */

	for (i = 0; i < (int)(sizeof(sublang_table) / sizeof(sublang_table[0])); i++)
		if (sublang_table[i].id == langid)
			return sublang_table[i].po_name;
	langid = MAKELANGID(PRIMARYLANGID(langid), SUBLANG_NEUTRAL);
	for (i = 0; i < (int)(sizeof(sublang_table) / sizeof(sublang_table[0])); i++)
		if (sublang_table[i].id == langid)
			return sublang_table[i].po_name;
	return "C";
}

/*
 * opposite of gl_locale_name_from_win32_LANGID ()
 */
LANGID gl_locale_win32_langid_from_name(const char *name)
{
	int i;

	if (!name || !*name)
		return MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
	for (i = 0; i < (int)(sizeof(sublang_table) / sizeof(sublang_table[0])); i++)
		if (strncmp(name, sublang_table[i].po_name, sublang_table[i].namelen) == 0)
			return sublang_table[i].id;

	return MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
}


static const char *gl_locale_name_from_win32_LCID (LCID lcid)
{
  LANGID langid;

  /* Strip off the sorting rules, keep only the language part.  */
  langid = LANGIDFROMLCID (lcid);

  return gl_locale_name_from_win32_LANGID (langid);
}

#endif


#ifdef HAVE_USELOCALE /* glibc or Mac OS X */

/* Simple hash set of strings.  We don't want to drag in lots of hash table
   code here.  */

# define SIZE_BITS (sizeof (size_t) * CHAR_BIT)

/* A hash function for NUL-terminated char* strings using
   the method described by Bruno Haible.
   See http://www.haible.de/bruno/hashfunc.html.  */
static size_t
string_hash (const void *x)
{
  const char *s = (const char *) x;
  size_t h = 0;

  for (; *s; s++)
    h = *s + ((h << 9) | (h >> (SIZE_BITS - 9)));

  return h;
}

/* A hash table of fixed size.  Multiple threads can access it read-only
   simultaneously, but only one thread can insert into it at the same time.  */

/* A node in a hash bucket collision list.  */
struct hash_node
  {
    struct hash_node * volatile next;
    char contents[100]; /* has variable size */
  };

# define HASH_TABLE_SIZE 257
static struct hash_node * volatile struniq_hash_table[HASH_TABLE_SIZE]
  /* = { NULL, ..., NULL } */;

/* This lock protects the struniq_hash_table against multiple simultaneous
   insertions.  */
gl_lock_define_initialized(static, struniq_lock)

/* Store a copy of the given string in a string pool with indefinite extent.
   Return a pointer to this copy.  */
static const char *
struniq (const char *string)
{
  size_t hashcode = string_hash (string);
  size_t slot = hashcode % HASH_TABLE_SIZE;
  size_t size;
  struct hash_node *new_node;
  struct hash_node *p;
  for (p = struniq_hash_table[slot]; p != NULL; p = p->next)
    if (strcmp (p->contents, string) == 0)
      return p->contents;
  size = strlen (string) + 1;
  new_node =
    (struct hash_node *)
    malloc (offsetof (struct hash_node, contents[0]) + size);
  if (new_node == NULL)
    /* Out of memory.  Return a statically allocated string.  */
    return "C";
  memcpy (new_node->contents, string, size);
  /* Lock while inserting new_node.  */
  gl_lock_lock (struniq_lock);
  /* Check whether another thread already added the string while we were
     waiting on the lock.  */
  for (p = struniq_hash_table[slot]; p != NULL; p = p->next)
    if (strcmp (p->contents, string) == 0)
      {
        free (new_node);
        new_node = p;
        goto done;
      }
  /* Really insert new_node into the hash table.  Fill new_node entirely first,
     because other threads may be iterating over the linked list.  */
  new_node->next = struniq_hash_table[slot];
  struniq_hash_table[slot] = new_node;
 done:
  /* Unlock after new_node is inserted.  */
  gl_lock_unlock (struniq_lock);
  return new_node->contents;
}

#endif


#if defined(HAVE_USELOCALE)

/* Like gl_locale_name_thread, except that the result is not in storage of
   indefinite extent.  */
static const char *gl_locale_name_thread_unsafe (int category, const char *categoryname)
{
#if defined(HAVE_USELOCALE)
  {
    locale_t thread_locale = uselocale (NULL);
  (void) category;
  (void) categoryname;
    if (thread_locale != LC_GLOBAL_LOCALE)
      {
#  if defined(__GLIBC__) && __GLIBC__ >= 2 && !defined __UCLIBC__
        /* Work around an incorrect definition of the _NL_LOCALE_NAME macro in
           glibc < 2.12.
           See <http://sourceware.org/bugzilla/show_bug.cgi?id=10968>.  */
        const char *name =
          nl_langinfo (_NL_ITEM ((category), _NL_ITEM_INDEX (-1)));
        if (name[0] == '\0')
          /* Fallback code for glibc < 2.4, which did not implement
             nl_langinfo (_NL_LOCALE_NAME (category)).  */
          name = thread_locale->__names[category];
        return name;
#  elif defined __FreeBSD__ || (defined __APPLE__ && defined __MACH__)
        /* FreeBSD, Mac OS X */
        int mask;

        switch (category)
          {
          case LC_CTYPE:
            mask = LC_CTYPE_MASK;
            break;
          case LC_NUMERIC:
            mask = LC_NUMERIC_MASK;
            break;
          case LC_TIME:
            mask = LC_TIME_MASK;
            break;
          case LC_COLLATE:
            mask = LC_COLLATE_MASK;
            break;
          case LC_MONETARY:
            mask = LC_MONETARY_MASK;
            break;
          case LC_MESSAGES:
            mask = LC_MESSAGES_MASK;
            break;
          default: /* We shouldn't get here.  */
            return "";
          }
        return querylocale (mask, thread_locale);
#  endif
      }
  }
#else
  (void) category;
  (void) categoryname;
# endif
  return NULL;
}

#endif

const char *
gl_locale_name_thread (int category, const char *categoryname)
{
#ifdef HAVE_USELOCALE
  const char *name = gl_locale_name_thread_unsafe (category, categoryname);
  if (name != NULL)
    return struniq (name);
#else
  (void) category;
  (void) categoryname;
#endif
  return NULL;
}

/* XPG3 defines the result of 'setlocale (category, NULL)' as:
   "Directs 'setlocale()' to query 'category' and return the current
    setting of 'local'."
   However it does not specify the exact format.  Neither do SUSV2 and
   ISO C 99.  So we can use this feature only on selected systems (e.g.
   those using GNU C Library).  */
#if defined _LIBC || defined _MINT_SOURCE || ((defined __GLIBC__ && __GLIBC__ >= 2) && !defined __UCLIBC__)
# define HAVE_LOCALE_NULL
#endif

const char *
gl_locale_name_posix (int category, const char *categoryname)
{
  /* Use the POSIX methods of looking to 'LC_ALL', 'LC_xxx', and 'LANG'.
     On some systems this can be done by the 'setlocale' function itself.  */
#if defined HAVE_SETLOCALE && defined HAVE_LC_MESSAGES && defined HAVE_LOCALE_NULL
  (void) categoryname;
  return setlocale (category, NULL);
#else
  /* On other systems we ignore what setlocale reports and instead look at the
     environment variables directly.  This is necessary
       1. on systems which have a facility for customizing the default locale
          (Mac OS X, native Windows, Cygwin) and where the system's setlocale()
          function ignores this default locale (Mac OS X, Cygwin), in two cases:
          a. when the user missed to use the setlocale() override from libintl
             (for example by not including <libintl.h>),
          b. when setlocale supports only the "C" locale, such as on Cygwin
             1.5.x.  In this case even the override from libintl cannot help.
       2. on all systems where setlocale supports only the "C" locale.  */
  /* Strictly speaking, it is a POSIX violation to look at the environment
     variables regardless whether setlocale has been called or not.  POSIX
     says:
         "For C-language programs, the POSIX locale shall be the
          default locale when the setlocale() function is not called."
     But we assume that all programs that use internationalized APIs call
     setlocale (LC_ALL, "").  */
  return gl_locale_name_environ (category, categoryname);
#endif
}

const char *
gl_locale_name_environ (int category, const char *categoryname)
{
  const char *retval;

  (void) category;
  /* Setting of LC_ALL overrides all other.  */
  retval = getenv ("LC_ALL");
  if (retval != NULL && retval[0] != '\0')
    return retval;
  /* Next comes the name of the desired category.  */
  retval = getenv (categoryname);
  if (retval != NULL && retval[0] != '\0')
    return retval;
  /* Last possibility is the LANG environment variable.  */
  retval = getenv ("LANG");
  if (retval != NULL && retval[0] != '\0')
    {
#if defined HAVE_CFLOCALECOPYCURRENT || defined HAVE_CFPREFERENCESCOPYAPPVALUE
      /* Mac OS X 10.2 or newer.
         Ignore invalid LANG value set by the Terminal application.  */
      if (strcmp (retval, "UTF-8") != 0)
#endif
#if defined __CYGWIN__
      /* Cygwin.
         Ignore dummy LANG value set by ~/.profile.  */
      if (strcmp (retval, "C.UTF-8") != 0)
#endif
        return retval;
    }

  return NULL;
}

#if defined __MINT__ || defined __TOS__
#include <mint/cookie.h>
#include <mint/mintbind.h>
#include <mint/sysvars.h>
#include <mint/ssystem.h>
static long get_oshdr(void)
{
	OSHEADER *hdr = *((OSHEADER **)0x4f2l);

	return (long)hdr;
}
#endif

const char *
gl_locale_name_default (void)
{
  /* POSIX:2001 says:
     "All implementations shall define a locale as the default locale, to be
      invoked when no environment variables are set, or set to the empty
      string.  This default locale can be the POSIX locale or any other
      implementation-defined locale.  Some implementations may provide
      facilities for local installation administrators to set the default
      locale, customizing it for each location.  POSIX:2001 does not require
      such a facility.

     The systems with such a facility are Mac OS X and Windows: They provide a
     GUI that allows the user to choose a locale.
       - On Mac OS X, by default, none of LC_* or LANG are set.  Starting with
         Mac OS X 10.4 or 10.5, LANG is set for processes launched by the
         'Terminal' application (but sometimes to an incorrect value "UTF-8").
         When no environment variable is set, setlocale (LC_ALL, "") uses the
         "C" locale.
       - On native Windows, by default, none of LC_* or LANG are set.
         When no environment variable is set, setlocale (LC_ALL, "") uses the
         locale chosen by the user.
       - On Cygwin 1.5.x, by default, none of LC_* or LANG are set.
         When no environment variable is set, setlocale (LC_ALL, "") uses the
         "C" locale.
       - On Cygwin 1.7, by default, LANG is set to "C.UTF-8" when the default
         ~/.profile is executed.
         When no environment variable is set, setlocale (LC_ALL, "") uses the
         "C.UTF-8" locale, which operates in the same way as the "C" locale.
  */

#if defined __MINT__ || defined __TOS__
	
	{
		long val = 0;
		short lang;
		OSHEADER *hdr;
		
		if (Cookie_ReadJar(C__AKP, &val))
		{
			lang = (short)(val >> 8) & 0xff;
		} else
		{
			if (Ssystem(-1, 0, 0) == 0)
			{
				val = Ssystem(S_OSHEADER, 26, 0);
				lang = (short)(val >> 1) & 0x7f;
			} else
			{
				hdr = (OSHEADER *)Supexec(get_oshdr);
				lang = (short)(hdr->os_conf >> 1) & 0x7f;
			}
		}
		switch (lang)
		{
			case 0: return "en_US";
			case 1: return "de";
			case 2: return "fr";
			case 3: return "en_UK";
			case 4: return "es";
			case 5: return "it";
			case 6: return "nl";
			case 7: return "fr_CH";
			case 8: return "de_CH";
			case 9: return "tr";
			case 10: return "fi";
			case 11: return "nb";
			case 12: return "da";
			case 13: return "ar_SA";
			case 14: return "nl";
			case 15: return "cs";
			case 16: return "hu";
			case 17: return "pl";
			case 18: return "lt";
			case 19: return "ru";
			case 20: return "et";
			case 21: return "be";
			case 22: return "uk";
			case 23: return "sk";
			case 24: return "ro";
			case 25: return "bg";
			case 26: return "sl";
			case 27: return "hr";
			case 28: return "sr";
			case 29: return "sr_ME";
			case 30: return "mk";
			case 31: return "el";
			case 32: return "lv";
			case 33: return "he";
			case 34: return "af_ZA";
			case 35: return "pt";
			case 36: return "nl_BE";
			case 37: return "ja";
			case 38: return "zh";
			case 39: return "ko";
			case 40: return "vi";
			case 41: return "hi_IN";
			case 42: return "fa";
			case 43: return "mn";
			case 44: return "ne";
			case 45: return "lo";
			case 46: return "km";
			case 47: return "id";
			case 48: return "bn_BD";
		}
	}
	
	/* The system does not have a way of setting the locale, other than the
	   POSIX specified environment variables.  We use C as default locale.  */
	return "C";

#elif !(defined HAVE_CFLOCALECOPYCURRENT || defined HAVE_CFPREFERENCESCOPYAPPVALUE || defined WINDOWS_NATIVE)

  /* The system does not have a way of setting the locale, other than the
     POSIX specified environment variables.  We use C as default locale.  */
  return "C";

#else

  /* Return an XPG style locale name language[_territory][@modifier].
     Don't even bother determining the codeset; it's not useful in this
     context, because message catalogs are not specific to a single
     codeset.  */

# if defined HAVE_CFLOCALECOPYCURRENT || defined HAVE_CFPREFERENCESCOPYAPPVALUE
  /* Mac OS X 10.2 or newer */
  {
    /* Cache the locale name, since CoreFoundation calls are expensive.  */
    static const char *cached_localename;

    if (cached_localename == NULL)
      {
        char namebuf[256];
#  if defined HAVE_CFLOCALECOPYCURRENT /* Mac OS X 10.3 or newer */
        CFLocaleRef locale = CFLocaleCopyCurrent ();
        CFStringRef name = CFLocaleGetIdentifier (locale);

        if (CFStringGetCString (name, namebuf, sizeof (namebuf),
                                kCFStringEncodingASCII))
          {
            gl_locale_name_canonicalize (namebuf);
            cached_localename = strdup (namebuf);
          }
        CFRelease (locale);
#  elif defined HAVE_CFPREFERENCESCOPYAPPVALUE /* Mac OS X 10.2 or newer */
        CFTypeRef value =
          CFPreferencesCopyAppValue (CFSTR ("AppleLocale"),
                                     kCFPreferencesCurrentApplication);
        if (value != NULL
            && CFGetTypeID (value) == CFStringGetTypeID ()
            && CFStringGetCString ((CFStringRef)value,
                                   namebuf, sizeof (namebuf),
                                   kCFStringEncodingASCII))
          {
            gl_locale_name_canonicalize (namebuf);
            cached_localename = strdup (namebuf);
          }
#  endif
        if (cached_localename == NULL)
          cached_localename = "C";
      }
    return cached_localename;
  }

# endif

# if defined WINDOWS_NATIVE
  {
    LCID lcid;

    /* Use native Windows API locale ID.  */
    lcid = GetThreadLocale ();

    return gl_locale_name_from_win32_LCID (lcid);
  }
# endif
#endif
}

# if defined WINDOWS_NATIVE
LANGID gl_locale_win32_messages_langid(void)
{
	return gl_locale_win32_langid_from_name(gl_locale_name(LC_MESSAGES, "LC_MESSAGES"));
}
#endif

/* Determine the current locale's name, and canonicalize it into XPG syntax
     language[_territory][.codeset][@modifier]
   The codeset part in the result is not reliable; the locale_charset()
   should be used for codeset information instead.
   The result must not be freed; it is statically allocated.  */

const char *
gl_locale_name (int category, const char *categoryname)
{
  const char *retval;

  retval = gl_locale_name_thread (category, categoryname);
  if (retval != NULL)
    return retval;

  retval = gl_locale_name_posix (category, categoryname);
  if (retval != NULL)
    return retval;

  return gl_locale_name_default ();
}


#if defined WINDOWS_NATIVE

#ifndef LOCALE_SNAME
#define LOCALE_SNAME                  0x0000005c
#endif
#ifndef LOCALE_SENGLISHLANGUAGENAME
#define LOCALE_SENGLISHLANGUAGENAME   LOCALE_SENGLANGUAGE
#endif

#ifndef LOCALE_ALL
#define LOCALE_ALL                  0                     /* enumerate all named based locales */
typedef BOOL (CALLBACK *LOCALE_ENUMPROCEX)(LPWSTR lpLocaleString, DWORD dwFlags, LPARAM lParam);

WINBASEAPI BOOL WINAPI EnumSystemLocalesEx(LOCALE_ENUMPROCEX, DWORD, LPARAM lParam, LPVOID lpReserved);
WINBASEAPI LCID WINAPI LocaleNameToLCID(LPCWSTR lpName, DWORD dwFlags);
#endif

typedef BOOL (WINAPI *LPENUMSYSTEMLOCALESEX)(LOCALE_ENUMPROCEX, DWORD, LPARAM lParam, LPVOID lpReserved);


static struct locale_search {
	char *crt_localename;
	char *crt_language;
	char *crt_country;
	unsigned int langid;
} static_lsearch;


static BOOL enum_lang(unsigned int langid, struct locale_search *psearch)
{
	char buf[100];
	if (!GetLocaleInfo(MAKELCID(langid, SORT_DEFAULT), LOCALE_SENGLISHLANGUAGENAME, buf, sizeof(buf)))
		return TRUE; /* continue enumeration */
	if (strcmp(buf, psearch->crt_language) != 0)
		return TRUE; /* continue enumeration */
	if (psearch->crt_country)
	{
		if (!GetLocaleInfo(MAKELCID(langid, SORT_DEFAULT), LOCALE_SENGCOUNTRY, buf, sizeof(buf)))
			return TRUE; /* continue enumeration */
		if (strcmp(buf, psearch->crt_country) != 0)
			return TRUE; /* continue enumeration */
	}
	psearch->langid = langid;
	/* found, stop enumeration */
	return FALSE;
}


static BOOL CALLBACK enum_languagesex(LPWSTR cp, DWORD flags, LPARAM lParam)
{
	struct locale_search *psearch = (struct locale_search *)lParam;
	(void) flags;
	
	return enum_lang(LANGIDFROMLCID(LocaleNameToLCID(cp, 0)), psearch);
}

static BOOL CALLBACK enum_languages(LPSTR cp)
{
	return enum_lang((unsigned int)(strtoul(cp, NULL, 16) & 0xffff), &static_lsearch);
}


char *gl_locale_name_posify(const char *locale)
{
	unsigned int langid;
	char *dot, *underscore;
	struct locale_search search;
	struct locale_search *psearch = &search;
	LPENUMSYSTEMLOCALESEX pEnumSystemLocalesEx;
	static char retbuf[64];
	static char lastsearched[64];
	static char lastfound[64];
	const char *retval;
	
	if (locale == NULL)
		return NULL;
	if (strcmp(lastsearched, locale) == 0)
		return strcpy(retbuf, lastfound);
	psearch->crt_localename = strdup(locale);
	if (psearch->crt_localename == NULL)
	{
		retval = locale;
		dot = NULL;
	} else
	{
		psearch->crt_language = psearch->crt_localename;
		dot = strchr(psearch->crt_localename, '.');
		if (dot != NULL)
			*dot = '\0';
		if (*psearch->crt_localename == '\0' || strcmp(psearch->crt_localename, "C") == 0)
		{
			retval = psearch->crt_localename;
		} else
		{
			underscore = strchr(psearch->crt_localename, '_');
			if (underscore != NULL)
			{
				*underscore++ = '\0';
				psearch->crt_country = underscore;
			} else
			{
				psearch->crt_country = NULL;
			}
			psearch->langid = 0;
			
			pEnumSystemLocalesEx = EnumSystemLocalesEx;
			if (pEnumSystemLocalesEx)
			{
				pEnumSystemLocalesEx(enum_languagesex, LOCALE_ALL, (LPARAM)psearch, NULL);
			} else
			{
				static_lsearch = *psearch;
				psearch = &static_lsearch;
				EnumSystemLocales(enum_languages, LCID_SUPPORTED);
			}
			langid = psearch->langid;
			if (langid == 0)
				retval = locale;
			else
				retval = gl_locale_name_from_win32_LANGID(langid);
		}
	}
	strncpy(lastsearched, locale, sizeof(lastsearched) - 1);
	strncpy(lastfound, retval, sizeof(lastfound) - 1);
	if (dot != NULL)
	{
		*dot++ = '.';
		strncat(lastfound, ".", sizeof(lastfound) - 1);
		if (strcmp(dot, "65001") == 0 || strcmp(dot, "CP65001") == 0)
			strncat(lastfound, "UTF-8", sizeof(lastfound) - 1);
		else if (strcmp(dot, "1252") == 0 || strcmp(dot, "CP1252") == 0)
			strncat(lastfound, "CP1252", sizeof(lastfound) - 1);
		else
			strncat(lastfound, dot, sizeof(lastfound) - 1);
	}
	free(psearch->crt_localename);
	return strcpy(retbuf, lastfound);
}

#endif
