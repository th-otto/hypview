/*
 * Character set conversion functions.
 * Theoretically, this could be done with
 * g_convert() or iconv(), but we don't do that for several reasons:
 * - the libraries might not be available, and have a rather large overhead
 * - neither Glib nor iconv() know about AtariST character encoding
 * - we only need 3 well known character encodings here
 * - on Atari, we usually don't need any conversion at all.
 */

#include "hypdefs.h"
#include "hypdebug.h"

#include "cp_atari.h"
#include "cp_850.h"
#include "cp_mac.h"
#include "cp_1252.h"

#ifndef HAVE_GLIB
#include "gunichar.h"

#define G_N_ELEMENTS(t)  (sizeof(t) / sizeof(t[0]))


#define ATTR_TABLE(Page) (((Page) <= G_UNICODE_LAST_PAGE_PART1) \
                          ? attr_table_part1[Page] \
                          : attr_table_part2[(Page) - 0xe00])

#define ATTTABLE(Page, Char) \
  ((ATTR_TABLE(Page) == G_UNICODE_MAX_TABLE_INDEX) ? 0 : (attr_data[ATTR_TABLE(Page)][Char]))

#define TTYPE_PART1(Page, Char) \
  ((type_table_part1[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part1[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part1[Page]][Char]))

#define TTYPE_PART2(Page, Char) \
  ((type_table_part2[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part2[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part2[Page]][Char]))

#define TYPE(Char) \
  (((Char) <= G_UNICODE_LAST_CHAR_PART1) \
   ? TTYPE_PART1 ((Char) >> 8, (Char) & 0xff) \
   : (((Char) >= 0xe0000UL && (Char) <= G_UNICODE_LAST_CHAR) \
      ? TTYPE_PART2 (((Char) - 0xe0000UL) >> 8, (Char) & 0xff) \
      : G_UNICODE_UNASSIGNED))


#define IS(Type, Class)	(((guint)1 << (Type)) & (Class))
#define OR(Type, Rest)	(((guint)1 << (Type)) | (Rest))



#define ISALPHA(Type)	IS ((Type),				\
			    OR (G_UNICODE_LOWERCASE_LETTER,	\
			    OR (G_UNICODE_UPPERCASE_LETTER,	\
			    OR (G_UNICODE_TITLECASE_LETTER,	\
			    OR (G_UNICODE_MODIFIER_LETTER,	\
			    OR (G_UNICODE_OTHER_LETTER,		0))))))

#define ISALDIGIT(Type)	IS ((Type),				\
			    OR (G_UNICODE_DECIMAL_NUMBER,	\
			    OR (G_UNICODE_LETTER_NUMBER,	\
			    OR (G_UNICODE_OTHER_NUMBER,		\
			    OR (G_UNICODE_LOWERCASE_LETTER,	\
			    OR (G_UNICODE_UPPERCASE_LETTER,	\
			    OR (G_UNICODE_TITLECASE_LETTER,	\
			    OR (G_UNICODE_MODIFIER_LETTER,	\
			    OR (G_UNICODE_OTHER_LETTER,		0)))))))))

#define ISMARK(Type)	IS ((Type),				\
			    OR (G_UNICODE_NON_SPACING_MARK,	\
			    OR (G_UNICODE_SPACING_MARK,	\
			    OR (G_UNICODE_ENCLOSING_MARK,	0))))

#define ISZEROWIDTHTYPE(Type)	IS ((Type),			\
			    OR (G_UNICODE_NON_SPACING_MARK,	\
			    OR (G_UNICODE_ENCLOSING_MARK,	\
			    OR (G_UNICODE_FORMAT,		0))))

#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

HYP_CHARSET hyp_default_charset(HYP_OS os)
{
	switch (os)
	{
	case HYP_OS_UNKNOWN:
		/*
		 * can happen if
		 * - type 0 was expclicitly set in HYP/REF file
		 * - no OS entry was specified in REF file
		 * - a Node/Alias/Label was found in REF before any OS entry
		 * Should maybe use some heuristic to determine real characterset,
		 * but for now just assumes Atari.
		 */
	case HYP_OS_ATARI:
		return HYP_CHARSET_ATARI;
	case HYP_OS_AMIGA:
		return HYP_CHARSET_CP850;
	case HYP_OS_MAC:
		return HYP_CHARSET_MACROMAN;
	case HYP_OS_WIN32:
		return HYP_CHARSET_CP1252;
	case HYP_OS_UNIX:
		return HYP_CHARSET_UTF8;
	default:
		/*
		 * this should not happen at all
		 */
		break;
	}
	return HYP_CHARSET_UTF8;
}

/*** ---------------------------------------------------------------------- ***/

HYP_CHARSET hyp_get_current_charset(void)
{
	return hyp_default_charset(hyp_get_current_os());
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Return the charset used to translate filenames
 * when accessing them. Not necessarily the same
 * as hyp_get_current_charset() (which is
 * used to display it in messages).
 */
HYP_CHARSET hyp_get_filename_charset(void)
{
	/*
	 * for now, use the same.
	 * May have to investigate locale() functions
	 */
	return hyp_get_current_charset();
}

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
   { "csisolatin8",         HYP_CHARSET_BALTIC   },
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

/*** ---------------------------------------------------------------------- ***/

static const h_unichar_t *get_cset(HYP_CHARSET charset)
{
	switch (charset)
	{
	case HYP_CHARSET_NONE:
	case HYP_CHARSET_ATARI:
		return atari_to_unicode;
	case HYP_CHARSET_CP850:
		return cp850_to_unicode;
	case HYP_CHARSET_MACROMAN:
		return macroman_to_unicode;
	case HYP_CHARSET_CP1252:
		return cp1252_to_unicode;
	case HYP_CHARSET_UTF8:
		/* not an error, we use utf8 here directly */
		return NULL;
	default:
		/*
		 * this should not happen at all
		 */
		break;
	}
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

#define hyp_put_unichar(p, wc) \
	if (wc < 0x80) \
	{ \
		*p++ = wc; \
	} else if (wc < 0x800) \
	{ \
		p[1] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[0] = wc | 0xc0; \
		p += 2; \
	} else if (wc < 0x10000UL) \
	{ \
		p[2] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[1] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[0] = wc | 0xe0; \
		p += 3; \
	} else if (wc < 0x200000UL) \
	{ \
		p[3] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[2] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[1] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[0] = wc | 0xf0; \
		p += 4; \
	} else if (wc < 0x4000000UL) \
	{ \
		p[4] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[3] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[2] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[1] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[0] = wc | 0xf8; \
		p += 5; \
	} else \
	{ \
		p[5] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[4] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[3] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[2] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[1] = (wc & 0x3f) | 0x80; \
		wc >>= 6; \
		p[0] = wc | 0xfc; \
		p += 6; \
	}

/*** ---------------------------------------------------------------------- ***/

char *hyp_conv_to_utf8(HYP_CHARSET charset, const void *src, size_t len)
{
	char *dst;
	const h_unichar_t *cset;
	size_t i;
	
	if (src == NULL)
		return NULL;
	if (len == STR0TERM)
		len = strlen((const char *) src);
	cset = get_cset(charset);
	if (cset != NULL)
	{
		const unsigned char *ptr;
		char *p;
		h_unichar_t wc;
		
		ptr = (const unsigned char *)src;
		
		dst = p = g_new(char, len * HYP_UTF8_CHARMAX + 1);
		if (dst != NULL)
		{
			ptr = (const unsigned char *)src;
			for (i = 0; i < len; i++)
			{
				wc = cset[*ptr++];
				hyp_put_unichar(p, wc);
			}
			*p++ = '\0';
			dst = g_renew(char, dst, p - dst);
		}
	} else
	{
		dst = g_strndup((const char *)src, len);
	}
	return dst;
}

/*** ---------------------------------------------------------------------- ***/

#define CONTINUATION_CHAR                           \
  if ((*(const unsigned char *)p & 0xc0) != 0x80) /* 10xxxxxx */ \
    goto error;                                     \
  val <<= 6;                                        \
  val |= (*(const unsigned char *)p) & 0x3f

#define UNICODE_VALID(Char)                   \
    ((Char) < 0x110000UL &&                     \
     (((Char) & 0xFFFFF800UL) != 0xD800UL) &&     \
     ((Char) < 0xFDD0UL || (Char) > 0xFDEFUL) &&  \
     ((Char) & 0xFFFEUL) != 0xFFFEUL)

static const char *hyp_utf8_getchar(const char *p, h_unichar_t *ch)
{
	const char *last;

	if (*(const unsigned char *) p < 0x80)
	{
		*ch = *(const unsigned char *) p;
		return p + 1;
	}
	last = p;
	if ((*(const unsigned char *) p & 0xe0) == 0xc0)	/* 110xxxxx */
	{
		if (G_UNLIKELY((*(const unsigned char *) p & 0x1e) == 0))
			goto error;
		*ch = (*(const unsigned char *) p & 0x1f) << 6;
		p++;
		if (G_UNLIKELY((*(const unsigned char *) p & 0xc0) != 0x80))	/* 10xxxxxx */
			goto error;
		*ch |= (*(const unsigned char *) p) & 0x3f;
	} else
	{
		h_unichar_t val = 0;
		h_unichar_t min = 0;
		
		if ((*(const unsigned char *) p & 0xf0) == 0xe0)	/* 1110xxxx */
		{
			min = (1 << 11);
			val = *(const unsigned char *) p & 0x0f;
			goto TWO_REMAINING;
		} else if ((*(const unsigned char *) p & 0xf8) == 0xf0)	/* 11110xxx */
		{
			min = ((h_unichar_t)1 << 16);
			val = *(const unsigned char *) p & 0x07;
		} else
		{
			goto error;
		}
		
		p++;
		CONTINUATION_CHAR;
	  TWO_REMAINING:
		p++;
		CONTINUATION_CHAR;
		p++;
		CONTINUATION_CHAR;

		if (G_UNLIKELY(val < min))
			goto error;

		if (G_UNLIKELY(!UNICODE_VALID(val)))
			goto error;
		*ch = val;
	}

	return p + 1;

  error:
    *ch = 0xffff;
	return last + 1;
}

/*** ---------------------------------------------------------------------- ***/

#include "casefold.h"

char *hyp_utf8_casefold(const char *str, size_t len)
{
	char *result;
	char *dst;
	const char *p, *end;

	if (str == NULL)
		return NULL;

	if (len == STR0TERM)
		len = strlen(str);
	result = dst = g_new(char, len * HYP_UTF8_CHARMAX + 1);
	if (result == NULL)
		return NULL;
	p = str;
	end = str + len;
	while (p < end)
	{
		h_unichar_t ch;
		size_t start, end, half;
		
		start = 0;
		end = sizeof(casefold_table) / sizeof(casefold_table[0]);
		
		p = hyp_utf8_getchar(p, &ch);
		if (ch >= casefold_table[start].ch && ch <= casefold_table[end - 1].ch)
		{
			for (;;)
			{
				half = (start + end) / 2;

				if (ch == casefold_table[half].ch)
				{
					strcpy(dst, casefold_table[half].data);
					dst += strlen(casefold_table[half].data);
					goto next;
				} else if (half == start)
					break;
				else if (ch > casefold_table[half].ch)
					start = half;
				else
					end = half;
			}
		}

		ch = g_unichar_tolower(ch);
		hyp_put_unichar(dst, ch);

	  next:
		;
	}
	
	*dst++ = '\0';
	return g_renew(char, result, dst - result);
}

/*** ---------------------------------------------------------------------- ***/

int g_utf8_strcasecmp(const char *s1, const char *s2)
{
	h_unichar_t c1, c2;
	
	while (*s1 && *s2)
	{
		s1 = hyp_utf8_getchar(s1, &c1);
		s2 = hyp_utf8_getchar(s2, &c2);
		c1 = g_unichar_tolower(c1);
		c2 = g_unichar_tolower(c2);
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
	}
	
	return (((int) (unsigned char) *s1) - ((int) (unsigned char) *s2));
}

/*** ---------------------------------------------------------------------- ***/

int g_utf8_strncasecmp(const char *s1, const char *s2, size_t n)
{
	h_unichar_t c1, c2;
	const char *end = s2 + n;
	
	while (s2 < end && *s1 && *s2)
	{
		s1 = hyp_utf8_getchar(s1, &c1);
		s2 = hyp_utf8_getchar(s2, &c2);
		c1 = g_unichar_tolower(c1);
		c2 = g_unichar_tolower(c2);
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
	}
	
	if (s2 < end)
		return (((int) (unsigned char) *s1) - ((int) (unsigned char) *s2));
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Like strcasecmp(), using two strings in HYP encoding
 */
int hyp_name_cmp(HYP_CHARSET charset, const unsigned char *str1, const unsigned char *str2)
{
	char *s1, *s2;
	int res;
	
	if (charset == HYP_CHARSET_UTF8)
	{
		res = g_utf8_strcasecmp((const char *) str1, (const char *) str2);
	} else if (get_cset(charset) != NULL)
	{
		s1 = hyp_conv_to_utf8(charset, str1, STR0TERM);
		s2 = hyp_conv_to_utf8(charset, str2, STR0TERM);
		res = g_utf8_strcasecmp(s1, s2);
		g_free(s2);
		g_free(s1);
	} else
	{
		res = strcasecmp((const char *)str1, (const char *)str2);
	}
	return res;
}

/*** ---------------------------------------------------------------------- ***/

const char *g_utf8_skipchar(const char *p)
{
	const char *last;

	if (*(const unsigned char *) p < 0x80)
		return p + 1;
	last = p;
	if ((*(const unsigned char *) p & 0xe0) == 0xc0)	/* 110xxxxx */
	{
		if (G_UNLIKELY((*(const unsigned char *) p & 0x1e) == 0))
			goto error;
		p++;
		if (G_UNLIKELY((*(const unsigned char *) p & 0xc0) != 0x80))	/* 10xxxxxx */
			goto error;
	} else
	{
		h_unichar_t val = 0;
		h_unichar_t min = 0;
		
		if ((*(const unsigned char *) p & 0xf0) == 0xe0)	/* 1110xxxx */
		{
			min = (1 << 11);
			val = *(const unsigned char *) p & 0x0f;
			goto TWO_REMAINING;
		} else if ((*(const unsigned char *) p & 0xf8) == 0xf0)	/* 11110xxx */
		{
			min = ((h_unichar_t)1 << 16);
			val = *(const unsigned char *) p & 0x07;
		} else
		{
			goto error;
		}
		
		p++;
		CONTINUATION_CHAR;
	  TWO_REMAINING:
		p++;
		CONTINUATION_CHAR;
		p++;
		CONTINUATION_CHAR;

		if (G_UNLIKELY(val < min))
			goto error;

		if (G_UNLIKELY(!UNICODE_VALID(val)))
			goto error;
	}

	return p + 1;

  error:
	return last + 1;
}

/*** ---------------------------------------------------------------------- ***/

h_unichar_t hyp_utf8_get_char(const char *_p)
{
	const unsigned char *p = (const unsigned char *)_p;
	h_unichar_t ch;
	
	if (*p < 0x80)
	{
		return *p;
	}
	if ((*p & 0xe0) == 0xc0)	/* 110xxxxx */
	{
		if (G_UNLIKELY((*p & 0x1e) == 0))
			goto error;
		ch = (*p & 0x1f) << 6;
		p++;
		if (G_UNLIKELY((*p & 0xc0) != 0x80))	/* 10xxxxxx */
			goto error;
		ch |= (*p) & 0x3f;
	} else
	{
		h_unichar_t val = 0;
		h_unichar_t min = 0;
		
		if ((*p & 0xf0) == 0xe0)	/* 1110xxxx */
		{
			min = (1 << 11);
			val = *p & 0x0f;
			goto TWO_REMAINING;
		} else if ((*p & 0xf8) == 0xf0)	/* 11110xxx */
		{
			min = ((h_unichar_t)1 << 16);
			val = *p & 0x07;
		} else
		{
			goto error;
		}
		
		p++;
		CONTINUATION_CHAR;
	  TWO_REMAINING:
		p++;
		CONTINUATION_CHAR;
		p++;
		CONTINUATION_CHAR;

		if (G_UNLIKELY(val < min))
			goto error;

		if (G_UNLIKELY(!UNICODE_VALID(val)))
			goto error;
		ch = val;
	}

	return ch;

  error:
    return 0xffff;
}

/*** ---------------------------------------------------------------------- ***/

#ifndef HAVE_GLIB
h_unichar_t g_unichar_tolower(h_unichar_t c)
{
	int t = TYPE(c);

	if (t == G_UNICODE_UPPERCASE_LETTER)
	{
		h_unichar_t val = ATTTABLE(c >> 8, c & 0xff);

		if (val >= 0x1000000UL)
		{
			const char *p = special_case_table + val - 0x1000000UL;

			return hyp_utf8_get_char(p);
		} else
		{
			/* Not all uppercase letters are guaranteed to have a lowercase
			 * equivalent.  If this is the case, val will be zero. */
			return val ? val : c;
		}
	} else if (t == G_UNICODE_TITLECASE_LETTER)
	{
		unsigned int i;

		for (i = 0; i < G_N_ELEMENTS(title_table); ++i)
		{
			if (title_table[i][0] == c)
				return title_table[i][2];
		}
	}
	return c;
}
#endif

/*** ---------------------------------------------------------------------- ***/

#ifndef HAVE_GLIB
h_unichar_t g_unichar_toupper(h_unichar_t c)
{
	int t = TYPE(c);
	
	if (t == G_UNICODE_LOWERCASE_LETTER)
	{
		h_unichar_t val = ATTTABLE(c >> 8, c & 0xff);
		if (val >= 0x1000000UL)
		{
			const char *p = special_case_table + val - 0x1000000UL;
			val = hyp_utf8_get_char(p);
		}
		/* Some lowercase letters, e.g., U+000AA, FEMININE ORDINAL INDICATOR,
		 * do not have an uppercase equivalent, in which case val will be
		 * zero. 
		 */
		return val ? val : c;
	} else if (t == G_UNICODE_TITLECASE_LETTER)
	{
		unsigned int i;
		for (i = 0; i < G_N_ELEMENTS(title_table); ++i)
		{
			if (title_table[i][0] == c)
				return title_table[i][1] ? title_table[i][1] : c;
		}
	}
	return c;
}
#endif

/*** ---------------------------------------------------------------------- ***/

#ifndef HAVE_GLIB
gboolean g_unichar_isupper(h_unichar_t c)
{
	return TYPE(c) == G_UNICODE_UPPERCASE_LETTER;
}
#endif

/*** ---------------------------------------------------------------------- ***/

#ifndef HAVE_GLIB
gboolean g_unichar_islower(gunichar c)
{
	return TYPE(c) == G_UNICODE_LOWERCASE_LETTER;
}
#endif

/* ------------------------------------------------------------------------- */

const char *g_utf8_strcasestr(const char *searchee, const char *lookfor)
{
	h_unichar_t ch1, ch2;
	
	while (*searchee)
	{
		const char *scan1 = lookfor;
		const char *scan2 = searchee;
		for (;;)
		{
			if (*scan1 == 0)
				return searchee;
			
			ch1 = g_unichar_tolower(hyp_utf8_get_char(scan1));
			ch2 = g_unichar_tolower(hyp_utf8_get_char(scan2));
			if (ch1 != ch2)
				break;
			scan1 = g_utf8_skipchar(scan1);
			scan2 = g_utf8_skipchar(scan2);
		}
		searchee = g_utf8_skipchar(searchee);
	}

	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

size_t g_utf8_str_len(const char *p, size_t len)
{
	size_t l = 0;
	const char *end;
	
	if (p == NULL)
		return l;
	if (len == STR0TERM)
		len = strlen(p);
	end = p + len;
	while (p < end && *p)
	{
		p = g_utf8_skipchar(p);
		l++;
	}
	return l;
}

/*** ---------------------------------------------------------------------- ***/

static char *hyp_conv_to_atari(const char *src, size_t len, gboolean *converror)
{
	const char *end = src + len;
	h_unichar_t ch;
	const char *p;
	size_t dstlen = 1;
	char *dst;
	
	p = src;
	while (p < end)
	{
		p = g_utf8_skipchar(p);
		dstlen++;
	}
	dst = g_malloc(dstlen);
	if (G_UNLIKELY(dst == NULL))
		return NULL;
	dstlen = 0;
	p = src;
	while (p < end)
	{
		p = hyp_utf8_getchar(p, &ch);
		dst[dstlen] = (*utf16_to_atari[ch >> 8])[ch & 0xff];
		if ((unsigned char)dst[dstlen] == 0xff && atari_to_unicode[0xff] != ch)
			if (converror)
				*converror = TRUE;
		dstlen++;
	}
	dst[dstlen] = '\0';
	return dst;
}

/*** ---------------------------------------------------------------------- ***/

static char *hyp_conv_to_cp850(const char *src, size_t len, gboolean *converror)
{
	const char *end = src + len;
	h_unichar_t ch;
	const char *p;
	size_t dstlen = 1;
	char *dst;
	
	p = src;
	while (p < end)
	{
		p = g_utf8_skipchar(p);
		dstlen++;
	}
	dst = g_malloc(dstlen);
	if (G_UNLIKELY(dst == NULL))
		return NULL;
	dstlen = 0;
	p = src;
	while (p < end)
	{
		p = hyp_utf8_getchar(p, &ch);
		dst[dstlen] = (*utf16_to_cp850[ch >> 8])[ch & 0xff];
		if ((unsigned char)dst[dstlen] == 0xff && cp850_to_unicode[0xff] != ch)
			if (converror)
				*converror = TRUE;
		dstlen++;
	}
	dst[dstlen] = '\0';
	return dst;
}

/*** ---------------------------------------------------------------------- ***/

static char *hyp_conv_to_macroman(const char *src, size_t len, gboolean *converror)
{
	const char *end = src + len;
	h_unichar_t ch;
	const char *p;
	size_t dstlen = 1;
	char *dst;
	
	p = src;
	while (p < end)
	{
		p = g_utf8_skipchar(p);
		dstlen++;
	}
	dst = g_malloc(dstlen);
	if (G_UNLIKELY(dst == NULL))
		return NULL;
	dstlen = 0;
	p = src;
	while (p < end)
	{
		p = hyp_utf8_getchar(p, &ch);
		dst[dstlen] = (*utf16_to_macroman[ch >> 8])[ch & 0xff];
		if ((unsigned char)dst[dstlen] == 0xff && macroman_to_unicode[0xff] != ch)
			if (converror)
				*converror = TRUE;
		dstlen++;
	}
	dst[dstlen] = '\0';
	return dst;
}

/*** ---------------------------------------------------------------------- ***/

static char *hyp_conv_to_cp1252(const char *src, size_t len, gboolean *converror)
{
	const char *end = src + len;
	h_unichar_t ch;
	const char *p;
	size_t dstlen = 1;
	char *dst;
	
	p = src;
	while (p < end)
	{
		p = g_utf8_skipchar(p);
		dstlen++;
	}
	dst = g_malloc(dstlen);
	if (G_UNLIKELY(dst == NULL))
		return NULL;
	dstlen = 0;
	p = src;
	while (p < end)
	{
		p = hyp_utf8_getchar(p, &ch);
		dst[dstlen] = (*utf16_to_cp1252[ch >> 8])[ch & 0xff];
		if ((unsigned char)dst[dstlen] == 0xff && cp1252_to_unicode[0xff] != ch)
			if (converror)
				*converror = TRUE;
		dstlen++;
	}
	dst[dstlen] = '\0';
	return dst;
}

/*** ---------------------------------------------------------------------- ***/

char *hyp_utf8_to_charset(HYP_CHARSET charset, const void *src, size_t len, gboolean *converror)
{
	if (src == NULL)
		return NULL;
	if (len == STR0TERM)
		len = strlen((const char *) src);
	switch (charset)
	{
	case HYP_CHARSET_UTF8:
		return g_strndup((const char *)src, len);
	case HYP_CHARSET_ATARI:
		return hyp_conv_to_atari((const char *)src, len, converror);
	case HYP_CHARSET_CP850:
		return hyp_conv_to_cp850((const char *)src, len, converror);
	case HYP_CHARSET_MACROMAN:
		return hyp_conv_to_macroman((const char *)src, len, converror);
	case HYP_CHARSET_CP1252:
		return hyp_conv_to_cp1252((const char *)src, len, converror);
	case HYP_CHARSET_NONE:
		break;
	}
	return g_strndup((const char *)src, len);
}

/*** ---------------------------------------------------------------------- ***/

const char *hyp_utf8_conv_char(HYP_CHARSET charset, const char *src, char *buf, gboolean *converror)
{
	h_unichar_t ch;
	const char *end;
	size_t len;
	
	switch (charset)
	{
	case HYP_CHARSET_UTF8:
		end = g_utf8_skipchar(src);
		len = end - src;
		memcpy(buf, src, len);
		buf[len] = '\0';
		return end;
	case HYP_CHARSET_ATARI:
		end = hyp_utf8_getchar(src, &ch);
		buf[0] = (*utf16_to_atari[ch >> 8])[ch & 0xff];
		if ((unsigned char)buf[0] == 0xff && atari_to_unicode[0xff] != ch)
			if (converror)
				*converror = TRUE;
		buf[1] = '\0';
		return end;
	case HYP_CHARSET_CP850:
		end = hyp_utf8_getchar(src, &ch);
		buf[0] = (*utf16_to_cp850[ch >> 8])[ch & 0xff];
		if ((unsigned char)buf[0] == 0xff && cp850_to_unicode[0xff] != ch)
			if (converror)
				*converror = TRUE;
		buf[1] = '\0';
		return end;
	case HYP_CHARSET_MACROMAN:
		end = hyp_utf8_getchar(src, &ch);
		buf[0] = (*utf16_to_macroman[ch >> 8])[ch & 0xff];
		if ((unsigned char)buf[0] == 0xff && macroman_to_unicode[0xff] != ch)
			if (converror)
				*converror = TRUE;
		buf[1] = '\0';
		return end;
	case HYP_CHARSET_CP1252:
		end = hyp_utf8_getchar(src, &ch);
		buf[0] = (*utf16_to_cp1252[ch >> 8])[ch & 0xff];
		if ((unsigned char)buf[0] == 0xff && cp1252_to_unicode[0xff] != ch)
			if (converror)
				*converror = TRUE;
		buf[1] = '\0';
		return end;
	case HYP_CHARSET_NONE:
		break;
	}
	*buf++ = *src++;
	*buf = '\0';
	return src;
}

/*** ---------------------------------------------------------------------- ***/

char *hyp_conv_charset(HYP_CHARSET from, HYP_CHARSET to, const void *src, size_t len, gboolean *converror)
{
	char *tmp, *res;
	
	if (src == NULL)
		return NULL;
	if (len == STR0TERM)
		len = strlen((const char *) src);
	if (from == to)
		return g_strndup((const char *)src, len);
	switch (from)
	{
	case HYP_CHARSET_UTF8:
		return hyp_utf8_to_charset(to, src, len, converror);
	case HYP_CHARSET_ATARI:
		switch (to)
		{
		case HYP_CHARSET_UTF8:
			return hyp_conv_to_utf8(from, src, len);
		case HYP_CHARSET_ATARI:
			break;
		case HYP_CHARSET_CP850:
		case HYP_CHARSET_MACROMAN:
		case HYP_CHARSET_CP1252:
			tmp = hyp_conv_to_utf8(from, src, len);
			res = hyp_utf8_to_charset(to, tmp, strlen(tmp), converror);
			g_free(tmp);
			return res;
		case HYP_CHARSET_NONE:
			break;
		}
		break;
	case HYP_CHARSET_CP850:
		switch (to)
		{
		case HYP_CHARSET_UTF8:
			return hyp_conv_to_utf8(from, src, len);
		case HYP_CHARSET_CP850:
			break;
		case HYP_CHARSET_ATARI:
		case HYP_CHARSET_MACROMAN:
		case HYP_CHARSET_CP1252:
			tmp = hyp_conv_to_utf8(from, src, len);
			res = hyp_utf8_to_charset(to, tmp, strlen(tmp), converror);
			g_free(tmp);
			return res;
		case HYP_CHARSET_NONE:
			break;
		}
		break;
	case HYP_CHARSET_MACROMAN:
		switch (to)
		{
		case HYP_CHARSET_UTF8:
			return hyp_conv_to_utf8(from, src, len);
		case HYP_CHARSET_MACROMAN:
			break;
		case HYP_CHARSET_CP850:
		case HYP_CHARSET_ATARI:
		case HYP_CHARSET_CP1252:
			tmp = hyp_conv_to_utf8(from, src, len);
			res = hyp_utf8_to_charset(to, tmp, strlen(tmp), converror);
			g_free(tmp);
			return res;
		case HYP_CHARSET_NONE:
			break;
		}
		break;
	case HYP_CHARSET_CP1252:
		switch (to)
		{
		case HYP_CHARSET_UTF8:
			return hyp_conv_to_utf8(from, src, len);
		case HYP_CHARSET_CP1252:
			break;
		case HYP_CHARSET_MACROMAN:
		case HYP_CHARSET_CP850:
		case HYP_CHARSET_ATARI:
			tmp = hyp_conv_to_utf8(from, src, len);
			res = hyp_utf8_to_charset(to, tmp, strlen(tmp), converror);
			g_free(tmp);
			return res;
		case HYP_CHARSET_NONE:
			break;
		}
		break;
	case HYP_CHARSET_NONE:
		break;
	}
	return g_strndup((const char *)src, len);
}

/*** ---------------------------------------------------------------------- ***/

wchar_t *hyp_utf8_to_wchar(const char *str, size_t len, size_t *lenp)
{
	size_t wlen;
	wchar_t *dst, *p;
	h_unichar_t ch;
	const char *end;
	
	if (str == NULL)
		return NULL;
	if (len == STR0TERM)
		len = strlen(str);
	wlen = g_utf8_str_len(str, len);
	dst = g_new(wchar_t, wlen + 1);
	if (G_UNLIKELY(dst == NULL))
		return NULL;
	end = str + len;
	p = dst;
	while (str < end)
	{
		str = hyp_utf8_getchar(str, &ch);
		*p++ = (wchar_t) ch;
	}
	*p = 0;
	if (lenp)
		*lenp = wlen;
	return dst;
}

/*** ---------------------------------------------------------------------- ***/

size_t hyp_unichar_to_utf8(char *buf, h_unichar_t wc)
{
	char *p = buf;
	hyp_put_unichar(p, wc);
	*p = '\0';
	return p - buf;
}

/*** ---------------------------------------------------------------------- ***/

char *hyp_wchar_to_utf8(const wchar_t *str, size_t wlen)
{
	size_t len;
	char *dst, *p;
	h_unichar_t wc;
	
	if (str == NULL)
		return NULL;
	if (wlen == STR0TERM)
		wlen = wcslen(str);
	len = wlen * HYP_UTF8_CHARMAX + 1;
	dst = p = g_new(char, len);
	if (G_UNLIKELY(dst == NULL))
		return NULL;
	while (wlen)
	{
		wc = *str++;
		hyp_put_unichar(p, wc);
		wlen--;
	}
	*p++ = '\0';
	return g_renew(char, dst, p - dst);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * printf-like functions for printing utf8 input in a specified charset
 */
int hyp_utf8_vfprintf_charset(FILE *fp, HYP_CHARSET charset, const char *format, va_list args)
{
	char *str;
	char *dst;
	int res;
	gboolean converror = FALSE;

	if (fp == NULL)
		return EOF;
	
#ifdef __WIN32__
	/*
	 * Another win32 nuisance: when writing to the console,
	 * we have to use WriteConsoleW. For this to work,
	 * the console has to be in UNICODE mode.
	 */
	{
		size_t len;
		DWORD mode;
		wchar_t *w;
		UINT cpout;
		DWORD written;
		HANDLE cons;
		
		cons = (HANDLE)(DWORD)_get_osfhandle(fileno(fp));
		res = GetConsoleMode(cons, &mode);
		if (res)
		{
			str = g_strdup_vprintf(format, args);
			len = strlen(str);
			w = hyp_utf8_to_wchar(str, STR0TERM, &len);
			g_free(str);
			cpout = GetConsoleOutputCP();
			SetConsoleOutputCP(CP_UTF16_LE);
			if (WriteConsoleW(cons, w, len, &written, NULL))
			{
				res = (int)written;
			} else
			{
				errno = win32_to_errno(GetLastError());
				res = EOF;
			}
			g_free(w);
			SetConsoleOutputCP(cpout);
			return res;
		}
	}
#endif

	switch (charset)
	{
	case HYP_CHARSET_UTF8:
		res = vfprintf(fp, format, args);
		break;
	case HYP_CHARSET_ATARI:
		str = g_strdup_vprintf(format, args);
		if (str == NULL)
			return EOF;
		dst = hyp_conv_to_atari(str, strlen(str), &converror);
		if (dst == NULL)
			res = EOF;
		else
			res = fputs(dst, fp);
		g_free(dst);
		g_free(str);
		break;
	case HYP_CHARSET_CP850:
		str = g_strdup_vprintf(format, args);
		if (str == NULL)
			return EOF;
		dst = hyp_conv_to_cp850(str, strlen(str), &converror);
		if (dst == NULL)
			res = EOF;
		else
			res = fputs(dst, fp);
		g_free(dst);
		g_free(str);
		break;
	case HYP_CHARSET_MACROMAN:
		str = g_strdup_vprintf(format, args);
		if (str == NULL)
			return EOF;
		dst = hyp_conv_to_macroman(str, strlen(str), &converror);
		if (dst == NULL)
			res = EOF;
		else
			res = fputs(dst, fp);
		g_free(dst);
		g_free(str);
		break;
	case HYP_CHARSET_CP1252:
		str = g_strdup_vprintf(format, args);
		if (str == NULL)
			return EOF;
		dst = hyp_conv_to_cp1252(str, strlen(str), &converror);
		if (dst == NULL)
			res = EOF;
		else
			res = fputs(dst, fp);
		g_free(dst);
		g_free(str);
		break;
	case HYP_CHARSET_NONE:
	default:
		res = vfprintf(fp, format, args);
		break;
	}
	return res;
}

/*** ---------------------------------------------------------------------- ***/

int hyp_utf8_printf_charset(HYP_CHARSET charset, const char *format, ...)
{
	va_list args;
	int res;
	
	va_start(args, format);
	res = hyp_utf8_vfprintf_charset(stdout, charset, format, args);
	va_end(args);
	return res;
}

/*** ---------------------------------------------------------------------- ***/

int hyp_utf8_fprintf_charset(FILE *fp, HYP_CHARSET charset, const char *format, ...)
{
	va_list args;
	int res;
	
	va_start(args, format);
	res = hyp_utf8_vfprintf_charset(fp, charset, format, args);
	va_end(args);
	return res;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * printf-like functions for printing utf8 input in system charset
 */
int hyp_utf8_vfprintf(FILE *fp, const char *format, va_list args)
{
	return hyp_utf8_vfprintf_charset(fp, hyp_get_current_charset(), format, args);
}

/*** ---------------------------------------------------------------------- ***/

int hyp_utf8_fprintf(FILE *fp, const char *format, ...)
{
	va_list args;
	int res;
	
	va_start(args, format);
	res = hyp_utf8_vfprintf(fp, format, args);
	va_end(args);
	return res;
}

/*** ---------------------------------------------------------------------- ***/

int hyp_utf8_printf(const char *format, ...)
{
	va_list args;
	int res;
	
	va_start(args, format);
	res = hyp_utf8_vfprintf(stdout, format, args);
	va_end(args);
	return res;
}
