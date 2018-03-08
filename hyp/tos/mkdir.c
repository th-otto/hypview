#include "hypdefs.h"
#include <stdlib.h>
#include <sys/types.h>

#include <limits.h>
#include <errno.h>
#include <osbind.h>
#include <mintbind.h>
#include "stat_.h"

#if !defined(__PUREC__) || defined(__USE_GEMLIB)
#define _XltErr(r) ((int)(-(r)))
#else
extern int _XltErr(unsigned int err);
#endif
#ifndef EFAULT
# define EFAULT EINVAL
#endif

int mkdir(const char *_path, mode_t mode)
{
	struct stat statbuf;
	int rv, umask;
	const char *path = _path;

	if (_path == NULL)
	{
		__set_errno(EFAULT);
		return -1;
	}

	rv = rpl_stat(path, &statbuf);	/* Stat directory */
	if (rv == 0)
	{									/* Does it exist ? */
		__set_errno(EEXIST);			/* Yes, so tell user. */
		return -1;
	}

	if (errno != ENOENT)
	{									/* Return stat error, if other than */
		/* File not found. */
		if (errno == ENOTDIR)
			__set_errno(ENOENT);
		return -1;
	}

	rv = Dcreate(path);
	if (rv < 0)
	{
		__set_errno(_XltErr(rv));
		return -1;
	}

	if ((umask = Pumask(0)) != -ETOS_NOSYS)
	{
		(void) Pumask(umask);
		(void) Fchmod(path, (int)(mode & ~umask));
	}
	return 0;
}
