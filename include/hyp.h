#ifndef __HYP_H__
#define __HYP_H__

#include <dirent.h>


#undef min
#define	min(a, b)	((a) < (b) ? (a) : (b))
#undef max
#define	max(a, b)	((a) > (b) ? (a) : (b))
#undef abs
#define	abs(a)		((a) >= 0  ? (a) : -(a))


#if defined(__PUREC__) && defined(_PUREC_SOURCE)
#define fclose(file) purec_fclose(file)
int purec_fclose(FILE *fp);
#endif


#ifndef G_STATIC_ASSERT
#define G_PASTE_ARGS(identifier1,identifier2) identifier1 ## identifier2
#define G_PASTE(identifier1,identifier2) G_PASTE_ARGS(identifier1, identifier2)
#ifdef __COUNTER__
#define G_STATIC_ASSERT(expr) typedef char G_PASTE(_Assertion_, __COUNTER__)[(expr) ? 1 : -1]
#else
#define G_STATIC_ASSERT(expr) typedef char G_PASTE(_Assertion_, __LINE__)[(expr) ? 1 : -1]
#endif
#define G_STATIC_ASSERT_EXPR(expr) ((void) sizeof (char[(expr) ? 1 : -1]))
#endif



static inline size_t ustrlen(const unsigned char *str)
{
	return strlen((const char *)str);
}


static inline int ustrcmp(const unsigned char *str1, const unsigned char *str2)
{
	return strcmp((const char *)str1, (const char *)str2);
}


static inline char *ustrcpy(unsigned char *dst, const unsigned char *src)
{
	return strcpy((char *)dst, (const char *)src);
}




/*
 * File related structures
 */
typedef struct
{
	long magic;                  /* Magic constant 'HDOC' */
	long itable_size;            /* Length of index table in bytes */
	unsigned short itable_num;   /* Number of entries in table */
	unsigned char compiler_vers; /* Hypertext compiler version */
	unsigned char compiler_os;   /* Operating system used for translation */
} HYP_HEADER;
/* sizeof(HYP_HEADER) on disk */
#define SIZEOF_HYP_HEADER 12


/* Valid values for HYP_HEADER->compiler_os */
typedef enum {
	HYP_OS_UNKNOWN = 0,
	HYP_OS_AMIGA = 1,
	HYP_OS_ATARI = 2,
	HYP_OS_MAC = 3,
	HYP_OS_WIN32 = 4,
	HYP_OS_UNIX = 5,
	HYP_OS_RES1 = 6,
	HYP_OS_RES2 = 7,
	HYP_OS_RES3 = 8,
	HYP_OS_RES4 = 9
} HYP_OS;

/*
 * Internal values of character set identifiers
 * (encoded as strings when written to a file)
 */
typedef enum {
	HYP_CHARSET_NONE = 0,
	HYP_CHARSET_UTF8 = 1,
	HYP_CHARSET_ATARI = 2,
	HYP_CHARSET_CP850 = 3,
	HYP_CHARSET_MACROMAN = 4,
	HYP_CHARSET_CP1252 = 5,
	HYP_CHARSET_BINARY = 6,
	HYP_CHARSET_BINARY_TABS = 7,
	/* NYI: */
	HYP_CHARSET_CP1250 = HYP_CHARSET_NONE,
	HYP_CHARSET_CP1251 = HYP_CHARSET_NONE,
	HYP_CHARSET_CP1253 = HYP_CHARSET_NONE,
	HYP_CHARSET_CP1254 = HYP_CHARSET_NONE,
	HYP_CHARSET_CP1255 = HYP_CHARSET_NONE,
	HYP_CHARSET_CP1256 = HYP_CHARSET_NONE,
	HYP_CHARSET_CP1257 = HYP_CHARSET_NONE,
	HYP_CHARSET_CP1258 = HYP_CHARSET_NONE,
	HYP_CHARSET_LATIN1 = HYP_CHARSET_CP1252, /* not really, but CP1252 is a superset */
	HYP_CHARSET_LATIN2 = HYP_CHARSET_NONE,
	HYP_CHARSET_LATIN3 = HYP_CHARSET_NONE,
	HYP_CHARSET_LATIN4 = HYP_CHARSET_NONE,
	HYP_CHARSET_CYRILLIC = HYP_CHARSET_NONE,
	HYP_CHARSET_ARABIC = HYP_CHARSET_NONE,
	HYP_CHARSET_GREEK = HYP_CHARSET_NONE,
	HYP_CHARSET_HEBREW = HYP_CHARSET_NONE,
	HYP_CHARSET_TURKISH = HYP_CHARSET_NONE,
	HYP_CHARSET_NORDIC = HYP_CHARSET_NONE,
	HYP_CHARSET_THAI = HYP_CHARSET_NONE,
	HYP_CHARSET_BALTIC = HYP_CHARSET_NONE,
	HYP_CHARSET_CELTIC = HYP_CHARSET_NONE,
	HYP_CHARSET_LATIN9 = HYP_CHARSET_NONE,
	HYP_CHARSET_LATIN10 = HYP_CHARSET_NONE,
	HYP_CHARSET_MAC_CE = HYP_CHARSET_NONE,
	HYP_CHARSET_CP437 = HYP_CHARSET_NONE,
	HYP_CHARSET_HP8 = HYP_CHARSET_NONE,
	HYP_CHARSET_NEXT = HYP_CHARSET_NONE,

	HYP_CHARSET_MAX = HYP_CHARSET_BINARY_TABS
} HYP_CHARSET;


#include "hypcfg.h"


/* Valid values for INDEX_ENTRY->type */
typedef enum {
	HYP_NODE_INTERNAL = 0,
	HYP_NODE_POPUP = 1,
	HYP_NODE_EXTERNAL_REF = 2,
	HYP_NODE_IMAGE = 3,
	HYP_NODE_SYSTEM_ARGUMENT = 4,
	HYP_NODE_REXX_SCRIPT = 5,
	HYP_NODE_REXX_COMMAND = 6,
	HYP_NODE_QUIT = 7,
	HYP_NODE_CLOSE = 8,
	HYP_NODE_EOF = 0xff
} hyp_indextype;
#define HYP_NODE_IS_TEXT(type) ((type) <= HYP_NODE_POPUP)


#define SIZEOF_SHORT 2 /* sizeof(short) on disk */
#define SIZEOF_LONG  4 /* sizeof(long) on disk */

/*
 * some strings have a length byte, with an offset of 32
 */
#define HYP_STRLEN_OFFSET 32u

typedef unsigned short hyp_nodenr;
#define HYP_NOINDEX ((hyp_nodenr)-1)

typedef unsigned short hyp_lineno;

typedef struct
{
	unsigned char length;       /* Length */
	/* hyp_indextype */ unsigned char type;         /* Type */
	unsigned long seek_offset;  /* File seek offset*/
	unsigned short comp_diff;   /* Difference packed/unpacked in bytes */
	hyp_nodenr next;            /* Index of successor entry */
	hyp_nodenr previous;        /* Index of predecessor entry */
	hyp_nodenr toc_index;       /* Index of directory/parent entry */
	unsigned char name[1];      /* First character of 0-terminated entry name */
} INDEX_ENTRY;
/* sizeof(INDEX_ENTRY) on disk */
#define SIZEOF_INDEX_ENTRY 14U /* +zero-terminated name */

/*
 * encoding of the index entry length as "unsigned char"
 * restricts the possible length of node names.
 * Length of entries in a REF file are also
 * encoded as "unsigned char", so the same restriction
 * applies to labels and aliases.
 */
#define HYP_NODENAME_MAX (253U - SIZEOF_INDEX_ENTRY)

/*
 * Node numbers are (sometimes) encoded as base-255, in
 * 2 bytes. This gives a theoretical maximum of 65025;
 * leave some place to be on the safe side.
 */
#define HYP_NODE_MAX ((hyp_nodenr)65000U)

/* same for line numbers */
#define HYP_LINENO_MAX ((hyp_lineno)65000U)

/*
 * The length of a link text is encoded
 * in an "unsigned char", with an offset of HYP_STRLEN_OFFSET
 */
#define HYP_LINKTEXT_MAX (255u - HYP_STRLEN_OFFSET)

/*
 * values used for extended headers
 */
typedef enum  {
	HYP_EXTH_EOF = 0,
	HYP_EXTH_DATABASE = 1,
	HYP_EXTH_DEFAULT = 2,
	HYP_EXTH_HOSTNAME = 3,
	HYP_EXTH_OPTIONS = 4,
	HYP_EXTH_AUTHOR = 5,
	HYP_EXTH_VERSION = 6,
	HYP_EXTH_HELP = 7,
	HYP_EXTH_SUBJECT = 8,
	HYP_EXTH_TREEHEADER = 9,
	HYP_EXTH_STGUIDE_FLAGS = 10,
	HYP_EXTH_WIDTH = 11,
	/* new definitions: */
	HYP_EXTH_CHARSET = 30
} hyp_ext_header;


#define HYP_EOL '\0'

/* ESC values used in (p)nodes */
#define HYP_ESC 				0x1b	/* value used to start esc sequences */
#define HYP_ESC_ESC 			0x1b	/* ESC value quoted */
#define HYP_ESC_WINDOWTITLE 	0x23    /* Window title */
#define HYP_ESC_LINK			0x24	/* link */
#define HYP_ESC_LINK_LINE		0x25	/* link with linenumber */
#define HYP_ESC_ALINK			0x26    /* alink */
#define HYP_ESC_ALINK_LINE		0x27	/* alink with line */
#define HYP_ESC_DATA0			0x28	/* data block */
#define HYP_ESC_DATA1			0x29	/* data block */
#define HYP_ESC_DATA2			0x2a	/* data block */
#define HYP_ESC_DATA3			0x2b	/* data block */
#define HYP_ESC_DATA4			0x2c	/* data block */
#define HYP_ESC_DATA5			0x2d	/* data block */
#define HYP_ESC_DATA6			0x2e	/* data block */
#define HYP_ESC_DATA7			0x2f	/* data block */
#define HYP_ESC_EXTERNAL_REFS	0x30	/* external data block */
#define HYP_ESC_OBJTABLE		0x31	/* table with objects */
#define HYP_ESC_PIC 			0x32	/* picture */
#define HYP_ESC_LINE			0x33    /* line */
#define HYP_ESC_BOX				0x34	/* box */
#define HYP_ESC_RBOX			0x35	/* rounded box */
#define HYP_ESC_TEXTATTR_FIRST	0x64	/* text/font attribute */
#define HYP_ESC_TEXTATTR_LAST	0xa3

#define HYP_TXT_NORMAL       0x0000
#define HYP_TXT_BOLD         0x0001
#define HYP_TXT_LIGHT        0x0002
#define HYP_TXT_ITALIC       0x0004
#define HYP_TXT_UNDERLINED   0x0008
#define HYP_TXT_OUTLINED     0x0010
#define HYP_TXT_SHADOWED     0x0020
#define HYP_TXT_MASK         0x003f

#define HYP_ESC_IS_TEXATTR(c) ((c) >= HYP_ESC_TEXTATTR_FIRST && (c) <= HYP_ESC_TEXTATTR_LAST)

#define HYP_ESC_CASE_DATA \
	     HYP_ESC_DATA0: \
	case HYP_ESC_DATA1: \
	case HYP_ESC_DATA2: \
	case HYP_ESC_DATA3: \
	case HYP_ESC_DATA4: \
	case HYP_ESC_DATA5: \
	case HYP_ESC_DATA6: \
	case HYP_ESC_DATA7

#define HYP_ESC_CASE_TEXTATTR \
	     0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: \
	case 0x6c: case 0x6d: case 0x6e: case 0x6f: case 0x70: case 0x71: case 0x72: case 0x73: \
	case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: \
	case 0x7c: case 0x7d: case 0x7e: case 0x7f: case 0x80: case 0x81: case 0x82: case 0x83: \
	case 0x84: case 0x85: case 0x86: case 0x87: case 0x88: case 0x89: case 0x8a: case 0x8b: \
	case 0x8c: case 0x8d: case 0x8e: case 0x8f: case 0x90: case 0x91: case 0x92: case 0x93: \
	case 0x94: case 0x95: case 0x96: case 0x97: case 0x98: case 0x99: case 0x9a: case 0x9b: \
	case 0x9c: case 0x9d: case 0x9e: case 0x9f: case 0xa0: case 0xa1: case 0xa2: case 0xa3

/* picture header structure */
typedef struct
{
	unsigned short width;        /* width in pixels */
	unsigned short height;       /* height in pixel */
	unsigned char planes;        /* number of planes (1..8) */
	unsigned char plane_pic;     /* available planes bit vector */
	unsigned char plane_on_off;  /* filled plane bit vector */
	unsigned char filler;        /* fill byte used to align size of structure */
} HYP_PICTURE;
/* sizeof(HYP_PICTURE) on disk */
#define SIZEOF_HYP_PICTURE 8


/*
 * File extensions in use
 */
#define HYP_EXT_HYP ".hyp"
#define HYP_EXT_REF ".ref"
#define HYP_EXT_STG ".stg"
#define HYP_EXT_RSC ".rsc"
#define HYP_EXT_TXT ".txt"
#define HYP_EXT_GUIDE ".guide"
#define HYP_EXT_HTML ".html"

/*
 * The values original ST-Guide uses,
 * not neccesarrily the one we use.
 * Especially any limits do not apply,
 * they are only used to warn about
 * compatibility problems.
 */
#define HYP_STGUIDE_DEFAULT_LINEWIDTH 75
#define HYP_STGUIDE_MAX_LINEWIDTH 126
#define HYP_STGUIDE_MIN_LINEWIDTH 20


/*
 * default filenames to use for CATALOG searches
 */
#define HYP_FILENAME_HYPFIND "hypfind" HYP_EXT_HYP


#define HYP_MAGIC_REF 0x48524546L   /* 'HREF', read as big-endian */
#define HYP_MAGIC_HYP 0x48444f43L   /* 'HDOC', read as big-endian */

/*
 *	Definitions used by loading routines
 */

typedef enum
{
	REF_FILENAME = 0,
	REF_NODENAME = 1,
	REF_ALIASNAME = 2,
	REF_LABELNAME = 3,
	REF_DATABASE = 4,
	REF_OS = 5,
	REF_TITLE = 6,        /* undocumented; only found in finder.ref */
	REF_UNKNOWN = 8,      /* undocumented; only found in finder.ref */
	REF_CHARSET = 30
} hyp_reftype;

typedef struct {
	hyp_reftype type;
	union {
		const unsigned char *hyp;
		char *utf8;
	} name;
	hyp_lineno lineno; /* only valid if type == REF_LABELNAME */
} REF_ENTRY;

typedef struct _ref_module REF_MODULE;
struct _ref_module {
	long module_len;
	long module_offset;
	const unsigned char *data;
	long num_entries;
	const unsigned char *module_filename;	/* as entered in REF file, not the filename we load from; in HYP encoding */
	char *filename;	/* as entered in REF file, not the filename we load from; in utf8 encoding */
	const unsigned char *module_database;	/* as entered in REF file, in HYP encoding */
	char *database;	/* as entered in REF file, in utf8 encoding */
	HYP_OS os;
	HYP_CHARSET charset;
	gboolean had_filename;			/* TRUE if module had a FILENAME entry */
	gboolean had_os;
	gboolean had_charset;
	gboolean mod_name_matches;		/* TRUE if the module filename matches the file it was loaded from */
	REF_MODULE *next;
	REF_ENTRY *entries;
};

typedef struct
{
	size_t data_size;
	REF_MODULE *modules;
	gboolean is_converted;
	char *filename; /* local filename we are reading/writing */
	unsigned char data[1];
} REF_FILE;

typedef struct
{
	hyp_nodenr number;          /* Page number of this entry */
	unsigned char *start;       /* Pointer to start of data */
	unsigned char *end;         /* Pointer to end of data */
	long width, height;
	const unsigned char *window_title; /* Pointer to window title, in encoding of HYP */
	const unsigned char **line_ptr;
	struct hyp_gfx *gfx;		/* graphics commands */
} HYP_NODE;

typedef struct _hyp_hostname HYP_HOSTNAME;
struct _hyp_hostname {
	HYP_HOSTNAME *next;
	char *name;
};

typedef struct
{
	hyp_nodenr num_index;       /* Number of entries in hypertext file */
	long itable_size;			/* Size of index table */
	unsigned char comp_vers;    /* Version of compiler used for translation */
	HYP_OS comp_os;             /* Operating system ID */
	HYP_CHARSET comp_charset;   /* character set */
	short st_guide_flags;       /* Special ST-Guide flags (encryption, ...) */
	short line_width;           /* Line width to use for display (@width) */
	char *database; 			/* Description for hypertext database (@database) */
	HYP_HOSTNAME *hostname;     /* List of Names of host applications (@hostname) */
	char *author;      			/* Name of authors (@author) */
	char *version;              /* Hypertext version string (@$VER:) */
	char *hcp_options;			/* options used to compile source (@options) */
	char *subject;				/* katalog subject (@subject) */
	char *help_name;			/* help page name (@help) */
	char *default_name;			/* default page name (@default) */
	INDEX_ENTRY **indextable;   /* Pointer to index table */
	gboolean flatindex;			/* TRUE if index data is kept in continguous memory */
	HYP_NODE **cache;           /* Pointer to cache table */
	const char *file;           /* Full access path to file */
	int handle;                 /* file handle if already open */
	hyp_nodenr index_page;      /* Page number of hypertext index */
	hyp_nodenr default_page;    /* Page number of default page (@default) */
	hyp_nodenr help_page;       /* Page number of help page */
	REF_FILE *ref;              /* Pointer to REF file structure */
	hyp_nodenr first_text_page; /* first node number with text */
	hyp_nodenr last_text_page;  /* last node number with text */
	int ref_count;				/* usage count */
} HYP_DOCUMENT;

/*
 * Valid values for HYP_DOCUMENT->st_guide_flags
 * Also used for -k[VAL] when calling hcp.
 */
#define STG_ALLOW_FOLDERS_IN_XREFS  0x0001
#define	STG_ENCRYPTED	            0x0002


#define hypnode_valid(hyp, node) ((node) != HYP_NOINDEX && (node) < (hyp)->num_index)


/*
 * cached entry for images
 */
typedef struct {
	hyp_nodenr number;          /* Page number of this entry */
	gboolean decompressed;
	MFDB pic;
} HYP_IMAGE;


/*
 * values used to calculate the "lines" and "columns" of a picture
 */
#define HYP_PIC_FONTW 8
#define HYP_PIC_FONTH 16


/*
 * Decode a base 255 value.
 * Such values are used in popup texts to encode node numbers.
 * Although the ST-Guide documentation does not explicitly mention it,
 * these values seem to be strangely encoded in little endian order
 * (low byte first).
 * Another bad design; the purpose of this was to avoid zero bytes in
 * the page text, but there are other places where these occure.
 */
#define DEC_255(ptr)	((unsigned short) dec_from_chars((const unsigned char *)(ptr)))
static inline short dec_from_chars(const unsigned char *ptr)
{
	unsigned short val;

	val = (ptr[1] - 1U) * 255U;
	val += ptr[0];
	val--;
	return val;
}


static inline void long_to_chars(long val, unsigned char *p)
{
	p[3] = (unsigned char)val;
	val >>= 8;
	p[2] = (unsigned char)val;
	val >>= 8;
	p[1] = (unsigned char)val;
	val >>= 8;
	p[0] = (unsigned char)val;
}


static inline long long_from_chars(const unsigned char *p)
{
	unsigned long val;
	
	val  = ((unsigned long)p[0]) << 24;
	val |= ((unsigned long)p[1]) << 16;
	val |= ((unsigned long)p[2]) << 8;
	val |= ((unsigned long)p[3]);
	if ((val & 0x80000000UL) && LONG_MAX > 2147483647L)
		val |= ~0xFFFFFFFFUL;
	return val;
}


static inline void short_to_chars(short val, unsigned char *p)
{
	p[1] = (unsigned char)val;
	val >>= 8;
	p[0] = (unsigned char)val;
}


static inline void short_to_lechars(short val, unsigned char *p)
{
	p[0] = (unsigned char)val;
	val >>= 8;
	p[1] = (unsigned char)val;
}


static inline short short_from_chars(const unsigned char *p)
{
	unsigned short val;
	
	val  = ((unsigned short)p[0]) << 8;
	val |= ((unsigned short)p[1]);
	return val;
}


static inline short short_from_lechars(const unsigned char *p)
{
	unsigned short val;
	
	val  = ((unsigned short)p[1]) << 8;
	val |= ((unsigned short)p[0]);
	return val;
}


/* Return values for file loading routines */
typedef enum {
	HYP_FT_NONE = -3,
	HYP_FT_LOADERROR = -2,      /* Error while loading */
	HYP_FT_UNKNOWN = -1,        /* Unknown file type */
	HYP_FT_BINARY = 0,          /* binary format */
	HYP_FT_ASCII,               /* ASCII format */
	HYP_FT_HYP,                 /* ST-Guide hypertext format */
	HYP_FT_REF,					/* references file */
	HYP_FT_RSC,					/* GEM resource file */
	HYP_FT_CHEADER,				/* C header file */
	HYP_FT_STG,					/* ST-Guide source file */
	HYP_FT_GUIDE,				/* AmigaGuide source file */
	HYP_FT_IMAGE,				/* some image format */
	HYP_FT_HTML					/* some HTML format */
} hyp_filetype;


/* internal number for identifying a filename (compiler only) */
typedef int FILE_ID;

/* types of supported image formats */
typedef enum {
	HYP_PIC_ORIG = 0,
	HYP_PIC_UNKNOWN = HYP_PIC_ORIG,
	HYP_PIC_IFF = 1,
	HYP_PIC_ICN = 2,
	HYP_PIC_IMG = 3,
	HYP_PIC_BMP = 4,
	HYP_PIC_GIF = 5,
	HYP_PIC_PNG = 6,
	HYP_PIC_LAST = HYP_PIC_PNG
} hyp_pic_format;

/*
 * graphics commands data (@line/@box/@rbox/@image/@limage)
 */
struct hyp_gfx
{
	struct hyp_gfx *next;
	
	/*
	 * value from ESC command
	 */
	short type;
	
	/*
	 * column number for graphic commands
	 * - @line/@box/@rbox:
	 *   - value specified in source file: 1-255
	 *   - value used internally: 1-255
	 *   - value written to file: 1-255
	 * - @image/@limage:
	 *   - value specified in source file: 0-255
	 *   - value used internally: 0-255
	 *   - value written to file: 0-255
	 *   (0 == centered)
	 */
	_WORD x_offset;
	
	/*
	 * line number for graphic commands
	 * - @line/@box/@rbox/@image/@limage:
	 *   - value specified in source file: input line number where command appears
	 *   - value used internally: 0-65000
	 *   - value written to file: base-255 encoded
	 */
	hyp_lineno y_offset;
	
	/*
	 * width for graphic commands
	 * - @image/@limage:
	 *   - value specified in source file: none
	 *   - value used internally: 0, or 1 for limage
	 *   - value written to file: 0, or 1 for limage
	 * - @box/@rbox:
	 *   - value specified in source file: 1-255
	 *   - value used internally: 1-255
	 *   - value written to file: 1-255
	 * - @line:
	 *   - value specified in source file: -127-+127
	 *   - value used internally: -127-+127
	 *   - value written to file: 1-255
	 */
	_WORD width;
	
	/*
	 * height for graphic commands
	 * - @image/@limage:
	 *   - value specified in source file: none
	 *   - value used internally: none
	 *   - value written to file: 0
	 * - @box/@rbox:
	 *   - value specified in source file: 1-255
	 *   - value used internally: 1-255
	 *   - value written to file: 1-255
	 * - @line:
	 *   - value specified in source file: 0-254
	 *   - value used internally: 0-254
	 *   - value written to file: 1-255
	 */
	_WORD height;

	/*
	 * for @line only: line end + style
	 *   - value used internally: 0-254
	 *   - value written to file: 1-255
	 */
 	unsigned char attr;

	/*
	 * for @line only: line end attributes
	 *   - value specified in source file: 0-3
	 *   - value used internally: 0-3
	 *   - value written to file: 0-3 into lower 3 bits of attr
	 */
 	unsigned char begend;

	/*
	 * style for graphic commands
	 * - @box/@rbox:
	 *   - value specified in source file: 0-36
	 *   - value used internally: 0-36
	 *   - value written to file: 0-36
	 *   (0 = hollow, 1-24 = pattern index, 25-36 = hatch index + 24; pattern index 8 is solid)
	 * - @line:
	 *   - value specified in source file: 1-7
	 *   - value used internally: 1-7
	 *   - value written to file: 0-6 into upper 5 bits of attr
	 */
 	unsigned char style;
	
	/* TRUE for @limage only */
	gboolean islimage;
	
	/*
	 * for images only: index entry of graphic data
	 */
 	hyp_nodenr extern_node_index;
 	
	/*
	 * for compiler & images only: id of filename
	 */
 	FILE_ID id;

	/*
	 * for images: values read from header of picture node
	 * for other graphics commands: scaled dimensions of command
	 */
	int pixwidth;				/* width in pixels */
	int pixheight;				/* height in pixel */
	/*
	 * for images only: values read from header of picture node
	 */
	unsigned char planes;		/* number of planes (1..8) */
	hyp_pic_format format;
	
	/* for decompiler only: */
	gboolean used;
	
#ifdef WITH_GUI_GTK
	int window_x, window_y;
	int window_margin;
	void /* cairo_surface_t */ *surf;
#endif
};


/*
 * gfx.c
 */

void hyp_decode_gfx(HYP_DOCUMENT *hyp, const unsigned char *src, struct hyp_gfx *adm);
gboolean hyp_prep_graphics(HYP_DOCUMENT *hyp, HYP_NODE *node);
void hyp_free_graphics(HYP_NODE *node);
gboolean W_Fix_Bitmap(void **data, _WORD width, _WORD height, _WORD planes);
void W_Release_Bitmap(void **data, _WORD width, _WORD height, _WORD planes);
_WORD GetNumPlanes(void);


/*
 * hyp.c
 */
extern char const hyp_default_main_node_name[] /* = "Main" */ ;
extern char const hyp_default_index_node_name[] /* = "Index" */ ;
extern char const hyp_default_help_node_name[] /* = "Help" */ ;

char *hyp_lib_version(void);
char *hyp_compiler_version(void);

HYP_NODE *hyp_node_alloc(long size);
void hyp_node_free(HYP_NODE *node);
void hyp_image_free(HYP_IMAGE *image);

HYP_DOCUMENT *hyp_new(void);
void hyp_delete(HYP_DOCUMENT *hyp);
HYP_DOCUMENT *hyp_load(int handle, hyp_filetype *err);
HYP_DOCUMENT *hyp_ref(HYP_DOCUMENT *hyp);
HYP_DOCUMENT *hyp_unref(HYP_DOCUMENT *hyp);

const char *hyp_osname(HYP_OS os);
HYP_OS hyp_os_from_name(const char *name);

char *hyp_find_file(const char *path);
hyp_filetype hyp_guess_filetype(const char *name);
char *hyp_invalid_page(hyp_nodenr page);


/*
 * hyp_file.c
 */

#define HYP_DEFAULT_FILEMODE 0644
#define HYP_DEFAULT_DIRMODE 0755

FILE *hyp_utf8_fopen(const char *filename, const char *mode);
int hyp_utf8_open(const char *filename, int flags, mode_t mode);
int hyp_utf8_close(int fd);
int hyp_utf8_fclose(FILE *fp);
int hyp_utf8_unlink(const char *name);
int hyp_utf8_rename(const char *oldname, const char *newname);
DIR *hyp_utf8_opendir(const char *dirname);
char *hyp_utf8_readdir(DIR *dir);
void hyp_utf8_closedir(DIR *dir);
ssize_t hyp_utf8_write(int fd, const void *buf, size_t len);

/*
 * hyp_load.c
 */

/*
 * return uncompressed size of index entry <num>
 */
static inline unsigned long GetCompressedSize(HYP_DOCUMENT *hyp, hyp_nodenr num)
{
	return hyp->indextable[num + 1]->seek_offset - hyp->indextable[num]->seek_offset;
}

void hyp_decrypt(unsigned char *ptr, long bytes);
unsigned long GetDataSize(HYP_DOCUMENT *hyp, hyp_nodenr nr);
unsigned char *hyp_loaddata(HYP_DOCUMENT *hyp, hyp_nodenr num);
HYP_NODE *hyp_loadtext(HYP_DOCUMENT *hyp, hyp_nodenr node_num);
gboolean GetEntryBytes(HYP_DOCUMENT *doc, hyp_nodenr nr, const unsigned char *src, unsigned char *dst, long bytes);


/*
 * hyp_save.c
 */

#define hyp_encrypt(ptr, bytes) hyp_decrypt(ptr, bytes)
void SetCompressedSize(HYP_DOCUMENT *hyp, hyp_nodenr num, unsigned long prev_pos, unsigned long curr_pos);
gboolean SetDataSize(HYP_DOCUMENT *hyp, hyp_nodenr num, unsigned long datasize);
gboolean WriteEntryBytes(HYP_DOCUMENT *hyp, hyp_nodenr num, unsigned char *src, unsigned long *bytes, FILE *outfile, gboolean compress);



/*
 * tool.c
 */
hyp_nodenr find_nr_by_title(HYP_DOCUMENT *hyp_doc, const char *title);
const unsigned char *hyp_skip_esc(const unsigned char *pos);
gboolean hyp_node_find_windowtitle(HYP_NODE *nodeptr);
hyp_nodenr hyp_first_text_page(HYP_DOCUMENT *hyp_doc);
hyp_nodenr hyp_last_text_page(HYP_DOCUMENT *hyp_doc);


/*
 * cache.c
 */
void InitCache(HYP_DOCUMENT *hyp);
void ClearCache(HYP_DOCUMENT *hyp);
gboolean TellCache(HYP_DOCUMENT *hyp, hyp_nodenr node_num, HYP_NODE *node);
HYP_NODE *AskCache(HYP_DOCUMENT *hyp, hyp_nodenr node_num);
void RemoveNodes(HYP_DOCUMENT *hyp);
void RemovePictures(HYP_DOCUMENT *hyp, gboolean reload);


/*
 * prepare.c
 */
void hyp_pic_get_header(HYP_PICTURE *hyp_pic, const unsigned char *hyp_pic_raw);


/*
 * lh5d.c
 */
gboolean lh5_decode(unsigned char *unpackedMem, unsigned long unpackedLen, const unsigned char *packedMem, unsigned long packedLen);
gboolean lh5_encode(FILE *outfile, const unsigned char *unpackedMem, unsigned long orgsize, unsigned int bufsize, unsigned long *packedLen);
void lh5_make_crctable(void);
unsigned short lh5_update_crc(const unsigned char *p, unsigned long n, unsigned short crc);


/*
 * ref.c
 */
#ifndef __TOS__
typedef struct _lbox_item LBOX_ITEM;

struct _lbox_item {
	LBOX_ITEM *next;                    /* Pointer to the next entry in the list */
	short selected;                     /* Specifies if the object is selected */
	short data1;                        /* Data for the program... */
	void *data2;
	void *data3;
};
#endif

typedef struct _result_entry_ 
{
	/* FIXME: TOS: remove LBOX_ITEM head, convert to own list including charset conversion */
	LBOX_ITEM item;
	char str[256];

	const char *path;
	const char *dbase_description;
	char *node_name;
	hyp_lineno lineno;
	char *label_name;
	char *alias_name;
} RESULT_ENTRY;

REF_FILE *ref_load(const char *filename, int handle, gboolean verbose);
int ref_num_modules(REF_FILE *ref);
char *ref_findnode(REF_FILE *ref, const char *string, hyp_lineno *line, gboolean only_first, gboolean *freename);
RESULT_ENTRY *ref_findall(REF_FILE *ref, const char *string, long *num_results, gboolean *aborted);
void ref_close(REF_FILE *ref);
void ref_freeresults(RESULT_ENTRY *list);
gboolean ref_list(REF_FILE *ref, FILE *outfile, gboolean all);
REF_FILE *ref_new(const char *filename, size_t size);
char *ref_hyp_basename(const char *name);
gboolean ref_list_entries(const char *filename, FILE *outfile, gboolean all, gboolean verbose);
gboolean ref_del_entries(const char *refname, int argc, const char **argv, FILE *outfile, gboolean verbose);
gboolean ref_write_module(int handle, REF_MODULE *mod, gboolean verbose);
gboolean ref_extract_entries(const char *refname, int argc, const char **argv, FILE *outfile, gboolean verbose);
gboolean ref_add_entries(const char *refname, const char *modname, gboolean delete_mod, FILE *outfile, gboolean verbose);
gboolean ref_write_header(int ref_handle);
gboolean ref_write_trailer(int ref_handle);
const char *ref_osname(HYP_OS os);
void ref_conv_to_utf8(REF_FILE *ref);


/*
 * cset.c
 */
typedef uint32_t h_unichar_t;
#define HYP_UTF8_CHARMAX 6

const char *hyp_charset_name(HYP_CHARSET charset);
HYP_CHARSET hyp_charset_from_name(const char *name);
HYP_CHARSET hyp_default_charset(HYP_OS os);
HYP_CHARSET hyp_get_current_charset(void);
HYP_CHARSET hyp_get_filename_charset(void);
HYP_OS hyp_get_current_os(void);
int hyp_utf8_vfprintf(FILE *fp, const char *format, va_list args);
int __attribute__((format(printf, 2, 3))) hyp_utf8_fprintf(FILE *fp, const char *format, ...);
int __attribute__((format(printf, 1, 2))) hyp_utf8_printf(const char *format, ...);
int hyp_utf8_vfprintf_charset(FILE *fp, HYP_CHARSET charset, const char *format, va_list args);
int __attribute__((format(printf, 3, 4))) hyp_utf8_fprintf_charset(FILE *fp, HYP_CHARSET charset, const char *format, ...);
int __attribute__((format(printf, 2, 3))) hyp_utf8_printf_charset(HYP_CHARSET charset, const char *format, ...);

char *hyp_utf8_to_charset(HYP_CHARSET charset, const void *src, size_t len, gboolean *converror);
char *hyp_conv_charset(HYP_CHARSET from, HYP_CHARSET to, const void *src, size_t len, gboolean *converror);
const char *hyp_utf8_conv_char(HYP_CHARSET charset, const char *src, char *buf, gboolean *converror);
char *hyp_conv_to_utf8(HYP_CHARSET charset, const void *src, size_t len);
int hyp_name_cmp(HYP_CHARSET charset, const unsigned char *str1, const unsigned char *str2);

wchar_t *hyp_utf8_to_wchar(const char *str, size_t len, size_t *lenp);
char *hyp_wchar_to_utf8(const wchar_t *str, size_t len);
char *hyp_utf8_casefold(const char *str, size_t len);
gboolean g_unichar_isupper(h_unichar_t c);
gboolean g_unichar_islower(h_unichar_t c);
size_t hyp_unichar_to_utf8(char *buf, h_unichar_t wc);

h_unichar_t g_unichar_tolower(h_unichar_t ch);
h_unichar_t g_unichar_toupper(h_unichar_t ch);

h_unichar_t hyp_utf8_get_char(const char *);
int g_utf8_strcasecmp(const char *s1, const char *s2);
int g_utf8_strncasecmp(const char *s1, const char *s2, size_t n);
const char *g_utf8_strcasestr(const char *searchee, const char *lookfor);


/*
 * ext_refs.c
 */
short HypCountExtRefs(HYP_NODE *entry);



/*
 * win32.c
 */
#ifdef __WIN32__

#undef CP_UTF16_LE
#define CP_UTF16_LE 1200

char *win32_errstring(DWORD err);
int win32_to_errno(DWORD oserrno);
DWORD win32_from_errno(int err_no);
#define mkdir(s, m) _mkdir(s)
#endif


/*
 * misc.c
 */
char *hyp_utf8_strerror(int err);
char *chomp(char *str);
const char *g_utf8_skipchar(const char *p);
size_t g_utf8_str_len(const char *p, size_t len);

gboolean walk_dir(const char *dirname, gboolean (*f)(const char *filename, void *data), void *data);
gboolean walk_pathlist(const char *list, gboolean (*f)(const char *filename, void *data), void *data);

#if !defined(P_WAIT) && defined(_P_WAIT)
#define P_WAIT   _P_WAIT
#define P_NOWAIT _P_NOWAIT
#define P_OVERLAY _P_OVERLAY
#endif
#ifndef P_WAIT
#define P_WAIT		0
#define P_NOWAIT	1
#define P_OVERLAY	2
#endif
int hyp_utf8_spawnvp(int mode, int argc, const char *const argv[]);
#ifdef __TOS__
char *make_argv(char cmd[128], const char *const argv[]);
#endif

#endif /* __HYP_H__ */
