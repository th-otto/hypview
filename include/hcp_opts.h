#ifndef __HCP_OPTS_H__
#define __HCP_OPTS_H__

#define LIST_NODES   0x0001
#define LIST_PNODES  0x0002
#define LIST_XREFS   0x0004
#define LIST_PICS    0x0008
#define LIST_SYSTEM  0x0010
#define LIST_RXS     0x0020
#define LIST_RX      0x0040
#define LIST_QUIT    0x0080
#define LIST_CLOSE   LIST_QUIT
#define LIST_EOF     0x0100
#define LIST_UNKNOWN 0x8000
#define LIST_ALL (~0)

typedef unsigned long FILE_LINENO;

typedef struct {
	FILE_ID id;
	FILE_LINENO lineno;
} FILE_LOCATION;

typedef struct _uses HCP_USES;
struct _uses {
	char *filename;
	HCP_USES *next;
	FILE_LOCATION source_location;
};


typedef struct {
	gboolean do_list;
	hyp_filetype recompile_format;
	gboolean do_compile;
	gboolean do_help;
	gboolean do_version;
	unsigned int list_flags;
	int verbose;
	int wait_key;
	ssize_t block_size;
	gboolean compression;
	gboolean autoreferences;
	int min_ref_distance;
	gboolean alias_to_index;
	gboolean alabel_to_index;
	gboolean nodes_to_index;
	gboolean gen_index;
	int index_width;
	int compat_flags;
	gboolean read_images;
	gboolean split_lines;
	int tabwidth;
	gboolean title_for_index;
	gboolean caseinsensitive_first;
	int write_references;
	gboolean print_unknown;
	char *error_filename;
	char *output_filename;
	hyp_pic_format pic_format;
	FILE *outfile;
	FILE *errorfile;
	HCP_USES *uses;
	gboolean long_filenames;
	gboolean warn_compat;
	char *image_name_prefix;
	char *output_dir;
	HYP_CHARSET output_charset;
	gboolean bracket_links;
	gboolean all_links;
	gboolean for_cgi;
	int optind;
} hcp_opts;

/*
 * values where options originated from
 */
typedef enum {
	/* option was specified on command line */
	OPTS_FROM_COMMANDLINE,
	/* option was specified in configuration file */
	OPTS_FROM_CONFIG,
	/* option was specified in environment variable, i.e. $HCP_OPT */
	OPTS_FROM_ENV,
	/* option was specified in the input file, i.e. @options */
	OPTS_FROM_SOURCE
} opts_origin;


void hcp_opts_init(hcp_opts *opts);
void hcp_opts_free(hcp_opts *opts);
HCP_USES *hcp_add_uses(HCP_USES **uses, const char *filename);
void hcp_free_uses(HCP_USES **uses);
gboolean hcp_opts_parse(hcp_opts *opts, int argc, const char **argv, opts_origin origin);
gboolean hcp_opts_parse_string(hcp_opts *opts, const char *argstring, opts_origin origin);
hyp_pic_format hcp_pic_format_from_name(const char *name);
const char *hcp_pic_format_to_name(hyp_pic_format format);
void hcp_opts_copy(hcp_opts *opts, const hcp_opts *src);

char *hcp_get_option_string(hcp_opts *opts);

void hcp_usage_error(const char *msg, ...) __attribute__((format(printf, 1, 2)));

#endif /* __HCP_OPTS_H__ */
