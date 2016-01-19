#include "hypdefs.h"
#include <picture.h>

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void FileErrorNr(const char *path, int err_no)
{
	FileError(path, strerror(err_no));
}

/*** ---------------------------------------------------------------------- ***/

void FileErrorErrno(const char *path)
{
	FileErrorNr(path, errno);
}

/*** ---------------------------------------------------------------------- ***/

void err_bmp_rle(const char *path)
{
	FileError(path, _("NYI: BMP RLE8 compression"));
}
