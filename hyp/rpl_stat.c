/*
 * Work around platform bugs in stat.
 */
#include "hypdefs.h"
#include <limits.h>
#include <unistd.h>


#if defined(__PUREC__) && !defined(_GNUC_SOURCE) && !defined(_MINTLIB_SOURCE)

#include <sys/ioctl.h>


/*
 * undocumented function in PCSTDLIB.LIB;
 * translates TOS error codes to unix style error
 * codes used by the library.
 */
#if !defined(__PUREC__) || defined(__USE_GEMLIB)
#define _XltErr(r) ((int)(-(r)))
#else
extern int _XltErr(unsigned int err);
#endif

/*
 * functions taken from MiNTlib
 */
int __libc_unix_names = 0;
ino_t __inode = 32;
int __mint;
static int _x_Bit_set_in_stat = 0;
static char _rootdir;
static uid_t __uid;
static gid_t __gid;

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

uid_t getuid(void)
{
	long r;

	r = Pgetuid();
	if (r != -ETOS_NOSYS)
		return (uid_t) r;
	return __uid;
}

/* ------------------------------------------------------------------------- */

uid_t geteuid(void)
{
	long r;

	r = Pgeteuid();
	if (r != -ETOS_NOSYS)
		return (uid_t) r;

	return getuid();
}

/* ------------------------------------------------------------------------- */

gid_t getgid(void)
{
	long r;

	r = Pgetgid();
	if (r != -ETOS_NOSYS)
		return (gid_t) r;

	return __gid;
}

/* ------------------------------------------------------------------------- */

gid_t getegid(void)
{
	long r;

	r = Pgetegid();
	if (r != -ETOS_NOSYS)
		return (gid_t) r;

	return getgid();
}

/* ------------------------------------------------------------------------- */

pid_t getpid(void)
{
	long r;

	r = Pgetpid();
	if (r != -ETOS_NOSYS)
		return (pid_t) r;

	return ((pid_t) (((long) _base) >> 8));
}

/* ------------------------------------------------------------------------- */

/* date for files (like root directories) that don't have one */
#define OLDDATE __unixtime(0,0)

/* Convert a GEMDOS time to seconds since the epoch.
   The calculated value is always in UTC. */
static time_t __unixtime(unsigned timestamp, unsigned datestamp)
{
	struct tm tmbuf;
	register struct tm *tm = &tmbuf;

	tm->tm_sec = (timestamp & 31) << 1;
	tm->tm_min = (timestamp >> 5) & 63;
	tm->tm_hour = (timestamp >> 11) & 31;

	tm->tm_mday = datestamp & 31;
	tm->tm_mon = ((datestamp >> 5) & 15) - 1;
	tm->tm_year = 80 + ((datestamp >> 9) & 127);

	return mktime(tm);
}

/* ------------------------------------------------------------------------- */

/* 
 * stat system call wrapper
 * 
 * first try Fstat64, then fallback to Fxattr and convert
 * to struct stat
 */
static long __sys_stat(const char *path, struct stat *st, int lflag, int exact)
{
	long r;

	/* first try the native syscall */
	r = Fstat64(lflag, path, st);
	if (r == -ETOS_NOSYS || r == -ETOS_INVAL)
	{
		/* fall back to Fxattr */
		struct xattr xattr;

		r = Fxattr(lflag, path, &xattr);
		if (r == 0)
		{
			memset(st, 0, sizeof(*st));

			__quad_make(st->st_dev, 0, xattr.st_dev);
			st->st_ino = (__ino_t) xattr.st_ino;
			st->st_mode = (__mode_t) xattr.st_mode;
			st->st_nlink = (__nlink_t) xattr.st_nlink;
			st->st_uid = (__uid_t) xattr.st_uid;
			st->st_gid = (__gid_t) xattr.st_gid;
			__quad_make(st->st_rdev, 0, xattr.st_rdev);

			if (exact)
			{
				union
				{
					unsigned short s[2];
					unsigned long l;
				} data;

				data.l = xattr.st_mtime;
				st->st_mtime = __unixtime(data.s[0], data.s[1]);
				data.l = xattr.st_atime;
				st->st_atime = __unixtime(data.s[0], data.s[1]);
				data.l = xattr.st_ctime;
				st->st_ctime = __unixtime(data.s[0], data.s[1]);
			}

			st->st_size = (__off_t) xattr.st_size;
			st->st_blocks = (__off_t) (((__off_t) xattr.st_blocks * (__off_t) xattr.st_blksize) >> 9);
			st->st_blksize = xattr.st_blksize;
			/* st->st_flags = 0; */
			/* st->st_gen = 0; */
		}
	}

	return r;
}

/* ------------------------------------------------------------------------- */

int _enoent(const char *path)
{
	register const char *s;
	long oldmask;
	int dir_seen = 0;
	char *tmp = NULL;

	if (!path)
		return 0;

	for (s = path; *s; s++)
		/* nop */;

	oldmask = Psigblock(~0L);

	for ( ; s != path; s--)
	{
		if (*s == '\\' || *s == '/')
		{
			struct stat st;
			long r;

			dir_seen = 1;

			if (!tmp)
			{
				tmp = malloc(s - path + 1);
				if (!tmp)
					return 0;

				strncpy(tmp, path, s - path);
			}

			tmp[s - path] = '\0';
			r = __sys_stat (tmp, &st, 0, 0);

			if (r == -ETOS_NOSYS 
			    || (r == 0 && ((st.st_mode & S_IFMT) != S_IFDIR)))
			{
				if (tmp)
					free(tmp);

				(void) Psigsetmask (oldmask);

				/* Either we don't have Fstat or existing
				 * non-directory in PATH.  ENOTDIR is ok in
				 * either case.
				 */
				return 0;
			}
		}
	}

	if (tmp)
		free(tmp);

	(void) Psigsetmask (oldmask);
	return dir_seen; /* should have been ENOENT */
}

/* ------------------------------------------------------------------------- */

int _unx2dos(const char *unx, char *dos, size_t len)
{
	register int unx_length = (int)strlen(unx);
	register int count = 0;
	const char *u;
	char *d, c;

	dos[0] = 0;
	len--;								/* for terminating NUL */
	u = unx;
	d = dos;
	if (!strncmp(u, "/dev/", 5))
	{
		u += 5;
		/* make /dev/A/foo the same as A:/foo */

		if (*u && isalpha(*u) && (u[1] == 0 || (u[1] == '/' || u[1] == '\\')))
		{
			d[0] = *u++;
			d[1] = ':';
			d += 2;
			len -= 2;
		}
		/* check for a unix device name */
		else if (__mint)
		{
			strcpy(d, "U:\\dev\\");
			d += 7;
			len -= 7;
		} else
		{
			strncpy(d, u, len);
			len -= strlen(u);
			strncat(d, ":", len);
			if (!strcmp(d, "tty:"))
				strcpy(d, "con:");
			return 1;
		}
	} else if (__mint && !strncmp(u, "/pipe/", 6))
	{
		u += 6;
		strcpy(d, "U:\\pipe\\");
		d += 8;
		len -= 8;
	} else if (*u == '/' && _rootdir)
	{
		*d++ = _rootdir;
		*d++ = ':';
		len -= 2;
	}

	while ((c = *u++) != 0)
	{
		count++;
		if (c == '/')
			c = '\\';
		*d++ = c;
		len--;
		if (len == 0)
		{
			if (count < unx_length)
			{
				__set_errno(ENAMETOOLONG);
				*d = 0;
				return -1;
			}
			break;
		}
	}
	*d = 0;
	return 0;
}

/* ------------------------------------------------------------------------- */

int _dos2unx(const char *dos, char *unx, size_t len)
{
	register int dos_length = (int)strlen(dos);
	register int count = 0;
	register char c;

	len--;								/* for terminating NUL */
	/* replace A:\x with /dev/a/x,
	 * replace A:\x with /x, if _rootdir is 'a',
	 * replace A:\x with /a/x, if _rootdir is 'u'.
	 * BUG/FEATURE: A:x is converted to A:\x, you lose the feature
	 *              of one current directory per device.
	 *              This is because we assume that /dev/a/x is always 
	 *              an absolute path.
	 */
	if (*dos && dos[1] == ':')
	{
		register char dev = tolower(*dos);

		dos += 2;
		if (dev != _rootdir)
		{
			if (_rootdir != 'u')
			{
				*unx++ = '/';
				*unx++ = 'd';
				*unx++ = 'e';
				*unx++ = 'v';
				len -= 4;
			}
			*unx++ = '/';
			*unx++ = dev;
			len -= 2;
		}
		if (*dos != '/' && *dos != '\\')
		{
			*unx++ = '/';
			len--;
		}
	}
	/* convert slashes
	 */
	while ((c = *dos++) != 0)
	{
		count++;
		if (c == '\\')
			c = '/';
		else if (__mint < 7)
			c = tolower(c);
		*unx++ = c;
		len--;
		if (len == 0)
		{
			if (count < dos_length)
			{
				__set_errno(ENAMETOOLONG);
				*unx = 0;
				return -1;
			}
			break;
		}
	}
	*unx = 0;
	return 0;
}

/* ------------------------------------------------------------------------- */

static int __do_stat(const char *_path, struct stat *st, int lflag)
{
	char pathbuf[PATH_MAX];
	char *path = (char *) _path;
	int nval;
	long r;

	if (!_path)
	{
		__set_errno(EFAULT);
		return -1;
	}

	if (*_path == '\0')
	{
		__set_errno(ENOENT);
		return -1;
	}

	if (__libc_unix_names)
	{
		nval = 0;
	} else
	{
		/* _unx2dos returns 1 for device names (like /dev/con) */
		path = pathbuf;
		nval = _unx2dos(_path, path, sizeof(pathbuf));
	}

	r = __sys_stat(path, st, lflag, 1);
	if (r != -ETOS_NOSYS)
	{
		if (r)
		{
			if ((r == -ETOS_NOTDIR) && _enoent(path))
			{
				r = -ETOS_NOENT;
			}
			__set_errno(_XltErr((int)r));
			return -1;
		}
		return 0;
	}

	{
		_DTA *olddta;
		char *ext, drv;
		char *prevdir = NULL;
		int fd;
		short magic;
		_DTA d;
		int isdot = 0;
		int isdir = 0;

		memset(st, 0, sizeof(*st));

		/* otherwise, check to see if we have a name like CON: or AUX: */
		if (nval == 1)
		{
			st->st_mode = S_IFCHR | 0600;
			st->st_flags = 0;
			st->st_ino = __inode++;
			__quad_make(st->st_rdev, 0, 0);
			st->st_mtime = st->st_ctime = st->st_atime = time((time_t *) 0) - 2;
			__quad_make(st->st_dev, 0, 0);
			st->st_nlink = 1;
			st->st_uid = geteuid();
			st->st_gid = getegid();
			st->st_size = st->st_blocks = 0;
			st->st_blksize = 1024;
			return 0;
		}

		/* A file name: check for root directory of a drive */
		if (path[0] == '\\' && path[1] == 0)
		{
			drv = Dgetdrv() + 'A';
			isdir = 1;
			goto rootdir;
		}

		if (((drv = path[0]) != 0) && path[1] == ':' && (path[2] == 0 || (path[2] == '\\' && path[3] == 0)))
		{
		  rootdir:
			isdir = 1;
			st->st_mode = S_IFDIR | 0755;
			st->st_flags = 0;
			__quad_make(st->st_dev, 0, isupper(drv) ? drv - 'A' : drv - 'a');
			st->st_ino = 2;
			st->st_mtime = st->st_ctime = st->st_atime = OLDDATE;
			goto fill_dir;
		}

		/* forbid wildcards in path names */
		if (strchr(path, '*') || strchr(path, '?'))
		{
			__set_errno(ENOENT);
			return -1;
		}

		/*
		 * OK, here we're going to have to do an Fsfirst to get the date
		 * 
		 * NOTE: Fsfirst(".",-1) or Fsfirst("..",-1) both fail under TOS,
		 * so we kludge around this by using the fact that Fsfirst(".\*.*"
		 * or "..\*.*" will return the correct file first (except, of course,
		 * in root directories :-( ).
		 * NOTE2: Some versions of TOS don't like Fsfirst("RCS\\", -1) either,
		 * so we do the same thing if the path ends in '\\'.
		 */

		/* Find the end of the string, and previous directory for kludging */
		for (ext = path; ext[0] && ext[1]; ext++)
		{
			if (ext[1] && ext[1] != '.')
			{
				if (ext[0] == '\\')
				{
					prevdir = ext;
				}
			}
		}

		/* Add appropriate kludge if necessary. */

		/* Handle C:\XXXX\. */
		if (*ext == '.' && (ext == path || ext[-1] == '\\'))
		{
			isdot = 1;
			strcat(path, "\\*.*");
		}
		/* Now, Handle C:\XXXX\.. */
		else if (*ext == '.' && (ext == path || ext[-1] == '.'))
		{
			/*
			 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			 * FIXME.
			 * Need to handle recursively, such as....
			 * C:\XXXX\YYYY\ZZZZ\..\..\..
			 *
			 * Also, need to handle non-rooted drives such as...
			 * ..\..\.., where the CWD needs to be retrieved.
			 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			 */

			isdot = 1;
			if (prevdir)
			{
				/* 
				 * In the case of C:\XXXX\YYYY\.., we now have....
				 * C:\XXXX\*.*
				 */
				strcpy(prevdir, "\\*.*\0");
			} else
			{
				/*
				 * In the case of C:\.., we now have....
				 * C:\*.*
				 */
				strcpy(&ext[-2], "\\*.*\0");
			}
		}
		/* Finally, Handle C:\XXXX\ */
		else if (*ext == '\\')
		{
			isdot = 1;
			strcat(path, "*.*");
		}
		olddta = Fgetdta();
		Fsetdta(&d);
		r = Fsfirst(path, 0xff);
		Fsetdta(olddta);
		if (r < 0)
		{
			/* 
			 * This is incorrect. When Fsfirst fails for things such as
			 * C:\\FOO\\ and appends *.*, to become C:\\FOO\\*.*, and
			 * we get ENOENT, why did we say it was a directory and return
			 * success ???
			 *
			 * Commenting out. See bug....
			 * http://sparemint.atariforge.net/bugtracker/view.php?id=191
			 *
			 * if (isdot && r == -ENOENT) goto rootdir;
			 */
			__set_errno(ENOENT);
			return -1;
		}

		if (isdot && ((d.dta_name[0] != '.') || (d.dta_name[1])))
		{
			goto rootdir;
		}

		st->st_mtime = st->st_ctime = st->st_atime = __unixtime(d.dta_time, d.dta_date);
		if (((drv = *path) != 0) && path[1] == ':')
			__quad_make(st->st_dev, 0, toupper(drv) - 'A');
		else
			__quad_make(st->st_dev, 0, Dgetdrv());

		isdir = (d.dta_attribute & FA_DIR) != 0;

		st->st_ino = __inode++;
		st->st_flags = 0;
		st->st_mode = 0644 | (isdir ? S_IFDIR | 0111 : S_IFREG);

		if (st->st_flags & FA_RDONLY)
			st->st_mode &= ~0222;		/* no write permission */
		if (st->st_flags & FA_HIDDEN)
			st->st_mode &= ~0444;		/* no read permission */

		/* check for a file with an executable extension */
		ext = strrchr(_path, '.');
		if (ext)
		{
			if (!strcmp(ext, ".app") ||
				!strcmp(ext, ".gtp") ||
				!strcmp(ext, ".ttp") ||
				!strcmp(ext, ".prg") ||
				!strcmp(ext, ".tos"))
			{
				st->st_mode |= 0111;
			}
		}
		if ((st->st_mode & S_IFMT) == S_IFREG)
		{
			if (_x_Bit_set_in_stat)
			{
				if ((fd = (int) Fopen(path, 0)) < 0)
				{
					__set_errno(_XltErr(fd));
					return -1;
				}
				magic = 0;
				(void) Fread(fd, 2, (char *) &magic);
				(void) Fclose(fd);
				if (magic == 0x601A		/* TOS executable */
					|| magic == 0x2321)	/* "#!" shell file */
					st->st_mode |= 0111;
			}
			st->st_size = d.dta_size;
			/* in Unix, blocks are measured in 512 bytes */
			st->st_blocks = (st->st_size + 511) / 512;
			st->st_nlink = 1;			/* we dont have hard links */
		} else
		{
		  fill_dir:
			st->st_size = 1024;
			st->st_blocks = 2;
			st->st_nlink = 2;			/* "foo" && "foo/.." */
		}

		st->st_uid = geteuid();			/* the current user owns every file */
		st->st_gid = getegid();
		st->st_blksize = 1024;

	}

	return 0;
}

/* ------------------------------------------------------------------------- */

static int mint_stat(const char *path, struct stat *st)
{
	return __do_stat(path, st, 0);
}

/* ------------------------------------------------------------------------- */

static int mint_lstat(const char *path, struct stat *st)
{
	return __do_stat(path, st, 1);
}

/* ------------------------------------------------------------------------- */

static long __sys_fstat (short fd, struct stat *st, int exact)
{
	long r;

	/* first try the native syscall */
	r = Ffstat64 (fd, st);
	if (r == -ETOS_NOSYS)
	{
		/* try the stat64 fcntl() */
		r = Fcntl (fd, st, FSTAT64);
		if (r == -ETOS_NOSYS || r == -ETOS_INVAL)
		{
			/* fall back to the xattr fcntl() */
			struct xattr xattr;

			r = Fcntl (fd, &xattr, FSTAT);
			if (r == 0)
			{
				memset(st, 0, sizeof (*st));

				__quad_make(st->st_dev, 0, xattr.st_dev);
				st->st_ino = (ino_t) xattr.st_ino;
				st->st_mode = (mode_t) xattr.st_mode;
				st->st_nlink = (nlink_t) xattr.st_nlink;
				st->st_uid = (uid_t) xattr.st_uid;
				st->st_gid = (gid_t) xattr.st_gid;
				__quad_make(st->st_rdev, 0, xattr.st_rdev);

				if (exact)
				{
					union { unsigned short s[2]; unsigned long l; } data;
					data.l = xattr.st_mtime;
					st->st_mtime = __unixtime (data.s[0], data.s[1]);
					data.l = xattr.st_atime;
					st->st_atime = __unixtime (data.s[0], data.s[1]);
					data.l = xattr.st_ctime;
					st->st_ctime = __unixtime (data.s[0], data.s[1]);
				}

				st->st_size = (off_t) xattr.st_size;
				st->st_blocks = 
					(off_t) (((off_t) xattr.st_blocks 
						 * (off_t) xattr.st_blksize) >> 9);
				st->st_blksize = xattr.st_blksize;
				/* st->st_flags = 0; */
				/* st->st_gen = 0; */
			}
		}
	}
	return r;
}

/* ------------------------------------------------------------------------- */

int __do_fstat(int fd, struct stat *st, int exact)
{
	long r;

	r = __sys_fstat(fd, st, exact);
	if (r != -ETOS_NOSYS)
	{
		if (r)
		{
			__set_errno(_XltErr((int)r));
			return -1;
		}
		return 0;
	}

	/* emulation for TOS */
	memset(st, 0, sizeof (*st));

	{
		long oldplace;
		_DOSTIME timeptr;
		short magic;

		if (!__mint && isatty(fd))
			r = -ETOS_BADF;
		else
			r = Fdatime(&timeptr, fd, 0);
		
		if (r < 0)			/* assume TTY */
		{
			st->st_mode = S_IFCHR | 0600;
			st->st_flags = 0;
			if (exact)
				st->st_mtime = st->st_ctime = st->st_atime =
					time ((time_t*) 0) - 2;
					
			st->st_size = 0;
		} else
		{
			if (exact)
				st->st_mtime = st->st_atime = st->st_ctime =
					__unixtime(timeptr.time, timeptr.date);
			st->st_mode = S_IFREG | 0644;	/* this may be false */
			st->st_flags = 0;		/* because this is */

			/* get current file location */
			oldplace = Fseek(0L, fd, SEEK_CUR);
			if (oldplace < 0)	/* can't seek -- must be pipe */
			{
				st->st_mode = S_IFIFO | 0644;
				st->st_size = 1024;
			} else
			{
				/* Go to end of file. */
				r = Fseek(0L, fd, SEEK_END);
				st->st_size = r;
				/* Go to start of file. */
				(void) Fseek (0L, fd, SEEK_SET);
				/* Check for executable file. */
				if (Fread (fd, 2, (char *)&magic) == 2)
				{
					if (magic == 0x601a 
					    || magic == 0x2321)
						st->st_mode |= 0111;
				}
				(void) Fseek (oldplace, fd, SEEK_SET);
			}
		}

		/* all this stuff is likely bogus as well. sigh. */
		__quad_make(st->st_dev, 0, Dgetdrv());
		__quad_make(st->st_rdev, 0, 0);
		st->st_uid = getuid();
		st->st_gid = getgid();
		st->st_blksize = 1024;
		/* Note: most Unixes measure st_blocks in 512 byte units. */
		st->st_blocks = (st->st_size + 511) / 512;
		st->st_ino = __inode++;
		st->st_nlink = 1;
	
	}

	return 0;
}

/* ------------------------------------------------------------------------- */

static int mint_fstat(int fd, struct stat *st)
{
	return __do_fstat(fd, st, 1);
}


#define real_stat mint_stat
#define real_lstat mint_lstat
#define real_fstat mint_fstat

#else

#define real_stat stat
#ifdef HAVE_LSTAT
#define real_lstat lstat
#else
#define real_lstat stat
#endif
#define real_fstat fstat

#endif

/* ------------------------------------------------------------------------- */

/* Store information about NAME into ST.  Work around bugs with
   trailing slashes.  Mingw has other bugs (such as st_ino always
   being 0 on success) which this wrapper does not work around.  But
   at least this implementation provides the ability to emulate fchdir
   correctly.
*/
static int rpl_do_stat(char const *name, struct stat *st, int lflag)
{
	int result = (lflag ? real_lstat : real_stat)(name, st);

	/*
	 * Solaris 9 mistakenly succeeds when given a non-directory with a
	 * trailing slash.
	 */
	if (result == 0 && !S_ISDIR(st->st_mode))
	{
		size_t len = strlen(name);

		if (len > 0 && G_IS_DIR_SEPARATOR(name[len - 1]))
		{
			errno = ENOTDIR;
			return -1;
		}
	}

	if (result == -1 && errno == ENOENT)
	{
		/* Due to mingw's oddities, there are some directories (like
		   c:\) where stat() only succeeds with a trailing slash, and
		   other directories (like c:\windows) where stat() only
		   succeeds without a trailing slash.  But we want the two to be
		   synonymous, since chdir() manages either style.  Likewise, Mingw also
		   reports ENOENT for names longer than PATH_MAX, when we want
		   ENAMETOOLONG, and for stat("file/"), when we want ENOTDIR.
		   Fortunately, mingw PATH_MAX is small enough for stack
		   allocation. */
		char fixed_name[PATH_MAX + 1];
		size_t len = strlen(name);
		int check_dir = FALSE;

		if (PATH_MAX <= len)
		{
			errno = ENAMETOOLONG;
		} else if (len)
		{
			strcpy(fixed_name, name);
			if (G_IS_DIR_SEPARATOR(fixed_name[len - 1]))
			{
				check_dir = TRUE;
				while (len && G_IS_DIR_SEPARATOR(fixed_name[len - 1]))
					fixed_name[--len] = '\0';
				if (!len)
					fixed_name[0] = '/';
			} else
			{
				fixed_name[len++] = '/';
			}
			result = (lflag ? real_lstat : real_stat)(fixed_name, st);
			if (result == 0 && check_dir && !S_ISDIR(st->st_mode))
			{
				result = -1;
				errno = ENOTDIR;
			}
		}
	}

	return result;
}

/* ------------------------------------------------------------------------- */

int rpl_stat(char const *name, struct stat *st)
{
	return rpl_do_stat(name, st, 0);
}

/* ------------------------------------------------------------------------- */

int rpl_lstat(char const *name, struct stat *st)
{
	return rpl_do_stat(name, st, 1);
}

/* ------------------------------------------------------------------------- */

int rpl_fstat(int handle, struct stat *st)
{
	return real_fstat(handle, st);
}

/* ------------------------------------------------------------------------- */

int hyp_utf8_stat(char const *name, struct stat *st)
{
	/* TODO: hyp_utf8_stat */
	return rpl_do_stat(name, st, 0);
}

/* ------------------------------------------------------------------------- */

int hyp_utf8_lstat(char const *name, struct stat *st)
{
	/* TODO: hyp_utf8_lstat */
	return rpl_do_stat(name, st, 1);
}
