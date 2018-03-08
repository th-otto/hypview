#include "hypdefs.h"

#define CP_UTF16_LE 1200

struct errentry {
        unsigned long oscode;           /* OS return value */
        int errnocode;  /* System V error code */
};

static struct errentry errtable[] = {
	{  ERROR_SUCCESS,                0 },
	{  ERROR_INVALID_PARAMETER,      EINVAL    },
	{  ERROR_INVALID_FUNCTION,       ENOSYS    },
	{  ERROR_FILE_NOT_FOUND,         ENOENT    },
	{  ERROR_PATH_NOT_FOUND,         ENOENT    },
	{  ERROR_TOO_MANY_OPEN_FILES,    EMFILE    },
	{  ERROR_BAD_UNIT,               ENXIO     },
	{  ERROR_ACCESS_DENIED,          EACCES    },
	{  ERROR_ACCESS_DENIED,          EPERM     },
	{  ERROR_INVALID_HANDLE,         EBADF     },
	{  ERROR_INVALID_HANDLE,         ESRCH     },
	{  ERROR_NOT_ENOUGH_MEMORY,      ENOMEM    },
	{  ERROR_ARENA_TRASHED,          ENOMEM    },
	{  ERROR_INVALID_DRIVE,          ENODEV    },
	{  ERROR_BAD_NETPATH,            ENODEV    },
	{  ERROR_BAD_FORMAT,             ENOEXEC   },
	{  ERROR_INVALID_BLOCK,          EFAULT    },
	{  ERROR_BAD_ENVIRONMENT,        E2BIG     },
	{  ERROR_INVALID_ACCESS,         EINVAL    },
	{  ERROR_INVALID_DATA,           EINVAL    },
	{  ERROR_INVALID_DRIVE,          ENOENT    },
	{  ERROR_CANNOT_MAKE,            EISDIR    },
	{  ERROR_NOT_READY,              ENOTTY    },
	{  ERROR_CURRENT_DIRECTORY,      EBUSY     },
	{  ERROR_NOT_SAME_DEVICE,        EXDEV     },
	{  ERROR_NO_MORE_FILES,          ENOENT    },
	{  ERROR_LOCK_VIOLATION,         EACCES    },
	{  ERROR_INVALID_FUNCTION,       ENOSYS    },
	{  ERROR_BAD_NETPATH,            ENOENT    },
	{  ERROR_NETWORK_ACCESS_DENIED,  EACCES    },
	{  ERROR_BAD_NET_NAME,           ENOENT    },
	{  ERROR_FILE_EXISTS,            EEXIST    },
	{  ERROR_CANNOT_MAKE,            EACCES    },
	{  ERROR_FAIL_I24,               EACCES    },
	{  ERROR_NO_PROC_SLOTS,          EAGAIN    },
	{  ERROR_DRIVE_LOCKED,           EACCES    },
	{  ERROR_BROKEN_PIPE,            EPIPE     },
	{  223,                          EFBIG     },
	{  ERROR_DISK_FULL,              ENOSPC    },
	{  ERROR_INVALID_TARGET_HANDLE,  EBADF     },
	{  ERROR_INVALID_HANDLE,         EINVAL    },
	{  ERROR_WAIT_NO_CHILDREN,       ECHILD    },
	{  ERROR_CHILD_NOT_COMPLETE,     ECHILD    },
	{  ERROR_DIRECT_ACCESS_HANDLE,   EBADF     },
	{  ERROR_NEGATIVE_SEEK,          EINVAL    },
	{  ERROR_SEEK_ON_DEVICE,         EACCES    },
	{  ERROR_DIR_NOT_EMPTY,          ENOTEMPTY },
	{  ERROR_NOT_LOCKED,             EACCES    },
	{  ERROR_BAD_PATHNAME,           ENOENT    },
	{  ERROR_MAX_THRDS_REACHED,      EAGAIN    },
	{  ERROR_LOCK_FAILED,            EACCES    },
	{  ERROR_ALREADY_EXISTS,         EEXIST    },
	{  ERROR_FILENAME_EXCED_RANGE,   ENOENT    },
	{  ERROR_NESTING_NOT_ALLOWED,    EAGAIN    },
	{  ERROR_NOT_ENOUGH_QUOTA,       ENOMEM    },
	{  ERROR_SEEK,                   ESPIPE    },
	{  ERROR_WRITE_PROTECT,          EROFS     },
	{  ERROR_FILENAME_EXCED_RANGE,   ENAMETOOLONG },
};


/* size of the table */
#define ERRTABLESIZE (sizeof(errtable)/sizeof(errtable[0]))

/* The following two constants must be the minimum and maximum
   values in the (contiguous) range of Exec Failure errors. */
#define MIN_EXEC_ERROR ERROR_INVALID_STARTING_CODESEG
#define MAX_EXEC_ERROR ERROR_INFLOOP_IN_RELOC_CHAIN

/* These are the low and high value in the range of errors that are
   access violations */
#define MIN_EACCES_RANGE ERROR_WRITE_PROTECT
#define MAX_EACCES_RANGE ERROR_SHARING_BUFFER_EXCEEDED


char *win32_errstring(DWORD err)
{
	wchar_t buf[2048];
	size_t len;
	char *res;
	
	buf[0] = 0;
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, buf, sizeof(buf) / sizeof(buf[0]), NULL);
	res = hyp_wchar_to_utf8(buf, STR0TERM);
	if (res == NULL)
		return g_strdup_printf(_("unknown win32 error %u"), (unsigned int) err);
	len = strlen(res);
	while (len && (res[len - 1] == '\n' || res[len - 1] == '\r'))
		res[--len] = '\0';
	return res;
}


int win32_to_errno(DWORD oserrno)
{
	size_t i;

	/* check the table for the OS error code */
	for (i = 0; i < ERRTABLESIZE; ++i)
	{
		if (oserrno == errtable[i].oscode)
			return errtable[i].errnocode;
	}

	/* The error code wasn't in the table.	We check for a range of */
	/* EACCES errors or exec failure errors (ENOEXEC).  Otherwise	*/
	/* EINVAL is returned.						*/

	if (oserrno >= MIN_EACCES_RANGE && oserrno <= MAX_EACCES_RANGE)
		return EACCES;
	if (oserrno >= MIN_EXEC_ERROR && oserrno <= MAX_EXEC_ERROR)
		return ENOEXEC;
	return EINVAL;
}


DWORD win32_from_errno(int err_no)
{
	size_t i;

	/* check the table for the OS error code */
	for (i = 0; i < ERRTABLESIZE; ++i)
	{
		if (err_no == errtable[i].errnocode)
			return errtable[i].oscode;
	}
	return ERROR_INVALID_PARAMETER;
}


char *hyp_utf8_strerror(int err)
{
	return win32_errstring(win32_from_errno(err));
}


int hyp_utf8_open(const char *filename, int flags, mode_t mode)
{
	wchar_t *wstr;
	int fd;
	size_t len;
	
	wstr = hyp_utf8_to_wchar(filename, STR0TERM, &len);
	if (G_UNLIKELY(wstr == NULL))
		return -1;
	fd = _wopen(wstr, flags, mode);
	g_free(wstr);
	return fd;
}


int hyp_utf8_unlink(const char *name)
{
	wchar_t *wstr;
	int ret;
	size_t len;
	
	wstr = hyp_utf8_to_wchar(name, STR0TERM, &len);
	if (G_UNLIKELY(wstr == NULL))
		return -1;
	ret = _wunlink(wstr);
	g_free(wstr);
	return ret;
}


int hyp_utf8_rename(const char *oldname, const char *newname)
{
	wchar_t *wstr1, *wstr2;
	int ret;
	size_t len;
	
	wstr1 = hyp_utf8_to_wchar(oldname, STR0TERM, &len);
	wstr2 = hyp_utf8_to_wchar(newname, STR0TERM, &len);
	if (G_UNLIKELY(wstr1 == NULL || wstr2 == NULL))
	{
		g_free(wstr2);
		g_free(wstr1);
		return -1;
	}
	ret = _wrename(wstr1, wstr2);
	g_free(wstr2);
	g_free(wstr1);
	return ret;
}


FILE *hyp_utf8_fopen(const char *filename, const char *mode)
{
	wchar_t *wstr;
	FILE *fp;
	size_t len;
	wchar_t *wmode;
	
	wmode = hyp_utf8_to_wchar(mode, STR0TERM, &len);
	wstr = hyp_utf8_to_wchar(filename, STR0TERM, &len);
	if (G_UNLIKELY(wmode == NULL || wstr == NULL))
	{
		g_free(wstr);
		g_free(wmode);
		return NULL;
	}
	fp = _wfopen(wstr, wmode);
	g_free(wstr);
	g_free(wmode);
	return fp;
}


DIR *hyp_utf8_opendir(const char *dirname)
{
	wchar_t *wstr;
	_WDIR *dir;
	size_t len;

	wstr = hyp_utf8_to_wchar(dirname, STR0TERM, &len);
	if (G_UNLIKELY(wstr == NULL))
		return NULL;
	dir = _wopendir(wstr);
	g_free(wstr);
	return (DIR *)dir;
}

char *hyp_utf8_readdir(DIR *dir)
{
	_WDIR *wdir = (_WDIR *)dir;
	struct _wdirent *ent;
	char *str;
	
	ent = _wreaddir(wdir);
	if (ent == NULL)
		return NULL;
	str = hyp_wchar_to_utf8(ent->d_name, STR0TERM);
	return str;
}


void hyp_utf8_closedir(DIR *dir)
{
	_WDIR *wdir = (_WDIR *)dir;
	_wclosedir(wdir);
}

