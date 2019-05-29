/*
 * HypView - (c)      - 2019 Thorsten Otto
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, see <http://www.gnu.org/licenses/>.
 */

#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "picture.h"
#include "hcp.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include "cgic.h"
#include "outcomm.h"
#include "outhtml.h"
#include "stat_.h"
#include "../rsc/include/portvdi.h"
#include "../rsc/include/rsrcload.h"
#include "../rsc/src/fileio.h"
#include "../rsc/include/rsc.h"
#include "../rsc/include/ws.h"
#include "cgirsc.h"

/* ------------------------------------------------------------------------- */

gboolean show_resource(const char *filename, hcp_opts *opts, GString *out, _UWORD treenr, hyp_pic_format *pic_format)
{
	html_out_header(NULL, opts, out, _("404 Not Found"), HYP_NOINDEX, NULL, NULL, NULL, TRUE);
	hyp_utf8_sprintf_charset(out, opts->output_charset, "%s: Sorry, displaying of resource files not yet supported\n", hyp_basename(filename));
	html_out_trailer(NULL, opts, out, HYP_NOINDEX, TRUE, FALSE);
	(void)treenr;
	(void)pic_format;
	return FALSE;
}
