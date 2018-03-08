#include "hypdefs.h"


#ifdef __TOS__
static char bootDrive = -1;

static long getBootDrive(void)
{
	bootDrive = *((char *) 0x447) + 'A';

	return bootDrive;
}


char GetBootDrive(void)
{
	Supexec(getBootDrive);
	return bootDrive;
}


#if defined(__PUREC__) && defined(_PUREC_SOURCE)
/*
 * Work-around for bug in Pure-C
 */
int purec_fclose(FILE *fp)
{
	int ret = 0;
	fflush(fp);
	if (ferror(fp))
	{
		ret = (fclose)(fp);
		/* workaround a library bug */
		if (fp->Flags & 3)
		{
			close(fileno(fp));
			if (fp->Flags & 0x08)
				(free)(fp->BufStart);
			fp->Flags = 0;
		}
		return ret;
	}
	return (fclose)(fp);
}
#endif

#endif



#ifdef __WIN32__

/* in win32.c */

#else

int hyp_utf8_open(const char *filename, int flags, mode_t mode)
{
	char *str;
	gboolean converror = FALSE;
	int fd;
	
	str = hyp_utf8_to_charset(hyp_get_filename_charset(), filename, STR0TERM, &converror);
	if (G_UNLIKELY(str == NULL))
		return -1;
	fd = open(str, flags, mode);
	g_free(str);
	return fd;
}


FILE *hyp_utf8_fopen(const char *filename, const char *mode)
{
	char *str;
	gboolean converror = FALSE;
	FILE *fp;
	
	str = hyp_utf8_to_charset(hyp_get_filename_charset(), filename, STR0TERM, &converror);
	if (G_UNLIKELY(str == NULL))
		return NULL;
	fp = fopen(str, mode);
	g_free(str);
	return fp;
}


int hyp_utf8_unlink(const char *name)
{
	char *str;
	int ret;
	
	gboolean converror = FALSE;
	str = hyp_utf8_to_charset(hyp_get_filename_charset(), name, STR0TERM, &converror);
	if (G_UNLIKELY(str == NULL))
		return -1;
	ret = unlink(str);
	g_free(str);
	return ret;
}


int hyp_utf8_rename(const char *oldname, const char *newname)
{
	char *str1, *str2;
	int ret;
	
	gboolean converror = FALSE;
	str1 = hyp_utf8_to_charset(hyp_get_filename_charset(), oldname, STR0TERM, &converror);
	str2 = hyp_utf8_to_charset(hyp_get_filename_charset(), newname, STR0TERM, &converror);
	if (G_UNLIKELY(str1 == NULL || str2 == NULL))
	{
		g_free(str2);
		g_free(str1);
		return -1;
	}
	ret = rename(str1, str2);
	g_free(str2);
	g_free(str1);
	return ret;
}


char *hyp_utf8_strerror(int err)
{
	const char *str = strerror(err);
	return hyp_conv_to_utf8(hyp_get_current_charset(), str, STR0TERM);
}


DIR *hyp_utf8_opendir(const char *dirname)
{
	char *str;
	DIR *dir;
	
	gboolean converror = FALSE;
	str = hyp_utf8_to_charset(hyp_get_filename_charset(), dirname, STR0TERM, &converror);
	if (G_UNLIKELY(str == NULL))
		return NULL;
	dir = opendir(str);
	g_free(str);
	return dir;
}


char *hyp_utf8_readdir(DIR *dir)
{
	struct dirent *ent;
	gboolean converror = FALSE;
	char *str;
	
	ent = readdir(dir);
	if (ent == NULL)
		return NULL;
	str = hyp_conv_charset(hyp_get_filename_charset(), HYP_CHARSET_UTF8, ent->d_name, STR0TERM, &converror);
	return str;
}


void hyp_utf8_closedir(DIR *dir)
{
	closedir(dir);
}

#endif


static gboolean do_walk_dir(const char *dirname, gboolean (*f)(const char *filename, void *data), void *data, gboolean retval_if_fails)
{
	DIR *dir;
	char *ent;
	gboolean ret = TRUE;
	char *path;
	struct stat st;
	
	dir = hyp_utf8_opendir(dirname);
	if (dir == NULL)
		return retval_if_fails;
	while ((ent = hyp_utf8_readdir(dir)) != NULL)
	{
		if (strcmp(ent, ".") == 0 ||
			strcmp(ent, "..") == 0)
		{
			g_free(ent);
			continue;
		}
		path = g_build_filename(dirname, ent, NULL);
		g_free(ent);
		if (path == NULL)
		{
			ret = FALSE;
			break;
		}
		if (hyp_utf8_stat(path, &st) < 0)
		{
			ret = TRUE;
		} else if (S_ISDIR(st.st_mode))
		{
			ret = do_walk_dir(path, f, data, TRUE);
		} else if (S_ISREG(st.st_mode))
		{
			ret = (*f)(path, data);
		} else
		{
			ret = TRUE;
		}
		g_free(path);
		if (ret == FALSE)
			break;
	}
	hyp_utf8_closedir(dir);
	return ret;
}


gboolean walk_dir(const char *dirname, gboolean (*f)(const char *filename, void *data), void *data)
{
	return do_walk_dir(dirname, f, data, FALSE);
}


gboolean walk_pathlist(const char *list, gboolean (*f)(const char *filename, void *data), void *data)
{
	const char *end;
	char *tmp, *dirname;
	gboolean ret;
	
	if (empty(list))
		return TRUE;
	while (*list)
	{
		end = list;
		while (*end != '\0' && (*end != G_SEARCHPATH_SEPARATOR))
			end++;
		tmp = g_strndup(list, end - list);
		dirname = path_subst(tmp);
		g_free(tmp);
		ret = do_walk_dir(dirname, f, data, TRUE);
		g_free(dirname);
		if (ret == FALSE)
			return FALSE;
	
		list = end;
		if (*list != '\0')
			list++;
	}
	return TRUE;
}


/*
 * just for consistency
 */
int hyp_utf8_close(int fd)
{
	return close(fd);
}


int hyp_utf8_fclose(FILE *fp)
{
	return fclose(fp);
}
