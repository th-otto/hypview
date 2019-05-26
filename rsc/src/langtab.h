#ifndef _LANG_TABLE_H
#define _LANG_TABLE_H

#include <stddef.h>

/* TOS NVRAM language/keyboard setting */
typedef enum {
	COUNTRY_NONE = -1,
	LANGUAGE_NONE = -1,
	COUNTRY_US,	/* USA */
	LANGUAGE_EN_US = COUNTRY_US, /* 0 */
	COUNTRY_DE,	/* Germany */
	LANGUAGE_DE_DE = COUNTRY_DE, /* 1 */
	LANGUAGE_DE_AT = COUNTRY_DE, /* 1 */
	COUNTRY_FR,	/* France */
	LANGUAGE_FR_FR = COUNTRY_FR, /* 2 */
	COUNTRY_UK,	/* United Kingdom */
	LANGUAGE_EN_GB = COUNTRY_UK, /* 3 */
	COUNTRY_ES,	/* Spain */
	LANGUAGE_ES_ES = COUNTRY_ES, /* 4 */
	LANGUAGE_ES_AR = COUNTRY_ES, /* 4 */
	LANGUAGE_ES_IC = COUNTRY_ES, /* 4 */
	COUNTRY_IT,	/* Italy */
	LANGUAGE_IT_IT = COUNTRY_IT, /* 5 */
	COUNTRY_SE,	/* Sweden */
	LANGUAGE_SV_SE = COUNTRY_SE,    /* 6 */
	COUNTRY_SF,	/* Switzerland (French) */
	LANGUAGE_FR_CH = COUNTRY_SF, /* 7 */
	COUNTRY_SG,	/* Switzerland (German) */
	LANGUAGE_DE_CH = COUNTRY_SG, /* 8 */
	COUNTRY_TR,	/* Turkey */
	LANGUAGE_TR_TR = COUNTRY_TR, /* 9 */
	COUNTRY_FI,	/* Finland */
	LANGUAGE_FI_FI = COUNTRY_FI, /* 10 */
	COUNTRY_NO,	/* Norway */
	LANGUAGE_NB_NO = COUNTRY_NO, /* 11 */
	LANGUAGE_NN_NO = COUNTRY_NO,
	COUNTRY_DK,	/* Denmark */
	LANGUAGE_DA_DK = COUNTRY_DK, /* 12 */
	COUNTRY_SA,	/* Saudi Arabia */
	LANGUAGE_AR_SA = COUNTRY_SA, /* 13 */
	COUNTRY_NL,	/* Holland */
	LANGUAGE_NL_NL = COUNTRY_NL, /* 14 */
	COUNTRY_CZ,	/* Czech Republic */
	LANGUAGE_CS_CZ = COUNTRY_CZ, /* 15 */
	COUNTRY_HU,	/* Hungary */
	LANGUAGE_HU_HU = COUNTRY_HU, /* 16 */
	COUNTRY_PL,	/* Poland */
	LANGUAGE_PL_PL = COUNTRY_PL, /* 17 */
	COUNTRY_LT,	/* Lithuania */
	LANGUAGE_LT_LT = COUNTRY_LT, /* 18 */
	COUNTRY_RU, /* Russia */
	LANGUAGE_RU_RU = COUNTRY_RU, /* 19 */
	COUNTRY_EE, /* Estonia */
	LANGUAGE_ET_EE = COUNTRY_EE, /* 20 */
	COUNTRY_BY, /* Belarus */
	LANGUAGE_BE_BY = COUNTRY_BY, /* 21 */
	COUNTRY_UA, /* Ukraine */
	LANGUAGE_UK_UA = COUNTRY_UA, /* 22 */
	COUNTRY_SK,	/* Slovak Republic */
	LANGUAGE_SK_SK = COUNTRY_SK, /* 23 */
	COUNTRY_RO, /* Romania */
	LANGUAGE_RO_RO = COUNTRY_RO, /* 24 */
	COUNTRY_BG, /* Bulgaria */
	LANGUAGE_BG_BG = COUNTRY_BG, /* 25 */
	COUNTRY_SI, /* Slovenia */
	LANGUAGE_SL_SI = COUNTRY_SI, /* 26 */
	COUNTRY_HR, /* Croatia */
	LANGUAGE_HR_HR = COUNTRY_HR, /* 27 */
	COUNTRY_RS, /* Serbia */
	LANGUAGE_SR_RS = COUNTRY_RS, /* 28 */
	COUNTRY_ME, /* Montenegro */
	LANGUAGE_SR_ME = COUNTRY_ME, /* 29 */
	COUNTRY_MK, /* Macedonia */
	LANGUAGE_MK_MK = COUNTRY_MK, /* 30 */
	COUNTRY_GR,	/* Greece */
	LANGUAGE_EL_GR = COUNTRY_GR, /* 31 */
	COUNTRY_LV, /* Latvia */
	LANGUAGE_LV_LV = COUNTRY_LV, /* 32 */
	COUNTRY_IL, /* Israel */
	LANGUAGE_HE_IL = COUNTRY_IL, /* 33 */
	COUNTRY_ZA, /* South Africa */
	LANGUAGE_AF_ZA = COUNTRY_ZA, /* 34 */
	COUNTRY_PT, /* Portugal */
	LANGUAGE_PT_PT = COUNTRY_PT, /* 35 */
	LANGUAGE_PT_BR = COUNTRY_PT, /* 35 */
	COUNTRY_BE, /* Belgium */
	LANGUAGE_NL_BE = COUNTRY_BE, /* 36 */
	COUNTRY_JP, /* Japan */
	LANGUAGE_JA_JP = COUNTRY_JP, /* 37 */
	COUNTRY_CN, /* China */
	LANGUAGE_ZH_CN = COUNTRY_CN, /* 38 */
	LANGUAGE_ZH_HK = COUNTRY_CN, /* 38 */
	LANGUAGE_ZH_TW = COUNTRY_CN, /* 38 */
	COUNTRY_KR, /* Korea */
	LANGUAGE_KO_KO = COUNTRY_KR, /* 39 */
	COUNTRY_VN, /* Vietnam */
	LANGUAGE_VI_VN = COUNTRY_VN, /* 40 */
	COUNTRY_IN, /* India */
	LANGUAGE_HI_IN = COUNTRY_IN, /* 41 */
	COUNTRY_IR, /* Iran */
	LANGUAGE_FA_IR = COUNTRY_IR, /* 42 */
	COUNTRY_MN, /* Mongolia */
	LANGUAGE_MN_MN = COUNTRY_MN, /* 43 */
	COUNTRY_NP, /* Nepal */
	LANGUAGE_NE_NP = COUNTRY_NP, /* 44 */
	COUNTRY_LA, /* Lao People's Democratic Republic */
	LANGUAGE_LO_LA = COUNTRY_LA, /* 45 */
	COUNTRY_KH, /* Cambodia */
	LANGUAGE_KM_KH = COUNTRY_KH, /* 46 */
	COUNTRY_ID, /* Indonesia */
	LANGUAGE_ID_ID = COUNTRY_ID, /* 47 */
	COUNTRY_BD, /* Bangladesh */
	LANGUAGE_BN_BD = COUNTRY_BD, /* 48 */
	COUNTRY_MX = 99, /* Mexico (found in Atari sources) */
	LANGUAGE_ES_MX = COUNTRY_MX  /* 99 */
} language_t;

struct language_table_entry
{
	const char *code;
	const char *english;
	language_t language_code;
};

extern struct language_table_entry const language_table[];
extern const size_t language_table_size;

extern struct language_table_entry const language_variant_table[];
extern const size_t language_variant_table_size;

#endif /* _LANG_TABLE_H */
