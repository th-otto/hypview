#include "hv_gtk.h"
#include "hypdebug.h"
#include "hcp_opts.h"
#include "picture.h"
#include "hcp.h"

#include "../hyp/outasc.h"
#include "../hyp/outstg.h"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static gboolean recompile(HYP_DOCUMENT *hyp, hcp_opts *opts, recompile_func func)
{
	gboolean retval;
	char *dir;
	char *output_filename = NULL;
	
	if ((opts->errorfile == NULL || opts->errorfile == stderr) && opts->error_filename != NULL)
	{
		opts->errorfile = hyp_utf8_fopen(opts->error_filename, "w");
		if (opts->errorfile == NULL)
		{
			hyp_utf8_fprintf(stderr, "%s: %s\n", opts->error_filename, strerror(errno));
			return FALSE;
		}
	}

	if ((hyp->st_guide_flags & STG_ENCRYPTED) && !is_MASTER)
	{
		hyp_utf8_fprintf(opts->errorfile, _("%s: fatal: protected hypertext: %s\n"), gl_program_name, hyp->file);
		return FALSE;
	}
	if (hyp->comp_vers > HCP_COMPILER_VERSION)
		hyp_utf8_fprintf(opts->errorfile, _("%s: warning: %s created by compiler version %u\n"), gl_program_name, hyp->file, hyp->comp_vers);
	if ((opts->outfile == NULL || opts->outfile == stdout) && opts->output_filename != NULL)
	{
		output_filename = g_strdup(opts->output_filename);
		opts->outfile = hyp_utf8_fopen(output_filename, "wb");
		if (opts->outfile == NULL)
		{
			FileErrorErrno(output_filename);
			g_free(output_filename);
			return FALSE;
		}
		dir = hyp_path_get_dirname(output_filename);
	} else
	{
		dir = NULL;
	}
	if (empty(dir))
	{
		g_free(dir);
		dir = g_strdup(".");
	}
	if (opts->long_filenames < 0)
		opts->long_filenames = check_long_filenames(dir);
	opts->output_dir = dir;
	
	if (opts->long_filenames)
	{
		g_free(opts->image_name_prefix);
		opts->image_name_prefix = replace_ext(hyp_basename(hyp->file), NULL, "_img_");
	}
	
	if (opts->verbose >= 0 && opts->outfile != stdout)
	{
		hyp_utf8_fprintf(stdout, _("recompiling %s to %s\n"), hyp->file, output_filename);
	}
	
	retval = func(hyp, opts, 0, NULL);
	if (output_filename)
	{
		hyp_utf8_fclose(opts->outfile);
		opts->outfile = NULL;
		g_free(output_filename);
	}
	return retval;
}

/* ------------------------------------------------------------------------- */

gboolean hv_recompile(HYP_DOCUMENT *hyp, const char *output_filename, hyp_filetype type)
{
	hcp_opts _opts;
	hcp_opts *opts = &_opts;
	gboolean retval = TRUE;

	is_MASTER = getenv("TO_MASTER") != NULL;
	
	output_charset = gl_profile.output.output_charset;
	if (output_charset == HYP_CHARSET_NONE)
		output_charset = hyp_get_current_charset();
	hcp_opts_init(opts);
	if (!hcp_opts_parse_string(opts, gl_profile.hcp.options, OPTS_FROM_CONFIG))
		retval = FALSE;
	opts->tabwidth = gl_profile.viewer.ascii_tab_size;
	opts->gen_index = gl_profile.output.output_index;
	opts->output_filename = g_strdup(output_filename);
	opts->bracket_links = gl_profile.output.bracket_links;
	opts->all_links = gl_profile.output.all_links;
	if (retval != FALSE)
	{
		if (opts->read_images)
		{
			/* remove the (already converted) images */
			RemovePictures(hyp, FALSE);
		}
		switch (type)
		{
		case HYP_FT_ASCII:
			retval = recompile(hyp, opts, recompile_ascii);
			break;
		case HYP_FT_STG:
			retval = recompile(hyp, opts, recompile_stg);
			break;
		default:
			retval = FALSE;
			break;
		}
	}
	hcp_opts_free(opts);

	SwitchFont(NULL, TRUE);
	
	return retval;
}
