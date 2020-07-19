#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "picture.h"
#include "hcp.h"
#include <utime.h>
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include "hv_vers.h"
#include "outcomm.h"
#include "outasc.h"

/*
 * Pure-C is not able to compile the ~20MB file model.c :(
 */
#ifdef __PUREC__
#  define NO_LANGID 1
#endif
#include "llangid.h"

char const gl_program_name[] = "hypinfo";
char const gl_program_version[] = HYP_VERSION;

static gboolean language_only;
static int verbose;
static const char *fix_lang;
static HYP_CHARSET fix_charset;
static gboolean show_tree;
static gboolean fix_tree;
static gboolean preserve;

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void print_version(FILE *out)
{
	char *url = g_strdup_printf(_("%s is Open Source (see %s for further information)."), gl_program_name, HYP_URL);
	char *compiler = hyp_compiler_version();
	char *msg = g_strdup_printf("%s %s\n"
		"%s\n"
		"Using %s\n"
		"%s\n",
		gl_program_name, gl_program_version,
		HYP_COPYRIGHT,
		printnull(compiler),
		printnull(url));
	
	fflush(stdout);
	fflush(stderr);
	hyp_utf8_fprintf(out, "%s", printnull(msg));
	g_free(msg);
	g_free(compiler);
	g_free(url);
}

/* ------------------------------------------------------------------------- */

#if 0
static void print_short_version(FILE *out)
{
	char *msg = g_strdup_printf("%s %s\n"
		"%s\n\n",
		gl_program_name, gl_program_version,
		HYP_COPYRIGHT);
	
	fflush(stdout);
	fflush(stderr);
	hyp_utf8_fprintf(out, "%s", printnull(msg));
	g_free(msg);
}
#endif

/* ------------------------------------------------------------------------- */

static void print_usage(FILE *out)
{
	print_version(out);
	hyp_utf8_fprintf(out, "\n");
	hyp_utf8_fprintf(out, _("usage: %s [-options] file1 [file2 ...]\n"), gl_program_name);
	hyp_utf8_fprintf(out, _("options:\n"));
	hyp_utf8_fprintf(out, _("  -w, --wait              wait for keypress\n"));
	hyp_utf8_fprintf(out, _("  -q, --quiet             be quiet\n"));
	hyp_utf8_fprintf(out, _("  -v, --verbose           be verbose\n"));
	hyp_utf8_fprintf(out, _("  --charset <set>         specify output character set\n"));
	hyp_utf8_fprintf(out, _("  --language              only output language of file\n"));
	hyp_utf8_fprintf(out, _("  -p, --preserve          preserve timestamps\n"));
	hyp_utf8_fprintf(out, _("  -t, --tree              print tree structure\n"));
	hyp_utf8_fprintf(out, _("  --fix-tree              fix @tree header field\n"));
	hyp_utf8_fprintf(out, _("  --fix-lang <lang>       fix @language header field\n"));
	hyp_utf8_fprintf(out, _("  --fix-charset <set>     fix @charset header field\n"));
}

/* ------------------------------------------------------------------------- */

static void warn_if_empty(HYP_DOCUMENT *hyp, hcp_opts *opts)
{
	if (hyp->first_text_page == HYP_NOINDEX)
		hyp_utf8_fprintf(opts->errorfile, _("%s%s does not have any text pages\n"), _("warning: "), hyp->file);
	if (hyp->comp_vers > HCP_COMPILER_VERSION)
		hyp_utf8_fprintf(opts->errorfile, _("%s%s created by compiler version %u\n"), _("warning: "), hyp->file, hyp->comp_vers);
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static char *quote_nodename(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	INDEX_ENTRY *entry;
	size_t namelen;
	char *buf;
	
	entry = hyp->indextable[node];
	namelen = entry->length - SIZEOF_INDEX_ENTRY;
	buf = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
	return buf;
}

/* ------------------------------------------------------------------------- */

static gboolean refinfo(REF_FILE *ref, hcp_opts *opts, gboolean print_filename)
{
	FILE *outfile = opts->outfile;
	char *prefix;
	REF_MODULE *mod;

	prefix = print_filename ? g_strdup_printf("%s: ", ref->filename) : g_strdup("");

	for (mod = ref->modules; mod != NULL; mod = mod->next)
	{
		gboolean language_guessed = mod->language == NULL;

		if (language_only)
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s%s%s\n", prefix, mod->language ? mod->language : "unknown", language_guessed ? _(" (guessed)") : "");
		} else
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@filename: %s\n", prefix, mod->had_filename ? mod->filename : "");
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:name_matches: %d\n", prefix, mod->mod_name_matches);
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:offset: %ld\n", prefix, mod->module_offset);
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:size: %ld\n", prefix, mod->module_len);
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:entries: %ld\n", prefix, mod->num_entries);
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@os: %s\n", prefix, mod->had_os ? hyp_osname(mod->os) : "unknown");
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@charset: %s\n", prefix, mod->had_charset ? hyp_charset_name(mod->charset) : "unknown");
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@lang: %s\n", prefix, mod->language ? mod->language : "unknown");
			if (mod->language != NULL)
			{
				hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:lang_guessed: %d\n", prefix, language_guessed);
			}
			if (mod->database != NULL)
			{
				hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@database: %s\n", prefix, mod->database);
			}
		}
	}

	g_free(prefix);

	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean hypinfo(const char *filename, hcp_opts *opts, gboolean print_filename, LanguageIdentifier *lid)
{
	HYP_DOCUMENT *hyp;
	hyp_filetype type = HYP_FT_NONE;
	int handle;
	FILE *outfile = opts->outfile;
	char *prefix;
	char *str;
	char *newer;
	hyp_nodenr num_nodes;
	hyp_nodenr num_pnodes;
	hyp_nodenr num_images;
	hyp_nodenr num_external;
	hyp_nodenr num_other;
	hyp_nodenr num_eof;
	hyp_nodenr num_unknown;
	hyp_nodenr node;
	INDEX_ENTRY *entry;

	handle = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);

	if (handle < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, filename, hyp_utf8_strerror(errno));
		return FALSE;
	}

	hyp = hyp_load(filename, handle, &type);
	if (hyp == NULL)
	{
		REF_FILE *ref = ref_load(filename, handle, FALSE);
		gboolean ret;

		hyp_utf8_close(handle);
		if (ref != NULL)
		{
			ret = refinfo(ref, opts, print_filename);
			ref_close(ref);
			return ret;
		}
		hyp_utf8_fprintf(opts->errorfile, _("%s: %s: not a HYP file\n"), gl_program_name, filename);
		return FALSE;
	}
	
	if ((hyp->st_guide_flags & STG_ENCRYPTED) && !is_MASTER)
	{
		hyp_unref(hyp);
		hyp_utf8_close(handle);
		hyp_utf8_fprintf(opts->errorfile, _("%s: fatal: protected hypertext: %s\n"), gl_program_name, filename);
		return FALSE;
	}
	
	warn_if_empty(hyp, opts);
	prefix = print_filename ? g_strdup_printf("%s: ", hyp->file) : g_strdup("");
	
	if (hyp->language == NULL)
	{
		const char *lang;
		
		if (hyp->first_text_page == HYP_NOINDEX)
		{
			lang = "empty";
		} else
		{
#ifndef NO_LANGID
			if (lid != NULL)
			{
				GString *out = g_string_new(NULL);
				hyp_nodenr count = 0;
				double prob;
				
				for (node = 0; node < hyp->num_index && count < 10; node++)
				{
					entry = hyp->indextable[node];
					if (node == hyp->index_page)
						continue;
					if (HYP_NODE_IS_TEXT((hyp_indextype) entry->type))
					{
						ascii_out_node(hyp, opts, out, node, NULL);
						count++;
					}
				}
				
				lang = langid_identify(lid, out->str, out->len, &prob);
				g_string_free(out, TRUE);
				hyp->language_guessed = TRUE;
			} else
			{
				lang = NULL;
			}
#else
			UNUSED(lid);
			lang = NULL;
#endif
		}
		hyp->language = g_strdup(lang);
	}
	if (language_only)
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s%s%s\n", prefix, hyp->language ? hyp->language : "unknown", hyp->language_guessed ? _(" (guessed)") : "");
	} else
	{
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@os: %s\n", prefix, hyp_osname(hyp->comp_os));
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@charset: %s\n", prefix, hyp_charset_name(hyp->comp_charset));
	
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@lang: %s\n", prefix, hyp->language ? hyp->language : "unknown");
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:lang_guessed: %d\n", prefix, hyp->language_guessed);
		
		if (hyp->database != NULL)
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@database: %s\n", prefix, hyp->database);
		}
	
		if (hypnode_valid(hyp, hyp->default_page))
		{
			str = quote_nodename(hyp, hyp->default_page);
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@default: %s\n", prefix, str);
			g_free(str);
		}
	
		if (hyp->hcp_options != NULL)
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@options: %s\n", prefix, hyp->hcp_options);
		}
		if (hyp->author != NULL)
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@author: %s\n", prefix, hyp->author);
		}
		if (hypnode_valid(hyp, hyp->help_page))
		{
			str = quote_nodename(hyp, hyp->help_page);
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@help: %s\n", prefix, str);
			g_free(str);
		}
		if (hyp->version != NULL)
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@version: %s\n", prefix, hyp->version);
		}
		if (hyp->subject != NULL)
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@subject: %s\n", prefix, hyp->subject);
		}
		/* if (hyp->line_width != HYP_STGUIDE_DEFAULT_LINEWIDTH) */
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@width: %d\n", prefix, hyp->line_width);
		}
		/* if (hyp->st_guide_flags != 0) */
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:flags: 0x%04x\n", prefix, hyp->st_guide_flags);
		}
		
		if (hyp->hostname != NULL)
		{
			HYP_HOSTNAME *h;
			
			for (h = hyp->hostname; h != NULL; h = h->next)
			{
				hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s@hostname: %s\n", prefix, h->name);
			}
		}
	
		newer = hyp->comp_vers > HCP_COMPILER_VERSION ? g_strdup_printf(" (> %u)", HCP_COMPILER_VERSION) : g_strdup("");
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:hcpversion: %u%s\n", prefix, hyp->comp_vers, newer);
		g_free(newer);

		num_nodes = num_pnodes = num_images = num_external = num_other = num_eof = num_unknown = 0;
		for (node = 0; node < hyp->num_index; node++)
		{
			entry = hyp->indextable[node];
			switch ((hyp_indextype) entry->type)
			{
			case HYP_NODE_INTERNAL:
				num_nodes++;
				break;
			case HYP_NODE_POPUP:
				num_pnodes++;
				break;
			case HYP_NODE_IMAGE:
				num_images++;
				break;
			case HYP_NODE_EXTERNAL_REF:
				num_external++;
				break;
			case HYP_NODE_SYSTEM_ARGUMENT:
			case HYP_NODE_REXX_SCRIPT:
			case HYP_NODE_REXX_COMMAND:
			case HYP_NODE_QUIT:
			case HYP_NODE_CLOSE:
				num_other++;
				break;
			case HYP_NODE_EOF:
				if ((node + 1u) != hyp->num_index)
					hyp_utf8_fprintf(opts->errorfile, _("%s: EOF entry at %u\n"), hyp->file, node);
				else
					hyp_utf8_fprintf(opts->outfile, "%s:eof: %u\n", prefix, node);
				num_eof++;
				break;
			default:
				hyp_utf8_fprintf(opts->errorfile, _("%s: unknown index entry type %u\n"), hyp->file, entry->type);
				num_unknown++;
				break;
			}
		}
	
		if (hyp->hyptree_len == 0 || verbose == 0)
		{
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:hyptree: %u\n", prefix, hyp->hyptree_len);
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:titles: %ld\n", prefix, hyp->hyptree_len && hyp->hyptree_data ? long_from_chars(hyp->hyptree_data) : 0l);
		} else
		{
			long title_len;
			const unsigned char *p;
			hyp_nodenr i, bitlen;
			hyp_nodenr shouldbe;
			long real_titlelen = 0;
			hyp_nodenr mismatch_bits = 0;
			
			p = hyp->hyptree_data;
			title_len = long_from_chars(p);
			shouldbe = SIZEOF_LONG + (((hyp->last_text_page + 16u) >> 4) << 1);
			p += SIZEOF_LONG;
			bitlen = hyp->hyptree_len - SIZEOF_LONG;

			for (node = 0; node < hyp->num_index; node++)
			{
				entry = hyp->indextable[node];
				if (HYP_NODE_IS_TEXT((hyp_indextype) entry->type))
				{
					HYP_NODE *nodeptr;
					gboolean mismatch;
					
					if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
					{
						hyp_node_find_windowtitle(nodeptr);
						if (nodeptr->window_title)
						{
							real_titlelen += strlen((const char *)nodeptr->window_title) + 1;
							mismatch = !hyp_tree_isset(hyp, node);
							if (verbose >= 2)
							{
								char *title = hyp_conv_to_utf8(hyp->comp_charset, nodeptr->window_title, STR0TERM);
								hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "  title:%u%s: %s\n", node, mismatch ? "*" : "", title);
								g_free(title);
							}
						} else
						{
							mismatch = hyp_tree_isset(hyp, node) /* && entry->type != HYP_NODE_POPUP */;
						}
						if (mismatch)
							mismatch_bits++;
						hyp_node_free(nodeptr);
					}
				}
			}

			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL,
				"%s:titles: %ld\n",
				prefix,
				title_len);
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL,
				"%s:hyptree: %u%s%s%s:",
				prefix,
				hyp->hyptree_len,
				(title_len != 0 && shouldbe != hyp->hyptree_len) ? "*l" : "",
				title_len != real_titlelen ?  "*t" : "",
				mismatch_bits != 0 ?  "*b" : "");
			for (i = 0; i < bitlen; i++)
			{
				hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, " %02x", *p);
				p++;
			}
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "\n");
			if (title_len != 0 && shouldbe != hyp->hyptree_len)
			{
				fflush(outfile);
				fprintf(opts->errorfile, _("%shyptree: datalen is %u, should be %u\n"), prefix, hyp->hyptree_len, shouldbe);
				fflush(opts->errorfile);
			}
			if (title_len != real_titlelen)
			{
				fflush(outfile);
				fprintf(opts->errorfile, _("%shyptree: title len is %ld, should be %ld\n"), prefix, title_len, real_titlelen);
				fflush(opts->errorfile);
			}
			if (mismatch_bits != 0)
			{
				fflush(outfile);
				fprintf(opts->errorfile, _("%shyptree: %u errors in title bits\n"), prefix, mismatch_bits);
				fflush(opts->errorfile);
			}
		}
			
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:indexsize: %lu\n", prefix, hyp->itable_size);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:totalnodes: %u\n", prefix, hyp->num_index);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:nodes: %u\n", prefix, num_nodes);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:pnodes: %u\n", prefix, num_pnodes);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:images: %u\n", prefix, num_images);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:external: %u\n", prefix, num_external);
		hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:other: %u\n", prefix, num_other);
		if (num_unknown != 0)
			hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%s:unknown: %u\n", prefix, num_unknown);
	}
			
	g_free(prefix);

	hyp_unref(hyp);
	hyp_utf8_close(handle);

	return TRUE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void set_utime(const char *filename, hcp_opts *opts, struct stat *st)
{
#ifndef _PUREC_SOURCE
	struct utimbuf m;
	m.actime = st->st_atime;
	m.modtime = st->st_mtime;
	if (utime(filename, &m) < 0)
		hyp_utf8_fprintf(opts->errorfile, "%s: %s\n", filename, hyp_utf8_strerror(errno));
#else
	UNUSED(filename);
	UNUSED(opts);
	UNUSED(st);
#endif
}

/* ------------------------------------------------------------------------- */

static void remove_entries(REF_MODULE *mod, hyp_reftype type)
{
	long num;
	size_t size;
	
	for (num = 0; num < mod->num_entries; )
	{
		REF_ENTRY *entry = &mod->entries[num];
		if (entry->type == type)
		{
			unsigned char *pos = (unsigned char *)entry->name.utf8 - 2;
			size = pos[1] + 2;
			mod->module_len -= size;
			memmove(pos, pos + size, mod->module_len - (pos - mod->data));
			mod->num_entries--;
			memmove(entry, entry + 1, (mod->num_entries - num) * sizeof(*entry));
		} else
		{
			num++;
		}
	}
}

/* ------------------------------------------------------------------------- */

static gboolean reffix(REF_FILE *ref, hcp_opts *opts, struct stat *st)
{
	REF_MODULE *mod;
	char *tmpname;
	int ref_handle;

	if (fix_lang == NULL && fix_charset == HYP_CHARSET_NONE)
		return TRUE;
	
	if (ref->modules == NULL)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, ref->filename, _("no modules"));
		return FALSE;
	}
	
	if (ref->modules->next != NULL)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, ref->filename, _("more than 1 module"));
		return FALSE;
	}
	
	{
		char *dirname = hyp_path_get_dirname(ref->filename);
		char *tmp = g_strdup_printf("hy(%u).ref", (int)getpid());
		tmpname = g_build_filename(dirname, tmp, NULL);
		g_free(tmp);
		g_free(dirname);
	}
	
	ref_handle = hyp_utf8_open(tmpname, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, 0644);
	if (ref_handle < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, tmpname, hyp_utf8_strerror(errno));
		g_free(tmpname);
		return FALSE;
	}
	
	/*
	 * write the file header
	 */
	if (!ref_write_header(ref_handle))
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, tmpname, hyp_utf8_strerror(errno));
		goto error;
	}

	for (mod = ref->modules; mod != NULL; mod = mod->next)
	{
		if (fix_lang != NULL)
		{
			if (mod->language)
			{
				remove_entries(mod, REF_LANGUAGE);
			}
			g_free(mod->language);
			mod->language = g_strdup(fix_lang);
		}

		if (fix_charset != HYP_CHARSET_NONE)
		{
			if (mod->had_charset)
			{
				remove_entries(mod, REF_CHARSET);
				mod->had_charset = FALSE;
			}
			mod->charset = fix_charset;
		}

		if (!ref_write_module(ref_handle, mod, opts->verbose))
		{
			hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, tmpname, hyp_utf8_strerror(errno));
			goto error;
		}
	}

	/*
	 * write an empty module header as terminator
	 */
	if (!ref_write_trailer(ref_handle))
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, tmpname, hyp_utf8_strerror(errno));
		goto error;
	}
	hyp_utf8_close(ref_handle);
	ref_handle = -1;

	if (hyp_utf8_unlink(ref->filename) < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, ref->filename, hyp_utf8_strerror(errno));
		goto error;
	}
	if (hyp_utf8_rename(tmpname, ref->filename) < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, ref->filename, hyp_utf8_strerror(errno));
		goto error;
	}
	if (preserve)
	{
		set_utime(ref->filename, opts, st);
	}

	g_free(tmpname);
	return TRUE;

error:
	if (ref_handle > 0)
		hyp_utf8_close(ref_handle);
	if (tmpname)
	{
		hyp_utf8_unlink(tmpname);
		g_free(tmpname);
	}
	return FALSE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean copybytes(FILE *out, int in, long size)
{
	unsigned char *buf;
	long n;
	
	if (size == 0)
		return TRUE;
	buf = g_new(unsigned char, size);
	if (buf == NULL)
		return FALSE;
	n = read(in, buf, size);
	if (n != size)
	{
		g_free(buf);
		if (n >= 0)
			errno = EIO;
		return FALSE;
	}
	n = fwrite(buf, 1, size, out);
	g_free(buf);
	if (n != size)
	{
		if (n >= 0)
			errno = EIO;
		return FALSE;
	}
	return TRUE;
}

__attribute__((noinline))
static gboolean write_ext_header(FILE *outfile, hyp_ext_header type, unsigned short len, void *data)
{
	unsigned char info[2 * SIZEOF_SHORT];
	size_t ret;
	
	short_to_chars(type, info);
	if (len & 1)
		short_to_chars(len + 1, info + 2);
	else
		short_to_chars(len, info + 2);
	ret = fwrite(info, 1, sizeof(info), outfile);
	if (ret != sizeof(info))
		return FALSE;
	if (len != 0)
	{
		ret = fwrite(data, 1, len, outfile);
		if (ret != len)
			return FALSE;
		if (len & 1)
		{
			if (fputc('\0', outfile) != 0)
				return FALSE;
		}
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

__attribute__((noinline))
static gboolean write_ext_header_string(HYP_DOCUMENT *hyp, FILE *outfile, hyp_ext_header type, const char *str)
{
	char *data;
	unsigned short len;
	gboolean converror = FALSE;
	
	if (str == NULL)
		return TRUE;
	data = hyp_utf8_to_charset(hyp->comp_charset, str, STR0TERM, &converror);
	if (data == NULL)
		return FALSE;
	len = (unsigned short)strlen(data);
	if (write_ext_header(outfile, type, len + 1, data) == FALSE)
	{
		g_free(data);
		return FALSE;
	}
	g_free(data);
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean write_index(HYP_DOCUMENT *hyp, FILE *outfile, gboolean update)
{
	size_t ret;
	size_t size;
	hyp_nodenr i;
	INDEX_ENTRY *entry;
	unsigned char rawent[SIZEOF_INDEX_ENTRY];
	
	for (i = 0; i < hyp->num_index; i++)
	{
		entry = hyp->indextable[i];
		switch (entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			break;
		case HYP_NODE_IMAGE:
			if (update)
			{
				entry->previous = 0;
				/* keep next for extending comp_diff, and toc_index for the file format */
			}
			break;
		case HYP_NODE_EXTERNAL_REF:
		case HYP_NODE_SYSTEM_ARGUMENT:
		case HYP_NODE_REXX_SCRIPT:
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_QUIT:
		case HYP_NODE_CLOSE:
		case HYP_NODE_EOF:
			/* reset unused fields for others */
			if (update)
			{
				entry->previous = 0;
				entry->next = 0;
				entry->toc_index = 0;
			}
			break;
		default:
			break;
		}
		rawent[0] = entry->length;
		rawent[1] = entry->type;
		long_to_chars(entry->seek_offset, rawent + 2);
		short_to_chars(entry->comp_diff, rawent + 6);
		short_to_chars(entry->next, rawent + 8);
		short_to_chars(entry->previous, rawent + 10);
		short_to_chars(entry->toc_index, rawent + 12);
		ret = fwrite(rawent, 1, SIZEOF_INDEX_ENTRY, outfile);
		if (ret != SIZEOF_INDEX_ENTRY)
			return FALSE;
		size = entry->length - SIZEOF_INDEX_ENTRY;
		ret = fwrite(entry->name, 1, size, outfile);
		if (ret != size)
			return FALSE;
	}
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean write_header(HYP_DOCUMENT *hyp, FILE *outfile)
{
	unsigned char rawhead[SIZEOF_HYP_HEADER];
	ssize_t ret;
	HYP_HEADER head;
	unsigned char info[SIZEOF_SHORT];
	
	head.magic = HYP_MAGIC_HYP;
	head.itable_size = hyp->itable_size;
	head.itable_num = hyp->num_index;
	head.compiler_os = hyp->comp_os;
	head.compiler_vers = hyp->comp_vers;
	
	long_to_chars(head.magic, rawhead);
	long_to_chars(head.itable_size, rawhead + 4);
	short_to_chars(head.itable_num, rawhead + 8);
	rawhead[10] = head.compiler_vers;
	rawhead[11] = head.compiler_os;
	ret = fwrite(rawhead, 1, SIZEOF_HYP_HEADER, outfile);
	if (ret != SIZEOF_HYP_HEADER)
		return FALSE;
	
	/*
	 * write the index table out. Some of the data is
	 * not yet known and will be updated later
	 */
	if (write_index(hyp, outfile, FALSE) == FALSE)
		return FALSE;
	
	/*
	 * write extra headers
	 */
	
	/* charset first because the other strings are encoded using it */
	
	if (write_ext_header_string(hyp, outfile, HYP_EXTH_CHARSET, hyp_charset_name(hyp->comp_charset)) == FALSE)
		return FALSE;
	
	if (write_ext_header_string(hyp, outfile, HYP_EXTH_DATABASE, hyp->database) == FALSE)
		return FALSE;
	if (write_ext_header_string(hyp, outfile, HYP_EXTH_DEFAULT, hyp->default_name) == FALSE)
		return FALSE;
	{
		HYP_HOSTNAME *h;
		
		for (h = hyp->hostname; h != NULL; h = h->next)
			if (write_ext_header_string(hyp, outfile, HYP_EXTH_HOSTNAME, h->name) == FALSE)
				return FALSE;
	}
	if (write_ext_header_string(hyp, outfile, HYP_EXTH_OPTIONS, hyp->hcp_options) == FALSE)
		return FALSE;
	if (write_ext_header_string(hyp, outfile, HYP_EXTH_AUTHOR, hyp->author) == FALSE)
		return FALSE;
	if (write_ext_header_string(hyp, outfile, HYP_EXTH_VERSION, hyp->version) == FALSE)
		return FALSE;
	if (write_ext_header_string(hyp, outfile, HYP_EXTH_HELP, hyp->help_name) == FALSE)
		return FALSE;
	if (write_ext_header_string(hyp, outfile, HYP_EXTH_SUBJECT, hyp->subject) == FALSE)
		return FALSE;
	if (hyp->hyptree_len != 0)
		if (write_ext_header(outfile, HYP_EXTH_TREEHEADER, hyp->hyptree_len, hyp->hyptree_data) == FALSE)
			return FALSE;
	/*
	 * see hyp_load() for comments about why next two fields are
	 * written in little endian order.
	 */
	short_to_lechars(hyp->st_guide_flags, info);
	if (write_ext_header(outfile, HYP_EXTH_STGUIDE_FLAGS, SIZEOF_SHORT, info) == FALSE)
		return FALSE;
	short_to_lechars(hyp->line_width, info);
	if (write_ext_header(outfile, HYP_EXTH_WIDTH, SIZEOF_SHORT, info) == FALSE)
		return FALSE;
	if (write_ext_header_string(hyp, outfile, HYP_EXTH_LANGUAGE, hyp->language) == FALSE)
		return FALSE;
	if (write_ext_header(outfile, HYP_EXTH_EOF, 0, NULL) == FALSE)
		return FALSE;
	
	fflush(outfile);
	if (ferror(outfile))
		return FALSE;
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean hypfix(const char *filename, hcp_opts *opts)
{
	char *tmpoutname;
	FILE *out;
	HYP_DOCUMENT *hyp;
	hyp_filetype type = HYP_FT_NONE;
	int handle;
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	unsigned long prev_pos, curr_pos, datasize;
	struct stat st;

	handle = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);

	if (handle < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, filename, hyp_utf8_strerror(errno));
		return FALSE;
	}
	memset(&st, 0, sizeof(st));
	rpl_fstat(handle, &st);
	
	hyp = hyp_load(filename, handle, &type);
	if (hyp == NULL)
	{
		REF_FILE *ref = ref_load(filename, handle, FALSE);
		gboolean ret;

		hyp_utf8_close(handle);
		if (ref != NULL)
		{
			ret = reffix(ref, opts, &st);
			ref_close(ref);
			return ret;
		}
		hyp_utf8_fprintf(opts->errorfile, _("%s: %s: not a HYP file\n"), gl_program_name, filename);
		return FALSE;
	}
	
	if ((hyp->st_guide_flags & STG_ENCRYPTED) && !is_MASTER)
	{
		hyp_unref(hyp);
		hyp_utf8_close(handle);
		hyp_utf8_fprintf(opts->errorfile, _("%s: fatal: protected hypertext: %s\n"), gl_program_name, filename);
		return FALSE;
	}
	
	warn_if_empty(hyp, opts);

	tmpoutname = replace_ext(filename, NULL, ".$$$");
	out = hyp_utf8_fopen(tmpoutname, "wb");
	if (out == NULL)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s\n", tmpoutname, hyp_utf8_strerror(errno));
		g_free(tmpoutname);
		return FALSE;
	}

	if (fix_lang != NULL)
	{
		g_free(hyp->language);
		hyp->language = g_strdup(fix_lang);
	}
	
	if (fix_charset != HYP_CHARSET_NONE)
	{
		hyp->comp_charset = fix_charset;
	}
	
	if (fix_tree)
	{
		g_free(hyp->hyptree_data);
		hyp->hyptree_data = NULL;
		hyp->hyptree_len = 0;
		hyp_tree_alloc(hyp);
	}
	
	write_header(hyp, out);
	prev_pos = ftell(out);
	
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		datasize = GetCompressedSize(hyp, node);
		lseek(handle, entry->seek_offset, SEEK_SET);
		if (copybytes(out, handle, datasize) == FALSE)
		{
			hyp_utf8_fprintf(opts->errorfile, "%s: %s\n", tmpoutname, hyp_utf8_strerror(errno));
			goto error;
		}
		curr_pos = ftell(out);
		entry->seek_offset = prev_pos;
		prev_pos = curr_pos;
	}
	hyp->indextable[node]->seek_offset = prev_pos;
	hyp_utf8_close(handle);
	handle = -1;
	
	if (fseek(out, SIZEOF_HYP_HEADER, SEEK_SET) < 0)
		goto error;
	if (write_index(hyp, out, TRUE) == FALSE)
		goto error;
	hyp_utf8_fclose(out);
	out = NULL;
	
	if (hyp_utf8_unlink(filename) < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s\n", filename, hyp_utf8_strerror(errno));
		goto error;
	}
	if (hyp_utf8_rename(tmpoutname, filename) < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s\n", filename, hyp_utf8_strerror(errno));
		goto error;
	}
	if (preserve)
	{
		set_utime(filename, opts, &st);
	}
		
	g_free(tmpoutname);
	hyp_unref(hyp);
	return TRUE;

error:
	if (handle > 0)
		hyp_utf8_close(handle);
	if (out != NULL)
		hyp_utf8_fclose(out);
	hyp_utf8_unlink(tmpoutname);
	g_free(tmpoutname);
	hyp_unref(hyp);
	return FALSE;
}

/* ------------------------------------------------------------------------- */

/*
 *                        |     2502
 * trv_intersec:          |-    251c 2500
 *                        |
 * 
 *                        |
 * trv_intersecminus:     O-
 *                        |
 * 
 *                        |
 * trv_intersecminus_end: O-    2514
 *
 * 
 *                        |
 * trv_intersecplus:      +-
 *                        |
 * 
 *                        |
 * trv_intersecplus_end:  +-
 *
 *                        |
 * trv_end:               L-
 *
 *
 */

#define line_down              "\342\224\202"
#define line_hor               "\342\224\200"
#define line_intersec          "\342\224\234"
#define line_collapsed         "\342\224\234"
#define line_collapsed_end     "\342\224\224"
#define line_expanded          "\342\224\234"
#define line_expanded_end      "\342\224\224"
#define line_end               "\342\224\224"


static void print_indent(FILE *outfile, HYPTREE *tree, hyp_nodenr node)
{
	hyp_nodenr parent;
	hyp_nodenr pparent;
	gboolean is_last;
	
	if (node == 0 || node == HYP_NOINDEX)
		return;
	parent = tree[node].parent;
	print_indent(outfile, tree, parent);
	is_last = FALSE;
	if (parent == HYP_NOINDEX)
	{
		is_last = TRUE;
	} else
	{
		pparent = tree[parent].parent;
		if (pparent == HYP_NOINDEX || tree[pparent].tail == parent)
			is_last = TRUE;
	}
	if (is_last)
	{
		fputs(" ", outfile);
	} else
	{
		fputs(line_down, outfile);
	}
	fputs(" ", outfile);
}

static void lstree(hcp_opts *opts, HYPTREE *tree, hyp_nodenr node, int depth)
{
	FILE *outfile = opts->outfile;
	hyp_nodenr child;
	
	print_indent(outfile, tree, node);

	if (tree[node].num_childs != 0)
	{
		if (tree[node].flags & HYPTREE_IS_EXPANDED)
		{
			if (tree[node].next == HYP_NOINDEX)
				fputs(line_expanded_end, outfile);
			else
				fputs(line_expanded, outfile);
		} else
		{
			if (tree[node].next == HYP_NOINDEX)
				fputs(line_collapsed_end, outfile);
			else
				fputs(line_collapsed, outfile);
		}
	} else
	{
		if (tree[node].next == HYP_NOINDEX)
		{
			fputs(line_end, outfile);
		} else
		{
			fputs(line_intersec, outfile);
		}
	}
	hyp_utf8_fprintf_charset(outfile, opts->output_charset, NULL, "%u: %s\n", node, tree[node].title);
	child = tree[node].head;
	while (child != HYP_NOINDEX)
	{
		lstree(opts, tree, child, depth + 1);
		child = tree[child].next;
	}
}

static gboolean print_tree(const char *filename, hcp_opts *opts)
{
	HYP_DOCUMENT *hyp;
	hyp_filetype type = HYP_FT_NONE;
	int handle;
	HYPTREE *tree;
	
	handle = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);

	if (handle < 0)
	{
		hyp_utf8_fprintf(opts->errorfile, "%s: %s: %s\n", gl_program_name, filename, hyp_utf8_strerror(errno));
		return FALSE;
	}
	
	hyp = hyp_load(filename, handle, &type);
	if (hyp == NULL)
	{
		hyp_utf8_fprintf(opts->errorfile, _("%s: %s: not a HYP file\n"), gl_program_name, filename);
		return FALSE;
	}
	
	if ((hyp->st_guide_flags & STG_ENCRYPTED) && !is_MASTER)
	{
		hyp_unref(hyp);
		hyp_utf8_close(handle);
		hyp_utf8_fprintf(opts->errorfile, _("%s: fatal: protected hypertext: %s\n"), gl_program_name, filename);
		return FALSE;
	}
	
	warn_if_empty(hyp, opts);
	tree = hyp_tree_build(hyp);
	if (tree == NULL)
	{
		hyp_unref(hyp);
		hyp_utf8_close(handle);
		hyp_utf8_fprintf(opts->errorfile, "%s\n", strerror(ENOMEM));
		return FALSE;
	}
	
	hyp_utf8_close(handle);
	handle = -1;
	
	/*
	 * print tree as ascii
	 */
	lstree(opts, tree, 0, 0);

	/*
	 * cleanup
	 */
	hyp_tree_free(hyp, tree);
	hyp_unref(hyp);

	return TRUE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

enum hypinfo_option {
	OPT_HELP = 'h',
	OPT_VERSION = 'V',
	OPT_QUIET = 'q',
	OPT_WAIT = 'w',
	OPT_TREE = 't',
	OPT_VERBOSE = 'v',
	OPT_PRESERVE = 'p',

	OPT_SETVAR = 0,
	OPT_OPTERROR = '?',

	OPT_CHARSET = 1024,
	OPT_LANGUAGE,
	OPT_FIX_LANG,
	OPT_FIX_CHARSET,
	OPT_FIX_TREE
};

static struct option const long_options[] = {
	{ "wait", optional_argument, NULL, OPT_WAIT },
	{ "quiet", no_argument, NULL, OPT_QUIET },
	{ "charset", required_argument, NULL, OPT_CHARSET },
	{ "language", no_argument, NULL, OPT_LANGUAGE },
	{ "verbose", no_argument, NULL, OPT_VERBOSE },
	{ "preserve", no_argument, NULL, OPT_PRESERVE },
	{ "tree", no_argument, NULL, OPT_TREE },
	{ "fix-tree", no_argument, NULL, OPT_FIX_TREE },
	{ "fix-lang", required_argument, NULL, OPT_FIX_LANG },
	{ "fix-charset", required_argument, NULL, OPT_FIX_CHARSET },
	
	{ "help", no_argument, NULL, OPT_HELP },
	{ "version", no_argument, NULL, OPT_VERSION },
	
	{ NULL, no_argument, NULL, 0 }
};
	
static int getopt_on_r(struct _getopt_data *d)
{
	return getopt_switch_r(d) == '+';
}


static gboolean opts_parse(hcp_opts *opts, int argc, const char **argv)
{
	struct _getopt_data *d;
	gboolean retval = TRUE;
	int c;
	
	getopt_init_r(gl_program_name, &d);
	while ((c = getopt_long_r(argc, argv, "pqtvw::hV?", long_options, NULL, d)) != EOF)
	{
		switch ((enum hypinfo_option)c)
		{
		case OPT_QUIET:
			if (getopt_on_r(d))
				opts->verbose++;
			else
				opts->verbose--;
			break;

		case OPT_WAIT:
			if (getopt_arg_r(d) != NULL)
				opts->wait_key = (int)strtol(getopt_arg_r(d), NULL, 0);
			else
				opts->wait_key = 2;
			break;

		case OPT_CHARSET:
			{
				HYP_CHARSET output_charset = hyp_charset_from_name(getopt_arg_r(d));
				if (output_charset == HYP_CHARSET_NONE)
				{
					hcp_usage_error(_("unrecognized character set %s"), getopt_arg_r(d));
					retval = FALSE;
				} else
				{
					opts->output_charset = output_charset;
				}
			}
			break;

		case OPT_LANGUAGE:
			language_only = TRUE;
			break;
		
		case OPT_VERBOSE:
			verbose++;
			break;
		
		case OPT_PRESERVE:
			preserve = TRUE;
			break;
		
		case OPT_TREE:
			show_tree = TRUE;
			break;
		
		case OPT_FIX_TREE:
			fix_tree = TRUE;
			break;
		
		case OPT_FIX_LANG:
			fix_lang = getopt_arg_r(d);
			break;
		
		case OPT_FIX_CHARSET:
			fix_charset = hyp_charset_from_name(getopt_arg_r(d));
			if (fix_charset == HYP_CHARSET_NONE)
			{
				hcp_usage_error(_("unrecognized character set %s"), getopt_arg_r(d));
				retval = FALSE;
			}
			break;
		
		case OPT_HELP:
			opts->do_help = TRUE;
			break;
		case OPT_VERSION:
			opts->do_version = TRUE;
			break;

		case OPT_OPTERROR:
			if (getopt_opt_r(d) == '?')
			{
				opts->do_help = TRUE;
			} else
			{
				retval = FALSE;
			}
			break;
		
		case OPT_SETVAR:
			/* option which just sets a var */
			break;
		
		default:
			/* error message already issued */
			retval = FALSE;
			break;
		}
	}
	
	opts->optind = getopt_ind_r(d);
	getopt_finish_r(&d);
	
	
	return retval;
}


#include "hypmain.h"

int main(int argc, const char **argv)
{
	int c;
	int retval = EXIT_SUCCESS;
	hcp_opts _opts;
	hcp_opts *opts = &_opts;
	int wait_key;
	
	HypProfile_Load(TRUE);
	
	hcp_opts_init(opts);
	if (!hcp_opts_parse_string(opts, gl_profile.hcp.options, OPTS_FROM_CONFIG))
		retval = EXIT_FAILURE;
	if (opts_parse(opts, argc, argv) == FALSE)
		retval = EXIT_FAILURE;
	
	if (retval != EXIT_SUCCESS)
	{
	} else if (opts->do_version)
	{
		print_version(stdout);
	} else if (opts->do_help)
	{
		print_usage(stdout);
	} else
	{
		int num_args;
		
		c = opts->optind;
		num_args = argc - c;

		if (retval == EXIT_SUCCESS && num_args <= 0)
		{
			hcp_usage_error(_("no files specified"));
#if defined(__TOS__) || defined(__atarist__)
			if (empty(argv[0]))
			{
				if (opts->wait_key == 0)
					opts->wait_key = 1;
			} else
			{
				if (opts->wait_key == 1)
					opts->wait_key = 0;
			}
#endif
			retval = EXIT_FAILURE;
		}
		
		if (retval == EXIT_SUCCESS)
		{
			LanguageIdentifier *lid = NULL;
			
			if (opts->output_charset == HYP_CHARSET_NONE)
				opts->output_charset = hyp_get_current_charset();
			while (c < argc)
			{
				const char *filename = argv[c++];
				if (show_tree)
				{
					if (!print_tree(filename, opts))
					{
						retval = EXIT_FAILURE;
					}
				} else if (fix_tree || fix_lang || fix_charset != HYP_CHARSET_NONE)
				{
					if (!hypfix(filename, opts))
					{
						retval = EXIT_FAILURE;
					}
				} else
				{
#ifndef NO_LANGID
					if (lid == NULL)
						lid = langid_get_default_identifier();
#endif
					if (!hypinfo(filename, opts, num_args > 1, lid))
					{
						retval = EXIT_FAILURE;
					}
				}
			}
#ifndef NO_LANGID
			langid_destroy_identifier(lid);
#endif
		}
	}
	
	wait_key = opts->wait_key;
	hcp_opts_free(opts);
	
	if (wait_key == 2 ||
		(wait_key == 1 && retval != EXIT_SUCCESS))
	{
		fflush(stderr);
#if defined(__TOS__) || defined(__atarist__)
		hyp_utf8_printf(_("<press any key>"));
		fflush(stdout);
		Cnecin();
#else
		hyp_utf8_printf(_("<press RETURN>"));
		fflush(stdout);
		getchar();
#endif
	}
	
	HypProfile_Delete();
	x_free_resources();

	return retval;
}
