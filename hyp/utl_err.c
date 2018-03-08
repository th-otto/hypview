#include "hypdefs.h"


void FileError(const char *path, const char *str)
{
	fprintf(stderr, "%s: %s: %s\n", gl_program_name, path, str);
}
