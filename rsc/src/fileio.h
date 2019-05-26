/*****************************************************************************
 * FILEIO.H
 *****************************************************************************/

#ifndef __PORTAB_H__
#include <portab.h>
#endif
#ifndef __XRSRC_H__
#include <xrsrc.h>
#endif /* __XRSRC_H__ */

/*
 * Definition file format used by Interface (*.hrd)
 */
#define HRD_NAMELEN 16

#define HRD_DIALOG	0
#define HRD_MENU	1
#define HRD_ALERT	2
#define HRD_FRSTR	3
#define HRD_FRIMG	4
#define HRD_OBJECT	5
#define HRD_EOF		6
#define HRD_PREFIX	7

#define HRD_C			0x0001
#define HRD_PASCAL		0x0002
#define HRD_MODULA		0x0004
#define HRD_BASIC		0x0008 /* used by INTERFACE */
#define HRD_WFORTRAN	0x0008 /* used by KUMA */
#define HRD_GFA			0x0010 /* used by INTERFACE */
#define HRD_ASSEMBLER	0x0010 /* used by KUMA */
#define HRD_STATIC		0x0020 /* used by INTERFACE */
#define HRD_WBASIC		0x0020 /* used by KUMA */
#define HRD_VERBOSE		0x0040 /* long format, used by INTERFACE */
#define HRD_CSOURCE		0x0080

typedef struct {
	_UWORD hrd_version;
	_UWORD hrd_flags;
	_UBYTE filler1;
	_UBYTE hrd_nametype;
#define HRD_NAME_UPCASE 0
#define HRD_NAME_UPPER  1
#define HRD_NAME_LOWER  2
	_UBYTE filler2;
	_UBYTE filler3;
} HRD_HEADER;

typedef struct {
	_UBYTE entry_type;
	_UBYTE filler;
	_UWORD tree_index;
	_UWORD ob_index;
	_UBYTE hrd_name[HRD_NAMELEN+1];
} HRD_ENTRY;

/*
 * how to interpret ambiguous flags
 */
#define INTERFACE 1

typedef struct
{
	signed char x;
	signed char w;
} RSM_OBLPOS;

/*
 * Extended header used by RSM
 * with friendly support from Armin Diedering
 */
typedef struct {
	_ULONG rmxh_id;						/* RSM-XHRD ID */
#define RSM_XHRD_ID 0x524D5848UL		/* 'RMXH' */
	_UWORD rmxh_flags;
#define RSM_XHRD_ZOOM          0x0001		/* bit 0 = Zoom On/Off old Version */
#define RSM_XHRD_NEWZOOM       0x0002		/* bit 1 = Zoom On/Off new Version */
#define RSM_XHRD_MULTILAYER    0x0004		/* bit 2 = Multilayer On/Off */
#define RSM_XHRD_EXTMULTILAYER 0x0008		/* bit 3 = ExtMultilayer On/Off */
#define RSM_XHRD_NEWMULTILAYER (RSM_XHRD_MULTILAYER | RSM_XHRD_EXTMULTILAYER)
#define RSM_XHRD_MOVING        0x0010       /* bit 4 = moving-method On/Off */
#define RSM_XHRD_DUMMYICONS    0x0020		* bit 5 = dummy-icons present */
	_WORD rmxh_xzoom;						/* x-factor for zoom */
	_WORD rmxh_yzoom;						/* y-factor for zoom */
	_WORD rmxh_layers;						/* number of layers */
	union {							
		_ULONG offset;						/* file offset of coordinate arrays */
		RSM_OBLPOS **pos;					/* pointer to coordinate arrays */
	} rmxh_obrect;							
	_ULONG xrsm_flags;
#define RSM_xRSM_MOVING        0x0001		/* bit 0 = moving-method */
#define RSM_xRSM_DUMMYICONS    0x0001       /* bit 1 = dummy-icons present */
	_ULONG xrsm_id;							/* xRSM ID */
#define RSM_xRSM_ID 0x7852534DUL	/* 'xRSM' */
} RSM_XHRD;
#define RSM_SIZEOF_XHRD 24					/* size of above structure in file */


/*
 * RSM embedded CRC strings
 */
#define RSM_CRC_STRING "& RSM-crc >%04X< crc-MSR $"
/* length of printed version of above, including NUL byte */
#define RSM_CRC_STRLEN 27
/* name that is used for the string name */
#define RSM_CRC_STRID "_RSM_CRC_"
/* name that is used for the string itself */
#define RSM_CRC_STRINGID "_RSM_CRC_STRING_"


#define outw(c) outc(((c) >> 8) & 0xff); outc((c) & 0xff)
#define outwc(c) outc(((c) >> 8) & 0xff); outc((c) & 0xff); add_checksum(c)
#define outl(c) outwc((_UWORD)((c) >> 16)); outwc((_UWORD)(c))

#define output(format, arg) fprintf(ffp, format, arg)
#define output2(format, arg1, arg2) fprintf(ffp, format, arg1, arg2)
#define output3(format, arg1, arg2, arg3) fprintf(ffp, format, arg1, arg2, arg3)
#define output4(format, arg1, arg2, arg3, arg4) fprintf(ffp, format, arg1, arg2, arg3, arg4)
#define output5(format, arg1, arg2, arg3, arg4, arg5) fprintf(ffp, format, arg1, arg2, arg3, arg4, arg5)
#define outstr(str) fputs(str, ffp)
#define outc(c) putc((_WORD)(c), ffp)

#define c_is_print(c) ((unsigned char)(c) >= 0x20 && (unsigned char)(c) <= 0x7e)

extern char const program_name[];
extern char const program_version[];
extern char const program_date[];

typedef struct rsc_counter {
	_ULONG total_size;
	_ULONG ctotal_size;
	_ULONG ext_size;
	_ULONG string_space_objects;
	_ULONG string_space_free;
	_ULONG string_space_total;
	_UWORD userblks;
	_ULONG imdata_size;
	_ULONG cimdata_size;
	_ULONG cstring_space;
	_WORD nnames;
	struct {
		_UWORD menus, dialogs, alerts, strings, images;
		_UWORD bgh, bgh_more, bgh_user;
	} types;
	struct {
		_LONG trees, objects, tedinfos, iconblks, ciconblks, cicons, bitblks, frstr, strings, images;
	} conditional;
	RSC_RSM_CRC crc_for_string;
	char crc_string_buf[RSM_CRC_STRLEN];
} rsc_counter;


/* FILEIO.C */


extern FILE *ffp;
extern _BOOL ask_if_swapped;
extern const char *fname;
extern _BOOL (*rsc_a_source_func)(RSCFILE *file, rsc_counter *counter, char *filename, char *buf, _BOOL (*output_names)(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter));
extern _BOOL (*rsc_c1_source_func)(RSCFILE *file, rsc_counter *counter, char *filename, char *buf, _BOOL (*output_names)(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter));
extern _BOOL (*rsc_c2_source_func)(RSCFILE *file, rsc_counter *counter, char *filename, char *buf, _BOOL (*output_names)(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter));
extern _BOOL (*rsc_p_source_func)(RSCFILE *file, rsc_counter *counter, char *filename, char *buf, _BOOL (*output_names)(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter));
extern _BOOL (*rsc_rc_source_func)(RSCFILE *file, rsc_counter *counter, char *filename, char *filename2, char *buf);
extern _BOOL (*rsc_sdl_source_func)(RSCFILE *file, rsc_counter *counter, char *filename, char *buf);
extern _BOOL (*rsc_xml_source_func)(RSCFILE *file, rsc_counter *counter, char *filename, char *buf);
extern _BOOL (*rsc_bgh_source_func)(RSCFILE *file, rsc_counter *counter, char *filename, char *buf);

void xrsc_get_header(XRS_HEADER *xrsc_header, const char *buf);
char *rsx_basename(const char *);
const char *rsc_basename(const char *path);
char *rsc_path_get_basename(const char *path);
char *rsc_path_get_dirname(const char *path);
_BOOL file_create(const char *filename, const char *mode);
_BOOL file_open(const char *filename, const char *mode);
_BOOL file_close(_BOOL status);
_BOOL test_read(void *buf, size_t size);
const char *type_name(_WORD type);
const char *flags_name(char *sbuf, _UWORD flags, enum emutos rsctype);
const char *state_name(char *sbuf, _UWORD flags);
const char *c2_ti_fontname(char *buf, size_t bufsize, _UWORD size, _BOOL verbose);
char *c2_ti_just(char *buf, size_t bufsize, _UWORD just, _BOOL verbose);
_BOOL rsc_tree_save(RSCTREE *tree);
char *rsc_alloc_buf(XRS_HEADER *xrsc_header, RSCFILE *file, rsc_counter *counter, _BOOL for_file);
_BOOL f_exists(const char *filename);
_BOOL save_all_files(RSCFILE *file, const char *filename);
_BOOL load_all_files(RSCFILE *file, const char *filename, _ULONG which);
_BOOL rsc_activate(RSCFILE *file);
_BOOL rsc_gfa_names(RSCFILE *file, RSCTREE *tree);
_BOOL write_strings(RSCFILE *file, const char *filename);
_BOOL read_strings(RSCFILE *file, const char *filename);

#define FINAMES    0
#define FIBUILD    1
#define FIWFILE    2
#define FIUSER     3
#define FISTRING   4
#define FIIMDATA   5
#define FIFRSTR    6
#define FIBITBLK   7
#define FIFRIMG    8
#define FIICNBLK   9
#define FICICNBLK 10
#define FITEDINF  11
#define FICICONS  12
#define FIOBJECT  13
#define FITREES   14
#define FIBGH     15

void err_popup_not_found(const char *name);

void rsc_h_print_header(FILE *fp);
void rsc_c1_print_header(FILE *fp);
void rsc_c2_print_header(FILE *fp);
void rsc_p_print_header(FILE *fp);
void rsc_rsi_print_header(FILE *fp);
void rsc_gfa_print_header(FILE *fp);
void rsc_bas_print_header(FILE *fp);
void rsc_mod_print_def_header(FILE *fp);
void rsc_mod_print_mod_header(FILE *fp);
void rsc_for_print_header(FILE *fp);
void rsc_a_print_header(FILE *fp);
void rsc_ass_print_header(FILE *fp);
void rsc_inc_print_header(FILE *fp);
void rsc_rc_print_rh_header(FILE *fp);
void rsc_rc_print_rc_header(FILE *fp);
void rsc_sdl_print_header(FILE *fp);
void rsc_xml_print_header(FILE *fp);

/* CSOURCE1.C */

_BOOL rsc_c1_source(RSCFILE *file, rsc_counter *counter, char *filename, char *buf, _BOOL (*output_names)(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter));


/* CSOURCE2.C */

void p_source_info(FILE *fp, const char *output_for, const char *beg_comment, const char *mid_comment, const char *end_comment);
_BOOL c_out_str(const char *str, _WORD mx, _BOOL translatable);
_BOOL c_out_lang_str(char *str, _WORD mx, _BOOL translatable);
_BOOL rsc_c2_source(RSCFILE *file, rsc_counter *counter, char *filename, char *buf, _BOOL (*output_names)(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter));
_BOOL translatestr(RSCFILE *file, const char *str);


/* ASOURCE.C */

_BOOL rsc_a_source(RSCFILE *file, rsc_counter *counter, char *filename, char *buf, _BOOL (*output_names)(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter));


/* PSOURCE.C */

_BOOL rsc_p_source(RSCFILE *file, rsc_counter *counter, char *filename, char *buf, _BOOL (*output_names)(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter));
void rsc_rsi_print_header(FILE *fp);

/* RCSOURCE.C */

_BOOL rsc_rc_source(RSCFILE *file, rsc_counter *counter, char *filename, char *filename2, char *buf);


/* SDSOURCE.C */

_BOOL rsc_sdl_source(RSCFILE *file, rsc_counter *counter, char *filename, char *buf);

/* XMLOUT.C */

_BOOL rsc_xml_source(RSCFILE *file, rsc_counter *counter, char *filename, char *buf);

/* BGH.C */

_BOOL bgh_split_cmnt(cstringarray cmnt, stringarray *pcmnt, stringarray *pbgh);
_BOOL rsc_bgh_source(RSCFILE *file, rsc_counter *counter, char *filename, char *buf);


/* COUNT.C */

void count_trees(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter, _BOOL for_file);
void count_init(XRS_HEADER *xrsc_header, RSCFILE *file, rsc_counter *counter);
void calc_offsets(XRS_HEADER *xrsc_header, RSCFILE *file, rsc_counter *counter, _BOOL for_file);
const char *rtype_name(_WORD type);
const char *rtype_name_short(_WORD type);

_BOOL output_source_file(
	RSCFILE *file,
	XRS_HEADER *xrsc_header,
	rsc_counter *counter,
	char *buf,
	const char *h_ext,
	const char *default_header1,
	const char *default_header2,
	const char *default_file,
	_BOOL (*output_data)(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter, char *buf),
	_BOOL (*output_name)(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter));

void err_fopen(const char *fname);
void err_fread(const char *fname);
void err_fwrite(const char *fname);
void err_fcreate(const char *fname);
void err_rename(const char *oldname, const char *newname);

void set_extension(char *filename, const char *ext);
_BOOL is_mouseform(BITBLK *bit);

