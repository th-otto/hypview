#include "hypdefs.h"
#include "hypdebug.h"

#undef G_N_ELEMENTS
#define G_N_ELEMENTS(t)  (sizeof(t) / sizeof(t[0]))

/*** ---------------------------------------------------------------------- ***/

/*
 * these happen to be the same names UDO uses
 */
static struct {
	const char *name;
	HYP_CHARSET id;
} const charset_names[] = {
   { "UTF-8",               HYP_CHARSET_UTF8     },
   { "utf8",                HYP_CHARSET_UTF8     },
   
   { "CP1250",              HYP_CHARSET_CP1250   },
   { "ms-ee",               HYP_CHARSET_CP1250   },
   { "windows-1250",        HYP_CHARSET_CP1250   },
   
   { "CP1251",              HYP_CHARSET_CP1251   },
   { "ms-cyrl",             HYP_CHARSET_CP1251   },
   { "windows-1251",        HYP_CHARSET_CP1251   },
   { "russian",             HYP_CHARSET_CP1251   },

   { "CP1252",              HYP_CHARSET_CP1252   },
   { "ms-ansi",             HYP_CHARSET_CP1252   },
   { "windows-1252",        HYP_CHARSET_CP1252   },
   { "WIN",                 HYP_CHARSET_CP1252   },
   { "iso",                 HYP_CHARSET_CP1252   }, /* deprecated */

   { "CP1253",              HYP_CHARSET_CP1253   },
   { "greek",               HYP_CHARSET_CP1253   },
   { "ms-greek",            HYP_CHARSET_CP1253   },
   { "windows-1253",        HYP_CHARSET_CP1253   },
   
   { "CP1254",              HYP_CHARSET_CP1254   },
   { "ms-turk",             HYP_CHARSET_CP1254   },
   { "turkish",             HYP_CHARSET_CP1254   },
   { "windows-1254",        HYP_CHARSET_CP1254   },

   { "CP1255",              HYP_CHARSET_CP1255   },
   { "hebrew" ,             HYP_CHARSET_CP1255   },
   { "ms-hebr",             HYP_CHARSET_CP1255   },
   { "windows-1255",        HYP_CHARSET_CP1255   },

   { "CP1256",              HYP_CHARSET_CP1256   },
   { "arabic" ,             HYP_CHARSET_CP1256   },
   { "ms-arab",             HYP_CHARSET_CP1256   },
   { "windows-1256",        HYP_CHARSET_CP1256   },

   { "CP1257",              HYP_CHARSET_CP1257   },
   { "baltic",              HYP_CHARSET_CP1257   },
   { "winbaltrim",          HYP_CHARSET_CP1257   },
   { "windows-1257",        HYP_CHARSET_CP1257   },

   { "CP1258",              HYP_CHARSET_CP1258   },
   { "windows-1258",        HYP_CHARSET_CP1258   },

   { "iso-8859-1",          HYP_CHARSET_LATIN1   },
   { "iso-ir-100",          HYP_CHARSET_LATIN1   },
   { "iso8859-1",           HYP_CHARSET_LATIN1   },
   { "iso_8859-1",          HYP_CHARSET_LATIN1   },
   { "latin1",              HYP_CHARSET_LATIN1   },
   { "l1",                  HYP_CHARSET_LATIN1   },
   { "csisolatin1",         HYP_CHARSET_LATIN1   },
   
   { "iso-8859-2",          HYP_CHARSET_LATIN2   },
   { "iso-ir-101",          HYP_CHARSET_LATIN2   },
   { "iso8859-2",           HYP_CHARSET_LATIN2   },
   { "iso_8859-2",          HYP_CHARSET_LATIN2   },
   { "latin2",              HYP_CHARSET_LATIN2   },
   { "l2",                  HYP_CHARSET_LATIN2   },
   { "csisolatin2",         HYP_CHARSET_LATIN2   },
   
   { "iso-8859-3",          HYP_CHARSET_LATIN3   },
   { "iso-ir-109",          HYP_CHARSET_LATIN3   },
   { "iso8859-3",           HYP_CHARSET_LATIN3   },
   { "iso_8859-3",          HYP_CHARSET_LATIN3   },
   { "latin3",              HYP_CHARSET_LATIN3   },
   { "l3",                  HYP_CHARSET_LATIN3   },
   { "csisolatin3",         HYP_CHARSET_LATIN3   },
   
   { "iso-8859-4",          HYP_CHARSET_LATIN4   },
   { "iso-ir-110",          HYP_CHARSET_LATIN4   },
   { "iso8859-4",           HYP_CHARSET_LATIN4   },
   { "iso_8859-4",          HYP_CHARSET_LATIN4   },
   { "latin4",              HYP_CHARSET_LATIN4   },
   { "l4",                  HYP_CHARSET_LATIN4   },
   { "csisolatin4",         HYP_CHARSET_LATIN4   },
   
   { "iso-8859-5",          HYP_CHARSET_CYRILLIC },
   { "iso-ir-144",          HYP_CHARSET_CYRILLIC },
   { "iso8859-5",           HYP_CHARSET_CYRILLIC },
   { "iso_8859-5",          HYP_CHARSET_CYRILLIC },
   { "cyrillic",            HYP_CHARSET_CYRILLIC },
   { "csisolatincyrillic",  HYP_CHARSET_CYRILLIC },
   
   { "iso-8859-6",          HYP_CHARSET_ARABIC   },
   { "iso-ir-127",          HYP_CHARSET_ARABIC   },
   { "iso8859-6",           HYP_CHARSET_ARABIC   },
   { "iso_8859-6",          HYP_CHARSET_ARABIC   },
   { "arabic",              HYP_CHARSET_ARABIC   },
   { "csisolatinarabic",    HYP_CHARSET_ARABIC   },
   { "asmo-708",            HYP_CHARSET_ARABIC   },
   { "ecma-114",            HYP_CHARSET_ARABIC   },
   
   { "iso-8859-7",          HYP_CHARSET_GREEK    },
   { "iso-ir-126",          HYP_CHARSET_GREEK    },
   { "iso8859-7",           HYP_CHARSET_GREEK    },
   { "iso_8859-7",          HYP_CHARSET_GREEK    },
   { "greek",               HYP_CHARSET_GREEK    },
   { "greek8",              HYP_CHARSET_GREEK    },
   { "csisolatingreek",     HYP_CHARSET_GREEK    },
   { "ecma-118",            HYP_CHARSET_GREEK    },
   { "elot_928",            HYP_CHARSET_GREEK    },
   
   { "iso-8859-8",          HYP_CHARSET_HEBREW   },
   { "iso-ir-138",          HYP_CHARSET_HEBREW   },
   { "iso8859-8",           HYP_CHARSET_HEBREW   },
   { "iso_8859-8",          HYP_CHARSET_HEBREW   },
   { "hebrew",              HYP_CHARSET_HEBREW   },
   { "csisolatinhebrew",    HYP_CHARSET_HEBREW   },
   
   { "iso-8859-9",          HYP_CHARSET_TURKISH  },
   { "iso-ir-148",          HYP_CHARSET_TURKISH  },
   { "iso8859-9",           HYP_CHARSET_TURKISH  },
   { "iso_8859-9",          HYP_CHARSET_TURKISH  },
   { "latin5",              HYP_CHARSET_TURKISH  },
   { "l5",                  HYP_CHARSET_TURKISH  },
   { "csisolatin5",         HYP_CHARSET_TURKISH  },
   { "turkish",             HYP_CHARSET_TURKISH  },
   
   { "iso-8859-10",         HYP_CHARSET_NORDIC   },
   { "iso-ir-157",          HYP_CHARSET_NORDIC   },
   { "iso8859-10",          HYP_CHARSET_NORDIC   },
   { "iso_8859-10",         HYP_CHARSET_NORDIC   },
   { "latin6",              HYP_CHARSET_NORDIC   },
   { "l6",                  HYP_CHARSET_NORDIC   },
   { "csisolatin6",         HYP_CHARSET_NORDIC   },
   { "nordic",              HYP_CHARSET_NORDIC   },
   
   { "iso-8859-11",         HYP_CHARSET_THAI     },
   { "iso8859-11",          HYP_CHARSET_THAI     },
   { "iso_8859-11",         HYP_CHARSET_THAI     },
   { "CP874",               HYP_CHARSET_CP874    },
   { "thai",                HYP_CHARSET_THAI     },
   
   { "iso-8859-13",         HYP_CHARSET_BALTIC   },
   { "iso-ir-179",          HYP_CHARSET_BALTIC   },
   { "iso8859-13",          HYP_CHARSET_BALTIC   },
   { "iso_8859-13",         HYP_CHARSET_BALTIC   },
   { "latin7",              HYP_CHARSET_BALTIC   },
   { "l7",                  HYP_CHARSET_BALTIC   },
   { "csisolatin7",         HYP_CHARSET_BALTIC   },
   { "baltic",              HYP_CHARSET_BALTIC   },
   
   { "iso-8859-14",         HYP_CHARSET_CELTIC   },
   { "iso-ir-199",          HYP_CHARSET_CELTIC   },
   { "iso8859-14",          HYP_CHARSET_CELTIC   },
   { "iso_8859-14",         HYP_CHARSET_CELTIC   },
   { "latin8",              HYP_CHARSET_CELTIC   },
   { "l8",                  HYP_CHARSET_CELTIC   },
   { "csisolatin8",         HYP_CHARSET_CELTIC   },
   { "iso-celtic",          HYP_CHARSET_CELTIC   },
   { "celtic",              HYP_CHARSET_CELTIC   },
   
   { "iso-8859-15",         HYP_CHARSET_LATIN9   },
   { "iso-ir-203",          HYP_CHARSET_LATIN9   },
   { "iso8859-15",          HYP_CHARSET_LATIN9   },
   { "iso_8859-15",         HYP_CHARSET_LATIN9   },
   { "latin9",              HYP_CHARSET_LATIN9   },
   { "l9",                  HYP_CHARSET_LATIN9   },
   { "csisolatin9",         HYP_CHARSET_LATIN9   },
   
   { "iso-8859-16",         HYP_CHARSET_LATIN10  },
   { "iso-ir-226",          HYP_CHARSET_LATIN10  },
   { "iso8859-16",          HYP_CHARSET_LATIN10  },
   { "iso_8859-16",         HYP_CHARSET_LATIN10  },
   { "latin10",             HYP_CHARSET_LATIN10  },
   { "l10",                 HYP_CHARSET_LATIN10  },
   { "csisolatin10",        HYP_CHARSET_LATIN10  },
   
   { "macroman",            HYP_CHARSET_MACROMAN },
   { "mac",                 HYP_CHARSET_MACROMAN },
   { "macintosh",           HYP_CHARSET_MACROMAN },
   { "csmacintosh",         HYP_CHARSET_MACROMAN },
   
   { "mac_ce",              HYP_CHARSET_MAC_CE   },
   { "maccentraleurope",    HYP_CHARSET_MAC_CE   },
   
   { "atarist",             HYP_CHARSET_ATARI    },
   { "tos",                 HYP_CHARSET_ATARI    }, /* non-standard iconv name(s); iconv does not know about Atari codepage */
   { "atari",               HYP_CHARSET_ATARI    },
   
   { "russian-atarist",     HYP_CHARSET_ATARI_RU },
   { "russianst",           HYP_CHARSET_ATARI_RU },

   { "437",                 HYP_CHARSET_CP437    },
   { "cp437",               HYP_CHARSET_CP437    },
   { "ibm437",              HYP_CHARSET_CP437    },
   { "cspc8codepage437",    HYP_CHARSET_CP437    },
   { "dos",                 HYP_CHARSET_CP437    }, /* UDO Version 6.xx compatibility, deprecated */
   
   { "850",                 HYP_CHARSET_CP850    },
   { "cp850",               HYP_CHARSET_CP850    },
   { "ibm850",              HYP_CHARSET_CP850    },
   { "cspc850multilingual", HYP_CHARSET_CP850    },
   { "os2",                 HYP_CHARSET_CP850    }, /* UDO Version 6.xx compatibility, deprecated */
   
   { "hp8",                 HYP_CHARSET_HP8      }, /* UDO Version 6.xx compatibility, deprecated */
   { "hp-roman8",           HYP_CHARSET_HP8      },
   { "r8",                  HYP_CHARSET_HP8      },
   { "roman8",              HYP_CHARSET_HP8      },
   { "cshproman8",          HYP_CHARSET_HP8      },
   
   { "next",                HYP_CHARSET_NEXT     },
   { "nextstep",            HYP_CHARSET_NEXT     }
};

const char *hyp_charset_name(HYP_CHARSET charset)
{
	static char ret[20];
	unsigned int i;
	
	if (charset == HYP_CHARSET_NONE)
		return _("Unknown");
	for (i = 0; i < G_N_ELEMENTS(charset_names); i++)
		if (charset_names[i].id == charset)
			return charset_names[i].name;
	sprintf(ret, _("Charset 0x%02x"), charset);
	return ret;
}

/*** ---------------------------------------------------------------------- ***/

HYP_CHARSET hyp_charset_from_name(const char *name)
{
	unsigned int i;
	
	for (i = 0; i < G_N_ELEMENTS(charset_names); i++)
		if (g_ascii_strcasecmp(name, charset_names[i].name) == 0)
			return charset_names[i].id;
	return HYP_CHARSET_NONE;
}
