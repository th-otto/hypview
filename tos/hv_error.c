#include "hv_defs.h"
#include "hypview.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void FileError(const char *path, const char *msg)
{
	char *filename;
	char *str;

	filename = hyp_path_get_basename(path);
	str = g_strdup_printf(_("[3][ %s: | File <%s>: | %s ][ OK ]"), gl_program_name, path, msg);
	form_alert(1, str);
	g_free(str);
	g_free(filename);
}

/*** ---------------------------------------------------------------------- ***/

void FileExecError(const char *path)
{
	char *filename = path_subst(path);
	char *str = g_strdup_printf(rs_string(HV_ERR_EXEC), filename);
	form_alert(1, str);
	g_free(str);
	g_free(filename);
}
