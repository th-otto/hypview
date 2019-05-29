/*****************************************************************************
 * RSRCLOAD.H
 *****************************************************************************/

#ifndef __RSRCLOAD_H__
#define __RSRCLOAD_H__

#include <portab.h>
#include "portaes.h"
#include "xrsrc.h"
#include "mobject.h"
#include "extob.h"
#include <time.h>

EXTERN_C_BEG


/****** Resource library *****************************************************/

/*
 * sizes of various field types
 * in the resource file. Not neccessarily the same as
 * sizeof(native type)
 */
#define RSC_SIZEOF_WORD 2
#define RSC_SIZEOF_LONG 4
#define RSC_SIZEOF_PTR  4

#define RSC_SIZEOF_TEDINFO  28
#define RSC_SIZEOF_ICONBLK  34
#define RSC_SIZEOF_BITBLK   14
#define RSC_SIZEOF_CICON    22
#define RSC_SIZEOF_CICONBLK 38
#define RSC_SIZEOF_USERBLK   8
#define RSC_SIZEOF_OBJECT   24

enum emutos {
	EMUTOS_NONE,
	EMUTOS_DESK,
	EMUTOS_ICONS,
	EMUTOS_AES
};

typedef _ULONG CSET[8];

typedef struct _namerule {
	_UWORD /* _BOOL */ upper;
	_UWORD /* _BOOL */ lower;
	_UWORD /* _BOOL */ alpha;
	_UWORD /* _BOOL */ alnum;
	_UBYTE add[40];
	_UBYTE sub[40];
	CSET charset;
} NAMERULE;

typedef struct rsc_lang rsc_lang;
struct rsc_lang {
	char *id;			/* name of language */
	char *filename;		/* associated file */
	char *charset;
	const char *const *const *hash;		/* hash of strings for this language */
	rsc_lang *next;		
};
	
struct _rscfile {
	XRS_HEADER header;		/* header from file */
	char *data;				/* file or memory contents */
	_BOOL rsc_little_endian;
	unsigned int allocated;	/* bitmask of allocated arrays */
#define RSC_ALLOC_TRINDEX  0x0001
#define RSC_ALLOC_OBJECT   0x0002
#define RSC_ALLOC_TEDINFO  0x0004
#define RSC_ALLOC_ICONBLK  0x0008
#define RSC_ALLOC_BITBLK   0x0010
#define RSC_ALLOC_FRSTR    0x0020
#define RSC_ALLOC_FRIMG    0x0040
#define RSC_ALLOC_CICONBLK 0x0080
#define RSC_ALLOC_USERBLK  0x0100
	OBJECT **rs_trindex;
	OBJECT *rs_object;
	TEDINFO *rs_tedinfo;
	ICONBLK *rs_iconblk;
	BITBLK *rs_bitblk;
	char **rs_frstr;
	BITBLK **rs_frimg;
	CICONBLK *rs_ciconblk;
	USERBLK *rs_userblk;

	_LONG rsc_ntrees;			/* number of RSCTREEs */
	_ULONG rsc_flags;			/* output flags */
	_ULONG rsc_flags2;			/* output flags */
	_WORD rsc_namelen;
	_BOOL rsc_exact;
	NAMERULE rsc_rule1;
	NAMERULE rsc_rule2;
	RSCTREE rsc_treehead;		/* head of resource tree list */
	char rsc_rsxname[FNAMELEN+1];/* name of resource */
	char rsc_rsxfilename[PATH_MAX];	/* full pathname */
	stringarray rsc_cmnt;
	enum emutos rsc_emutos;
	_BOOL rsc_use_oldlang;
	char *rsc_emutos_frstrcond_name;
	char *rsc_emutos_frstrcond_string;
	char *rsc_emutos_othercond_name;
	char *rsc_emutos_othercond_string;
	char *rsc_output_prefix;
	char *rsc_output_basename;
	time_t rsc_date_created;
	time_t rsc_date_changed;
	_LONG rsc_edition;
	_BOOL rsc_swap_flag;		/* TRUE if file has been swapped when loading */
	_BOOL rsc_xrsc_flag;		/* TRUE if we need an extended header */
	EXTOB_OPTIONS rsc_extob;	/* mode of extended object types */
	rsc_options rsc_opts;
	RSC_RSM_CRC rsc_rsm_crc;
	RSC_RSM_CRC rsc_crc_for_string;
	rsc_lang *rsc_langs;
	_BOOL had_rsm_hdr;
	_BOOL need_rsm_hdr;
	_LONG rsc_nciconblks;		/* number of CICONBLK structs */
	_LONG rsc_ncicons;			/* number of CICON structs */
	_LONG rsc_nstrings;         /* total number of strings */
	_LONG rsc_nimages;			/* total number of images */
	_LONG rsc_nuserblks;		/* number of USERBLK structs */
	int fontset;
};

/*
 * xrsrc_load() flags
 */
#define XRSC_NO_INSERT_POPUPS 0x0001	/* do not do ORCS style popup inserts */
#define XRSC_NO_ZERO_ROOT     0x0002	/* do not set root object to 0x0 */
#define XRSC_NO_OBFIX         0x0004	/* do not run rsrc_obfix() on dialogs */
#define XRSC_SAFETY_CHECKS    0x0008	/* perform lots of validation checks */
#define XRSC_REPORT_PO        0x0010	/* report message statistics from loading po file */
#define XRSC_REPORT_RSC       0x0020	/* report translation statistics from translating resource */


RSCFILE *xrsrc_load(const char *fname, _WORD wchar, _WORD hchar, _UWORD flags);
_BOOL xrsrc_free(RSCFILE *file);
_BOOL xrsrc_gaddr(RSCFILE *file, _WORD type, _WORD idx, void *gaddr);
_BOOL xrsrc_saddr(RSCFILE *file, _WORD type, _WORD idx, void *saddr);
_WORD xrsrc_obfix(OBJECT *re_otree, _WORD re_oobject);

_BOOL W_Cicon_Setpalette(_WORD *palette);

EXTERN_C_END

#endif /* __RSRCLOAD_H__ */
