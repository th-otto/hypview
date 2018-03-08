#include "hypdefs.h"
#include <stdlib.h>
#include <sys/types.h>

#include <limits.h>
#include <errno.h>
#include <osbind.h>
#include <mintbind.h>
#include <dirent.h>

extern int __libc_unix_names;
extern ino_t __inode;
#if !defined(__PUREC__) || defined(__USE_GEMLIB)
#define _XltErr(r) ((int)(-(r)))
#else
extern int _XltErr(unsigned int err);
#endif
extern int _enoent (const char *path);
extern int _unx2dos(const char *unx, char *dos, size_t len);
extern int _dos2unx(const char *dos, char *unx, size_t len);


/* Important note: Metados can have opendir for some device and do not
 * have it for others, so there is no point in having a status variable
 * saying there is an opendir call. Check this every time.
 */
 

/* a new value for DIR->status, to indicate that the file system is not
 * case sensitive.
 */
#define _NO_CASE  8

 
DIR *opendir(const char *uname)
{
	DIR *d;
	long r;
	_DTA *olddta;
	char namebuf[PATH_MAX];
	char dirpath[PATH_MAX];
	char *p;
	const char* name = uname;
	
	d = malloc(sizeof(DIR));
	if (!d) {
		__set_errno (ENOMEM);
		return NULL;
	}

	if (!__libc_unix_names)
	  {
	    _unx2dos(uname, namebuf, sizeof (namebuf));
	    name = namebuf;
	  }
	  
	r = Dopendir(name, 0);
	if (r != -ETOS_NOSYS) {
		if ( (r & 0xff000000L) == 0xff000000L ) {
			if ((r == -ETOS_NOTDIR) && (_enoent(name)))
				r = -ETOS_NOENT;
			free(d);
			__set_errno (_XltErr((int)r));
			return NULL;
		}
		else {
			d->handle = r;
			d->buf.d_off = 0;

			/* test if the file system is case sensitive */
			r = Dpathconf(name, 6);
			if (r == 1 || r == -ETOS_NOSYS)
				d->status = _NO_CASE;
			else
				d->status = 0;
			return d;
		}
	}
	d->handle = 0xff000000L;  /* indicate that the handle is invalid */

/* TOS emulation routines */

	if (name != namebuf)
		strcpy(namebuf, name);
	p = namebuf;
	if (p) {
	/* find the end of the string */
		for (p = namebuf; *p; p++) ;

	/* make sure the string ends in '\' */
		if (*(p-1) != '\\') {
			*p++ = '\\';
		}
	}

	strcpy(p, "*.*");
	olddta = Fgetdta();
	Fsetdta(&(d->dta));
	r = Fsfirst(namebuf, 0x17);
	Fsetdta(olddta);

	if (r == 0) {
		d->status = _STARTSEARCH;
	} else if (r == -ETOS_NOENT) {
		d->status = _NMFILE;
	} else {
		free(d);
		__set_errno (_XltErr((int)r));
		return 0;
	}
	d->buf.d_off = 0;
/* for rewinddir: if necessary, build a relative path */
	if (namebuf[1] == ':') {	/* absolute path, no problem */
		dirpath[0] = 0;
	} else {
		dirpath[0] = Dgetdrv() + 'A';
		dirpath[1] = ':';
		dirpath[2] = 0;
		if (*namebuf != '\\')
			(void)Dgetpath(dirpath+2, 0);
	}
	d->dirname = malloc(strlen(dirpath)+strlen(namebuf)+1);
	if (d->dirname) {
		strcpy(d->dirname, dirpath);
		strcat(d->dirname, namebuf);
	}
	return d;
}


int closedir(DIR *dirp)
{
	long r;

	/* The GNU libc closedir returns gracefully if a NULL pointer is
	   passed.  We follow here.  */
	if (dirp == NULL) {
		__set_errno (EBADF);
		return -1;
	}
	
	if (dirp->handle != (long)0xff000000L)
		r = Dclosedir(dirp->handle);
	else
		r = 0;
	free(dirp);
	if (r == -ETOS_NOSYS) {
		/* hmm, something went wrong, just ignore it. */
	} else if (r < 0)
	{
		__set_errno(_XltErr((int)r));
		return -1;
	}
	return 0;
}


struct dirent *readdir(DIR *d)
{
	struct dbuf {
		long ino;
		char name[NAME_MAX + 1];
	} dbuf;
	long r;
	_DTA *olddta;
	struct dirent *dd = &d->buf;

	if (d == NULL) {
		__set_errno (EBADF);
		return NULL;
	}
	
	if (d->handle != (long) 0xff000000L)  {
		/* The directory descriptor was optained by calling Dopendir(), as
		 * there is a valid handle.
		 */
		r = (int)Dreaddir((int)(NAME_MAX+1+sizeof(long)), d->handle, (char *) &dbuf);
		if (r == -ETOS_NMFILES)
			return NULL;
		else if (r) {
			__set_errno (_XltErr((int)r));
			return NULL;
		}
		else {
			dd->d_ino = dbuf.ino;
			dd->d_off++;
			dd->d_namlen = (short)strlen(dbuf.name);
			strcpy(dd->d_name, dbuf.name);

			/* if file system is case insensitive, transform name to lowercase */
			if (d->status == _NO_CASE)
				strlwr(dd->d_name);

			return dd;
		}
	}
/* ordinary TOS search, using Fsnext. Note that the first time through,
 * Fsfirst has already provided valid data for us; for subsequent
 * searches, we need Fsnext.
 */
	if (d->status == _NMFILE)
		return 0;
	if (d->status == _STARTSEARCH) {
		d->status = _INSEARCH;
	} else {
		olddta = Fgetdta();
		Fsetdta(&(d->dta));
		r = Fsnext();
		Fsetdta(olddta);
		if (r == -ETOS_NMFILES) {
			d->status = _NMFILE;
			return NULL;
		} else if (r) {
			__set_errno (_XltErr((int)r));
			return NULL;
		}
	}
	dd->d_ino = __inode++;
	dd->d_off++;
	_dos2unx(d->dta.dta_name, dd->d_name, sizeof (dd->d_name));
	dd->d_namlen = (short)strlen(dd->d_name);
	return dd;
}
