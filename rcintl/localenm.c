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

/* Specification. */
#include "localename.h"

#ifdef HAVE_USELOCALE
/* Mac OS X 10.5 defines the locale_t type in <xlocale.h>. */
# if defined(__APPLE__) && defined(__MACH__)
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

#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__MSYS__)
# define WINDOWS_NATIVE
#endif

#if defined(WINDOWS_NATIVE) || defined(LANG_STRINGS)
#if defined(WINDOWS_NATIVE)
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
/* GetLocaleInfoA operations. */
# ifndef LOCALE_SNAME
# define LOCALE_SNAME                  0x0000005c
# endif
# ifndef LOCALE_SENGLISHLANGUAGENAME
# define LOCALE_SENGLISHLANGUAGENAME   LOCALE_SENGLANGUAGE
# endif
#endif
#ifndef MAKELANGID
#define MAKELANGID(p,s)        ((((LANGID)(s))<<10)|(LANGID)(p))
#endif
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
/* Mingw headers don't have latest language and sublanguage codes. */
#include "windows.rh"
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
     CFLocaleCreateCanonicalLocaleIdentifierFromString(). */
	typedef struct {
		const char legacy[21+1];
		const char unixy[5+1];
	} legacy_entry;
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
	   to Unix (ISO 639 and ISO 3166) names. */
	typedef struct {
		const char langtag[7+1];
		const char unixy[12+1];
	} langtag_entry;
	static const langtag_entry langtag_table[] = {
		/* Mac OS X has "az-Arab", "az-Cyrl", "az-Latn".
		   The default script for az on Unix is Latin. */
		{ "az-Latn", "az" },
		/* Mac OS X has "ga-dots". Does not yet exist on Unix. */
		{ "ga-dots", "ga" },
		/* Mac OS X has "kk-Cyrl". Does not yet exist on Unix. */
		/* Mac OS X has "mn-Cyrl", "mn-Mong".
		   The default script for mn on Unix is Cyrillic. */
		{ "mn-Cyrl", "mn" },
		/* Mac OS X has "ms-Arab", "ms-Latn".
		   The default script for ms on Unix is Latin. */
		{ "ms-Latn", "ms" },
		/* Mac OS X has "tg-Cyrl".
		   The default script for tg on Unix is Cyrillic. */
		{ "tg-Cyrl", "tg" },
		/* Mac OS X has "tk-Cyrl". Does not yet exist on Unix. */
		/* Mac OS X has "tt-Cyrl".
		   The default script for tt on Unix is Cyrillic. */
		{ "tt-Cyrl", "tt" },
		/* Mac OS X has "zh-Hans", "zh-Hant".
		   Country codes are used to distinguish these on Unix. */
		{ "zh-Hans", "zh_CN" },
		{ "zh-Hant", "zh_TW" }
	};

	/* Convert script names (ISO 15924) to Unix conventions.
	   See http://www.unicode.org/iso15924/iso15924-codes.html */
	typedef struct {
		const char script[4+1];
		const char unixy[9+1];
	} script_entry;
	static const script_entry script_table[] = {
		{"Arab", "arabic"},
		{"Cyrl", "cyrillic"},
		{"Mong", "mongolian"}
	};
	
	/* Step 1: Convert using legacy_table. */
	if (name[0] >= 'A' && name[0] <= 'Z')
	{
		unsigned int i1, i2;
	
		i1 = 0;
		i2 = sizeof(legacy_table) / sizeof(legacy_entry);
		while (i2 - i1 > 1)
		{
			/* At this point we know that if name occurs in legacy_table,
			   its index must be >= i1 and < i2. */
			unsigned int i = (i1 + i2) >> 1;
			const legacy_entry *p = &legacy_table[i];
	
			if (strcmp(name, p->legacy) < 0)
				i2 = i;
			else
				i1 = i;
		}
		if (strcmp(name, legacy_table[i1].legacy) == 0)
		{
			strcpy(name, legacy_table[i1].unixy);
			return;
		}
	}
	
	/* Step 2: Convert using langtag_table and script_table. */
	if (strlen(name) == 7 && name[2] == '-')
	{
		unsigned int i1, i2;
	
		i1 = 0;
		i2 = sizeof(langtag_table) / sizeof(langtag_entry);
		while (i2 - i1 > 1)
		{
			/* At this point we know that if name occurs in langtag_table,
			   its index must be >= i1 and < i2. */
			unsigned int i = (i1 + i2) >> 1;
			const langtag_entry *p = &langtag_table[i];
	
			if (strcmp(name, p->langtag) < 0)
				i2 = i;
			else
				i1 = i;
		}
		if (strcmp(name, langtag_table[i1].langtag) == 0)
		{
			strcpy(name, langtag_table[i1].unixy);
			return;
		}
	
		i1 = 0;
		i2 = sizeof(script_table) / sizeof(script_entry);
		while (i2 - i1 > 1)
		{
			/* At this point we know that if (name + 3) occurs in script_table,
			   its index must be >= i1 and < i2. */
			unsigned int i = (i1 + i2) >> 1;
			const script_entry *p = &script_table[i];
	
			if (strcmp(name + 3, p->script) < 0)
				i2 = i;
			else
				i1 = i;
		}
		if (strcmp(name + 3, script_table[i1].script) == 0)
		{
			name[2] = '@';
			strcpy(name + 3, script_table[i1].unixy);
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


#if defined(WINDOWS_NATIVE) || defined(LANG_STRINGS)
/* Dispatch on language.
   See also http://www.unicode.org/unicode/onlinedat/languages.html .
   For details about languages, see http://www.ethnologue.com/ .
 */
static struct _sublang_table {
	LANGID id;
#ifdef LANG_STRINGS
#  define LANG(lang, sub) MAKELANGID(lang, sub), #lang, #sub
	const char *langname;
	const char *subname;
#else
#  define LANG(lang, sub) MAKELANGID(lang, sub)
#endif
	unsigned short namelen;
	char po_name[8];
} const sublang_table[] = {
#define LN(x) (unsigned short)sizeof(x) - 1, x
	{ LANG(LANG_AFRIKAANS, SUBLANG_AFRIKAANS_SOUTH_AFRICA), LN("af_ZA") },
	{ LANG(LANG_AFRIKAANS, SUBLANG_NEUTRAL), LN("af") },
	{ LANG(LANG_ALBANIAN, SUBLANG_ALBANIAN_ALBANIA), LN("sq_AL") },
	{ LANG(LANG_ALBANIAN, SUBLANG_NEUTRAL), LN("sq") },
	{ LANG(LANG_ALSATIAN, SUBLANG_ALSATIAN_FRANCE), LN("al_FR") },
	{ LANG(LANG_ALSATIAN, SUBLANG_NEUTRAL), LN("al") },
	{ LANG(LANG_AMHARIC, SUBLANG_AMHARIC_ETHIOPIA), LN("am_ET") },
	{ LANG(LANG_AMHARIC, SUBLANG_NEUTRAL), LN("am") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_SAUDI_ARABIA), LN("ar_SA") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_IRAQ), LN("ar_IQ") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_EGYPT), LN("ar_EG") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_LIBYA), LN("ar_LY") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_ALGERIA), LN("ar_DZ") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_MOROCCO), LN("ar_MA") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_TUNISIA), LN("ar_TN") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_OMAN), LN("ar_OM") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_YEMEN), LN("ar_YE") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_SYRIA), LN("ar_SY") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_JORDAN), LN("ar_JO") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_LEBANON), LN("ar_LB") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_KUWAIT), LN("ar_KW") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_UAE), LN("ar_AE") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_BAHRAIN), LN("ar_BH") },
	{ LANG(LANG_ARABIC, SUBLANG_ARABIC_QATAR), LN("ar_QA") },
	{ LANG(LANG_ARABIC, SUBLANG_NEUTRAL), LN("ar") },
	{ LANG(LANG_ARMENIAN, SUBLANG_ARMENIAN_ARMENIA), LN("hy_AM") },
	{ LANG(LANG_ARMENIAN, SUBLANG_NEUTRAL), LN("hy") },
	{ LANG(LANG_ASSAMESE, SUBLANG_ASSAMESE_INDIA), LN("as_IN") },
	{ LANG(LANG_ASSAMESE, SUBLANG_NEUTRAL), LN("as") },
	{ LANG(LANG_AZERI, SUBLANG_AZERI_LATIN), LN("az_AZ") },
	{ LANG(LANG_AZERI, SUBLANG_AZERI_CYRILLIC), LN("az_AZ") },
	{ LANG(LANG_AZERI, 0x1d), LN("az") },
	{ LANG(LANG_AZERI, 0x1e), LN("az") },
	{ LANG(LANG_AZERI, SUBLANG_NEUTRAL), LN("az") },
	{ LANG(LANG_BASHKIR, SUBLANG_BASHKIR_RUSSIA), LN("ba_RU") },
	{ LANG(LANG_BASHKIR, SUBLANG_NEUTRAL), LN("ba") },
	{ LANG(LANG_BASQUE, SUBLANG_BASQUE_BASQUE), LN("eu_ES") },
	{ LANG(LANG_BASQUE, SUBLANG_NEUTRAL), LN("eu") },
	{ LANG(LANG_BELARUSIAN, SUBLANG_BELARUSIAN_BELARUS), LN("be_BY") },
	{ LANG(LANG_BELARUSIAN, SUBLANG_NEUTRAL), LN("be") },
	{ LANG(LANG_BENGALI, SUBLANG_BENGALI_INDIA), LN("bn_IN") },
	{ LANG(LANG_BENGALI, SUBLANG_BENGALI_BANGLADESH), LN("bn_BD") },
	{ LANG(LANG_BENGALI, SUBLANG_NEUTRAL), LN("bn") },
	{ LANG(LANG_BRETON, SUBLANG_BRETON_FRANCE), LN("br_FR") },
	{ LANG(LANG_BRETON, SUBLANG_NEUTRAL), LN("br") },
	{ LANG(LANG_BOSNIAN, SUBLANG_NEUTRAL), LN("bs") },
	{ LANG(LANG_BULGARIAN, SUBLANG_BULGARIAN_BULGARIA), LN("bg_BG") },
	{ LANG(LANG_BULGARIAN, SUBLANG_NEUTRAL), LN("bg") },
	{ LANG(LANG_BURMESE, SUBLANG_BURMESE_BURMA), LN("my_MM") },
	{ LANG(LANG_BURMESE, SUBLANG_NEUTRAL), LN("my") },
	{ LANG(LANG_CATALAN, SUBLANG_CATALAN_CATALAN), LN("ca_ES") },
	{ LANG(LANG_CATALAN, SUBLANG_VALENCIAN_VALENCIA), LN("ca_ES") },
	{ LANG(LANG_CATALAN, SUBLANG_NEUTRAL), LN("ca") },
	{ LANG(LANG_CHEROKEE, SUBLANG_CHEROKEE_CHEROKEE), LN("chr_US") },
	{ LANG(LANG_CHEROKEE, SUBLANG_NEUTRAL), LN("chr") },
	{ LANG(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL), LN("zh_TW") },
	{ LANG(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), LN("zh_CN") },
	{ LANG(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG), LN("zh_HK") },
	{ LANG(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE), LN("zh_SG") },
	{ LANG(LANG_CHINESE, SUBLANG_CHINESE_MACAU), LN("zh_MO") },
	{ LANG(LANG_CHINESE, 0x1f), LN("zh_TW") },
	{ LANG(LANG_CHINESE, SUBLANG_NEUTRAL), LN("zh") },
	{ LANG(LANG_CORSICAN, SUBLANG_CORSICAN_FRANCE), LN("co_FR") },
	{ LANG(LANG_CORSICAN, SUBLANG_NEUTRAL), LN("co") },
	{ LANG(LANG_CZECH, SUBLANG_CZECH_CZECH_REPUBLIC), LN("cs_CZ") },
	{ LANG(LANG_CZECH, SUBLANG_NEUTRAL), LN("cs") },
	{ LANG(LANG_DANISH, SUBLANG_DANISH_DENMARK), LN("da_DK") },
	{ LANG(LANG_DANISH, SUBLANG_NEUTRAL), LN("da") },
	{ LANG(LANG_DARI, SUBLANG_DARI_AFGHANISTAN), LN("prs_AF") },
	{ LANG(LANG_DARI, SUBLANG_NEUTRAL), LN("prs") },
	{ LANG(LANG_DIVEHI, SUBLANG_DIVEHI_MALDIVES), LN("dv_MV") },
	{ LANG(LANG_DIVEHI, SUBLANG_NEUTRAL), LN("dv") },
	{ LANG(LANG_DUTCH, SUBLANG_DUTCH), LN("nl_NL") },
	{ LANG(LANG_DUTCH, SUBLANG_DUTCH_BELGIAN), LN("nl_BE") },
	{ LANG(LANG_DUTCH, SUBLANG_DUTCH_SURINAM), LN("nl_SR") },
	{ LANG(LANG_DUTCH, SUBLANG_NEUTRAL), LN("nl") },
	{ LANG(LANG_DZONGKHA, SUBLANG_DZONGKHA_BHUTAN), LN("dz_BT") },
	{ LANG(LANG_DZONGKHA, SUBLANG_NEUTRAL), LN("dz") },
	{ LANG(LANG_EDO,  SUBLANG_EDO_NIGERIA), LN("bin_NG") },
	{ LANG(LANG_EDO, SUBLANG_NEUTRAL), LN("bin") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_US), LN("en_US") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_UK), LN("en_GB") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_AUS), LN("en_AU") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_CAN), LN("en_CA") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_NZ), LN("en_NZ") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_EIRE), LN("en_IE") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_IRELAND), LN("en_IE") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_SOUTH_AFRICA), LN("en_ZA") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_JAMAICA), LN("en_JM") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_CARIBBEAN), LN("en_GD") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_BELIZE), LN("en_BZ") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_TRINIDAD), LN("en_TT") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_ZIMBABWE), LN("en_ZW") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_PHILIPPINES), LN("en_PH") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_INDONESIA), LN("en_ID") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_HONGKONG), LN("en_HK") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_INDIA), LN("en_IN") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_MALAYSIA), LN("en_MY") },
	{ LANG(LANG_ENGLISH, SUBLANG_ENGLISH_SINGAPORE), LN("en_SG") },
	{ LANG(LANG_ENGLISH, SUBLANG_NEUTRAL), LN("en") },
	{ LANG(LANG_ESTONIAN, SUBLANG_ESTONIAN_ESTONIA), LN("et_EE") },
	{ LANG(LANG_ESTONIAN, SUBLANG_NEUTRAL), LN("et") },
	{ LANG(LANG_FAEROESE, SUBLANG_FAEROESE_FAROE_ISLANDS), LN("fo_FO") },
	{ LANG(LANG_FAEROESE, SUBLANG_NEUTRAL), LN("fo") },
	{ LANG(LANG_FARSI, SUBLANG_FARSI_IRAN), LN("fa_IR") },
	{ LANG(LANG_FARSI, SUBLANG_NEUTRAL), LN("fa") },
	{ LANG(LANG_FILIPINO, SUBLANG_FILIPINO_PHILIPPINES), LN("fil_PH") },
	{ LANG(LANG_FILIPINO, SUBLANG_NEUTRAL), LN("fil") },
	{ LANG(LANG_FINNISH, SUBLANG_FINNISH_FINLAND), LN("fi_FI") },
	{ LANG(LANG_FINNISH, SUBLANG_NEUTRAL), LN("fi") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH), LN("fr_FR") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_BELGIAN), LN("fr_BE") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_CANADIAN), LN("fr_CA") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_SWISS), LN("fr_CH") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_LUXEMBOURG), LN("fr_LU") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_MONACO), LN("fr_MC") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_WESTINDIES), LN("fr_CB") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_REUNION), LN("fr_RE") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_CONGO), LN("fr_CG") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_SENEGAL), LN("fr_SN") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_CAMEROON), LN("fr_CM") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_COTEDIVOIRE), LN("fr_CI") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_MALI), LN("fr_ML") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_MOROCCO), LN("fr_MA") },
	{ LANG(LANG_FRENCH, SUBLANG_FRENCH_HAITI), LN("fr_HT") },
	{ LANG(LANG_FRENCH, SUBLANG_NEUTRAL), LN("fr") },
	{ LANG(LANG_FRISIAN, SUBLANG_FRISIAN_NETHERLANDS), LN("fy_NL") },
	{ LANG(LANG_FRISIAN, SUBLANG_NEUTRAL), LN("fy") },
	{ LANG(LANG_FULAH, SUBLANG_FULAH_NIGERIA), LN("ff_NG") },
	{ LANG(LANG_FULAH, SUBLANG_FULAH_SENEGAL), LN("ff_SN") },
	{ LANG(LANG_FULAH, SUBLANG_NEUTRAL), LN("ff") },
	{ LANG(LANG_GALICIAN, SUBLANG_GALICIAN_GALICIAN), LN("gl_ES") },
	{ LANG(LANG_GALICIAN, SUBLANG_NEUTRAL), LN("gl") },
	{ LANG(LANG_GEORGIAN, SUBLANG_GEORGIAN_GEORGIA), LN("ka_GE") },
	{ LANG(LANG_GEORGIAN, SUBLANG_NEUTRAL), LN("ka") },
	{ LANG(LANG_GERMAN, SUBLANG_GERMAN), LN("de_DE") },
	{ LANG(LANG_GERMAN, SUBLANG_GERMAN_SWISS), LN("de_CH") },
	{ LANG(LANG_GERMAN, SUBLANG_GERMAN_AUSTRIAN), LN("de_AT") },
	{ LANG(LANG_GERMAN, SUBLANG_GERMAN_LUXEMBOURG), LN("de_LU") },
	{ LANG(LANG_GERMAN, SUBLANG_GERMAN_LIECHTENSTEIN), LN("de_LI") },
	{ LANG(LANG_GERMAN, SUBLANG_NEUTRAL), LN("de") },
	{ LANG(LANG_GREEK, SUBLANG_GREEK_GREECE), LN("el_GR") },
	{ LANG(LANG_GREEK, SUBLANG_NEUTRAL), LN("el") },
	{ LANG(LANG_GREENLANDIC, SUBLANG_GREENLANDIC_GREENLAND), LN("kl_GL") },
	{ LANG(LANG_GREENLANDIC, SUBLANG_NEUTRAL), LN("kl") },
	{ LANG(LANG_GUARANI, SUBLANG_GUARANI_PARAGUAY), LN("gn_PY") },
	{ LANG(LANG_GUARANI, SUBLANG_NEUTRAL), LN("gn") },
	{ LANG(LANG_GUJARATI, SUBLANG_GUJARATI_INDIA), LN("gu_IN") },
	{ LANG(LANG_GUJARATI, SUBLANG_NEUTRAL), LN("gu") },
	{ LANG(LANG_HAUSA, SUBLANG_HAUSA_NIGERIA_LATIN), LN("ha_NG") },
	{ LANG(LANG_HAUSA, 0x1f), LN("ha") },
	{ LANG(LANG_HAUSA, SUBLANG_NEUTRAL), LN("ha") },
	{ LANG(LANG_HAWAIIAN, SUBLANG_HAWAIIAN_US), LN("haw_US") },
	{ LANG(LANG_HAWAIIAN, SUBLANG_DEFAULT), LN("cpe_US") },
	{ LANG(LANG_HAWAIIAN, SUBLANG_NEUTRAL), LN("haw") },
	{ LANG(LANG_HEBREW, SUBLANG_HEBREW_ISRAEL), LN("he_IL") },
	{ LANG(LANG_HEBREW, SUBLANG_NEUTRAL), LN("he") },
	{ LANG(LANG_HINDI, SUBLANG_HINDI_INDIA), LN("hi_IN") },
	{ LANG(LANG_HINDI, SUBLANG_NEUTRAL), LN("hi") },
	{ LANG(LANG_HUNGARIAN, SUBLANG_HUNGARIAN_HUNGARY), LN("hu_HU") },
	{ LANG(LANG_HUNGARIAN, SUBLANG_NEUTRAL), LN("hu") },
	{ LANG(LANG_IBIBIO, SUBLANG_IBIBIO_NIGERIA), LN("ibb_NG") },
	{ LANG(LANG_IBIBIO, SUBLANG_NEUTRAL), LN("ibb") },
	{ LANG(LANG_ICELANDIC, SUBLANG_ICELANDIC_ICELAND), LN("is_IS") },
	{ LANG(LANG_ICELANDIC, SUBLANG_NEUTRAL), LN("is") },
	{ LANG(LANG_IGBO, SUBLANG_IGBO_NIGERIA), LN("ig_NG") },
	{ LANG(LANG_IGBO, SUBLANG_NEUTRAL), LN("ig") },
	{ LANG(LANG_INDONESIAN, SUBLANG_INDONESIAN_INDONESIA), LN("id_ID") },
	{ LANG(LANG_INDONESIAN, SUBLANG_NEUTRAL), LN("id") },
	{ LANG(LANG_INUKTITUT, SUBLANG_INUKTITUT_CANADA), LN("iu_CA") },
	{ LANG(LANG_INUKTITUT, SUBLANG_INUKTITUT_CANADA_LATIN), LN("iu_CA") },
	{ LANG(LANG_INUKTITUT, 0x1e), LN("iu") },
	{ LANG(LANG_INUKTITUT, 0x1f), LN("iu") },
	{ LANG(LANG_INUKTITUT, SUBLANG_NEUTRAL), LN("iu") },
	{ LANG(LANG_IRISH, SUBLANG_GAELIC), LN("gd_DB") },
	{ LANG(LANG_IRISH, SUBLANG_IRISH_IRELAND), LN("ga_IE") },
	{ LANG(LANG_IRISH, SUBLANG_NEUTRAL), LN("ga") },
	{ LANG(LANG_ITALIAN, SUBLANG_ITALIAN), LN("it_IT") },
	{ LANG(LANG_ITALIAN, SUBLANG_ITALIAN_SWISS), LN("it_CH") },
	{ LANG(LANG_ITALIAN, SUBLANG_NEUTRAL), LN("it") },
	{ LANG(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN), LN("ja_JP") },
	{ LANG(LANG_JAPANESE, SUBLANG_NEUTRAL), LN("ja") },
	{ LANG(LANG_KANURI, SUBLANG_KANURI_NIGERIA), LN("kr_NG") },
	{ LANG(LANG_KANURI, SUBLANG_NEUTRAL), LN("kr") },
	{ LANG(LANG_KANNADA, SUBLANG_KANNADA_INDIA), LN("kn_IN") },
	{ LANG(LANG_KANNADA, SUBLANG_NEUTRAL), LN("kn") },
	{ LANG(LANG_KASHMIRI, SUBLANG_KASHMIRI_PAKISTAN), LN("ks_PK") },
	{ LANG(LANG_KASHMIRI, SUBLANG_KASHMIRI_SASIA), LN("ks_IN") },
	{ LANG(LANG_KASHMIRI, SUBLANG_NEUTRAL), LN("ks") },
	{ LANG(LANG_KAZAK, SUBLANG_KAZAK_KAZAKHSTAN), LN("kk_KZ") },
	{ LANG(LANG_KAZAK, SUBLANG_NEUTRAL), LN("kk") },
	{ LANG(LANG_KHMER, SUBLANG_KHMER_CAMBODIA), LN("km_KH") },
	{ LANG(LANG_KHMER, SUBLANG_NEUTRAL), LN("km") },
	{ LANG(LANG_KICHE, SUBLANG_KICHE_GUATEMALA), LN("quc_GT") },
	{ LANG(LANG_KICHE, SUBLANG_NEUTRAL), LN("quc") },
	{ LANG(LANG_KINYARWANDA, SUBLANG_KINYARWANDA_RWANDA), LN("rw_RW") },
	{ LANG(LANG_KINYARWANDA, SUBLANG_NEUTRAL), LN("rw") },
	{ LANG(LANG_KONKANI, SUBLANG_KONKANI_INDIA), LN("kok_IN") },
	{ LANG(LANG_KONKANI, SUBLANG_NEUTRAL), LN("kok") },
	{ LANG(LANG_KOREAN, SUBLANG_KOREAN), LN("ko_KR") },
	{ LANG(LANG_KOREAN, SUBLANG_NEUTRAL), LN("ko") },
	{ LANG(LANG_CENTRAL_KURDISH, SUBLANG_CENTRAL_KURDISH_IRAQ), LN("ku_IQ") },
	{ LANG(LANG_CENTRAL_KURDISH, SUBLANG_NEUTRAL), LN("ku") },
	{ LANG(LANG_KYRGYZ, SUBLANG_KYRGYZ_KYRGYZSTAN), LN("ky_KG") },
	{ LANG(LANG_KYRGYZ, SUBLANG_NEUTRAL), LN("ky") },
	{ LANG(LANG_LAO, SUBLANG_LAO_LAO), LN("lo_LA") },
	{ LANG(LANG_LAO, SUBLANG_NEUTRAL), LN("lo") },
	{ LANG(LANG_LATIN, SUBLANG_LATIN_VATICAN), LN("la_VA") },
	{ LANG(LANG_LATIN, SUBLANG_NEUTRAL), LN("la") },
	{ LANG(LANG_LATVIAN, SUBLANG_LATVIAN_LATVIA), LN("lv_LV") },
	{ LANG(LANG_LATVIAN, SUBLANG_NEUTRAL), LN("lv") },
	{ LANG(LANG_LITHUANIAN, SUBLANG_LITHUANIAN), LN("lt_LT") },
	{ LANG(LANG_LITHUANIAN, SUBLANG_LITHUANIAN_CLASSIC), LN("lt_LT") },
	{ LANG(LANG_LITHUANIAN, SUBLANG_NEUTRAL), LN("lt") },
	{ LANG(LANG_LUXEMBOURGISH, SUBLANG_LUXEMBOURGISH_LUXEMBOURG), LN("lb_LU") },
	{ LANG(LANG_LUXEMBOURGISH, SUBLANG_NEUTRAL), LN("lb") },
	{ LANG(LANG_MACEDONIAN, SUBLANG_MACEDONIAN_MACEDONIA), LN("mk_MK") },
	{ LANG(LANG_MACEDONIAN, SUBLANG_NEUTRAL), LN("mk") },
	{ LANG(LANG_MALAGASY, SUBLANG_MALAGASY_MADAGASCAR), LN("mg_MG") },
	{ LANG(LANG_MALAGASY, SUBLANG_NEUTRAL), LN("mg") },
	{ LANG(LANG_MALAY, SUBLANG_MALAY_MALAYSIA), LN("ms_MY") },
	{ LANG(LANG_MALAY, SUBLANG_MALAY_BRUNEI_DARUSSALAM), LN("ms_BN") },
	{ LANG(LANG_MALAY, SUBLANG_NEUTRAL), LN("ms") },
	{ LANG(LANG_MALAYALAM, SUBLANG_MALAYALAM_INDIA), LN("ml_IN") },
	{ LANG(LANG_MALAYALAM, SUBLANG_NEUTRAL), LN("ml") },
	{ LANG(LANG_MALTESE, SUBLANG_MALTESE_MALTA), LN("mt_MT") },
	{ LANG(LANG_MALTESE, SUBLANG_NEUTRAL), LN("mt") },
	{ LANG(LANG_MANIPURI, SUBLANG_MANIPURI_MANIPUR), LN("mni_IN") },
	{ LANG(LANG_MANIPURI, SUBLANG_NEUTRAL), LN("mni") },
	{ LANG(LANG_MAORI, SUBLANG_MAORI_NEW_ZEALAND), LN("mi_NZ") },
	{ LANG(LANG_MAORI, SUBLANG_NEUTRAL), LN("mi") },
	{ LANG(LANG_MAPUDUNGUN, SUBLANG_MAPUDUNGUN_CHILE), LN("arn_CL") },
	{ LANG(LANG_MAPUDUNGUN, SUBLANG_NEUTRAL), LN("arn") },
	{ LANG(LANG_MARATHI, SUBLANG_MARATHI_INDIA), LN("mr_IN") },
	{ LANG(LANG_MARATHI, SUBLANG_NEUTRAL), LN("mr") },
	{ LANG(LANG_MOHAWK, SUBLANG_MOHAWK_MOHAWK), LN("moh_CA") },
	{ LANG(LANG_MOHAWK, SUBLANG_NEUTRAL), LN("moh") },
	{ LANG(LANG_MONGOLIAN, SUBLANG_MONGOLIAN_CYRILLIC_MONGOLIA), LN("mn_MN") },
	{ LANG(LANG_MONGOLIAN, SUBLANG_MONGOLIAN_PRC), LN("mn_CN") },
	{ LANG(LANG_MONGOLIAN, 0x1e), LN("mn_MN") },
	{ LANG(LANG_MONGOLIAN, 0x1f), LN("mn_CN") },
	{ LANG(LANG_MONGOLIAN, SUBLANG_NEUTRAL), LN("mn") },
	{ LANG(LANG_NEPALI, SUBLANG_NEPALI_NEPAL), LN("ne_NP") },
	{ LANG(LANG_NEPALI, SUBLANG_NEPALI_INDIA), LN("ne_IN") },
	{ LANG(LANG_NEPALI, SUBLANG_NEUTRAL), LN("ne") },
	{ LANG(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL), LN("nb_NO") },
	{ LANG(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_NYNORSK), LN("nn_NO") },
	{ LANG(LANG_NORWEGIAN, 0x1e), LN("nn") },
	{ LANG(LANG_NORWEGIAN, 0x1f), LN("nb") },
	{ LANG(LANG_NORWEGIAN, SUBLANG_NEUTRAL), LN("nb") },
	{ LANG(LANG_OCCITAN, SUBLANG_OCCITAN_FRANCE), LN("oc_FR") },
	{ LANG(LANG_OCCITAN, SUBLANG_NEUTRAL), LN("oc") },
	{ LANG(LANG_ORIYA, SUBLANG_ORIYA_INDIA), LN("or_IN") },
	{ LANG(LANG_ODIA, SUBLANG_NEUTRAL), LN("or") },
	{ LANG(LANG_OROMO, SUBLANG_OROMO_ETHIOPIA), LN("om_ET") },
	{ LANG(LANG_OROMO, SUBLANG_NEUTRAL), LN("om") },
	{ LANG(LANG_PAPIAMENTO, SUBLANG_PAPIAMENTO_NETHERLANDS_ANTILLES), LN("pap_AN") },
	{ LANG(LANG_PAPIAMENTO, SUBLANG_NEUTRAL), LN("pap") },
	{ LANG(LANG_PASHTO, SUBLANG_PASHTO_AFGHANISTAN), LN("ps_AF") },
	{ LANG(LANG_PASHTO, SUBLANG_NEUTRAL), LN("ps") },
	/* Note: LANG_PERSIAN == LANG_FARSI */
	{ LANG(LANG_PERSIAN, SUBLANG_PERSIAN_IRAN), LN("fa_IR") },
	{ LANG(LANG_PERSIAN, SUBLANG_NEUTRAL), LN("fa") },
	{ LANG(LANG_POLISH, SUBLANG_POLISH_POLAND), LN("pl_PL") },
	{ LANG(LANG_POLISH, SUBLANG_NEUTRAL), LN("pl") },
	{ LANG(LANG_PORTUGUESE, SUBLANG_PORTUGUESE), LN("pt_PT") },
	{ LANG(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN), LN("pt_BR") },
	{ LANG(LANG_PORTUGUESE, SUBLANG_NEUTRAL), LN("pt") },
	{ LANG(LANG_PUNJABI, SUBLANG_PUNJABI_INDIA), LN("pa_IN") },
	{ LANG(LANG_PUNJABI, SUBLANG_PUNJABI_PAKISTAN), LN("pa_PK") },
	{ LANG(LANG_PUNJABI, SUBLANG_NEUTRAL), LN("pa") },
	{ LANG(LANG_QUECHUA, SUBLANG_QUECHUA_BOLIVIA), LN("qu_BO") },
	{ LANG(LANG_QUECHUA, SUBLANG_QUECHUA_ECUADOR), LN("qu_EC") },
	{ LANG(LANG_QUECHUA, SUBLANG_QUECHUA_PERU), LN("qu_PE") },
	{ LANG(LANG_QUECHUA, SUBLANG_NEUTRAL), LN("qu") },
	{ LANG(LANG_ROMANIAN,  SUBLANG_ROMANIAN_ROMANIA), LN("ro_RO") },
	{ LANG(LANG_ROMANIAN,  SUBLANG_ROMANIAN_MOLDAVIA), LN("ro_MD") },
	{ LANG(LANG_ROMANIAN, SUBLANG_NEUTRAL), LN("ro") },
	{ LANG(LANG_ROMANSH, SUBLANG_ROMANSH_SWITZERLAND), LN("rm_CH") },
	{ LANG(LANG_ROMANSH, SUBLANG_NEUTRAL), LN("rm") },
	{ LANG(LANG_RUSSIAN, SUBLANG_RUSSIAN_RUSSIA), LN("ru_RU") },
	{ LANG(LANG_RUSSIAN, SUBLANG_RUSSIAN_MOLDAVIA), LN("ru_MD") },
	{ LANG(LANG_RUSSIAN, SUBLANG_NEUTRAL), LN("ru") },
	{ LANG(LANG_SAMI, SUBLANG_SAMI_NORTHERN_NORWAY), LN("se_NO") },
 	{ LANG(LANG_SAMI, SUBLANG_SAMI_NORTHERN_SWEDEN), LN("se_SE") },
 	{ LANG(LANG_SAMI, SUBLANG_SAMI_NORTHERN_FINLAND), LN("se_FI") },
 	{ LANG(LANG_SAMI, SUBLANG_SAMI_LULE_NORWAY), LN("smj_NO") },
 	{ LANG(LANG_SAMI, SUBLANG_SAMI_LULE_SWEDEN), LN("smj_SE") },
 	{ LANG(LANG_SAMI, SUBLANG_SAMI_SOUTHERN_NORWAY), LN("sma_NO") },
 	{ LANG(LANG_SAMI, SUBLANG_SAMI_SOUTHERN_SWEDEN), LN("sma_SE") },
 	{ LANG(LANG_SAMI, SUBLANG_SAMI_SKOLT_FINLAND), LN("sms_FI") },
 	{ LANG(LANG_SAMI, SUBLANG_SAMI_INARI_FINLAND), LN("smn_FI") },
 	{ LANG(LANG_SAMI, 0x1c), LN("smn") },
 	{ LANG(LANG_SAMI, 0x1d), LN("sms") },
 	{ LANG(LANG_SAMI, 0x1e), LN("sma") },
 	{ LANG(LANG_SAMI, 0x1f), LN("smj") },
	{ LANG(LANG_SAMI, SUBLANG_NEUTRAL), LN("se") },
	{ LANG(LANG_SANSKRIT, SUBLANG_SANSKRIT_INDIA), LN("sa_IN") },
	{ LANG(LANG_SANSKRIT, SUBLANG_NEUTRAL), LN("sa") },
	{ LANG(LANG_SCOTTISH_GAELIC, SUBLANG_SCOTTISH_GAELIC), LN("gd_GB") },
	{ LANG(LANG_SCOTTISH_GAELIC, SUBLANG_NEUTRAL), LN("gd") },
 	/* Note: LANG_BOSNIAN == LANG_SERBIAN == LANG_CROATIAN */
	{ LANG(LANG_BOSNIAN, SUBLANG_BOSNIAN_BOSNIA_HERZEGOVINA_LATIN), LN("bs_BA") },
	{ LANG(LANG_BOSNIAN, SUBLANG_BOSNIAN_BOSNIA_HERZEGOVINA_CYRILLIC), LN("bs_BA") },
	{ LANG(LANG_CROATIAN, SUBLANG_CROATIAN_CROATIA), LN("hr_HR") },
	{ LANG(LANG_CROATIAN, SUBLANG_CROATIAN_BOSNIA_HERZEGOVINA_LATIN), LN("hr_BA") },
	{ LANG(LANG_SERBIAN, SUBLANG_SERBIAN_CROATIA), LN("sr_RS") },
	{ LANG(LANG_SERBIAN, SUBLANG_SERBIAN_LATIN), LN("sr_RS") },
	{ LANG(LANG_SERBIAN, SUBLANG_SERBIAN_CYRILLIC), LN("sr_RS") },
	{ LANG(LANG_SERBIAN, SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_LATIN), LN("sr_BA") },
	{ LANG(LANG_SERBIAN, SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_CYRILLIC), LN("sr_BA") },
	{ LANG(LANG_SERBIAN, SUBLANG_SERBIAN_SERBIA_LATIN), LN("sr_RS") },
	{ LANG(LANG_SERBIAN, SUBLANG_SERBIAN_SERBIA_CYRILLIC), LN("sr_RS") },
	{ LANG(LANG_SERBIAN, SUBLANG_SERBIAN_MONTENEGRO_LATIN), LN("sr_ME") },
	{ LANG(LANG_SERBIAN, SUBLANG_SERBIAN_MONTENEGRO_CYRILLIC), LN("sr_ME") },
	{ LANG(LANG_BOSNIAN, 0x19), LN("bs") },
	{ LANG(LANG_BOSNIAN, 0x1a), LN("bs") },
	{ LANG(LANG_SERBIAN, 0x1b), LN("sr") },
	{ LANG(LANG_SERBIAN, 0x1c), LN("sr") },
	{ LANG(LANG_BOSNIAN, 0x1e), LN("bs") },
	{ LANG(LANG_SERBIAN, 0x1f), LN("sr_RS") },
	{ LANG(LANG_SERBIAN, SUBLANG_NEUTRAL), LN("sr") },
	{ LANG(LANG_SINDHI, SUBLANG_SINDHI_INDIA), LN("sd_IN") },
 	{ LANG(LANG_SINDHI, SUBLANG_SINDHI_PAKISTAN), LN("sd_PK") },
 	{ LANG(LANG_SINDHI, SUBLANG_SINDHI_AFGHANISTAN), LN("sd_AF") },
	{ LANG(LANG_SINDHI, SUBLANG_NEUTRAL), LN("sd") },
	{ LANG(LANG_SINHALESE, SUBLANG_SINHALESE_SRI_LANKA), LN("si_LK") },
	{ LANG(LANG_SINHALESE, SUBLANG_NEUTRAL), LN("si") },
	{ LANG(LANG_SLOVAK, SUBLANG_SLOVAK_SLOVAKIA), LN("sk_SK") },
	{ LANG(LANG_SLOVAK, SUBLANG_NEUTRAL), LN("sk") },
	{ LANG(LANG_SLOVENIAN, SUBLANG_SLOVENIAN_SLOVENIA), LN("sl_SI") },
	{ LANG(LANG_SLOVENIAN, SUBLANG_NEUTRAL), LN("sl") },
	{ LANG(LANG_SOMALI, SUBLANG_SOMALI_SOMALIA), LN("so_SO") },
	{ LANG(LANG_SOMALI, SUBLANG_NEUTRAL), LN("so") },
	{ LANG(LANG_SOTHO, SUBLANG_SOTHO_NORTHERN_SOUTH_AFRICA), LN("nso_ZA") },
	{ LANG(LANG_SOTHO, SUBLANG_NEUTRAL), LN("nso") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH), LN("es_ES") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_MEXICAN), LN("es_MX") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_MODERN), LN("es_ES") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_GUATEMALA), LN("es_GT") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_COSTA_RICA), LN("es_CR") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_PANAMA), LN("es_PA") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_DOMINICAN_REPUBLIC), LN("es_DO") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_VENEZUELA), LN("es_VE") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_COLOMBIA), LN("es_CO") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_PERU), LN("es_PE") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_ARGENTINA), LN("es_AR") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_ECUADOR), LN("es_EC") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_CHILE), LN("es_CL") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_URUGUAY), LN("es_UY") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_PARAGUAY), LN("es_PY") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_BOLIVIA), LN("es_BO") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_EL_SALVADOR), LN("es_SV") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_HONDURAS), LN("es_HN") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_NICARAGUA), LN("es_NI") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_PUERTO_RICO), LN("es_PR") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_US), LN("es_US") },
	{ LANG(LANG_SPANISH, SUBLANG_SPANISH_LATIN_AMERICA), LN("es_LA") },
	{ LANG(LANG_SPANISH, SUBLANG_NEUTRAL), LN("es") },
	{ LANG(LANG_SUTU, SUBLANG_SUTU_SOUTH_AFRICA), LN("sx_ZA") },
	{ LANG(LANG_SUTU, SUBLANG_NEUTRAL), LN("sx") },
	{ LANG(LANG_SWAHILI, SUBLANG_SWAHILI_KENYA), LN("sw_KE") },
	{ LANG(LANG_SWAHILI, SUBLANG_NEUTRAL), LN("sw") },
	{ LANG(LANG_SWEDISH, SUBLANG_SWEDISH), LN("sv_SE") },
	{ LANG(LANG_SWEDISH, SUBLANG_SWEDISH_FINLAND), LN("sv_FI") },
	{ LANG(LANG_SWEDISH, SUBLANG_NEUTRAL), LN("sv") },
	{ LANG(LANG_SYRIAC, SUBLANG_SYRIAC_SYRIA), LN("syr_SY") },
	{ LANG(LANG_SYRIAC, SUBLANG_NEUTRAL), LN("syr") },
	{ LANG(LANG_TAJIK, SUBLANG_TAJIK_TAJIKISTAN), LN("tg_TJ") },
	{ LANG(LANG_TAJIK, 0x1f), LN("tg") },
	{ LANG(LANG_TAJIK, SUBLANG_NEUTRAL), LN("tg") },
	{ LANG(LANG_TAMAZIGHT, SUBLANG_TAMAZIGHT_MOROCCO), LN("tzm_MA") },
	{ LANG(LANG_TAMAZIGHT, SUBLANG_TAMAZIGHT_MOROCCO_TIFINAGH), LN("tzm_MA") },
	{ LANG(LANG_TAMAZIGHT, SUBLANG_TAMAZIGHT_ALGERIA_LATIN), LN("tzm_DZ") },
	{ LANG(LANG_TAMAZIGHT, 0x1f), LN("ber") },
	{ LANG(LANG_TAMAZIGHT, SUBLANG_NEUTRAL), LN("tzm") },
	{ LANG(LANG_TAMIL, SUBLANG_TAMIL_INDIA), LN("ta_IN") },
	{ LANG(LANG_TAMIL, SUBLANG_TAMIL_SRI_LANKA), LN("ta_LK") },
	{ LANG(LANG_TAMIL, SUBLANG_NEUTRAL), LN("ta") },
	{ LANG(LANG_TATAR, SUBLANG_TATAR_RUSSIA), LN("tt_RU") },
	{ LANG(LANG_TATAR, SUBLANG_NEUTRAL), LN("tt") },
	{ LANG(LANG_TELUGU, SUBLANG_TELUGU_INDIA), LN("te_IN") },
	{ LANG(LANG_TELUGU, SUBLANG_NEUTRAL), LN("te") },
	{ LANG(LANG_THAI, SUBLANG_THAI_THAILAND), LN("th_TH") },
	{ LANG(LANG_THAI, SUBLANG_NEUTRAL), LN("th") },
	{ LANG(LANG_TIBETAN, SUBLANG_TIBETAN_PRC), LN("bo_CN") },
	{ LANG(LANG_TIBETAN, SUBLANG_TIBETAN_BHUTAN), LN("bo_BT") },
	{ LANG(LANG_TIBETAN, SUBLANG_NEUTRAL), LN("bo") },
	{ LANG(LANG_TIGRIGNA, SUBLANG_TIGRIGNA_ETHIOPIA), LN("ti_ET") },
	{ LANG(LANG_TIGRIGNA, SUBLANG_TIGRIGNA_ERITREA), LN("ti_ER") },
	{ LANG(LANG_TIGRIGNA, SUBLANG_NEUTRAL), LN("ti") },
	{ LANG(LANG_TSONGA, SUBLANG_TSONGA_SOUTH_AFRICA), LN("ts_ZA") },
	{ LANG(LANG_TSONGA, SUBLANG_NEUTRAL), LN("ts") },
	{ LANG(LANG_TSWANA, SUBLANG_TSWANA_SOUTH_AFRICA), LN("tn_ZA") },
	{ LANG(LANG_TSWANA, SUBLANG_TSWANA_BOTSWANA), LN("tn_BW") },
	{ LANG(LANG_TSWANA, SUBLANG_NEUTRAL), LN("tn") },
	{ LANG(LANG_TURKISH, SUBLANG_TURKISH_TURKEY), LN("tr_TR") },
	{ LANG(LANG_TURKISH, SUBLANG_NEUTRAL), LN("tr") },
	{ LANG(LANG_TURKMEN, SUBLANG_TURKMEN_TURKMENISTAN), LN("tk_TM") },
	{ LANG(LANG_TURKMEN, SUBLANG_NEUTRAL), LN("tk") },
	{ LANG(LANG_UIGHUR, SUBLANG_UIGHUR_PRC), LN("ug_CN") },
	{ LANG(LANG_UIGHUR, SUBLANG_NEUTRAL), LN("ug") },
	{ LANG(LANG_UKRAINIAN, SUBLANG_UKRAINIAN_UKRAINE), LN("uk_UA") },
	{ LANG(LANG_UKRAINIAN, SUBLANG_NEUTRAL), LN("uk") },
	{ LANG(LANG_SORBIAN, SUBLANG_UPPER_SORBIAN_GERMANY), LN("hsb_DE") },
	{ LANG(LANG_SORBIAN, SUBLANG_LOWER_SORBIAN_GERMANY), LN("dsb_DE") },
	{ LANG(LANG_SORBIAN, 0x1e), LN("hsb") },
	{ LANG(LANG_SORBIAN, 0x1f), LN("dsb") },
	{ LANG(LANG_SORBIAN, SUBLANG_NEUTRAL), LN("hsb") },
	{ LANG(LANG_URDU, SUBLANG_URDU_PAKISTAN), LN("ur_PK") },
	{ LANG(LANG_URDU, SUBLANG_URDU_INDIA), LN("ur_IN") },
	{ LANG(LANG_URDU, SUBLANG_NEUTRAL), LN("ur") },
	{ LANG(LANG_UZBEK, SUBLANG_UZBEK_LATIN), LN("uz_UZ") },
	{ LANG(LANG_UZBEK, SUBLANG_UZBEK_CYRILLIC), LN("uz_UZ") },
	{ LANG(LANG_UZBEK, 0x1e), LN("uz") },
	{ LANG(LANG_UZBEK, 0x1f), LN("uz") },
	{ LANG(LANG_UZBEK, SUBLANG_NEUTRAL), LN("uz") },
	{ LANG(LANG_VENDA, SUBLANG_VENDA_SOUTH_AFRICA), LN("ve_ZA") },
	{ LANG(LANG_VENDA, SUBLANG_NEUTRAL), LN("ve") },
	{ LANG(LANG_VIETNAMESE, SUBLANG_VIETNAMESE_VIETNAM), LN("vi_VN") },
	{ LANG(LANG_VIETNAMESE, SUBLANG_NEUTRAL), LN("vi") },
	{ LANG(LANG_WELSH, SUBLANG_WELSH_UNITED_KINGDOM), LN("cy_GB") },
	{ LANG(LANG_WELSH, SUBLANG_NEUTRAL), LN("cy") },
	{ LANG(LANG_WOLOF, SUBLANG_WOLOF_SENEGAL), LN("wo_SN") },
	{ LANG(LANG_WOLOF, SUBLANG_NEUTRAL), LN("wo") },
	{ LANG(LANG_XHOSA, SUBLANG_XHOSA_SOUTH_AFRICA), LN("xh_ZA") },
	{ LANG(LANG_XHOSA, SUBLANG_NEUTRAL), LN("xh") },
	{ LANG(LANG_YAKUT, SUBLANG_YAKUT_RUSSIA), LN("sah_RU") },
	{ LANG(LANG_YAKUT, SUBLANG_NEUTRAL), LN("sah") },
	{ LANG(LANG_YI, SUBLANG_YI_PRC), LN("ii_CN") },
	{ LANG(LANG_YI, SUBLANG_NEUTRAL), LN("ii") },
	{ LANG(LANG_YIDDISH, SUBLANG_YIDDISH_ISRAEL), LN("yi_IL") },
	{ LANG(LANG_YIDDISH, SUBLANG_NEUTRAL), LN("yi") },
	{ LANG(LANG_YORUBA, SUBLANG_YORUBA_NIGERIA), LN("yo_NG") },
	{ LANG(LANG_YORUBA, SUBLANG_NEUTRAL), LN("yo") },
	{ LANG(LANG_ZULU, SUBLANG_ZULU_SOUTH_AFRICA), LN("zu_ZA") },
	{ LANG(LANG_ZULU, SUBLANG_NEUTRAL), LN("zu") },

	/* wine only */
	{ LANG(LANG_WALLOON, SUBLANG_WALLOON_BELGIUM), LN("wa_BE") },
	{ LANG(LANG_WALLOON, SUBLANG_NEUTRAL), LN("wa") },
	{ LANG(LANG_CORNISH, SUBLANG_CORNISH_UK), LN("kw_GB") },
	{ LANG(LANG_CORNISH, SUBLANG_NEUTRAL), LN("kw") },
	{ LANG(LANG_ESPERANTO, SUBLANG_DEFAULT), LN("eo") },
	{ LANG(LANG_ESPERANTO, SUBLANG_NEUTRAL), LN("eo") },
	{ LANG(LANG_GAELIC, SUBLANG_GAELIC), LN("ga_IE") },
	{ LANG(LANG_GAELIC, SUBLANG_GAELIC_SCOTTISH), LN("gd_GB") },
	{ LANG(LANG_GAELIC, SUBLANG_GAELIC_MANX), LN("gv_GB") },
	{ LANG(LANG_GAELIC, SUBLANG_NEUTRAL), LN("ga") },

	/* other languages not covered by windows */
	{ LANG(LANG_AFAR, SUBLANG_NEUTRAL), LN("aa") },
	{ LANG(LANG_ABKHAZIAN, SUBLANG_NEUTRAL), LN("ab") },
	{ LANG(LANG_AVESTAN, SUBLANG_NEUTRAL), LN("ae") },
	{ LANG(LANG_AKAN, SUBLANG_NEUTRAL), LN("ak") },
	{ LANG(LANG_ARAGONESE, SUBLANG_NEUTRAL), LN("an") },
	{ LANG(LANG_AVARIC, SUBLANG_NEUTRAL), LN("av") },
	{ LANG(LANG_AYMARA, SUBLANG_NEUTRAL), LN("ay") },
	{ LANG(LANG_BIHARI, SUBLANG_NEUTRAL), LN("bh") },
	{ LANG(LANG_BISLAMA, SUBLANG_NEUTRAL), LN("bi") },
	{ LANG(LANG_BAMBARA, SUBLANG_NEUTRAL), LN("bm") },
	{ LANG(LANG_CHECHEN, SUBLANG_NEUTRAL), LN("ce") },
	{ LANG(LANG_CHAMORRO, SUBLANG_NEUTRAL), LN("ch") },
 	{ LANG(LANG_CREE, SUBLANG_NEUTRAL), LN("cr") },
	{ LANG(LANG_CHURCH_SLAVONIC, SUBLANG_NEUTRAL), LN("cu") },
	{ LANG(LANG_CHUVASH, SUBLANG_NEUTRAL), LN("cv") },
	{ LANG(LANG_EWE, SUBLANG_NEUTRAL), LN("ee") },
	{ LANG(LANG_FIJIAN, SUBLANG_NEUTRAL), LN("fj") },
	{ LANG(LANG_HIRI_MOTU, SUBLANG_NEUTRAL), LN("ho") },
	{ LANG(LANG_HAITIAN, SUBLANG_NEUTRAL), LN("ht") },
	{ LANG(LANG_HERERO, SUBLANG_NEUTRAL), LN("hz") },
	{ LANG(LANG_INTERLINGUA, SUBLANG_NEUTRAL), LN("ia") },
	{ LANG(LANG_INTERLINGUE, SUBLANG_NEUTRAL), LN("ie") },
	{ LANG(LANG_INUPIAQ, SUBLANG_NEUTRAL), LN("ik") },
	{ LANG(LANG_IDO, SUBLANG_NEUTRAL), LN("io") },
	{ LANG(LANG_JAVANESE, SUBLANG_NEUTRAL), LN("jv") },
	{ LANG(LANG_KONGO, SUBLANG_NEUTRAL), LN("kg") },
	{ LANG(LANG_GIKUYU, SUBLANG_NEUTRAL), LN("ki") },
	{ LANG(LANG_KWANYAMA, SUBLANG_NEUTRAL), LN("kj") },
	{ LANG(LANG_KOMI, SUBLANG_NEUTRAL), LN("kv") },
	{ LANG(LANG_GANDA, SUBLANG_NEUTRAL), LN("lg") },
	{ LANG(LANG_LIMBURGISH, SUBLANG_NEUTRAL), LN("li") },
	{ LANG(LANG_LINGALA, SUBLANG_NEUTRAL), LN("ln") },
	{ LANG(LANG_LUBA_KATANGA, SUBLANG_NEUTRAL), LN("lu") },
	{ LANG(LANG_MARSHALLESE, SUBLANG_NEUTRAL), LN("mh") },
	{ LANG(LANG_MOLDOVAN, SUBLANG_NEUTRAL), LN("mo") },
	{ LANG(LANG_NAURUAN, SUBLANG_NEUTRAL), LN("na") },
	{ LANG(LANG_NDEBELE_NORTH, SUBLANG_NEUTRAL), LN("nd") },
	{ LANG(LANG_NDEBELE_SOUTH, SUBLANG_NEUTRAL), LN("nr") },
	{ LANG(LANG_NDONGA, SUBLANG_NEUTRAL), LN("ng") },
	{ LANG(LANG_NAVAJO, SUBLANG_NEUTRAL), LN("nv") },
	{ LANG(LANG_CHEWA, SUBLANG_NEUTRAL), LN("ny") },
	{ LANG(LANG_OJIBWA, SUBLANG_NEUTRAL), LN("oj") },
	{ LANG(LANG_OSSETIC, SUBLANG_NEUTRAL), LN("os") },
	{ LANG(LANG_PALI, SUBLANG_NEUTRAL), LN("pI") },
	{ LANG(LANG_KIRUNDI, SUBLANG_NEUTRAL), LN("rn") },
	{ LANG(LANG_SARDINIAN, SUBLANG_NEUTRAL), LN("sc") },
	{ LANG(LANG_SANGO, SUBLANG_NEUTRAL), LN("sg") },
	{ LANG(LANG_SAMOAN, SUBLANG_NEUTRAL), LN("sm") },
	{ LANG(LANG_SHONA, SUBLANG_NEUTRAL), LN("sn") },
	{ LANG(LANG_SWAZI, SUBLANG_NEUTRAL), LN("ss") },
	{ LANG(LANG_SUNDANESE, SUBLANG_NEUTRAL), LN("su") },
	{ LANG(LANG_TAGALOG, SUBLANG_NEUTRAL), LN("tl") },
	{ LANG(LANG_TONGAN, SUBLANG_NEUTRAL), LN("to") },
	{ LANG(LANG_TWI, SUBLANG_NEUTRAL), LN("tw") },
	{ LANG(LANG_TAHITIAN, SUBLANG_NEUTRAL), LN("ty") },
	{ LANG(LANG_VOLAPUK, SUBLANG_NEUTRAL), LN("vo") },
	{ LANG(LANG_ZHUANG, SUBLANG_NEUTRAL), LN("za") },
	{ LANG(LANG_NEUTRAL, SUBLANG_NEUTRAL), LN("C") }
#undef LN
#undef LANG
};

#endif

#if defined(WINDOWS_NATIVE)

/* Canonicalize a Windows native locale name to a Unix locale name.
   NAME is a sufficiently large buffer.
   On input, it contains the Windows locale name.
   On output, it contains the Unix locale name. */
static void gl_locale_name_canonicalize(char *name)
{
	/* FIXME: This is probably incomplete: it does not handle "zh-Hans" and
	   "zh-Hant". */
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

const char *gl_locale_name_from_win32_LANGID(LANGID langid)
{
	int i;

	/* Activate the new code only when the GETTEXT_MUI environment variable is
	   set, for the time being, since the new code is not well tested. */
	if (getenv("GETTEXT_MUI") != NULL)
	{
		static char namebuf[256];

		/* Query the system's notion of locale name.
		   On Windows95/98/ME, GetLocaleInfoA returns some incorrect results.
		   But we don't need to support systems that are so old. */
		if (GetLocaleInfoA(MAKELCID(langid, SORT_DEFAULT), LOCALE_SNAME, namebuf, sizeof(namebuf) - 1))
		{
			/* Convert it to a Unix locale name. */
			gl_locale_name_canonicalize(namebuf);
			return namebuf;
		}
	}
	/* Internet Explorer has an LCID to RFC3066 name mapping stored in
	   HKEY_CLASSES_ROOT\Mime\Database\Rfc1766. But we better don't use that
	   since IE's i18n subsystem is known to be inconsistent with the native
	   Windows base (e.g. they have different character conversion facilities
	   that produce different results). */
	/* Use our own table. */

	for (i = 0; i < (int) (sizeof(sublang_table) / sizeof(sublang_table[0])); i++)
		if (sublang_table[i].id == langid)
			return sublang_table[i].po_name;
	langid = MAKELANGID(PRIMARYLANGID(langid), SUBLANG_NEUTRAL);
	for (i = 0; i < (int) (sizeof(sublang_table) / sizeof(sublang_table[0])); i++)
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


const char *gl_locale_name_from_win32_LCID(LCID lcid)
{
	LANGID langid;

	/* Strip off the sorting rules, keep only the language part. */
	langid = LANGIDFROMLCID (lcid);

	return gl_locale_name_from_win32_LANGID (langid);
}

#endif /* WINDOWS_NATIVE */


#ifdef HAVE_USELOCALE /* glibc or Mac OS X */

/* Simple hash set of strings. We don't want to drag in lots of hash table
   code here. */

# define SIZE_BITS (sizeof (size_t) * CHAR_BIT)

/* A hash function for NUL-terminated char* strings using
   the method described by Bruno Haible.
   See http://www.haible.de/bruno/hashfunc.html. */
static size_t string_hash(const void *x)
{
	const char *s = (const char *) x;
	size_t h = 0;

	for (; *s; s++)
		h = *s + ((h << 9) | (h >> (SIZE_BITS - 9)));

	return h;
}

/* A hash table of fixed size. Multiple threads can access it read-only
   simultaneously, but only one thread can insert into it at the same time. */

/* A node in a hash bucket collision list. */
struct hash_node
{
	struct hash_node * volatile next;
	char contents[100]; /* has variable size */
};

static const char *struniq(const char *string)
{
	size_t hashcode = string_hash(string);
	size_t slot = hashcode % HASH_TABLE_SIZE;
	size_t size;
	struct hash_node *new_node;
	struct hash_node *p;

	for (p = struniq_hash_table[slot]; p != NULL; p = p->next)
		if (strcmp(p->contents, string) == 0)
			return p->contents;
	size = strlen(string) + 1;
	new_node = (struct hash_node *) malloc(offsetof(struct hash_node, contents[0]) + size);

	if (new_node == NULL)
		/* Out of memory. Return a statically allocated string. */
		return "C";
	memcpy(new_node->contents, string, size);
	/* Lock while inserting new_node. */
	gl_lock_lock(struniq_lock);
	/* Check whether another thread already added the string while we were
	   waiting on the lock.  */
	for (p = struniq_hash_table[slot]; p != NULL; p = p->next)
		if (strcmp(p->contents, string) == 0)
		{
			free(new_node);
			new_node = p;
			goto done;
		}
	/* Really insert new_node into the hash table.  Fill new_node entirely first,
	   because other threads may be iterating over the linked list.  */
	new_node->next = struniq_hash_table[slot];
	struniq_hash_table[slot] = new_node;
  done:
	/* Unlock after new_node is inserted.  */
	gl_lock_unlock(struniq_lock);
	return new_node->contents;
}


# define HASH_TABLE_SIZE 257
static struct hash_node * volatile struniq_hash_table[HASH_TABLE_SIZE]
  /* = { NULL, ..., NULL } */;

/* This lock protects the struniq_hash_table against multiple simultaneous
   insertions.  */
gl_lock_define_initialized(static, struniq_lock)

/* Store a copy of the given string in a string pool with indefinite extent.
   Return a pointer to this copy.  */

#endif /* HAVE_USELOCALE */


/* Like gl_locale_name_thread, except that the result is not in storage of
   indefinite extent. */
const char *gl_locale_name_thread_unsafe(int category, const char *categoryname)
{
#if defined(HAVE_USELOCALE)
	{
		locale_t thread_locale = uselocale(NULL);

		(void) category;
		(void) categoryname;
		if (thread_locale != LC_GLOBAL_LOCALE)
		{
#  if defined(__GLIBC__) && __GLIBC__ >= 2 && !defined __UCLIBC__
			/* Work around an incorrect definition of the _NL_LOCALE_NAME macro in
			   glibc < 2.12.
			   See <http://sourceware.org/bugzilla/show_bug.cgi?id=10968>.  */
			const char *name = nl_langinfo(_NL_ITEM((category), _NL_ITEM_INDEX(-1)));

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
			default:					/* We shouldn't get here.  */
				return "";
			}
			return querylocale(mask, thread_locale);
#  endif
		}
	}
#else
	(void) category;
	(void) categoryname;
# endif
	return NULL;
}


const char *gl_locale_name_thread(int category, const char *categoryname)
{
#ifdef HAVE_USELOCALE
	const char *name = gl_locale_name_thread_unsafe(category, categoryname);

	if (name != NULL)
		return struniq(name);
#else
	(void) category;
	(void) categoryname;
#endif
	return NULL;
}

/* XPG3 defines the result of 'setlocale (category, NULL)' as:
   "Directs 'setlocale()' to query 'category' and return the current
    setting of 'local'."
   However it does not specify the exact format. Neither do SUSV2 and
   ISO C 99. So we can use this feature only on selected systems (e.g.
   those using GNU C Library). */
#if defined(_LIBC) || defined(_MINT_SOURCE) || ((defined(__GLIBC__) && __GLIBC__ >= 2) && !defined(__UCLIBC__))
# define HAVE_LOCALE_NULL
#endif

const char *gl_locale_name_posix(int category, const char *categoryname)
{
	/* Use the POSIX methods of looking to 'LC_ALL', 'LC_xxx', and 'LANG'.
	   On some systems this can be done by the 'setlocale' function itself. */
#if defined(HAVE_SETLOCALE) && defined(HAVE_LC_MESSAGES) && defined(HAVE_LOCALE_NULL)
	(void) categoryname;
	return setlocale(category, NULL);
#else
	/* On other systems we ignore what setlocale reports and instead look at the
	   environment variables directly. This is necessary
	   1. on systems which have a facility for customizing the default locale
	      (Mac OS X, native Windows, Cygwin) and where the system's setlocale()
	      function ignores this default locale (Mac OS X, Cygwin), in two cases:
	      a. when the user missed to use the setlocale() override from libintl
	         (for example by not including <libintl.h>),
	      b. when setlocale supports only the "C" locale, such as on Cygwin
	         1.5.x. In this case even the override from libintl cannot help.
	   2. on all systems where setlocale supports only the "C" locale. */
	/* Strictly speaking, it is a POSIX violation to look at the environment
	   variables regardless whether setlocale has been called or not. POSIX
	   says:
	        "For C-language programs, the POSIX locale shall be the
	        default locale when the setlocale() function is not called."
	   But we assume that all programs that use internationalized APIs call
	   setlocale (LC_ALL, ""). */
	return gl_locale_name_environ(category, categoryname);
#endif
}

const char *gl_locale_name_environ(int category, const char *categoryname)
{
	const char *retval;

	(void) category;
	/* Setting of LC_ALL overrides all other. */
	retval = getenv("LC_ALL");
	if (retval != NULL && retval[0] != '\0')
		return retval;
	/* Next comes the name of the desired category. */
	retval = getenv(categoryname);
	if (retval != NULL && retval[0] != '\0')
		return retval;
	/* Last possibility is the LANG environment variable. */
	retval = getenv("LANG");
	if (retval != NULL && retval[0] != '\0')
	{
#if defined(HAVE_CFLOCALECOPYCURRENT) || defined(HAVE_CFPREFERENCESCOPYAPPVALUE)
		/* Mac OS X 10.2 or newer.
		   Ignore invalid LANG value set by the Terminal application. */
		if (strcmp(retval, "UTF-8") != 0)
#endif
#ifdef __CYGWIN__
			/* Cygwin.
			   Ignore dummy LANG value set by ~/.profile. */
			if (strcmp(retval, "C.UTF-8") != 0)
#endif
				return retval;
	}

	return NULL;
}


#if defined(__MINT__) || defined(__TOS__)
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

const char *gl_locale_name_default(void)
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

#if defined(__MINT__) || defined(__TOS__)
	
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
	   POSIX specified environment variables. We use C as default locale. */
	return "C";

#elif !(defined(HAVE_CFLOCALECOPYCURRENT) || defined(HAVE_CFPREFERENCESCOPYAPPVALUE) || defined(WINDOWS_NATIVE))

	/* The system does not have a way of setting the locale, other than the
	   POSIX specified environment variables. We use C as default locale. */
	return "C";

#else

	/* Return an XPG style locale name language[_territory][@modifier].
	   Don't even bother determining the codeset; it's not useful in this
	   context, because message catalogs are not specific to a single
	   codeset. */

# if defined(HAVE_CFLOCALECOPYCURRENT) || defined(HAVE_CFPREFERENCESCOPYAPPVALUE)
	/* Mac OS X 10.2 or newer */
	{
		/* Cache the locale name, since CoreFoundation calls are expensive. */
		static const char *cached_localename;
	
		if (cached_localename == NULL)
		{
			char namebuf[256];
	
#  if defined(HAVE_CFLOCALECOPYCURRENT)	/* Mac OS X 10.3 or newer */
			CFLocaleRef locale = CFLocaleCopyCurrent();
			CFStringRef name = CFLocaleGetIdentifier(locale);
	
			if (CFStringGetCString(name, namebuf, sizeof(namebuf), kCFStringEncodingASCII))
			{
				gl_locale_name_canonicalize(namebuf);
				cached_localename = strdup(namebuf);
			}
			CFRelease(locale);
#  elif defined(HAVE_CFPREFERENCESCOPYAPPVALUE)	/* Mac OS X 10.2 or newer */
			CFTypeRef value = CFPreferencesCopyAppValue(CFSTR("AppleLocale"),
														kCFPreferencesCurrentApplication);
	
			if (value != NULL
				&& CFGetTypeID(value) == CFStringGetTypeID()
				&& CFStringGetCString((CFStringRef) value, namebuf, sizeof(namebuf), kCFStringEncodingASCII))
			{
				gl_locale_name_canonicalize(namebuf);
				cached_localename = strdup(namebuf);
			}
#  endif
			if (cached_localename == NULL)
				cached_localename = "C";
		}
		return cached_localename;
	}

# endif

# if defined(WINDOWS_NATIVE)
	{
		LCID lcid;

		/* Use native Windows API locale ID. */
		lcid = GetThreadLocale();

		return gl_locale_name_from_win32_LCID(lcid);
	}
# endif
#endif
}

# if defined(WINDOWS_NATIVE)
LANGID gl_locale_win32_messages_langid(void)
{
	return gl_locale_win32_langid_from_name(gl_locale_name(LC_MESSAGES, "LC_MESSAGES"));
}
#endif

/* Determine the current locale's name, and canonicalize it into XPG syntax
     language[_territory][.codeset][@modifier]
   The codeset part in the result is not reliable; the locale_charset()
   should be used for codeset information instead.
   The result must not be freed; it is statically allocated. */

const char *gl_locale_name(int category, const char *categoryname)
{
	const char *retval;

	retval = gl_locale_name_thread(category, categoryname);
	if (retval != NULL)
		return retval;

	retval = gl_locale_name_posix(category, categoryname);
	if (retval != NULL)
		return retval;

	return gl_locale_name_default();
}


#if defined(WINDOWS_NATIVE)

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
	if (!GetLocaleInfoA(MAKELCID(langid, SORT_DEFAULT), LOCALE_SENGLISHLANGUAGENAME, buf, sizeof(buf)))
		return TRUE; /* continue enumeration */
	if (strcmp(buf, psearch->crt_language) != 0)
		return TRUE; /* continue enumeration */
	if (psearch->crt_country)
	{
		if (!GetLocaleInfoA(MAKELCID(langid, SORT_DEFAULT), LOCALE_SENGCOUNTRY, buf, sizeof(buf)))
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
			if (underscore != NULL)
				*--underscore = '_';
			langid = psearch->langid;
			if (langid == 0)
				retval = psearch->crt_localename;
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

#endif /* WINDOWS_NATIVE */
