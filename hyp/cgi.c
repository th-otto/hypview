#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "picture.h"
#include "hcp.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include "hv_vers.h"

char const gl_program_name[] = "hcpview.cgi";
char const gl_program_version[] = HYP_VERSION;

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

#define CMDLINE_VERSION 1

#include "outcomm.h"
#include "outhtml.h"

/* ------------------------------------------------------------------------- */

static const char *hcp_pic_format_to_mimetype(hyp_pic_format format)
{
	switch (format)
	{
		case HYP_PIC_ORIG: break;
		case HYP_PIC_IFF: return "image/x-iff";
		case HYP_PIC_ICN: return "image/x-icn";
		case HYP_PIC_IMG: return "image/x-gem";
		case HYP_PIC_BMP: return "image/bmp";
		case HYP_PIC_GIF: return "image/gif";
		case HYP_PIC_PNG: return "image/png";
	}
	return NULL;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean recompile_html_node(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, hyp_nodenr output_node, hyp_pic_format *pic_format)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	symtab_entry *syms;
	
	ret = TRUE;
	
	if (output_charset == HYP_CHARSET_ATARI && opts->errorfile != stdout)
	{
		hyp_utf8_fprintf(opts->errorfile, _("warning: writing html output in atari encoding might not work with non-atari browsers\n"));
	}

	if (!hypnode_valid(hyp, output_node))
	{
		hyp_utf8_sprintf_charset(out, output_charset, _("node index %u invalid"), output_node);
		return FALSE;
	}
			
	if (opts->read_images && hyp->cache == NULL)
		InitCache(hyp);
	
	/* load REF if not done already */
	syms = ref_loadsyms(hyp);
	
#if 0
	create_patterns();
#endif
	
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			ret &= sym_check_links(hyp, opts, node, &syms);
			break;
		case HYP_NODE_IMAGE:
		case HYP_NODE_EXTERNAL_REF:
		case HYP_NODE_SYSTEM_ARGUMENT:
		case HYP_NODE_REXX_SCRIPT:
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_QUIT:
		case HYP_NODE_CLOSE:
		case HYP_NODE_EOF:
		default:
			break;
		}
	}
	
	node = output_node;
	entry = hyp->indextable[node];
	switch ((hyp_indextype) entry->type)
	{
	case HYP_NODE_INTERNAL:
		ret &= html_out_node(hyp, opts, out, node, syms, FALSE);
		break;
	case HYP_NODE_POPUP:
		ret &= html_out_node(hyp, opts, out, node, syms, FALSE);
		break;
	case HYP_NODE_IMAGE:
		{
			ret &= write_image(hyp, opts, node, HTML_DEFAULT_PIC_TYPE, out);
			if (ret)
			{
				*pic_format = format_from_pic(opts, entry, HTML_DEFAULT_PIC_TYPE);
			} else
			{
				hyp_utf8_sprintf_charset(out, output_charset, "%s", hyp_utf8_strerror(errno));
			}
		}
		break;
	case HYP_NODE_EXTERNAL_REF:
	case HYP_NODE_SYSTEM_ARGUMENT:
	case HYP_NODE_REXX_SCRIPT:
	case HYP_NODE_REXX_COMMAND:
	case HYP_NODE_QUIT:
	case HYP_NODE_CLOSE:
	case HYP_NODE_EOF:
		break;
	default:
		if (opts->print_unknown)
			hyp_utf8_fprintf(opts->errorfile, _("unknown index entry type %u\n"), entry->type);
		break;
	}
	
#ifdef CMDLINE_VERSION
	ClearCache(hyp);
#endif
	
	free_symtab(syms);
	return ret;
}


static gboolean recompile(const char *filename, hcp_opts *opts, GString *out, hyp_nodenr node, hyp_pic_format *pic_format)
{
	gboolean retval;
	HYP_DOCUMENT *hyp;
	char *dir;
	hyp_filetype type = HYP_FT_NONE;
	int handle;
	hcp_opts hyp_opts;
	
	*pic_format = HYP_PIC_UNKNOWN;
	handle = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);

	if (handle < 0)
	{
		hyp_utf8_sprintf_charset(out, output_charset, "%s: %s: %s\n", gl_program_name, filename, hyp_utf8_strerror(errno));
		return FALSE;
	}

	hyp = hyp_load(handle, &type);
	if (hyp == NULL)
	{
		hyp_utf8_close(handle);
		hyp_utf8_sprintf_charset(out, output_charset, _("%s: %s: not a HYP file\n"), gl_program_name, filename);
		return FALSE;
	}
	hyp->file = filename;
	
	if ((hyp->st_guide_flags & STG_ENCRYPTED) && !is_MASTER)
	{
		hyp_unref(hyp);
		hyp_utf8_close(handle);
		hyp_utf8_sprintf_charset(out, output_charset, _("%s: fatal: protected hypertext: %s\n"), gl_program_name, filename);
		return FALSE;
	}
	if (hyp->comp_vers > HCP_COMPILER_VERSION)
		hyp_utf8_sprintf_charset(out, output_charset, _("%s: warning: %s created by compiler version %u\n"), gl_program_name, hyp->file, hyp->comp_vers);
	dir = NULL;
	if (empty(dir))
	{
		g_free(dir);
		dir = g_strdup(".");
	}
	opts->long_filenames = TRUE;
	(void) check_long_filenames;
	(void) recompile_html;
	opts->output_dir = dir;
	if (opts->long_filenames)
	{
		g_free(opts->image_name_prefix);
		opts->image_name_prefix = replace_ext(hyp_basename(filename), NULL, "_img_");
	}
	
	hcp_opts_copy(&hyp_opts, opts);
	if (hyp->hcp_options != NULL)
	{
		hcp_opts_parse_string(&hyp_opts, hyp->hcp_options, OPTS_FROM_SOURCE);
	}
	g_freep(&hyp_opts.output_filename);
	retval = recompile_html_node(hyp, &hyp_opts, out, node, pic_format);
	hyp_opts.outfile = NULL;
	hyp_opts.errorfile = NULL;
	hcp_opts_free(&hyp_opts);
	hyp_unref(hyp);
	hyp_utf8_close(handle);
	g_freep(&opts->output_dir);
	
	return retval;
}

/* ------------------------------------------------------------------------- */

static void html_out_response_header(FILE *out, HYP_CHARSET charset, unsigned long len, hyp_pic_format pic_format)
{
	const char *mimetype;
	
	mimetype = hcp_pic_format_to_mimetype(pic_format);
	if (mimetype)
	{
		fprintf(out, "Content-Type: %s\r\n", mimetype);
	} else
	{
		fprintf(out, "Content-Type: text/html;charset=%s\r\n", hyp_charset_name(charset));
	}
	fprintf(out, "Content-Length: %lu\r\n", len);
	fprintf(out, "\r\n");
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

#include "hypmain.h"

int main(int unused_argc, const char **unused_argv)
{
	int retval = EXIT_SUCCESS;
	hcp_opts _opts;
	hcp_opts *opts = &_opts;
	FILE *out = stdout;
	char *query;
	GString *body;
	hyp_pic_format pic_format = HYP_PIC_UNKNOWN;
	
	UNUSED(unused_argc);
	UNUSED(unused_argv);
	is_MASTER = getenv("TO_MASTER") != NULL;
	
	output_charset = hyp_get_current_charset();
	HypProfile_Load();
	
	hcp_opts_init(opts);
	opts->tabwidth = gl_profile.viewer.ascii_tab_size;
	if (!hcp_opts_parse_string(opts, gl_profile.hcp.options, OPTS_FROM_CONFIG))
		{}
	opts->all_links = !opts->autoreferences;

	g_freep(&opts->error_filename);
	opts->errorfile = stdout;
	g_freep(&opts->output_filename);
	opts->outfile = stdout;
	opts->pic_format = HTML_DEFAULT_PIC_TYPE;
	opts->for_cgi = TRUE;
	
	if (opts->output_charset != HYP_CHARSET_NONE)
		output_charset = opts->output_charset;

	html_init(out);

	body = g_string_new(NULL);

	query = getenv("SCRIPT_NAME");
	if (query)
		cgi_scriptname = query;
	query = getenv("QUERY_STRING");
	if (query == NULL)
	{
		html_out_header(NULL, opts, body, NULL, 0, NULL, NULL);
		g_string_append(body, "no query string\n");
		html_out_trailer(body);
	} else
	{
		const char *arg;
		char *url = NULL;
		char *lang = NULL;
		int hidemenu = 0;
		int hideimages = 0;
		hyp_nodenr node = 0;
		
		arg = strtok(query, "&");
		while (arg != NULL)
		{
			char *val = strchr(arg, '=');
			if (val)
			{
				*val++ = '\0';
				if (strcmp(arg, "url") == 0)
				{
					url = g_strdup(val);
				} else if (strcmp(arg, "lang") == 0)
				{
					lang = g_strdup(val);
				} else if (strcmp(arg, "charset") == 0)
				{
					output_charset = hyp_charset_from_name(val);
					if (output_charset == HYP_CHARSET_NONE)
						output_charset = HYP_CHARSET_UTF8;
				} else if (strcmp(arg, "hidemenu") == 0)
				{
					hidemenu = (int)strtol(val, NULL, 10);
				} else if (strcmp(arg, "hideimages") == 0)
				{
					hideimages = (int)strtol(val, NULL, 10);
				} else if (strcmp(arg, "index") == 0)
				{
					node = (int)strtol(val, NULL, 10);
				}
				*--val = '=';
			}
			arg = strtok(NULL, "&");
		}
		
		if (empty(url))
		{
			html_out_header(NULL, opts, body, NULL, 0, NULL, NULL);
			g_string_append(body, "missing url\n");
			html_out_trailer(body);
		} else
		{
			char *filename = hyp_uri_unescape_string(url, NULL);
			
			stg_nl = "\n";
			html_referer_url = filename;
			opts->read_images = hideimages == 0;
			if (recompile(filename, opts, body, node, &pic_format) == FALSE)
			{
				retval = EXIT_FAILURE;
			}
			g_free(filename);
		}

		g_free(url);
		g_free(lang);
	}
	
	html_out_response_header(out, output_charset, body->len, pic_format);
	write_strout(body, out);
	g_string_free(body, TRUE);
	
	hcp_opts_free(opts);
	
	HypProfile_Delete();
	x_free_resources();

	return retval;
}


/*
HTTP_HOST=127.0.0.2
HTTP_USER_AGENT=Mozilla/5.0 (X11; Linux x86_64; rv:45.0) Gecko/20100101 Firefox/45.0
HTTP_ACCEPT=text/html,application/xhtml+xml,application/xml;q=0.9;q=0.8
HTTP_ACCEPT_LANGUAGE=en-US,en;q=0.8,de-DE;q=0.5,de;q=0.3
HTTP_ACCEPT_ENCODING=gzip, deflate
HTTP_DNT=1
HTTP_REFERER=http://127.0.0.2/hypview/
HTTP_CONNECTION=keep-alive
PATH=/usr/local/OW19/binl:/home/sebilla/bin:/usr/local/bin:/usr/bin:/usr/sbin:/bin:/usr/bin/X11:/usr/X11R6/bin:/var/lib/dosemu:/usr/games:/opt/kde3/bin:/sbin
SERVER_SIGNATURE=
Apache/2.2.17 (Linux/SUSE) Server at 127.0.0.2 Port 80

SERVER_SOFTWARE=Apache/2.2.17 (Linux/SUSE)
SERVER_NAME=127.0.0.2
SERVER_ADDR=127.0.0.2
SERVER_PORT=80
REMOTE_ADDR=127.0.0.1
DOCUMENT_ROOT=/srv/www/htdocs
SERVER_ADMIN=[no address given]
SCRIPT_FILENAME=/srv/www/htdocs/hypview/hypview.cgi
REMOTE_PORT=48150
GATEWAY_INTERFACE=CGI/1.1
SERVER_PROTOCOL=HTTP/1.1
REQUEST_METHOD=GET
QUERY_STRING=url=&hideimages=0&hidemenu=0&dstenc=
REQUEST_URI=/hypview/hypview.cgi?url=&hideimages=0&hidemenu=0&dstenc=
SCRIPT_NAME=/hypview/hypview.cgi
*/
