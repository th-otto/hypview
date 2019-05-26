#include "config.h"
#include <portab.h>
#include <gem.h>
#include <stdarg.h>
#include <errno.h>
#include "fileio.h"
#include "nls.h"
#include "debug.h"


/*** ---------------------------------------------------------------------- ***/

static void error(const char *msg, ...)
{
	va_list args;
	
	va_start(args, msg);
	errout("%s: error: ", program_name);
	erroutv(msg, args);
	va_end(args);
	errout("\n");
}

/*** ---------------------------------------------------------------------- ***/

static void warn(const char *msg, ...)
{
	va_list args;
	
	va_start(args, msg);
	errout("%s: warning: ", program_name);
	erroutv(msg, args);
	va_end(args);
	errout("\n");
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL yorn(void)
{
	fprintf(stdout, " (y/n) ? "); fflush(stdout);
	fprintf(stdout, "y\n");
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void err_fcreate(const char *filename)
{
	error(_("can't create %s: %s"), filename, strerror(errno));
}

/*** ---------------------------------------------------------------------- ***/

void err_fopen(const char *filename)
{
	error(_("can't open %s: %s"), filename, strerror(errno));
}

/*** ---------------------------------------------------------------------- ***/

void err_fread(const char *filename)
{
	error(_("reading %s"), filename);
}

/*** ---------------------------------------------------------------------- ***/

void err_fwrite(const char *filename)
{
	error(_("writing %s"), filename);
}

/*** ---------------------------------------------------------------------- ***/

void err_rename(const char *oldname, const char *newname)
{
	error(_("can't rename %s to %s"), oldname, newname);
}

/*** ---------------------------------------------------------------------- ***/

void err_nota_rsc(const char *filename)
{
	error(_("not a resource file: %s"), filename);
}

/*** ---------------------------------------------------------------------- ***/

void warn_damaged(const char *filename, const char *where)
{
	warn(_("problems in %s while scanning %s"), filename, where);
}

/*** ---------------------------------------------------------------------- ***/

void warn_cicons(void)
{
	warn(_("I couldn't find any color icons although\nthe flag is set the header"));
}

/*** ---------------------------------------------------------------------- ***/

void warn_crc_mismatch(const char *filename, RSC_RSM_CRC header_crc, RSC_RSM_CRC file_crc)
{
	warn(_("%s: CRC $%04x does not match resource file $%04x"), filename, header_crc, file_crc);
}

/*** ---------------------------------------------------------------------- ***/

void warn_crc_string_mismatch(const char *filename)
{
	warn(_("%s: embedded string CRC does not match resource file"), filename);
}

/*** ---------------------------------------------------------------------- ***/

void warn_def_damaged(const char *filename)
{
	warn(_("%s: illegal definition file"), filename);
}

/*** ---------------------------------------------------------------------- ***/

void warn_names_truncated(_WORD maxlen)
{
	warn(_("Names truncated (maxlen = %d)"), maxlen);
}
/*** ---------------------------------------------------------------------- ***/

void warn_interface_flags(const char *filename)
{
	warn(_("%s: some flags have been interpreted as being written by INTRFACE"), filename);
}

/*** ---------------------------------------------------------------------- ***/

_BOOL ask_tree_notfound(_WORD trindex)
{
	fprintf(stdout, _("Tree %d not found. Continue"), trindex);
	return yorn();
}

/*** ---------------------------------------------------------------------- ***/

_BOOL ask_object_notfound(_LONG ob_index, char *tree_name)
{
	fprintf(stdout, _("No object #%ld in tree %s. Continue"), ob_index, tree_name);
	return yorn();
}

/*** ---------------------------------------------------------------------- ***/

void warn_rso_toonew(void)
{
	warn(_("RSO-File created by newer Version of ORCS"));
}
