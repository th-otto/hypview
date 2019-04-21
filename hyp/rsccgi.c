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

char const gl_program_name[] = "rscview.cgi";
char const gl_program_version[] = HYP_VERSION;

static char const cgi_cachedir[] = "cache";

struct curl_parms {
	const char *filename;
	FILE *fp;
	hcp_opts *opts;
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

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean recompile(const char *filename, hcp_opts *opts, GString *out, hyp_nodenr node, hyp_pic_format *pic_format)
{
	UNUSED(node);
	UNUSED(pic_format);
	UNUSED(filename);
	html_out_header(NULL, opts, out, _("404 Not Found"), HYP_NOINDEX, NULL, NULL, NULL, TRUE);
	g_string_append_printf(out,
		_("Sorry, displaying of resource files not yet supported\n"));
	html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE);
	return FALSE;
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

static void html_out_response_header(FILE *out, HYP_CHARSET charset, unsigned long len, hyp_pic_format pic_format, gboolean xml)
{
	const char *mimetype;
	
	mimetype = hcp_pic_format_to_mimetype(pic_format);
	if (mimetype)
	{
		fprintf(out, "Content-Type: %s\015\012", mimetype);
	} else
	{
		fprintf(out, "Content-Type: %s;charset=%s\015\012", xml ? "application/xhtml+xml" : "text/html", hyp_charset_name(charset));
	}
	fprintf(out, "Content-Length: %lu\015\012", len);
	fprintf(out, "\015\012");
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static const char *currdate(void)
{
	struct tm *tm;
	static char buf[40];
	time_t t;
	t = time(NULL);
	tm = localtime(&t);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
	return buf;
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

	hcp_opts_init(opts);

	g_freep(&opts->error_filename);
	opts->errorfile = fopen("hypview.log", "a");
	if (opts->errorfile == NULL)
		opts->errorfile = stderr;
	else
		dup2(fileno(opts->errorfile), 2);
	g_freep(&opts->output_filename);
	opts->outfile = out;
	opts->pic_format = HTML_DEFAULT_PIC_TYPE;
	opts->for_cgi = TRUE;
	opts->hidemenu = FALSE;
	opts->hideimages = FALSE;
	opts->cgi_cached = FALSE;
	opts->showstg = FALSE;
	opts->recompile_format = HYP_FT_HTML;
	
	body = g_string_new(NULL);
	cgiInit(body);

	{
		char *dir = hyp_path_get_dirname(cgiScriptFilename);
		char *cache_dir = g_build_filename(dir, cgi_cachedir, NULL);
		opts->output_dir = g_build_filename(cache_dir, cgiRemoteAddr, NULL);

		if (mkdir(cache_dir, 0750) < 0 && errno != EEXIST)
			fprintf(opts->errorfile, "%s: %s\n", cache_dir, strerror(errno));
		if (mkdir(opts->output_dir, 0750) < 0 && errno != EEXIST)
			fprintf(opts->errorfile, "%s: %s\n", opts->output_dir, strerror(errno));
		
		g_free(cache_dir);
		g_free(dir);
	}
	
	html_init(opts);

	if (cgiAccept && strstr(cgiAccept, "application/xhtml+xml") != NULL)
		opts->recompile_format = HYP_FT_HTML_XML;

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
		char *url = cgiFormString("file");
		char *filename = g_strdup(url);
		char *scheme = empty(filename) ? g_strdup("undefined") : uri_has_scheme(filename) ? g_strndup(filename, strchr(filename, ':') - filename) : g_strdup("file");
		
		if (filename && filename[0] == '/')
		{
			html_referer_url = filename;
			filename = g_strconcat(cgiDocumentRoot, filename, NULL);
		} else if (empty(hyp_basename(filename)) || (!opts->cgi_cached && g_ascii_strcasecmp(scheme, "file") == 0))
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
			if (!opts->cgi_cached)
			{
				html_out_header(NULL, opts, body, _("500 Internal Server Error"), HYP_NOINDEX, NULL, NULL, NULL, TRUE);
				g_string_append(body, _("could not initialize curl\n"));
				html_out_trailer(NULL, opts, body, HYP_NOINDEX, TRUE, FALSE);
				retval = EXIT_FAILURE;
			} else
			{
				char *local_filename;
				
				if (opts->cgi_cached)
				{
					html_referer_url = g_strdup(hyp_basename(filename));
					local_filename = g_build_filename(opts->output_dir, html_referer_url, NULL);
					g_free(filename);
					filename = local_filename;
				}
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
		
		g_string_truncate(body, 0);
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
				local_filename = g_build_filename(opts->output_dir, hyp_basename(filename), NULL);
				fd = mkstemps(local_filename, 4);
				if (fd > 0)
					close(fd);
				}
#elif defined(HAVE_MKSTEMP)
				{
				int fd;
				filename = g_strdup("tmpfile.hyp.XXXXXX");
				local_filename = g_build_filename(opts->output_dir, hyp_basename(filename), NULL);
				fd = mkstemp(local_filename);
				if (fd > 0)
					close(fd);
				}
#else
				filename = g_strdup("tmpfile.hyp.XXXXXX");
				local_filename = g_build_filename(opts->output_dir, hyp_basename(filename), NULL);
				mktemp(local_filename);
#endif
			} else
			{
				local_filename = g_build_filename(opts->output_dir, hyp_basename(filename), NULL);
			}

			hyp_utf8_fprintf(opts->errorfile, "%s: POST from %s, file=%s, size=%d\n", currdate(), fixnull(cgiRemoteHost), hyp_basename(filename), len);

			fp = hyp_utf8_fopen(local_filename, "wb");
			if (fp == NULL)
			{
				const char *err = hyp_utf8_strerror(errno);
				hyp_utf8_fprintf(opts->errorfile, "%s: %s\n", local_filename, err);
				html_out_header(NULL, opts, body, _("404 Not Found"), HYP_NOINDEX, NULL, NULL, NULL, TRUE);
				g_string_append_printf(body, "%s: %s\n", hyp_basename(filename), err);
				html_out_trailer(NULL, opts, body, -1, TRUE, FALSE);
				retval = EXIT_FAILURE;
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
			g_free(local_filename);
		}
		g_free(filename);
		g_freep(&html_referer_url);
	}
	
#if 0
	{
		FILE *tmp = fopen("test.out", "wb");
		if (tmp)
		{
			html_out_response_header(tmp, opts->output_charset, body->len, pic_format, opts->recompile_format == HYP_FT_HTML_XML);
			write_strout(body, tmp);
			fclose(tmp);
		}
	}
#endif
	html_out_response_header(out, opts->output_charset, body->len, pic_format, opts->recompile_format == HYP_FT_HTML_XML);
	cgiExit();
	write_strout(body, out);
	g_string_free(body, TRUE);
	
	g_free(lang);
	hcp_opts_free(opts);
	
	x_free_resources();

#ifdef ENABLE_NLS
	xs_locale_exit();
#endif

	_crtexit();

	(void)recompile_html;
	(void)recompile_stg;
	(void)check_long_filenames;
	(void) is_MASTER;

	return retval;
}
