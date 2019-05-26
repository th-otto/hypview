/*****************************************************************************
 * RSC_LANG.C
 *****************************************************************************/

#include "config.h"
#include <stdint.h>
#include <gem.h>
#include "debug.h"
#include "rsc_lang.h"
#include "langtab.h"

#define MAX_LANGUAGE_NAME 8

#define UTF8_BOM "\357\273\277"  /* "\xef\xbb\xbf" \uffef */

struct lang_id {
	RSC_LANG id;
	language_t akp;
	const char name[MAX_LANGUAGE_NAME];
	const char *english_name;
	const char *native_name;
};

static struct lang_id const lang_ids[] = {
	{ RSC_LANG_ENGLISH, LANGUAGE_EN_US, "en_US", "English (US)", "English (US)" },
	{ RSC_LANG_ENGLISH_GB, LANGUAGE_EN_GB, "en_GB", "English (GB)", "English (GB)" },
	{ RSC_LANG_ENGLISH_GB, LANGUAGE_EN_GB, "en_UK", NULL, NULL },
	{ RSC_LANG_GERMAN, LANGUAGE_DE_DE, "de_DE", "German", "Deutsch" },
	{ RSC_LANG_SPANISH, LANGUAGE_ES_ES, "es_ES", "Spanish", "Espa\244ol" },
	{ RSC_LANG_FRENCH, LANGUAGE_FR_FR, "fr_FR", "French", "Fran\207ais" },
	{ RSC_LANG_ITALIAN, LANGUAGE_IT_IT, "it_IT", "Italian", "Italiano" },
	{ RSC_LANG_DUTCH, LANGUAGE_NL_NL, "nl_NL", "Dutch", "Nederlands" },
	{ RSC_LANG_SWEDISH, LANGUAGE_SV_SE, "sv_SE", "Swedish", "Svenska" },
	{ RSC_LANG_SWISS_GERMAN, LANGUAGE_DE_CH, "de_CH", "Swiss (German)", "Schweizerisch (Deutsch)" },
	{ RSC_LANG_SWISS_FRENCH, LANGUAGE_FR_CH, "fr_CH", "Swiss (French)", "Suisse (Fran\207ais)" },
	{ RSC_LANG_TURKISH, LANGUAGE_TR_TR, "tr_TR", "Turkish", "T\201rk\207e" },
	{ RSC_LANG_FINNISH, LANGUAGE_FI_FI, "fi_FI", "Finnish", "Suomi" },
	{ RSC_LANG_NORWEGIAN, LANGUAGE_NB_NO, "nb_NO", "Norwegian", "Norsk" },
	{ RSC_LANG_NORWEGIAN, LANGUAGE_NN_NO, "nn_NO", NULL, NULL },
	{ RSC_LANG_NORWEGIAN, LANGUAGE_NB_NO, "no_NO", NULL, NULL }, /* obsolete old name */
	{ RSC_LANG_DANISH, LANGUAGE_DA_DK, "da_DK", "Danish", "Dansk" },
	{ RSC_LANG_ARABIC, LANGUAGE_AR_SA, "ar_SA", "Arabic", UTF8_BOM "\330\247\331\204\330\271\330\261\330\250\331\212\330\251" },
	{ RSC_LANG_CZECH, LANGUAGE_CS_CZ, "cs_CZ", "Czech", UTF8_BOM "\304\206esky" },
	{ RSC_LANG_HUNGARIAN, LANGUAGE_HU_HU, "hu_HU", "Hungarian", "Magyar" },
	{ RSC_LANG_POLISH, LANGUAGE_PL_PL, "pl_PL", "Polish", "Polskie" },
	{ RSC_LANG_LITHUANIAN, LANGUAGE_LT_LT, "lt_LT", "Lithuanian", UTF8_BOM "Lietuvi\305\263 kalba" },
	{ RSC_LANG_RUSSIAN, LANGUAGE_RU_RU, "ru_RU", "Russian", UTF8_BOM "\320\240\321\203\321\201\321\201\320\272\320\270\320\271" },
	{ RSC_LANG_ESTONIAN, LANGUAGE_ET_EE, "et_EE", "Estonian", "Eesti" },
	{ RSC_LANG_BELARUSIAN, LANGUAGE_BE_BY, "be_BY", "Belarusian", UTF8_BOM "\320\261\320\265\320\273\320\260\321\200\321\203\321\201\320\272\320\260\321\217 \320\274\320\276\320\262\320\260" },
	{ RSC_LANG_UKRAINIAN, LANGUAGE_UK_UA, "uk_UA", "Ukrainian", UTF8_BOM "\320\243\320\272\321\200\320\260\321\227\320\275\321\201\321\214\320\272\320\260" },
	{ RSC_LANG_SLOVAK, LANGUAGE_SK_SK, "sk_SK", "Slovak", UTF8_BOM "Sloven\304\215ina" },
	{ RSC_LANG_ROMANIAN, LANGUAGE_RO_RO, "ro_RO", "Romanian", UTF8_BOM "Rom\303\242n\304\203" },
	{ RSC_LANG_BULGARIAN, LANGUAGE_BG_BG, "bg_BG", "Bulgarian", UTF8_BOM "\320\261\321\212\320\273\320\263\320\260\321\200\321\201\320\272\320\270" },
	{ RSC_LANG_SLOVENIAN, LANGUAGE_SL_SI, "sl_SI", "Slovenian", UTF8_BOM "Sloven\305\241\304\215ina" },
	{ RSC_LANG_CROATIAN, LANGUAGE_HR_HR, "hr_HR", "Croatian", "Hrvatski jezik" },
	{ RSC_LANG_SERBIAN, LANGUAGE_SR_RS, "sr_RS", "Serbian", UTF8_BOM "\321\201\321\200\320\277\321\201\320\272\320\270 \321\230\320\265\320\267\320\270\320\272" },
	{ RSC_LANG_MONTENEGRIN, LANGUAGE_SR_ME, "sr_ME", "Montenegrin", UTF8_BOM "\321\206\321\200\320\275\320\276\320\263\320\276\321\200\321\201\320\272\320\270" },
	{ RSC_LANG_MACEDONIAN, LANGUAGE_MK_MK, "mk_MK", "Macedonian", UTF8_BOM "\320\274\320\260\320\272\320\265\320\264\320\276\320\275\321\201\320\272\320\270 \321\230\320\260\320\267\320\270\320\272" },
	{ RSC_LANG_GREEK, LANGUAGE_EL_GR, "el_GR", "Greek", UTF8_BOM "\316\225\316\273\316\273\316\267\316\275\316\271\316\272\316\254" },
	{ RSC_LANG_LATVIAN, LANGUAGE_LV_LV, "lv_LV", "Latvian", UTF8_BOM "Latvie\305\241u" },
	{ RSC_LANG_HEBREW, LANGUAGE_HE_IL, "he_IL", "Hebrew", UTF8_BOM "\327\242\326\264\327\221\327\250\326\264\327\231\327\252" },
	{ RSC_LANG_AFRIKAANS_SOUTH_AFRICA, LANGUAGE_AF_ZA, "af_ZA", "Afrikaans", "Afrikaans" },
	{ RSC_LANG_PORTUGUESE, LANGUAGE_PT_PT, "pt_PT", "Portuguese", "Portugu\302\210s" },
	{ RSC_LANG_BELGIAN, LANGUAGE_NL_BE, "nl_BE", "Dutch (Belgian)", "Nederlands (Belgian)" },
	{ RSC_LANG_JAPANESE, LANGUAGE_JA_JP, "ja_JP", "Japanese", UTF8_BOM "\346\227\245\346\234\254\350\252\236" },
	{ RSC_LANG_CHINESE, LANGUAGE_ZH_CN, "zh_CN", "Chinese", UTF8_BOM "\344\270\255\346\226\207" },
	{ RSC_LANG_KOREAN, LANGUAGE_KO_KO, "ko_KO", "Korean", UTF8_BOM "\355\225\234\352\265\255\354\226\264" },
	{ RSC_LANG_VIETNAMESE, LANGUAGE_VI_VN, "vi_VN", "Vietnamese", UTF8_BOM "Ti\341\272\277ng Vi\341\273\207t" },
	{ RSC_LANG_HINDI, LANGUAGE_HI_IN, "hi_IN", "Hindi", UTF8_BOM "\340\244\271\340\244\277\340\244\202\340\244\246\340\245\200" },
	{ RSC_LANG_FARSI, LANGUAGE_FA_IR, "fa_IR", "Farsi", UTF8_BOM "\331\201\330\247\330\261\330\263\333\214" },
	{ RSC_LANG_MONGOLIAN, LANGUAGE_MN_MN, "mn_MN", "Mongolian", UTF8_BOM "\320\234\320\276\320\275\320\263\320\276\320\273 \321\205\321\215\320\273" },
	{ RSC_LANG_NEPALI, LANGUAGE_NE_NP, "ne_NP", "Nepali", UTF8_BOM "\340\244\250\340\245\207\340\244\252\340\244\276\340\244\262\340\245\200" },
	{ RSC_LANG_LAO, LANGUAGE_LO_LA, "lo_LA", "Lao", UTF8_BOM "\340\272\245\340\272\262\340\272\247" },
	{ RSC_LANG_CAMBODIAN, LANGUAGE_KM_KH, "km_KH", "Khmer", UTF8_BOM "\341\236\227\341\236\266\341\236\237\341\236\266\341\236\201\341\237\222\341\236\230\341\237\202\341\236\232" },
	{ RSC_LANG_INDONESIAN, LANGUAGE_ID_ID, "id_ID", "Indonesian", "bahasa Indonesia" },
	{ RSC_LANG_BENGALI, LANGUAGE_BN_BD, "bn_BD", "Bengali", UTF8_BOM "\340\246\254\340\246\276\340\246\231\340\246\276\340\246\262\340\246\277" },
	
	{ RSC_LANG_NONE, 0, { 0 }, NULL, NULL }
};

/* ------------------------------------------------------------------- */

_BOOL rsc_lang_split(LANG_ARRAY arr, const char *_str)
{
	_WORD count = 0;
	char *start;
	_BOOL found = FALSE;
	char *str;

	str = (char *)NO_CONST(_str);
	start = str;
	while (*str != '\0')
	{
		arr[count].start = str;
		if (*str == RSC_LANG_DELIM)
		{
			arr[count].start = start;
			found = TRUE;
		}
		while (*str != '\0' && *str != RSC_LANG_DELIM)
			str++;
		arr[count].end = str;
		count++;
		if (*str != '\0')
			*str++ = '\0';
	}
	if (count > 0)
		arr[count - 1].end = NULL;
	while (count < RSC_LANG_MAX)
	{
		arr[count].start = start;
		arr[count].end = NULL;
		count++;
	}
	return found;
}

/* ------------------------------------------------------------------- */

_VOID rsc_lang_unsplit(LANG_ARRAY arr)
{
	_WORD i;

	for (i = 0; i < RSC_LANG_MAX; i++)
	{
		if (arr[i].end != NULL)
			arr[i].end[0] = RSC_LANG_DELIM;
	}
}

/* ------------------------------------------------------------------- */

char *rsc_lang_make_str(LANG_ARRAY arr)
{
	_WORD i, count;
	_LONG len;
	char *txt, *dst, *src;

	/*
	 * eliminate duplicate strings, except for the first one
	 */
	for (i = 1; i < RSC_LANG_MAX; i++)
	{
		if (arr[i].start != arr[0].start &&
			strcmp(arr[i].start, arr[0].start) == 0)
			arr[i].start[0] = '\0';
	}
	count = RSC_LANG_MAX;
	while (count > 1 && (arr[count - 1].start == arr[0].start || arr[count - 1].start[0] == '\0'))
		--count;
	/*
	 * be sure first string isn't empty if others aren't
	 */
	if (count > 1 && arr[0].start[0] == '\0')
	{
		for (i = count; --i > 0; )
		{
			if (arr[i].start[0] != '\0')
			{
				arr[0].start = arr[i].start;
				arr[i].start = NULL;
				if (i == (count - 1))
					--count;
				break;
			}
		}
	}
	/*
	 * calculate needed len
	 */
	len = 0;
	for (i = 0; i < count; i++)
		if (arr[i].start != NULL)
			len += strlen(arr[i].start) + 1;
	if ((txt = g_new(char, len)) != NULL)
	{
		/*
		 * construct string
		 */
		dst = txt;
		for (i = 0; i < count; i++)
		{
			src = arr[i].start;
			if (src != NULL)
			{
				while ((*dst++ = *src++) != '\0')
					{}
				dst[-1] = RSC_LANG_DELIM;
			}
		}
		dst[-1] = '\0';
	}
	return txt;
}

/* ------------------------------------------------------------------- */

char *rsc_language_str(char *str, RSC_LANG lang)
{
	LANG_ARRAY arr;
	char *txt;

	if (str == NULL)
		return NULL;
	rsc_lang_split(arr, str);
	txt = arr[lang].start;
	if (*txt == '\0')
		txt = arr[0].start;
	return txt;
}

/* ------------------------------------------------------------------- */

char *rsc_language_strdup(const char *str, RSC_LANG lang)
{
	char *txt;
	LANG_ARRAY arr;

	(void) rsc_lang_split(arr, str);
	txt = arr[lang].start;
	if (*txt == '\0')
		txt = arr[0].start;
	txt = g_strdup(txt);
	rsc_lang_unsplit(arr);
	return txt;
}

/* ------------------------------------------------------------------- */

RSC_LANG rsc_lang_name_to_id(const char *name)
{
	_WORD i;
	size_t len;
	
	for (i = 0; lang_ids[i].id >= 0; i++)
		if (g_ascii_strcasecmp(name, lang_ids[i].name) == 0)
			return lang_ids[i].id;
	len = 2;
	for (i = 0; lang_ids[i].id >= 0; i++)
		if (g_ascii_strncasecmp(name, lang_ids[i].name, len) == 0)
			return lang_ids[i].id;
	return RSC_LANG_NONE;
}
