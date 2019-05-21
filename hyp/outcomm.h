extern gboolean force_crlf;
extern gboolean is_MASTER;
extern gboolean cmdline_version;


/*
 * node/label names are case sensitiv
 */
#define namecmp strcmp


typedef struct _symtab_entry symtab_entry;
struct _symtab_entry {
	char *nodename;
	hyp_lineno lineno;
	hyp_reftype type;
	char *name;
	gboolean freeme;
	gboolean from_ref;
	gboolean from_idx;
	gboolean referenced;
	symtab_entry *next;
};

struct textattr {
	unsigned char curattr;
	unsigned char curfg;
	unsigned char curbg;
	unsigned char newattr;
	unsigned char newfg;
	unsigned char newbg;
};

typedef gboolean (*recompile_func)(HYP_DOCUMENT *hyp, hcp_opts *opt, int argc, const char **argv);

gboolean check_long_filenames(const char *dir);
char *format_dithermask(unsigned short dithermask);
char *image_name(hyp_pic_format format, HYP_DOCUMENT *hyp, hyp_nodenr node, const char *name_prefix);
hyp_pic_format format_from_pic(hcp_opts *opts, INDEX_ENTRY *entry, hyp_pic_format default_format);
symtab_entry *sym_find(symtab_entry *sym, const char *search, hyp_reftype type);
gboolean sym_check_links(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node, symtab_entry **syms);
gboolean write_image(HYP_DOCUMENT *hyp, hcp_opts *opts, hyp_nodenr node, hyp_pic_format default_format, GString *out);
symtab_entry *ref_loadsyms(HYP_DOCUMENT *hyp);
void free_symtab(symtab_entry *sym);
void write_strout(GString *s, FILE *outfp);
