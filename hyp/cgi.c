#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "picture.h"
#include "hcp.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include <curl/curl.h>
#include <sys/time.h>
#include <utime.h>
#include "cgic.h"
#include "hv_vers.h"

char const gl_program_name[] = "hypview.cgi";
char const gl_program_version[] = HYP_VERSION;

static char const cgi_cachedir[] = "cache";

struct curl_parms {
	const char *filename;
	FILE *fp;
	hcp_opts *opts;
	char *cachedir;
};

#define ALLOWED_PROTOS ( \
	CURLPROTO_FTP | \
	CURLPROTO_FTPS | \
	CURLPROTO_HTTP | \
	CURLPROTO_HTTPS | \
	CURLPROTO_SCP | \
	CURLPROTO_SFTP | \
	CURLPROTO_TFTP)

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

#define CGI_VERSION 1
#define CMDLINE_VERSION 1

#include "outcomm.h"
#include "outstg.h"
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
	char *destname;
	
	ret = TRUE;
	
	if (hyp->first_text_page == HYP_NOINDEX)
	{
		html_out_header(NULL, opts, out, _("Corrupt HYP file"), HYP_NOINDEX, NULL, NULL, NULL, TRUE);
		hyp_utf8_sprintf_charset(out, opts->output_charset, _("%s%s does not have any text pages\n"), _("error: "), hyp_basename(hyp->file));
		html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE);
		return FALSE;
	}
	if (!hypnode_valid(hyp, output_node))
	{
		html_out_header(NULL, opts, out, _("Invalid Node index"), output_node, NULL, NULL, NULL, TRUE);
		hyp_utf8_sprintf_charset(out, opts->output_charset, _("node index %u invalid\n"), output_node);
		html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE);
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
	entry = hyp->indextable[output_node];
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
				hyp_utf8_sprintf_charset(out, opts->output_charset, "%s", hyp_utf8_strerror(errno));
			}
		}
		break;
	case HYP_NODE_EXTERNAL_REF:
		html_out_header(NULL, opts, out, "@{ link }", node, NULL, NULL, syms, FALSE);
		destname = html_quote_nodename(hyp, node);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "@{ link \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE);
		break;
	case HYP_NODE_SYSTEM_ARGUMENT:
		html_out_header(NULL, opts, out, "@{ system }", node, NULL, NULL, syms, FALSE);
		destname = html_quote_nodename(hyp, node);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "@{ system \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE);
		break;
	case HYP_NODE_REXX_SCRIPT:
		html_out_header(NULL, opts, out, "@{ rxs }", node, NULL, NULL, syms, FALSE);
		destname = html_quote_nodename(hyp, node);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "@{ rxs \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE);
		break;
	case HYP_NODE_REXX_COMMAND:
		html_out_header(NULL, opts, out, "@{ rx }", node, NULL, NULL, syms, FALSE);
		destname = html_quote_nodename(hyp, node);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "@{ rx \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE);
		break;
	case HYP_NODE_QUIT:
		html_out_header(NULL, opts, out, "@{ quit }", node, NULL, NULL, syms, FALSE);
		destname = html_quote_nodename(hyp, node);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "@{ quit \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE);
		break;
	case HYP_NODE_CLOSE:
		html_out_header(NULL, opts, out, "@{ close }", node, NULL, NULL, syms, FALSE);
		destname = html_quote_nodename(hyp, node);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "@{ close \"%s\"}\n", destname);
		g_free(destname);
		html_out_trailer(NULL, opts, out, node, FALSE, FALSE);
		break;
	case HYP_NODE_EOF:
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

/* ------------------------------------------------------------------------- */

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
		html_out_header(NULL, opts, out, _("404 not found"), HYP_NOINDEX, NULL, NULL, NULL, TRUE);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "%s: %s\n", hyp_basename(filename), hyp_utf8_strerror(errno));
		html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE);
		return FALSE;
	}

	hyp = hyp_load(filename, handle, &type);
	if (hyp == NULL)
	{
		REF_FILE *ref = ref_load(filename, handle, FALSE);
		gboolean ret;
		
		if (ref != NULL)
		{
			char *title = g_strdup_printf(_("Listing of %s"), hyp_basename(filename));
			hyp_utf8_close(handle);
#if 0
			/* TODO: rewrite ref_list to append to body instead of writing to opts->outfile */
			html_out_header(NULL, opts, out, title, HYP_NOINDEX, NULL, NULL, NULL, FALSE);
			ret = ref_list(ref, opts->outfile, opts->list_flags != 0);
			ref_close(ref);
			html_out_trailer(NULL; opts, out, 0, FALSE, FALSE);
#else
			html_out_header(NULL, opts, out, title, HYP_NOINDEX, NULL, NULL, NULL, TRUE);
			ref_close(ref);
			hyp_utf8_sprintf_charset(out, opts->output_charset, "Listing of ref files not yet supported");
			html_out_trailer(NULL, opts, out, 0, FALSE, FALSE);
			ret = TRUE;
#endif
			return ret;
		}
		hyp_utf8_close(handle);
		html_out_header(NULL, opts, out, _("not a HYP file"), HYP_NOINDEX, NULL, NULL, NULL, TRUE);
		hyp_utf8_sprintf_charset(out, opts->output_charset, "%s: %s\n", hyp_basename(filename), _("not a HYP file"));
		html_out_trailer(NULL, opts, out, 0, TRUE, FALSE);
		return FALSE;
	}
	
	if ((hyp->st_guide_flags & STG_ENCRYPTED) && !is_MASTER)
	{
		hyp_unref(hyp);
		hyp_utf8_close(handle);
		html_out_header(NULL, opts, out, _("protected hypertext"), HYP_NOINDEX, NULL, NULL, NULL, TRUE);
		hyp_utf8_sprintf_charset(out, opts->output_charset, _("fatal: %s: %s\n"), _("protected hypertext"), hyp_basename(filename));
		html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE);
		return FALSE;
	}
	if (hyp->comp_vers > HCP_COMPILER_VERSION && opts->errorfile != stdout)
		hyp_utf8_fprintf(opts->errorfile, _("%s%s created by compiler version %u\n"), _("warning: "), hyp->file, hyp->comp_vers);
	dir = NULL;
	if (empty(dir))
	{
		g_free(dir);
		dir = g_strdup(".");
	}
	opts->long_filenames = TRUE;
	(void) check_long_filenames;
	(void) recompile_html;
	(void) recompile_stg;
	opts->output_dir = dir;
	if (opts->long_filenames)
	{
		g_free(opts->image_name_prefix);
		opts->image_name_prefix = replace_ext(hyp_basename(filename), NULL, "_img_");
	}
	
	hcp_opts_copy(&hyp_opts, opts);
	if (hyp->hcp_options != NULL)
	{
		/* TODO: redirect error messages from option parsing to HTML body */
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

static gboolean uri_has_scheme(const char *uri)
{
	gboolean colon = FALSE;
	
	if (uri == NULL)
		return FALSE;
	while (*uri)
	{
		if (*uri == ':')
			colon = TRUE;
		else if (*uri == '/')
			return colon;
		uri++;
	}
	return colon;
}

/* ------------------------------------------------------------------------- */

static void html_out_response_header(FILE *out, HYP_CHARSET charset, unsigned long len, hyp_pic_format pic_format)
{
	const char *mimetype;
	
	mimetype = hcp_pic_format_to_mimetype(pic_format);
	if (mimetype)
	{
		fprintf(out, "Content-Type: %s\015\012", mimetype);
	} else
	{
		fprintf(out, "Content-Type: text/html;charset=%s\015\012", hyp_charset_name(charset));
	}
	fprintf(out, "Content-Length: %lu\015\012", len);
	fprintf(out, "\015\012");
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static size_t mycurl_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	struct curl_parms *parms = (struct curl_parms *) userdata;
	
	if (size == 0 || nmemb == 0)
		return 0;
	if (parms->fp == NULL)
	{
		parms->fp = hyp_utf8_fopen(parms->filename, "wb");
		if (parms->fp == NULL && errno == ENOENT)
		{
			mkdir(parms->cachedir, 0750);
			parms->fp = hyp_utf8_fopen(parms->filename, "wb");
		}
		if (parms->fp == NULL)
			hyp_utf8_fprintf(parms->opts->errorfile, "%s: %s\n", parms->filename, hyp_utf8_strerror(errno));
	}
	if (parms->fp == NULL)
		return 0;
	return fwrite(ptr, size, nmemb, parms->fp);
}

/* ------------------------------------------------------------------------- */

static int mycurl_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userdata)
{
	struct curl_parms *parms = (struct curl_parms *) userdata;

	UNUSED(handle);
	switch (type)
	{
	case CURLINFO_TEXT:
		hyp_utf8_fprintf(parms->opts->errorfile, "== Info: %s", data);
		if (size == 0 || data[size - 1] != '\n')
			fputc('\n', parms->opts->errorfile);
		break;
	case CURLINFO_HEADER_OUT:
		hyp_utf8_fprintf(parms->opts->errorfile, "=> Send header %ld\n", (long)size);
		fwrite(data, 1, size, parms->opts->errorfile);
		break;
	case CURLINFO_DATA_OUT:
		hyp_utf8_fprintf(parms->opts->errorfile, "=> Send data %ld\n", (long)size);
		break;
	case CURLINFO_SSL_DATA_OUT:
		hyp_utf8_fprintf(parms->opts->errorfile, "=> Send SSL data %ld\n", (long)size);
		break;
	case CURLINFO_HEADER_IN:
		hyp_utf8_fprintf(parms->opts->errorfile, "<= Recv header %ld\n", (long)size);
		fwrite(data, 1, size, parms->opts->errorfile);
		break;
	case CURLINFO_DATA_IN:
		hyp_utf8_fprintf(parms->opts->errorfile, "<= Recv data %ld\n", (long)size);
		break;
	case CURLINFO_SSL_DATA_IN:
		hyp_utf8_fprintf(parms->opts->errorfile, "<= Recv SSL data %ld\n", (long)size);
		break;
	default:
		break;
 	}
	return 0;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/*
 * this module does not get any parameters, so we don't need our wrappers */
/* #include "hypmain.h" */

int main(void)
{
	int retval = EXIT_SUCCESS;
	hcp_opts _opts;
	hcp_opts *opts = &_opts;
	FILE *out = stdout;
	GString *body;
	hyp_pic_format pic_format = HYP_PIC_UNKNOWN;
	CURL *curl = NULL;
	char *lang;
	hyp_nodenr node = 0;
	char *val;
	
#ifdef __WIN32__
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif

	_crtinit();

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, xs_get_locale_dir());
	textdomain(GETTEXT_PACKAGE);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif

	HypProfile_Load(FALSE);
	
	hcp_opts_init(opts);
	opts->tabwidth = gl_profile.viewer.ascii_tab_size;
	if (!hcp_opts_parse_string(opts, gl_profile.hcp.options, OPTS_FROM_CONFIG))
		{}
	opts->all_links = !opts->autoreferences;

	g_freep(&opts->error_filename);
	opts->errorfile = fopen("hypview.log", "a");
	if (opts->errorfile == NULL)
		opts->errorfile = stdout;
	else
		dup2(2, fileno(opts->errorfile));
	g_freep(&opts->output_filename);
	opts->outfile = out;
	opts->pic_format = HTML_DEFAULT_PIC_TYPE;
	opts->for_cgi = TRUE;
	opts->hidemenu = FALSE;
	opts->hideimages = FALSE;
	opts->cgi_cached = FALSE;
	opts->showstg = FALSE;
	
	html_init(opts);

	body = g_string_new(NULL);
	cgiInit(body);
	
	if (cgiScriptName)
		cgi_scriptname = cgiScriptName;
	
	lang = cgiFormString("lang");
	
	if ((val = cgiFormString("charset")) != NULL)
	{
		opts->output_charset = hyp_charset_from_name(val);
		g_free(val);
	}
	if (opts->output_charset == HYP_CHARSET_NONE)
		opts->output_charset = HYP_CHARSET_UTF8;
	if ((val = cgiFormString("hidemenu")) != NULL)
	{
		opts->hidemenu = (int)strtol(val, NULL, 10) != 0;
		g_free(val);
	}
	if ((val = cgiFormString("showstg")) != NULL)
	{
		opts->showstg = (int)strtol(val, NULL, 10) != 0;
		g_free(val);
	}
	if ((val = cgiFormString("hideimages")) != NULL)
	{
		opts->hideimages = (int)strtol(val, NULL, 10) != 0;
		g_free(val);
	}
	if ((val = cgiFormString("index")) != NULL)
	{
		node = (int)strtoul(val, NULL, 10);
		g_free(val);
	}
	if ((val = cgiFormString("cached")) != NULL)
	{
		opts->cgi_cached = (int)strtol(val, NULL, 10) != 0;
		g_free(val);
	}
	opts->read_images = !opts->hideimages || opts->showstg;
	
	if (g_ascii_strcasecmp(cgiRequestMethod, "GET") == 0)
	{
		char *url = cgiFormString("url");
		char *filename = g_strdup(url);
		char *scheme = empty(filename) ? g_strdup("undefined") : uri_has_scheme(filename) ? g_strndup(filename, strchr(filename, ':') - filename) : g_strdup("file");
		
		if (filename && filename[0] == '/')
		{
			html_referer_url = filename;
			filename = g_strconcat(cgiDocumentRoot, filename, NULL);
		} else if (empty(filename) || (!opts->cgi_cached && g_ascii_strcasecmp(scheme, "file") == 0))
		{
			/*
			 * disallow file URIs, they would resolve to local files on the WEB server
			 */
			html_out_header(NULL, opts, body, _("403 Forbidden"), HYP_NOINDEX, NULL, NULL, NULL, TRUE);
			g_string_append_printf(body,
				_("Sorry, this type of\n"
				  "<a href=\"http://www.w3.org/Addressing/\">URL</a>\n"
				  "<a href=\"http://www.iana.org/assignments/uri-schemes.html\">scheme</a>\n"
				  "(<q>%s</q>) is not\n"
				  "supported by this service. Please check that you entered the URL correctly.\n"),
				scheme);
			html_out_trailer(NULL, opts, body, HYP_NOINDEX, TRUE, FALSE);
			g_free(filename);
			filename = NULL;
		} else
		{
			if (!opts->cgi_cached &&
				(curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK ||
				(curl = curl_easy_init()) == NULL))
			{
				html_out_header(NULL, opts, body, _("500 Internal Server Error"), HYP_NOINDEX, NULL, NULL, NULL, TRUE);
				g_string_append(body, _("could not initialize curl\n"));
				html_out_trailer(NULL, opts, body, HYP_NOINDEX, TRUE, FALSE);
				retval = EXIT_FAILURE;
			} else
			{
				char *dir;
				char *local_filename;
				struct curl_parms parms;
				struct stat st;
				long unmet;
				long respcode;
				CURLcode curlcode;
				double size;
				char err[CURL_ERROR_SIZE];
				char *content_type;
				
				dir = hyp_path_get_dirname(cgiScriptFilename);
				parms.cachedir = g_build_filename(dir, cgi_cachedir, NULL);
				g_free(dir);
				if (opts->cgi_cached)
				{
					html_referer_url = g_strdup(hyp_basename(filename));
					local_filename = g_build_filename(parms.cachedir, html_referer_url, NULL);
					g_free(filename);
					filename = local_filename;
				} else
				{
					html_referer_url = g_strdup(filename);
					curl_easy_setopt(curl, CURLOPT_URL, filename);
					curl_easy_setopt(curl, CURLOPT_REFERER, filename);
					local_filename = g_build_filename(parms.cachedir, hyp_basename(filename), NULL);
					parms.filename = local_filename;
					parms.fp = NULL;
					parms.opts = opts;
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mycurl_write_callback);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, &parms);
					curl_easy_setopt(curl, CURLOPT_STDERR, opts->errorfile);
					curl_easy_setopt(curl, CURLOPT_PROTOCOLS, ALLOWED_PROTOS);
					curl_easy_setopt(curl, CURLOPT_ENCODING, "");
					curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, (long)1);
					curl_easy_setopt(curl, CURLOPT_FILETIME, (long)1);
					curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, mycurl_trace);
					curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &parms);
					*err = 0;
					curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err);
					
					/* set this to 1 to activate debug code above */
					curl_easy_setopt(curl, CURLOPT_VERBOSE, (long)0);
	
					if (hyp_utf8_stat(local_filename, &st) == 0)
					{
						curlcode = curl_easy_setopt(curl, CURLOPT_TIMECONDITION, (long)CURL_TIMECOND_IFMODSINCE);
						curlcode = curl_easy_setopt(curl, CURLOPT_TIMEVALUE, (long)st.st_mtime);
					}
					
					/*
					 * TODO: reject attempts to connect to local addresses
					 */
					curlcode = curl_easy_perform(curl);
					
					respcode = 0;
					unmet = -1;
					size = 0;
					content_type = NULL;
					curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respcode);
					curl_easy_getinfo(curl, CURLINFO_CONDITION_UNMET, &unmet);
					curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &size);
					curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
					hyp_utf8_fprintf(opts->errorfile, "GET from %s, url=%s, curl=%d, resp=%ld, size=%ld\n", fixnull(cgiRemoteHost), filename, curlcode, respcode, (long)size);
					
					if (parms.fp)
					{
						hyp_utf8_fclose(parms.fp);
						parms.fp = NULL;
					}
					
					if (curlcode != CURLE_OK)
					{
						html_out_header(NULL, opts, body, err, HYP_NOINDEX, NULL, NULL, NULL, TRUE);
						g_string_append_printf(body, "%s:\n%s", _("Download error"), err);
						html_out_trailer(NULL, opts, body, HYP_NOINDEX, TRUE, FALSE);
						unlink(local_filename);
						g_free(local_filename);
						local_filename = NULL;
					} else if ((respcode != 200 && respcode != 304) ||
						(respcode == 200 && (content_type == NULL || strcmp(content_type, "text/plain") != 0)))
					{
						/* most likely the downloaded data will contain the error page */
						parms.fp = hyp_utf8_fopen(local_filename, "rb");
						if (parms.fp != NULL)
						{
							size_t nread;
							
							while ((nread = fread(err, 1, sizeof(err), parms.fp)) > 0)
								g_string_append_len(body, err, nread);
							hyp_utf8_fclose(parms.fp);
						}
						unlink(local_filename);
						g_free(local_filename);
						local_filename = NULL;
					} else
					{
						long ft = -1;
						if (curl_easy_getinfo(curl, CURLINFO_FILETIME, &ft) == CURLE_OK && ft != -1)
						{
							struct utimbuf ut;
							ut.actime = ut.modtime = ft;
							utime(local_filename, &ut);
						}
					}
					g_free(filename);
					filename = local_filename;
				}
				g_free(parms.cachedir);
			}
		}
		if (filename && retval == EXIT_SUCCESS)
		{
			if (recompile(filename, opts, body, node, &pic_format) == FALSE)
			{
				retval = EXIT_FAILURE;
			}
			g_free(filename);
		}
		g_free(scheme);
		g_freep(&html_referer_url);

		g_free(url);
	} else if (g_ascii_strcasecmp(cgiRequestMethod, "POST") == 0 ||
		g_ascii_strcasecmp(cgiRequestMethod, "BOTH") == 0)
	{
		char *filename;
		int len;
		
		filename = cgiFormFileName("file", &len);
		if (filename == NULL || len == 0)
		{
			const char *scheme = "undefined";
			html_out_header(NULL, opts, body, _("403 Forbidden"), HYP_NOINDEX, NULL, NULL, NULL, TRUE);
			g_string_append_printf(body,
				_("Sorry, this type of\n"
				  "<a href=\"http://www.w3.org/Addressing/\">URL</a>\n"
				  "<a href=\"http://www.iana.org/assignments/uri-schemes.html\">scheme</a>\n"
				  "(<q>%s</q>) is not\n"
				  "supported by this service. Please check that you entered the URL correctly.\n"),
				scheme);
			html_out_trailer(NULL, opts, body, HYP_NOINDEX, TRUE, FALSE);
		} else
		{
			char *dir = hyp_path_get_dirname(cgiScriptFilename);
			char *cachedir = g_build_filename(dir, cgi_cachedir, NULL);
			FILE *fp;
			char *local_filename;
			const char *data;
			
			if (*filename == '\0')
			{
				g_free(filename);
#if defined(HAVE_MKSTEMPS)
				{
				int fd;
				filename = g_strdup("tmpfile.XXXXXX.hyp");
				local_filename = g_build_filename(cachedir, hyp_basename(filename), NULL);
				fd = mkstemps(local_filename, 4);
				if (fd > 0)
					close(fd);
				}
#elif defined(HAVE_MKSTEMP)
				{
				int fd;
				filename = g_strdup("tmpfile.hyp.XXXXXX");
				local_filename = g_build_filename(cachedir, hyp_basename(filename), NULL);
				fd = mkstemp(local_filename);
				if (fd > 0)
					close(fd);
				}
#else
				filename = g_strdup("tmpfile.hyp.XXXXXX");
				local_filename = g_build_filename(cachedir, hyp_basename(filename), NULL);
				mktemp(local_filename);
#endif
			} else
			{
				local_filename = g_build_filename(cachedir, hyp_basename(filename), NULL);
			}
			g_free(dir);

			hyp_utf8_fprintf(opts->errorfile, "POST from %s, file=%s, size=%d\n", fixnull(cgiRemoteHost), hyp_basename(filename), len);

			fp = hyp_utf8_fopen(local_filename, "wb");
			if (fp == NULL && errno == ENOENT)
			{
				mkdir(cachedir, 0750);
				fp = hyp_utf8_fopen(local_filename, "wb");
			}

			if (fp == NULL)
			{
				hyp_utf8_fprintf(opts->errorfile, "%s: %s\n", local_filename, hyp_utf8_strerror(errno));
			} else
			{
				data = cgiFormFileData("file", &len);
				fwrite(data, 1, len, fp);
				fclose(fp);
				opts->cgi_cached = TRUE;
				html_referer_url = g_strdup(filename);
				if (recompile(local_filename, opts, body, node, &pic_format) == FALSE)
				{
					retval = EXIT_FAILURE;
				}
			}
			g_free(cachedir);
			g_free(local_filename);
		}
		g_free(filename);
		g_freep(&html_referer_url);
	}
	
	html_out_response_header(out, opts->output_charset, body->len, pic_format);
	cgiExit();
	write_strout(body, out);
	g_string_free(body, TRUE);
	
	g_free(lang);
	hcp_opts_free(opts);
	
	if (curl)
	{
		curl_easy_cleanup(curl);
		curl_global_cleanup();
	}
	
	HypProfile_Delete();
	x_free_resources();

#ifdef ENABLE_NLS
	xs_locale_exit();
#endif

	_crtexit();

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
PATH=/usr/bin:/bin:/usr/sbin:/sbin
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
